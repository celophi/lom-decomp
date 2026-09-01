#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} Struct_D80105AE0_87CE0;

typedef struct
{
    u8 pad0[0x28];
    u8 unk28;
    u8 pad29;
    s16 unk2A;
    s16 unk2C;
    u8 pad2E[0x54 - 0x2E];
} Struct_D800FDF58_87CE0;

extern Struct_D80105AE0_87CE0 D_80105AE0[];
extern Struct_D800FDF58_87CE0 D_800FDF58[];

s32 func_80087CE0(s32 key, u8 value)
{
    Struct_D800FDF58_87CE0 *scan;
    Struct_D800FDF58_87CE0 *found;
    Struct_D80105AE0_87CE0 *e;
    s32 i;
    s32 result;
    s16 state;

    scan = D_800FDF58;
    e = D_80105AE0;
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
        goto found_label;
    e++;
    scan++;
    if (i < 13)
        goto loop;
    found = (Struct_D800FDF58_87CE0 *)-1;
check:
    if (found != (Struct_D800FDF58_87CE0 *)-1)
        goto body;
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
body:
    state = found->unk2A;
    if ((u16)(state - 0x93) < 2)
    {
        result = -1;
        goto done;
    }
    if (state == 0x90 || state == 0xAE || state == 0x8E)
    {
        result = -1;
        goto done;
    }
    found->unk28 = value;
    found->unk2C = 0;
    found->unk2A = 0;
    result = 0;
done:
    return result;
}
