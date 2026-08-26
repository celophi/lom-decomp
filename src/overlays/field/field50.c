#include "common.h"

extern void func_dead6(s32, s32, s32, s32, s32, s32);

void func_8009CA08(u32 *ptr, u32 size)
{
    u8 *end;

    if (0)
    {
        func_dead6(0, 0, 0, 0, 0, 0);
    }
    size &= 0xFFFFC;
    ptr[0] = (size - 8) & 0xFFFFF;
    end = (u8 *) ptr + size;
    *(u32 *) (end - 4) |= 0x80000000;
    *(u32 *) (end - 4) |= 0x7FF00000;
}
