#include "common.h"

extern u8 *D_80123FB0;
extern void func_800B70F4(s32 arg0, s32 *out);
extern void func_800B7164(s32 arg0, s32 *out);
extern void func_800B729C(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
extern s32 func_800B742C(s32 arg0, s32 arg1);
extern void func_800B78C0(void);
extern void saturating_counter_add(void *counter, s32 delta);

/**
 * @brief Resolve the active packed field operation and update its saturating counter.
 * @return Result returned by func_800B742C, or zero when the active state block is unavailable.
 */
s32 func_800B69B0(void)
{
    s32 sp10;
    s32 sp14;
    u32 packed;
    s32 byte8;
    s32 ret;
    s32 remainder;
    s32 idx;
    s32 product;
    s32 *out1;
    s32 *out2;

    packed = *(u32 *)(*(u8 **)(D_80123FB0 + 0x1C) + 4);
    out1 = &sp10;
    func_800B70F4(packed & 0xF, out1);
    out2 = &sp14;
    func_800B7164((packed >> 4) & 0xF, out2);
    func_800B729C(0, 0, out1, out2);
    ret = func_800B742C(sp10, sp14);

    if (*(s32 *)((u8 *)(*(void **)(*(u8 **)(D_80123FB0 + 0x20) + 0x10)) + 4) == 0)
    {
        return 0;
    }

    byte8 = (packed >> 8) & 0xFF;
    if ((*(u8 *)(*(u8 **)(D_80123FB0 + 0x24) + 0x39) & byte8) ||
        ((*(u32 *)(*(u8 **)(D_80123FB0 + 0x1C)) & 0xF) != 2))
    {
        if ((packed >> 24) == 0)
        {
            packed &= 0xFFFFFF;
            packed |= 0x1000000;
        }

        remainder = rand() % (s32)(packed >> 24);
        idx = ((packed >> 16) & 0xFF) + remainder;
        product = ret * idx;
        saturating_counter_add(*(void **)(*(u8 **)(D_80123FB0 + 0x20) + 0x10), (u32)product >> 7);
    }

    func_800B78C0();
    return ret;
}
