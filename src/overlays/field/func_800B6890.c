#include "common.h"

extern u8 *D_80123FB0;

extern void func_800B2B54(s32 arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern void func_800B70F4(s32 arg0, s32 *out);
extern void func_800B7164(s32 arg0, s32 *out);
extern void func_800B729C(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
extern s32 func_800B742C(s32 arg0, s32 arg1);
extern void func_800B78C0(void);

/**
 * @brief Unpack the current field opcode word and drive its handler chain.
 *
 * Reads the packed word at D_80123FB0->unk1C->unk4, splits it into nibbles and
 * a byte, and feeds func_800B70F4/7164/729C/742C, then conditionally
 * func_800B2B54, and finally func_800B78C0.
 *
 * WIP 98.58% (gcc280_g0). Residue is pure ALLOC-ORDER: the saved-register
 * assignment for `packed` vs the &sp18/&sp1C address temps is permuted (target
 * packed->s1, ours ->s4). The only known 100% path adds a redundant value-copy
 * of `packed` (a split_tmp) to shift greg's numbering; that is scaffolding, so
 * it is not applied here. Struct-typed access, pointer intermediates, and
 * declaration reordering were all measured inert.
 *
 * @return func_800B742C's result.
 */
s32 func_800B6890(void)
{
    s32 sp18;
    s32 sp1C;
    u32 packed;
    s32 byte8;
    s32 ret;

    packed = *(u32 *)(*(u8 **)(D_80123FB0 + 0x1C) + 4);
    func_800B70F4(packed & 0xF, &sp18);
    func_800B7164((packed >> 4) & 0xF, &sp1C);
    byte8 = (packed >> 8) & 0xFF;
    func_800B729C(0, byte8, &sp18, &sp1C);
    ret = func_800B742C(sp18, sp1C);
    if ((*(u8 *)(*(u8 **)(D_80123FB0 + 0x24) + 0x39) & byte8) ||
        ((**(u32 **)(D_80123FB0 + 0x1C) & 0xF) != 2))
    {
        func_800B2B54(
            *(s32 *)(D_80123FB0 + 0x20),
            *(void **)(D_80123FB0 + 0x24),
            0,
            (packed >> 0x14) & 0xF,
            (((packed >> 0x10) & 0xF) + 1) * 0x10,
            (packed >> 0x18) * 0x10);
    }
    func_800B78C0();
    return ret;
}
