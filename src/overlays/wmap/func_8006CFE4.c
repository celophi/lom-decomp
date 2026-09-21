#include "common.h"

extern void func_8006D014(s32, s32, s32, s32, s32, s32, s32);

/**
 * @brief Forward six arguments with the final option cleared.
 * @param arg0 First forwarded argument.
 * @param arg1 Second forwarded argument.
 * @param arg2 Third forwarded argument.
 * @param arg3 Fourth forwarded argument.
 * @param arg4 Fifth forwarded argument.
 * @param arg5 Sixth forwarded argument.
 */
void func_8006CFE4(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    func_8006D014(arg0, arg1, arg2, arg3, arg4, arg5, 0);
}
