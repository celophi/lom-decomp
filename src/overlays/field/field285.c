#include "common.h"

extern u8 *D_80123FB8;

void func_80089D44(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void func_800BD0A4(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 v;

    if (arg0 == 0xFF)
    {
        v = D_80123FB8[0];
    }
    else
    {
        v = arg0;
    }
    func_80089D44(v,
                  (arg1 == 0xFF) ? -1 : arg1,
                  (arg2 == 0xFF) ? -1 : arg2,
                  (arg3 == 0xFF) ? -1 : arg3);
}
