#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 92.213110% (gcc280_g0). */
#include "common.h"

extern u16 D_8013B238;
extern u8 D_80182DC0;
extern s32 D_8011CF2C;
extern s32 D_80139234;
extern s32 D_801B25D8;
extern s32 D_801B2D10;
extern s32 D_801B2D14;
extern void func_800675F0(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5,
                          s32 a6, s32 a7, s32 a8, s32 a9);

/**
 * @brief World-map animated element: draw and tick two refcounts.
 * @note Best match ~92.21% (gcc280_g0); residual is a delay-slot scheduling
 *       tie (target hoists the %hi(D_801B2D14) lui into the clamp branch slot).
 */
void func_8009DABC(void)
{
    u16 *s2 = &D_8013B238;
    s32 t;
    s32 c;

    func_8006CFA8(&D_80182DC0, s2);
    func_800675F0(D_8011CF2C, D_80139234 & 3, 0xA, 0x36, 0x7900, 0x1001,
                  D_801B25D8, 0, 0xF, -1);
    t = D_801B25D8 + 2;
    D_801B25D8 = t;
    if (t >= 0x82)
    {
        D_801B25D8 = 0x81;
    }
    *(u16 *)((u8 *)s2 + 4) += 0x14;
    c = D_801B2D14 - 1;
    D_801B2D14 = c;
    D_80139234 += 1;
    if (c == 0)
    {
        D_801B2D10 += 1;
    }
}
