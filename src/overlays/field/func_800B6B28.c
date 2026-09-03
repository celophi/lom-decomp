#include "common.h"

extern u8 *D_80123FB0;
extern void func_800B2D64(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern void func_800B70F4(s32 arg0, s32 *out);
extern void func_800B7164(s32 arg0, s32 *out);
extern void func_800B729C(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
extern s32 func_800B742C(s32 arg0, s32 arg1);
extern void func_800B78C0(void);

/**
 * @brief Decode the current packed field command and dispatch its two effects.
 *
 * The source shape mirrors func_800B6890: the two stack outputs remain ordinary
 * locals while a copy of the packed command keeps the long-lived value web used
 * by the later command dispatches.
 *
 * @return The result produced by func_800B742C.
 */
s32 func_800B6B28(void)
{
    s32 sp10;
    s32 sp14;
    u32 packed;
    u32 packed_tail;
    s32 byte8;
    s32 ret;

    packed = *(u32 *)(*(u8 **)(D_80123FB0 + 0x1C) + 4);
    packed_tail = packed;
    func_800B70F4(packed & 0xF, &sp10);
    func_800B7164((packed >> 4) & 0xF, &sp14);
    byte8 = (packed >> 8) & 0xFF;
    func_800B729C(0, byte8, &sp10, &sp14);
    ret = func_800B742C(sp10, sp14);
    if ((*(u8 *)(*(u8 **)(D_80123FB0 + 0x24) + 0x39) & byte8) ||
        ((**(u32 **)(D_80123FB0 + 0x1C) & 0xF) != 2))
    {
        func_800B2D64(
            *(s32 *)(D_80123FB0 + 0x20),
            (packed_tail >> 20) & 0xF,
            (packed_tail >> 16) & 0xF,
            -1);
        func_800B2D64(
            *(s32 *)(D_80123FB0 + 0x24),
            packed_tail >> 28,
            (packed_tail >> 24) & 0xF,
            -1);
    }
    func_800B78C0();
    return ret;
}
