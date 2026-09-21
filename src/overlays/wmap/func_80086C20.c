#include "common.h"

#include "sdk/libgte.h"

extern s32 D_80182DF0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B28F8;
extern s32 D_801B28FC;
extern void func_80085818(void);

/** @brief Clear two rotation vectors, set the flag, and start a 128-tick sequence step. */
void func_80086C20(void)
{
    D_80182DF0 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    D_801B28FC = 0x80;
    D_801B28F8 += 1;
    func_80085818();
}
