#include "common.h"

extern void func_80064094(void);
extern s32 D_8013B208;
extern s32 D_801B10A8;

/** @brief Clear the world-map flag, run setup, and advance the state. */
void func_8006D4F0(void)
{
    D_8013B208 = 0;
    func_80064094();
    D_801B10A8 += 1;
}
