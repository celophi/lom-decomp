#include "common.h"

extern u8 D_800FDF58;
extern u8 D_80105AE0;
extern u8 *g_pad_ctx;

/**
 * @brief Bump a pad-slot counter and clear an actor slot's animation bits.
 *
 * Uses the D_800FDF58 entry for @p arg0 (stride 0x54) to index into the pad
 * context and increment a per-controller counter, dispatches func_800C2640 for
 * the actor slot's @c unk14 handle, marks the pad entry served (0xFF), and
 * clears the actor slot's @c unk18E byte.
 *
 * @param arg0 Actor/pad slot index.
 * @see decomp.me (100%) TODO
 */
void func_800AF824(s32 arg0)
{
    u8 *s1;
    u8 *v1;
    u8 *s0;
    u8 *pc = g_pad_ctx;
    u8 *base = &D_800FDF58;

    s1 = base + arg0 * 0x54;
    v1 = pc + (s1[0x21] & 0x7F);
    v1[0x2640] = v1[0x2640] + 1;
    s0 = &D_80105AE0 + arg0 * 0x23C;
    func_800C2640(*(s32 *)(s0 + 0x14), 0xFF);
    s1[0x25] = 0xFF;
    s0[0x18E] = 0;
}
