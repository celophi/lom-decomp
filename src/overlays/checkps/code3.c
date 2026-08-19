#include "checkps.h"
/**
 * @brief Renders a Shift-JIS encoded string to VRAM by uploading each glyph via DrawKanjiGlyph.
 *
 * @details Walks @p text as Shift-JIS, consuming two bytes for each non-newline character, combining each pair into a 16-bit Shift-JIS
 * character code and passing the result to Krom2RawAdd() to retrieve the glyph bitmap from
 * the PS1 Kanji ROM. Each glyph is drawn at the current drawState position and the x cursor
 * is advanced by 17px. Newline bytes (0x0A) reset x to its original value and advance y by
 * 18px instead of drawing a glyph.
 *
 * drawState->position x/y are not restored on exit; the caller's draw cursor is left
 * at the end of the last line rendered.
 *
 * @param text      Null-terminated Shift-JIS string. Newlines are single-byte 0x0A values.
 * @param drawState VRAM destination position and glyph size. x and y are updated in place
 *                  as each character is drawn.
 * @param color     Foreground pixel value passed to DrawKanjiGlyph for each glyph.
 *
 * @return void No return value.
 *
 */
void DrawKanjiString(const char* text, KanjiDrawState* drawState, s32 color)
{
    const char* end;
    const char* textEnd;
    s32 isNewline;
    s32 savedX;
    s32 highByte;
    s32 glyphColor;
    u16 characterCode;
    s32 newline;
    glyphColor = color;
    end = text + strlen(text);
    newline = 10;
    savedX = drawState->position.coord.x;
    if (text < end)
    {
        textEnd = end;
        do
        {
            isNewline = (*((u8*)text)) == newline;
            if (isNewline)
            {
                drawState->position.coord.x = savedX;
                drawState->position.coord.y += 18;
            }
            else
            {
                highByte = *((u8*)text);
                text++;
                characterCode = (highByte << 8) | (*((u8*)text));
                DrawKanjiGlyph(drawState, (u8*)Krom2RawAdd(characterCode), glyphColor);
                drawState->position.coord.x += 17;
            }
            text++;
        } while (text < textEnd);
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
 * drawState->position x/y are restored on exit, so the caller's draw cursor
 * is not modified.
 *
 * @param drawState VRAM destination and size for the upload. x and y are used as the
 *                  target coordinates; packedSize is passed directly to the GPU packet.
 * @param bitmap    Pointer to the raw 1bpp glyph data (2 bytes per row, 15 rows).
 *                  Typically obtained from Krom2RawAdd() for kanji/kana characters.
 * @param color     Foreground pixel value written for set bits. Clear bits write 0.
 *
 * @return void No return value.
 *
 */
void DrawKanjiGlyph(KanjiDrawState* drawState, u8* bitmap, s32 color)
{
    struct
    {
        s32 tag;
        s32 code;
        s32 xy;
        s32 wh;
        s16 pixels[16];
    } packet;
    s32 originalX;
    s32 originalY;
    s32 row;
    s32 pass;
    s16* writePtr;
    s16* pixelPtr;
    s32 bit;
    s16 pixelValue;
    u8* source = bitmap;
    s16 foregroundColor = color;

    // Save position so DrawKanjiGlyph leaves drawState unchanged after upload
    originalX = drawState->position.coord.x;
    originalY = drawState->position.coord.y;

    // GPU CPU→VRAM (LoadImage) packet: tag=11 following words, code=0xA0
    packet.tag = 0x0B000000;
    packet.code = 0xA0000000;
    packet.wh = drawState->packedSize;
    for (row = 0; row < 15; row++)
    {
        writePtr = packet.pixels;

        // Expand 2 bytes (16 bits) of 1bpp bitmap into 16 pixel values
        for (pass = 0; pass < 2; pass++)
        {
            for (bit = 7; bit >= 0; bit--)
            {
                pixelPtr = writePtr;
                writePtr = pixelPtr + 1;
                pixelValue = 0;

                if ((*source >> bit) & 1)
                {
                    pixelValue = foregroundColor;
                }
                *pixelPtr = pixelValue;
            }

            source++;
        }

        // Upload the row twice one pixel apart to thicken the glyph horizontally
        for (pass = 0; pass < 2; pass++)
        {
            packet.xy = drawState->position.packed;
            DrawPrim(&packet);

            drawState->position.coord.x++;
        }

        drawState->position.coord.x = originalX;
        drawState->position.coord.y++;
    }

    drawState->position.coord.y = originalY;
}
