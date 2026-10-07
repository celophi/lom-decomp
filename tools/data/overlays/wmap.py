#!/usr/bin/env python3
"""Export WMAP's world-map tables, sound effects and step tables as readable files.

The build links one unchanged data blob. Readers describe its resources as
Parts and the byte map covers the whole blob, including the large zero-filled
runtime state at its end. Fixed table layouts live in wmap_tables.py.

WMAP runs most of its animation as step tables: arrays of function pointers
that a sequence walks one entry at a time. Which symbols are step tables comes
from their declarations in src/overlays/wmap, and every entry is written as
the name of the function it points to.

Example:

    python3 -m tools.data.overlays.wmap --version us assets/exports/us/overlays/wmap
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass, field
from pathlib import Path
import re
import shutil
import struct
import sys
import tempfile

from tools.data.overlays import splat_config, symbols
from tools.data.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)
from tools.data.overlays.wmap_tables import TABLES

REPO_ROOT = Path(__file__).resolve().parents[3]
OVERLAY_CONFIG = "overlays/WMAP.BIN.yaml"
SYMBOL_FILE = "symbols/wmap_symbol_addrs.txt"
SOURCES = REPO_ROOT / "src/overlays/wmap"
SCRIPT_SOURCE = SOURCES / "wmap_main.c"

# g_wmap_sfx_buffers has one pointer per sound; the code plays sound id N from entry N - 1.
SOUND_COUNT = 63
SOUND_MAGIC = b"AKAO"
SOUND_LIST = struct.Struct("<4siiI")  # akao_sfx_play_list: magic, entry count, bank key, reserved
SOUND_OFFSETS = 0x10
SOUND_DATA = 0x20
NO_SEQUENCE = 0xFFFF
INPUT_SCRIPT_COUNT = 3
# Script commands that take one argument word (wmap_step_input_script).
SCRIPT_ARGUMENT_COMMANDS = ("WMAP_SCRIPT_IMAGE", "WMAP_SCRIPT_BUTTON_MASK", "WMAP_SCRIPT_VISIBILITY")
HANDLER_DECLARATION = re.compile(
    r"^\s*extern\s+(?:void\s*\(\s*\*\s*(\w+)\s*\[[^\]]*\]\s*\)\s*\([^)]*\)|WmapHandler\s+(\w+)\s*\[[^\]]*\])\s*;",
    re.MULTILINE,
)


# ---------------------------------------------------------------------------
# Inputs and decoded parts


@dataclass(frozen=True)
class WmapSymbols:
    """Resource anchors plus the complete symbol map for table and function names."""

    first: int
    sounds: int
    input_scripts: int
    variables: int
    named: dict[str, int]
    sizes: dict[str, int] = field(default_factory=dict)

    @classmethod
    def load(cls, path: Path) -> WmapSymbols:
        named = symbols.load(path)
        required = set(SYMBOL_NAMES.values()) | {spec.symbol for spec in TABLES}
        missing = sorted(required - named.keys())
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in tools/data/overlays/wmap.py or TABLES in wmap_tables.py."
            )
        return cls(**{key: named[name] for key, name in SYMBOL_NAMES.items()}, named=named, sizes=symbols.load_sizes(path))


SYMBOL_NAMES = {
    "first": "g_wmap_game_continue_prompt",
    "sounds": "g_wmap_sfx_buffers",
    "input_scripts": "g_wmap_input_scripts",
    "variables": "g_wmap_game_displayed_score",
}


@dataclass(frozen=True)
class Inputs:
    version: str
    config: Path
    assets: Path


@dataclass(frozen=True)
class Table:
    address: int
    values: dict[str, object]
    raw: bytes


@dataclass(frozen=True)
class Document:
    """A YAML file that describes several parts of the blob."""

    values: dict[str, object]


@dataclass(frozen=True)
class HandlerTable:
    symbol: str
    source: str
    address: int
    steps: tuple[str | dict[str, str] | None, ...]


def read_bytes(blob: Blob, address: int, size: int, what: str) -> bytes:
    start = blob.offset(address)
    if size < 0 or start < 0 or start + size > len(blob.data):
        raise ValueError(f"{what} is outside {blob.file_name}")
    return blob.data[start : start + size]


def table_part(blob: Blob, name: str, address: int, raw: bytes, values: dict, file: str, note: str = "") -> Part:
    start = blob.offset(address)
    return Part(name, start, start + len(raw), file, Table(address, values, raw), note or None)


def symbol_aliases(named: dict[str, int]) -> dict[int, list[str]]:
    aliases: dict[int, list[str]] = {}
    for name, address in named.items():
        aliases.setdefault(address, []).append(name)
    return {address: sorted(names) for address, names in aliases.items()}


def next_symbol(blob: Blob[WmapSymbols], address: int) -> int:
    """Where the next named symbol after @p address starts, or the blob end."""
    end = blob.address + len(blob.data)
    return min((value for value in blob.symbols.named.values() if address < value < end), default=end)


def script_names() -> dict[int, str]:
    """Scripted-input codes and commands from their owning C enum."""
    source = SCRIPT_SOURCE.read_text(encoding="ascii")
    match = re.search(r"enum\s*\{([^}]*WMAP_SCRIPT_END[^}]*)\}", source)
    if match is None:
        raise ValueError(f"{SCRIPT_SOURCE} has no WMAP_SCRIPT_ enum")
    return {int(value): name for name, value in re.findall(r"(WMAP_SCRIPT_\w+)\s*=\s*(-?\d+)", match[1])}


def handler_declarations(sources: Path = SOURCES) -> dict[str, str]:
    """Step and handler tables declared in the C sources, each with the first file that declares it."""
    tables: dict[str, str] = {}
    for path in sorted(sources.rglob("*.c"), key=lambda path: (path.name, path.as_posix())):
        for match in HANDLER_DECLARATION.finditer(path.read_text(encoding="ascii")):
            tables.setdefault(match[1] or match[2], path.relative_to(sources).with_suffix("").as_posix())
    return tables


# ---------------------------------------------------------------------------
# Reading the blob


def read_tables(blob: Blob[WmapSymbols]) -> list[Part]:
    parts = []
    aliases = symbol_aliases(blob.symbols.named)
    for spec in TABLES:
        address = blob.symbols.named[spec.symbol]
        layout = struct.Struct("<" + spec.format)
        raw = read_bytes(blob, address, layout.size * spec.count, spec.symbol)
        rows = []
        for values in layout.iter_unpack(raw):
            if spec.fields:
                row = dict(zip(spec.fields, values, strict=True))
            elif spec.resolve_symbols:
                row = {"value": hex_address(values[0])}
                if values[0] in aliases:
                    row["symbols"] = aliases[values[0]]
            else:
                row = values[0] if len(values) == 1 else list(values)
            rows.append(row)
        values = {"symbol": spec.symbol, "record_format": "<" + spec.format, "count": spec.count, "entries": rows}
        if spec.note:
            values["note"] = spec.note
        name = spec.symbol.removeprefix("g_wmap_")
        parts.append(table_part(blob, spec.symbol, address, raw, values, f"tables/{name}.yaml", spec.note))
    return parts


def read_sounds(blob: Blob[WmapSymbols]) -> list[Part]:
    """Decode the pointer table and each AKAO sound-effect list; the lists stay whole in .akao files."""
    table = blob.symbols.sounds
    raw_table = read_bytes(blob, table, SOUND_COUNT * 4, "sound effect table")
    pointers = struct.unpack(f"<{SOUND_COUNT}I", raw_table)
    starts = sorted(set(pointers))
    if starts[0] < blob.address or starts[-1] >= table:
        raise ValueError("sound effect pointers must lie between the blob start and their table")
    ends = dict(zip(starts, starts[1:] + [table]))
    entries, parts = [], []
    for index, address in enumerate(pointers):
        raw = read_bytes(blob, address, ends[address] - address, f"sound effect {index + 1}")
        if len(raw) < SOUND_DATA or raw[:4] != SOUND_MAGIC:
            raise ValueError(f"sound effect {index + 1} has no AKAO header")
        _, count, bank, _ = SOUND_LIST.unpack_from(raw)
        if not 0 < count <= (SOUND_DATA - SOUND_OFFSETS) // 4:
            raise ValueError(f"sound effect {index + 1} has an invalid entry count")
        channels = []
        for offset in struct.unpack_from(f"<{count}i", raw, SOUND_OFFSETS):
            entry = SOUND_DATA + offset
            if not SOUND_DATA <= entry <= len(raw) - 4:
                raise ValueError(f"sound effect {index + 1} has an entry outside its buffer")
            first, second = struct.unpack_from("<2H", raw, entry)
            sequences = [None if first == NO_SEQUENCE else f"0x{entry + 4 + first:X}",
                         None if second == NO_SEQUENCE else f"0x{entry + 4 + second:X}"]
            channels.append({"offset": f"0x{entry:X}", "sequences": sequences})
        file = f"sounds/sound_{index + 1:02d}.akao"
        entry = {"sound_id": index + 1, "address": hex_address(address), "file": file, "size": f"0x{len(raw):X}",
                 "header": raw[4:SOUND_OFFSETS].hex(" "), "entry_count": count, "bank_key": bank,
                 "entries": channels}
        if pointers.index(address) != index:
            entry["same_as"] = pointers.index(address) + 1
        else:
            start = blob.offset(address)
            parts.append(Part(f"sound effect {index + 1}", start, start + len(raw), file, raw))
        entries.append(entry)
    document = Document({
        "address": hex_address(table), "count": SOUND_COUNT, "entries": entries,
        "note": "Each .akao file is one AKAO buffer passed to akao_play_sfx_from_buffer. Offsets are from the "
                "buffer start; each entry names the two channel sequences the sound driver starts (null: none).",
    })
    start = blob.offset(table)
    parts.append(Part("sound effect table", start, start + len(raw_table), "sounds/sounds.yaml", document,
                      "Sound id N plays the buffer in entry N - 1."))
    return parts


def decode_script(words: tuple[int, ...], names: dict[int, str]) -> tuple[list[dict[str, object]], int]:
    """Walk one input script as wmap_step_input_script does; returns the steps and the words used."""
    steps, cursor = [], 0
    while cursor < len(words):
        duration = words[cursor]
        if names.get(duration) == "WMAP_SCRIPT_END":
            steps.append({"code": names[duration]})
            return steps, cursor + 1
        if cursor + 1 >= len(words):
            break
        if names.get(duration) == "WMAP_SCRIPT_COMMAND":
            command = names.get(words[cursor + 1], words[cursor + 1])
            step = {"code": names[duration], "command": command}
            cursor += 2
            if command in SCRIPT_ARGUMENT_COMMANDS:
                if cursor >= len(words):
                    break
                argument = words[cursor]
                step["argument"] = f"0x{argument & 0xFFFF:04X}" if command == "WMAP_SCRIPT_BUTTON_MASK" else argument
                cursor += 1
        elif duration > 0:
            step = {"hold_frames": duration, "buttons": f"0x{words[cursor + 1] & 0xFFFF:04X}"}
            cursor += 2
        else:
            step = {"wait_buttons": f"0x{words[cursor + 1] & 0xFFFF:04X}"}
            cursor += 2
        steps.append(step)
    raise ValueError("input script has no end code")


def read_input_scripts(blob: Blob[WmapSymbols]) -> list[Part]:
    table = blob.symbols.input_scripts
    raw_table = read_bytes(blob, table, INPUT_SCRIPT_COUNT * 4, "input script table")
    names = script_names()
    scripts, parts = [], []
    for index, address in enumerate(struct.unpack(f"<{INPUT_SCRIPT_COUNT}I", raw_table)):
        limit = next_symbol(blob, address)
        raw = read_bytes(blob, address, (limit - address) & ~1, f"input script {index + 1}")
        steps, used = decode_script(struct.unpack(f"<{len(raw) // 2}h", raw), names)
        scripts.append({"script": index + 1, "address": hex_address(address), "steps": steps,
                        "bytes": raw[: used * 2].hex(" ")})
        start = blob.offset(address)
        parts.append(Part(f"input script {index + 1}", start, start + used * 2, "scripts/input_scripts.yaml"))
    document = Document({
        "address": hex_address(table), "count": INPUT_SCRIPT_COUNT, "scripts": scripts,
        "note": "Scripted controller input, one s16 word at a time: a positive duration holds the buttons that "
                "follow, zero waits for a button press (0x800 set: only the masked buttons), -2 runs a command.",
    })
    start = blob.offset(table)
    parts.append(Part("input script table", start, start + len(raw_table), "scripts/input_scripts.yaml", document))
    return parts


def read_handlers(blob: Blob[WmapSymbols], declared: dict[str, str]) -> list[Part]:
    """Every step table the C declares, entry by entry, grouped into one file per source file."""
    targets = {
        address: sorted(names, key=lambda name: (name.startswith(("func_", "D_")), name))[0]
        for address, names in symbol_aliases(blob.symbols.named).items()
    }
    groups: dict[str, list[HandlerTable]] = {}
    parts = []
    for symbol, source in declared.items():
        address = blob.symbols.named.get(symbol)
        if address is None or not blob.address <= address < blob.symbols.variables:
            continue
        raw = read_bytes(blob, address, (next_symbol(blob, address) - address) & ~3, symbol)
        steps: list[str | dict[str, str] | None] = []
        for index, (value,) in enumerate(struct.iter_unpack("<I", raw)):
            if value and value not in targets:
                inside = [(name, value - start) for name, start in blob.symbols.named.items()
                          if start < value < start + blob.symbols.sizes.get(name, 0)]
                if not inside:
                    raise ValueError(f"{symbol}[{index}] (0x{value:08X}) is not a known symbol")
                targets[value] = f"{inside[0][0]}+0x{inside[0][1]:X}"
            if not value:
                steps.append(None)
            elif value < blob.address:
                steps.append(targets[value])
            else:
                steps.append({"data": targets[value]})  # points into WMAP's data, not at code
        groups.setdefault(source, []).append(HandlerTable(symbol, source, address, tuple(steps)))
        start = blob.offset(address)
        parts.append(Part(symbol, start, start + len(raw), f"handlers/{Path(source).name}.yaml"))
    for source, tables in groups.items():
        tables.sort(key=lambda table: table.address)
        document = Document({
            "source": f"src/overlays/wmap/{source}.c",
            "note": "Step tables: each entry is the function a sequence runs for that step; null is an empty "
                    "entry and {data: name} a pointer into WMAP's data.",
            "tables": [{"symbol": table.symbol, "address": hex_address(table.address),
                        "count": len(table.steps), "steps": list(table.steps)} for table in tables],
        })
        first = next(part for part in parts if part.name == tables[0].symbol)
        parts[parts.index(first)] = Part(first.name, first.start, first.end, first.file, document)
    return parts


def read_variables(blob: Blob[WmapSymbols]) -> Part:
    start = blob.offset(blob.symbols.variables)
    if any(blob.data[start:]):
        return Part("runtime state", start, len(blob.data), "unknown/variables.bin", blob.data[start:])
    return Part("runtime state", start, len(blob.data),
                note="WMAP's runtime variables and buffers; all zero on disc, not exported.")


def read_blob(blob: Blob[WmapSymbols], declared: dict[str, str] | None = None) -> list[Part]:
    """Collect the known resources in memory order and preserve all remaining bytes."""
    parts = read_tables(blob)
    parts += read_sounds(blob)
    parts += read_input_scripts(blob)
    parts += read_handlers(blob, handler_declarations() if declared is None else declared)
    parts.append(read_variables(blob))
    return cover_gaps(blob, sorted(parts, key=lambda part: part.start))


# ---------------------------------------------------------------------------
# Writing


@write_part.register
def _write_table(content: Table, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.values, "bytes": content.raw.hex(" ")})


@write_part.register
def _write_document(content: Document, path: Path) -> None:
    dump_yaml(path, content.values)


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> Blob[WmapSymbols]:
    names = WmapSymbols.load(inputs.config / SYMBOL_FILE)
    files = splat_config.data_files(inputs.config / OVERLAY_CONFIG, inputs.assets)
    source = splat_config.file_containing(files, names.first, SYMBOL_NAMES["first"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")
    data = source.path.read_bytes()
    if len(data) != source.end - source.start:
        raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
    if source.start != names.first:
        raise ValueError(f"{SYMBOL_NAMES['first']} does not start {source.path}")
    previous = source.start - 1
    for key in SYMBOL_NAMES:
        address = getattr(names, key)
        if not source.contains(address):
            raise ValueError(f"{SYMBOL_NAMES[key]} is outside {source.path}")
        if address <= previous:
            raise ValueError(f"{SYMBOL_NAMES[key]} is out of resource order")
        previous = address
    for spec in TABLES:
        if not source.start <= names.named[spec.symbol] < names.variables:
            raise ValueError(f"{spec.symbol} is outside the initialized part of {source.path}")
    return Blob(data, source.start, source.path.name, inputs.version, names)


def extract(inputs: Inputs, output: Path, declared: dict[str, str] | None = None) -> None:
    """Read everything, write a temporary folder, then move the finished export into place."""
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    blob = load_blob(inputs)
    parts = read_blob(blob, declared)
    byte_map = {
        "source": blob.file_name, "address": hex_address(blob.address),
        "size": f"0x{len(blob.data):X}", "ranges": [byte_map_entry(blob, part) for part in parts],
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        for part in parts:
            if part.content is not None:
                write_part(part.content, staging / part.file)
        dump_yaml(staging / "byte-map.yaml", byte_map)
        staging.rename(output)
    except BaseException:
        shutil.rmtree(staging, ignore_errors=True)
        raise


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("output", type=Path, help="new folder to write")
    parser.add_argument("--version", choices=("us", "jp"), default="us")
    parser.add_argument("--config", type=Path, help="config folder (default config/<version>)")
    parser.add_argument("--assets", type=Path, help="splat assets (default assets/<version>)")
    args = parser.parse_args()
    inputs = Inputs(args.version, args.config or REPO_ROOT / "config" / args.version,
                    args.assets or REPO_ROOT / "assets" / args.version)
    try:
        extract(inputs, args.output)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
