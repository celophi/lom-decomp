#include "common.h"

#define FB0_BYTES ((u8 *)D_80123FB0)

extern u8 *D_80123FB0;
void func_800B70F4(s32 arg0, s32 *out);
void func_800B7164(s32 arg0, s32 *out);
void func_800B729C(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
s32 func_800B742C(s32 arg0, s32 arg1);
void func_800B78C0(void);

/**
 * @brief Apply the packed field-state operation selected by the active state block.
 * @return Result returned by func_800B742C.
 */
s32 func_800B6EC0(void)
{
    s32 sp10;
    s32 sp14;
    s32 *p10;
    s32 *p14;
    u32 packed;
    u32 packed_tail;
    s32 byte8;
    s32 ret;
    u8 *p;

    p10 = &sp10;
    p14 = &sp14;
    packed = *(u32 *)(*(u8 **)(FB0_BYTES + 0x1C) + 4);
    packed_tail = packed;
    func_800B70F4(packed & 0xF, p10);
    func_800B7164((packed >> 4) & 0xF, p14);
    func_800B729C(0, 0, p10, p14);
    ret = func_800B742C(sp10, sp14);
    func_800B78C0();
    byte8 = (packed >> 8) & 0xFF;
    switch (byte8)
    {
    case 0:
        *(s16 *)(*(u8 **)(*(u8 **)(FB0_BYTES + 0x24) + 0x10) + 0x48) = 0;
        break;
    case 1:
        p = *(u8 **)(FB0_BYTES + 0x24);
        *(s32 *)(p + 0xC) |= (packed_tail >> 0x10) << 0x10;
        break;
    case 2:
        p = *(u8 **)(FB0_BYTES + 0x24);
        *(s32 *)(p + 0xC) |= packed_tail >> 0x10;
        break;
    case 3:
        p = *(u8 **)(FB0_BYTES + 0x24);
        *(u8 *)(p + 0x39) |= packed_tail >> 0x10;
        break;
    default:
        return ret;
    }
    return ret;
}
