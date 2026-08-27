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
    u8 data[0x54];
} Struct_D800FDF58;

extern Struct_D80105AE0 D_80105AE0[];
extern Struct_D800FDF58 D_800FDF58[];
void func_8008BE38(Struct_D800FDF58 *, s32);

/**
 * @brief Finds the actor slot matching @p key and activates its record.
 *
 * Scans the first 13 D_80105AE0 slots for one whose 0x14 field equals @p key.
 * On a hit, activates the parallel D_800FDF58 record (func_8008BE38 with 1) and
 * returns 0; if no slot matches, returns -1.
 */
s32 func_8008AD44(s32 key)
{
    Struct_D800FDF58 *p = D_800FDF58;
    Struct_D80105AE0 *e = D_80105AE0;
    s32 i;

    i = 0;
    while (i < 13)
    {
        if (e->unk14 == key)
        {
            goto found;
        }
        i++;
        e++;
        p++;
    }
    p = (Struct_D800FDF58 *)-1;
found:
    if (p == (Struct_D800FDF58 *)-1)
    {
        return -1;
    }
    func_8008BE38(p, 1);
    return 0;
}
