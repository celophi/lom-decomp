"""Item names and drop-level boundaries from a selected region's FIELD.BIN.

Scene IMGs do not identify their region. Callers explicitly choose the reference
binary; reports retain its path, hash and decompressed byte offsets beside the
decoded data. Offsets include FIELD's leading header byte, as in the splat config.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
from pathlib import Path

import yaml

from tools.assets.name_entry_resource import DICTIONARY_TOKENS
from tools.overlays import symbols, text_table
from tools.overlays.card_data import Chart
from tools.overlays.field import Inputs, load_text_chart
from tools.splat_ext.decompress import decompress


REPO_ROOT = Path(__file__).resolve().parents[2]


def decode_name(raw: bytes, version: str, chart: Chart | None = None) -> str:
    """Expand known US dictionary tokens; preserve unrecognized codes in braces."""
    if version == "jp":
        return chart.decode(raw) if chart else text_table.decode(raw, text_table.TWO_BYTE_CODES[version])
    pieces = []
    cursor = 0
    while cursor < len(raw):
        size = 2 if raw[cursor] in text_table.TWO_BYTE_CODES["us"] else 1
        token = raw[cursor:cursor + size]
        pieces.append(DICTIONARY_TOKENS.get(token, text_table.decode(token, text_table.TWO_BYTE_CODES["us"])))
        cursor += size
    return "".join(pieces)


@dataclass(frozen=True)
class ReferenceData:
    """Decoded shared data; source records the FIELD file and offset space."""

    source: dict
    items: tuple[dict, ...]
    drop_boundaries: tuple[int, ...]
    chart: Chart | None = None

    @classmethod
    def parse(cls, data: bytes, *, source: str, version: str, names_start: int,
              names_end: int, drops_start: int, chart: Chart | None = None) -> ReferenceData:
        if version not in text_table.TWO_BYTE_CODES:
            raise ValueError(f"unsupported FIELD region: {version}")
        if not 0 <= names_start < names_end <= len(data) or not 0 <= drops_start <= len(data) - 8:
            raise ValueError("FIELD reference tables lie outside the binary")
        raw = data[names_start:names_end]
        if len(raw) < 512 or int.from_bytes(raw[:2], "little") != 512:
            raise ValueError("FIELD item names must have 256 offset entries")
        table = text_table.parse(raw, 0, text_table.TWO_BYTE_CODES[version])
        items = []
        for entry in table.entries:
            name = decode_name(entry.data, version, chart)
            items.append({"status": "resolved" if name and "{" not in name else "encoded",
                          "name": name, "name_bytes": entry.data.hex(" "),
                          "file": source, "name_offset": names_start + entry.offset,
                          "pointer_offset": names_start + entry.index * 2})
        metadata = {"file": source, "version": version, "sha256": hashlib.sha256(data).hexdigest(),
                    "item_table_offset": names_start, "drop_table_offset": drops_start}
        return cls(metadata, tuple(items), tuple(data[drops_start:drops_start + 8]), chart)

    def item(self, item_id: int) -> dict:
        # field_receive_item/get_item_count reject 0xFF, despite its table entry.
        if not 0 <= item_id < 0xFF:
            return {"status": "invalid", "reason": "Inventory operations require an ID from 0x00 to 0xFE."}
        return dict(self.items[item_id])

    def name(self, raw: bytes) -> str:
        version = self.source["version"]
        end = text_table.string_end(raw, 0, text_table.TWO_BYTE_CODES[version])
        return decode_name(raw if end < 0 else raw[:end], version, self.chart)


def resolve_item(item_id: int, reference: ReferenceData | None) -> dict:
    if reference is None:
        return {"status": "unresolved", "reason": "No FIELD reference selected; use --version and optionally --field-bin."}
    return reference.item(item_id)


def item_label(item_id: int, item: dict) -> str:
    if item.get("name"):
        suffix = " [encoded text]" if item["status"] == "encoded" else ""
        return f"{item['name']} (0x{item_id:04X}){suffix}"
    return f"0x{item_id:04X} [{item['status']} name]"


def load_reference(version: str, source: Path | None = None) -> ReferenceData:
    """Read full FIELD.BIN using the matching repository's symbol addresses."""
    config = REPO_ROOT / "config" / version
    document = yaml.safe_load((config / "overlays/FIELD.BIN.yaml").read_text())
    segment = document["segments"][0]
    named = symbols.load(config / "symbols/field_symbol_addrs.txt")
    source = source or REPO_ROOT / "disc" / version / "BIN/FIELD.BIN"
    compressed = source.read_bytes()
    try:
        data = compressed[:segment["start"]] + decompress(compressed[segment["start"]:])
    except (IndexError, ValueError) as error:
        raise ValueError(f"{source}: invalid compressed FIELD stream") from error
    if len(data) != document["segments"][1][0]:
        raise ValueError(f"{source}: size does not match the {version} FIELD layout")

    def offset(symbol: str) -> int:
        return named[symbol] - segment["vram"] + segment["start"]

    chart = load_text_chart(Inputs(version, config, REPO_ROOT / "assets" / version))
    reference = ReferenceData.parse(data, source=str(source.resolve()), version=version,
                               names_start=offset("g_field_item_name_table"),
                               names_end=offset("g_field_drop_handlers"),
                               drops_start=offset("g_field_drop_slots_by_level"), chart=chart)
    reference.source.update(sha256=hashlib.sha256(compressed).hexdigest(),
                            decompressed_sha256=hashlib.sha256(data).hexdigest(),
                            offset_space="Decompressed FIELD.BIN, including its leading header byte.")
    if chart:
        reference.source["text_decoder"] = {
            "file": str(REPO_ROOT / "assets" / version / chart.blob.file_name),
            "sha256": hashlib.sha256(chart.blob.data).hexdigest(),
            "chart_offset": chart.start, "chart_end_offset": chart.end,
        }
    for item in reference.items:
        item["offset_space"] = "decompressed_field"
    return reference


def add_reference_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--version", choices=("us", "jp"),
                        help="region of the scene; enables names and drop rates from disc/<version>/BIN/FIELD.BIN")
    parser.add_argument("--field-bin", type=Path, help="alternate FIELD.BIN with the selected region's layout")


def reference_from_arguments(args: argparse.Namespace) -> ReferenceData | None:
    if args.field_bin and not args.version:
        raise ValueError("--field-bin requires --version; scene IMGs do not identify their region")
    return load_reference(args.version, args.field_bin) if args.version else None
