#ifndef CHECKPS_INTERNAL_H
#define CHECKPS_INTERNAL_H

#include "checkps.h"

/* GPU packet fields shared by the display and glyph renderers. */
#define CHECKPS_GPU_TAG_LENGTH_MASK 0xFF000000
#define CHECKPS_GPU_TAG_ADDRESS_MASK 0x00FFFFFF

/* Glyph geometry shared by the display, cached-font, and Kanji renderers. */
#define CHECKPS_GLYPH_VRAM_X 960
#define CHECKPS_GLYPH_BITMAP_ROWS 15

/**
 * @brief VRAM position and dimensions used while drawing Kanji glyphs.
 */
typedef struct
{
    union
    {
        struct
        {
            s16 x;
            s16 y;
        } coord;
        s32 packed;
    } position;
    union
    {
        struct
        {
            s16 width;
            s16 height;
        } dimensions;
        u32 packed;
    } size;
} KanjiDrawState;

#define CHECKPS_HARDWARE_WARNING_SIZE 60

/**
 * @brief Shift-JIS hardware-modification warning shown before termination.
 *
 * Defined in pattern.c, whose GNU-as .rodata holds the warning, the pattern
 * vertex signs and the assembler's 16-byte section padding.
 */
extern const char g_hardware_modification_warning[CHECKPS_HARDWARE_WARNING_SIZE];

/* Cached-font lifecycle called by the CHECKPS display loop. */
void begin_glyph_cache_frame(void);
void evict_unused_glyphs(void);
void reset_glyph_renderer(void);

/* Hardware-warning rendering implemented in separate translation units. */
void draw_kanji_string(const char* text, KanjiDrawState* draw_state, s32 color);
void draw_hardware_check_pattern(void);

#endif
