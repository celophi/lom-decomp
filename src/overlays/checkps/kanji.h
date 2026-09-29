#ifndef CHECKPS_KANJI_H
#define CHECKPS_KANJI_H

#include "checkps.h"

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

void draw_kanji_string(const char* text, KanjiDrawState* draw_state, s32 color);

#endif
