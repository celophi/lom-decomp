#!/usr/bin/env python3
"""Extract and rebuild fixed-slot PSX sprite animation assets."""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "psx_sprite_animation"
FORMAT_VERSION = 1
SPRITE_SIZE = 4


class SpriteAnimationError(ValueError):
    """Raised when a sprite animation binary or YAML document is invalid."""


@dataclass(frozen=True)
class AnimationSprite:
    """One glyph selection, position, and raw per-slot control byte."""

    x: int
    y: int
    glyph_id: int
    control: int

    def document(self) -> dict[str, int]:
        return {
            "x": self.x,
            "y": self.y,
            "glyph_id": self.glyph_id,
            "control": self.control,
        }


@dataclass(frozen=True)
class AnimationFrame:
    """One fixed-size group of sprite slots."""

    sprites: tuple[AnimationSprite, ...]

    def document(self) -> dict[str, object]:
        return {"sprites": [sprite.document() for sprite in self.sprites]}


@dataclass(frozen=True)
class SpriteAnimation:
    """Ordered animation frames followed by zero alignment padding."""

    sprites_per_frame: int
    trailing_padding: int
    frames: tuple[AnimationFrame, ...]

    @classmethod
    def parse_binary(
        cls,
        data: bytes,
        frame_count: int,
        sprites_per_frame: int,
        trailing_padding: int,
    ) -> "SpriteAnimation":
        frame_count = _validate_int("frame_count", frame_count, 1, 0xFFFFFFFF)
        sprites_per_frame = _validate_int(
            "sprites_per_frame", sprites_per_frame, 1, 0xFFFFFFFF
        )
        trailing_padding = _validate_int(
            "trailing_padding", trailing_padding, 0, 0xFFFFFFFF
        )

        frame_size = sprites_per_frame * SPRITE_SIZE
        expected_size = frame_count * frame_size + trailing_padding
        if len(data) != expected_size:
            raise SpriteAnimationError(
                f"binary size is 0x{len(data):X}; expected 0x{expected_size:X}"
            )

        if data[frame_count * frame_size :] != bytes(trailing_padding):
            raise SpriteAnimationError("trailing padding must contain only zero bytes")

        frames = []
        for frame_index in range(frame_count):
            frame_offset = frame_index * frame_size
            sprites = []
            for sprite_index in range(sprites_per_frame):
                offset = frame_offset + sprite_index * SPRITE_SIZE
                x, y, glyph_id, control = struct.unpack_from("<BBBB", data, offset)
                sprites.append(AnimationSprite(x, y, glyph_id, control))
            frames.append(AnimationFrame(tuple(sprites)))

        return cls(sprites_per_frame, trailing_padding, tuple(frames))

    @classmethod
    def parse_document(cls, document: object) -> "SpriteAnimation":
        if not isinstance(document, dict):
            raise SpriteAnimationError("YAML root must be a mapping")

        expected_keys = {
            "format",
            "version",
            "frame_count",
            "sprites_per_frame",
            "trailing_padding",
            "frames",
        }
        actual_keys = set(document)
        if actual_keys != expected_keys:
            missing = sorted(expected_keys - actual_keys)
            unknown = sorted(actual_keys - expected_keys)
            details = []
            if missing:
                details.append(f"missing keys: {', '.join(missing)}")
            if unknown:
                details.append(f"unknown keys: {', '.join(unknown)}")
            raise SpriteAnimationError(
                "invalid YAML schema (" + "; ".join(details) + ")"
            )

        if document["format"] != FORMAT_NAME:
            raise SpriteAnimationError(
                f"format must be '{FORMAT_NAME}', got {document['format']!r}"
            )
        if document["version"] != FORMAT_VERSION:
            raise SpriteAnimationError(
                f"version must be {FORMAT_VERSION}, got {document['version']!r}"
            )

        frame_count = _validate_int(
            "frame_count", document["frame_count"], 1, 0xFFFFFFFF
        )
        sprites_per_frame = _validate_int(
            "sprites_per_frame", document["sprites_per_frame"], 1, 0xFFFFFFFF
        )
        trailing_padding = _validate_int(
            "trailing_padding", document["trailing_padding"], 0, 0xFFFFFFFF
        )

        raw_frames = document["frames"]
        if not isinstance(raw_frames, list):
            raise SpriteAnimationError("frames must be a sequence")
        if len(raw_frames) != frame_count:
            raise SpriteAnimationError(
                f"YAML contains {len(raw_frames)} frames, but frame_count is "
                f"{frame_count}"
            )

        frames = []
        for frame_index, raw_frame in enumerate(raw_frames):
            if not isinstance(raw_frame, dict):
                raise SpriteAnimationError(f"frames[{frame_index}] must be a mapping")
            if set(raw_frame) != {"sprites"}:
                raise SpriteAnimationError(
                    f"frames[{frame_index}] must contain exactly sprites"
                )

            raw_sprites = raw_frame["sprites"]
            if not isinstance(raw_sprites, list):
                raise SpriteAnimationError(
                    f"frames[{frame_index}].sprites must be a sequence"
                )
            if len(raw_sprites) != sprites_per_frame:
                raise SpriteAnimationError(
                    f"frames[{frame_index}] contains {len(raw_sprites)} sprites; "
                    f"expected {sprites_per_frame}"
                )

            sprites = []
            for sprite_index, raw_sprite in enumerate(raw_sprites):
                label = f"frames[{frame_index}].sprites[{sprite_index}]"
                if not isinstance(raw_sprite, dict):
                    raise SpriteAnimationError(f"{label} must be a mapping")
                if set(raw_sprite) != {"x", "y", "glyph_id", "control"}:
                    raise SpriteAnimationError(
                        f"{label} must contain exactly x, y, glyph_id, and control"
                    )
                sprites.append(
                    AnimationSprite(
                        _validate_int(f"{label}.x", raw_sprite["x"], 0, 0xFF),
                        _validate_int(f"{label}.y", raw_sprite["y"], 0, 0xFF),
                        _validate_int(
                            f"{label}.glyph_id", raw_sprite["glyph_id"], 0, 0xFF
                        ),
                        _validate_int(
                            f"{label}.control", raw_sprite["control"], 0, 0xFF
                        ),
                    )
                )
            frames.append(AnimationFrame(tuple(sprites)))

        return cls(sprites_per_frame, trailing_padding, tuple(frames))

    def to_bytes(self) -> bytes:
        output = bytearray()
        for frame in self.frames:
            if len(frame.sprites) != self.sprites_per_frame:
                raise SpriteAnimationError(
                    f"frame contains {len(frame.sprites)} sprites; expected "
                    f"{self.sprites_per_frame}"
                )
            for sprite in frame.sprites:
                x = _validate_int("x", sprite.x, 0, 0xFF)
                y = _validate_int("y", sprite.y, 0, 0xFF)
                glyph_id = _validate_int("glyph_id", sprite.glyph_id, 0, 0xFF)
                control = _validate_int("control", sprite.control, 0, 0xFF)
                output.extend(struct.pack("<BBBB", x, y, glyph_id, control))
        output.extend(bytes(self.trailing_padding))
        return bytes(output)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "frame_count": len(self.frames),
            "sprites_per_frame": self.sprites_per_frame,
            "trailing_padding": self.trailing_padding,
            "frames": [frame.document() for frame in self.frames],
        }


def _validate_int(label: str, value: object, minimum: int, maximum: int) -> int:
    if type(value) is not int:
        raise SpriteAnimationError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise SpriteAnimationError(
            f"{label} value {value} is outside [{minimum}, {maximum}]"
        )
    return value


def dump_animation_yaml(animation: SpriteAnimation) -> str:
    """Serialize an animation into its canonical YAML representation."""
    return yaml.safe_dump(
        animation.document(), sort_keys=False, allow_unicode=False, width=100
    )


def load_animation_yaml(path: Path) -> SpriteAnimation:
    """Read and validate one sprite animation YAML document."""
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise SpriteAnimationError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise SpriteAnimationError(f"invalid YAML in {path}: {error}") from error
    return SpriteAnimation.parse_document(document)


def validate_roundtrip(yaml_path: Path, binary_path: Path) -> SpriteAnimation:
    """Confirm that an animation YAML rebuilds to an existing binary exactly."""
    animation = load_animation_yaml(yaml_path)
    if animation.to_bytes() != binary_path.read_bytes():
        raise SpriteAnimationError(f"{yaml_path} rebuild differs from {binary_path}")
    return animation


def _summary(path: Path, animation: SpriteAnimation) -> str:
    return (
        f"{path}: {len(animation.frames)} frames, "
        f"{animation.sprites_per_frame} sprites/frame, "
        f"0x{len(animation.to_bytes()):X} bytes"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    extract_parser = subparsers.add_parser(
        "extract", help="extract a binary animation to YAML"
    )
    extract_parser.add_argument("source", type=Path)
    extract_parser.add_argument("output", type=Path)
    extract_parser.add_argument("--frame-count", type=int, required=True)
    extract_parser.add_argument("--sprites-per-frame", type=int, required=True)
    extract_parser.add_argument("--trailing-padding", type=int, default=0)

    build_parser = subparsers.add_parser(
        "build", help="build a binary animation from YAML"
    )
    build_parser.add_argument("source", type=Path)
    build_parser.add_argument("output", type=Path)

    validate_parser = subparsers.add_parser(
        "validate", help="validate sprite animation YAML files"
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
            animation = SpriteAnimation.parse_binary(
                args.source.read_bytes(),
                args.frame_count,
                args.sprites_per_frame,
                args.trailing_padding,
            )
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(
                dump_animation_yaml(animation), encoding="ascii", newline="\n"
            )
            print(f"Wrote {args.output}")
        elif args.command == "build":
            animation = load_animation_yaml(args.source)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(animation.to_bytes())
            print(f"Wrote {args.output}")
        elif args.command == "validate":
            for path in args.paths:
                animation = load_animation_yaml(path)
                print(f"{_summary(path, animation)} [valid]")
        elif args.command == "roundtrip":
            animation = validate_roundtrip(args.source, args.binary)
            print(f"{_summary(args.source, animation)} [round-trip OK]")
    except (OSError, SpriteAnimationError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
