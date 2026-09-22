#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 func_8006683C(s32);
extern void func_800AF9C4(void);
extern void func_800B0D24(void);
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern s32 D_801B2EF8;
extern s32 D_801B2EFC;

/** @brief Register callbacks, set effect flags and color, and begin an eight-tick delay. */
void func_800AEDEC(void)
{
    func_8006CAC0(func_800B0D24);
    func_8006CAC0(func_800AF9C4);
    D_80139244 = 1;
    D_801ADAF4 = 1;
    func_8006683C(0x701020);
    D_801ADAE0 = 1;
    D_801B2EFC = 8;
    D_801B2EF8 += 1;
}
