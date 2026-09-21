#include "common.h"

#include "sdk/libgte.h"

extern s32 D_80182DF0;
extern SVECTOR D_801B2490;
extern s32 D_801B2928;
extern s32 D_801B292C;
extern void func_80086F48(void);

/** @brief Clear the rotation vector, set the flag, and start a 128-tick sequence step. */
void func_80087F60(void)
{
    D_80182DF0 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B292C = 0x80;
    D_801B2928 += 1;
    func_80086F48();
}
