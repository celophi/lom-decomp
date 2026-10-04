"""Connect scene layout objects to event routines, monster templates and drops.

Links describe the stored layout and statically reachable operations. They do
not evaluate save conditions or execute scripts. Drop odds are conditional on
level and defeat flags; item names come from an explicitly selected FIELD.BIN.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

from tools.scenes.actors import read_actors
from tools.scenes.event_scripts import read_event_scripts
from tools.scenes.layout import COMMON_CHEST_INITIALIZER, LAYOUT_RECORD, FieldLayoutRecord, read_layout_records
from tools.scenes.scene_format import SceneHeader
from tools.scenes.scene_resources import read_resource_directory
from tools.scenes.script_links import EventLinks, describe_operation
from tools.scenes.drop_rates import choice_rates, format_rates, profiles
from tools.scenes.presentation import dump_yaml
from tools.scenes.reference_data import (ReferenceData, add_reference_arguments, item_label,
                                         reference_from_arguments, resolve_item)


KINDS = ("actor", "scene_event", "party_0", "party_1", "party_2", "menu_action", "script_only_actor", "group_actor")
EVENT_ROLES = {0: "Interaction", 2: "On screen", 3: "Off screen", 8: "Idle behavior",
               13: "Interaction / battle phase", 14: "Per-frame event", 15: "Initialization"}
# The common initializer splits scripts[5] into facing and collection reference.
CHEST_FLAG_SETUP = bytes.fromhex(
    "08 0a 50 e0 00 80 ff ff 05 0a 00 40 94 27 b0 01 07 00 "
    "40 84 27 b0 1c 58 50 e0 ff 7f 30 e0 00"
)


def describe_drop(drop: dict, rewards: list[dict]) -> str:
    """Describe a stored choice without assigning a probability to its slot."""
    kind = drop["kind"]
    if kind == "experience_and_money":
        return f"{drop['experience_pickups']} experience pickup(s), {drop['money_pickups']} money pickup(s)"
    if kind == "restore":
        return f"Restore pickup; half-restore chance {drop['half_restore_chance_out_of_256']}/256 within this slot"
    if kind == "inventory_item":
        return "Inventory item " + item_label(drop["item_id"], drop["item"])
    if kind == "template_item":
        indices = drop["reward_indices"]
        if not indices:
            return f"Reward key {drop['reward_key']} (no matching reward record)"
        reward = rewards[indices[0]]
        name = drop.get("reward_name") or reward["name_ascii"] or "encoded item name"
        return f"Reward {indices[0]}: {name} (key {drop['reward_key']}; first matching record)"
    return "No drop"


def template_catalog(data: bytes, header: SceneHeader, reference: ReferenceData | None = None) -> tuple[list[dict], list[dict]]:
    """Keep battle resources distinct; ambiguous resource selection is not guessed."""
    _, resources = read_resource_directory(data[header.records:header.actors])
    battles = []
    templates = []
    for resource in resources:
        document = resource.document(header.records)
        if "battle" not in document:
            continue
        file = f"records/{resource.slots[0]:03d}.yaml"
        battle = document["battle"]
        battles.append({"file": file, "offset": document["offset"]})
        for monster in battle["monsters"]:
            drops = []
            for stored in monster["drops"]:
                drop = dict(stored)
                drop["offset"] = int(monster["offset"], 16) + 0x40 + drop["slot"] * 2
                if drop["kind"] == "inventory_item":
                    drop["item"] = resolve_item(drop["item_id"], reference)
                if drop["kind"] == "template_item" and drop["reward_indices"]:
                    reward = battle["rewards"][drop["reward_indices"][0]]
                    drop["reward_name_offset"] = int(reward["offset"], 16) + 4
                    if reference:
                        drop["reward_name"] = reference.name(bytes.fromhex(reward["item_bytes"])[:20])
                drop["description"] = describe_drop(drop, battle["rewards"])
                drops.append(drop)
            templates.append({
                "index": monster["index"], "id": monster["id"], "name": monster["name_ascii"],
                "file": file, "path": f"battle.monsters[{monster['index']}]", "offset": monster["offset"],
                "hp_base": monster["hp_base"], "hp_growth": monster["hp_growth"],
                "hp_uses_stat_4_curve": monster["hp_uses_stat_4_curve"],
                "drops": drops,
            })
    return battles, templates


def link_scripts(record: FieldLayoutRecord, chest: bool, links: EventLinks) -> tuple[list[dict], list[dict]]:
    """Name known slots; keep parameters and unknown low-valued slots separate."""
    scripts = []
    parameters = []
    for slot, raw in enumerate(record.scripts):
        if raw == 0xFFFF:
            continue
        if chest and slot in (4, 5):
            parameters.append({"slot": slot, "raw": raw,
                               "role": "item_id" if slot == 4 else "collection_reference_and_facing"})
            continue
        if slot not in EVENT_ROLES and not raw & 0x8000:
            parameters.append({"slot": slot, "raw": raw, "role": "unclassified"})
            continue
        # Both the touch and action-button paths dispatch scripts[0].
        enabled = bool(record.enabled_events & (3 if slot == 0 else 1 << slot))
        script = {"slot": slot, "role": EVENT_ROLES.get(slot, f"Event slot {slot} (role unconfirmed)"),
                  "raw": raw, "enabled": enabled}
        if slot == 0 and not raw & 0x8000:
            script.update(kind="message", string_index=raw)
        else:
            entry = raw & 0x7FFF
            resolved = links.resolve(entry)
            script.update(kind="event_script", entry=entry, file="event_scripts/data.txt",
                          decoded_instruction_count=len(resolved["instructions"]),
                          returns_immediately=len(resolved["instructions"]) == 1 and
                          resolved["instructions"][0]["opcode"] == 0)
            if "offset" in resolved:
                script.update(offset=resolved["offset"], label=f"L_{resolved['offset']:08X}")
            if resolved["unresolved"]:
                script["unresolved"] = resolved["unresolved"]
        scripts.append(script)
    scripts.sort(key=lambda item: ({15: 0, 0: 1, 8: 2, 14: 3}.get(item["slot"], 4), item["slot"]))
    return scripts, parameters


def build_report(data: bytes, header: SceneHeader, events: dict, actors: dict,
                 reference: ReferenceData | None = None) -> dict:
    """Build a report, reusing section documents when called by the full extractor."""
    records = read_layout_records(data, header)
    event_section = data[header.event_scripts:header.strings]
    links = EventLinks(events)
    battles, templates = template_catalog(data, header, reference)
    drop_profiles = profiles(reference.drop_boundaries) if reference else []
    for template in templates:
        groups = {}
        for drop in template["drops"]:
            # Keep mechanically different choices distinct, even if their names match.
            groups.setdefault((drop["handler"], drop["value"]), []).append(drop)
        template["drop_choices"] = []
        for drops in groups.values():
            slots = [drop["slot"] for drop in drops]
            template["drop_choices"].append({
                "slots": slots, "description": drops[0]["description"],
                "rates": {profile["name"]: choice_rates(slots, profile) for profile in drop_profiles},
            })
    objects = []
    for index, record in enumerate(records):
        kind = (record.control >> 4) & 15
        selector = (record.control >> 8) & 255
        chest = record.is_common_chest(event_section)
        folder = "chests" if chest else "layout"
        offset = header.layout + 4 + index * LAYOUT_RECORD.size
        item = {
            "index": index, "kind": "chest" if chest else KINDS[kind] if kind < len(KINDS) else "unknown",
            "file": f"{folder}/{index:03d}.yaml", "offset": offset,
            "position": {"x": record.position & 0xFFFF, "y": record.position >> 30,
                         "z": (record.position >> 16) & 0x7FF},
            "condition": {"variable_ref": record.condition_variable_ref,
                          "minimum": record.condition_minimum, "maximum": record.condition_maximum},
            "trigger_group": record.control & 15, "selector": selector,
            "hidden": bool(record.control & 0x40000000),
            "interaction": {"touch": bool(record.enabled_events & 1),
                            "action_button": bool(record.enabled_events & 2)},
        }
        if kind in (0, 6, 7):
            item["actor_id"] = index + 3
        elif kind in (2, 3, 4):
            item["actor_id"] = kind - 2
        elif kind == 1:
            item["actor_id"] = 0x80
        if kind in (0, 7):
            resource = record.source & 7
            item["actor_resource"] = {"resource_index": resource + 3}
            if resource == 5:
                item["actor_resource"]["kind"] = "shared_chest_resource"
            elif resource < len(actors["actors"]):
                actor = actors["actors"][resource]
                item["actor_resource"].update(file="actors/data.yaml", index=resource, offset=actor["offset"])
            else:
                item["actor_resource"]["unresolved"] = "No matching scene actor description."
        scripts, parameters = link_scripts(record, chest, links)
        item.update(scripts=scripts, parameters=parameters)
        chest_info = None
        if chest:
            initializer = links.resolve(record.scripts[15] & 0x7FFF)
            start = initializer.get("offset", -1)
            signature = COMMON_CHEST_INITIALIZER + CHEST_FLAG_SETUP
            chest_info = {"item_id": record.scripts[4], "collection_variable_ref": record.scripts[5] & 0x7FFF,
                          "alternate_facing": bool(record.scripts[5] & 0x8000),
                          "flag_binding": start >= 0 and data[start:start + len(signature)] == signature,
                          "initializer_offset": start, "initializer_enabled": bool(record.enabled_events & 0x8000)}
            chest_info.update(item=resolve_item(record.scripts[4], reference),
                              item_id_offset=offset + 24, collection_setting_offset=offset + 26)
            item["chest"] = chest_info
        elif kind in (0, 7):
            matches = [template for template in templates if template["id"] == selector]
            link = {"template_id": selector, "selector_offset": offset + 1,
                    "basis": "Layout selector becomes object-state byte 0x11, used as the battle template ID."}
            if len(battles) > 1:
                link.update(status="unresolved", reason="Multiple battle resources; active resource is not selected statically.")
            elif matches:
                first = matches[0]
                link.update(status="linked", name=first["name"], file=first["file"], index=first["index"],
                            offset=first["offset"], matching_indices=[match["index"] for match in matches])
                if len(matches) > 1:
                    link["note"] = "The runtime uses the first matching template ID."
            else:
                link.update(status="no_match", reason="No matching template in the scene's battle resources.")
            item["monster_template"] = link
        operations = {}
        for script in scripts:
            if script["kind"] != "event_script" or not script["enabled"]:
                continue
            for instruction in links.resolve(script["entry"])["instructions"]:
                description = describe_operation(instruction, chest_info, reference)
                if description is None:
                    continue
                operation = operations.setdefault(instruction["offset"], {
                    "offset": instruction["offset"], "description": description,
                    "file": "event_scripts/data.txt", "via_slots": [],
                })
                if instruction["opcode"] == 0x44:
                    operands = instruction["operands"]
                    if operands["subcommand"].get("value") in (0x12, 0x13, 0x14):
                        item_id = operands["argument"].get("value")
                        if item_id is not None:
                            operation.update(item_id=item_id, item=resolve_item(item_id, reference))
                operation["via_slots"].append(script["slot"])
        item["possible_operations"] = list(operations.values())
        objects.append(item)
    return {"layout_count": len(records), "objects": objects, "monster_templates": templates,
            "reference_data": reference.source if reference else {"status": "not_selected"},
            "drop_rates": {"status": "conditional" if reference else "unresolved",
                           "table_bytes": list(reference.drop_boundaries) if reference else [],
                           "profiles": drop_profiles,
                           "runtime_source": "src/overlays/field/records/field_reward_command_ops.c:field_roll_defeat_drop"},
            "notes": ["Conditions are stored tests, not evaluated against a save.",
                      "Possible operations follow all static paths from enabled scripts; they are not an execution order.",
                      "Summaries cover selected gameplay commands. Full instructions remain in event_scripts/data.txt.",
                      "Template links describe the initially loaded selector. Scripts may change actor state or resources.",
                      "Drop rates describe slot selection with uniform random low bits, not successful item collection.",
                      "The live monster level and defeat flags are not inferred from the scene.",
                      "Item names and drop-level boundaries use the selected FIELD reference; missing names remain explicit.",
                      "Item-name offsets are in decompressed FIELD (including its header byte); drop and chest setting offsets are in the IMG."]}


def read_scene_report(data: bytes, header: SceneHeader, reference: ReferenceData | None = None) -> dict:
    """Inspect one original scene without requiring an existing export."""
    return build_report(data, header, read_event_scripts(data, header), read_actors(data, header), reference)


def format_report(document: dict) -> str:
    """Present object settings and links first, with technical evidence alongside."""
    lines = ["SCENE OBJECTS", f"Layout records: {document['layout_count']}",
             "Paths refer to a full scene export. Offsets locate original IMG bytes.",
             "Conditions depend on the save. Script summaries show possible operations, not their order."]
    reference = document["reference_data"]
    if "file" in reference:
        lines.append(f"FIELD reference ({reference['version']}): {reference['file']}")
        lines.append("FIELD name/table offsets refer to the decompressed binary, including its header byte.")
    else:
        lines.append("Stored item IDs and drop pairs are shown. External item names and drop probabilities are optional annotations.")
    templates = {(item["file"], item["index"]): item for item in document["monster_templates"]}
    for obj in document["objects"]:
        monster = obj.get("monster_template", {})
        title = "Chest" if obj["kind"] == "chest" else obj["kind"].replace("_", " ").title()
        name = monster.get("name") if monster.get("status") == "linked" else None
        lines.extend(["", f"{title} {obj['index']}" + (f" - {name}" if name else ""), "-" * 64])
        pos = obj["position"]
        lines.append(f"  Position: X={pos['x']}, Z={pos['z']}, Y={pos['y']}   Trigger group: {obj['trigger_group']}")
        lines.append(f"  Layout: {obj['file']}   IMG 0x{obj['offset']:08X}")
        condition = obj["condition"]
        lines.append(f"  Present when variable 0x{condition['variable_ref']:04X} is "
                     f"{condition['minimum']}..{condition['maximum']} (inclusive)")
        if obj["hidden"]:
            lines.append("  Starts hidden.")
        if "chest" in obj:
            chest = obj["chest"]
            lines.extend([f"  Item: {item_label(chest['item_id'], chest['item'])} [IMG 0x{chest['item_id_offset']:08X}]",
                          f"  Collected flag reference: 0x{chest['collection_variable_ref']:04X} [IMG 0x{chest['collection_setting_offset']:08X}]",
                          f"  Alternate facing: {'yes' if chest['alternate_facing'] else 'no'}",
                          "  Initializer code loads the item into 0xE040 and collection/facing settings into 0xE050."])
            if "name_offset" in chest["item"]:
                lines.append(f"  Item name: decompressed FIELD 0x{chest['item']['name_offset']:08X}")
            if chest["flag_binding"]:
                lines.append("  Confirmed setup splits facing from the collection reference, stored in 0xE030.")
        if "actor_resource" in obj:
            resource = obj["actor_resource"]
            detail = resource.get("file", resource.get("kind", resource.get("unresolved", "")))
            if "index" in resource:
                detail += f" -> actors[{resource['index']}]"
            lines.append(f"  Graphics/actions: resource {resource['resource_index']} ({detail})")
        if monster:
            if monster["status"] == "linked":
                lines.append(f"  Initial monster template: ID {monster['template_id']}, {monster['file']} -> battle.monsters[{monster['index']}]")
                template = templates[(monster["file"], monster["index"])]
                hp_growth = "stat 4 curve" if template["hp_uses_stat_4_curve"] else str(template["hp_growth"])
                lines.append(f"  Stored HP: base {template['hp_base']}, growth {hp_growth}; full stats in the template YAML.")
                lines.append("  Drops (chances below assume no defeat modifiers):")
                for choice in template["drop_choices"]:
                    slots = ", ".join(map(str, choice["slots"]))
                    offsets = ", ".join(f"0x{template['drops'][slot]['offset']:08X}" for slot in choice["slots"])
                    lines.append(f"    Slots {slots}: {choice['description']}")
                    rates = choice["rates"].get("normal")
                    lines.append("      " + (format_rates(rates) if rates else "Probability uses an external FIELD table; no reference selected."))
                    lines.append(f"      Stored handler/value pairs: IMG {offsets}")
                    first = template["drops"][choice["slots"][0]]
                    if "name_offset" in first.get("item", {}):
                        lines.append(f"      Item name: decompressed FIELD 0x{first['item']['name_offset']:08X}")
                    elif "reward_name_offset" in first:
                        lines.append(f"      Reward name: IMG 0x{first['reward_name_offset']:08X}")
            elif monster["status"] == "no_match":
                lines.append(f"  No scene monster template for selector {monster['template_id']}; actor type is not inferred.")
            else:
                lines.append(f"  Monster template unresolved: {monster['reason']}")
        triggers = obj["interaction"]
        lines.append(f"  Interaction: touch {'enabled' if triggers['touch'] else 'disabled'}, "
                     f"action button {'enabled' if triggers['action_button'] else 'disabled'} (both use slot 0)")
        lines.append("  Script links:")
        empty_routines = []
        for script in obj["scripts"]:
            if script.get("returns_immediately") and script["slot"] not in (0, 8, 14, 15):
                empty_routines.append(f"slot {script['slot']} -> entry {script['entry']}")
                continue
            enabled = "" if script["enabled"] else " [disabled in layout]"
            if script["kind"] == "message":
                target = f"scene message {script['string_index']}"
            else:
                target = f"event entry {script['entry']}"
                if "label" in script:
                    target += f" -> {script['file']} : {script['label']}"
            lines.append(f"    {script['role']} (slot {script['slot']}): {target}{enabled}")
            for reason in script.get("unresolved", []):
                lines.append(f"      UNRESOLVED: {reason}")
        if empty_routines:
            lines.append("    Return-only routines: " + "; ".join(empty_routines))
        if not obj["scripts"]:
            lines.append("    No identified script references.")
        if obj["parameters"]:
            lines.append("  Parameter/unclassified slots: " + ", ".join(
                f"{item['slot']}=0x{item['raw']:04X} ({item['role']})" for item in obj["parameters"]))
        if obj["possible_operations"]:
            lines.append("  Possible script operations (not execution order):")
            groups = {}
            for operation in obj["possible_operations"]:
                groups.setdefault(operation["description"], []).append(operation)
            for description, operations in groups.items():
                operation = operations[0]
                slots = sorted({slot for entry in operations for slot in entry["via_slots"]})
                evidence = f"IMG 0x{operation['offset']:08X}; slots {', '.join(map(str, slots))}"
                if len(operations) > 1:
                    evidence += f"; {len(operations)} sites, all offsets in objects.yaml"
                lines.append(f"    - {description} [{evidence}]")
    if templates and document["drop_rates"]["profiles"]:
        lines.extend(["", "DROP SLOT CHANCES BY LEVEL AND DEFEAT FLAGS",
                      "  Sum the slots above for an item's combined chance. All four cases are also in objects.yaml.",
                      "  extra_slots = flag 0x04000000; no_common = flag 0x08000000; both = both flags."])
        for profile in document["drop_rates"]["profiles"]:
            lines.append(f"  {profile['name']}:")
            lines.append("    Level       Slot 0    Slot 1    Slot 2    Slot 3    Slot 4    Slot 5    Slot 6    Slot 7")
            for band in profile["bands"]:
                levels = f"{band['level_min']}-{band['level_max']}"
                chances = " ".join(f"{weight * 100 / 128:g}%".rjust(9) for weight in band["weights_out_of_128"])
                lines.append(f"    {levels:<8} {chances}")
    lines.extend(["", "READING THIS REPORT"] + ["  " + note for note in document["notes"]])
    return "\n".join(lines) + "\n"


def main(argv: list[str] | None = None) -> int:
    """Print one scene's object report as YAML or text."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="original scene IMG")
    parser.add_argument("-o", "--output", type=Path, help="new output file (default: standard output)")
    parser.add_argument("--format", choices=("yaml", "text"), default="yaml")
    add_reference_arguments(parser)
    args = parser.parse_args(argv)
    try:
        reference = reference_from_arguments(args)
        data = args.source.read_bytes()
        document = read_scene_report(data, SceneHeader.parse(data), reference)
        text = format_report(document) if args.format == "text" else dump_yaml(document, "scene_report")
        if args.output is None:
            sys.stdout.write(text)
        else:
            with args.output.open("x", encoding="utf-8") as output:
                output.write(text)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
