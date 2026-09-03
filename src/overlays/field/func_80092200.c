#include "common.h"
extern s32 D_8010AE58;
extern s32 D_8010AE60;
extern s32 D_8010AE68;
extern s32 D_8010AE6C;
extern s32 D_8010AE70;

void func_80092200(void)
{
    s32 temp_a1;
    s32 temp_v1;

    if (D_8010AE58 != 0) {
        s32 *cur60 = &D_8010AE60;
        temp_a1 = (D_8010AE6C - *cur60) / D_8010AE58;
        temp_v1 = (D_8010AE70 - D_8010AE68) / D_8010AE58;
        D_8010AE58 -= 1;
        D_8010AE60 += temp_a1;
        D_8010AE68 += temp_v1;
    }
}
