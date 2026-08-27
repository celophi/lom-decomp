#include "common.h"

extern s32 func_8008B288(void *arg0);
extern s32 func_80087F44(void *arg0, s32 *out);

s32 func_800B302C(void *arg0, void *arg1)
{
    s32 kind;
    s32 a[4];
    s32 b[4];

    kind = func_8008B288(arg1);
    func_80087F44(arg0, a);
    func_80087F44(arg1, b);
    if (a[0] - b[0] < 0)
    {
        if ((u32)(kind - 0x40) >= 0x81)
        {
            return 0;
        }
        return -1;
    }
    else
    {
        if ((u32)(kind - 0x40) >= 0x81)
        {
            return -1;
        }
        return 0;
    }
}
