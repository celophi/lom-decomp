#include "common.h"

extern u8 D_80122C0D;

void func_80087F44(s32 arg0, s32 *out);
s32 func_8008B288(s32 arg0);
s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Move the active field actor one step in its current heading.
 */
void func_800C7DB8(void)
{
    s32 pos[3];
    s32 x;
    s32 y;
    s32 z;
    s32 heading;
    s32 owner;

    owner = D_80122C0D;
    func_80087F44(0, pos);

    x = pos[0];
    if (x < 0)
    {
        x += 0xFF;
    }
    pos[0] = x >> 8;

    y = pos[1];
    if (y < 0)
    {
        y += 0xFF;
    }
    pos[1] = y >> 8;

    z = pos[2];
    if (z < 0)
    {
        z += 0xFF;
    }
    pos[2] = z >> 8;

    heading = func_8008B288(0);
    if (heading == 0)
    {
        pos[0] += 0x14;
    }
    else if ((u32)(heading - 1) < 0x3F)
    {
        pos[0] = pos[0] + 0xA;
        pos[2] = pos[2] - 0xA;
    }
    else if (heading == 0x40)
    {
        pos[2] -= 0xC;
    }
    else if ((u32)(heading - 0x41) < 0x3F)
    {
        pos[0] = pos[0] - 0xA;
        pos[2] = pos[2] - 0xA;
    }
    else if (heading == 0x80)
    {
        pos[0] -= 0x14;
    }
    else if ((u32)(heading - 0x81) < 0x3F)
    {
        pos[0] = pos[0] - 0xA;
        pos[2] = pos[2] + 0xA;
    }
    else if (heading == 0xC0)
    {
        pos[2] += 0xC;
    }
    else if ((u32)(heading - 0xC1) < 0x3F)
    {
        pos[0] = pos[0] + 0xA;
        pos[2] = pos[2] + 0xA;
    }

    func_80087D8C(owner, pos[0], pos[1] - 0xC, pos[2]);
}
