#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14; /* 0x14 */
    u8 pad18[0x23C - 0x18];
} RecA80105AE0;

typedef struct
{
    u8 pad0[0x21];
    u8 unk21; /* 0x21 */
    u8 pad22[0x54 - 0x22];
} RecB800FDF58;

extern RecA80105AE0 D_80105AE0[];
extern RecB800FDF58 D_800FDF58[];

s32 func_8008ADB4(s32 arg0)
{
    RecA80105AE0 *ra;
    RecB800FDF58 *rb;
    RecB800FDF58 *found;
    s32 i;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (RecB800FDF58 *) -1;
check:
    if (found != (RecB800FDF58 *) -1)
    {
        return found->unk21 & 0x7F;
    }
    return -1;
found_it:
    found = rb;
    goto check;
}
