#include "common.h"

extern s32 D_801178D4;

void func_8009C77C(s32 slot, s32 idx, s32 options)
{
    field_text_set_string(slot, (u8 *)(D_801178D4 + *(u16 *)((idx * 2) + D_801178D4)), options);
}
