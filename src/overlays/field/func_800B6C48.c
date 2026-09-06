#include "common.h"

extern u8 *D_80123FB0;
extern void func_800B729C(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
extern s32 func_800B742C(s32 arg0, s32 arg1);
extern s32 func_800B78C0(void);

/**
 * @brief Handler 3 of D_800F0B98: roll a random offset into the entry's byte-2
 *        index, scale it by a nested resource factor, and run the
 *        func_800B729C .. func_800B742C chain.
 *
 * Near-clone of func_800B6890 and func_800B6B28 (the other decoded handlers in
 * this table): the entry's packed word at +4 supplies byte 2 as a base index
 * and byte 3 as a modulus for rand(). When byte 3 is zero it is forced to 1
 * (with the packed word retagged into the 0x1000000 range) before the modulo,
 * avoiding a divide-by-zero.
 *
 * @return func_800B742C's result.
 */
s32 func_800B6C48(void)
{
    s32 sp10;
    s32 sp14;
    u32 packed;
    s32 remainder;
    s32 factor;
    s32 idx;
    s32 product;
    s32 ret;

    packed = *(u32 *)(*(u8 **)(D_80123FB0 + 0x1C) + 4);
    if ((packed >> 24) == 0)
    {
        packed &= 0xFFFFFF;
        packed |= 0x1000000;
    }

    remainder = rand() % (s32)(packed >> 24);

    factor = *(s32 *)(*(s32 *)(*(u8 **)(D_80123FB0 + 0x20) + 0x10) + 4);
    idx = ((packed >> 16) & 0xFF) + remainder;
    product = factor * idx;

    sp14 = 0;
    sp10 = (u32)product >> 4;
    func_800B729C(0, 0, &sp10, &sp14);
    ret = func_800B742C(sp10, sp14);
    func_800B78C0();
    return ret;
}
