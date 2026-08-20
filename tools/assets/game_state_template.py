#!/usr/bin/env python3
"""Extract and rebuild partially mapped Legend of Mana game-state templates."""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path, PurePath

import yaml


FORMAT_NAME = "lom_game_state_template"
FORMAT_VERSION = 1
WEAPON_CATEGORY_COUNT = 11


class _HexInt(int):
    pass


class _GameStateDumper(yaml.SafeDumper):
    pass


_GameStateDumper.add_representer(
    _HexInt,
    lambda dumper, value: dumper.represent_scalar(
        "tag:yaml.org,2002:int", f"0x{int(value):08X}"
    ),
)


class GameStateTemplateError(ValueError):
    """Raised when a game-state template or its YAML metadata is invalid."""


@dataclass(frozen=True)
class GameStateTemplate:
    payload: str
    data: bytes
    field_config: int
    option_id: int
    sub_mode: int
    music_track: int
    scene_mode: int
    field_flags: int
    layout_flags: int
    state_flags: int
    weapon_category_masks: tuple[int, ...]
    rng_seed: int
    mode_flags: int
    slot_flags: int

    @classmethod
    def parse_binary(
        cls, data: bytes, payload: str, expected_size: int
    ) -> "GameStateTemplate":
        _validate_payload_name(payload)
        _validate_size(data, expected_size)
        return cls(
            payload,
            data,
            struct.unpack_from("<I", data, 0x18)[0],
            struct.unpack_from("<h", data, 0x1C)[0],
            struct.unpack_from("<b", data, 0x1E)[0],
            struct.unpack_from("<I", data, 0x20)[0],
            struct.unpack_from("<H", data, 0x24)[0],
            data[0x26],
            data[0x27],
            struct.unpack_from("<I", data, 0x28)[0],
            struct.unpack_from(f"<{WEAPON_CATEGORY_COUNT}I", data, 0x34),
            struct.unpack_from("<h", data, 0xD4)[0],
            struct.unpack_from("<I", data, 0x2E0)[0],
            struct.unpack_from("<I", data, 0x608)[0],
        )

    @classmethod
    def parse_document(
        cls,
        document: object,
        payload_data: bytes,
        expected_size: int | None = None,
        expected_payload: str | None = None,
    ) -> "GameStateTemplate":
        if not isinstance(document, dict):
            raise GameStateTemplateError("YAML root must be a mapping")
        expected_keys = {
            "format",
            "version",
            "template_size",
            "payload",
            "known_fields",
        }
        if set(document) != expected_keys:
            raise GameStateTemplateError(
                "YAML must contain format, version, template_size, payload, and known_fields"
            )
        if document["format"] != FORMAT_NAME:
            raise GameStateTemplateError(f"format must be '{FORMAT_NAME}'")
        if document["version"] != FORMAT_VERSION:
            raise GameStateTemplateError(f"version must be {FORMAT_VERSION}")
        template_size = _validate_u32("template_size", document["template_size"])
        if expected_size is not None and template_size != expected_size:
            raise GameStateTemplateError(
                f"template_size is 0x{template_size:X}; expected 0x{expected_size:X}"
            )
        _validate_size(payload_data, template_size)
        payload = _validate_payload_name(document["payload"])
        if expected_payload is not None and payload != expected_payload:
            raise GameStateTemplateError(
                "YAML payload differs from the Splat configuration"
            )
        fields = document["known_fields"]
        field_keys = {
            "field_config",
            "option_id",
            "sub_mode",
            "music_track",
            "scene_mode",
            "field_flags",
            "layout_flags",
            "state_flags",
            "weapon_category_masks",
            "rng_seed",
            "mode_flags",
            "slot_flags",
        }
        if not isinstance(fields, dict) or set(fields) != field_keys:
            raise GameStateTemplateError("known_fields has an invalid schema")
        masks = fields["weapon_category_masks"]
        if not isinstance(masks, list) or len(masks) != WEAPON_CATEGORY_COUNT:
            raise GameStateTemplateError(
                f"weapon_category_masks must contain {WEAPON_CATEGORY_COUNT} values"
            )
        return cls(
            payload,
            payload_data,
            _validate_u32("known_fields.field_config", fields["field_config"]),
            _validate_s16("known_fields.option_id", fields["option_id"]),
            _validate_s8("known_fields.sub_mode", fields["sub_mode"]),
            _validate_u32("known_fields.music_track", fields["music_track"]),
            _validate_u16("known_fields.scene_mode", fields["scene_mode"]),
            _validate_u8("known_fields.field_flags", fields["field_flags"]),
            _validate_u8("known_fields.layout_flags", fields["layout_flags"]),
            _validate_u32("known_fields.state_flags", fields["state_flags"]),
            tuple(
                _validate_u32(f"known_fields.weapon_category_masks[{index}]", value)
                for index, value in enumerate(masks)
            ),
            _validate_s16("known_fields.rng_seed", fields["rng_seed"]),
            _validate_u32("known_fields.mode_flags", fields["mode_flags"]),
            _validate_u32("known_fields.slot_flags", fields["slot_flags"]),
        )

    def to_bytes(self) -> bytes:
        output = bytearray(self.data)
        if len(self.weapon_category_masks) != WEAPON_CATEGORY_COUNT:
            raise GameStateTemplateError(
                f"weapon_category_masks must contain {WEAPON_CATEGORY_COUNT} values"
            )
        struct.pack_into("<I", output, 0x18, _validate_u32("field_config", self.field_config))
        struct.pack_into("<h", output, 0x1C, _validate_s16("option_id", self.option_id))
        struct.pack_into("<b", output, 0x1E, _validate_s8("sub_mode", self.sub_mode))
        struct.pack_into("<I", output, 0x20, _validate_u32("music_track", self.music_track))
        struct.pack_into("<H", output, 0x24, _validate_u16("scene_mode", self.scene_mode))
        output[0x26] = _validate_u8("field_flags", self.field_flags)
        output[0x27] = _validate_u8("layout_flags", self.layout_flags)
        struct.pack_into("<I", output, 0x28, _validate_u32("state_flags", self.state_flags))
        struct.pack_into(
            f"<{WEAPON_CATEGORY_COUNT}I",
            output,
            0x34,
            *(
                _validate_u32(f"weapon_category_masks[{index}]", value)
                for index, value in enumerate(self.weapon_category_masks)
            ),
        )
        struct.pack_into("<h", output, 0xD4, _validate_s16("rng_seed", self.rng_seed))
        struct.pack_into("<I", output, 0x2E0, _validate_u32("mode_flags", self.mode_flags))
        struct.pack_into("<I", output, 0x608, _validate_u32("slot_flags", self.slot_flags))
        return bytes(output)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "template_size": len(self.data),
            "payload": self.payload,
            "known_fields": {
                "field_config": self.field_config,
                "option_id": self.option_id,
                "sub_mode": self.sub_mode,
                "music_track": self.music_track,
                "scene_mode": self.scene_mode,
                "field_flags": self.field_flags,
                "layout_flags": self.layout_flags,
                "state_flags": self.state_flags,
                "weapon_category_masks": list(self.weapon_category_masks),
                "rng_seed": self.rng_seed,
                "mode_flags": self.mode_flags,
                "slot_flags": self.slot_flags,
            },
        }


def _validate_payload_name(value: object) -> str:
    if not isinstance(value, str) or not value:
        raise GameStateTemplateError("payload must be a nonempty relative filename")
    path = PurePath(value)
    if path.is_absolute() or ".." in path.parts or len(path.parts) != 1:
        raise GameStateTemplateError("payload must be a filename beside the YAML file")
    return value


def _validate_size(data: bytes, expected_size: int) -> None:
    if expected_size < 0x60C:
        raise GameStateTemplateError("template is too small for the mapped fields")
    if len(data) != expected_size:
        raise GameStateTemplateError(
            f"payload size is 0x{len(data):X}; expected 0x{expected_size:X}"
        )


def _validate_int(label: str, value: object, minimum: int, maximum: int) -> int:
    if type(value) is not int or not minimum <= value <= maximum:
        raise GameStateTemplateError(
            f"{label} must be an integer in [{minimum}, {maximum}]"
        )
    return value


def _validate_s8(label: str, value: object) -> int:
    return _validate_int(label, value, -0x80, 0x7F)


def _validate_u8(label: str, value: object) -> int:
    return _validate_int(label, value, 0, 0xFF)


def _validate_s16(label: str, value: object) -> int:
    return _validate_int(label, value, -0x8000, 0x7FFF)


def _validate_u16(label: str, value: object) -> int:
    return _validate_int(label, value, 0, 0xFFFF)


def _validate_u32(label: str, value: object) -> int:
    return _validate_int(label, value, 0, 0xFFFFFFFF)


def dump_game_state_template_yaml(template: GameStateTemplate) -> str:
    document = template.document()
    document["template_size"] = _HexInt(document["template_size"])
    fields = document["known_fields"]
    for name in (
        "field_config",
        "music_track",
        "state_flags",
        "mode_flags",
        "slot_flags",
    ):
        fields[name] = _HexInt(fields[name])
    fields["weapon_category_masks"] = [
        _HexInt(value) for value in fields["weapon_category_masks"]
    ]
    return yaml.dump(
        document,
        Dumper=_GameStateDumper,
        sort_keys=False,
        allow_unicode=False,
        width=100,
    )


def load_game_state_template_yaml(path: Path) -> GameStateTemplate:
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise GameStateTemplateError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise GameStateTemplateError(f"invalid YAML in {path}: {error}") from error
    if not isinstance(document, dict):
        raise GameStateTemplateError("YAML root must be a mapping")
    payload = _validate_payload_name(document.get("payload"))
    payload_path = path.parent / payload
    return GameStateTemplate.parse_document(document, payload_path.read_bytes())


def validate_roundtrip(yaml_path: Path, binary_path: Path) -> GameStateTemplate:
    template = load_game_state_template_yaml(yaml_path)
    if template.to_bytes() != binary_path.read_bytes():
        raise GameStateTemplateError(f"{yaml_path} rebuild differs from {binary_path}")
    return template


def _summary(path: Path, template: GameStateTemplate) -> str:
    return f"{path}: 0x{len(template.to_bytes()):X} bytes, 12 mapped field groups"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    build_parser = subparsers.add_parser("build")
    build_parser.add_argument("source", type=Path)
    build_parser.add_argument("output", type=Path)
    validate_parser = subparsers.add_parser("validate")
    validate_parser.add_argument("paths", nargs="+", type=Path)
    roundtrip_parser = subparsers.add_parser("roundtrip")
    roundtrip_parser.add_argument("source", type=Path)
    roundtrip_parser.add_argument("binary", type=Path)
    args = parser.parse_args()

    try:
        if args.command == "build":
            template = load_game_state_template_yaml(args.source)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(template.to_bytes())
        elif args.command == "validate":
            for path in args.paths:
                template = load_game_state_template_yaml(path)
                print(f"{_summary(path, template)} [valid]")
        elif args.command == "roundtrip":
            template = validate_roundtrip(args.source, args.binary)
            print(f"{_summary(args.source, template)} [round-trip OK]")
    except (OSError, GameStateTemplateError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
