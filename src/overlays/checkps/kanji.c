#include "checkps_internal.h"
/**
 * @brief Draw a Shift-JIS string using glyphs from the PS1 Kanji ROM.
 * @param text Null-terminated Shift-JIS text; newline resets the x cursor.
 * @param draw_state VRAM cursor and glyph dimensions, updated while drawing.
 * @param color Foreground pixel value.
 */
void draw_kanji_string(const char* text, KanjiDrawState* draw_state, s32 color)
{
    const char* end;
    const char* text_end;
    s32 is_newline;
    s32 saved_x;
    s32 high_byte;
    s32 glyph_color;
    u16 character_code;
    s32 newline;
    glyph_color = color;
    end = text + strlen(text);
    newline = '\n';
    saved_x = draw_state->position.coord.x;
    if (text < end)
    {
        text_end = end;
        do
        {
            is_newline = (*((u8*)text)) == newline;
            if (is_newline)
            {
                draw_state->position.coord.x = saved_x;
                draw_state->position.coord.y += CHECKPS_KANJI_LINE_HEIGHT;
            }
            else
            {
                high_byte = *((u8*)text);
                text++;
                character_code = (high_byte << 8) | (*((u8*)text));
                draw_kanji_glyph(draw_state, (u8*)Krom2RawAdd(character_code), glyph_color);
                draw_state->position.coord.x += CHECKPS_KANJI_ADVANCE;
            }
            text++;
        } while (text < text_end);
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
        s32 tag;
        s32 code;
        s32 xy;
        s32 wh;
        s16 pixels[CHECKPS_KANJI_PIXELS_PER_ROW];
    } packet;
    s32 original_x;
    s32 original_y;
    s32 row;
    s32 pass;
    s16* write_ptr;
    s16* pixel_ptr;
    s32 bit;
    s16 pixel_value;
    u8* source = bitmap;
    s16 foreground_color = color;

    // Save position so draw_kanji_glyph leaves draw_state unchanged after upload
    original_x = draw_state->position.coord.x;
    original_y = draw_state->position.coord.y;

    // GPU CPU→VRAM (LoadImage) packet: tag=11 following words, code=0xA0
    packet.tag = CHECKPS_KANJI_GPU_TAG;
    packet.code = CHECKPS_KANJI_GPU_LOAD_IMAGE;
    packet.wh = draw_state->packed_size;
    for (row = 0; row < CHECKPS_GLYPH_BITMAP_ROWS; row++)
    {
        write_ptr = packet.pixels;

        // Expand 2 bytes (16 bits) of 1bpp bitmap into 16 pixel values
        for (pass = 0; pass < 2; pass++)
        {
            for (bit = 7; bit >= 0; bit--)
            {
                pixel_ptr = write_ptr;
                write_ptr = pixel_ptr + 1;
                pixel_value = 0;

                if ((*source >> bit) & 1)
                {
                    pixel_value = foreground_color;
                }
                *pixel_ptr = pixel_value;
            }

            source++;
        }

        // Upload the row twice one pixel apart to thicken the glyph horizontally
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
