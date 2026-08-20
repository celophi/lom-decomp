"""Splat segment for little-endian u32 index-boundary assets."""

import importlib.util
import sys
from pathlib import Path

from splat.segtypes.common.databin import CommonSegDatabin
from splat.util import log, options


_tool_path = Path(__file__).resolve().parent.parent / "assets" / "index_boundaries.py"
_tool_spec = importlib.util.spec_from_file_location("lom_index_boundaries", _tool_path)
assert _tool_spec is not None
_tool_module = importlib.util.module_from_spec(_tool_spec)
assert _tool_spec.loader is not None
sys.modules[_tool_spec.name] = _tool_module
_tool_spec.loader.exec_module(_tool_module)


class PSXSegIndex_boundaries(CommonSegDatabin):
    """Extract editable index-boundary YAML and link its rebuilt binary."""

    def yaml_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.index_boundaries.yaml"

    def bin_path(self) -> Path:
        return options.opts.asset_path / self.dir / f"{self.name}.index_boundaries.bin"

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
            boundaries = _tool_module.IndexBoundaries.parse_binary(
                source, self.expected_count()
            )
            yaml_text = _tool_module.dump_boundaries_yaml(boundaries)
            document = _tool_module.yaml.safe_load(yaml_text)
            rebuilt = _tool_module.IndexBoundaries.parse_document(
                document, self.expected_count()
            ).to_bytes()
            if rebuilt != source:
                raise _tool_module.IndexBoundariesError(
                    "binary/YAML/binary round trip differs from extracted bytes"
                )
        except _tool_module.IndexBoundariesError as error:
            log.error(f"index boundaries segment '{self.name}' is invalid: {error}")

        yaml_path = self.yaml_path()
        yaml_path.parent.mkdir(parents=True, exist_ok=True)
        yaml_path.write_text(yaml_text, encoding="ascii", newline="\n")

        bin_path = self.bin_path()
        bin_path.parent.mkdir(parents=True, exist_ok=True)
        bin_path.write_bytes(rebuilt)

        self.log(f"Wrote {self.name} to {yaml_path} and {bin_path}")
