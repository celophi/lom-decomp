"""Disassemble the actor-script section of a scene IMG.

Opcode sizes and operand names follow field_run_actor_script_command in
src/overlays/field/field_actor_script_ops.c. This is the actor movement/action
interpreter, not the separate event-script or animation-resource bytecode.
"""

from __future__ import annotations

import struct
from typing import NamedTuple

from tools.scenes.scene_format import SceneHeader
from tools.scenes.section_cli import run


class Opcode(NamedTuple):
    """A known command and its little-endian operand layout."""

    name: str
    operands: str = ""
    fields: tuple[str, ...] = ()


OPCODES = {
    0x00: Opcode("nop"),
    0x81: Opcode("step", "B", ("animation_state",)),
    0x82: Opcode("hit"),
    0x83: Opcode("action_0"),
    0x84: Opcode("action_1"),
    0x85: Opcode("action", "B", ("action_index",)),
    0x88: Opcode("walk", "BB", ("direction", "animation_state")),
    0x89: Opcode("rise", "B", ("animation_state",)),
    0x8A: Opcode("sink", "B", ("animation_state",)),
    0x8B: Opcode("walk_to_target", "B", ("animation_state",)),
    0x8C: Opcode("walk_from_target", "B", ("animation_state",)),
    0x8D: Opcode("sequence_step", "BB", ("animation", "animation_state")),
    0x8E: Opcode("knock_down"),
    0x8F: Opcode("jump", "BBB", ("heading", "animation", "wait")),
    0x90: Opcode("defeat"),
    0x97: Opcode("move_texture", "HBBBHB", ("x", "y", "width", "height", "destination_x", "destination_y")),
    0x9C: Opcode("command_9c", "BBB", ("animation", "command_parameter", "unknown_26")),
    0x9D: Opcode("command_9d", "BBB", ("animation", "command_parameter", "unknown_26")),
    0x9E: Opcode("load_technique", "H", ("resource_id",)),
    # The interpreter advances by two but never reads the second byte.
    0x9F: Opcode("play_bound_action", "B", ("unused",)),
    0xA0: Opcode("approach_target", "B", ("command_parameter",)),
    0xA1: Opcode("load_bound_animation", "H", ("resource_id",)),
    0xA2: Opcode("clear_unknown_10"),
    0xA3: Opcode("set_unknown_10"),
    0xA4: Opcode("action_end", "B", ("animation",)),
    0xA5: Opcode("set_part_flag_800000"),
    0xA6: Opcode("clear_part_flag_800000"),
    0xA7: Opcode("timed_walk", "BBBB", ("direction", "command_parameter", "timer", "animation")),
    0xA8: Opcode("wait_animation", "BB", ("animation", "animation_state")),
    0xA9: Opcode("load_action", "HB", ("resource_id", "action_index")),
    0xAA: Opcode("play_sound", "B", ("sound_id",)),
    0xAB: Opcode("play_object_sound", "B", ("sound_id",)),
    0xAC: Opcode("run_to_target", "B", ("animation_state",)),
    0xAD: Opcode("run", "BB", ("direction", "animation_state")),
    0xB0: Opcode("walk_path"),
    0xB1: Opcode("run_path"),
    0xB2: Opcode("turn"),
    0xB4: Opcode("toggle_hidden"),
    0xB6: Opcode("timed_slide", "BBBB", ("direction", "command_parameter", "timer", "animation")),
    0xB7: Opcode("wait", "BBB", ("animation", "animation_state", "command_parameter")),
    0xB9: Opcode("spawn_attached", "B", ("animation",)),
    0xBC: Opcode("start_bound_animation"),
    0xFF: Opcode("end"),
}


def decode_script(data: bytes, offset: int) -> dict:
    """Decode up to END or an uncertain boundary, preserving any remaining bytes."""
    instructions = []
    cursor = 0
    stop_reason = "range_end"
    while cursor < len(data):
        value = data[cursor]
        opcode = OPCODES.get(value)
        if opcode is None:
            stop_reason = "unknown_opcode"
            break
        size = 1 + struct.calcsize("<" + opcode.operands)
        if cursor + size > len(data):
            stop_reason = "truncated_operands"
            break
        values = struct.unpack_from("<" + opcode.operands, data, cursor + 1)
        instructions.append({
            "offset": f"0x{offset + cursor:X}",
            "opcode": f"0x{value:02X}",
            "command": opcode.name,
            "operands": dict(zip(opcode.fields, values)),
            "bytes": data[cursor:cursor + size].hex(" "),
        })
        cursor += size
        if value == 0xFF:
            stop_reason = "end"
            break
    return {
        "offset": f"0x{offset:X}",
        "size": len(data),
        "instructions": instructions,
        "stop_reason": stop_reason,
        "stop_offset": f"0x{offset + cursor:X}",
        "remaining_bytes": data[cursor:].hex(" "),
    }


def read_actor_scripts(data: bytes, header: SceneHeader) -> dict:
    """Read the u16 offset directory and decode each distinct script range.

    The first offset gives the directory size. An entry at section end is
    empty. Original scenes also contain entries pointing back into the table;
    these are reported as unresolved rather than interpreted as instructions.
    """
    section = data[header.actor_scripts:header.records]
    if not section:
        return {"offset": f"0x{header.actor_scripts:X}", "size": 0, "entries": [], "scripts": []}
    if len(section) < 2:
        raise ValueError("truncated actor-script directory")
    table_size = struct.unpack_from("<H", section)[0]
    if table_size < 2 or table_size % 2 or table_size > len(section):
        raise ValueError("invalid actor-script directory size")
    offsets = struct.unpack_from(f"<{table_size // 2}H", section)
    starts = sorted({offset for offset in offsets if table_size <= offset < len(section)})
    script_indices = {offset: index for index, offset in enumerate(starts)}
    entries = []
    for index, start in enumerate(offsets):
        entry = {"index": index, "relative_offset": f"0x{start:X}"}
        if start == len(section):
            entry["empty"] = True
        elif start in script_indices:
            entry["script"] = script_indices[start]
        else:
            entry["unresolved"] = "offset points into the directory or outside the section"
        entries.append(entry)
    scripts = []
    for index, (start, end) in enumerate(zip(starts, starts[1:] + [len(section)])):
        script = decode_script(section[start:end], header.actor_scripts + start)
        script = {"index": index, **script}
        if script["stop_reason"] == "range_end" and end < len(section):
            script["continues_at_script"] = script_indices[end]
        scripts.append(script)
    return {
        "offset": f"0x{header.actor_scripts:X}",
        "size": len(section),
        "directory_size": table_size,
        "entries": entries,
        "scripts": scripts,
    }


def format_disassembly(document: dict) -> str:
    """List directory references and instructions with original IMG byte offsets."""
    lines = [
        "ACTOR SCRIPTS",
        f"Section: {int(document['offset'], 16):08X}   Size: {document['size']} bytes",
        "Offsets and raw bytes are hexadecimal. Operands are decimal unless prefixed 0x.",
        "Entry and script indices are zero-based. This listing is descriptive, not assembler input.",
        "",
        "DIRECTORY",
        " Entry  Relative  Target",
    ]
    references: dict[int, list[int]] = {}
    for entry in document["entries"]:
        if "script" in entry:
            index = entry["script"]
            target = f"script {index}"
            references.setdefault(index, []).append(entry["index"])
        elif entry.get("empty"):
            target = "EMPTY (section end)"
        else:
            target = f"UNRESOLVED: {entry['unresolved']}"
        lines.append(f" {entry['index']:5d}  {int(entry['relative_offset'], 16):08X}  {target}")
    if not document["entries"]:
        lines.append(" (empty)")
    for script in document["scripts"]:
        entries = ", ".join(str(index) for index in references.get(script["index"], []))
        lines.extend(["", f"SCRIPT {script['index']}   Entries: {entries}   Range: {script['size']} bytes",
                      "Offset    Bytes                       Command / operands"])
        for instruction in script["instructions"]:
            operands = []
            for name, value in instruction["operands"].items():
                display = f"0x{value:04X}" if name == "resource_id" else str(value)
                operands.append(f"{name}={display}")
            line = (f"{int(instruction['offset'], 16):08X}  {instruction['bytes'].upper():<27} "
                    f"{instruction['command']}")
            lines.append(f"{line} {' '.join(operands)}".rstrip())
        stop = int(script["stop_offset"], 16)
        reason = script["stop_reason"]
        if "continues_at_script" in script:
            status = f"FALLTHROUGH -> SCRIPT {script['continues_at_script']} (no END in this range)"
        elif reason == "end":
            status = "END"
        else:
            status = f"STOP: {reason.upper()}"
        lines.append(f"{stop:08X}  [{status}]")
        remaining = bytes.fromhex(script["remaining_bytes"])
        if remaining:
            lines.append("Remaining bytes (not decoded):")
            for start in range(0, len(remaining), 16):
                lines.append(f"{stop + start:08X}  {remaining[start:start + 16].hex(' ').upper()}")
    return "\n".join(lines) + "\n"


def main(argv: list[str] | None = None) -> int:
    """Inspect the actor-script section of a scene IMG."""
    return run(read_actor_scripts, __doc__, argv, kind="actor_scripts", text_renderer=format_disassembly)


if __name__ == "__main__":
    raise SystemExit(main())
