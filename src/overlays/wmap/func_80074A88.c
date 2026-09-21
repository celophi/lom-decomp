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
 * @brief Draw a world-map actor, seeding its position from a wrapping timer.
 * @note Copies the shared timer low half into two object fields, renders the
 *       actor at a cursor-relative offset, advances the timer and wraps it.
 * @note The dead Vec2s reproduces the original -0x30 frame reservation (FRAME-01).
 * @note Best match ~77% (gcc280_g0); residual is a callee-saved register tie
 *       (obj vs pos in s0/s1) plus a scheduling order difference on the timer read.
 */
void func_80074A88(void)
{
    u8* obj = D_800D95AC;
    Vec2s scratch;
    s32 pos;

    pos = ((*(u16*)&D_8011CF4C - 0x18) & 0xFFFF) | ((*(u16*)((u8*)&D_8011CF4C + 0x2) - 0xA) << 16);
    *(s16*)(&obj[0x24]) = *(u16*)&D_801B25E4;
    *(s16*)(&obj[0x22]) = *(u16*)&D_801B25E4;
    func_8006CC4C(obj, D_80139A20);
    func_80066F9C(obj, pos, 0x4, 0xB, 0);
    D_801B25E4 += 0x8;
    if (D_801B25E4 >= 0x82)
    {
        D_801B25E4 = 0x81;
    }
    if (--D_801B25BC == 0)
    {
        D_801B25B8 += 1;
    }
}
