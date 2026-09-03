#include "common.h"

extern s32 D_8010AE58;
extern s32 D_8010AE60;
extern s32 D_8010AE68;
extern s32 D_8010AE6C;
extern s32 D_8010AE70;
extern s32 D_8010AE74;
extern s32 D_8010AE7C;
extern s32 D_8010AE80;
extern s32 D_8010CFD8;
extern s32 D_8010CFDC;

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

void func_800922B8(void)
{
    s32 temp_a1;
    s32 temp_v1;

    if (D_8010AE74 != 0) {
        s32 *cur = &D_8010AE7C;
        temp_a1 = (D_8010CFD8 - *cur) / D_8010AE74;
        temp_v1 = (D_8010CFDC - D_8010AE80) / D_8010AE74;
        D_8010AE74 -= 1;
        D_8010AE7C += temp_a1;
        D_8010AE80 += temp_v1;
        return;
    }
    D_8010AE7C = D_8010CFD8;
    D_8010AE80 = D_8010CFDC;
}
