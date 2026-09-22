# WMAP translation-unit reorganization

## Map labels

The range `0x8005F9BC..0x80060720` is consolidated into
`src/overlays/wmap/wmap_map_labels.c`. It owns label selection, outgoing/current
label fades, sprite template reset, and label/decorative packet submission.
The four definitions retain their original address order and `gcc280_g0` route.
YAML and the overlay registry now name the combined source.

`include/wmap_map_labels.h` declares the three externally called functions.
Ten caller files include that header instead of duplicating declarations or
relying on implicit declarations. The rendering helper `func_80060230` has its
prototype only in the implementation file. Its external linkage is retained:
making it static changes its call relocation and lowers the caller's objdiff
score from 38.630726% to 38.617252%, with no other instruction changes.

Sprite declarations shared by the four functions now use the SDK `SPRT` type.
The previous triangle-shaped declaration in the reset routine described the
same 20-byte storage but obscured its sprite use. The label-position type is
private to this unit.

| Function | Before | After |
| --- | ---: | ---: |
| `func_8005F9BC` | 38.630726% | 38.630726% |
| `func_8005FF88` | 100% | 100% |
| `func_80060230` | 56.248890% | 56.248890% |
| `func_800605B4` | 100% | 100% |

Both partial functions remain work in progress. Their raw packet accesses and
surplus texture-page helper arguments are preserved during this organizational
change. The palette word `D_800D0368` has one `s32` declaration; the partial
renderer explicitly reads its low `u16` half to preserve its existing load
width. Further semantic cleanup belongs with their matching work.

The relocation-aware instruction audit confirms all 920 instructions across
the four functions are unchanged. Evidence and pre-change snapshots are in
`working/wmap-tu-reorganization/`, including `labels-instruction-audit.json`,
`before-labels/`, and the build and score audit logs.

Validation: `make SHELL="bash -o pipefail" wmap-objdiff wmap -j12` completes
successfully. The full audit covers all 3,738 functions, with 3,562 exact matches,
no missing functions, and no score changes from the preceding TU checkpoint.
Objdiff configuration was regenerated. Existing partial matches mean this is
not a claim that the entire overlay is byte-identical to the original binary.

## World-map driver

The next range, `0x80060720..0x800641DC`, is consolidated from twelve sources
into `src/overlays/wmap/wmap_main.c`. It covers frame setup, the overlay entry,
initialization, display transitions, input and pending actions, the resource and
rendering loop, tile-layout reconstruction, and refresh.

`include/wmap_main.h` exposes the refresh function and includes the existing
`world_map.h` entry interface without duplicating its declaration. Four callers
now include the owner header. Helper prototypes and private layouts stay in the
implementation. Original external helper linkage is retained because static
linkage changes call relocations and lowers several existing objdiff scores.
Callbacks now have the sequence runtime's event parameter, and three internal
routines declare their existing, unused caller arguments.

The combined source has one render-context declaration and uses that layout for
the transition packet cursor and ordering table. Conflicting byte, sprite,
polygon, tile-array, and scalar declarations within the former files have been
reconciled. Existing partial implementations and their surplus CD-read and
texture-page helper arguments remain work in progress.

The three switch tables and intervening display-transfer rectangle share the
new `.rodata` owner. The rectangle definition stays between the second and third
switch-producing functions to preserve emitted order. The 188-byte read-only
layout, including the original four-byte alignment gap, retains all constants
and switch destinations. The instruction audit finds only the expected section
addend changes for the second and third switch tables (24 and 72 bytes).

| Function | Before regrouping | Combined target and C |
| --- | ---: | ---: |
| `func_80060720` | 100% | 100% |
| `run_world_map` | 89.292305% | 89.292305% |
| `func_80060918` | 68.622690% | 68.622690% |
| `func_80060F04` | 56.904133% | 56.904133% |
| `func_80061878` | 96.412840% | 96.412840% |
| `func_80061A2C` | 73.200000% | 73.200000% |
| `func_80061F18` | 100% | 100% |
| `func_80061FF8` | 66.991840% | 66.991840% |
| `func_800623CC` | 91.872810% | 91.866320% |
| `func_80063BE0` | 85.911220% | 85.911220% |
| `func_80063F38` | 100% | 100% |
| `func_80064094` | 100% | 100% |

The main-loop score difference is a target-grouping effect: its original C
object also scores 91.866320% against the new combined target. Its 1,523
instructions are unchanged except for the switch-table section addend. No
source-level match regression is accepted. The probe and audit records are
`main-diff.json`, `main-loop-old-new.json`, `main-instruction-audit.json`, and
`main-rodata-audit.json` under `working/wmap-tu-reorganization/`.

Validation: the final `wmap-objdiff wmap` build succeeds, and regenerated
objdiff configuration includes the combined unit. The full 3,738-function audit
has no missing functions or regressions against the original C objects measured
on the same target layout. All 3,562 exact matches remain exact. Compared with
the preceding checkpoint, only the documented main-loop grouping score changes.
The WMAP source count falls from 1,999 to 1,988.

## Resource and display support

The range `0x80064F14..0x8006544C` is consolidated from twelve sources into
`src/overlays/wmap/wmap_resource_support.c`. It contains word copying, TIM
loading/streaming/upload, resource-read queuing, sound lookup, display clearing,
texture-page packet submission, actor-state initialization, and a resource
header accessor. Definition order and the `gcc280_g0` route are preserved.

`include/wmap_resource_support.h` owns the callable interface. Seventy-seven
WMAP caller units now include it, replacing their duplicate declarations.
Implementation layouts remain private. The TIM loaders use SDK `RECT` and the
CD API header, with descriptive resource and data variable names. The textured
triangle retains its local layout because the project's GPU ABI header does
not currently provide `POLY_FT3`.

Three header declarations intentionally remain non-prototyped: the empty
`func_80064F14` handler, sound helper `func_800652A8`, and texture-page helper
`func_8006534C`. Their existing callers pass differing numbers of arguments;
partial reconstructions still contain surplus arguments. The texture-page
parameter now has word width at the interface and is narrowed by its packet
field store, preserving the existing instructions. The resource-header accessor
also declares its caller's two unused arguments. These choices preserve the
current source and caller code generation while centralizing declarations.

All 304 instructions across the twelve helpers are unchanged, including
relocations. Nine functions remain exact. The three partial scores are unchanged:

| Function | Before | After |
| --- | ---: | ---: |
| `func_80064F1C` | 99.062500% | 99.062500% |
| `func_80064F64` | 73.188410% | 73.188410% |
| `func_80065078` | 73.188410% | 73.188410% |

Evidence is recorded in `support-diff.json`, `support-instruction-audit.json`,
`before-support/`, and the support build/audit logs under
`working/wmap-tu-reorganization/`. YAML, registry routing, and objdiff
configuration reference the combined unit.

Validation: the final `wmap-objdiff wmap` build succeeds. The full audit covers
all 3,738 functions, with 3,562 exact matches, zero missing functions, and no
score changes from the preceding checkpoint, including the 77 caller units.
The WMAP source count falls from 1,988 to 1,977.

## View effects

The range `0x8006544C..0x80066F9C` is consolidated from fifteen source files
into `src/overlays/wmap/wmap_view_effects.c`. It contains seventeen functions:
the original `func_800654BC.c` also held `func_8006544C` and `func_800654D4`.
The unit owns view-state dispatch, tint updates, map-grid initialization,
projection and UV updates, fade-edge coordinates, camera transitions, and
radial-particle state. Function order and `gcc280_g0` routing are preserved.

`include/wmap_view_effects.h` owns the public interface, and 52 caller units
include it. The tint callback is private and static. Four other internal
helpers retain external linkage with prototypes only in the C file: making
those helpers static lowers the caller scores, including the exact tint
update. Data-table callbacks keep external linkage for their extracted tables.
Both sequence callbacks now declare their unused event parameter.

The render buffer has a single private layout, with flat and row-indexed views
of the same tile storage. Separate declarations of the map transform, actor
motion records, and tint target have been reconciled. The tint target uses
`CVECTOR`; its packed-word store remains an explicit representation access.
Projection continues to use the existing SDK/DMPSX GTE compatibility macros.

The flat indexing in `func_800654F8` and the byte-addressed packed-color store
in `func_800667E8` are retained. Replacing these with row/member indexing lowered
their exact scores to 82.797295% and 66.476190%, respectively. The accepted
version keeps both at 100%. Partial functions retain their existing low-level
accesses where a wider cleanup would exceed this organizational change.

Ten functions remain exact; all seven partial scores are preserved:

| Function | Before | After |
| --- | ---: | ---: |
| `func_8006579C` | 91.988010% | 91.988010% |
| `func_80065E20` | 91.246750% | 91.246750% |
| `func_80065F54` | 99.777780% | 99.777780% |
| `func_800660BC` | 82.125490% | 82.125490% |
| `func_800664B8` | 70.612750% | 70.612750% |
| `func_8006688C` | 93.534090% | 93.534090% |
| `func_80066BA4` | 93.723404% | 93.723404% |

Snapshots and probe evidence are under `working/wmap-tu-reorganization/`,
including `before-view/`, `view-diff.json`, and the view build/audit logs.
YAML, registry routing, and objdiff configuration name the combined unit.

The 1,735-instruction audit is unchanged apart from the tint callback address
relocations in `func_8006683C`: the private callback is now referenced as
`.text + 0x1D4`, its confirmed symbol offset, instead of an external symbol.
Its caller remains an exact match.

Validation: the final `wmap-objdiff wmap` build succeeds. All 3,738 functions
are covered by the score audit, with no missing functions, no score changes
from the preceding checkpoint, and all 3,562 exact matches preserved. The WMAP
source count falls from 1,977 to 1,963.

## Sprite-part renderer

The range `0x80066F9C..0x800675F0` now belongs to
`src/overlays/wmap/wmap_sprite_render.c`, with its public interface in
`include/wmap_sprite_render.h`. This candidate is already a single-function
unit, so organization consists of the semantic filename, owner header, and
centralized declarations in 177 caller units. YAML, compiler routing, and
objdiff configuration use the new filename; the fixed-address function symbol
remains unchanged.

The five parameters now describe the actor, packed screen position, texture
resource index, ordering-table index, and variant selector. The ordering-table
index is correctly declared as `s32`, consistent with its arithmetic and
callers. Its dead initial assignment to a pointer temporary is removed. The
existing DMPSX GTE compatibility macros and partial reconstruction remain.

`func_80066F9C` remains at 75.209880%. All 424 generated instructions and their
relocations are identical to the preceding implementation. The renderer is
still partial; this change does not claim a completed decompilation.
Evidence is in `before-sprite/`, `sprite-diff.json`,
`sprite-instruction-audit.json`, and the sprite build/audit logs under
`working/wmap-tu-reorganization/`.

Validation: `wmap-objdiff wmap` builds successfully. The full audit covers all
3,738 functions with no missing functions or score changes from the preceding
checkpoint; all 3,562 exact matches remain exact. The source count remains
1,963 because this TU required a rename rather than consolidation. New source
and header files are staged in Git to prevent omission from a CI checkout.
