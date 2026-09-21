#include "common.h"

extern s32 D_801B24B0;
extern s32 D_801B2468;
extern s16 D_801B2490[];
extern s16 D_801B2498[];
extern s32 D_801B24F0;
extern s32 D_801B24F4;
extern void func_800709E8(void);

/** @brief World-map step: reset counters and advance to the next handler. */
void func_800721C8(void)
{
    D_801B24B0 = 0;
    D_801B2468 = 1;
    D_801B2490[0] = 0;
    D_801B2490[1] = 0;
    D_801B2490[2] = 0;
    D_801B2498[0] = 0;
    D_801B2498[1] = 0;
    D_801B2498[2] = 0;
    D_801B24F4 = 0x54;
    D_801B24F0 += 1;
    func_800709E8();
}
