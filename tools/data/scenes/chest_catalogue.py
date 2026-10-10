"""Chest catalogue for a batch export: one entry per chest, with every scene placing it.

Some rooms exist as several scene files that place the same chest. The chest's
collection flag is its identity, so opening it in one version removes it from
all of them. A batch scans every scene for chests first, so each export can
list the other placements, then writes chests.yaml at the end.
"""

from __future__ import annotations

from collections import defaultdict
from dataclasses import asdict, dataclass
from pathlib import Path

import yaml

from tools.data.scenes.layout import read_layout_records
from tools.data.scenes.presentation import SceneDumper, format_value
from tools.data.scenes.reference_data import item_label
from tools.data.scenes.scene_format import SceneHeader


CATALOGUE_FILE = "chests.yaml"
CATALOGUE_NOTES = (
    "One entry per chest, identified by its collection flag (chest_id).",
    "A chest can be placed in several scene versions of one room; opening it in one removes it from all.",
    "loot: a script hands over the item or money loaded into 0xE040. quest: no script does.",
    "money: the chest gives this many Lucre instead of an item.",
    "items_differ: placements store different rewards under one flag; it depends on the scene version.",
    "Files are relative to this directory. Coordinates are decimal; IDs are hexadecimal integers.",
)
REWARD_FIELDS = ("item_id", "item", "money")


@dataclass(frozen=True)
class ChestPlacement:
    """One chest record in one scene; scene is the batch name, e.g. INFO_GRB/GRB_E01A."""

    scene: str
    index: int
    chest_id: int

    @property
    def file(self) -> str:
        return f"{self.scene}/chests/{self.index:03d}.yaml"


class ChestIndex:
    """Every chest placement in a batch, looked up by chest ID or by scene."""

    def __init__(self, placements: list[ChestPlacement]):
        self.by_id: dict[int, list[ChestPlacement]] = defaultdict(list)
        self.by_scene: dict[str, list[ChestPlacement]] = defaultdict(list)
        for placement in placements:
            self.by_id[placement.chest_id].append(placement)
            self.by_scene[placement.scene].append(placement)

    @classmethod
    def scan(cls, scenes: dict[str, Path]) -> ChestIndex:
        """Read only the layout of each scene. Broken scenes are left for extraction to report."""
        placements = []
        for name, source in scenes.items():
            data = source.read_bytes()
            try:
                header = SceneHeader.parse(data)
                records = read_layout_records(data, header)
            except ValueError:
                continue
            event_scripts = data[header.event_scripts:header.strings]
            placements.extend(ChestPlacement(name, index, record.chest_id)
                              for index, record in enumerate(records) if record.is_common_chest(event_scripts))
        return cls(placements)

    def other_placements(self, scene: str) -> dict[int, list[str]]:
        """Map each chest's layout index in this scene to the files of its other placements."""
        return {placement.index: [other.file for other in self.by_id[placement.chest_id] if other != placement]
                for placement in self.by_scene[scene]}


@dataclass(frozen=True)
class ChestTotals:
    """Catalogue counts; chests counts chest IDs, scenes counts scenes with at least one chest."""

    chests: int
    loot: int
    quest: int
    placements: int
    scenes: int


def placement_entry(scene: str, obj: dict) -> dict:
    """Describe one placement from its object report entry; money chests have no item."""
    chest = obj["chest"]
    if "money" in chest:
        reward = {"money": chest["money"]}
    else:
        reward = {"item_id": chest["item_id"], "item": item_label(chest["item_id"], chest["item"])}
    return {"scene": scene, "chest_id": chest["collection_variable_ref"], "file": f"{scene}/{obj['file']}",
            "kind": chest["kind"], **reward, "x": obj["position"]["x"], "z": obj["position"]["z"]}


def catalogue_entry(chest_id: int, placements: list[dict]) -> dict:
    """Show the kind and reward once when every placement agrees on them."""
    listed = [{key: value for key, value in placement.items() if key not in ("scene", "chest_id")}
              for placement in placements]
    kinds = {placement["kind"] for placement in placements}
    entry = {"chest_id": chest_id, "kind": kinds.pop() if len(kinds) == 1 else "mixed"}
    rewards = [{key: placement[key] for key in REWARD_FIELDS if key in placement} for placement in listed]
    if all(reward == rewards[0] for reward in rewards):
        entry.update(rewards[0])
    else:
        entry["items_differ"] = True
    entry["placements"] = listed
    return entry


def build_catalogue(placements: list[dict]) -> dict:
    """Group placement entries by chest ID and count them."""
    groups: dict[int, list[dict]] = defaultdict(list)
    for placement in placements:
        groups[placement["chest_id"]].append(placement)
    chests = [catalogue_entry(chest_id, groups[chest_id]) for chest_id in sorted(groups)]
    totals = ChestTotals(
        chests=len(chests),
        loot=sum(1 for chest in chests if chest["kind"] == "loot"),
        quest=sum(1 for chest in chests if chest["kind"] == "quest"),
        placements=len(placements),
        scenes=len({placement["scene"] for placement in placements}),
    )
    return {"summary": asdict(totals), "chests": chests}


def write_catalogue(path: Path, catalogue: dict) -> None:
    """Write the catalogue with a short reading guide."""
    heading = "".join(f"# {note}\n" for note in CATALOGUE_NOTES)
    document = yaml.dump(format_value(catalogue), Dumper=SceneDumper, sort_keys=False,
                         allow_unicode=False, width=100)
    path.write_text(heading + "\n" + document, encoding="ascii")
