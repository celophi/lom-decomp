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
    """Extract a strict TIM and reproduce its exact in-overlay representation."""

    def tim_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.tim"

    def has_trailing_duplicate_word(self) -> bool:
        value = False
        if isinstance(self.yaml, dict):
            value = self.yaml.get("trailing_duplicate_word", False)
        if type(value) is not bool:
            log.error(
                f"TIM segment '{self.name}' trailing_duplicate_word must be true or false"
            )
        return value

    def bin_path(self) -> Path:
        if self.has_trailing_duplicate_word():
            return (
                options.opts.asset_path
                / self.dir
                / f"{self.name}.tim_trail.bin"
            )
        return self.tim_path()

    def write_bin(self, rom_bytes):
        assert isinstance(self.rom_start, int)
        assert isinstance(self.rom_end, int)

        source = bytes(rom_bytes[self.rom_start : self.rom_end])
        try:
            duplicate_word = self.has_trailing_duplicate_word()
            image = _tool_module.parse_embedded_tim(source, duplicate_word)
            rebuilt = _tool_module.build_embedded_tim(image, duplicate_word)
            if rebuilt != source:
                raise _tool_module.TimFormatError(
                    "parser/builder round trip differs from the extracted bytes"
                )
        except _tool_module.TimFormatError as error:
            log.error(f"TIM segment '{self.name}' is invalid: {error}")

        tim_path = self.tim_path()
        tim_path.parent.mkdir(parents=True, exist_ok=True)
        tim_path.write_bytes(image.to_bytes())

        bin_path = self.bin_path()
        if bin_path != tim_path:
            bin_path.write_bytes(rebuilt)

        self.log(f"Wrote {self.name} to {tim_path} and {bin_path}")
