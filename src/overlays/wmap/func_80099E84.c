#include "common.h"

extern void func_80099754(s32);
extern s16 D_800D6508[];
extern s32 D_800D9164;
extern s32 D_800DCED8;
extern s32 D_800DCEE4;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398AC;
extern s32 D_801398D0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_8019D240;
extern s16 D_801AFBD2;

/**
 * @brief Start map movement after the actor faces its route, then wait for completion.
 * @return One while movement is pending, zero after arrival.
 */
s32 func_80099E84(void)
{
    if (D_800D9164 == 0)
    {
        func_80099754(1);
        if (D_801AFBD2 == D_800D6508[D_8019D240])
        {
            D_801398D0 = 2;
            D_80182D68 = (D_800DCED8 - D_800DCEF8) * 0x30;
            D_80182D78 = (D_800DCEE4 - D_800DCF00) * 0x30;
            D_800D9164 += 1;
        }
        return 1;
    }
    if (D_801398D0 == 2)
    {
        return 1;
    }
    D_801398AC = 0;
    D_800DCEF8 = D_800DCED8;
    D_800DCF00 = D_800DCEE4;
    return 0;
}
