#include "common.h"

extern s32 D_8010AE58;
extern s32 D_8010AE6C;
extern s32 D_8010AE70;
extern s16 D_801ED400;

void func_800920FC(void)
{
    D_8010AE6C = 0;
    D_8010AE58 = 0x20;
    D_8010AE70 = (s32) D_801ED400;
}
