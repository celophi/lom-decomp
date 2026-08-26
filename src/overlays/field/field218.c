#include "common.h"

s32 func_800CB064(s32 arg0)
{
    return func_800CA364(0x18, 0x17, arg0) > 0;
}

s32 func_800CB08C(s32 arg0)
{
    return (func_800CA364(0x17, 0xC, arg0) + func_800CA364(0x17, 0xD, arg0)) > 0;
}
