#include "common.h"

extern u8 D_800F0BE0[];
extern u8 D_800F0BEC[];

void func_800B7A74(void *arg0, s32 arg1, u8 *arg2)
{
    s32 i;
    u8 *p;
    u32 v;
    s32 mode;
    u8 *tbl;

    p = (u8 *)arg0;
    i = 0;
    *arg2 = 0;
    do
    {
        if (i != arg1)
        {
            v = *(u32 *)(p + 0x64);
            mode = (v >> 8) & 3;
            switch (mode)
            {
            case 0:
                tbl = &D_800F0BE0[(v >> 10) & 0x3F];
                break;
            case 1:
                tbl = &D_800F0BEC[(v >> 10) & 0x3F];
                break;
            default:
                p += 0x40;
                i += 1;
                continue;
            }
            v = *arg2;
            v |= *tbl;
            *arg2 = v;
        }
        p += 0x40;
        i += 1;
    } while (i < 4);
}
