#include "common.h"
extern u8 *D_80123FB8;
extern s32 func_800BD204(s32, void *, s32 *);

s32 func_800BD1B4(s32 arg0, void *arg1, s32 *arg2)
{
    s32 result;

    result = func_800BD204(arg0, arg1, arg2);
    if (*arg2 == 0xFF) {
        *arg2 = *D_80123FB8;
    }
    return result;
}
