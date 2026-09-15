# Disc Layout

This document describes the contents of the files on the disc, and how the game addresses data during the runtime.
Right now, it's only written to describe SLUS-01013 since that is the NTSC-U version of the game.

## Disc Identity

| Item | Value |
|---|---|
| Product code | SLUS-01013 (US) |
| Boot config | `SYSTEM.CNF` |
| Boot executable | `cdrom:\SLUS_010.13;1` |
| Main EXE size | ~194 KB (`SLUS_010.13`) |
| Extracted total | ~563 MB, 5036 files |

`SYSTEM.CNF`:

```
BOOT = cdrom:\SLUS_010.13;1
TCB = 4
EVENT = 16
STACK = 801FFFF0
```

`SLUS_010.13` is the main executable and has the data for shared libraries like Psy-Q.
It orchestrates the loading of overlays from `BIN/` into fixed addresses depending on the game mode.

## Runtime Addressing Model

The ISO directory tree further down (`ANA`, `MAP`, `WM`, ...) is the *authoring* layout. 
At runtime the game does not open files by path. There are no `cdrom:\...` path strings in the code outside the boot line. 
Instead, every read is issued against a **resource table** that maps an integer resource index to a raw disc location (LBA) plus a byte size.

The addressing structure is contained in a file named `SKCDPOSE.DAT` (maybe meaning skip CD position?), and the data
consists of an array structure where each element is a `CdResourceEntry`.

- `CdResourceEntry` = `{ CdlLOCRaw location; s32 data_size; }` (8 bytes each):
  a 4-byte disc location (BCD minute/second/frame + a mode byte) plus a
  little-endian byte size. The mode byte is `0xFF` on every live entry and
  `0x00` on the null slots. See [src/cdrom.c](../src/cdrom.c).
- The table lives in RAM at `CD_RESOURCE_ENTRIES = 0x801ED998`.
- You can use this file and convert `CdlLOC` entries into LBA with math:
`(((minute * 60) + second) * 75) + sector - 150 = LBA`
- Game code then calls `cdrom_queue_read(resource_index, dst)` and the CD
  subsystem resolves index -> `CdResourceEntry` -> LBA. See
  [cd-system-architecture.md](cd-system-architecture.md).

Named indices are in [include/cd_resources.h](../include/cd_resources.h). The
first entries (all confirmed against the parsed table):

| Idx | File | Idx | File |
|--:|---|--:|---|
| 0 | SYSTEM.CNF | 12 | CARDA.BIN |
| 1 | SLUS_010.13 | 13 | GOSUB.BIN |
| 2 | FIELD.BIN | 14 | WSEL.BIN |
| 3 | WMAP.BIN | 15 | CHECKPS.BIN |
| 4 | TITLE.BIN | 16 | CLOAD.BIN |
| 5 | GNAME.BIN | 17 | NIKI.BIN |
| 6 | MENU.BIN | 18 | ADDHERO.BIN |
| 7 | SHOP.BIN | 19 | *(null/reserved slot)* |
| 8 | ZUKAN.BIN | 20 | *(null/reserved slot)* |
| 9 | GOLEM.BIN | 21 | SOUND/EFFECT.SET |
| 10 | GOVER.BIN | 22 | SOUND/WAVE0001.DAT (ADPCM sample bank) |
| 11 | MOVIE.BIN | 23 | SOUND/MSC_DATA.DAT |
| | | 24+ | MUSIC002.SET, MUSIC003.SET, ... |

The file names in the directory tree are just friendly labels.
The shipped game reaches each asset by its precomputed LBA in `SKCDPOSE.DAT`.

## Disc validation

`MAP/FDATA/SK.DAT` (21 bytes) is the disc identity string checked at boot.
"SK" is probably for *seiken* (Sacred Sword, the Seiken Densetsu series). Its bytes are:

```
90 b9 8c 95 20 53 51 55 41 52 45 20 53 4f 46 54 28 55 53 29 00
                S  Q  U  A  R  E     S  O  F  T  (  U  S  )
```

The first four bytes `90 b9 8c 95` are Shift-JIS for "聖剣" (seiken), followed by " SQUARE SOFT(US)". 
`cdrom_verify_disc()` reads this sector via `CdGetSector` and compares it against `g_disc_validation_id[21]`,
 and a mismatch rejects the disc (`CD_INIT_STATE_ERROR_PAUSE`).

## Directory map

| Dir | Files | Size | Contents |
|---|---:|---:|---|
| `ANA` | 3241 | 150M | Character/object graphics: sprite banks, animations, textures |
| `BIN` | 17 | 2.0M | Game-mode overlays (streamed MIPS code) |
| `FUKU` | 1 | 72K | A single test texture (`TEST.TIM`); leftover dev asset |
| `MAP` | 490 | 72M | Field map data (`.MPD` / compressed `.PRS`) + `FDATA` |
| `MOVIE` | 17 | 266M | FMV: `.STR` MDEC video + `.DAT` companion audio |
| `NULL` | 1 | 27M | `NULL3.DAT`, a padding/alignment filler file |
| `SND_HERO` | 13 | 256K | Player weapon/action sound-effect banks (`.EFF`) |
| `SND_MON` | 38 | 1.4M | Monster/enemy sound-effect banks (`.EFF`) |
| `SND_SOTO` | 21 | 2.5M | Outdoor/field music sequence sets (`.SET`) |
| `SOUND` | 53 | 12M | Music + master SFX sequence sets (`.SET`, AKAO) |
| `WM` | 1141 | 32M | World-map ("Land Make") data, tiles, and effects |

### Overlays (`BIN`)

The 17 `.BIN` files are the game-mode overlays. 
Each one corresponds 1:1 to a folder under [src/overlays/](../src/overlays/) and carries a 4-byte module-ID prefix before its code
(see [overlay-id-prefix.md](overlay-id-prefix.md)). Load addresses and IDs are documented there. 
Some of the overlays are easy to recognize if you know Japanese.
Mapping:

| BIN | src/overlays | Notes |
|---|---|---|
| `ADDHERO.BIN` | `addhero` | Character add screen |
| `CARDA.BIN` | `carda` | Card |
| `CHECKPS.BIN` | `checkps` | Debugging and initialization routines |
| `CLOAD.BIN` | `cload` | Load screen |
| `FIELD.BIN` | `field` | Main field engine |
| `GNAME.BIN` | `gname` | Name entry |
| `GOLEM.BIN` | `golem` | Golem workshop minigame |
| `GOSUB.BIN` | `gosub` | Shared sub-routine overlay |
| `GOVER.BIN` | `gover` | Game over |
| `MENU.BIN` | `menu` | Main menu |
| `MOVIE.BIN` | `movie` | FMV playback driver |
| `NIKI.BIN` | `niki` | Diary |
| `SHOP.BIN` | `shop` | Shop |
| `TITLE.BIN` | `title` | Title screen |
| `WMAP.BIN` | (wm) | World map |
| `WSEL.BIN` | `wsel` | World/land selection |
| `ZUKAN.BIN` | `zukan` | Encyclopedia |

### Field maps (`MAP`)

Split into five bucket folders `MAP0`..`MAP4` plus `FDATA`. These are two on-disc forms:

- `.MPD` - uncompressed field map, magic `"SKmapDat"`.
  Only a handful ship uncompressed (test/battle maps in `MAP0`).
- `.PRS` - LZ-compressed `.MPD`. Header starts `01 0e`, then the first literal
  bytes of the payload ("SKmapDat") are visible. The bulk of ship maps are `.PRS`.

`MAP/FDATA` holds three shared assets: `FONT.PRS` (compressed font), `M_WIN.PRS`
(menu window graphics), and `SK.DAT` (the disc-validation string above).

**Map naming convention** `PREFIX_AREA`:

- Prefix = location code, e.g. `DRL`, `SHP` (ship), `TWR` (tower), `JGL`
  (jungle), `SEA`, `MNT` (mountain), `SNW` (snow), `RUI` (ruins), `MIN` (mine),
  `LAK` (lake), `PRT` (port), `CV1` (cave). ~30 distinct prefixes.
- Area suffix = sub-area type, e.g. `FD` (field), `RM` (room), `DN` (dungeon),
  `BSS` (boss), `STR` (street), `DCK` (deck), `BCH` (beach), `ENT` (entrance),
  `1F`/`2F`/`3F` (floors), numbered variants (`FD00`, `FD01`, ...).

### FMV (`MOVIE`)

- `.STR` - standard PSX MDEC video streams (sector-interleaved with XA audio).
  Includes `OPEN.STR` (opening, 43M), `POST.STR`, `STAFF.STR` (staff roll, 110M
  - the largest single file on the disc), and per-boss/cutscene clips.
- `.DAT` - companion streamed-audio blobs for certain sequences
  (`GIRA.DAT`/`GIRA.STR`, `KAJU.DAT`/`KAJU.STR`), plus boss-intro audio
  (`*_BSS*.DAT`). Content is raw ADPCM-style sample data, not MDEC.

Played by the `movie` overlay / `MOVIE.BIN` through the streaming path in [cdrom.c](../src/cdrom.c).

### Object graphics (`ANA`)

Extensions and roles:

- `.IMG` - custom Square sprite/animation banks (header is a table of 32-bit offsets to frames/parts, not a fixed magic).
- `.DAT` - generic companion data blobs paired with the `.IMG` banks (`.IMG` graphics + `.DAT` control data is the common pairing).
- `.STV` - **battle-effect animation scripts**, all under `EFFECT/*` sub-folders (`BAT_EF*`, `RENEF*`). 
  Companion forms in the same folders: `.SV` (7, a variant) and `.ST` (1, `JYO_SET.ST`, an effect-set index).
- `.TIM` - normal PSX textures.
- Numeric-extension files - these are **TIM image sequences** whose extension is a frame number, not a type. 
  `ANA/ENDMES/SC_END.1..84` (ending-message cards) and `ANA/EVTITLE/TITLEF.1..99` (event/area title cards) 
  both start with the TIM magic. Treat them as ordered TIM frame sets.
- No-extension `ANA/B_NPCOBJ/BTLNPC0B` is a battle-NPC object graphic (leading TIM header) that simply lost its extension.

Sub-folders group assets by role: `ADD_OBJ`, `EFFECT` (battle effects),`B_NPCOBJ`/`F_NPCOBJ` (battle vs field NPC objects), 
`B_PETOBJ`/`F_PETOBJ` (pet objects), `BTL_OBJ` (battle), `RINGDATA` (ring-command UI), `ZUKAN_P` (encyclopedia portraits), 
`INFO_*` (per-region info), `MAPINFO`, `ENDMES`(ending), `EVTITLE` (title cards).

### World map (`WM`)

The "Land Make" world-map subsystem (`WMAP.BIN`). 26 sub-folders. 
Extensions:

- `.DAT` - generic layout/control data.
- `.PIM` - packed/compressed world-map tile images (header `01 00 10 f0`, same compressed style as `.PRS`/`.PAT`), 
  in `WMAP` and `WMTIM/WMAPT*`.
- `.TIM` - textures.
- `.BTP` - world-map effect graphics (offset-table format like `.IMG`),
  in the `WEFF*`/`WCV1`/`WKAJU`/`WPRT`/`WRUI` effect folders.
- `.PAT` (1, `WMAP`) - compressed world-map pattern/layout; its uncompressed
  form is indexed by the no-extension `WM/TEST/LANDDATA` offset table (both
  share the same offset values, e.g. `68 4a`, `54 6b`).
- `.CLT` (1, `WEFF`) - a TIM CLUT (palette); header is the TIM magic. "CLT" = CLUT.

Sub-folders group by region/feature: `WLAND`, `WSEA`, `WSNW`, `WMNT`, `WTWR`, `WPRT`, `WRUI`, `WMAP`, `WMDATA`, 
`WEFF*` (effects), `WLOAD`, `WHLP` (help), `WKAJU`, `MOGU`, `SND`.

### Audio

Square's **AKAO** sound engine (magic `"AKAO"`, visible at offset `0x0C` of each`.SET`). 
AKAO stuff is all over the place. Big thanks to the `FFVII` decomp wiki for helping me to understand this.  
Three sequence pools plus effect banks:

| Dir | Ext | Role |
|---|---|---|
| `SOUND` | `.SET` | Master music + `EFFECT.SET`; `MSC_DATA.DAT` index |
| `SND_SOTO` | `.SET` | Outdoor/field music (`MUSICS01`..`MUSIC026`) |
| `SND_HERO` | `.EFF` | Player weapon/action SFX (`KEN`=sword, `AXE`, `YARI`=spear, `ROD`, `HAMMER`, ...) |
| `SND_MON` | `.EFF` | Monster SFX, one bank per enemy/boss |

`.EFF` files begin with a table of 32-bit offsets indexing individual samples in the bank. 
`MUSIC*.SET` names are the BGM track numbers.

### Filler

- `NULL/NULL3.DAT` (27M) and `FUKU/TEST.TIM` are non-gameplay: `NULL3.DAT` is an
  alignment/seek-time padding file more than likely to influence the generated LBA values.
  `TEST.TIM` is a leftover test texture.

## File format quick reference

| Ext | Magic (first bytes) | Format |
|---|---|---|
| `.MPD` | `53 4b 6d 61 70 44 61 74` "SKmapDat" | Uncompressed field map |
| `.PRS` | `01 0e ...` | LZ-compressed `.MPD`/asset |
| `.STR` | PSX sector sync header | MDEC video + XA audio stream |
| `.SET` | `... "AKAO" @0x0C` | AKAO music/effect sequence |
| `.EFF` | offset table | AKAO sample bank (SFX) |
| `.TIM` | `10 00 00 00` | Standard PSX TIM texture |
| `.CLT` | `10 00 00 00` | TIM CLUT (palette) |
| `.IMG` | offset table | Square sprite/animation bank |
| `.STV` / `.SV` | `xx xx 01 00 ...` | Battle-effect animation script (`ANA/EFFECT`) |
| `.BTP` | offset table | World-map effect graphics |
| `.PIM` | `01 00 10 f0 ...` | Compressed world-map tile image |
| `.PAT` | `01 00 a0 f0 ...` | Compressed world-map pattern/layout |
| `.BIN` | MIPS code + 4-byte ID | Streamed game-mode overlay |
| `.DAT` | (varies) | Generic container: companion data, streamed audio, or table |
| `SC_END.N` / `TITLEF.N` | `10 00 00 00` | Numbered TIM image sequence (ending / title cards) |
| `SK.DAT` | `90 b9 8c 95` (SJIS) | Disc-validation ID string |
| `SKCDPOSE.DAT` | (binary table) | LBA/size resource table (loaded at boot) |

## TODO
- Figure out more extensions and prefix meanings.

## See also

- [cd-system-architecture.md](cd-system-architecture.md) - how reads are queued, streamed, validated, and recovered at runtime.
- [overlay-id-prefix.md](overlay-id-prefix.md) - overlay module IDs and load addresses.
- [handling-copyrighted-data.md](handling-copyrighted-data.md).
