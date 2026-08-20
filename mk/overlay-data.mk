# ============================================================================
#  Overlay generated data
# ============================================================================
#
# Some overlays embed copyrighted content (artwork, TIM images, packed layout
# tables) that cannot be committed to source control. Rather than link a raw
# .incbin blob, we build these regions from C so the data is legible, typed,
# and structurally verified: if a table's layout is wrong, the compiled bytes
# diverge from the original and the overlay stops matching.
#
# A per-overlay generator decompresses the original ROM overlay, slices the
# data region into one initializer fragment per named symbol, and writes those
# fragments to a gitignored src/overlays/<ov>/gen/ directory. The committed
# src/overlays/<ov>/<ov>_data.c holds only the schema (the array declarations)
# and #includes the fragments.
#
# Because the fragments are extracted from the ROM exactly like the splat asm
# and linker scripts, generation is wired into `make splat`. Developers never
# invoke the generators directly; `make splat` produces everything the build
# needs, and a normal `make` compiles the fragments as ordinary sources.
#
# ── How to add generated data for a new overlay ──────────────────────────────
#
#   1. Write tools/gen_<ov>_data.py (see tools/gen_gname_data.py).
#   2. Add <ov> to OVERLAY_DATA below and define overlay_<ov>_data_gen.
#   3. Commit src/overlays/<ov>/<ov>_data.c (the schema) and route it in
#      mk/overlay-registry.mk. Ensure src/overlays/*/gen/ stays gitignored.
#
# ─────────────────────────────────────────────────────────────────────────────

# Overlays that build embedded ROM data from C via a generator script.
# Each entry <ov> must define overlay_<ov>_data_gen with its generator command.
OVERLAY_DATA := gname title

overlay_gname_data_gen := python3 tools/gen_gname_data.py
overlay_title_data_gen := python3 tools/gen_title_data.py

.PHONY: overlay-data $(addprefix overlay-data-,$(OVERLAY_DATA))

overlay-data: $(addprefix overlay-data-,$(OVERLAY_DATA))
	@echo "Overlay data fragments generated."

$(addprefix overlay-data-,$(OVERLAY_DATA)): overlay-data-%:
	@echo "Generating $* overlay data fragments..."
	$(overlay_$*_data_gen)

# Regenerate overlay data whenever splat re-extracts from the ROM. This adds a
# prerequisite to the `splat` target defined in mk/splat.mk without redefining
# its recipe, so `make splat` fills the fragments as part of extraction.
splat: overlay-data
