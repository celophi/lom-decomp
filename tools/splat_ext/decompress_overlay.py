from pathlib import Path
from typing import Optional

from splat.segtypes.common.segment import CommonSegment
from splat.util import log, options

from decompress import decompress


class PSXSegDecompress_overlay(CommonSegment):
    @staticmethod
    def is_data() -> bool:
        return True

    def out_path(self) -> Optional[Path]:
        return options.opts.asset_path / self.dir / f"{self.name}.bin"

    def split(self, rom_bytes):
        path = self.out_path()
        assert path is not None
        path.parent.mkdir(parents=True, exist_ok=True)

        if self.rom_end is None:
            log.error(
                f"segment {self.name} needs to know where it ends; "
                "add a position marker [0xDEADBEEF] after it"
            )

        assert isinstance(self.rom_start, int)
        assert isinstance(self.rom_end, int)

        compressed = rom_bytes[self.rom_start : self.rom_end]
        decompressed = decompress(compressed)

        with open(path, "wb") as f:
            f.write(decompressed)

        self.log(f"Wrote decompressed {self.name} ({len(compressed)} -> {len(decompressed)} bytes) to {path}")
