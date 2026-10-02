"""YAML presentation for scene inspection; parsers retain their stored values."""

from __future__ import annotations

from dataclasses import dataclass

import yaml


# Explicit field names keep counts, coordinates and stat values in decimal.
HEX_WIDTHS = {
    "info": 8, "params": 8, "status": 8, "word": 8, "frame_offset": 8, "frame_table_offset": 8,
    "pointer_base": 8, "script_pointer_base": 8, "pointer_word": 4, "mode": 2, "element_mask": 2,
    "name_offset": 8, "pointer_offset": 8, "item_table_offset": 8, "drop_table_offset": 8,
    "reward_name_offset": 8, "item_id_offset": 8, "collection_setting_offset": 8, "defeat_flags": 8,
    "initializer_offset": 8, "selector_offset": 8, "variable_ref": 4, "collection_variable_ref": 4, "raw": 4,
    "target_offset": 8, "instruction_offset": 8, "table_offset": 8, "reference": 4,
    "offset": 8, "header_offset": 8, "section_offset": 8, "stop_offset": 8, "relative_offset": 4, "relative_offsets": 4,
    "templates_offset": 8, "rewards_offset": 8, "opcode": 2,
    "flags": 2, "bound_animation_flags": 4, "map_word": 4,
    "header_word": 8, "reward_marker": 4, "words": 8,
    "raise_mask": 2, "lower_mask": 2, "immunity_flags": 2,
    "weak_elements": 2, "resist_elements": 2, "unknown_flag_bits": 2,
    "target_filter": 2, "resource_id": 4, "item_id": 2,
}
EMPTY_FIELDS = {"operands", "remaining_bytes", "trailing_bytes", "directory_trailing_bytes"}
NOTES = {
    "scene_report": (
        "Object links describe stored layout settings. Save conditions and script effects are not simulated.",
        "possible_operations are selected commands reachable from enabled slots, not an execution order.",
        "Monster templates match the initial selector by ID. Later script changes are not evaluated.",
        "Name offsets refer to decompressed FIELD including its header byte. Chest/drop setting offsets refer to the IMG.",
        "Drop chances are conditional on level and defeat flags; the current game state is not inferred.",
    ),
    "event_scripts": (
        "Directory slots may hold data rather than usable code offsets; unresolved slots retain their values.",
        "Instructions follow static branches/calls. Variables and runtime conditions are not evaluated.",
        "encoding=variable reads a value; encoding=reference names a variable. See the variables list.",
        "A zero branch delta returns. Indexed-jump table lengths are not inferred.",
        "undecoded spans preserve all remaining bytes. diagnostics identify conflicting or unsupported targets.",
    ),
    "actors": (
        "relative_offsets are relative to this section. Actor resource_index = index + 3.",
        "image_slots: 0xFF ends the list; later slots are stored but not used.",
        "Action animation 0xFFFF is the no-animation sentinel; parameters depend on command.",
        "Repeated actor offsets share the same stored description.",
    ),
    "actor_scripts": (
        "entries are directory slots; scripts are distinct ranges. Indices are zero-based.",
        "Multiple entries may share a script. See data.txt for a compact disassembly in full exports.",
        "continues_at_script marks fallthrough. unresolved entries have not been decoded.",
        "stop_offset is the first byte after decoded instructions. Nonempty remaining_bytes are retained.",
    ),
    "strings": (
        "Text entries use section-relative u16 offsets; aliases share string_index, interior pointers stay separate.",
        "Text expands known US compression tokens. Braces show controls or unmapped glyph bytes.",
        "Macro values and glyphs from external font charts are not resolved. End (0) and finish (6) terminate text.",
    ),
    "geometry": (
        "Consecutive u32 boundaries delimit one animation resource per actor description.",
        "Animation pointer bit 15 selects four-byte frame entries; other entries have two bytes.",
        "Frame record flags bit 5 selects placement commands. Their operands remain explicit bytes.",
        "Sprites decode local coordinates, UVs, dimensions, palette/page selectors, mirrors and tilt.",
    ),
    "resource": (
        "Resource IDs select local stored layouts. Diagnostic payloads and unknown IDs remain binary.",
        "Text offsets are relative to the text table after the four-byte resource header.",
        "Item scripts and placement operands are preserved; extraction does not execute them.",
    ),
    "battle": (
        "Monster stats are stored base/growth pairs, not calculated battle stats.",
        "Stat arrays retain their original order. hp_growth 0xFFFF uses the stat 4 curve.",
        "Drop slots are stored choices, not equal probabilities. reward_indices refer to rewards below.",
        "Action descriptors use info/params words; parameter_fields follow the handler-specific union.",
        "Reward item fields use FieldItemRecord. Encoded names and original bytes remain authoritative.",
    ),
}


@dataclass(frozen=True)
class HexValue:
    """A YAML integer with a minimum hexadecimal display width."""

    value: int
    width: int


class FlowList(list):
    """A short numeric list that fits on one line."""


class FlowMapping(dict):
    """A small stat pair displayed on one line."""


class ByteString(str):
    """Raw bytes displayed in rows of sixteen, still readable by bytes.fromhex."""


class SceneDumper(yaml.SafeDumper):
    """Indent sequence entries to match the hand-written layout exports."""

    def increase_indent(self, flow: bool = False, indentless: bool = False):
        return super().increase_indent(flow, False)


def represent_hex(dumper: SceneDumper, value: HexValue):
    """Emit a real YAML integer, not a quoted hexadecimal string."""
    # Out-of-range relative branches can produce negative offsets in diagnostic exports.
    sign = "-" if value.value < 0 else ""
    text = f"{sign}0x{abs(value.value):0{value.width}X}"
    return dumper.represent_scalar("tag:yaml.org,2002:int", text)


SceneDumper.add_representer(HexValue, represent_hex)
SceneDumper.add_representer(FlowList, lambda dumper, value: dumper.represent_sequence(
    "tag:yaml.org,2002:seq", value, flow_style=True))
SceneDumper.add_representer(FlowMapping, lambda dumper, value: dumper.represent_mapping(
    "tag:yaml.org,2002:map", value, flow_style=True))
SceneDumper.add_representer(ByteString, lambda dumper, value: dumper.represent_scalar(
    "tag:yaml.org,2002:str", value, style="|"))


def format_value(value, field: str = ""):
    """Prepare a display copy without dropping zeroes, false flags or empty references."""
    if isinstance(value, dict):
        return {key: format_value(item, key) for key, item in value.items()
                if not (key in EMPTY_FIELDS and item in ("", {}))}
    if isinstance(value, list):
        items = [format_value(item, field) for item in value]
        if field in ("stats", "equipment_stats"):
            return [FlowMapping(item) for item in items]
        if len(items) <= 8 and all(isinstance(item, (int, HexValue)) for item in items):
            return FlowList(items)
        return items
    if value is None or isinstance(value, bool):
        return value
    if field == "command" and isinstance(value, int):
        return HexValue(value, 4)
    if field in HEX_WIDTHS:
        width = HEX_WIDTHS[field]
        if isinstance(value, str):
            if not value.startswith("0x"):
                return value
            width = max(width, len(value) - 2)
            return HexValue(int(value, 16), width)
        return HexValue(value, width)
    if (field == "image_slots" and value == 0xFF) or (
        field in ("animation", "parameter", "hp_growth") and value == 0xFFFF
    ):
        return HexValue(value, 2 if field == "image_slots" else 4)
    if field.endswith("_bytes") and isinstance(value, str):
        octets = value.split()
        if len(octets) > 16:
            return ByteString("\n".join(" ".join(octets[start:start + 16])
                                        for start in range(0, len(octets), 16)))
    return value


def dump_yaml(document: dict, kind: str = "") -> str:
    """Render descriptive YAML with numeric hex values and a short reading guide."""
    notes = [
        "Decoded format reference. Stored fields coexist with interpretations such as split flags and ASCII previews.",
        "Reconstruction uses companion binaries (layout YAML uses record_bytes).",
        "Offsets are absolute byte positions in the original IMG unless named relative_offset(s).",
        "Hex values are integers. Counts, coordinates and ordinary stat values are decimal.",
    ]
    if kind == "resource" and "battle" in document:
        kind = "battle"
    notes.extend(NOTES.get(kind, ()))
    heading = "".join(f"# {note}\n" for note in notes)
    return heading + "\n" + yaml.dump(format_value(document), Dumper=SceneDumper,
                                      sort_keys=False, allow_unicode=False, width=100)
