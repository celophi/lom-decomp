#include "common.h"

extern s32 D_8011F3AC[];
extern s32 D_8011F378;
extern u8 D_8011F388[];

s32 func_800A4744(void)
{
    s32 value;

    value = D_8011F3AC[0];
    if (value == 0)
    {
        s32 index = D_8011F378;
        value = D_8011F388[index];
    }
    else
    {
        value = -1;
    }
    return value;
}
