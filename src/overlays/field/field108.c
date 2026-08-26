#include "common.h"

extern u8 *D_80122B74;

void func_800BC9C4(s32 arg0, s32 arg1)
{
    u8 *temp_v1;

    temp_v1 = D_80122B74 + arg0 * 0xC;
    temp_v1[0x2F0] = (temp_v1[0x2F0] & 0xCF) | ((arg1 & 3) << 4);
}
