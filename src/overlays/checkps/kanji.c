#include "checkps.h"
/**
 * @brief Renders a Shift-JIS encoded string to VRAM by uploading each glyph via draw_kanji_glyph.
 *
 * @details Walks @p text as Shift-JIS, consuming two bytes for each non-newline character, combining each pair into a 16-bit Shift-JIS
 * character code and passing the result to Krom2RawAdd() to retrieve the glyph bitmap from
 * the PS1 Kanji ROM. Each glyph is drawn at the current draw_state position and the x cursor
 * is advanced by 17px. Newline bytes (0x0A) reset x to its original value and advance y by
 * 18px instead of drawing a glyph.
 *
 * draw_state->position x/y are not restored on exit; the caller's draw cursor is left
 * at the end of the last line rendered.
 *
 * @param text      Null-terminated Shift-JIS string. Newlines are single-byte 0x0A values.
 * @param draw_state VRAM destination position and glyph size. x and y are updated in place
 *                  as each character is drawn.
 * @param color     Foreground pixel value passed to draw_kanji_glyph for each glyph.
 *
 * @return void No return value.
 *
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
    newline = 10;
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
                draw_state->position.coord.y += 18;
            }
            else
            {
                high_byte = *((u8*)text);
                text++;
                character_code = (high_byte << 8) | (*((u8*)text));
                draw_kanji_glyph(draw_state, (u8*)Krom2RawAdd(character_code), glyph_color);
                draw_state->position.coord.x += 17;
            }
            text++;
        } while (text < text_end);
    }
}
/**
 * @brief Uploads a single 1bpp glyph row-by-row to VRAM using GPU LoadImage packets.
 *
 * @details For each of the 15 scanlines in the glyph, reads 2 bytes (16 bits) from
 * the raw bitmap and expands each bit into a 16-bit pixel: set bits become color,
 * clear bits become 0. The decoded row is then sent to VRAM twice via DrawPrim,
 * advancing x by one pixel between the two uploads. The duplicate upload thickens
 * each scanline horizontally; after each row, x is restored and y advances by one.
 *
 * draw_state->position x/y are restored on exit, so the caller's draw cursor
 * is not modified.
 *
 * @param draw_state VRAM destination and size for the upload. x and y are used as the
 *                  target coordinates; packed_size is passed directly to the GPU packet.
 * @param bitmap    Pointer to the raw 1bpp glyph data (2 bytes per row, 15 rows).
 *                  Typically obtained from Krom2RawAdd() for kanji/kana characters.
 * @param color     Foreground pixel value written for set bits. Clear bits write 0.
 *
 * @return void No return value.
 *
 */
void draw_kanji_glyph(KanjiDrawState* draw_state, u8* bitmap, s32 color)
{
    struct
    {
        s32 tag;
        s32 code;
        s32 xy;
        s32 wh;
        s16 pixels[16];
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
    packet.tag = 0x0B000000;
    packet.code = 0xA0000000;
    packet.wh = draw_state->packed_size;
    for (row = 0; row < 15; row++)
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
