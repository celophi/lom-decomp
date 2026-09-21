#include "common.h"

extern void func_8006D0F0(s32, s32 *, s32 *);
extern void func_8009A668(void);
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern s32 D_80139950[];
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2C4C;

/** @brief Calculate the selected map position relative to the current projection origin. */
void func_8009A5BC(void)
{
    func_8006D0F0(2, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950[0];
    D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950[1];
    D_801B2C4C += 1;
    func_8009A668();
}
