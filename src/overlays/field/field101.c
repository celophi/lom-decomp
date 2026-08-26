#include "common.h"

extern void func_8006B1A0(s32 arg0, s32 arg1);

/**
 * @param arg0 Passed through to func_8006B1A0.
 * @param arg1 Passed through to func_8006B1A0.
 * @see decomp.me (100%) N/A -- trivial 8-instruction wrapper function, no scratch needed.
 */
void func_800BBA24(s32 arg0, s32 arg1)
{
    func_8006B1A0(arg0, arg1);
}
