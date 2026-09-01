#include "common.h"

extern u8 *D_80122B78;
extern void func_800C1D14(s32 arg0, s32 arg1);

void func_800C1D68(void)
{
    s32 i;
    s32 off;
    u8 *rec;

    for (i = 0; i < *(u16 *)(D_80122B78 + 0x400); i++)
    {
        off = i * 0x94;
        rec = D_80122B78 + off;
        if (!((*(u32 *)(rec + 0x4C0) >> 30) & 1))
        {
            *(u16 *)(rec + 0x436) &= 0xFEFF;
            func_800C1D14(rec[0x430], 1);
        }
    }
}
