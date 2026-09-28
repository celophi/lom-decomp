/* ADDHERO's copy of the shared glyph-cache text drawing (see include/glyph_cache.h). */
#include "addhero_internal.h"

#include "../common/draw_signed_decimal.inc.c"
#include "../common/draw_hex_byte.inc.c"
#include "../common/draw_cached_text.inc.c"
#include "../common/render_cached_glyph.inc.c"
#include "../common/emit_glyph_sprite.inc.c"
#include "../common/begin_glyph_cache_frame.inc.c"
#include "../common/evict_unused_glyphs.inc.c"
#include "../common/reset_glyph_cache.inc.c"
#include "../common/expand_text_glyph_codes.inc.c"
