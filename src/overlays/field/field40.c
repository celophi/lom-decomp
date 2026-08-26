#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x14];
    u8 unk20;
} UnkStruct21;

void func_80094BC4(UnkStruct21 *arg0, s32 arg1, s32 arg2)
{
    u8 temp_v0;

    temp_v0 = arg0->unk20;
    arg0->unk0 = arg0->unk0 + (arg1 * temp_v0);
    arg0->unk8 = arg0->unk8 + (arg2 * temp_v0);
}
