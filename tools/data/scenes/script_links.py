"""Static event links and a small vocabulary of source-backed gameplay actions.

The graph follows all decoded branches. Summaries describe possible operations,
not their execution order or the value of live game variables.
"""

from dataclasses import dataclass, field

from tools.data.scenes.reference_data import ReferenceData, item_label, resolve_item


@dataclass
class EventLinks:
    """Cache reachable instructions by entry, including unresolved paths."""

    document: dict
    cache: dict[int, dict] = field(default_factory=dict)
    instructions: dict[int, dict] = field(init=False, repr=False)

    def __post_init__(self):
        self.instructions = {item["offset"]: item for item in self.document["instructions"]}

    def resolve(self, entry: int) -> dict:
        if entry in self.cache:
            return self.cache[entry]
        result = {"entry": entry, "instructions": [], "unresolved": []}
        if not 0 <= entry < len(self.document["entries"]):
            result["unresolved"].append("Entry is outside the event directory.")
        else:
            directory = self.document["entries"][entry]
            if "unresolved" in directory or "target_offset" not in directory:
                result["unresolved"].append(directory.get("unresolved", "Entry is empty."))
            else:
                result["offset"] = directory["target_offset"]
                pending = [directory["target_offset"]]
                seen = set()
                while pending:
                    offset = pending.pop()
                    if offset in seen:
                        continue
                    seen.add(offset)
                    instruction = self.instructions.get(offset)
                    if instruction is None:
                        result["unresolved"].append(f"Undecoded target at 0x{offset:08X}.")
                        continue
                    result["instructions"].append(instruction)
                    for edge in instruction["successors"]:
                        if "unresolved" in edge:
                            result["unresolved"].append(f"0x{offset:08X}: {edge['unresolved']}")
                        if "target_offset" in edge:
                            pending.append(edge["target_offset"])
                result["instructions"].sort(key=lambda item: item["offset"])
        result["unresolved"] = list(dict.fromkeys(result["unresolved"]))
        self.cache[entry] = result
        return result


def literal(operand: dict) -> int | None:
    """Return only a stored or implicit constant; never substitute a runtime value."""
    return operand.get("value")


def operand_text(operand: dict) -> str:
    if operand.get("meaning") == "owner":
        return "this object"
    if operand.get("encoding") == "variable":
        return f"variable 0x{operand['reference']:04X}"
    if "value" in operand:
        return str(operand["value"])
    return "an unresolved operand"


def describe_operation(instruction: dict, chest: dict | None = None,
                       reference: ReferenceData | None = None) -> str | None:
    """Explain selected gameplay commands, retaining dynamic operands explicitly."""
    opcode = instruction["opcode"]
    operands = instruction["operands"]
    if opcode == 0x44:
        command = literal(operands["subcommand"])
        argument = operands["argument"]
        value = operand_text(argument)
        if command == 0x13:
            if chest and chest["initializer_enabled"] and argument.get("reference") == 0xE040:
                label = item_label(chest["item_id"], chest["item"])
                return f"Give the item held in variable 0xE040 (initialized to chest item {label})."
            item = literal(argument)
            return f"Give inventory item {item_label(item, resolve_item(item, reference))}." if item is not None else f"Give the inventory item selected by {value}."
        if command in (0x12, 0x14) and literal(argument) is not None:
            item = literal(argument)
            value = item_label(item, resolve_item(item, reference))
        if command == 0x2B:
            if chest and chest["initializer_enabled"] and chest.get("flag_binding") and argument.get("reference") == 0xE030:
                return ("Set the flag selected by variable 0xE030 to 1 "
                        f"(initializer selects collection flag 0x{chest['collection_variable_ref']:04X}).")
            flag = literal(argument)
            return f"Set variable 0x{flag & 0xFFFF:04X} to 1." if flag is not None else f"Set the variable selected by {value} to 1."
        descriptions = {
            0x12: "Check inventory count for item {value}.", 0x14: "Consume inventory item {value}.",
            0x22: "Add template item {value}.", 0x39: "Receive money: {value}.",
            0x3A: "Spend money: {value}.", 0x4A: "Wait {value} frame(s).",
            0x0F: "Make land {value} available.", 0x15: "Read land state for {value}.",
            0x38: "Defeat actor {value}.", 0x1B: "Request interaction {value}.",
        }
        if command in descriptions:
            return descriptions[command].format(value=value)
    if opcode == 0x03 and literal(operands["command_id"]) == 15:
        return "Spawn a monster using the runtime parameter block; actor and position are dynamic."
    if opcode == 0x1E:
        return f"Start a battle for trigger group {operand_text(operands['group'])}."
    if opcode == 0x4A:
        return f"Run actor action entry {operand_text(operands['entry'])} for {operand_text(operands['actor'])}."
    if opcode in (0x19, 0x34, 0x58):
        return f"Play sound {operand_text(operands['sound_id'])}."
    if opcode in (0x11, 0x33):
        return f"Show scene text {operand_text(operands['string_index'])}."
    if opcode == 0x15:
        return "Wait for the text window to finish."
    if opcode == 0x85:
        return f"Change scene to {operand_text(operands['scene_id'])}."
    return None
