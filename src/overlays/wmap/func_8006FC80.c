#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800652A8(s32, s32);
extern s32 D_801ADAE0;
extern s32 D_801B2400;
extern s32 D_801B2404;
extern void func_8006F7DC(void);
extern void func_80070530(void);

/** @brief Play sound 18, register two callbacks, and begin a seven-tick delay. */
void func_8006FC80(void)
{
    func_800652A8(0x12, 0x80);
    func_8006CAC0(&func_80070530);
    D_801ADAE0 = 1;
    func_8006CAC0(&func_8006F7DC);
    D_801B2404 = 7;
    D_801B2400 += 1;
}
