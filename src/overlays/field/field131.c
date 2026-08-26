#include "common.h"

typedef void (*UnkFunc800F0E58)(s32, s32);

extern UnkFunc800F0E58 D_800F0E58[];

void func_800BE5C8(s32 idx, s32 arg1, s32 arg2)
{
    D_800F0E58[idx](arg1, arg2);
}
