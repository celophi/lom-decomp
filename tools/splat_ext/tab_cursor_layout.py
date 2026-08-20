"""Splat segment for packed PSX tab-cursor layout assets."""

import importlib.util
import sys
from pathlib import Path

from splat.segtypes.common.databin import CommonSegDatabin
from splat.util import log, options


_tool_path = Path(__file__).resolve().parent.parent / "assets" / "tab_cursor_layout.py"
_tool_spec = importlib.util.spec_from_file_location("lom_tab_cursor_layout", _tool_path)
assert _tool_spec is not None
_tool_module = importlib.util.module_from_spec(_tool_spec)
assert _tool_spec.loader is not None
sys.modules[_tool_spec.name] = _tool_module
_tool_spec.loader.exec_module(_tool_module)


class PSXSegTab_cursor_layout(CommonSegDatabin):
    """Extract editable tab-cursor YAML and link its rebuilt binary."""

    def yaml_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.tab_cursor_layout.yaml"

    def bin_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.tab_cursor_layout.bin"

    def expected_count(self):
        if isinstance(self.yaml, dict):
            count = self.yaml.get("expected_count")
            if count is not None:
                return int(count)
        return None

    def write_bin(self, rom_bytes):
        assert isinstance(self.rom_start, int)
        assert isinstance(self.rom_end, int)

        source = bytes(rom_bytes[self.rom_start : self.rom_end])
        try:
            layout = _tool_module.TabCursorLayout.parse_binary(
                source, self.expected_count()
            )
            yaml_text = _tool_module.dump_layout_yaml(layout)
            document = _tool_module.yaml.safe_load(yaml_text)
            rebuilt = _tool_module.TabCursorLayout.parse_document(
                document, self.expected_count()
            ).to_bytes()
            if rebuilt != source:
                raise _tool_module.TabCursorLayoutError(
                    "binary/YAML/binary round trip differs from extracted bytes"
                )
        except _tool_module.TabCursorLayoutError as error:
            log.error(f"tab cursor segment '{self.name}' is invalid: {error}")

        yaml_path = self.yaml_path()
        yaml_path.parent.mkdir(parents=True, exist_ok=True)
        yaml_path.write_text(yaml_text, encoding="ascii", newline="\n")

        bin_path = self.bin_path()
        bin_path.parent.mkdir(parents=True, exist_ok=True)
        bin_path.write_bytes(rebuilt)

        self.log(f"Wrote {self.name} to {yaml_path} and {bin_path}")
