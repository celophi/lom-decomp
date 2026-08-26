#include "common.h"

s32 func_800AE864(u8 *str)
{
    s32 count;
    s32 c;

    count = 0;
    c = str[0];
    if (c != 0)
    {
        do
        {
            if ((u32) (c - 0x19) < 7)
            {
                str += 2;
            }
            else
            {
                str += 1;
            }
            c = str[0];
            count++;
        } while (c != 0);
    }
    return count;
}
