#include "common.h"

extern u8 D_801148B0[];

void *func_800C2958(s32 arg0, u16 arg1)
{
    u8 *rec;
    u8 *tbl;
    u32 count;

    rec = &D_801148B0[arg0 << 12];
    tbl = rec + *(s32 *) (rec + 8);
    count = *(u32 *) tbl;
    if (arg1 < count)
    {
        return tbl + (arg1 * 8 + 4);
    }
    return (void *) 0;
}
