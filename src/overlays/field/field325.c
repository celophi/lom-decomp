#include "common.h"

extern u8 *D_80122B78;

/**
 * @see decomp.me (100%) TODO
 */
void func_800B4390(void)
{
    s32 i;
    s32 off;

    for (i = 3; i < *(u16 *)(D_80122B78 + 0x400); i++)
    {
        off = i * 0x94;
        func_80087F0C((D_80122B78 + off)[0x430]);
    }
}
