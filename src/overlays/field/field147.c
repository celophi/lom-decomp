#include "common.h"

typedef struct
{
    s8 pad[0x2C];
    s32 unk2C;
} UnkStruct80122B74;

extern UnkStruct80122B74 *D_80122B74;

void func_800C0E18(s32 type, s32 amount)
{
    if (type < 3)
    {
        D_80122B74->unk2C += amount;
        if ((u32)D_80122B74->unk2C > 0x989680)
        {
            D_80122B74->unk2C = 0x989680;
        }
    }
}
