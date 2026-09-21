#include "common.h"

extern s16 D_800D937E;
extern s32 D_801B2990;
extern s32 D_801B2994;
extern void func_8008A3E4(void);

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void func_8008A3A0(void)
{
    D_800D937E = 4;
    D_801B2994 = 0x8C;
    D_801B2990 += 1;
    func_8008A3E4();
}
