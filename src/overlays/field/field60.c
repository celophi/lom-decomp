#include "common.h"

void akao_play_sfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void func_800A3960(s32 arg0, s32 arg1)
{
    akao_play_sfx(arg0, 0, arg1 * 2, 0x7F);
}
