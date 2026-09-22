/* Partial WMAP decompilation: 84.857140% (gcc280_g0). */
#include "common.h"

extern u8 D_80139988[];
extern void *D_8011F538;
extern s16 D_801AFBD0;
extern s32 D_801B0FD0;
extern s32 D_80139980;
extern s32 D_801B2430;
extern s32 D_801B2434;
extern void func_8006F678(void);

/**
 * @brief World-map step handler: seed a 30-entry table and advance the step.
 * @note Best match ~84.86% (gcc280_g0); residual is loop induction-variable
 *       register allocation (permuter territory).
 */
void func_8006F5EC(void)
{
    s16 *p;
    s32 off;
    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x80;
    p = &D_801AFBD0;
    i = 0;
    for (off = 0x30; i < 30; off += 8)
    {
        *p = 0;
        i += 1;
        *(void **)((u8 *)&D_80139988 + off + 4) = &D_8011F538;
        p += 0xA;
    }
    D_801B2434 = 0xC;
    D_801B2430 += 1;
    func_8006F678();
}
