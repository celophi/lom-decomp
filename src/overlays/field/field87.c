#include "common.h"

extern s32 func_800B2D34(u8 *arg0, s32 arg1);

s32 func_800B2FF8(u8 *arg0)
{
    s32 chance;

    chance = func_800B2D34(arg0, 7);
    return (u32) (rand() & 0xFF) < (u32) chance;
}
