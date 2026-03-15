import importlib.util
from pathlib import Path
from typing import Optional

from splat.segtypes.common.code import CommonSegCode
from splat.util import log, options

# Load decompress.py from the same directory as this extension
_ext_dir = Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("decompress", _ext_dir / "decompress.py")
_mod = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_mod)
_decompress = _mod.decompress


class PSXSegDecompress_overlay(CommonSegCode):
    def _get_decompressed_bytes(self, rom_bytes):
        """Decompress the segment data and return a new bytes object
        where the decompressed data is placed at self.rom_start,
        so subsegment rom_start offsets work correctly."""
        assert isinstance(self.rom_start, int)
        assert isinstance(self.rom_end, int)

        compressed = rom_bytes[self.rom_start : self.rom_end]
        decompressed = _decompress(compressed)

        # Build a byte array with decompressed data at the right offset
        # so children can index with their rom_start values
        result = bytearray(self.rom_start + len(decompressed))
        result[self.rom_start :] = decompressed
        return bytes(result)

    def scan(self, rom_bytes):
        decompressed_rom = self._get_decompressed_bytes(rom_bytes)
        super().scan(decompressed_rom)

    def split(self, rom_bytes):
        decompressed_rom = self._get_decompressed_bytes(rom_bytes)
        super().split(decompressed_rom)
