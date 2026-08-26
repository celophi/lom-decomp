#include "common.h"
extern u32 *D_80123FB0;
void func_800B65CC(s32 arg0)
{
    u32 *p;
    func_800BD520(0, 0x4288, arg0);
    p = D_80123FB0;
    *p |= 0x80000000;
    func_800B28E0(0x80, 0xD, 1, p);
}
