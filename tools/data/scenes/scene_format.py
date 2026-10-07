"""The shared ten-section scene IMG header."""

from __future__ import annotations

from dataclasses import dataclass
import struct
from typing import NamedTuple


SCENE_HEADER = struct.Struct("<10I")


@dataclass(frozen=True)
class SceneSection:
    """A named section's absolute IMG byte range; end is exclusive."""

    name: str
    start: int
    end: int


class SceneHeader(NamedTuple):
    """The 40-byte disk header: ten little-endian u32 offsets from IMG byte zero.

    Each section ends where the next begins. Equal offsets mean an empty section.
    These fields follow FIELD_SCENE_* in field_scene_transition.c.
    """

    layout: int          # 0x00: count and 48-byte actor/action records
    event_scripts: int   # 0x04: u16 offsets followed by event bytecode
    strings: int         # 0x08: scene text
    actor_scripts: int   # 0x0C: actor animation scripts
    records: int         # 0x10: general records
    actors: int          # 0x14: actor descriptions
    geometry: int        # 0x18: scene geometry
    images: int          # 0x1C: u32 offset table followed by TIM files
    portraits: int       # 0x20: count followed by palette-and-pixel records
    group_bounds: int    # 0x24: final section, extending to end of file

    @classmethod
    def parse(cls, data: bytes) -> SceneHeader:
        """Read the header and check that its section boundaries fit the IMG."""
        if len(data) < SCENE_HEADER.size:
            raise ValueError("truncated scene header")
        header = cls(*SCENE_HEADER.unpack_from(data))
        if header.layout != SCENE_HEADER.size:
            raise ValueError("expected scene layout offset 0x28")
        if any(offset % 4 or offset > len(data) for offset in header):
            raise ValueError("section offsets must be aligned and within the file")
        if tuple(header) != tuple(sorted(header)):
            raise ValueError("section offsets must be in file order")
        return header

    def sections(self, file_size: int) -> list[SceneSection]:
        """Pair each section start with its following boundary."""
        return [
            SceneSection("layout", self.layout, self.event_scripts),
            SceneSection("event_scripts", self.event_scripts, self.strings),
            SceneSection("strings", self.strings, self.actor_scripts),
            SceneSection("actor_scripts", self.actor_scripts, self.records),
            SceneSection("records", self.records, self.actors),
            SceneSection("actors", self.actors, self.geometry),
            SceneSection("geometry", self.geometry, self.images),
            SceneSection("images", self.images, self.portraits),
            SceneSection("portraits", self.portraits, self.group_bounds),
            SceneSection("group_bounds", self.group_bounds, file_size),
        ]
