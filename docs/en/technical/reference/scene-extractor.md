# FIELD scene extractor

[English documentation](../../README.md) | [Scene tools](../../../../tools/scenes/README.md)

The scene extractor reads the original game's `ANA/INFO_*/*.IMG` files. These
are Square scene containers, holding actor layouts, scripts, text, geometry and
textures. This tool operates on a file already extracted from the disc; it is
not a disc-image extractor or graphical scene viewer.

Run from the repository root with Python 3.10 or newer. No extra packages or
compiled game binaries are needed.

```sh
python3 -m tools.scenes.field_scene info /path/to/ANA/INFO_PRT/WAL_B020.IMG
python3 -m tools.scenes.field_scene info /path/to/ANA/INFO_PRT/WAL_B020.IMG --json
python3 -m tools.scenes.field_scene extract /path/to/ANA/INFO_PRT/WAL_B020.IMG output/scenes/WAL_B020
python3 -m tools.scenes.field_scene validate /path/to/ANA/INFO_PRT/*.IMG
```

The input is never modified. Extraction requires a new destination directory;
it refuses existing paths. Keep extracted assets in the ignored `output/` tree
or outside the repository.

## Output

| File | Contents |
| --- | --- |
| `summary.txt` | Section boundaries, layout/texture counts and chest candidates |
| `manifest.json` | Versioned metadata, source hash, section hashes, layout fields and TIM metadata |
| `header.bin` | Original 40-byte section-offset header |
| `sections/00_layout.bin` through `sections/09_group_bounds.bin` | All ten complete sections in original order |
| `textures/000.tim`, etc. | Individual TIMs in image-table index order |

The header followed by all ten section files reconstructs the input exactly.
Texture files are additional copies; they do not replace the raw image section.
Manifest offsets are absolute byte offsets into the source file, numeric values
are decimal, and output paths are relative to the extraction directory.
`info --json` describes the same paths without writing them.

The textures are sprite sheets. Rendering an assembled scene requires sprite
geometry, animation state and separate background resources. The common chest's
closed/open images also live in FIELD's shared resources, outside the scene IMG.

## Decoded fields

`layout_entries` contains all `FieldActionRequest` records, including actions
that are not visible actors. Raw control, position and source values are retained
alongside the understood bit fields, installation condition, event mask and
sixteen `scripts_and_parameters` values. The `active` flag describes the stored
bit; it does not evaluate the condition against a save.

Actor/action kinds 0 and 7 with source-selector low bits equal to 5 are marked
as chest graphics candidates. Only a recognized initialization-event prefix
produces `common_chest` metadata:

- Entry `[4]` supplies the item ID.
- Bits 0-14 of `[5]` supply the collection-variable reference.
- Bit 15 of `[5]` selects alternate facing.

Other candidates retain `common_chest: null`. Recognition is not script execution
or proof that the chest can be reached. Item names are not resolved, keeping the
tool independent of regional FIELD binaries.

## Format boundaries

The header is ten little-endian u32 offsets: layout, event scripts, strings,
actor scripts, records, actor descriptions, geometry, images, portraits and
group bounds. The first offset is 40 (`0x28`), not a magic identifier.

The parser validates section ordering and bounds, fixed-size record counts, image
offsets and TIM payloads. Layout entries are 48 bytes. Portrait records are
1,184 bytes: a 32-byte palette and 48-by-48 pixels at 4 bits per pixel. Portraits
are preserved in their raw section; they are not TIMs. Original TIM flag words,
including any nonzero upper bytes, are preserved.

Scripts, strings, general records, actor descriptions and geometry are not fully
decoded or semantically validated. Event-script offsets are not assumed to be
sorted, and the first script offset is not a table count. Text is not assumed to
be ASCII. Other IMG families and compressed files are outside the supported scope.

The extractor has been checked against 847 US and 847 Japanese scenes, including
byte-exact section reconstruction and TIM round trips. Those sets contain 1,727
and 1,725 TIMs respectively. Each has 161 common chest records and seven additional
chest graphics candidates; these are not counts of distinct obtainable chests.

```sh
make test-scenes
```

## Evidence and related research

This implementation follows the [scene loader](../../../../src/overlays/field/field_scene_transition.c),
[layout records](../../../../include/field_interaction_start.h),
[event lookup](../../../../src/overlays/field/field_actor_key_ops.c) and
[portrait representation](../../../../src/overlays/field/field_text.c).