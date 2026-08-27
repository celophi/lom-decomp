#include "common.h"

extern u8 *D_80123FB8;

extern s32 func_80087F44(s32 arg0, s32 *out);
extern void func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void func_800BC5E4(s32 arg0, s32 arg1)
{
    s32 v;
    s32 buf[4];

    if (arg0 == 0xFF)
    {
        v = D_80123FB8[0];
    }
    else
    {
        v = arg0;
    }
    func_80087F44(v, buf);
    buf[1] = -arg1;
    func_80087D8C(v, buf[0] >> 8, buf[1], buf[2] >> 8);
}
