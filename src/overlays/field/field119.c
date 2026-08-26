#include "common.h"

void akao_cmd_a9(s32 arg0, s32 arg1);

void func_800BCFBC(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    if (arg2 == 0)
    {
        arg2 = 1;
    }
    akao_cmd_a9(arg2, arg3);
}
