#include "common.h"

void field_control_animation(s32 list_kind, s32 index, s32 keyframe, s32 op);

void func_800BB9F4(s32 arg0, s32 arg1)
{
    field_control_animation(0, arg0, arg1, 1);
}
