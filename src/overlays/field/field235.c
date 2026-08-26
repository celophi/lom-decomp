#include "common.h"

s32 func_800A8DDC(u8 *arg0)
{
    s32 count;
    u8 c;

    count = 0;
    c = *arg0;
    if (c != 0)
    {
        do
        {
            if ((u32)(c - 0x19) < 7)
            {
                arg0 += 2;
                count += 2;
            }
            else
            {
                arg0 += 1;
                count += 1;
            }
            c = *arg0;
        } while (c != 0);
    }
    return count;
}
