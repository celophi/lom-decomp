#include "common.h"

extern void func_8006F92C(void);
extern s16 D_800D9268[];
extern s32 D_801B2438;
extern s32 D_801B243C;

void func_8006F8E0(void)
{
    D_800D9268[0xD2 / 2] = 0;
    D_800D9268[0xD6 / 2] = 2;
    D_801B243C = 0x5D;
    D_801B2438 += 1;
    func_8006F92C();
}
