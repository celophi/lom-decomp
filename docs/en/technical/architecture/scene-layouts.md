# How scene layouts decide what appears

[English documentation](../../README.md) | [日本語](../../../jp/technical/architecture/scene-layouts.md) | [Scene extractor](../reference/scene-extractor.md)

When we look at a chest in Legend of Mana, we can see where it is and whether
we've opened it. Looking at the extracted data takes a little more work. We
have a scene file, a layout, a record, and values like `0x0BC0`. We need to know
what each of those means before the condition makes much sense.

Let's start with the scene and work down to one record. Then we'll use an actual
chest to follow what the game does with it.

A scene is a map or room the game loads. The scene files we're looking at live
under `ANA/INFO_*` and have an `.IMG` extension. These files contain several
kinds of data, including graphics, text, scripts, and a layout.

The layout is a list of records. Each record describes something the scene can
place or activate. That might be an actor, such as a character or chest. It can
also be an event or another kind of scripted action. An actor here just means
an object the game can place and control; it doesn't have to be a person.

```text
Scene IMG
    Layout
        Record 0
        Record 1
        Record 2
        ...
```

We call this a layout record throughout the code and documentation. Both the
C code and the Python extractor name the type `FieldLayoutRecord`. In C,
`FieldSceneLayout.records` holds the layout records. These are names used in
the reconstructed code; we don't know the original developers' names.

Each record is 48 bytes long. It describes the kind of action, its condition,
position, resource settings, and the scripts and parameters used to control
it. Which fields matter depends on the kind of record. A script-only action,
for example, doesn't need to become a visible actor.

The condition is the part that decides whether installation can continue. It
has three fields:

| Field | What it tells the game |
| --- | --- |
| `variable_ref` | Reference identifying which stored value to read |
| `minimum` | The lowest value that passes |
| `maximum` | The highest value that passes |

The game reads the variable's current value and checks whether it falls within
that range. Both ends count. A minimum of 2 and a maximum of 5 would allow 2,
3, 4, or 5. That's just an example of the comparison, not a particular scene
record.

In C, the comparison looks like this:

```c
if ((value >= layout_record->condition.minimum) &&
    (value <= layout_record->condition.maximum))
```

Here, `value` is what the game just read from memory. The minimum and maximum
come from the layout record.

This distinction matters. The record tells the game which value to check and
which values are allowed. The current value lives elsewhere, in the game's
memory. The same scene file can therefore produce a different result depending
on the player's progress.

The `variable_ref` field is an encoded reference. The `_ref` suffix means
"reference": it identifies a stored value rather than containing its current
value. It describes where to read and how much to read. Some references select saved-game variables. Others select
values in the current field runtime. Some select a single bit, which can only
be 0 or 1; others select a larger value. We shouldn't assume every condition
is a checkbox, or that every condition reads saved-game data.

This is the installation path for records in a scene layout. Party members and
actors created while the game is running can use other paths.

Now let's use our chest.

The file is `ANA/INFO_PRT/WAL_B020.IMG`. `WAL_B020` is the scene's internal
filename; we haven't established its player-facing location name here. Its
layout contains three records. Record 2 is our chest. The numbering starts at
zero, so that's the third record.

After extracting this scene, its chest YAML is at
`assets/exports/us/scenes/WAL_B020/chests/002.yaml`. It contains these fields,
among others:

```yaml
x: 395
z: 180
item_id: 0x0096
collection_variable_ref: 0x0BC0
alternate_facing: false
condition:
  variable_ref: 0x0BC0
  minimum: 0
  maximum: 0
```

The first two values tell us where the chest goes. For the moment, we can leave
the item and facing settings alone and follow the condition.

`0x0BC0` tells the game which stored value to read. The `0x` prefix means the
number is written in hexadecimal, or base 16. In ordinary decimal notation,
`0x0BC0` is 3008. It isn't the current value of the collection variable.

For this particular reference, the game selects one bit in its saved-game
variable array, `g_field_game_state->words`. Each entry in that array holds
32 bits. The reference resolves like this:

```text
Bit offset:        3008
3008 divided by 32: 94, with a remainder of 0

Read word 94, bit 0.
```

Bit 0 is the lowest bit in that word. We can think of it as a checkbox that
means "this treasure has been collected." An unchecked box contains 0. A
checked box contains 1.

That decoding applies to `0x0BC0`. Other references also have type and storage
information encoded in them, so dividing an arbitrary reference by 32 isn't
the complete decoding rule.

The chest's minimum and maximum are both zero. That gives us exactly one
allowed value:

| Value read from the saved bit | Condition | Result |
| --- | --- | --- |
| 0 | Between 0 and 0 | Pass; install the chest |
| 1 | Outside 0 through 0 | Fail; skip the chest |

There doesn't need to be a 1 anywhere in this condition. The record says which
value is allowed. The 0 or 1 being tested comes from saved-game memory.

If the minimum and maximum were both 1, the condition would require a checked
box. If they were 0 and 1, either value of this one-bit variable would pass.
Our actual chest uses 0 and 0 because it should be installed while its treasure
hasn't been collected.

We can also see the condition in the original record bytes:

```text
C0 0B         00          00
variable_ref  minimum     maximum
0x0BC0        0           0
```

The two bytes of the variable are stored with the low byte first, which is why
`0x0BC0` appears as `C0 0B`. The following two zero bytes are separate fields.
They aren't part of `0x0BC0`. The extractor now shows all three fields directly
under `condition`, so we don't need to decode `record_bytes` to find them.

There's another detail in the YAML: `collection_variable_ref` and
`condition.variable_ref` both contain `0x0BC0`. This chest really does store that reference twice, for
two different uses.

The loader reads `condition.variable_ref` to decide whether to install the chest.
The chest's script gets its collection reference from slot 5 of the record's
`scripts` array. The extractor displays that parameter as
`collection_variable_ref`. The script needs to know which saved bit belongs to the treasure, too.

That array contains 16 entries. Some identify scripts to run, while others
provide data those scripts use. For the ordinary chest template:

| Slot | Use |
| --- | --- |
| `scripts[4]` | Item ID |
| `scripts[5]` | Collection-variable reference, with a facing flag in its top bit |
| `scripts[15]` | Initialization script reference |

The item and collection meanings come from the chest script. They aren't
universal meanings for those parameter slots in every actor record. In our
example, the facing bit is clear, so slot 5 contains `0x0BC0` directly.

Once the condition passes, the installer sets up the actor's script record,
copies the 16 entries, reserves its local variables, and schedules its startup
event. The scene loader then prepares and places the visible actor using the
record's resource and position settings. Scheduling the startup event means
its initialization script is ready to be run by the event system.

This also explains `active: false` in the extracted control fields. The export
shows the record before the game installs it. For this actor kind, the installer
sets `active` when the condition passes. That initial false value doesn't mean
the chest can never appear.

We can stop there for this part of the guide. We have followed the scene file
to one layout record, checked its condition against saved-game memory, and
prepared its actor and scripts. Opening the chest and handing out the reward
are the next part of the script's job. Changing the collection bit updates the
game's in-memory state; that change is written to a save file when the game is
saved.

If you want to follow this in the source, these are the places to start:

- [field_scene_transition.c](../../../../src/overlays/field/scene/field_scene_transition.c):
  `field_load_scene_actors()` walks the layout records and calls the installer.
- [field_interaction_start.c](../../../../src/overlays/field/scene/field_interaction_start.c):
  `field_install_actor_action()` reads the condition variable, checks its range,
  and sets up records that pass.
- [field_script_operands.c](../../../../src/overlays/field/scripts/field_script_operands.c):
  `field_read_script_var()` calls `field_resolve_script_var()` to locate the
  value, then reads it.
- [field_interaction_start.h](../../../../include/overlays/field/field_interaction_start.h):
  `FieldLayoutRecord` defines the layout record and its fields.
- [field_scene.py](../../../../tools/data/scenes/field_scene.py): `FieldLayoutRecord.chest_yaml()`
  turns the chest's layout fields into the readable YAML used in this example.
