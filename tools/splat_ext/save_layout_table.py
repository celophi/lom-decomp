"""Splat segment for TITLE save-layout primitive descriptor tables."""

import importlib.util
import sys
from pathlib import Path

from splat.segtypes.common.databin import CommonSegDatabin
from splat.util import log, options


_tool_path = Path(__file__).resolve().parent.parent / "assets" / "save_layout_table.py"
_tool_spec = importlib.util.spec_from_file_location("lom_save_layout_table", _tool_path)
assert _tool_spec is not None
_tool_module = importlib.util.module_from_spec(_tool_spec)
assert _tool_spec.loader is not None
sys.modules[_tool_spec.name] = _tool_module
_tool_spec.loader.exec_module(_tool_module)


class PSXSegSave_layout_table(CommonSegDatabin):
    """Extract editable save-layout descriptors and link their rebuilt binary."""

    def yaml_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.save_layout_table.yaml"

    def bin_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.save_layout_table.bin"

    def texture_names(self) -> tuple[str, ...]:
        if not isinstance(self.yaml, dict) or not isinstance(
            self.yaml.get("textures"), list
        ):
            log.error(f"save-layout segment '{self.name}' requires a textures list")
        return tuple(self.yaml["textures"])

    def expected_count(self) -> int:
        if not isinstance(self.yaml, dict) or self.yaml.get("expected_count") is None:
            log.error(f"save-layout segment '{self.name}' requires expected_count")
        return int(self.yaml["expected_count"])

    def write_bin(self, rom_bytes):
        assert isinstance(self.rom_start, int)
        assert isinstance(self.rom_end, int)
        source = bytes(rom_bytes[self.rom_start : self.rom_end])
        textures = self.texture_names()
        expected_count = self.expected_count()
        try:
            table = _tool_module.SaveLayoutTable.parse_binary(
                source, textures, expected_count
            )
            yaml_text = _tool_module.dump_save_layout_table_yaml(table)
            document = _tool_module.yaml.safe_load(yaml_text)
            rebuilt = _tool_module.SaveLayoutTable.parse_document(
                document, expected_count, textures
            ).to_bytes()
            if rebuilt != source:
                raise _tool_module.SaveLayoutTableError(
                    "binary/YAML/binary round trip differs from extracted bytes"
                )
        except _tool_module.SaveLayoutTableError as error:
            log.error(f"save-layout segment '{self.name}' is invalid: {error}")

        yaml_path = self.yaml_path()
        yaml_path.parent.mkdir(parents=True, exist_ok=True)
        yaml_path.write_text(yaml_text, encoding="ascii", newline="\n")
        bin_path = self.bin_path()
        bin_path.parent.mkdir(parents=True, exist_ok=True)
        bin_path.write_bytes(rebuilt)
        self.log(f"Wrote {self.name} to {yaml_path} and {bin_path}")
