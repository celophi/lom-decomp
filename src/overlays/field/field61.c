#include "common.h"

extern void func_800A39A8(s32 sfx_index, s32 pan, s32 unused, s32 channel_group);

void func_800A3988(s32 sfx_index, s32 pan, s32 unused)
{
    func_800A39A8(sfx_index, pan, unused, 0);
}
