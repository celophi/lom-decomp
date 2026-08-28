#include "common.h"

s32 func_800A9060(void);
void func_800A8F8C(s32 arg0, void *arg1);
u8 *func_800C1E40(s32 arg0);
void func_800B2844(s32 arg0, void *arg1, s32 arg2);

extern u8 D_80122C00;

/**
 * @see decomp.me (100%)
 */
void func_800C9BC4(void)
{
    s32 temp_s1;
    s32 temp_s0;
    s32 off;

    temp_s1 = D_80122C00;
    if (func_800A9060() != 0)
    {
        temp_s0 = func_800A9060();
        func_800A8F8C(temp_s0, func_800C1E40(5) + (off = (temp_s1 << 6) + 4));
        func_800B2844(0, func_800C1E40(5) + off, 0xFF);
    }
}
