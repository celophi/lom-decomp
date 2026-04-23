#include "decomp8.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/mLcZm
 */
void func_8001615C(u8* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    u8* p;
    u8* end;
    s32 len;
    s32 a3_val;

    D_80047404 = arg1;
    D_80047408 = arg2;
    a3_val = arg3;
    len = strlen((const char*)arg0);
    if (len > 0)
    {
        p = arg0;
        end = (u8*)(len + (s32)p);
        do
        {
            u8 ch = *p++;
            func_800165CC(ch, a3_val, arg4);
        } while ((s32)p < (s32)end);
    }
}

/**
 * decomp.me link (99.50%) https://decomp.me/scratch/ENN60
 */
void func_800161DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 q;
    int new_var;
    D_80047404 = arg1;
    D_80047408 = arg2;
    q = arg0 / 10;
    new_var = q * 10;
    func_800165CC(q + 0x30, arg3, arg4);
    func_800165CC((arg0 - new_var) + 0x30, arg3, arg4);
}