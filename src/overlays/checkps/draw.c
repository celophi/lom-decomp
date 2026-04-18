#include "checkps.h"

/**
 * @brief Renders a Shift-JIS encoded string to VRAM by uploading each glyph via DrawGlyph.
 *
 * @details Walks @p str two bytes at a time, combining each pair into a 16-bit Shift-JIS
 * character code and passing the result to Krom2RawAdd() to retrieve the glyph bitmap from
 * the PS1 Kanji ROM. Each glyph is drawn at the current drawState position and the x cursor
 * is advanced by 17px. Newline bytes (0x0A) reset x to its original value and advance y by
 * 18px instead of drawing a glyph.
 *
 * drawState->x and drawState->y are not restored on exit; the caller's draw cursor is left
 * at the end of the last line rendered.
 *
 * @param str       Null-terminated Shift-JIS string to render. Each character is 2 bytes.
 * @param drawState VRAM destination position and glyph size. x and y are updated in place
 *                  as each character is drawn.
 * @param color     Foreground pixel value passed to DrawGlyph for each glyph.
 *
 * @return void No return value.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/cjji6
 */
void DrawString(const char* str, GlyphDrawState* drawState, s32 color)
{
    const char* end;
    const char* strEnd;
    s32 isNewline;
    s32 savedX;
    s32 highByte;
    s32 localColor;
    u16 charCode;
    s32 newline;

    localColor = color;
    end = str + strlen(str);
    newline = 10;
    savedX = drawState->pos.coord.x;

    if (str < end)
    {
        strEnd = end;
        do
        {
            isNewline = (*((u8*)str)) == newline;

            // This is done for matching purposes.
            drawState->pos.coord = drawState->pos.coord;
            
            if (isNewline)
            {
                drawState->pos.coord.x = savedX;
                drawState->pos.coord.y += 18;
            }
            else
            {
                highByte = *((u8*)str);
                str++;
                charCode = (highByte << 8) | (*((u8*)str));
                DrawGlyph(drawState, Krom2RawAdd(charCode), localColor);
                drawState->pos.coord.x += 17;
            }
            str++;
        } while (str < strEnd);
    }
}

/**
 * @brief Uploads a single 1bpp glyph row-by-row to VRAM using GPU LoadImage packets.
 *
 * @details For each of the 15 scanlines in the glyph, reads 2 bytes (16 bits) from
 * the raw bitmap and expands each bit into a 16-bit pixel: set bits become color,
 * clear bits become 0. The decoded row is then sent to VRAM twice via DrawPrim,
 * advancing drawState->x by 1 between calls to write into both VRAM atlas slots.
 * After each row, x is reset to its original value and y is incremented by 1.
 *
 * drawState->x and drawState->y are restored on exit, so the caller's draw cursor
 * is not modified.
 *
 * @param drawState VRAM destination and size for the upload. x and y are used as the
 *                  target coordinates; wh is passed directly to the GPU packet.
 * @param bitmap    Pointer to the raw 1bpp glyph data (2 bytes per row, 15 rows).
 *                  Typically obtained from Krom2RawAdd() for kanji/kana characters.
 * @param color     Foreground pixel value written for set bits. Clear bits write 0.
 *
 * @return void No return value.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/PIDIi
 */
void DrawGlyph(GlyphDrawState* drawState, u8* bitmap, s32 color)
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
    s32 half;
    s16* writePtr;
    s16* pixelPtr;
    s32 bit;
    s16 pixelValue;

    u8* bitmapPtr = bitmap;
    s16 fgColor = color;

    // Save position so DrawGlyph leaves drawState unchanged after upload
    originalX = drawState->pos.coord.x;
    originalY = drawState->pos.coord.y;

    // GPU CPU→VRAM (LoadImage) packet: tag=11 following words, code=0xA0
    packet.tag = 0x0B000000;
    packet.code = 0xA0000000;
    packet.wh = drawState->wh;

    for (row = 0; row < 15; row++)
    {
        writePtr = packet.pixels;

        // Expand 2 bytes (16 bits) of 1bpp bitmap into 16 pixel values
        for (half = 0; half < 2; half++)
        {
            for (bit = 7; bit >= 0; bit--)
            {
                pixelPtr = writePtr;
                writePtr = pixelPtr + 1;
                pixelValue = 0;

                if (((*bitmapPtr) >> bit) & 1)
                {
                    // This is done for matching purposes.
                    color++;
                    color--;

                    pixelValue = fgColor;
                }

                *pixelPtr = pixelValue;
            }

            bitmapPtr++;
        }

        // Upload the decoded row twice, advancing x by 1, to write into both VRAM atlas slots
        for (half = 0; half < 2; half++)
        {
            packet.xy = drawState->pos.packed;
            DrawPrim(&packet);

            drawState->pos.coord.x++;
        }

        drawState->pos.coord.x = originalX;
        drawState->pos.coord.y++;
    }

    drawState->pos.coord.y = originalY;
}