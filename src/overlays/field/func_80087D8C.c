#include "common.h"

/**
 * @brief Per-actor animation/geometry slot; array element stride 0x23C.
 */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} Struct_D80105AE0;

/**
 * @brief Parallel per-actor record; array element stride 0x54.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x54 - 0xC];
} Struct_D800FDF58;

extern Struct_D80105AE0 D_80105AE0[];
extern Struct_D800FDF58 D_800FDF58[];

/**
 * @brief Stores a scaled position into the actor record matching @p key.
 *
 * Scans the first 13 D_80105AE0 slots for one whose 0x14 field equals @p key.
 * On a hit, writes @p x, @p y and @p z (each shifted left 8) into the parallel
 * D_800FDF58 record's first three words and returns 0; otherwise returns -1.
 */
s32 func_80087D8C(s32 key, s32 x, s32 y, s32 z)
{
    Struct_D800FDF58 *p = D_800FDF58;
    Struct_D80105AE0 *e = D_80105AE0;
    Struct_D800FDF58 *result;
    s32 i;

    i = 0;
    while (i < 13)
    {
        if (e->unk14 == key)
        {
            result = p;
            goto found;
        }
        i++;
        e++;
        p++;
    }
    result = (Struct_D800FDF58 *)-1;
found:
    if (result == (Struct_D800FDF58 *)-1)
    {
        return -1;
    }
    result->unk0 = x << 8;
    result->unk4 = y << 8;
    result->unk8 = z << 8;
    return 0;
}
