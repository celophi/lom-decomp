#include "common.h"

extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2910;
extern s32 D_801B2914;
extern void func_8006683C(s32 arg);
extern void func_800652A8(s32 a, s32 b);

/** @brief World-map step handler: kick two jobs and advance the step. */
void func_800877D4(void)
{
    D_8013B208 = 1;
    func_8006683C(0x802028);
    D_801ADAF4 = 4;
    func_800652A8(0x21, 0x80);
    D_801B2914 = 0x1E;
    D_801B2910 += 1;
}
