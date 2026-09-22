#include "common.h"

#include "sdk/libgte.h"

extern void func_8006DB68(void);
extern s32 D_801B2418;
extern s32 D_801B241C;
extern s32 D_801B2468;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B24B0;

/** @brief Clear effect state and two vectors, then begin a 60-tick sequence step. */
void func_8006F0FC(void)
{
    D_801B2468 = 0;
    D_801B24B0 = 0;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    D_801B241C = 0x3C;
    D_801B2418 += 1;
    func_8006DB68();
}
