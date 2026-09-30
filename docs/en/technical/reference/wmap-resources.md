# WMAP resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/overlays/README.md) | [Japanese](../../../jp/technical/reference/wmap-resources.md)

WMAP is the world map: placing artifacts, the artifact carousel, travel
between lands, the map game and the animated land effects. Both regional YAMLs keep its initialized data in one
`wmap_data` databin that runs from the end of the code to the end of the
overlay. The build links the original bytes, and a separate extractor turns
them into files you can read.

```sh
make splat
make extract-wmap

make splat VERSION=jp
make extract-wmap VERSION=jp
```

Files go to `assets/exports/<version>/overlays/wmap/`. Set
`WMAP_OUTPUT=/path/to/new-folder` to choose another destination. The folder
must be new. The extractor reads everything before writing, then moves its
temporary output into place. A failed write removes the temporary folder.

## What comes out

| Path | Contents |
| --- | --- |
| `byte-map.yaml` | Every range of the data blob, with the file it went to |
| `tables/*.yaml` | 56 fixed tables: sprite and primitive templates, map game rounds, the information panel, land texture slots, artifact images and transfer paths, land attributes, map labels and the backdrop geometry |
| `sounds/sound_01.akao` through `sound_63.akao` | The 63 original AKAO sound-effect buffers |
| `sounds/sounds.yaml` | Sound ids, sizes and the entries of each buffer |
| `scripts/input_scripts.yaml` | The three scripted-input sequences, decoded, with their original bytes |
| `handlers/*.yaml` | 453 step tables from 39 source files, each entry named after its function |
| `unknown/` | One 136-byte gap nothing names yet, saved unchanged |

Every table YAML has the symbol, the record format (Python `struct` syntax,
little-endian), the row count, the decoded rows, a short note about what the
code does with it, and the original bytes. Field names come from the C types
that read the table. Tables still called `D_<address>` in the source keep that
name in the file name too. The address in the name is the US one; the JP
symbol file uses the same names at JP addresses.

Most of the blob isn't data at all. The first 79,672 bytes in US (79,668 in
JP) are the tables below. The other 898,584 bytes in US (898,576 in JP) are
runtime variables and buffers, all zero on disc. The byte map lists them as
one range and nothing is written for them.

## Where things are

Offsets are from the start of the blob. They're the same in both regions,
because the JP data only moves as a whole: US starts at `0x800C4588`, JP at
`0x800C465C`.

| Offset | Symbol | What it is |
| --- | --- | --- |
| `0x0` | `g_wmap_game_continue_prompt` and six more | `SPRT` templates for the map game's prompts, score and countdown |
| `0x8C` | `g_wmap_game_spawn_patterns` | Which of the game's nine lands can rise, for each of eight rounds |
| `0xD4` | (sound buffers) | The 63 AKAO sound-effect buffers |
| `0x6C74` | `g_wmap_sfx_buffers` | One pointer per sound effect |
| `0x6D70` | `g_wmap_information_glyphs` | Information panel glyphs, placements and groups |
| `0x7BDC` | `g_wmap_artifact_images` | 64 artifact texture rectangles and drawing offsets |
| `0x8770` | `g_wmap_artifact_pickup_frames` | 740 frames of artifact pickup paths, 280 return frames follow |
| `0xB844` | `g_wmap_land_attributes` | 64 artifacts: eight spirit strengths and a flags word each |
| `0xBCCC` | `D_800D0254` | Land name label widths; the only table that differs between regions |
| `0xBFBC` | `g_wmap_input_scripts` | Pointers to the three input scripts just before it |
| `0xC170` | `g_wmap_menu_triangles` | 28 Gouraud triangles of the menu cursor |
| `0xC4E4` | `g_wmap_sprite_textures` | 48 sprite texture pages and palettes |
| `0xCA4C` | `D_800D0FD4` | Backdrop triangle vertices, triangles and point buffers |
| `0x106CC` | `D_800D4C54` onward | Step tables for the land effects and events |

The primitive templates are complete libgpu packets (`SPRT`, `POLY_FT4`,
`POLY_G4`, and Gouraud triangles), copied into the packet buffer and then
patched. Their tag word is exported too, so the YAML shows exactly what's
stored. Colors are the template colors; the code fades and tints several of
them at runtime.

## Sound effects

`g_wmap_sfx_buffers` has 63 pointers. Sound id N plays the buffer in entry
N - 1 (see `wmap_resource_support.c`), so the files are numbered by sound id.
Each buffer is an AKAO sound-effect list, as `akao_sfx_play_list` reads it:

| Offset | Size | Contents |
| --- | ---: | --- |
| `0x0` | 4 | `AKAO` |
| `0x4` | 4 | Entry count |
| `0x8` | 4 | Bank program key, looked up with `akao_bank_find_slot` |
| `0x10` | 4 per entry | Entry offsets, relative to `0x20` |
| `0x20` | | Entries and their sequence data |

Every entry has two u16 offsets, one per channel sequence. `0xFFFF` means that
channel has no sequence. `sounds.yaml` lists each entry and resolves both
offsets to positions in the buffer. The sequence bytes stay in the `.akao`
file; this is not a sequencer. All 63 buffers are identical in US and JP.

## Scripted input

WMAP can drive itself with a script of fake controller input. Pending land
events 0, 1 and 28 start scripts 1, 2 and 3 when the map opens (see
`wmap_main.c`). Each script is a run of s16 words read by
`wmap_step_input_script`:

| Words | Meaning |
| --- | --- |
| `N buttons` | Hold `buttons` for N frames (N > 0) |
| `0 0` | Wait for any real button press |
| `0 mask` | With bit `0x800` set in `mask`, wait for one of the other masked buttons; any other value stops the script |
| `-2 command [argument]` | Run a command |
| `-1` | End of the script |

Command names come from the `WMAP_SCRIPT_` enum in the source: `IMAGE`,
`BUTTON_MASK` and `VISIBILITY` take one argument word; the `WAIT_` commands and
`FIND_HOME` don't. `IMAGE` reads a CD file by id (for example 4435, `0x1153`)
into VRAM and shows it as the prompt image; an argument of zero or less hides
it. The US and JP scripts differ only in their button words: the scripted
confirm press is cross (`0x0040`) in US and circle (`0x0020`) in JP, the same
swap `wmap_main.h` makes for `WMAP_PAD_CONFIRM`.

## Step tables

Most of WMAP's land effects and events run as step tables: arrays of function
pointers that a sequence walks one entry per step. The extractor finds them
by their declarations in `src/overlays/wmap` (`void (*name[])(void)` or
`WmapHandler name[]`), so each file under `handlers/` matches one C file.
Every entry is written as the function's name. Most are still `func_<address>`
in the source. Those names are the US addresses; JP uses the same names.

A null entry is a zero word. The last US table,
`g_wmap_effect35_emitter_b_steps`, has one extra null word before the
runtime variables start; JP doesn't. One entry of `D_800D4DB4` in land effect
18 points into WMAP's own data (`D_800D9478`) instead of code. It's kept as
`{data: D_800D9478}` rather than hidden.

## What stays in the byte map

| Region | Data blob | Zero-filled runtime area | Unknown | Padding |
| --- | ---: | ---: | ---: | ---: |
| US | 978,256 bytes | 898,584 bytes | 136 bytes in one file | 8 bytes |
| JP | 978,244 bytes | 898,576 bytes | 136 bytes in one file | 8 bytes |

The byte map covers every byte, and counts each range once. The padding is the
zero bytes after the input scripts. Tables without a fixed C array bound
cover the stored run up to the next resource, so a few end with alignment
bytes, such as the three after `g_wmap_spirit_sequence_bounds`.

[`wmap.py`](../../../../tools/overlays/wmap.py) reads the resources and writes
the exports. [`wmap_tables.py`](../../../../tools/overlays/wmap_tables.py) holds
the table layouts. The rodata before the code, including the two 4-byte
alignment blobs, is unchanged. Editing an export doesn't change what
the build links.

## What we don't know yet

- TODO: the 136 bytes right after `g_wmap_carousel_rotation` (`unknown/800CC77C.bin`
  in US, `800CC850.bin` in JP) are 34 s32 values between 4362 and 4670, with -1
  in between. That's the same range as the CD file id in the first input
  script, but no WMAP code names these bytes.
- TODO: the AKAO sequence bytes aren't decoded. The sound driver's opcodes live
  in `akao_sequencer.c`.
- TODO: `g_wmap_land_attributes` flags: the placement rules read single bits,
  but most bits have no names yet.
- TODO: `g_wmap_sprite_textures` has an 8-byte header and a trailing u16 that
  the renderer never reads (`unknown_00` to `unknown_07`, `unknown_1a`).
- TODO: several tables are still `D_<address>`, and many step functions are
  still `func_<address>`. Their YAML notes say what the code does with them;
  renaming the C symbols renames the exports.
