#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14; /* 0x14 */
    u8 pad18[0x23C - 0x18];
} RecA80105AE0;

typedef struct
{
    s32 unk0; /* 0x0 */
    s32 unk4; /* 0x4 */
    s32 unk8; /* 0x8 */
    u8 padC[0x54 - 0xC];
} RecB800FDF58;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} OutRec;

extern RecA80105AE0 D_80105AE0[];
extern RecB800FDF58 D_800FDF58[];

/**
 * @see decomp.me (100%)
 */
s32 func_80087F44(s32 arg0, OutRec *arg1)
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
        arg1->unk0 = found->unk0;
        arg1->unk4 = found->unk4;
        arg1->unk8 = found->unk8;
        return 0;
    }
    return -1;
found_it:
    found = rb;
    goto check;
}
