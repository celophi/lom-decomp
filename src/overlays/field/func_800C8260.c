#include "common.h"

typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

void func_80087F44(s32 index, FieldPosition *position);
s32 func_8008B288(s32 arg0);
s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Offset the current field position according to its heading and submit the result.
 */
void func_800C8260(void)
{
    FieldPosition pos;
    s32 dir;
    s32 x;
    s32 y;
    s32 z;

    func_80087F44(0, &pos);

    x = pos.x;
    if (x < 0)
    {
        x += 0xFF;
    }
    pos.x = x >> 8;

    y = pos.y;
    if (y < 0)
    {
        y += 0xFF;
    }
    pos.y = y >> 8;

    z = pos.z;
    if (z < 0)
    {
        z += 0xFF;
    }
    pos.z = z >> 8;

    dir = func_8008B288(0);
    if (dir == 0)
    {
        pos.x += 0xA;
    }
    else
    {
        if ((u32)(dir - 1) < 0x3F)
        {
            pos.x += 8;
            pos.z -= 8;
        }
        else if (dir == 0x40)
        {
            pos.z -= 0xA;
        }
        else if ((u32)(dir - 0x41) < 0x3F)
        {
            pos.x -= 8;
            pos.z -= 8;
        }
        else if (dir == 0x80)
        {
            pos.x -= 0xA;
        }
        else if ((u32)(dir - 0x81) < 0x3F)
        {
            pos.x -= 8;
            pos.z += 8;
        }
        else if (dir == 0xC0)
        {
            pos.z += 0xA;
        }
        else if ((u32)(dir - 0xC1) < 0x3F)
        {
            pos.x += 8;
            pos.z += 8;
        }
    }

    func_80087D8C(0xC, pos.x, pos.y, pos.z);
}
