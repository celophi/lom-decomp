#include "common.h"

typedef struct
{
    u8 pad[0x2C];
    u32 unk2C;
} UnkStruct2C;

extern UnkStruct2C *D_80122B74;

s32 func_800C3894(u32 arg0)
{
    u32 temp_v1;

    temp_v1 = D_80122B74->unk2C;
    if (arg0 < temp_v1)
    {
        D_80122B74->unk2C = temp_v1 - arg0;
        return 1;
    }
    return 0;
}
