#!/usr/bin/env python3
"""
Generate objdiff.json for progress tracking across the main executable and overlays.

Reads the splat YAML configs to discover all C translation units, then writes
an objdiff.json with:
  - One progress category per build target ("main", "checkps", etc.)
  - One unit per C file, with target_path (splat asm) and base_path (compiled C)
  - Units tagged with their category for per-overlay progress breakdowns

Usage:
    python3 tools/objdiff/generate_objdiff_config.py [--version us]
"""

import argparse
import json
import yaml
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent.parent
OUTPUT_PATH = PROJECT_ROOT / "objdiff.json"

# Main executable file name per game version (mirrors mk/version.mk).
MAIN_EXECUTABLES = {
    "us": "SLUS_010.13",
    "jp": "SLPS_021.70",
}

# Set by configure_version() from the --version argument.
VERSION = "us"
MAIN_CONFIG = Path()
OVERLAY_CONFIG_DIR = Path()

# Manifest of overlays whose rebuilt+compressed BIN matches the original ROM.
# Written by the Makefile's verify-* targets. Units of overlays listed here are
# stamped with metadata.complete = true so objdiff reports them as linked.
COMPLETE_MANIFEST = Path()


def configure_version(version: str) -> None:
    """Point the config, overlay, and manifest paths at one game version."""
    global VERSION, MAIN_CONFIG, OVERLAY_CONFIG_DIR, COMPLETE_MANIFEST
    VERSION = version
    config_dir = PROJECT_ROOT / "config" / version
    MAIN_CONFIG = config_dir / f"{MAIN_EXECUTABLES[version]}.yaml"
    OVERLAY_CONFIG_DIR = config_dir / "overlays"
    COMPLETE_MANIFEST = PROJECT_ROOT / "build" / version / "complete_overlays.txt"

# SDK files are not our code — skip them in progress tracking
SKIP_PATHS = {"psyq"}

def load_yaml(path: Path) -> dict:
    with open(path, "r") as f:
        return yaml.safe_load(f)


def extract_c_subsegments(config: dict) -> list[str]:
    """Pull out file names from [offset, 'c', name] subsegment entries."""
    names = []
    for segment in config.get("segments", []):
        if not isinstance(segment, dict):
            continue
        for subseg in segment.get("subsegments", []):
            if isinstance(subseg, list) and len(subseg) >= 3 and subseg[1] == "c":
                names.append(subseg[2])
    return names


def extract_asm_subsegments(config: dict) -> list[str]:
    """Pull out names of [offset, 'asm', name] subsegments.

    These are code ranges not yet split into C translation units (the whole
    first-pass layout of a newly added game version). They become target-only
    units: objdiff counts their code toward the total with nothing matched.
    """
    names = []
    for segment in config.get("segments", []):
        if not isinstance(segment, dict):
            continue
        for subseg in segment.get("subsegments", []):
            if isinstance(subseg, list) and len(subseg) >= 3 and subseg[1] == "asm":
                names.append(subseg[2])
    return names


def extract_data_subsegments(config: dict) -> list[str]:
    """Pull out names of standalone data translation units.

    Matches [offset, '.data', name] subsegments whose name is not also a 'c'
    subsegment - i.e. data built by its own C file, which has a compiled base
    object and a generated target object to diff. Raw databin subsegments have
    no compiled C base object and are intentionally omitted.
    """
    c_names = set(extract_c_subsegments(config))
    names = []
    for segment in config.get("segments", []):
        if not isinstance(segment, dict):
            continue
        for subseg in segment.get("subsegments", []):
            if (isinstance(subseg, list) and len(subseg) >= 3
                    and subseg[1] == ".data"
                    and subseg[2] not in c_names):
                names.append(subseg[2])
    return names


def should_skip(file_name: str) -> bool:
    return any(part in SKIP_PATHS for part in file_name.split("/"))


def load_complete_overlays() -> set[str]:
    """Overlays whose rebuilt compressed BIN matched the original ROM SHA1."""
    if not COMPLETE_MANIFEST.exists():
        return set()
    return {
        line.strip()
        for line in COMPLETE_MANIFEST.read_text().splitlines()
        if line.strip()
    }


def build_main_units(config: dict, complete: bool = False) -> list[dict]:
    """Create units for the main SLUS executable.

    If `complete` is True (the linked executable matches the disc file), every
    unit is stamped with metadata.complete = true.
    """
    options = config.get("options", {})
    asm_path = options.get("asm_path", f"asm/{VERSION}")
    build_path = options.get("build_path", f"build/{VERSION}")
    src_path = options.get("src_path", "src")

    units = []
    for name in extract_c_subsegments(config):
        if should_skip(name):
            continue
        units.append({
            "name": f"main/{name}",
            "target_path": f"{build_path}/{asm_path}/{name}.o",
            "base_path": f"{build_path}/{src_path}/{name}.o",
            "metadata": {
                "progress_categories": ["main"],
                **({"complete": True} if complete else {}),
            },
        })
    for name in extract_asm_subsegments(config):
        units.append({
            "name": f"main/{name}",
            "target_path": f"{build_path}/{asm_path}/{name}.o",
            "metadata": {"progress_categories": ["main"]},
        })
    return units


def build_overlay_units(config: dict, overlay_name: str, complete: bool = False) -> list[dict]:
    """Create units for one overlay from its splat config.

    If `complete` is True, every unit is stamped with metadata.complete = true,
    which tells objdiff to treat the object as fully linked/matching.
    """
    options = config.get("options", {})
    build_path = options.get("build_path", f"build/{VERSION}/overlays/{overlay_name}")
    src_path = options.get("src_path", f"src/overlays/{overlay_name}")

    def _metadata() -> dict:
        meta = {"progress_categories": [overlay_name]}
        if complete:
            meta["complete"] = True
        return meta

    units = []
    for name in extract_c_subsegments(config):
        if should_skip(name):
            continue
        units.append({
            "name": f"{overlay_name}/{name}",
            "target_path": f"{build_path}/target/{name}.o",
            "base_path": f"{build_path}/{src_path}/{name}.o",
            "metadata": _metadata(),
        })

    # Code not yet split into C files: target-only units (see
    # extract_asm_subsegments). Never marked complete.
    for name in extract_asm_subsegments(config):
        units.append({
            "name": f"{overlay_name}/{name}",
            "target_path": f"{build_path}/target/{name}.o",
            "metadata": {"progress_categories": [overlay_name]},
        })

    # Standalone data translation units: the target object is generated from an
    # answer-key assembly file containing the original bytes; the base is the
    # compiled C data file.
    for name in extract_data_subsegments(config):
        if should_skip(name):
            continue
        units.append({
            "name": f"{overlay_name}/{name}",
            "target_path": f"{build_path}/target/{name}.o",
            "base_path": f"{build_path}/{src_path}/{name}.o",
            "metadata": _metadata(),
        })

    return units


def discover_overlay_configs() -> list[Path]:
    """Find all overlay YAML configs in config/<version>/overlays/."""
    if not OVERLAY_CONFIG_DIR.is_dir():
        return []
    return sorted(OVERLAY_CONFIG_DIR.glob("*.yaml"))


def overlay_name_from_config(config: dict) -> str:
    """Derive the lowercase overlay directory name from the config.

    Uses the segment name (e.g. 'checkps') which matches the directory
    convention: src/overlays/checkps/, asm/overlays/checkps/, etc.
    """
    for segment in config.get("segments", []):
        if isinstance(segment, dict) and "name" in segment:
            return segment["name"]
    # Fallback: lowercase the config basename without extension
    return config.get("name", "unknown").split(".")[0].lower()


def main():
    parser = argparse.ArgumentParser(description="Generate objdiff.json for one game version.")
    parser.add_argument(
        "--version",
        choices=sorted(MAIN_EXECUTABLES),
        default="us",
        help="game version whose configs and build outputs to use (default: us)",
    )
    configure_version(parser.parse_args().version)

    categories = [{"id": "main", "name": "Main Executable"}]
    units = []

    complete_overlays = load_complete_overlays()

    # ── Main executable ──
    if MAIN_CONFIG.exists():
        main_cfg = load_yaml(MAIN_CONFIG)
        main_complete = "main" in complete_overlays
        units.extend(build_main_units(main_cfg, complete=main_complete))
        print(f"Main executable: {len(units)} units" + (" [complete]" if main_complete else ""))

    # ── Overlays ──
    if complete_overlays:
        print(f"Complete overlays (BIN sha matches ROM): {sorted(complete_overlays)}")

    for cfg_path in discover_overlay_configs():
        cfg = load_yaml(cfg_path)
        name = overlay_name_from_config(cfg)
        is_complete = name in complete_overlays
        overlay_units = build_overlay_units(cfg, name, complete=is_complete)
        units.extend(overlay_units)
        categories.append({"id": name, "name": name.upper()})
        suffix = " [complete]" if is_complete else ""
        print(f"Overlay {name}: {len(overlay_units)} units{suffix}")

    # ── Write objdiff.json ──
    objdiff_config = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "custom_make": "make",
        "custom_args": ["objdiff-objects", f"VERSION={VERSION}"],
        "build_target": True,
        "build_base": True,
        "watch_patterns": [
            "*.c", "*.h", "*.s", "*.inc",
        ],
        "units": units,
        "progress_categories": categories,
    }

    with open(OUTPUT_PATH, "w") as f:
        json.dump(objdiff_config, f, indent=2)
        f.write("\n")

    print(f"\nWrote {OUTPUT_PATH} — {len(units)} total units, {len(categories)} categories")


if __name__ == "__main__":
    main()
