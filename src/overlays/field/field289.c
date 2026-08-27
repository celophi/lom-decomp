#include "common.h"

extern void func_800A8B90(void *out, s32 arg1, s32 arg2);
extern s32 func_800A88A0(void *arg0, void *arg1, void *arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);

void func_800A8A78(void *arg0, void *arg1, s32 arg2, s32 arg3, s16 *arg4, s32 arg5)
{
    u8 local[0x40];

    func_800A8B90(local, arg2, 0);
    func_800A88A0(arg1, arg0, local, arg3, arg4[0], arg4[1], arg5);
}
