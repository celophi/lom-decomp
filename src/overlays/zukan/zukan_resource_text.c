#include "common.h"

extern u8 *D_80157D6C;

s32 func_801428F0(s32 arg0, s32 arg1)
{
    s32 i;
    s32 x;
    u16 *offsets;
    u16 *base_offsets;
    u8 *text;
    volatile u8 text_buffer[0x100];

    base_offsets = (u16 *)(D_80157D6C + *(s32 *)(D_80157D6C + 8));
    offsets = base_offsets;
    x = 0x1A;
    i = 0;
    do
    {
        s32 width;

        text = (u8 *)base_offsets + *offsets;
        width = 0x30;
        if (*text == 0x20)
        {
            u8 inner_space = 0x20;

            do
            {
                text++;
                width += 0xC;
            } while (*text == inner_space);
        }
        arg0 = func_800A88A0(arg0, arg1, text, 0, width, x, 0);
        offsets++;
        i++;
        x += 0xD;
    } while (i < 0xC);
    return arg0;
}
