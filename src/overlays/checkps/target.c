#include "checkps.h"

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
