#include "common.h"

extern s32 D_801B24B0;
extern s32 D_801B2468;
extern s16 D_801B2490[];
extern s16 D_801B2498[];
extern s32 D_801B2888;
extern s32 D_801B288C;
extern void func_80083D64(void);

/** @brief World-map step: reset counters and advance to the next handler. */
void func_80084C6C(void)
{
    D_801B24B0 = 0;
    D_801B2468 = 1;
    D_801B2490[0] = 0;
    D_801B2490[1] = 0;
    D_801B2490[2] = 0;
    D_801B2498[0] = 0;
    D_801B2498[1] = 0;
    D_801B2498[2] = 0;
    D_801B288C = 0x40;
    D_801B2888 += 1;
    func_80083D64();
}
