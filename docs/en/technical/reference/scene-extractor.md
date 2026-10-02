# Scene IMG extractor

[English documentation](../../README.md) | [日本語](../../../jp/technical/reference/scene-extractor.md) | [Scene tools](../../../../tools/scenes/README.md) | [Scene layouts and conditions](../architecture/scene-layouts.md)

The extractor documents the stored scene IMG format for decompilation. TXT
explains the structure and YAML records decoded fields; raw bytes preserve
every section. By default it reads only the supplied IMG.

The extractor reads scene files under `ANA/INFO_*` and `ANA/MAPINFO`. It exports
all layout records, actor descriptions, actor scripts, event scripts, scene resource directories,
monster templates, drop tables, encoded strings, actor animation geometry, textures and portraits. Undecoded data stays under its section name. The input
file is never modified. Run with Python 3.10 or newer and the project's PyYAML
dependency installed.

```sh
make extract-scene SCENE=/path/to/ANA/INFO_PRT/WAL_B020.IMG
```

Output goes to `assets/exports/us/scenes/WAL_B020/`. `VERSION=jp` selects the JP
output folder and text dialect, and `SCENE_OUTPUT` changes the parent directory.
`SCENE_TEXT_ENCODING=us|jp` can override the text dialect without changing the
output folder. `VERSION` alone does not load external references. Extraction needs
a new destination directory. The files are generated and ignored by Git.

For all supported scenes in an extracted ANA directory:

```sh
make extract-scenes ANA=/path/to/ANA
```

The batch target reads `INFO_*/*.IMG` and `MAPINFO/*.IMG` (extensions are case-insensitive). Output keeps the group and scene names,
for example `assets/exports/us/scenes/INFO_PRT/WAL_B020/`. Use `VERSION=jp` with a
Japanese ANA directory. `SCENE_OUTPUT` also applies to batch extraction.

Every scene destination must be new. The tool checks for existing destinations
before starting. Invalid scenes stop the batch with a filename; files already
extracted remain in place. Other IMG families aren't included. Each scene is
written in a temporary directory, reconstructed from its exported files, and
compared byte for byte with its source. Only a successful export is moved to
its destination. Failed writes and failed reconstruction checks leave no
partial scene directory.

## Identifying scene IMGs

The filename alone does not identify an IMG format. Check a file's contents:

```sh
python3 -m tools.scenes.identify_img /path/to/file.IMG
```

The command prints `scene IMG` or `not scene IMG`, with the failed check for a
nonmatching file. An explicit file can have any extension. To scan a directory
and its subdirectories for `.IMG` files (case-insensitive):

```sh
python3 -m tools.scenes.identify_img --recursive /path/to/ANA
make identify-img IMG=/path/to/ANA
```

Without `--recursive`, the Python command checks only the directory's immediate
files. The Make target always includes subdirectories. A scan prints one result
per file and a final count. Nothing is extracted or modified.

Detection reuses the extractor's checks for the ten section offsets, layout
records, TIM images and portraits. It also checks the final group-bounds word
count against the file size. It does not interpret every resource or validate
script behavior. A damaged scene can therefore be reported as `not scene IMG`;
that result does not identify which other IMG family a file belongs to.

Exit status is 0 when inspection completes, including scans containing non-scene
files. Input or reading errors return 1; invalid command-line arguments return 2.
An empty directory is an input error.

## Output

| File | Contents |
| --- | --- |
| `objects.txt` and `.yaml` | Object settings, script links, initial monster template associations and readable drop choices |
| `scene.txt` | Header field map, section formats, decoding coverage, code references and paths to decoded records |
| `scene.yaml` | Header fields, section offsets/sizes/formats, coverage counts and code references, including empty sections |
| `byte-map.yaml` | Every original byte range, its file and optional decoded metadata |
| `header.bin` | The 40-byte section-offset header |
| `layout/count.bin` | Original layout record count |
| `layout/<index>.yaml` | Every non-chest layout record, with original bytes |
| `chests/<index>.yaml` | Recognized chest records, retaining the existing format |
| `records/directory.bin` and `.yaml` | Resource offsets and their file references |
| `records/<index>.bin`, `.yaml` and `.txt` | Original resource, stored fields and readable listing; index is its first directory slot |
| `textures/directory.bin` and `.yaml` | Original image offsets and their TIM references |
| `textures/000.tim`, etc. | Whole TIM assets in file order |
| `portraits/count.bin`, `portraits/000.bin`, etc. | Original count and portrait records |
| `group_bounds/data.bin` and `.yaml` | Original counted word array and its words |
| `actors/data.bin` and `.yaml` | Actor descriptions, image references and action slots |
| `actor_scripts/data.bin` and `.yaml` | Actor-script directory and disassembled instructions |
| `actor_scripts/data.txt` | Compact disassembly with shared references, fallthroughs and undecoded bytes |
| `event_scripts/data.bin` and `.yaml` | Original event section, directory slots, instructions, variables and undecoded spans |
| `event_scripts/data.txt` | Event disassembly with branch labels, calls, switch cases and diagnostics |
| `strings/data.bin`, `.yaml` and `.txt` | Original text section, directory entries, readable text, controls and encoded tokens |
| `geometry/data.bin`, `.yaml` and `.txt` | Actor resource boundaries, animation sequences, timing, sprite fields and placement records |
| `<section>/<offset>.bin` | Original bytes between separately extracted assets |

Each byte-map entry covers one range. `offset` is its hexadecimal position in
the original IMG, and `size` is its byte count. The range ends at
`offset + size`, exclusive. `file`, optional `metadata` and optional `listing` paths are relative to
the export directory. Layout records are always 48 original bytes, regardless
of the size of their YAML representation.

Shared TIM and resource references retain their directory slots but point to
one stored file. Empty resource entries are preserved too. Metadata files do
not own additional byte ranges; their companion binary remains authoritative.

To reconstruct an export in Python, call
`tools.scenes.field_scene.reconstruct(output_directory)` with a `Path`. It reads
the byte map, takes `record_bytes` from layout YAML, and concatenates those bytes
with the original binary files. It checks range positions and sizes. Extraction
also compares the result with the input. Editing decoded fields does not rebuild
a modified game file.

## Reading the format for decompilation

Start with `scene.txt` for the binary layout. Its header table shows where each
of the ten little-endian `u32` section offsets is stored. Section offsets are
relative to IMG byte zero; section starts are four-byte aligned. The overview
then describes record sizes and pointer bases, reports decoding coverage, and
identifies the C structures or loading functions supporting the interpretation.
`scene.yaml` carries the same header and section information as structured data.

Coverage is reported per section:

- `decoded`: the stored layout is mapped. Unnamed numeric fields or bits can
  still have unknown meanings.
- `partial`: raw payloads, unclassified words, unsupported instructions or
  decoding gaps remain. Script reports include counts of undecoded bytes and
  unresolved directory entries.
- `raw`: the section is preserved as binary without internal decoding.
- `empty`: the section contains no bytes.

Coverage describes structure rather than bytes preserved: every original byte
is retained regardless of status. Strings and animation geometry have decoded
directories and records; unreferenced spans remain explicit. TIM payloads are
validated, but their fields remain in the TIM files; portraits
retain their raw palette and pixel records. Recognized resource payloads have
TXT/YAML descriptions. Unrecognized IDs, item script bytecode and payloads
with invalid layouts retain their binary data with coverage notes or diagnostics.

Stored values and raw byte fields are distinguished from interpretations in the
reading notes. Split flags, ASCII name previews, sentinel explanations, and
`objects.txt`/`objects.yaml` summaries help interpret the same IMG. Those object
reports link layout selectors, scripts and monster templates within the file;
conditions and runtime effects are not evaluated. Optional external annotations
identify their source separately.

The overview also lists chest record paths and monster names stored in the
IMG alongside their resource YAML. Some scenes have no recognized chests or
battle templates; the overview says so. The path guide covers the supported output types, including
ones that may be absent in a particular scene.

In generated metadata YAML, offsets, opcodes, IDs named `resource_id` or `item_id`,
packed words and masks use hexadecimal **integers**, such as `offset: 0x00001670`.
Counts, indices, coordinates and ordinary stat values use decimal. Earlier
metadata exports quoted hexadecimal values as strings; consumers should now
read these fields as integers. Layout and chest files keep their existing schema.

Short scalar arrays stay on one line, and stat pairs appear as `{base: 5, growth: 6}`.
Raw byte fields longer than sixteen bytes use YAML literal blocks in rows of
sixteen; `bytes.fromhex()` accepts the resulting whitespace. Empty `operands`,
`remaining_bytes`, `trailing_bytes` and `directory_trailing_bytes` are omitted.
An omitted byte field means no bytes; meaningful empty lists such as unmatched
`reward_indices` remain visible, as do zero values and false flags.

Field notes explain `image_slots`' `0xFF` terminator, action animation's `0xFFFF`
sentinel and HP growth's `0xFFFF` stat-curve selector. Other stored values remain
literal, including command-dependent action parameters. Unknown fields retain
their names and values until the runtime interpretation is established.

Use `byte-map.yaml` to locate a record in the original IMG or its exported binary.
Indices are zero-based, and shared offsets refer to the same stored data. These
are reference exports: changing a decoded stat, drop or instruction does not
encode that change back into binary. Reconstruction uses the mapped binary
files and layout `record_bytes`. Byte-for-byte reconstruction checks preservation;
it does not verify the gameplay effects of a modified file.

## Layout records

All layout records expose their condition, control bits, position, source,
enabled events and sixteen script/parameter slots. Their filenames retain the
original zero-based layout index. Recognized chests go under `chests/`; every
other record goes under `layout/`.

A layout entry is not necessarily a monster or even a visible actor. The tool
does not execute scripts, assign species from a layout index, or assume that
parameter slots have the same meaning for every actor kind.

## Scene object reports

`objects.txt` brings each layout record's settings, event links and initial
monster template together. `objects.yaml` contains the same associations and
individual instruction evidence in a structured form. Both are descriptive
reports; they do not replace layout records or own any bytes in the byte map.

```sh
make inspect-scene-objects SCENE=/path/to/scene.IMG
python3 -m tools.scenes.scene_report /path/to/scene.IMG --format text
python3 -m tools.scenes.scene_report /path/to/scene.IMG -o objects.yaml
```

The Make target defaults to text; use `SCENE_REPORT_FORMAT=yaml` for YAML.
The Python command defaults to YAML and accepts `--format text`. An existing
`-o` destination is refused. Paths in standalone reports refer to where the
companion files would be in a full scene export.

Each object retains its layout index, original offset, position, trigger group,
hidden flag and inclusive appearance condition. The report does not evaluate
that condition against a save. Known event roles include initialization (slot
15), idle behavior (8), per-frame updates (14), on/off screen (2/3), and the
interaction/battle phase callback (13). Unconfirmed roles retain their slot
numbers. Missing or conflicting event targets remain unresolved.

Both touch and action-button interactions dispatch **slot 0** through
`field_start_interaction`; their enable bits are 0 and 1 respectively. Slot 1
is not labeled as the action-button routine. Slot 0 with bit 15 clear names a
message; with bit 15 set it names an event entry. Other known event slots use
the low 15 bits as the entry index. Unclassified low-valued slots remain data,
and `0xFFFF` means no reference. The report keeps enabled/disabled state.

For recognized chests, slots 4 and 5 are parameters, even if their bits could
look like script references. The report shows the configured item, collection
reference and facing. The initializer signature establishes the item and
settings variable bindings. A separate exact match of its flag-splitting code
is required before naming the collection-reference variable. Summaries only
use these initialization bindings when the initializer is enabled.

For example, `WAL_B020` chest 2 links initialization entry 47, interaction entry
48, and idle entry 54. With the US FIELD reference, its report identifies
Vampire Fang (`0x0096`), collection reference
`0x0BC0`, the give-item operation, and the operation that sets the selected flag.
Every operation links to its original instruction offset. These are possible
operations reachable through static branches/calls, not an execution order or
a guarantee that the game takes that path. Repeated descriptions are grouped
in text; YAML retains every individual instruction and referring slot.

For placed actor kinds 0 and 7, the loader copies the layout control word to
`FieldObjectState::group_flags`. Its selector byte is therefore object-state
byte `0x11`, also named `FieldStatusState::template_index`.
`field_init_monster_record` looks up that value by **template ID**, not directory
index. The report uses this confirmed chain to link the initial placement to
its local battle template, stored HP inputs and drop choices. Graphics/action
resources are linked separately through the layout's resource selector.

For example, `LAK_B050` links six placed actors to Stinger Bug, Spiny Cone and
Tonpole templates. Multiple placements may share one template. Duplicate IDs
within one battle resource retain their matches and select the first, as the
runtime does. Multiple battle resources leave resource selection unresolved.
An absent template does not establish an actor's type. Script-only, party and
scene-event layouts do not receive an inferred placed-monster association.

These links describe the initially loaded selector. Scripts can later change
actor state, resources or battle participation. Runtime-selected calls remain
unresolved, and operation summaries cover a selected vocabulary of gameplay
commands; consult the event listing for complete decoded instructions. Drop
slots are not equally likely; the selected FIELD reference supplies item names
and the level table used to calculate conditional chances.

The supporting runtime code is in
[field_scene_transition.c](../../../../src/overlays/field/field_scene_transition.c),
[field_interaction_start.c](../../../../src/overlays/field/field_interaction_start.c),
[field_contact_geometry.c](../../../../src/overlays/field/field_contact_geometry.c),
[field_actor_templates.c](../../../../src/overlays/field/field_actor_templates.c)
and [field_actor_lifecycle.c](../../../../src/overlays/field/field_actor_lifecycle.c).

## Item names and drop chances

The Make targets `inspect-scene-objects`, `extract-scene` and `extract-scenes`
read only the supplied IMG by default. Stored item IDs, drop handler/value pairs
and scene-local names remain available without FIELD or CLOAD files.

To request external annotations explicitly, set `SCENE_REFERENCE_VERSION=us`
or `jp`. This selects `disc/<reference-version>/BIN/FIELD.BIN`.
`SCENE_FIELD_BIN=/path/to/FIELD.BIN` selects an alternate compressed binary when
annotations are enabled. `VERSION` selects the output folder independently;
it does not enable annotations. Scene IMGs do not identify their region, so the
tool does not infer it from the filename.

```sh
make extract-scene SCENE=/path/to/scene.IMG SCENE_REFERENCE_VERSION=us
make inspect-scene-objects SCENE=/path/to/scene.IMG SCENE_REFERENCE_VERSION=jp
```

For direct Python commands, opt in explicitly:

```sh
python3 -m tools.scenes.scene_report scene.IMG --version us --format text
python3 -m tools.scenes.field_scene scene.IMG new-export --version us
python3 -m tools.scenes.scene_report scene.IMG --version us --field-bin modified/FIELD.BIN
```

Without `--version`, direct commands still work, but name and probability
fields explicitly remain unresolved. A selected reference that is missing,
malformed, or incompatible with the regional layout is an error. Japanese
names also need the CLOAD character chart produced by `make splat VERSION=jp`.
The caller is responsible for selecting a reference matching the scene.

`reference_data.py` reads `g_field_item_name_table`, the same table used by
`field_receive_item` and `field_get_item_count`. Chest settings, inventory-item
drops, and literal inventory commands show names alongside their IDs. Known
US dictionary tokens are expanded (for example, `Plat{16}um` becomes
`Platinum`); unknown text codes remain visible in braces. IDs outside
`0x00..0xFE` are marked invalid for inventory operations, even though the text
table contains an entry at `0xFF`. Template reward names stay attached to their
scene-local reward records and retain first-match lookup behavior.

The YAML keeps name bytes, name offsets, directory-pointer offsets, and the
reference binary's path, region and SHA-256. **FIELD offsets refer to the
uncompressed binary including its leading header byte**, matching the splat
config; they are not seek positions in the compressed disc file. The
uncompressed SHA-256 is included too. JP reports also identify the CLOAD chart
source. Chest item and collection-setting offsets, drop handler/value pair
offsets, and template reward name offsets refer directly to the original IMG.
Names are annotations; raw layout/resource bytes still drive reconstruction.

`drop_rates.py` follows `field_roll_defeat_drop`. The level table is indexed by
`level >> 4`. Its value names the final **fallback slot**. Earlier slots are
selected by the first set bit of a random mask, and the fallback receives the
remaining rolls. For the original level 1-15 table value of 3, slots 0-3 have
chances of 50%, 25%, 12.5%, and 12.5%; slots 4-7 cannot be selected.

The report calculates four cases:

- `normal`: no drop-selection modifiers.
- `extra_slots`: defeat flag `0x04000000` adds two to the fallback index, capped
  at slot 7.
- `no_common`: defeat flag `0x08000000` clears random-mask bits 0 and 1 before
  the roll.
- `both`: applies both modifications.

Each case covers levels 1-99. YAML stores exact weights out of 128, fractions,
and percentages. Repeated handler/value choices are combined, while every
original slot and offset remains available. Text shows combined chances with
no defeat modifiers next to each choice, followed by tables for all four
cases. For example, Stinger Bug's Bug Meat (`0x0086`) and Clear Feather
(`0x00AA`) each have a 12.5% selection chance at levels 1-99 without modifiers.

These are conditional **slot-selection** odds, assuming uniform random low
bits. The report does not infer the current monster level or defeat flags,
evaluate script participation, or guarantee successful item collection.
Restore drops retain their separate half-restore chance out of 256, conditional
on selecting that slot. Other defeat flags can change experience/money pickup
counts without changing the selected slot.

Sources:
[field_stat_counter_ops.c](../../../../src/overlays/field/field_stat_counter_ops.c),
[field_reward_command_ops.c](../../../../src/overlays/field/field_reward_command_ops.c),
and [field_state_ops.h](../../../../include/field_state_ops.h).

## Actor descriptions and actor scripts

The main extractor now writes readable YAML beside both sections' original
binary files. You can also inspect either section directly from a scene IMG:

```sh
python3 -m tools.scenes.actors /path/to/scene.IMG
python3 -m tools.scenes.actor_scripts /path/to/scene.IMG
```

These commands print YAML. To browse actor scripts as a compact text listing:

```sh
python3 -m tools.scenes.actor_scripts /path/to/scene.IMG --format text
make inspect-scene-actor-scripts SCENE=/path/to/scene.IMG SCENE_SCRIPT_FORMAT=text
```

For example, each instruction lists its original IMG offset, bytes and operands:

```text
00000504  A3                          set_unknown_10
00000505  81 01                       step animation_state=1
00000507  A2                          clear_unknown_10
00000508  FF                          end
00000509  [END]
```

The directory maps every entry to a distinct script, including aliases. Each
script heading lists its referring entries. `FALLTHROUGH`, `UNRESOLVED` and
`STOP` markers identify boundaries that need attention; remaining bytes are
shown with offsets. `[END]` marks the position after the END instruction.
Full exports include this listing as `actor_scripts/data.txt`.

Add `-o actors.yaml`, `-o actor_scripts.yaml`, or `--format text -o actor_scripts.txt` to
write a new file; an existing output is refused. They take the original IMG
so offsets and image references can be resolved in context. Equivalent Make
targets are `inspect-scene-actors` and `inspect-scene-actor-scripts`, both using
`SCENE=/path/to/scene.IMG`.

`actors/data.yaml` contains the scene's volume, party palette, map ID and
parts-timer flag, followed by the actor description directory. Each actor
exposes its original image slots, palette, animation flags and action table.
Image references identify both their absolute IMG offset and the exported TIM.
An empty image list means the loader inherits the previous resource's images.
Shared descriptions retain their separate actor indices and common offset.

Action records show the command, target filter and group, instrument flag,
animation and parameter. These are resource action slots, not scene-script
references. Their parameters remain literal values because the runtime gives
them different meanings depending on the command. Original record bytes and
unnamed flag bits remain available.

`actor_scripts/data.yaml` contains two lists: `entries` preserves every original
u16 directory slot, while `scripts` decodes each distinct referenced range once.
Instructions show their absolute IMG offset, opcode, command name, named
operands and original bytes. The command names and sizes follow
`field_run_actor_script_command` in
[field_actor_script_ops.c](../../../../src/overlays/field/field_actor_script_ops.c).
This is a different interpreter from the event scripts and animation resources.

Decoding stops at `end`, an unknown opcode, truncated operands or the next
script-directory boundary. `stop_reason`, `stop_offset` and any nonempty `remaining_bytes`
show exactly where it stopped. A range that runs into the next entry gets
`continues_at_script`; the reader does not invent an END instruction. Bytes
after END remain raw, including padding.

An entry pointing to the section end is marked `empty`. References into the
directory or beyond the section are marked `unresolved`. Two entries in the
original US scenes point into their own directories; they are preserved rather
than decoded as instructions. Invalid image references are likewise shown as
unresolved. These readers describe stored data, not a simulation of gameplay.

## Event scripts

Inspect the event section directly, or open `event_scripts/data.txt` in a full
export:

```sh
python3 -m tools.scenes.event_scripts /path/to/scene.IMG --format text
python3 -m tools.scenes.event_scripts /path/to/scene.IMG -o events.yaml
make inspect-scene-event-scripts SCENE=/path/to/scene.IMG SCENE_EVENT_FORMAT=text
```

YAML is the default. Both formats accept `-o` and refuse to overwrite a file.
The Make target defaults to YAML; `SCENE_EVENT_FORMAT=text` selects the listing.
The original section now lives at `event_scripts/data.bin`; the byte map points
to this file and its two descriptive companions.

The formats and command names come from
[field_script_ops.c](../../../../src/overlays/field/field_script_ops.c),
[field_script_operands.c](../../../../src/overlays/field/field_script_operands.c)
and [field_script_commands.c](../../../../src/overlays/field/field_script_commands.c).
Basic commands use fixed fields or two-bit operand descriptors. Pair commands
(`0x40`-`0x5F`) and extended commands (`0x80`-`0x8F`) use four-bit descriptors,
including implicit constants and variable reads. Descriptor bytes and the
original instruction bytes remain in the YAML.

The export contains:

- `entries`: every u16 directory slot, including aliases, raw values that do
  not address code, and a possible section-end entry marked `empty`. Entry zero
  gives the first script offset and the boundary used for this directory.
- `instructions`: one record per decoded instruction address, in file order.
  Instructions have named operands, original bytes and `successors` describing
  fallthrough, branches, calls, returns or scene changes.
- `variables`: the distinct reference addresses, decoded into kind, scope,
  word index, bit shift and whether the owner's local base applies. Gameplay
  meanings and runtime values are not inferred.
- `undecoded`: every remaining span after the directory, with its offset,
  size and raw bytes. These can be data, padding or unsupported instructions.
- `diagnostics`: unsupported or truncated instructions, targets outside the
  code area, and entry points that conflict with an existing instruction.

Valid directory offsets seed the reader's control-flow walk. It follows both
outcomes of a conditional branch and both the target and continuation of a
static call. A return ends that path, but other entries or branches can reach
later bytes. Shared code and loops are decoded once. Directory slots can also
contain values that are not usable code addresses; preserving them does not
establish that they are callable scripts.

Branch deltas are signed and relative to the opcode address, not the following
instruction. For `switch`, each delta is relative to its own case entry. A zero
delta means return. The reader lists all switch cases through the `0xFF` default.
An indexed jump (`0x0E`) has no stored table count; it shows the table address
and unresolved runtime dependency instead of guessing the table length.
Variable-selected event calls (`0x10`) retain their reference and unresolved
target; the reader also decodes the caller's continuation.

The text listing uses `L_<IMG offset>` labels for directory entries and static
branch/call targets. `var[0xE040]` reads a variable's value; `&var[0xE040]` names
its reference address. Literal `255` is labeled `owner` only for arguments whose
handler gives it that meaning. Commands that write a reference above `0xFFFF`
show the keep-top-bit behavior. Operands left unassigned by the interpreter
remain explicitly unresolved.

For example, the common chest initializer in `WAL_B020.IMG` begins with a
`set_variable` followed by two `read_record_bits` instructions. Those reads
expose actor-record halfword indices 8 and 9 and their destination variables.
The command listing lets a reader follow those operations without assigning
meanings to unrelated layout parameter slots.

Field command `0x03` includes a command name such as `spawn_monster`; its
arguments live in the runtime parameter block. Miscellaneous command `0x44`
includes a subcommand name when that number is literal. The tool does not
simulate those commands, evaluate conditions, resolve variable-selected calls,
or infer later changes to layout-to-species associations. The object report
separately links initial layout selectors to local templates. In the original US corpus, 27 candidate
targets overlap decoded instructions; these remain visible as diagnostics.

The original bytes remain authoritative. Editing YAML or the text listing does
not assemble new event bytecode. The directory bytes, decoded instruction bytes
and undecoded spans together account for the entire event section.


## Strings and actor animation geometry

```sh
make inspect-scene-strings SCENE=/path/to/scene.IMG
make inspect-scene-geometry SCENE=/path/to/scene.IMG
make inspect-scene-resources SCENE=/path/to/scene.IMG
```

These commands print readable TXT by default. Set `SCENE_DATA_FORMAT=yaml`
for structured output. Their Python modules use the same `source`,
`--format yaml|text`, and `-o new-file` convention as the script readers:

```sh
python3 -m tools.scenes.strings scene.IMG --format text
python3 -m tools.scenes.geometry scene.IMG --format yaml -o geometry.yaml
python3 -m tools.scenes.scene_resources scene.IMG --format text
```

String directory entries contain little-endian u16 offsets relative to the
text table. The first offset gives its directory size. Shared offsets retain
all directory slots; an entry can also point into another string. `strings`
contains each distinct pointed-to text, with its absolute IMG offset,
encoded bytes, tokens and readable `text`. `entries` links slots to those texts.

The US dialect expands the existing compression dictionary. `{macro:0}`,
`{wait_clear}`, `{color:2}` and similar labels expose text controls; they are
not substituted with runtime values. A newline control prints a line break.
Unknown glyphs remain byte tokens such as `{19 05}`. Tokens distinguish literal
characters from controls and retain their original bytes. End (`0x00`) and
finish (`0x06`) both stop an entry. Zero bytes used as control or glyph
arguments do not stop it. Unreferenced bytes, including alignment padding,
appear under `undecoded`.

`--text-encoding us|jp` chooses the control/glyph dialect in the Python
extractor, string reader and resource reader; the default is `us`. JP text
retains two-byte glyph codes rather than loading a font chart from another
file. This option is separate from external FIELD reference selection.

The section named `geometry` holds actor animation resources. The loader uses
`actor_count + 1` u32 boundaries; adjacent values delimit the resource for the
actor description with that index. Resource indices begin at 3. The section
contains animation and frame directories, sequences of timed frame entries,
and frame records. It is not the separate map geometry loaded through
`field_load_map`.

Animation pointer bit 15 selects four-byte entries `(frame, duration, height,
motion)`; other sequences use `(frame, duration)`. The resource mode selects
8-bit or 16-bit sprite coordinates. Sprite records expose local x/y, texture
u/v, width/height, CLUT and texture-page columns, mirror bits, tilt, flags and
original bytes. Placement commands expose their opcode and raw operands;
those operands are not yet given a complete field interpretation. No animation
is played or rendered. Aliases retain their original pointers, and unused
bytes remain visible.

## Other scene resource payloads

`records/<slot>.txt` describes the same stored resource as its YAML companion.
The resource `kind` and `code_reference` identify the reader used to establish
its format. All offsets locate the original IMG, except directory-relative
values and generation script offsets, whose pointer bases are explicit.

| ID | Stored payload exported |
| --- | --- |
| `0x0001` | Battle templates, handler-specific action descriptors, drops and keyed reward items |
| `0x0003` | Guest IDs and four character template banks; padding remains in the binary |
| `0x0004` | Weapon/armor generation tables and preserved item script bytes |
| `0x0005` | Counted item templates, including records with empty names |
| `0x0006` | Counted trigger bounds and script/battle commands |
| `0x0007` | Counted companion templates |
| `0x0008`, `0x0009`, `0x000C` | Item names, coordinate labels and text tables |
| `0x000A` | Shop offset directory, shared lists and packed item/price entries |
| `0x000B` | Effect picks and effect codes |
| `0x000D`, `0x000E` | Weapon/armor template arrays indexed directly by the game |
| `0x000F` | Instrument grid, material pairs, secondary scripts and preserved script bytes |
| `0x0010`, `0x0011`, `0x0012` | Effect thresholds, item values and region effect rows |
| `0x0013` | Packed nibble table cells, with the first column identified |
| `0x0100`, `0x0101`, `0x0103`, `0x0104` | Golem names and menu text tables |
| `0x0102` | Two stored values per menu object |

Weapon and armor arrays use the stored payload size for `record_count`:
their consumers index records directly, and the original header high word is
zero. Counted tables keep their actual count word. The debug scene
`DMY_DEB0.IMG` declares 65 companion records but stores only 64; its resource
is retained as binary with a diagnostic rather than inventing the missing
record. Unknown resource IDs also remain binary. These limitations are
reported in the scene coverage map.


## Monster templates and drops

The records section starts with section-relative u32 offsets. The first offset
also gives the directory's byte size. Each resource starts with a u16 ID;
resource 1 contains the battle tables. Unknown IDs retain their binary data and
basic metadata.

A battle resource's YAML contains:

- `monsters`: template IDs, HP and stat growth, elemental masks, flags, drop
  slots and decoded eight-byte battle action descriptors. `index` is the template directory index;
  `id` is the identifier the runtime looks up.
- `rewards`: keyed 64-byte item records, with an ASCII name preview when possible
  decoded `FieldItemRecord` fields and the complete item bytes.
- Absolute IMG offsets for the tables and individual records. Unnamed fields
  and trailing bytes remain visible rather than receiving guessed meanings.

Each monster has eight drop slots. `handler` and `value` are the stored bytes;
`kind` describes the handler. Handler 0 encodes experience and money pickup
counts; handler 1 supplies the half-restore chance out of 256; handler 2 holds
an inventory item ID. Handler 3 uses `monster_id * 16 + value` as a reward key.
Its `reward_indices` lists matching reward rows; an empty list means the key
was not found. The runtime uses the first match when keys repeat.

The slots are not equally likely. FIELD selects them according to monster
level, a random mask and defeat modifiers. This export describes the stored
choices. The object report adds conditional probabilities from the selected
FIELD reference, without inferring the monster's current battle level.
See [field_reward_command_ops.c](../../../../src/overlays/field/field_reward_command_ops.c).

`name_ascii` is a preview only. It is null when a name contains non-ASCII game
codes; the encoded bytes are preserved. This does not require guessing the
region's text encoding. Each action now has its original `bytes`, `info` and
`params` words, split flags, power, handler, and `parameter_fields` selected
from `FieldActionParams`. Unknown handlers retain the full parameter word.

For example, `ANA/INFO_PRT/LAK_B050.IMG` contains Stinger Bug, Spiny Cone and
Tonpole templates. Use `records/directory.yaml` to find its resource file;
the resource ID is stored in that file's YAML. The object report links initial placed-actor selectors to these template IDs.
Later script-driven changes still require following the runtime behavior.

## Chest records

The layout section starts with a u32 count followed by 48-byte
[`FieldLayoutRecord`](../../../../include/field_interaction_start.h) records.
A chest uses the same record structure as other actors. The extractor recognizes
the common chest resource selector and initializer script before labeling a
record as `chest`.

Its YAML file shows every layout field, including `condition.variable_ref` and
its minimum and maximum, decoded control and position fields, the resource
selector, enabled events, and all 16 script/parameter slots. It also shows the
chest settings `x`, `z`, `item_id`, `collection_variable_ref` and `alternate_facing`.
The item comes from `scripts[4]`. The collection reference uses bits 0-14 of
`scripts[5]`, and bit 15 selects the alternate facing. `record_bytes` keeps the
complete original record as hex.
The decoded fields describe that saved record; editing them does not change
`record_bytes`.

For example, `WAL_B020.IMG` has a chest at `(395, 180)` with item `0x96` and
collection-variable reference `0x0BC0`. It is exported as `chests/002.yaml` because it is layout
record 2. Event scripts are decoded under `event_scripts/`; the tool does not
execute them. Item names appear in `objects.txt` and `objects.yaml` when a FIELD
reference is selected. Records using other initializers still get
generic layout YAML, without the chest-specific parameter interpretation.

## Supported format

The header contains ten little-endian u32 offsets: layout, event scripts,
strings, actor scripts, records, actors, geometry, images, portraits and group
bounds. The first section starts at byte 40. Offsets must be aligned, in order
and within the file.

The images section has an offset table followed by TIMs, checked with the
existing TIM parser. The portraits section has a count followed by 1,184-byte
records: a 32-byte palette and 48-by-48 pixels at 4 bits per pixel. Portraits
aren't TIMs.

This supports the scene IMG layout, not every IMG family or compressed file.
See the [scene loader](../../../../src/overlays/field/field_scene_transition.c)
and [portrait representation](../../../../src/overlays/field/field_text.c) for
the code that reads these resources.

The Python code follows these structures: `SceneHeader` documents the header
fields (in `scene_format.py`) and `FieldLayoutRecord` documents the layout record (in `layout.py`). `read_layout()`,
`read_resources()`, `read_textures()`, `read_portraits()` and `read_group_bounds()`
handle container assets. `strings.py` and `geometry.py` decode text and actor
animation data; `resource_payloads.py` handles typed resource contents.
`scene_resources.py` owns the resource and battle
table parsers. `actors.py`, `actor_scripts.py` and `event_scripts.py` own their
section readers and standalone commands. `event_opcodes.py` holds the event
command layouts; `section_cli.py` shares their command-line output handling
and `presentation.py` formats the descriptive YAML. `scene_report.py` builds
object associations; `script_links.py` follows static paths and describes selected
gameplay operations.
`AssetRange` describes an exported byte range, not a disk structure.

Run the synthetic format and failure-handling tests with `make test-scene-tools`.
They are also included in `make test-tools`.
