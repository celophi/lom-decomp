#include "common.h"

extern s32 D_8011F310;

void func_800A3904(s32 arg0)
{
    akao_cmd_c1((&D_8011F310)[arg0]);
}
