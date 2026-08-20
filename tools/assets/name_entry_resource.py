#!/usr/bin/env python3
"""Extract and rebuild GNAME offset-table record archives."""

from __future__ import annotations

import argparse
import string
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "gname_name_entry_resource"
FORMAT_VERSION = 2
HEADER_SIZE = 0x14
TABLE_NAMES = (
    "panel_records",
    "kanji_records",
    "history_names",
    "random_names",
)

# The US localization's field-text renderer expands these byte sequences through
# its dictionary table.  Keeping them as distinct YAML parts preserves the exact
# compressed representation while making the displayed phrase readable.
DICTIONARY_TOKENS = {
    b"\x14": " the",
    b"\x15": " you",
    b"\x16": "in",
    b"\x17": " to ",
    b"\x18": "'s ",
    b"\x1A": " I ",
    b"\x1B": " and ",
    b"\x1C": "is ",
    b"\x1D": " so",
    b"\x1E": " it",
    b"\x1F\x00": " for",
    b"\x1F\x01": "er ",
    b"\x1F\x02": "ed ",
    b"\x1F\x03": "ing ",
    b"\x1F\x04": " of",
}
DICTIONARY_ENCODINGS = {text: encoded for encoded, text in DICTIONARY_TOKENS.items()}
DICTIONARY_TEXT_ORDER = tuple(
    sorted(DICTIONARY_ENCODINGS, key=lambda text: (-len(text), text))
)
DBCS_LEAD_MIN = 0x19
DBCS_LEAD_MAX = 0x1F


class NameEntryResourceError(ValueError):
    """Raised when a GNAME record archive or YAML document is invalid."""


@dataclass(frozen=True)
class RecordTable:
    """Records addressed by a leading table of self-relative u16 offsets."""

    records: tuple[bytes, ...]

    @classmethod
    def parse_binary(cls, data: bytes, label: str) -> "RecordTable":
        if len(data) < 2:
            raise NameEntryResourceError(f"{label} is too small for an offset table")

        first_offset = struct.unpack_from("<H", data, 0)[0]
        if first_offset < 2 or first_offset % 2 != 0:
            raise NameEntryResourceError(
                f"{label} first offset 0x{first_offset:X} is not a valid "
                "u16 table size"
            )
        if first_offset > len(data):
            raise NameEntryResourceError(
                f"{label} offset table size 0x{first_offset:X} exceeds section "
                f"size 0x{len(data):X}"
            )

        record_count = first_offset // 2
        offsets = tuple(
            struct.unpack_from("<H", data, index * 2)[0]
            for index in range(record_count)
        )
        for index, offset in enumerate(offsets):
            if offset < first_offset or offset > len(data):
                raise NameEntryResourceError(
                    f"{label} offset[{index}] 0x{offset:X} is outside record "
                    "data"
                )
            if index > 0 and offset < offsets[index - 1]:
                raise NameEntryResourceError(
                    f"{label} offset[{index}] 0x{offset:X} is less than the "
                    f"preceding offset 0x{offsets[index - 1]:X}"
                )

        records = tuple(
            data[offset : offsets[index + 1] if index + 1 < record_count else len(data)]
            for index, offset in enumerate(offsets)
        )
        return cls(records)

    @classmethod
    def parse_document(cls, document: object, label: str) -> "RecordTable":
        if not isinstance(document, dict):
            raise NameEntryResourceError(f"tables.{label} must be a mapping")
        if set(document) != {"record_count", "records"}:
            raise NameEntryResourceError(
                f"tables.{label} must contain exactly record_count and records"
            )

        record_count = _validate_int(
            f"tables.{label}.record_count",
            document["record_count"],
            1,
            0x7FFF,
        )
        raw_records = document["records"]
        if not isinstance(raw_records, list):
            raise NameEntryResourceError(
                f"tables.{label}.records must be a sequence"
            )
        if len(raw_records) != record_count:
            raise NameEntryResourceError(
                f"tables.{label} contains {len(raw_records)} records, but "
                f"record_count is {record_count}"
            )

        records = tuple(
            _parse_record(value, label, f"tables.{label}.records[{index}]")
            for index, value in enumerate(raw_records)
        )
        return cls(records)

    def to_bytes(self, label: str) -> bytes:
        if not self.records:
            raise NameEntryResourceError(f"{label} must contain at least one record")

        offset = len(self.records) * 2
        if offset > 0xFFFF:
            raise NameEntryResourceError(f"{label} offset table exceeds u16 range")

        offsets = []
        for index, record in enumerate(self.records):
            if not isinstance(record, bytes):
                raise NameEntryResourceError(f"{label} record[{index}] must be bytes")
            if offset > 0xFFFF:
                raise NameEntryResourceError(
                    f"{label} record[{index}] offset exceeds u16 range"
                )
            offsets.append(offset)
            offset += len(record)

        return b"".join(struct.pack("<H", value) for value in offsets) + b"".join(
            self.records
        )

    def document(self, label: str) -> dict[str, object]:
        return {
            "record_count": len(self.records),
            "records": [_record_document(record, label) for record in self.records],
        }


@dataclass(frozen=True)
class NameEntryResource:
    """GNAME header and its four ordered self-relative record tables."""

    unknown_0x00: int
    tables: tuple[RecordTable, ...]

    @classmethod
    def parse_binary(cls, data: bytes) -> "NameEntryResource":
        if len(data) < HEADER_SIZE:
            raise NameEntryResourceError(
                f"binary size 0x{len(data):X} is smaller than header size "
                f"0x{HEADER_SIZE:X}"
            )

        unknown_0x00, *table_offsets = struct.unpack_from("<IIIII", data, 0)
        if table_offsets[0] != HEADER_SIZE:
            raise NameEntryResourceError(
                f"first table offset must be 0x{HEADER_SIZE:X}, got "
                f"0x{table_offsets[0]:X}"
            )
        for index, offset in enumerate(table_offsets):
            if offset > len(data):
                raise NameEntryResourceError(
                    f"table offset[{index}] 0x{offset:X} exceeds binary size"
                )
            if index > 0 and offset <= table_offsets[index - 1]:
                raise NameEntryResourceError(
                    f"table offset[{index}] must be greater than its predecessor"
                )

        tables = tuple(
            RecordTable.parse_binary(
                data[
                    start : table_offsets[index + 1]
                    if index + 1 < len(table_offsets)
                    else len(data)
                ],
                TABLE_NAMES[index],
            )
            for index, start in enumerate(table_offsets)
        )
        resource = cls(unknown_0x00, tables)
        if resource.to_bytes() != data:
            raise NameEntryResourceError("parsed resource does not rebuild exactly")
        return resource

    @classmethod
    def parse_document(cls, document: object) -> "NameEntryResource":
        if not isinstance(document, dict):
            raise NameEntryResourceError("YAML root must be a mapping")
        expected_keys = {"format", "version", "unknown_0x00", "tables"}
        actual_keys = set(document)
        if actual_keys != expected_keys:
            missing = sorted(expected_keys - actual_keys)
            unknown = sorted(actual_keys - expected_keys)
            details = []
            if missing:
                details.append(f"missing keys: {', '.join(missing)}")
            if unknown:
                details.append(f"unknown keys: {', '.join(unknown)}")
            raise NameEntryResourceError(
                "invalid YAML schema (" + "; ".join(details) + ")"
            )

        if document["format"] != FORMAT_NAME:
            raise NameEntryResourceError(
                f"format must be '{FORMAT_NAME}', got {document['format']!r}"
            )
        if document["version"] != FORMAT_VERSION:
            raise NameEntryResourceError(
                f"version must be {FORMAT_VERSION}, got {document['version']!r}"
            )

        unknown_0x00 = _validate_int(
            "unknown_0x00", document["unknown_0x00"], 0, 0xFFFFFFFF
        )
        raw_tables = document["tables"]
        if not isinstance(raw_tables, dict):
            raise NameEntryResourceError("tables must be a mapping")
        if set(raw_tables) != set(TABLE_NAMES):
            raise NameEntryResourceError(
                "tables must contain exactly " + ", ".join(TABLE_NAMES)
            )

        tables = tuple(
            RecordTable.parse_document(raw_tables[name], name) for name in TABLE_NAMES
        )
        return cls(unknown_0x00, tables)

    def to_bytes(self) -> bytes:
        unknown_0x00 = _validate_int(
            "unknown_0x00", self.unknown_0x00, 0, 0xFFFFFFFF
        )
        if len(self.tables) != len(TABLE_NAMES):
            raise NameEntryResourceError(
                f"resource must contain exactly {len(TABLE_NAMES)} tables"
            )

        table_data = tuple(
            table.to_bytes(TABLE_NAMES[index])
            for index, table in enumerate(self.tables)
        )
        offsets = []
        offset = HEADER_SIZE
        for data in table_data:
            offsets.append(offset)
            offset += len(data)

        header = struct.pack("<IIIII", unknown_0x00, *offsets)
        return header + b"".join(table_data)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "unknown_0x00": self.unknown_0x00,
            "tables": {
                name: self.tables[index].document(name)
                for index, name in enumerate(TABLE_NAMES)
            },
        }


def _validate_int(label: str, value: object, minimum: int, maximum: int) -> int:
    if type(value) is not int:
        raise NameEntryResourceError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise NameEntryResourceError(
            f"{label} value {value} is outside [{minimum}, {maximum}]"
        )
    return value


def _parse_hex_bytes(value: object, label: str) -> bytes:
    if not isinstance(value, str):
        raise NameEntryResourceError(f"{label} must be an uppercase hex string")
    if len(value) % 2 != 0 or any(character not in string.hexdigits for character in value):
        raise NameEntryResourceError(f"{label} must contain complete hexadecimal bytes")
    if value != value.upper():
        raise NameEntryResourceError(f"{label} must use uppercase hexadecimal")
    return bytes.fromhex(value)


def _validate_text(value: object, label: str) -> bytes:
    if not isinstance(value, str):
        raise NameEntryResourceError(f"{label} must be a string")
    try:
        encoded = value.encode("ascii")
    except UnicodeEncodeError as error:
        raise NameEntryResourceError(
            f"{label} contains a character without a verified GNAME encoding"
        ) from error
    if any(byte < 0x20 or byte > 0x7E for byte in encoded):
        raise NameEntryResourceError(
            f"{label} may contain only printable ASCII characters"
        )
    return encoded


def _parse_glyph_code(value: object, label: str) -> bytes:
    encoded = _parse_hex_bytes(value, label)
    if len(encoded) != 2 or not DBCS_LEAD_MIN <= encoded[0] <= DBCS_LEAD_MAX:
        raise NameEntryResourceError(
            f"{label} must be a four-digit encoded glyph beginning with 19-1F"
        )
    return encoded


def _encode_dictionary_text(value: object, label: str) -> bytes:
    plain = _validate_text(value, label).decode("ascii")
    encoded = bytearray()
    cursor = 0
    while cursor < len(plain):
        token_text = next(
            (text for text in DICTIONARY_TEXT_ORDER if plain.startswith(text, cursor)),
            None,
        )
        if token_text is None:
            encoded.append(ord(plain[cursor]))
            cursor += 1
        else:
            encoded.extend(DICTIONARY_ENCODINGS[token_text])
            cursor += len(token_text)
    return bytes(encoded)


def _parse_record(value: object, table_name: str, label: str) -> bytes:
    if not isinstance(value, dict):
        raise NameEntryResourceError(f"{label} must be a mapping")

    if set(value) == {"hex"}:
        return _parse_hex_bytes(value["hex"], f"{label}.hex")

    allowed_keys = {"text", "encoding", "glyph_codes", "trailing_zero_bytes"}
    unknown_keys = set(value) - allowed_keys
    if unknown_keys:
        raise NameEntryResourceError(
            f"{label} has unknown fields: {', '.join(sorted(unknown_keys))}"
        )

    content_keys = set(value) & {"text", "glyph_codes"}
    if len(content_keys) != 1:
        raise NameEntryResourceError(
            f"{label} must contain exactly one text or glyph_codes field"
        )

    content_key = next(iter(content_keys))
    if content_key == "text":
        encoding = value.get("encoding")
        if encoding is None:
            encoded = _validate_text(value["text"], f"{label}.text")
        elif encoding == "dictionary":
            if table_name != "panel_records":
                raise NameEntryResourceError(
                    f"{label}.encoding dictionary is only valid in panel_records"
                )
            encoded = _encode_dictionary_text(value["text"], f"{label}.text")
        else:
            raise NameEntryResourceError(
                f"{label}.encoding must be 'dictionary' when present"
            )
    else:
        if "encoding" in value:
            raise NameEntryResourceError(
                f"{label}.encoding is only valid with a text field"
            )
        if table_name == "panel_records":
            raise NameEntryResourceError(
                f"{label}.glyph_codes is not valid in panel_records"
            )
        glyph_codes = value["glyph_codes"]
        if not isinstance(glyph_codes, list) or not glyph_codes:
            raise NameEntryResourceError(
                f"{label}.glyph_codes must be a non-empty sequence"
            )
        encoded = b"".join(
            _parse_glyph_code(code, f"{label}.glyph_codes[{index}]")
            for index, code in enumerate(glyph_codes)
        )

    trailing_zero_bytes = _validate_int(
        f"{label}.trailing_zero_bytes",
        value.get("trailing_zero_bytes", 1),
        1,
        0xFFFF,
    )
    return encoded + bytes(trailing_zero_bytes)


def _append_text_part(parts: list[dict[str, str]], character: str) -> None:
    if parts and "text" in parts[-1]:
        parts[-1]["text"] += character
    else:
        parts.append({"text": character})


def _decode_record(record: bytes, table_name: str) -> tuple[list[dict[str, str]], int] | None:
    parts: list[dict[str, str]] = []
    cursor = 0
    while cursor < len(record):
        byte = record[cursor]
        if byte == 0:
            if any(record[cursor:]):
                return None
            return parts, len(record) - cursor

        if 0x20 <= byte <= 0x7E:
            _append_text_part(parts, chr(byte))
            cursor += 1
            continue

        if table_name == "panel_records":
            token_size = 2 if byte == 0x1F else 1
            token = record[cursor : cursor + token_size]
            if len(token) != token_size or token not in DICTIONARY_TOKENS:
                return None
            parts.append({"dictionary": DICTIONARY_TOKENS[token]})
            cursor += token_size
            continue

        if DBCS_LEAD_MIN <= byte <= DBCS_LEAD_MAX and cursor + 1 < len(record):
            parts.append({"glyph_code": record[cursor : cursor + 2].hex().upper()})
            cursor += 2
            continue

        return None

    return None


def _record_document(record: bytes, table_name: str) -> dict[str, object]:
    decoded = _decode_record(record, table_name)
    if decoded is None:
        return {"hex": record.hex().upper()}

    parts, trailing_zero_bytes = decoded
    has_dictionary_tokens = any("dictionary" in part for part in parts)
    if has_dictionary_tokens:
        document = {
            "text": "".join(next(iter(part.values())) for part in parts),
            "encoding": "dictionary",
        }
    elif len(parts) == 1 and "text" in parts[0]:
        document: dict[str, object] = {"text": parts[0]["text"]}
    elif not parts:
        document = {"text": ""}
    elif all("glyph_code" in part for part in parts):
        document = {"glyph_codes": [part["glyph_code"] for part in parts]}
    else:
        return {"hex": record.hex().upper()}

    if trailing_zero_bytes != 1:
        document["trailing_zero_bytes"] = trailing_zero_bytes
    return document


def dump_resource_yaml(resource: NameEntryResource) -> str:
    """Serialize a GNAME record archive into canonical YAML."""
    return yaml.safe_dump(
        resource.document(), sort_keys=False, allow_unicode=True, width=100
    )


def load_resource_yaml(path: Path) -> NameEntryResource:
    """Read and validate one GNAME record-archive YAML document."""
    try:
        document = yaml.safe_load(path.read_text(encoding="utf-8"))
    except UnicodeDecodeError as error:
        raise NameEntryResourceError(f"{path} must contain UTF-8 text") from error
    except yaml.YAMLError as error:
        raise NameEntryResourceError(f"invalid YAML in {path}: {error}") from error
    return NameEntryResource.parse_document(document)


def validate_roundtrip(yaml_path: Path, binary_path: Path) -> NameEntryResource:
    """Confirm that record-archive YAML rebuilds to an existing binary exactly."""
    resource = load_resource_yaml(yaml_path)
    if resource.to_bytes() != binary_path.read_bytes():
        raise NameEntryResourceError(f"{yaml_path} rebuild differs from {binary_path}")
    return resource


def _summary(path: Path, resource: NameEntryResource) -> str:
    counts = ", ".join(
        f"{name}={len(resource.tables[index].records)}"
        for index, name in enumerate(TABLE_NAMES)
    )
    return f"{path}: {counts}, 0x{len(resource.to_bytes()):X} bytes"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    extract_parser = subparsers.add_parser(
        "extract", help="extract a binary GNAME record archive to YAML"
    )
    extract_parser.add_argument("source", type=Path)
    extract_parser.add_argument("output", type=Path)

    build_parser = subparsers.add_parser(
        "build", help="build a binary GNAME record archive from YAML"
    )
    build_parser.add_argument("source", type=Path)
    build_parser.add_argument("output", type=Path)

    validate_parser = subparsers.add_parser(
        "validate", help="validate GNAME record-archive YAML files"
    )
    validate_parser.add_argument("paths", nargs="+", type=Path)

    roundtrip_parser = subparsers.add_parser(
        "roundtrip", help="compare one YAML rebuild with its binary"
    )
    roundtrip_parser.add_argument("source", type=Path)
    roundtrip_parser.add_argument("binary", type=Path)

    args = parser.parse_args()

    try:
        if args.command == "extract":
            resource = NameEntryResource.parse_binary(args.source.read_bytes())
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(
                dump_resource_yaml(resource), encoding="utf-8", newline="\n"
            )
            print(f"Wrote {args.output}")
        elif args.command == "build":
            resource = load_resource_yaml(args.source)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(resource.to_bytes())
            print(f"Wrote {args.output}")
        elif args.command == "validate":
            for path in args.paths:
                resource = load_resource_yaml(path)
                print(f"{_summary(path, resource)} [valid]")
        elif args.command == "roundtrip":
            resource = validate_roundtrip(args.source, args.binary)
            print(f"{_summary(args.source, resource)} [round-trip OK]")
    except (OSError, NameEntryResourceError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
