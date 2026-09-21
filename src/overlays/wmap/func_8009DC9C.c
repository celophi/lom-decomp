#include "common.h"

extern u16 D_801B24A8;
extern u8 D_80182DC0;
extern s32 D_8011CF28;
extern s32 D_8013923C;
extern s32 D_80182DEC;
extern s32 D_801B2D18;
extern s32 D_801B2D1C;
extern void func_8006CFA8(void *a, void *b);
extern void func_800675F0(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5,
                          s32 a6, s32 a7, s32 a8, s32 a9);

/**
 * @brief World-map animated element: draw and tick two refcounts.
 * @note Best match ~92.21% (gcc280_g0); residual is a delay-slot scheduling
 *       tie (target hoists the %hi(D_801B2D1C) lui into the clamp branch slot).
 */
void func_8009DC9C(void)
{
    u16 *s2 = &D_801B24A8;
    s32 t;
    s32 c;

    func_8006CFA8(&D_80182DC0, s2);
    func_800675F0(D_8011CF28, D_8013923C & 3, 0x4, 0x35, 0x7800, 0x1001,
                  D_80182DEC, 0, 0xA, -1);
    t = D_80182DEC + 8;
    D_80182DEC = t;
    if (t >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    *(u16 *)((u8 *)s2 + 4) += 0x38;
    c = D_801B2D1C - 1;
    D_801B2D1C = c;
    D_8013923C += 1;
    if (c == 0)
    {
        D_801B2D18 += 1;
    }
}
