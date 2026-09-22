#include "common.h"

extern s32 D_801B2468;
extern s32 D_801B24B4;
extern s32 D_80182DE4;
extern s16 D_801B2490[];
extern s16 D_801B2498[];
extern s32 D_801B2ABC;
extern s32 D_801B2AB8;
extern void func_8008F630(void);

/** @brief World-map step: reset the sprite counters and advance to the next handler. */
void func_800913C4(void)
{
    D_801B2468 = 1;
    D_801B24B4 = 0;
    D_80182DE4 = 0;
    D_801B2490[0] = 0;
    D_801B2490[1] = 0;
    D_801B2490[2] = 0;
    D_801B2498[0] = 0;
    D_801B2498[1] = 0;
    D_801B2498[2] = 0;
    D_801B2ABC = 0x80;
    D_801B2AB8 += 1;
    func_8008F630();
}
