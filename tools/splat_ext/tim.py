"""Splat segment for validated, byte-exact PlayStation TIM assets."""

import importlib.util
import sys
from pathlib import Path

from splat.segtypes.common.databin import CommonSegDatabin
from splat.util import log, options


_tool_path = Path(__file__).resolve().parent.parent / "assets" / "psx_tim.py"
_tool_spec = importlib.util.spec_from_file_location("lom_psx_tim", _tool_path)
assert _tool_spec is not None
_tool_module = importlib.util.module_from_spec(_tool_spec)
assert _tool_spec.loader is not None
sys.modules[_tool_spec.name] = _tool_module
_tool_spec.loader.exec_module(_tool_module)


class PSXSegTim(CommonSegDatabin):
    """Extract a strict TIM file and link it in the original `.data` section."""

    def bin_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.tim"

    def split(self, rom_bytes):
        assert isinstance(self.rom_start, int)
        assert isinstance(self.rom_end, int)

        source = bytes(rom_bytes[self.rom_start : self.rom_end])
        try:
            image = _tool_module.parse_tim(source)
            if image.to_bytes() != source:
                raise _tool_module.TimFormatError(
                    "parser/builder round trip differs from the extracted bytes"
                )
        except _tool_module.TimFormatError as error:
            log.error(f"TIM segment '{self.name}' is invalid: {error}")

        super().split(rom_bytes)
