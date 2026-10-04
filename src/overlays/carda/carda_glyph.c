/* CARDA's copy of the shared glyph-cache text drawing (see include/common/glyph_cache.h). */
#include "internal/carda_glyph.h"
#include "internal/carda_internal.h"

#include "../../common/glyph_cache/draw_signed_decimal.inc.c"
#include "../../common/glyph_cache/draw_hex_byte.inc.c"
#include "../../common/glyph_cache/draw_cached_text.inc.c"
#include "../../common/glyph_cache/render_cached_glyph.inc.c"
#include "../../common/glyph_cache/emit_glyph_sprite.inc.c"
#include "../../common/glyph_cache/begin_glyph_cache_frame.inc.c"
#include "../../common/glyph_cache/evict_unused_glyphs.inc.c"
#include "../../common/glyph_cache/reset_glyph_cache.inc.c"
#include "../../common/glyph_cache/expand_text_glyph_codes.inc.c"
