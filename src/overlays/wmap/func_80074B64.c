/* Partial WMAP decompilation: 94.260000% (gcc280_g0). */
#include "common.h"
#include "vector.h"

extern u8 D_800D95AC[];
extern u8 D_80139A20[];
extern s32 D_8011CF4C;
extern s32 D_801B25E4;
extern s32 D_801B25B8;
extern s32 D_801B25BC;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);

/**
 * @brief Draw a world-map actor at a nudged copy of the cursor, then decay a timer.
 * @note Packs a per-actor coordinate offset from the cursor halves, renders the
 *       actor, copies a shared timer into two object fields, and steps counters.
 * @note Best match ~94.3% (gcc280_g0); residual is a callee-saved register tie
 *       (obj base vs packed pos land in s0/s1 swapped). The dead Vec2s reproduces
 *       the original -0x30 frame reservation (FRAME-01).
 */
void func_80074B64(void)
{
    Vec2s scratch;
    s32 pos;

    pos = ((*(u16*)&D_8011CF4C - 0x18) & 0xFFFF) | ((*(u16*)((u8*)&D_8011CF4C + 0x2) - 0xA) << 16);
    func_8006CC4C(D_800D95AC, D_80139A20);
    func_80066F9C(D_800D95AC, pos, 0x4, 0xB, 0);
    *(s16*)(&D_800D95AC[0x24]) = *(u16*)&D_801B25E4;
    *(s16*)(&D_800D95AC[0x22]) = *(u16*)&D_801B25E4;
    D_801B25E4 -= 0x8;
    if (D_801B25E4 < 0)
    {
        D_801B25E4 = 0;
    }
    if (--D_801B25BC == 0)
    {
        D_801B25B8 += 1;
    }
}
