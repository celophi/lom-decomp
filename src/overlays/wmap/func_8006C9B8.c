#include "common.h"

extern s32 D_800DBE70;
extern s32 D_800DBE78;
extern s32 D_8013B208;
extern s32 D_801B1098;
extern s32 D_801B109C;

/** @brief Set world-map flags, start an eight-tick delay, and advance the state. */
void func_8006C9B8(void)
{
    D_800DBE70 = 0;
    D_8013B208 = 1;
    D_800DBE78 = 1;
    D_801B109C = 8;
    D_801B1098 += 1;
}
