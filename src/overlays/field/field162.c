#include "common.h"

extern u8 *D_80122B74;
extern void func_800A8FB4(void);

s32 func_800C2AD0(void)
{
    s32 i;
    u8 *p;

    for (i = 0; i < 0x64; i++)
    {
        p = &D_80122B74[i * 0x40];
        p[0xCE0] = 0;
    }
    func_800A8FB4();
    return -1;
}
