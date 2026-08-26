#include "common.h"

void func_800BDA48(s32 unused, u8 *params)
{
    field_set_fade_target(*(s32 *)(params + 0x4), *(s32 *)(params + 0x8), *(s32 *)(params + 0xC), *(u16 *)(params + 0x0));
}
