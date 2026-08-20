"""Splat segment for fixed-slot PSX sprite animation assets."""

import importlib.util
import sys
from pathlib import Path

from splat.segtypes.common.databin import CommonSegDatabin
from splat.util import log, options


_tool_path = Path(__file__).resolve().parent.parent / "assets" / "sprite_animation.py"
_tool_spec = importlib.util.spec_from_file_location("lom_sprite_animation", _tool_path)
assert _tool_spec is not None
_tool_module = importlib.util.module_from_spec(_tool_spec)
assert _tool_spec.loader is not None
sys.modules[_tool_spec.name] = _tool_module
_tool_spec.loader.exec_module(_tool_module)


class PSXSegSprite_animation(CommonSegDatabin):
    """Extract editable sprite-animation YAML and link its rebuilt binary."""

    def yaml_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.sprite_animation.yaml"

    def bin_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.sprite_animation.bin"

    def _required_option(self, name: str) -> int:
        if not isinstance(self.yaml, dict) or name not in self.yaml:
            log.error(f"sprite animation segment '{self.name}' requires {name}")
        return int(self.yaml[name])

    def write_bin(self, rom_bytes):
        assert isinstance(self.rom_start, int)
        assert isinstance(self.rom_end, int)

        source = bytes(rom_bytes[self.rom_start : self.rom_end])
        try:
            animation = _tool_module.SpriteAnimation.parse_binary(
                source,
                self._required_option("frame_count"),
                self._required_option("sprites_per_frame"),
                self._required_option("trailing_padding"),
            )
            yaml_text = _tool_module.dump_animation_yaml(animation)
            document = _tool_module.yaml.safe_load(yaml_text)
            rebuilt = _tool_module.SpriteAnimation.parse_document(document).to_bytes()
            if rebuilt != source:
                raise _tool_module.SpriteAnimationError(
                    "binary/YAML/binary round trip differs from extracted bytes"
                )
        except _tool_module.SpriteAnimationError as error:
            log.error(f"sprite animation segment '{self.name}' is invalid: {error}")

        yaml_path = self.yaml_path()
        yaml_path.parent.mkdir(parents=True, exist_ok=True)
        yaml_path.write_text(yaml_text, encoding="ascii", newline="\n")

        bin_path = self.bin_path()
        bin_path.parent.mkdir(parents=True, exist_ok=True)
        bin_path.write_bytes(rebuilt)

        self.log(f"Wrote {self.name} to {yaml_path} and {bin_path}")
