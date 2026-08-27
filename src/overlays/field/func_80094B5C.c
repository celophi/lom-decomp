#include "common.h"

typedef struct
{
    u8 pad0[0x4];
    s32 unk4;  /* 0x04 */
    u8 pad8[0x20 - 0x8];
    u8 unk20;  /* 0x20 */
    u8 pad21[0x26 - 0x21];
    u8 unk26;  /* 0x26 */
    u8 pad27[0x2A - 0x27];
    s16 unk2A; /* 0x2A */
    u8 pad2C[0x30 - 0x2C];
} Struct80094B5C;

/**
 * @brief Advances an actor's countdown and scrolls its 0x04 offset field.
 *
 * Decrements the byte counter at @c unk26 and zeroes @c unk2A when it reaches
 * 0. Then, when @p flag is set, subtracts @c unk20 << 8 from @c unk4; otherwise
 * adds it, and if the sum is non-negative resets @c unk4 and @c unk2A to 0.
 *
 * @param a0 Actor record to update.
 * @param flag Nonzero subtracts the delta from @c unk4; zero adds it (with the
 *             non-negative reset).
 */
void func_80094B5C(Struct80094B5C *a0, s32 flag)
{
    s8 v;

    v = a0->unk26 - 1;
    a0->unk26 = v;
    if (v == 0)
    {
        a0->unk2A = 0;
    }

    if (flag != 0)
    {
        a0->unk4 -= a0->unk20 << 8;
    }
    else
    {
        s32 w = a0->unk4 + (a0->unk20 << 8);
        a0->unk4 = w;
        if (w >= 0)
        {
            a0->unk4 = 0;
            a0->unk2A = 0;
        }
    }
}
