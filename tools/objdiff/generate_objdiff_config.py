#!/usr/bin/env python3
"""
Generate objdiff.json for progress tracking across the main executable and overlays.

Reads the splat YAML configs to discover all C translation units, then writes
an objdiff.json with:
  - One progress category per build target ("main", "checkps", etc.)
  - One unit per C file, with target_path (splat asm) and base_path (compiled C)
  - Units tagged with their category for per-overlay progress breakdowns

Usage:
    python3 tools/objdiff/generate_objdiff_config.py
"""

import json
import yaml
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent.parent
MAIN_CONFIG = PROJECT_ROOT / "config" / "SLUS_010.13.yaml"
OVERLAY_CONFIG_DIR = PROJECT_ROOT / "config" / "overlays"
OUTPUT_PATH = PROJECT_ROOT / "objdiff.json"

# SDK files are not our code — skip them in progress tracking
SKIP_PATHS = {"psyq"}

# Per-overlay set of rodata segment names that have a compiled C counterpart.
# These are built via nolink_srcs in the Makefile (compiled for objdiff but
# excluded from the overlay link to avoid duplicate-symbol conflicts).
OVERLAY_NOLINK_SRCS: dict[str, set[str]] = {
    "checkps": {"data"},
}


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


def extract_rodata_subsegments(config: dict) -> list[str]:
    """Pull out file names from [offset, 'rodata', name] subsegment entries.

    These are binary-extracted .rodata.s references that also have a compiled
    C counterpart built via nolink_srcs (compiled for objdiff but not linked).
    """
    names = []
    for segment in config.get("segments", []):
        if not isinstance(segment, dict):
            continue
        for subseg in segment.get("subsegments", []):
            if isinstance(subseg, list) and len(subseg) >= 3 and subseg[1] == "rodata":
                names.append(subseg[2])
    return names


def should_skip(file_name: str) -> bool:
    return any(part in SKIP_PATHS for part in file_name.split("/"))


def build_main_units(config: dict) -> list[dict]:
    """Create units for the main SLUS executable."""
    units = []
    for name in extract_c_subsegments(config):
        if should_skip(name):
            continue
        units.append({
            "name": f"main/{name}",
            "target_path": f"build/asm/{name}.o",
            "base_path": f"build/src/{name}.o",
            "metadata": {
                "progress_categories": ["main"],
            },
        })
    return units


def build_overlay_units(config: dict, overlay_name: str) -> list[dict]:
    """Create units for one overlay from its splat config."""
    options = config.get("options", {})
    asm_path = options.get("asm_path", f"asm/overlays/{overlay_name}")
    build_path = options.get("build_path", f"build/overlays/{overlay_name}")
    src_path = options.get("src_path", f"src/overlays/{overlay_name}")

    nolink = OVERLAY_NOLINK_SRCS.get(overlay_name, set())

    units = []
    for name in extract_c_subsegments(config):
        if should_skip(name):
            continue
        units.append({
            "name": f"{overlay_name}/{name}",
            "target_path": f"{build_path}/target/{name}.o",
            "base_path": f"{build_path}/{src_path}/{name}.o",
            "metadata": {
                "progress_categories": [overlay_name],
            },
        })

    # Also emit units for rodata segments that have a compiled C counterpart
    # (built via nolink_srcs in the Makefile).
    for name in extract_rodata_subsegments(config):
        if should_skip(name) or name not in nolink:
            continue
        units.append({
            "name": f"{overlay_name}/{name}",
            "target_path": f"{build_path}/target/{name}.o",
            "base_path": f"{build_path}/{src_path}/{name}.o",
            "metadata": {
                "progress_categories": [overlay_name],
            },
        })

    return units


def discover_overlay_configs() -> list[Path]:
    """Find all overlay YAML configs in config/overlays/."""
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
    categories = [{"id": "main", "name": "Main Executable"}]
    units = []

    # ── Main executable ──
    if MAIN_CONFIG.exists():
        main_cfg = load_yaml(MAIN_CONFIG)
        units.extend(build_main_units(main_cfg))
        print(f"Main executable: {len(units)} units")

    # ── Overlays ──
    for cfg_path in discover_overlay_configs():
        cfg = load_yaml(cfg_path)
        name = overlay_name_from_config(cfg)
        overlay_units = build_overlay_units(cfg, name)
        units.extend(overlay_units)
        categories.append({"id": name, "name": name.upper()})
        print(f"Overlay {name}: {len(overlay_units)} units")

    # ── Write objdiff.json ──
    objdiff_config = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "custom_make": "make",
        "custom_args": ["objdiff-objects"],
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