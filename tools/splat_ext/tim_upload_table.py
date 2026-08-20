"""Splat segment for PSX TIM upload descriptor tables."""

import importlib.util
import sys
from pathlib import Path

from splat.segtypes.common.databin import CommonSegDatabin
from splat.util import log, options


_tool_path = Path(__file__).resolve().parent.parent / "assets" / "tim_upload_table.py"
_tool_spec = importlib.util.spec_from_file_location("lom_tim_upload_table", _tool_path)
assert _tool_spec is not None
_tool_module = importlib.util.module_from_spec(_tool_spec)
assert _tool_spec.loader is not None
sys.modules[_tool_spec.name] = _tool_module
_tool_spec.loader.exec_module(_tool_module)


class PSXSegTim_upload_table(CommonSegDatabin):
    """Extract editable TIM upload descriptors and link their rebuilt binary."""

    def yaml_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.tim_upload_table.yaml"

    def bin_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.tim_upload_table.bin"

    def target_names(self) -> tuple[str, ...]:
        if not isinstance(self.yaml, dict) or not isinstance(self.yaml.get("targets"), list):
            log.error(f"TIM upload table segment '{self.name}' requires a targets list")
        return tuple(self.yaml["targets"])

    def write_bin(self, rom_bytes):
        assert isinstance(self.rom_start, int)
        assert isinstance(self.rom_end, int)
        source = bytes(rom_bytes[self.rom_start : self.rom_end])
        try:
            table = _tool_module.TimUploadTable.parse_binary(
                source, self.target_names()
            )
            yaml_text = _tool_module.dump_tim_upload_table_yaml(table)
            document = _tool_module.yaml.safe_load(yaml_text)
            rebuilt = _tool_module.TimUploadTable.parse_document(document).to_bytes()
            if rebuilt != source:
                raise _tool_module.TimUploadTableError(
                    "binary/YAML/binary round trip differs from extracted bytes"
                )
        except _tool_module.TimUploadTableError as error:
            log.error(f"TIM upload table segment '{self.name}' is invalid: {error}")

        yaml_path = self.yaml_path()
        yaml_path.parent.mkdir(parents=True, exist_ok=True)
        yaml_path.write_text(yaml_text, encoding="ascii", newline="\n")
        bin_path = self.bin_path()
        bin_path.parent.mkdir(parents=True, exist_ok=True)
        bin_path.write_bytes(rebuilt)
        self.log(f"Wrote {self.name} to {yaml_path} and {bin_path}")
