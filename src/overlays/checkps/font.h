#ifndef CHECKPS_FONT_H
#define CHECKPS_FONT_H

#include "checkps.h"

void* draw_signed_decimal(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void draw_hex_byte(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 alignment);
void* draw_cached_text(void* primitive, u_long* ot_tag, const u8* text, s32 x, s32 y, s32 palette, s32 alignment);
void begin_glyph_cache_frame(void);
void evict_unused_glyphs(void);
void reset_glyph_renderer(void);

#endif
