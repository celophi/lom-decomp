/* Partial WMAP decompilation: 84.857140% (gcc280_g0). */
#include "common.h"

extern u8 D_80139988[];
extern void *D_8011F538;
extern s16 D_801AFBD0;
extern s32 D_801B0FD0;
extern s32 D_80139980;
extern s32 D_801B27E0;
extern s32 D_801B27E4;
extern void func_80080EE0(void);

/**
 * @brief World-map step handler: seed a 6-entry table and advance the step.
 * @note Best match ~84.86% (gcc280_g0); residual is loop induction-variable
 *       register allocation (permuter territory).
 */
void func_80080E54(void)
{
    s16 *p;
    s32 off;
    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x7F;
    p = &D_801AFBD0;
    i = 0;
    for (off = 0x660; i < 6; off += 8)
    {
        *p = 0;
        i += 1;
        *(void **)((u8 *)&D_80139988 + off + 4) = &D_8011F538;
        p += 0xA;
    }
    D_801B27E4 = 0x20;
    D_801B27E0 += 1;
    func_80080EE0();
}
