#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} Struct_D80105AE0;

typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} Struct_D800FDF58;

extern Struct_D80105AE0 D_80105AE0[];
extern Struct_D800FDF58 D_800FDF58[];
extern s32 D_80105880[];

/**
 * @brief Finds the track value associated with the actor slot matching @p key.
 *
 * Scans the first 13 actor slots in parallel with D_800FDF58. On a hit, the
 * record's track selector at 0x3A chooses one of the three 0x1C-byte entries
 * in D_80105880; selectors >= 2 clamp to the third entry.
 *
 * @note gcc272_cdk, 100% match.
 */
s32 func_8008B398(s32 key)
{
    Struct_D800FDF58 *scan;
    Struct_D800FDF58 *found;
    Struct_D80105AE0 *e;
    s32 i;
    s32 offset;
    s32 result;
    u8 *base;

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
    found = (Struct_D800FDF58 *)-1;
check:
    if (found != (Struct_D800FDF58 *)-1)
        goto lookup;
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
lookup:
    base = (u8 *)D_80105880;
    if (found->unk3A < 2)
        offset = found->unk3A * 0x1C;
    else
        offset = 0x38;
    result = *(s32 *)(base + offset);
done:
    return result;
}
