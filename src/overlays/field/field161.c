#include "common.h"

extern u8 *D_80122B74;
extern void func_800A8FB4(void);
extern void func_800C2AD0(void);

void func_800C2A88(s32 arg0)
{
    u8 *p;

    if (arg0 < 0x64)
    {
        p = &D_80122B74[arg0 * 0x40];
        p[0xCE0] = 0;
        func_800A8FB4();
    }
    else
    {
        func_800C2AD0();
    }
}
