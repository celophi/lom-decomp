# Scene name prefixes

[English documentation](../../README.md) | [日本語](../../../jp/technical/reference/scene-prefixes.md) | [Scene extractor](scene-extractor.md) | [Scene layouts and conditions](../architecture/scene-layouts.md) | [Disc layout](disc-layout.md)

Every scene file on the disc has a name like `WAL_B020.IMG` or `HTW_INN0.IMG`.
The three letters in front are a code for the land the scene belongs to, and
once you know a few of them, digging through `ANA` gets a lot less painful.
`HTW` is Domina, `WST` is Luon Highway, `DMY` is the debug room, and so on.
This page lists all of them.

A quick heads-up before the tables: which land each prefix belongs to comes
straight from the game's own data, so that part is pretty solid. What the
letters actually *stand for* is another story. Those are just my guesses for
now, and I'll refine them later as we learn more. The same goes for everything
under [Unknowns](#unknowns).

## Reading a scene name

A scene name is a prefix, an underscore and a four-character suffix:

```text
HTW_INN0.IMG
^^^ ^^^^
|   +------ which scene inside the land
+---------- which land
```

The suffixes don't follow one scheme across the game. Some of them read
naturally, like `INN0` and `PUB0` in Domina, and the common ones (`FD` for
field, `RM` for room, `DN` for dungeon and so on) are listed in the map naming
convention in [Disc layout](disc-layout.md#field-maps-map). The rest are room
numbers or something we haven't figured out yet.

Don't trust the folder. The scene files live under `ANA/INFO_*` and
`ANA/MAPINFO`, but those folders mix lands freely. `INFO_BON` has Lucemia,
Tree of Mana and Domina scenes next to the Bone Fortress ones, and `INFO_DRL`
holds all 80 `END` scenes. Always go by the file name.

The same prefixes show up on the map backgrounds under `MAP/MAP0` to
`MAP/MAP4` (`HTW_ALL0.PRS`, `WAL_DN08.PRS` and so on). The world map is the
exception: its effect files under `WM` use their own codes, such as `DOM`,
`MOR` and `MEK`, so don't mix those up with these. See
[WMAP effect resources](wmap-effect-resources.md) for those.

## Land prefixes

Sorted by land ID, the same numbering the save file uses for its land records.
The entry scene is the one the world map sends you to when you walk into the
land. The scene count covers every `INFO_*` and `MAPINFO` file with that
prefix.

| ID | Land | Prefix | Scenes | Entry scene | What the letters might mean (guess) |
|--:|---|---|--:|---|---|
| 0 | Home | `MHM` | 28 | `MHM_OS00` | "My Home" |
| 1 | Domina | `HTW` | 22 | `HTW_ALL0` | "Home Town" |
| 2 | Gato Grottoes | `WAL` | 53 | `WAL_ALL0` | "Wall", since the town is built into a cliff |
| 3 | Polpota Harbor | `PRT` | 9 | `PRT_ALL0` | "Port" |
| 4 | Lumina | `MON` | 10 | `MON_ALL0` | "Moon"? |
| 5 | Geo | `MGC` | 29 | `MGC_GAT0` | "Magic", for the magic academy |
| 7 | Madora Beach | `SEA` | 28 | `SEA_EVT0` | "Sea" |
| 8 | Ulkan Mines | `MIN` | 35 | `MIN_E000` | "Mine" |
| 9 | Bone Fortress | `BON` | 37 | `BON_VLLG` | "Bone" |
| 10 | Lake Kilma | `LAK` | 27 | `LAK_E000` | "Lake" |
| 11 | Tower of Leires | `TWR` | 48 | `TWR_E000` | "Tower" |
| 12 | White Forest | `WD1` | 29 | `WD1_EVT0` | "Wood 1" |
| 13 | Mekiv Caverns | `CV1` | 33 | `CV1_EVT0` | "Cave 1" |
| 15 | Norn Peaks | `MNT` | 38 | `MNT_EVT0` | "Mountain" |
| 16 | Underworld | `HEL` | 50 | `HEL_OS00`, `HEL_OS01` or `HEL_OS11` | "Hell" |
| 17 | Duma Desert | `DST` | 28 | `DST_B000` | "Desert" |
| 18 | Luon Highway | `WST` | 23 | `WST_EVT0` | "West"? |
| 19 | Junkyard | `GRB` | 27 | `GRB_E00A` | "Garbage" |
| 21 | Jungle | `JGL` | 36 | `JGL_E000` | "Jungle" |
| 23 | The Flames | `DRL` | 39 | `DRL_OS00` | "Dragon lair"? |
| 24 | Lucemia | `ESL` | 31 | `ESL_E000` | No idea yet |
| 25 | Bejeweled City | `JUL` | 33 | `JUL_EN1F` | "Jewel" |
| 26 | Fieg Snowfields | `SNW` | 22 | `SNW_E000` | "Snow" |
| 27 | SS Buccaneer | `SHP` | 45 | `SHP_DK00` | "Ship" |
| 30 | Mindas Ruins | `RUI` | 31 | `RUI_E000` | "Ruins" |
| 31 | Tree of Mana | `SEI` | 24 | `SEI_OS00` | Japanese *sei* (holy), as in *Seiken Densetsu*? |
| 32 | Orchard | `MHM` | (shared with Home) | `MHM_GDN2` | Same code as Home, since the orchard is part of it |

A few things that aren't obvious from the table:

- **Two lands borrow another land's prefix.** The Orchard enters `MHM_GDN2`,
  one of Home's scenes. Land 6 has no location name, but it points at
  `MGC_GAT0`, the same entry scene as Geo.
- **Underworld is the only land with more than one entrance.** Every land has
  four entry slots, picked by the land's state. Every other land puts the
  same scene in all four, but Underworld uses `HEL_OS00`, `HEL_OS01`,
  `HEL_OS11` and `HEL_OS00` again.
- **Unused IDs fall back to Luon Highway.** IDs 14, 20, 22, 28, 29 and 33, and
  everything from 34 to 63, point at `WST_EVT0`. It looks like a default value
  more than anything meaningful.

## Prefixes that aren't lands

| Prefix | Scenes | Files | What it looks like |
|---|--:|---|---|
| `DMY` | 13 | `DMY_DEB0`, `DMY_WMP0`, `DMY_MENU`, `DMY_MAP1`-`6`, `DMY_PUB0`-`3` | "Dummy". The debug room (`DMY_DEB0`), the debug world map room (`DMY_WMP0`) and the test maps they warp to |
| `BTL` | 10 | `BTL_MENU`, `BTL_SMP1`-`9` | "Battle". `BTL_SMP1` shows up as "BattleSample1" in the debug warp menu |
| `END` | 80 | `END_0000`-`0079` | "Ending", probably |
| `INIT` | 1 | `INIT_MAP` | Scene ID 0, the scene the game sets when it boots. The debug menu's "ToInitmap" also goes here |
| `MID` | 11 | `MID_MAP0`-`9`, `MID_MENU` | No idea yet. Their text is only the stock lines that most scenes carry |

## The debug warp menu

`DMY_WMP0` has a developer warp menu with one entry per land, eight to a page,
in land ID order. It's the closest thing to an official list of these codes
that we have, and it matches the land table above:

| IDs | Menu entries |
|---|---|
| 0-7 | `mhm`, `htw`, `wal`, `prt`, `mon`, (empty), `mgc`, `sea` |
| 8-15 | `min`, `bon`, `lak`, `twr`, `wd1`, `cv1`, (empty), `mnt` |
| 16-23 | `hel`, `dst`, `wst`, `grb`, (empty), `jgl`, (empty), `drl` |
| 24-31 | `esl`, `jul`, `snw`, `shp`, (empty), BattleArena, `rui`, `sei` |

Two entries don't line up with the retail game, and both are listed under
[Unknowns](#unknowns): the menu puts `mgc` on
slot 6 instead of 5, and it calls slot 29 "BattleArena".

The same scene has more warp pages with specific scenes written as
prefix plus suffix (`wst bs03`, `cv1 b022`, `lak bs14`, ...), the `DMY_PUB`
maps as Pub0 to Pub3, BattleSample1, two maps named after staff (FujitaMap and
ShindoMap), a 'Mana' entry, and pages of Boss0 to Boss7 and BATTLE0 to BATTLE2
entries.

## Unknowns

- [ ] Work out what each prefix stands for. Everything in the "might mean"
  column is a guess, and `MON`, `WST`, `DRL`, `ESL` and `SEI` are the shakiest.
- [ ] Find out whether Geo is land 5 or 6. The location-name table says 5, the
  debug warp menu puts `mgc` on 6, and the world map sends both to `MGC_GAT0`.
- [ ] Find out what land 29 was. The debug warp menu calls it "BattleArena",
  but the retail game points it at the `WST_EVT0` default and gives it no
  location name. It might be a cut land.
- [ ] Find how the game enters land 33, Lucemia's second form. Its entry slot
  is the `WST_EVT0` default, so it has to go through something else.
- [ ] Find which state of the Underworld picks `HEL_OS00`, `HEL_OS01` or
  `HEL_OS11`, and what each one means in the story.
- [ ] Figure out what the eleven `MID` scenes are for.
- [ ] Decode the Japanese text in `DMY_DEB0` and most of the `DMY_MAP` and
  `DMY_PUB` scenes. Neither of our text tables reads it cleanly yet, so the
  debug room's own menus are still unread.
- [ ] Map the rest of the suffixes, starting with `OS00`, `EVT0` and `ALL0` on
  the entry scenes.
