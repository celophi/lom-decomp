#!/usr/bin/env python3
"""Export SHOP's item text and sell prices.

The build links one unchanged data blob: a four-section text archive, the sell
price of every item kind, and the shop's zero-filled runtime variables.
Readers describe the resources as Parts, and the byte map covers the rest.
JP text uses CLOAD's character chart, as FIELD, GNAME and GOLEM do.

Example:

    python3 -m tools.overlays.shop --version us assets/exports/us/overlays/shop
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import shutil
import struct
import sys
import tempfile

from tools.overlays import cload, splat_config, symbols, text_table
from tools.overlays.card_data import Chart
from tools.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[2]
OVERLAY_CONFIG = "overlays/SHOP.BIN.yaml"
SYMBOL_FILE = "symbols/shop_symbol_addrs.txt"
HEADER = REPO_ROOT / "src/overlays/shop/shop_internal.h"
RENDER_SOURCE = REPO_ROOT / "src/overlays/shop/shop_render.c"
ITEM_KIND_COUNT = 256
MATERIAL_COUNT = 64
SPELLS_PER_SPIRIT = 14
SECTION_COUNT = 4
ARCHIVE_HEADER = struct.Struct("<HH4I")
# First name of each item category in the equipment-type section, in index order.
CATEGORY_FIRSTS = (("weapon", 0), ("armor", 11), ("instrument", 23))


# ---------------------------------------------------------------------------
# Inputs and decoded parts


@dataclass(frozen=True)
class ShopSymbols:
    """Resource boundaries in their stored order."""

    text: int
    prices: int
    variables: int

    @classmethod
    def load(cls, path: Path) -> ShopSymbols:
        named = symbols.load(path)
        missing = [name for name in SYMBOL_NAMES.values() if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in tools/overlays/shop.py."
            )
        return cls(**{key: named[name] for key, name in SYMBOL_NAMES.items()})


SYMBOL_NAMES = {
    "text": "g_shop_text_archive",
    "prices": "g_shop_item_sell_prices",
    "variables": "g_shop_title_text_id",
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
class TextArchive:
    address: int
    raw: bytes
    sections: tuple[dict[str, object], ...]

    def texts(self, name: str) -> list[str]:
        section = next(section for section in self.sections if section["name"] == name)
        return [entry["text"] for entry in section["entries"]]


def read_bytes(blob: Blob, address: int, end: int, what: str) -> bytes:
    start, stop = blob.offset(address), blob.offset(end)
    if not 0 <= start <= stop <= len(blob.data):
        raise ValueError(f"{what} is outside {blob.file_name}")
    return blob.data[start:stop]


def text_section_names() -> dict[int, str]:
    """Read the section names from their owning C enum."""
    source = HEADER.read_text(encoding="ascii")
    match = re.search(r"typedef enum\s*\{(.+?)\}\s*ShopTextSection;", source, re.DOTALL)
    if match is None:
        raise ValueError(f"{HEADER} has no ShopTextSection")
    return {int(value): name for name, value in re.findall(r"(SHOP_TEXT_\w+)\s*=\s*(\d+)", match[1])
            if name != "SHOP_TEXT_SECTION_COUNT"}


def equipment_type(index: int) -> dict[str, object]:
    """The item category and FIELD_ITEM_TYPE value the renderer looks up at @p index."""
    category, first = next((name, first) for name, first in reversed(CATEGORY_FIRSTS) if index >= first)
    return {"category": category, "type": index - first}


# ---------------------------------------------------------------------------
# Reading the blob


def section_entry(section: str, index: int) -> dict[str, object]:
    """What the C code indexes each section by."""
    if section == "item_names":
        return {"item": index, "material": index < MATERIAL_COUNT}
    if section == "item_descriptions":
        return {"item": index}
    if section == "equipment_types":
        return equipment_type(index)
    if section == "instrument_spells":
        return {"spirit": index // SPELLS_PER_SPIRIT, "spell": index % SPELLS_PER_SPIRIT}
    return {}


def read_text(blob: Blob[ShopSymbols], chart: Chart | None) -> Part:
    """The archive header holds a section count and four archive-relative offsets."""
    address = blob.symbols.text
    raw = read_bytes(blob, address, blob.symbols.prices, "text archive")
    if len(raw) < ARCHIVE_HEADER.size:
        raise ValueError("text archive header is truncated")
    count, unused, *offsets = ARCHIVE_HEADER.unpack_from(raw)
    if count != SECTION_COUNT or unused != 0:
        raise ValueError(f"text archive must hold {SECTION_COUNT} sections")
    if offsets[0] != ARCHIVE_HEADER.size or offsets != sorted(set(offsets)) or offsets[-1] >= len(raw):
        raise ValueError("invalid text archive section offsets")
    if blob.version == "jp" and chart is None:
        raise ValueError("JP SHOP text needs CLOAD's character chart")
    names = text_section_names()
    if sorted(names) != list(range(SECTION_COUNT)):
        raise ValueError(f"{HEADER} must name all {SECTION_COUNT} text sections")
    sections = []
    for index, (start, end) in enumerate(zip(offsets, (*offsets[1:], len(raw)))):
        symbol = names[index]
        name = symbol.removeprefix("SHOP_TEXT_").lower()
        table = text_table.parse(raw[start:end], 0, text_table.TWO_BYTE_CODES[blob.version])
        entries = [{
            "index": entry.index, **section_entry(name, entry.index), "offset": f"0x{entry.offset:X}",
            "text": chart.decode(entry.data) if chart else entry.text,
            "bytes": entry.data.hex(" "),
        } for entry in table.entries]
        section = {"index": index, "symbol": symbol, "name": name, "offset": f"0x{start:X}",
                   "address": hex_address(address + start), "entries": entries}
        tail = raw[start + table.size:end]
        if any(tail):
            section["trailing_bytes"] = tail.hex(" ")
        elif tail:
            section["padding"] = len(tail)
        sections.append(section)
    counts = {section["name"]: len(section["entries"]) for section in sections}
    for name in ("item_descriptions", "item_names"):
        if counts.get(name) != ITEM_KIND_COUNT:
            raise ValueError(f"{name} must have {ITEM_KIND_COUNT} entries, one per item kind")
    if counts.get("instrument_spells", 0) % SPELLS_PER_SPIRIT:
        raise ValueError(f"instrument_spells must hold complete rows of {SPELLS_PER_SPIRIT}")
    start = blob.offset(address)
    return Part("text archive", start, start + len(raw), "text/archive.yaml", TextArchive(address, raw, tuple(sections)))


def read_prices(blob: Blob[ShopSymbols], text: TextArchive) -> Part:
    """One u16 price per item kind, read by shop_build_sell_list for plain items."""
    address = blob.symbols.prices
    size = ITEM_KIND_COUNT * 2
    if blob.symbols.variables - address < size:
        raise ValueError(f"sell price table must hold {ITEM_KIND_COUNT} entries")
    raw = read_bytes(blob, address, address + size, "sell price table")
    names = text.texts("item_names")
    entries = [{"item": index, "name": names[index], "price": price}
               for index, price in enumerate(struct.unpack(f"<{ITEM_KIND_COUNT}H", raw))]
    values = {"entry_format": "<H", "entries": entries,
              "note": "The unit price the sell list offers for each item kind the player holds. "
                      "Equipment records are sold for their own stored value instead."}
    start = blob.offset(address)
    return Part("sell prices", start, start + size, "tables/sell_prices.yaml", Table(address, values, raw))


def read_variables(blob: Blob[ShopSymbols]) -> Part:
    start = blob.offset(blob.symbols.variables)
    if any(blob.data[start:]):
        return Part("shop state", start, len(blob.data), "unknown/variables.bin", blob.data[start:])
    return Part("shop state", start, len(blob.data), note="Runtime variables and buffers; all zero on disc, not exported.")


def read_blob(blob: Blob[ShopSymbols], chart: Chart | None = None) -> list[Part]:
    text = read_text(blob, chart)
    parts = [text, read_prices(blob, text.content), read_variables(blob)]
    return cover_gaps(blob, parts)


# ---------------------------------------------------------------------------
# Writing


@write_part.register
def _write_table(content: Table, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.values, "bytes": content.raw.hex(" ")})


@write_part.register
def _write_text(content: TextArchive, path: Path) -> None:
    sections = [{key: section[key] for key in ("index", "symbol", "offset")} | {"file": f"{section['name']}.yaml"}
                for section in content.sections]
    dump_yaml(path, {"address": hex_address(content.address), "file": "archive.bin",
                     "section_count": len(sections), "sections": sections})
    (path.parent / "archive.bin").write_bytes(content.raw)
    for section in content.sections:
        dump_yaml(path.parent / f"{section['name']}.yaml", section)


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> Blob[ShopSymbols]:
    names = ShopSymbols.load(inputs.config / SYMBOL_FILE)
    files = splat_config.data_files(inputs.config / OVERLAY_CONFIG, inputs.assets)
    source = splat_config.file_containing(files, names.text, SYMBOL_NAMES["text"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")
    data = source.path.read_bytes()
    if len(data) != source.end - source.start:
        raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
    previous = source.start - 1
    for key in SYMBOL_NAMES:
        address = getattr(names, key)
        if not source.contains(address):
            raise ValueError(f"{SYMBOL_NAMES[key]} is outside {source.path}")
        if address <= previous:
            raise ValueError(f"{SYMBOL_NAMES[key]} is out of resource order")
        previous = address
    return Blob(data, source.start, source.path.name, inputs.version, names)


def load_text_chart(inputs: Inputs) -> Chart | None:
    if inputs.version != "jp":
        return None
    return Chart(cload.load_blob(cload.Inputs(inputs.version, inputs.config, inputs.assets)))


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything, write a temporary folder, then move the finished export into place."""
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    blob = load_blob(inputs)
    chart = load_text_chart(inputs)
    parts = read_blob(blob, chart)
    byte_map = {
        "source": blob.file_name, "address": hex_address(blob.address),
        "size": f"0x{len(blob.data):X}", "ranges": [byte_map_entry(blob, part) for part in parts],
    }
    if chart:
        byte_map["text_decoder"] = {
            "source": chart.blob.file_name, "overlay": "CLOAD",
            "address": hex_address(chart.blob.address + chart.start), "size": f"0x{chart.end - chart.start:X}",
            "note": "JP text uses CLOAD's character chart. SHOP does not contain this conversion table.",
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
