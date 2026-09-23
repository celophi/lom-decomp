#include "checkps_internal.h"

#include "sdk/libapi.h"
#include "sdk/strings.h"

#define CHECKPS_KANJI_LINE_HEIGHT 18
#define CHECKPS_KANJI_ADVANCE 17
#define CHECKPS_KANJI_PIXELS_PER_ROW 16
/* Words following the tag in the glyph-row upload packet: command, xy, wh and 8 pixel words. */
#define CHECKPS_KANJI_PACKET_WORDS 11
/* GP0(A0h): copy a rectangle from CPU to VRAM. */
#define CHECKPS_GPU_LOAD_IMAGE_COMMAND 0xA0000000

void draw_kanji_glyph(KanjiDrawState* draw_state, u8* bitmap, s32 color);

/**
 * @brief Draw a Shift-JIS string using glyphs from the PS1 Kanji ROM.
 * @param text Null-terminated Shift-JIS text; newline resets the x cursor.
 * @param draw_state VRAM cursor and glyph dimensions, updated while drawing.
 * @param color Foreground pixel value.
 */
void draw_kanji_string(const char* text, KanjiDrawState* draw_state, s32 color)
{
    const u8* cursor;
    const u8* end;
    const u8* loop_end;
    s32 is_newline;
    s32 line_start_x;
    s32 high_byte;
    s32 glyph_color;
    u16 character_code;
    s32 newline;

    glyph_color = color;
    end = (const u8*)text + strlen(text);
    newline = '\n';
    cursor = (const u8*)text;
    line_start_x = draw_state->position.coord.x;
    /* The color, newline and loop_end copies reproduce the original register usage. */
    if (cursor < end)
    {
        loop_end = end;
        do
        {
            is_newline = *cursor == newline;
            if (is_newline)
            {
                draw_state->position.coord.x = line_start_x;
                draw_state->position.coord.y += CHECKPS_KANJI_LINE_HEIGHT;
            }
            else
            {
                high_byte = *cursor;
                cursor++;
                character_code = (high_byte << 8) | *cursor;
                draw_kanji_glyph(draw_state, (u8*)Krom2RawAdd(character_code), glyph_color);
                draw_state->position.coord.x += CHECKPS_KANJI_ADVANCE;
            }
            cursor++;
        } while (cursor < loop_end);
    }
}

/**
 * @brief Expand and upload one 1bpp Kanji-ROM glyph to VRAM.
 * @param draw_state VRAM destination and glyph dimensions.
 * @param bitmap Raw 1bpp glyph bitmap.
 * @param color Foreground pixel value.
 */
void draw_kanji_glyph(KanjiDrawState* draw_state, u8* bitmap, s32 color)
{
    struct
    {
        u32 tag;
        u32 command;
        u32 xy;
        u32 wh;
        s16 pixels[CHECKPS_KANJI_PIXELS_PER_ROW];
    } packet;
    s32 original_x;
    s32 original_y;
    s32 row;
    s32 pass;
    s16* write_ptr;
    s32 bit;

    /* Preserve the caller's position across the glyph upload. */
    original_x = draw_state->position.coord.x;
    original_y = draw_state->position.coord.y;

    packet.tag = CHECKPS_KANJI_PACKET_WORDS << 24;
    packet.command = CHECKPS_GPU_LOAD_IMAGE_COMMAND;
    packet.wh = draw_state->size.packed;
    for (row = 0; row < CHECKPS_GLYPH_BITMAP_ROWS; row++)
    {
        write_ptr = packet.pixels;

        /* Expand two 1bpp source bytes into 16 pixel values. */
        for (pass = 0; pass < 2; pass++)
        {
            for (bit = 7; bit >= 0; bit--)
            {
                *write_ptr++ = ((*bitmap >> bit) & 1) ? color : 0;
            }

            bitmap++;
        }

        /* Upload twice one pixel apart to thicken the row horizontally. */
        for (pass = 0; pass < 2; pass++)
        {
            packet.xy = draw_state->position.packed;
            DrawPrim(&packet);

            draw_state->position.coord.x++;
        }

        draw_state->position.coord.x = original_x;
        draw_state->position.coord.y++;
    }

    draw_state->position.coord.y = original_y;
}
