#include "common.h"

typedef struct
{
    u8 pad[0x2C];
    u32 unk2C;
} UnkStruct2C;

extern UnkStruct2C *D_80122B74;

s32 func_800C3860(s32 arg0)
{
    u32 temp_v0;

    temp_v0 = D_80122B74->unk2C + arg0;
    D_80122B74->unk2C = temp_v0;
    if (temp_v0 > 0x989680U)
    {
        D_80122B74->unk2C = 0x989680U;
    }
    return 1;
}
