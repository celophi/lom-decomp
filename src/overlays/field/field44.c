#include "common.h"

extern s32 D_8010D030;
extern s32 D_8010D040[];

s32 func_8009A3A0(s32 arg0)
{
    s32 count;

    count = D_8010D030;
    if (count < 0x10)
    {
        D_8010D040[count] = arg0;
        D_8010D030 = count + 1;
        return 0;
    }
    return 1;
}
