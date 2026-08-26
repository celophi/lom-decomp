#include "common.h"

extern unsigned char D_80117EF8;
extern s32 D_8011588C;
extern s32 D_8011F314;

void func_800A38D4(void)
{
    s32 temp_v0;

    temp_v0 = akao_cmd_19_c0((s32) &D_80117EF8, D_8011588C);
    D_8011F314 = temp_v0;
}
