#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14; /* 0x14 */
    u8 pad18[0x23C - 0x18];
} RecA80105AE0;

typedef struct
{
    u8 data[0x54];
} RecB800FDF58;

extern RecA80105AE0 D_80105AE0[];
extern RecB800FDF58 D_800FDF58[];

RecB800FDF58 *func_80087C9C(s32 arg0)
{
    RecA80105AE0 *ra;
    RecB800FDF58 *rb;
    s32 i;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            return rb;
        }
    }
    return (RecB800FDF58 *) -1;
}
