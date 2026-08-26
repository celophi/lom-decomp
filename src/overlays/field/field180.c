#include "common.h"

extern s16 D_80122C10;
extern u8 D_80043CB8[];

void func_800B2844(s32 arg0, u8* arg1, u8 arg2);

void func_800C69F4(void)
{
    func_800B2844(0, D_80043CB8 + (D_80122C10 << 6), 0xFF);
}
