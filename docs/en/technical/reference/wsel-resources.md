# WSEL resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/overlays/README.md) | [Japanese](../../../jp/technical/reference/wsel-resources.md)

WSEL is the "Select play area" screen. It opens on the world map with the hero
standing in front of it. Confirm zooms into the map and shows the prompt until
you press confirm again. Then the parchment land map fades in, and you move a
96-pixel square around a 19 x 19 grid to pick a cell. Darkened cells
are refused with an error sound. A second confirm writes the cell index to the
saved game's `world_map_cell` and leaves the overlay.

Everything the screen draws comes from eight TIM images embedded in the
overlay, plus a few small tables. Both regional YAMLs keep all of it in one
`wsel_data` databin. The build links those bytes unchanged, and a separate
extractor makes them readable.

```sh
make splat
make extract-wsel

make splat VERSION=jp
make extract-wsel VERSION=jp
```

Files go to `assets/exports/<version>/overlays/wsel/`. Set
`WSEL_OUTPUT=/path/to/new-folder` to use another destination. The folder must
be new. The extractor reads everything first, then writes a temporary folder
and moves it into place when it's finished. A failed write removes that folder.

## What comes out

| Path | Contents |
| --- | --- |
| `byte-map.yaml` | Every range in the data blob, including the repeated TIM words, padding and screen state |
| `images/<layer>/image.tim` | The original TIM, one per sprite layer |
| `images/<layer>/image.yaml` | Stored TIM layout, the sprite layer it belongs to and where the game uploads it |
| `images/<layer>/palette_00.png` | The whole image through its palette |
| `images/hero_default/poses/`, `images/hero_alternate/poses/` | The 12 pose cells of each hero sheet, cropped |
| `tables/sprite_layers.yaml` | The eight sprite layer records |
| `tables/hero_poses_default.yaml`, `tables/hero_poses_alternate.yaml` | 12 pose records for each hero sheet |
| `tables/cell_occupied.yaml` | One flag per grid cell; nonzero cells can't be chosen |
| `tables/cell_tile_masks.yaml` | A 6 x 6 tile mask for each of the 361 grid cells |

Each YAML table keeps its original bytes next to the decoded values. US and JP
are identical except for the prompt image, which is in English or Japanese.

## The images

`wsel_load_resources` uploads TIM n to sprite layer n. The layer names in the
export come from the `WSEL_SPRITE_*` defines in `wsel.c`.

| Folder | Size | Depth | What it is |
| --- | --- | --- | --- |
| `land_map` | 416 x 416 | 8-bit | The parchment land map you move the square over |
| `world_map` | 320 x 224 | 8-bit | The framed world map and its stone background |
| `cursor` | 120 x 118 | 4-bit | The yellow frame around the 96-pixel square |
| `world_overlay` | 320 x 224 | 4-bit | A dark vignette, drawn subtractively over the world map |
| `hero_default` | 256 x 256 | 8-bit | One hero's pose sheet |
| `hero_alternate` | 256 x 256 | 8-bit | The other hero's pose sheet |
| `hero_shadow` | 256 x 256 | 4-bit | Holds the 32 x 10 drop shadow drawn under the hero |
| `prompt` | 184 x 88 | 4-bit | "Select play area. Darkened areas cannot be chosen." |

Each TIM has one palette. The previews show value zero as transparent and every
other color opaque; the game's blending and brightness aren't applied, so the
vignette just looks like a dark picture.

The game ignores the VRAM coordinates stored in each TIM. `wsel_upload_tim`
uses the layer record instead: pixels go to the layer's `tpage_x`/`tpage_y`,
and the palette goes to `clut_x`/`clut_y` as a single row. `image.yaml` lists
both, as `stored_layout` and `uploaded_to`. If you replace an image, it's the
layer record that decides where it lands.

Each TIM is followed by a copy of its last four bytes. The byte map lists those
words; they aren't part of the exported TIM.

## Sprite layers and hero poses

A layer record is 24 bytes: `tpage_mode` (0 is 4-bit, 1 is 8-bit),
`blend_mode`, `semi_trans` and `brightness` as bytes, then ten 16-bit values:
texture page X and Y, palette X and Y, source U and V, width, height and screen
X and Y. Brightness 128 is neutral. The layer drawer cuts a layer into strips
of up to 128 x 256 pixels, one texture page each. Layer 2, the cursor, also
gets a black shadow two pixels down and to the right.

The screen changes a few of these fields while it runs. It fades layers 0 and
2 in by stepping their brightness and blend settings, and it moves them with
the scroll and cursor. The stored values are the starting state.

Layers 4 to 6 aren't drawn as plain sprites, so their width and height are 0.
`wsel_draw_hero` draws the hero instead. It picks the default sheet when the
low seven bits of the saved-game byte at `0x800435E0` (US) or `0x80043748`
(JP) are zero, and the alternate sheet otherwise. A pose record is six bytes,
all in 8-pixel units: U, V, width, height, X offset and Y offset. The pose's
top-left corner goes to the layer's X + 32 - X offset x 8, Y + 40 - Y offset x 8.
With the stored layer position (216, 180), pose 0 is a 32 x 48 cell drawn at
(216, 172). The shadow is the fixed cell at (184, 6) in the shadow sheet, drawn
at the layer's X, Y + 32.

Only pose 0, the standing pose, is ever drawn. The other eleven poses each hold
a weapon.

## The land-map grid

The land map is 26 cells of 16 pixels on each side. The cursor square covers
6 x 6 cells, and its top-left corner can sit on any of 19 x 19 cells, starting
one cell in from the border. The cell under the cursor is
`(cursor - scroll - 16) / 16` on each axis, and the chosen cell is stored as
`column + row x 19`.

`cell_occupied` has one byte per cell, row by row, followed by three zero bytes
of padding. A nonzero cell is darkened as a whole and refused with an error
sound. The export also writes the grid as 19 strings of `0` and `1`, so it
reads like a picture of the map.

`cell_tile_masks` has 36 bytes per cell: a 6 x 6 mask for the square that
starts there. While Square is held on a free cell, the screen brightens every
tile stored as 0. It also draws one extra row and column past the square, taken
from the last row of the cell below and the last column of the cell to the
right.

## Runtime state and the byte map

The data blob is 492,252 bytes in both versions. The last 16,500 bytes are zero
on disc. They hold the render-buffer pointer, the buffer that receives the
music sequence, and the fade, input, scroll and cursor variables. The byte map
records this area without writing another file. Every other byte belongs to a
resource, a repeated TIM word or the three padding bytes, so neither version
needs an `unknown/` folder. Unrecognized nonzero data would still be saved there
unchanged.

[`wsel.py`](../../../../tools/overlays/wsel.py) follows the same reader, `Part`
and writer structure as the other overlay extractors. Editing these exports
doesn't change what the build links.

## What we don't know yet

- TODO: what the saved-game byte at `0x800435E0` is. WSEL only uses its low
  seven bits to choose between the two hero sheets, so which value means which
  hero isn't confirmed.
- TODO: what the tile masks describe. They aren't slices of one shared map,
  since overlapping squares disagree about the same tile. The code only uses
  them for the Square highlight.
- TODO: why the shadow sheet also holds Japanese menu text, in both versions:
  a character prompt, a starting-weapon prompt and eleven weapon names. WSEL
  only reads the shadow cell. The eleven weapon poses in each hero sheet seem
  to match that list, but nothing in WSEL draws them either.
