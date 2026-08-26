#include "common.h"

extern u8 *D_80122B74;

void func_800C35AC(s32 arg0)
{
    u8 *temp_v1;

    if (arg0 < 0x40)
    {
        temp_v1 = D_80122B74 + arg0 * 0xC;
        temp_v1[0x2F0] |= 8;
    }
}
