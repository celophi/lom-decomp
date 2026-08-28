#include "common.h"

u8 *func_80141DA4(void *arg0)
{
    u8 *p = arg0;

    while ((u32)(*p - 0x30) < 10 || (u32)(*p - 0x61) < 6 || (u32)(*p - 0x41) < 6)
    {
        p++;
    }
    return p;
}
