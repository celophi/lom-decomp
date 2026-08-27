#include "common.h"

extern u8* D_80123FB8;

void func_800BB9C0(s32 arg0, s32 arg1)
{
    func_800BD520(*D_80123FB8, arg0, arg1);
}

void field_control_animation(s32 list_kind, s32 index, s32 keyframe, s32 op);

void func_800BB9F4(s32 arg0, s32 arg1)
{
    field_control_animation(0, arg0, arg1, 1);
}

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
