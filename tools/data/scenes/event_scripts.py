"""Disassemble scene event scripts using the FIELD event interpreter's formats.

Directory entries and static branches seed a bounded control-flow walk. Bytes
that cannot be reached or decoded remain raw. Runtime variables are described,
not evaluated; this is an inspection tool, not an event-script interpreter.
"""

from __future__ import annotations

from dataclasses import dataclass
import heapq
import struct

from tools.data.scenes.event_opcodes import CALCULATIONS, EXTENDED, FIELD_COMMANDS, FIXED, MISC_COMMANDS, OWNER_FIELDS, PAIR, TYPED
from tools.data.scenes.scene_format import SceneHeader
from tools.data.scenes.section_cli import run


class DecodeError(ValueError):
    """A boundary beyond which the reader cannot safely decode an instruction."""


@dataclass
class OperandReader:
    """A checked cursor in one event section, including both operand encodings."""

    data: bytes
    position: int

    def read(self, format: str) -> int:
        size = struct.calcsize("<" + format)
        if self.position + size > len(self.data):
            raise DecodeError("truncated_operands")
        value = struct.unpack_from("<" + format, self.data, self.position)[0]
        self.position += size
        return value

    def reference(self) -> dict:
        return {"encoding": "reference", "reference": self.read("H")}

    def basic(self, kind: int, owner: bool = False) -> dict:
        kind &= 3
        if kind == 0:
            result = {"encoding": "variable", "reference": self.read("H")}
            if owner:
                result["owner_when_255"] = True
            return result
        format, encoding = (("B", "u8"), ("H", "u16"), ("i", "s32"))[kind - 1]
        result = {"encoding": encoding, "value": self.read(format)}
        if owner and result["value"] == 0xFF:
            result["meaning"] = "owner"
        return result

    def nibble(self, kind: int) -> dict:
        if kind in (0, 1, 2, 4):
            format, encoding = {0: ("B", "u8"), 1: ("H", "u16"),
                                2: ("i", "s32"), 4: ("H", "u16_alt")}[kind]
            return {"encoding": encoding, "value": self.read(format)}
        if kind == 3:
            return {"encoding": "variable", "reference": self.read("H")}
        if kind == 5:
            return {"encoding": "u16_high", "value": self.read("H") + 0x10000}
        if kind in (6, 7):
            return {"encoding": f"unchanged_{kind}", "unresolved": "operand has no assigned value"}
        if kind in (8, 9, 10):
            return {"encoding": ("zero", "one", "owner_constant")[kind - 8], "value": (0, 1, 255)[kind - 8]}
        raise DecodeError(f"unknown_operand_type_0x{kind:X}")


def branch_target(base: int, delta: int, kind: str) -> dict:
    """A zero branch delta returns; other deltas are relative to the opcode/case."""
    if delta == 0:
        return {"kind": "return", "condition": kind}
    return {"kind": kind, "target_offset": base + delta}


def decode_instruction(data: bytes, position: int, base: int = 0) -> dict:
    """Decode one instruction and its possible successors, without running it."""
    reader = OperandReader(data, position)
    opcode = reader.read("B")
    instruction = {"offset": base + position, "opcode": opcode}
    operands = {}
    edges = []
    falls_through = True
    if opcode in FIXED:
        spec = FIXED[opcode]
        name = spec.name
        for field, format in zip(spec.fields, spec.format):
            if field.endswith("_ref"):
                operands[field] = reader.reference()
            else:
                operands[field] = {"encoding": {"B": "u8", "H": "u16", "h": "s16"}[format],
                                   "value": reader.read(format)}
        if opcode == 0x00:
            edges.append({"kind": "return"})
            falls_through = False
        elif opcode in (0x01, 0x02, 0x04, 0x05):
            kind = {1: "jump", 2: "call", 4: "branch_true", 5: "branch_false"}[opcode]
            edges.append(branch_target(base + position, operands["delta"]["value"], kind))
            falls_through = opcode != 1
        elif opcode == 0x10:
            edges.append({"kind": "call_event", "unresolved": "entry = value(script_ref) & 0x7FFF"})
        elif opcode == 0x2E:
            edges.append({"kind": "stalled", "unresolved": "handler does not advance the program counter"})
            falls_through = False
        elif opcode == 0x03:
            command = operands["command_id"]["value"]
            instruction["subcommand_name"] = FIELD_COMMANDS[command] if command < len(FIELD_COMMANDS) else "unhandled"
            instruction["note"] = "Command arguments come from the runtime parameter block."
        for field in ("actor", "speaker"):
            if field in operands and operands[field].get("value") == 255:
                operands[field]["meaning"] = "owner"
        if opcode == 0x13:
            instruction["note"] = "actor_ref is read with owner ID 0."
    elif opcode in PAIR or opcode in EXTENDED:
        name, fields = (PAIR if opcode in PAIR else EXTENDED)[opcode]
        descriptors = [reader.read("B") for _ in range(len(fields) // 2)]
        instruction["descriptor_bytes"] = bytes(descriptors).hex(" ")
        types = [kind for descriptor in descriptors for kind in (descriptor & 15, descriptor >> 4)]
        operands = {field: reader.nibble(kind) for field, kind in zip(fields, types)}
        for field in OWNER_FIELDS.get(opcode, ()):
            if operands[field].get("value") == 255:
                operands[field]["meaning"] = "owner"
            elif operands[field]["encoding"] == "variable":
                operands[field]["owner_when_255"] = True
        # These handlers interpret literal operands as references, not variable values.
        references = {0x40: ("destination",), 0x52: ("destination", "source"),
                      0x59: ("destination",), 0x5A: ("destination",)}.get(opcode, ())
        for field in references:
            if "value" in operands[field]:
                value = operands[field]["value"]
                operands[field]["reference"] = value & 0xFFFF
                if field == "destination" and (value & 0xFFFFFFFF) > 0xFFFF:
                    operands[field]["keep_top_bit"] = True
        if opcode == 0x44:
            command = operands["subcommand"].get("value")
            instruction["subcommand_name"] = MISC_COMMANDS.get(command, "dynamic" if command is None else "unhandled")
            if command == 0x4D:
                edges.append({"kind": "scene_change"})
                falls_through = False
        elif opcode == 0x85:
            edges.append({"kind": "scene_change"})
            falls_through = False
    elif opcode in TYPED:
        spec = TYPED[opcode]
        name = spec.name
        descriptor = reader.read("B")
        instruction["descriptor_bytes"] = f"{descriptor:02x}"
        operands = {field: reader.basic(descriptor >> shift, field in spec.owners)
                    for field, shift in zip(spec.fields, spec.shifts)}
        if opcode == 0x28:
            instruction["position_mode"] = descriptor & 3
    elif opcode in (0x08, 0x09, 0x0B, 0x0C, 0x0D, 0x1B, 0x1C, 0x1D):
        descriptor = reader.read("B")
        instruction["descriptor_bytes"] = f"{descriptor:02x}"
        if opcode == 0x08:
            name = "test_unsigned_range"
            operands = {"variable": reader.reference(), "minimum": reader.basic(descriptor),
                        "maximum": reader.basic(descriptor >> 2)}
            instruction["comparison"] = "unsigned_inclusive"
        elif opcode == 0x09:
            name = "switch"
            operands["value"] = reader.basic(descriptor)
            cases = []
            while True:
                case_offset = base + reader.position
                key = reader.read("B")
                delta = reader.read("h")
                edge = branch_target(case_offset, delta, "case")
                cases.append({"offset": case_offset, "key": key, "default": key == 255,
                              "delta": delta, **edge})
                edges.append(edge)
                if key == 255:
                    break
            instruction["cases"] = cases
            falls_through = False
        elif opcode == 0x0B:
            name = "copy_owner_variable"
            operands["source_owner"] = reader.basic(descriptor, owner=True)
            operands["source"] = reader.reference()
            # The original handler skips an extra halfword here; preserve it.
            operands["skipped_halfword"] = {"encoding": "u16", "value": reader.read("H")}
            operands["destination_owner"] = reader.basic(descriptor >> 2, owner=True)
            operands["destination"] = reader.reference()
        elif opcode in (0x0C, 0x0D):
            name = "read_record_bits" if opcode == 0x0C else "write_record_bits"
            selector = reader.read("B")
            operands["record_kind"] = {"encoding": "u8", "value": selector}
            operands["record_index"] = reader.basic(descriptor, owner=True)
            field = reader.read("I")
            operands["field_spec"] = {"encoding": "u32", "value": field}
            instruction["record_field"] = {
                "base": ("object", "runtime", "actor", "item", "battle", "status", "character", "game")[selector]
                        if selector < 8 else "unknown",
                "element_width": (field >> 30), "element_index": (field >> 16) & 0x3FFF,
                "bit_shift": (field >> 8) & 255, "bit_count": field & 255,
            }
            if opcode == 0x0C:
                operands["destination"] = reader.reference()
            else:
                operands["value"] = reader.basic(descriptor >> 2)
        elif opcode == 0x1B:
            name = "write_variable"
            operands = {"destination": reader.reference(), "value": reader.basic(descriptor)}
        elif opcode == 0x1C:
            name = "calculate"
            operands = {"left": reader.basic(descriptor), "right": reader.basic(descriptor >> 2),
                        "destination": reader.reference()}
            operation = descriptor >> 4
            instruction["calculation"] = CALCULATIONS[operation] if operation < len(CALCULATIONS) else "unhandled"
        else:
            name = "read_actor_position"
            operands = {"actor": reader.basic(descriptor, owner=True), "x_destination": reader.reference()}
    elif opcode == 0x0E:
        name = "indexed_jump"
        operands["index"] = reader.reference()
        edges.append({"kind": "indexed_jump", "table_offset": base + reader.position,
                      "unresolved": "table length and selected entry depend on the runtime variable"})
        falls_through = False
    else:
        raise DecodeError("rejected_opcode" if opcode < 0x40 or 0x60 <= opcode < 0x80 or opcode >= 0xC0
                          else "unknown_opcode")
    if falls_through:
        edges.append({"kind": "fallthrough", "target_offset": base + reader.position})
    return {"offset": base + position, "opcode": opcode, "command": name,
            "size": reader.position - position, "bytes": data[position:reader.position].hex(" "),
            "operands": operands, **instruction, "successors": edges}


def describe_variable(reference: int) -> dict:
    """Decode the address bits without guessing a variable's gameplay meaning."""
    kind = (reference >> 12) & 7
    return {"reference": reference, "variable_kind": kind,
            "scope": "saved_game" if kind < 3 else "runtime",
            "owner_relative": kind >= 3 and bool(reference & 0x8000),
            "word_index": (reference & 0xFFF) >> 5, "bit_shift": reference & 31}


def read_event_scripts(data: bytes, header: SceneHeader) -> dict:
    """Read directory slots and follow statically known control flow once per address."""
    section = data[header.event_scripts:header.strings]
    base = header.event_scripts
    result = {"offset": base, "size": len(section), "directory_size": 0, "entries": [], "instructions": [],
              "variables": [], "undecoded": [], "diagnostics": []}
    if not section:
        return result
    if len(section) < 2:
        raise ValueError("truncated event-script directory")
    table_size = struct.unpack_from("<H", section)[0]
    if table_size < 2 or table_size % 2 or table_size > len(section):
        raise ValueError("invalid event-script directory size")
    result["directory_size"] = table_size
    offsets = struct.unpack_from(f"<{table_size // 2}H", section)
    pending = []
    for index, offset in enumerate(offsets):
        entry = {"index": index, "relative_offset": offset}
        if table_size <= offset < len(section):
            entry["target_offset"] = base + offset
            heapq.heappush(pending, offset)
        elif offset == len(section):
            entry["empty"] = True
        else:
            entry["unresolved"] = "raw slot is not a code offset within this section"
        result["entries"].append(entry)

    instructions = {}
    attempted = set()
    owners = {}  # Byte position -> instruction start, to report overlapping entry points.
    while pending:
        position = heapq.heappop(pending)
        if position in attempted:
            continue
        attempted.add(position)
        if not table_size <= position < len(section):
            result["diagnostics"].append({"offset": base + position, "reason": "target_outside_code"})
            continue
        if position in owners:
            result["diagnostics"].append({"offset": base + position, "reason": "target_inside_instruction",
                                          "instruction_offset": base + owners[position]})
            continue
        try:
            instruction = decode_instruction(section, position, base)
        except DecodeError as error:
            result["diagnostics"].append({"offset": base + position, "reason": str(error),
                                          "opcode": section[position]})
            continue
        end = position + instruction["size"]
        if any(cursor in owners for cursor in range(position, end)):
            result["diagnostics"].append({"offset": base + position, "reason": "overlapping_instruction"})
            continue
        instructions[position] = instruction
        for cursor in range(position, end):
            owners[cursor] = position
        for edge in instruction["successors"]:
            if "target_offset" in edge:
                heapq.heappush(pending, edge["target_offset"] - base)

    result["instructions"] = [instructions[position] for position in sorted(instructions)]
    references = set()
    for instruction in result["instructions"]:
        for operand in instruction["operands"].values():
            if "reference" in operand:
                references.add(operand["reference"])
        for edge in instruction["successors"]:
            if "target_offset" in edge and edge["target_offset"] - base not in instructions:
                edge["unresolved"] = "target is not a decoded instruction"
    for entry in result["entries"]:
        if "target_offset" in entry and entry["target_offset"] - base not in instructions:
            entry["unresolved"] = "entry is not a decoded instruction; see diagnostics"
    result["variables"] = [describe_variable(reference) for reference in sorted(references)]
    position = table_size
    while position < len(section):
        if position in owners:
            position += 1
            continue
        start = position
        while position < len(section) and position not in owners:
            position += 1
        result["undecoded"].append({"offset": base + start, "size": position - start,
                                    "raw_bytes": section[start:position].hex(" ")})
    return result


def format_operand(operand: dict) -> str:
    """Keep runtime reads visibly distinct from literal reference addresses."""
    if operand["encoding"] == "variable":
        result = f"var[0x{operand['reference']:04X}]"
        return result + " (255=owner)" if operand.get("owner_when_255") else result
    if "reference" in operand:
        result = f"&var[0x{operand['reference']:04X}]"
        return result + " (keep top bit)" if operand.get("keep_top_bit") else result
    if operand.get("meaning") == "owner":
        return "owner"
    if "unresolved" in operand:
        return f"UNRESOLVED({operand['encoding']})"
    return str(operand["value"])


def format_disassembly(document: dict) -> str:
    """Render the directory, labeled instructions, variables and all undecoded spans."""
    lines = ["EVENT SCRIPTS", f"Section: {document['offset']:08X}   Size: {document['size']} bytes",
             "Offsets/bytes are hex. var[ref] reads a runtime value; &var[ref] names a variable.",
             "Entry indices are zero-based. Static paths are shown; runtime conditions are not evaluated.",
             "Raw directory slots may be data rather than code references. This is not assembler input.",
             "", "DIRECTORY", " Entry  Relative  Target"]
    labels = {}
    for entry in document["entries"]:
        if "unresolved" in entry:
            target = "UNRESOLVED: " + entry["unresolved"]
        elif entry.get("empty"):
            target = "EMPTY (section end)"
        else:
            target = f"L_{entry['target_offset']:08X}"
        lines.append(f" {entry['index']:5d}  {entry['relative_offset']:08X}  {target}")
        if "target_offset" in entry:
            labels.setdefault(entry["target_offset"], []).append(entry["index"])
    for instruction in document["instructions"]:
        for edge in instruction["successors"]:
            if "target_offset" in edge and edge["kind"] != "fallthrough":
                labels.setdefault(edge["target_offset"], [])
    lines.extend(["", "INSTRUCTIONS", "Offset    Bytes                                Command / operands"])
    for instruction in document["instructions"]:
        offset = instruction["offset"]
        if offset in labels:
            entries = ", ".join(map(str, labels[offset]))
            lines.extend(["", f"L_{offset:08X}:" + (f"  entries {entries}" if entries else "")])
        operands = " ".join(f"{name}={format_operand(value)}" for name, value in instruction["operands"].items())
        name = instruction["command"]
        if "subcommand_name" in instruction:
            name += "." + instruction["subcommand_name"]
        if "calculation" in instruction:
            name += "." + instruction["calculation"]
        raw = bytes.fromhex(instruction["bytes"])
        lines.append(f"{offset:08X}  {raw[:12].hex(' ').upper():<35}  {name} {operands}".rstrip())
        for start in range(12, len(raw), 12):
            lines.append(f"{offset + start:08X}  {raw[start:start + 12].hex(' ').upper():<35}  (continued bytes)")
        if "record_field" in instruction:
            field = instruction["record_field"]
            lines.append(f"          record: {field['base']} width_code={field['element_width']} "
                         f"index={field['element_index']} shift={field['bit_shift']} bits={field['bit_count']}")
        for case in instruction.get("cases", []):
            target = f"L_{case['target_offset']:08X}" if "target_offset" in case else "RETURN"
            key = "default" if case["default"] else str(case["key"])
            lines.append(f"          case {key} -> {target}")
        for edge in instruction["successors"]:
            if edge["kind"] in ("fallthrough", "case") and "unresolved" not in edge:
                continue
            target = f" -> L_{edge['target_offset']:08X}" if "target_offset" in edge else ""
            note = " UNRESOLVED: " + edge["unresolved"] if "unresolved" in edge else ""
            condition = f" ({edge['condition']})" if "condition" in edge else ""
            if "table_offset" in edge:
                target += f" table=0x{edge['table_offset']:08X}"
            lines.append(f"          [{edge['kind'].upper()}{condition}{target}{note}]")
        if "note" in instruction:
            lines.append("          " + instruction["note"])
    lines.extend(["", "VARIABLE REFERENCES", "Ref     Scope       Word  Bit  Owner-relative  Kind"])
    for variable in document["variables"]:
        lines.append(f"0x{variable['reference']:04X}  {variable['scope']:<10}  {variable['word_index']:4d}  "
                     f"{variable['bit_shift']:3d}  {str(variable['owner_relative']):<14}  {variable['variable_kind']}")
    lines.extend(["", "DIAGNOSTICS"])
    for diagnostic in document["diagnostics"]:
        lines.append(f"{diagnostic['offset']:08X}  {diagnostic['reason'].upper()}")
    if not document["diagnostics"]:
        lines.append("(none)")
    lines.extend(["", "UNDECODED BYTES (unreached data, padding or unsupported instructions)"])
    for span in document["undecoded"]:
        raw = bytes.fromhex(span["raw_bytes"])
        for start in range(0, len(raw), 16):
            lines.append(f"{span['offset'] + start:08X}  {raw[start:start + 16].hex(' ').upper()}")
    if not document["undecoded"]:
        lines.append("(none)")
    return "\n".join(lines) + "\n"


def main(argv: list[str] | None = None) -> int:
    """Inspect event bytecode in one original scene IMG."""
    return run(read_event_scripts, __doc__, argv, kind="event_scripts", text_renderer=format_disassembly)


if __name__ == "__main__":
    raise SystemExit(main())
