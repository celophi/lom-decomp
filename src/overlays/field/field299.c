#include "common.h"

s32 func_800B7B08(u8 *arg0, s32 arg1, u8 *arg2)
{
    u8 temp[0x40];
    u8 *slot;

    if (func_800B7980(arg0, arg1, arg2) != 0)
    {
        slot = arg0 + (arg1 * 0x40 + 0x50);
        func_800C1EC8(slot, temp, 0x40);
        func_800C1EC8(arg2, slot, 0x40);
        func_800C1EC8(temp, arg2, 0x40);
        func_800B7B98(arg0);
        return -1;
    }
    return 0;
}
