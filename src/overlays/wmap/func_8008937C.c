#include "wmap_resource_support.h"
#include "common.h"

extern s32 func_8006683C(s32);
extern s32 D_801ADAF4;
extern s32 D_801B2960;
extern s32 D_801B2964;

/** @brief Play sound 34, set the world-map color and value, and begin an eight-tick delay. */
void func_8008937C(void)
{
    func_800652A8(0x22, 0x80);
    D_801ADAF4 = 4;
    func_8006683C(0x304010);
    D_801B2964 = 8;
    D_801B2960 += 1;
}
