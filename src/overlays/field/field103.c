#include "common.h"

void func_800BC58C(s32 arg0, s32 arg1)
{
    if (arg1 == 0xFF)
    {
        arg1 = -1;
    }
    func_8005AF04(arg0, arg1, 1);
}

void func_800BC5B8(s32 obj_index, s32 part_index)
{
    if (part_index == 0xFF)
    {
        part_index = -1;
    }

    func_8005AF04(obj_index, part_index, 0);
}
