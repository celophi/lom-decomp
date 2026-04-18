#include "checkps.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/cjji6
 * PsyQ 4.3 / gcc 2.8.0
 */
void func_80051830(u32 arg0, void* arg1, s32 arg2)
{
    u32 end;
    int unk0;
    unsigned int new_var2;
    u32 new_var;
    s32 local_arg2 = arg2;
    end = arg0 + strlen((char*)arg0);
    unk0 = ((arg1struct*)arg1)->unk0;
    if (arg0 < end)
    {
        u8 nl = 0x0A;
        (void)nl;

        new_var = end;
        do
        {
            if ((*((u8*)arg0)) == nl)
            {
                ((arg1struct*)arg1)->unk0 = unk0;
                ((arg1struct*)arg1)->unk2 += 0x12;
            }
            else
            {
                int temp_a0 = *((u8*)arg0);
                arg0++;
                DrawGlyph(arg1, (u8*)Krom2RawAdd((*((u8*)arg0)) | (temp_a0 << 8)), local_arg2);
                ((arg1struct*)arg1)->unk0 =
                    (end = (new_var2 = (s16)(((0x11 * 0, (u16)((arg1struct*)arg1)->unk0)) + 0x11)));
            }
            arg0++;
        } while (arg0 < new_var);
    }
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/PIDIi
 * matches under Psy-Q 4.3 / gcc 2.8.0
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
    s16 local_arg2 = color;

    originalX = drawState->pos.coord.x;
    originalY = drawState->pos.coord.y;

    packet.tag = 0x0B000000;
    packet.code = 0xA0000000;
    packet.wh = drawState->wh;

    for (row = 0; row < 15; row++)
    {
        writePtr = packet.pixels;

        for (half = 0; half < 2; half++)
        {
            for (bit = 7; bit >= 0; bit--)
            {
                pixelPtr = writePtr;
                writePtr = pixelPtr + 1;
                pixelValue = 0;

                if ((*bitmapPtr >> bit) & 1)
                {
                    pixelValue = local_arg2;
                }

                *pixelPtr = pixelValue;
            }

            bitmapPtr++;
        }

        for (half = 0; half < 2; half++)
        {
            packet.xy = drawState->pos.packed;
            DrawPrim(&packet);

            drawState->pos.coord.x = drawState->pos.coord.x + 1;
        }

        drawState->pos.coord.x = originalX;
        drawState->pos.coord.y = drawState->pos.coord.y + 1;
    }

    drawState->pos.coord.y = originalY;
}