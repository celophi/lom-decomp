#include "common.h"

typedef struct
{
    u8 pad[0x400];
    u16 unk400;
} UnkStruct800C1E08;

extern UnkStruct800C1E08 *D_80122B78;

void func_800C1E08(void)
{
    s32 var_v1;
    u16 temp_v0;

    var_v1 = 0;
    temp_v0 = D_80122B78->unk400;
    if (temp_v0 != 0)
    {
        do
        {
            var_v1 += 1;
        } while (var_v1 < (s32) temp_v0);
    }
}
