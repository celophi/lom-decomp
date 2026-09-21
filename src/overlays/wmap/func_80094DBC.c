#include "common.h"

extern s32 D_801B24B0;
extern s32 D_801B2468;
extern s16 D_801B2490[];
extern s16 D_801B2498[];
extern s32 D_801B2B58;
extern s32 D_801B2B5C;
extern void func_80093648(void);

/** @brief World-map step: reset counters and advance to the next handler. */
void func_80094DBC(void)
{
    D_801B24B0 = 0;
    D_801B2468 = 1;
    D_801B2490[0] = 0;
    D_801B2490[1] = 0;
    D_801B2490[2] = 0;
    D_801B2498[0] = 0;
    D_801B2498[1] = 0;
    D_801B2498[2] = 0;
    D_801B2B5C = 0x40;
    D_801B2B58 += 1;
    func_80093648();
}
