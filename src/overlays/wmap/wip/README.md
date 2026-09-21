# WMAP matching drafts

These C files are unfinished decompilation candidates. Their functions remain in
assembly subsegments in `config/overlays/WMAP.BIN.yaml`; they are not build inputs.

Keep unfinished candidates here, or iterate under `working/<function>/`. To promote
a candidate, confirm a 100% match in the final source, move it into the parent WMAP
source directory, replace its assembly range with a C subsegment, and add its
compiler route in `mk/overlay-registry.mk` in the same change. Remove its entry from
the unmatched catalogue only after promotion.

The overlay build discovers C files directly in `src/overlays/wmap/`. Keeping these
drafts in this subdirectory preserves them in version control without introducing
unrouted production sources or duplicate assembly/C definitions.
