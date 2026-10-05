# World-map effect resources

`wmap_effect_resources.c` queues the files used by world-map land effects.
Its 36-entry `g_wmap_effect_resource_loaders` table contains 31 distinct
loaders. Indices 0, 6, 14, 20, 28 and 29 share the MHM resource set;
34 and 35 are special effects. The indices match in US and JP.

## Dispatch and completion

`wmap_queue_effect_resources` first waits for previously queued CD reads.
If the requested index equals `g_wmap_last_effect_resource_set`, it returns.
Otherwise it caches that original index and dispatches its loader. An index
outside 0-35 selects the shared loader and sets `g_wmap_exit_frame` to one.
The cached value remains the invalid request. A repeated request therefore
waits for earlier reads but does not queue the fallback again.

The loaders queue reads and return. They do not upload textures or wait for
their newly queued batch. `func_800591A8`, special-effect handlers and travel
handlers later wait and pass the texture buffers to `func_800651B4`.
The third texture is omitted for effect 2 and special effect 34.

## Storage and file types

| Storage | Size or spacing | Contents |
| --- | --- | --- |
| `g_wmap_effect_texture_buffer_0/1/2` | Starts are 0x8400 bytes apart | TIM palette and pixel blocks |
| `g_wmap_animation_bank_0` through `_5` | Starts are 0x2000 bytes apart | Sprite animation `.DAT` files |
| `g_wmap_load_buffer` | 0x30000 bytes before the first packet buffer | Base BTP model, embedded model bundle, and model workspace |
| `g_wmap_effect_model_pack_1` through `_9` | Pointer variables | Current effect's model-pack views |
| `g_wmap_effect_empty_model_slot` | Pointer variable, set only by effect 31 | Zero-filled trailing MANBTP slot; no recovered reader |

The `*_model_buffer_<offset>` symbols are fixed views inside
`g_wmap_load_buffer`. The suffix is a byte offset, not an independently
allocated buffer or a model number. Loader assignments divide this workspace
before queueing reads. Their order and byte increments matter because slots
can share the same arena in different effects.

A `.BTP` starts with a 32-bit model count and that many 32-bit offsets relative
to the BTP base. Each selected model starts with signed 16-bit vertex and face
counts, followed by eight-byte vertices, ten-byte faces and 24-byte face
attributes. This agrees with `wmap_draw_model` and the inspected files.

Five `.DAT` files are bundles of BTP packs: FIGBTP, MNTBTP, MGCBTP, JULBTP and
MANBTP. They are loaded once at the workspace base, and the fixed aliases
select packs already inside that file. A `.DAT` extension alone therefore
does not identify sprite animation data. BTP counts and model bounds were
checked for the model-pack views in these bundles.

Several configured pointers select zero-filled trailing slots, and some
loaders configure pointers that receive no read in that batch. Neither case
establishes a usable model resource. Other model pointer variables are not
reset by every loader; the selected effect must use its own populated slots.

## Loader map

The filename column lists a representative loaded asset, not a proposed
English name for the land or animation. Shared file bytes can be referenced
by several resource IDs; those IDs remain distinct.

| Effect indices | Original function | Loader | Representative asset |
| --- | --- | --- | --- |
| 0, 6, 14, 20, 28, 29 | `func_800A8B80` | `wmap_queue_shared_land_effect_resources` | `WM/WEFF1/MHM_AF.TIM` |
| 1 | `func_800A8E9C` | `wmap_queue_effect_01_resources` | `WM/WEFFD/DOM_AF.TIM` |
| 2 | `func_800A9358` | `wmap_queue_effect_02_resources` | `WM/WWAL/GAT_HI.TIM` |
| 3 | `func_800A97B0` | `wmap_queue_effect_03_resources` | `WM/WPRT/PRT_AF.TIM` |
| 4 | `func_800A8C80` | `wmap_queue_effect_04_resources` | `WM/WEFF1/MON_AF.TIM` |
| 5 | `func_800A9A38` | `wmap_queue_effect_05_resources` | `WM/WEFF1/MGC_LAN.TIM` |
| 7 | `func_800A93D4` | `wmap_queue_effect_07_resources` | `WM/WSEA/MAD_1.TIM` |
| 8 | `func_800A9C38` | `wmap_queue_effect_08_resources` | `WM/WEFF1/URU_AF.TIM` |
| 9 | `func_800A9D24` | `wmap_queue_effect_09_resources` | `WM/WEFF1/BON_AF.TIM` |
| 10 | `func_800A9B34` | `wmap_queue_effect_10_resources` | `WM/WEFF1/LAK_AF.TIM` |
| 11 | `func_800A96AC` | `wmap_queue_effect_11_resources` | `WM/WTWR/TWR_MIS.TIM` |
| 12 | `func_800A95D4` | `wmap_queue_effect_12_resources` | `WM/WWD1/MOR_KIR.TIM` |
| 13 | `func_800A8F74` | `wmap_queue_effect_13_resources` | `WM/WCV1/MEK_AF.TIM` |
| 15 | `func_800A926C` | `wmap_queue_effect_15_resources` | `WM/WMNT/NORU1.TIM` |
| 16 | `func_800A9E10` | `wmap_queue_effect_16_resources` | `WM/WEFF1/HEL_LAN.TIM` |
| 17 | `func_800A9960` | `wmap_queue_effect_17_resources` | `WM/WEFF1/DST_SUN.TIM` |
| 18 | `func_800A8D6C` | `wmap_queue_effect_18_resources` | `WM/WEFF/WST_1.TIM` |
| 19 | `func_800AA1AC` | `wmap_queue_effect_19_resources` | `WM/WEFF2/GRB_LAN.TIM` |
| 21 | `func_800A9888` | `wmap_queue_effect_21_resources` | `WM/WEFF1/JGL_SIBA.TIM` |
| 22 | `func_800AA2EC` | `wmap_queue_effect_22_resources` | `WM/WEFF2/DRL2_ETC.TIM` |
| 23 | `func_800AA040` | `wmap_queue_effect_23_resources` | `WM/WEFF2/DRL1_FIR.TIM` |
| 24 | `func_800AA6A0` | `wmap_queue_effect_24_resources` | `WM/WEFF3/ESL_AF.TIM` |
| 25 | `func_800AA444` | `wmap_queue_effect_25_resources` | `WM/WEFF2/JUL_KIR.TIM` |
| 26 | `func_800A9010` | `wmap_queue_effect_26_resources` | `WM/WSNW/FIG1.TIM` |
| 27 | `func_800A9EFC` | `wmap_queue_effect_27_resources` | `WM/WEFF2/SHP_LAN.TIM` |
| 30 | `func_800A94C0` | `wmap_queue_effect_30_resources` | `WM/WRUI/MIN_S.TIM` |
| 31 | `func_800AA564` | `wmap_queue_effect_31_resources` | `WM/WEFF3/MAN_AF.TIM` |
| 32 | `func_800A910C` | `wmap_queue_effect_32_resources` | `WM/WKAJU/KAJ1.TIM` |
| 33 | `func_800AA7E8` | `wmap_queue_effect_33_resources` | `WM/WEFF3/ELS2_0.TIM` |
| 34 | `func_800AA898` | `wmap_queue_effect_34_resources` | `WM/WEFF4/YOKOKU1.TIM` |
| 35 | `func_800AA914` | `wmap_queue_effect_35_resources` | `WM/WEFF4/KUS_1.TIM` |

## Embedded bundle views

Offsets are relative to `g_wmap_load_buffer` and are identical in both
regions. Pack numbering starts at one for additional packs after the BTP at
workspace offset zero. The shared aliases at 0xFDC and 0x1FB8 are used by more
than one bundle. Empty slots have zero count/offset words in the inspected
file; their broader purpose remains unknown.

| Bundle | Effect | Additional pack offsets | Trailing empty slot |
| --- | --- | --- | --- |
| `FIGBTP.DAT` | 26 | `0xfdc`, `0x1b70`, `0x2b4c`, `0x31fc`, `0x39cc` | `0x3f9c` |
| `MNTBTP.DAT` | 15 | `0x89c`, `0xf3c`, `0x1484`, `0x2460` | `0x343c` |
| `MGCBTP.DAT` | 5 | `0xfdc`, `0x1fb8`, `0x5c04`, `0x5ec0`, `0x6494` | `0x6a68` |
| `JULBTP.DAT` | 25 | `0xfdc`, `0x1fb8`, `0x2f94`, `0x3808`, `0x407c`, `0xc2a8` | `0x172f4` |
| `MANBTP.DAT` | 31 | `0xb04c`, `0xd648`, `0xdb3c`, `0xefc0`, `0xff9c`, `0x103f8`, `0x113d4`, `0x15320`, `0x162fc` | `0x1df48` |

## Resource identifiers

[`wmap_effect_resource_ids.h`](../../../../src/overlays/wmap/internal/wmap_effect_resource_ids.h)
names all 277 CD IDs used by these loaders. Names specify the effect and its
destination slot; comments identify files with matching bytes. The queue
order remains the original order, including deliberately swapped animation
banks and resources with identical payloads at different IDs.

The IDs were resolved through each region's CD-position table and checked
against bytes read directly from its disc image. File matches identify exact
content, while identical copies can have several matching paths. All 277
inspected payloads are byte-identical between US and JP.

## Remaining analysis

- The BTP geometry's visual role needs comparison with the effect timeline
  and rendered assets. Filenames alone do not prove a model is a cloud,
  flash, building, water surface or other named object.
- The reasons for the zero-filled trailing slots and configured-but-unloaded
  slots are not established. Preserve them until their consumers or original
  packing conventions explain them.
- Most animation-bank records and their relationship to sprite actor setup
  still need a separate format audit.
- `func_80064F64` is the palette upload path used by effect 18. Its broader
  shared-buffer behavior is outside this loader-family cleanup.
