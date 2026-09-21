#include "common.h"

extern void func_8006D4B0(void);
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern s32 D_80139950[];
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B10A8;

/** @brief Compute the map-coordinate offset and advance the sequence. */
void func_8006D420(void)
{

    D_801398D0 = 2;
    D_80182D68 = D_80139950[0] - ((D_800DCEF8 - 1) * 0x30);
    D_80182D78 = D_80139950[1] - ((D_800DCF00 - 1) * 0x30);
    D_801B10A8 += 1;
    func_8006D4B0();
}
