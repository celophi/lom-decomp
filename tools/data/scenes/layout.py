"""Stored scene layout records and their chest-specific interpretation."""

from __future__ import annotations

from dataclasses import dataclass
import struct

from tools.data.scenes.scene_format import SceneHeader


LAYOUT_RECORD = struct.Struct("<IHBBIHH16H")

# The common chest initializer sets local state, then reads layout parameters
# scripts[4] and scripts[5]. Other actors using the chest graphics can differ.
COMMON_CHEST_INITIALIZER = bytes.fromhex(
    "40 84 20 c0 "
    "0c 01 02 ff 10 00 08 40 40 e0 "
    "0c 01 02 ff 10 00 09 40 50 e0"
)


@dataclass(frozen=True)
class FieldLayoutRecord:
    """One 48-byte FieldLayoutRecord, all multi-byte values little-endian.

    The layout section starts with a u32 count, followed by these records.
    See include/overlays/field/field_interaction_start.h. scripts[15] selects the initializer;
    for the common chest script, scripts[4] is the item and scripts[5] holds the
    collection-variable reference, with bit 15 selecting the alternate facing.
    """

    control: int                 # 0x00: u32; bits 4-7 select the action kind
    condition_variable_ref: int  # 0x04: u16 script-variable reference
    condition_minimum: int       # 0x06: u8
    condition_maximum: int       # 0x07: u8
    position: int                # 0x08: u32; X in bits 0-15, Z in bits 16-26
    source: int                  # 0x0C: u16; low three bits select actor resources
    enabled_events: int          # 0x0E: u16 event mask
    scripts: tuple[int, ...]     # 0x10: sixteen u16 script references or parameters

    @classmethod
    def parse(cls, data: bytes, offset: int) -> FieldLayoutRecord:
        """Read one record at an absolute IMG offset, after validating its section."""
        control, variable_ref, minimum, maximum, position, source, events, *scripts = (
            LAYOUT_RECORD.unpack_from(data, offset)
        )
        return cls(control, variable_ref, minimum, maximum, position, source, events, tuple(scripts))

    def is_common_chest(self, event_scripts: bytes) -> bool:
        """Require both the chest resource selector and the known initializer."""
        kind = (self.control >> 4) & 0xF
        # Actor and grouped-actor records use resource selector 5 for chest graphics.
        if kind not in (0, 7) or (self.source & 7) != 5:
            return False
        initializer = self.scripts[15]
        if initializer == 0xFFFF or (initializer & 0x8000) == 0:
            return False
        table_offset = (initializer & 0x7FFF) * 2
        if table_offset + 2 > len(event_scripts):
            return False
        script_offset = struct.unpack_from("<H", event_scripts, table_offset)[0]
        # Script offsets are section-relative. Entry zero is not a script count.
        if script_offset < table_offset + 2:
            return False
        return event_scripts[script_offset:].startswith(COMMON_CHEST_INITIALIZER)

    def chest_yaml(self, record_bytes: bytes) -> str:
        """Keep the existing chest export, including its decoded parameters."""
        return self.layout_yaml(record_bytes, chest=True)

    def layout_yaml(self, record_bytes: bytes, chest: bool = False) -> str:
        """Show every layout field, plus decoded chest settings and packed bits.

        Script slots contain both references and parameters. Their comments
        describe the common chest template, not a rule for other actor kinds.
        The original record remains the source for byte-preserving recovery.
        """
        slot_roles = {
            0: "interaction script",
            1: "interaction script",
            4: "item ID",
            5: "collection-variable reference and alternate-facing bit",
            8: "idle script",
            15: "initialization script",
        }
        lines = [
            f"x: {self.position & 0xFFFF}",
            f"z: {(self.position >> 16) & 0x7FF}",
            f"item_id: 0x{self.scripts[4]:04X}",
            "# Reference to a collection variable, not its current saved value.",
            f"collection_variable_ref: 0x{self.scripts[5] & 0x7FFF:04X}",
            f"alternate_facing: {'true' if self.scripts[5] & 0x8000 else 'false'}",
        ] if chest else []
        lines.extend([
            "# The current variable value must be within this inclusive range.",
            "condition:",
            f"  variable_ref: 0x{self.condition_variable_ref:04X}",
            f"  minimum: {self.condition_minimum}",
            f"  maximum: {self.condition_maximum}",
            "# Fields of the original FieldLayoutRecord, before installation.",
            "control:",
            f"  raw: 0x{self.control:08X}",
            f"  trigger_group: {self.control & 0xF}",
            f"  kind: {(self.control >> 4) & 0xF}",
            f"  selector: {(self.control >> 8) & 0xFF}",
            f"  local_variable_count: {(self.control >> 16) & 0xFF}",
            f"  palette: {(self.control >> 24) & 0xF}",
            f"  group: {(self.control >> 28) & 3}",
            f"  hidden: {'true' if self.control & 0x40000000 else 'false'}",
            f"  active: {'true' if self.control & 0x80000000 else 'false'}",
            "position:",
            f"  raw: 0x{self.position:08X}",
            f"  x: {self.position & 0xFFFF}",
            f"  z: {(self.position >> 16) & 0x7FF}",
            f"  unknown_bits_27_29: {(self.position >> 27) & 7}",
            f"  y: {(self.position >> 30) & 3}",
            "source:",
            f"  raw: 0x{self.source:04X}",
            f"  resource_selector: {self.source & 7}",
            f"enabled_events: 0x{self.enabled_events:04X}",
            "# Zero-based slots; 0xFFFF means no script in script-reference slots.",
            "scripts:",
        ])
        for index, value in enumerate(self.scripts):
            role = slot_roles.get(index) if chest else None
            comment = f"[{index}]" + (f" {role}" if role else "")
            lines.append(f"  - 0x{value:04X}  # {comment}")
        lines.extend([
            "# Original 48 bytes, retained for byte-preserving recovery.",
            f'record_bytes: "{record_bytes.hex()}"',
        ])
        return "\n".join(lines) + "\n"



def read_layout_records(data: bytes, header: SceneHeader) -> list[FieldLayoutRecord]:
    """Validate the counted layout section and read its records in file order."""
    size = header.event_scripts - header.layout
    if size < 4:
        raise ValueError("missing layout record count")
    count = struct.unpack_from("<I", data, header.layout)[0]
    if size != 4 + count * LAYOUT_RECORD.size:
        raise ValueError("layout count does not match the section size")
    return [FieldLayoutRecord.parse(data, header.layout + 4 + index * LAYOUT_RECORD.size)
            for index in range(count)]
