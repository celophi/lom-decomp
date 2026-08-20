"""Splat segment for partially mapped Legend of Mana game-state templates."""

import importlib.util
import sys
from pathlib import Path

from splat.segtypes.common.databin import CommonSegDatabin
from splat.util import log, options


_tool_path = Path(__file__).resolve().parent.parent / "assets" / "game_state_template.py"
_tool_spec = importlib.util.spec_from_file_location("lom_game_state_template", _tool_path)
assert _tool_spec is not None
_tool_module = importlib.util.module_from_spec(_tool_spec)
assert _tool_spec.loader is not None
sys.modules[_tool_spec.name] = _tool_module
_tool_spec.loader.exec_module(_tool_module)


class PSXSegGame_state_template(CommonSegDatabin):
    """Extract game-state payload plus editable metadata and link its rebuild."""

    def yaml_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.game_state_template.yaml"

    def payload_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.payload.bin"

    def bin_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.game_state_template.bin"

    def expected_size(self) -> int:
        if not isinstance(self.yaml, dict) or self.yaml.get("expected_size") is None:
            log.error(f"game-state segment '{self.name}' requires expected_size")
        return int(self.yaml["expected_size"])

    def write_bin(self, rom_bytes):
        assert isinstance(self.rom_start, int)
        assert isinstance(self.rom_end, int)
        source = bytes(rom_bytes[self.rom_start : self.rom_end])
        payload_path = self.payload_path()
        payload_name = payload_path.name
        expected_size = self.expected_size()
        try:
            template = _tool_module.GameStateTemplate.parse_binary(
                source, payload_name, expected_size
            )
            yaml_text = _tool_module.dump_game_state_template_yaml(template)
            document = _tool_module.yaml.safe_load(yaml_text)
            rebuilt = _tool_module.GameStateTemplate.parse_document(
                document, source, expected_size, payload_name
            ).to_bytes()
            if rebuilt != source:
                raise _tool_module.GameStateTemplateError(
                    "binary/YAML/payload/binary round trip differs from extracted bytes"
                )
        except _tool_module.GameStateTemplateError as error:
            log.error(f"game-state segment '{self.name}' is invalid: {error}")

        yaml_path = self.yaml_path()
        yaml_path.parent.mkdir(parents=True, exist_ok=True)
        yaml_path.write_text(yaml_text, encoding="ascii", newline="\n")
        payload_path.parent.mkdir(parents=True, exist_ok=True)
        payload_path.write_bytes(source)
        bin_path = self.bin_path()
        bin_path.parent.mkdir(parents=True, exist_ok=True)
        bin_path.write_bytes(rebuilt)
        self.log(f"Wrote {self.name} metadata, payload, and rebuilt binary")
