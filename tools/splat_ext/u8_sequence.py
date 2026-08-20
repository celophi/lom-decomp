"""Splat segment for editable ordered u8 sequences."""

import importlib.util
import sys
from pathlib import Path

from splat.segtypes.common.databin import CommonSegDatabin
from splat.util import log, options


_tool_path = Path(__file__).resolve().parent.parent / "assets" / "u8_sequence.py"
_tool_spec = importlib.util.spec_from_file_location("lom_u8_sequence", _tool_path)
assert _tool_spec is not None
_tool_module = importlib.util.module_from_spec(_tool_spec)
assert _tool_spec.loader is not None
sys.modules[_tool_spec.name] = _tool_module
_tool_spec.loader.exec_module(_tool_module)


class PSXSegU8_sequence(CommonSegDatabin):
    """Extract editable u8-sequence YAML and link its rebuilt binary."""

    def yaml_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.u8_sequence.yaml"

    def bin_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.u8_sequence.bin"

    def expected_count(self):
        if isinstance(self.yaml, dict) and self.yaml.get("expected_count") is not None:
            return int(self.yaml["expected_count"])
        return None

    def write_bin(self, rom_bytes):
        assert isinstance(self.rom_start, int)
        assert isinstance(self.rom_end, int)
        source = bytes(rom_bytes[self.rom_start : self.rom_end])
        try:
            sequence = _tool_module.U8Sequence.parse_binary(
                source, self.expected_count()
            )
            yaml_text = _tool_module.dump_u8_sequence_yaml(sequence)
            document = _tool_module.yaml.safe_load(yaml_text)
            rebuilt = _tool_module.U8Sequence.parse_document(
                document, self.expected_count()
            ).to_bytes()
            if rebuilt != source:
                raise _tool_module.U8SequenceError(
                    "binary/YAML/binary round trip differs from extracted bytes"
                )
        except _tool_module.U8SequenceError as error:
            log.error(f"u8 sequence segment '{self.name}' is invalid: {error}")

        yaml_path = self.yaml_path()
        yaml_path.parent.mkdir(parents=True, exist_ok=True)
        yaml_path.write_text(yaml_text, encoding="ascii", newline="\n")
        bin_path = self.bin_path()
        bin_path.parent.mkdir(parents=True, exist_ok=True)
        bin_path.write_bytes(rebuilt)
        self.log(f"Wrote {self.name} to {yaml_path} and {bin_path}")
