#include "common.h"

typedef struct
{
    u8 unk0[4];
    u8 unk4;
} UnkStruct800C0D90_Arg2;

typedef struct
{
    u8 unk0[2];
    s16 unk2;
} UnkStruct800C0D90_Ret;

extern UnkStruct800C0D90_Ret *func_800C1B60(u8 arg0);

s32 func_800C0D90(s32 arg0, s32 arg1, UnkStruct800C0D90_Arg2 *arg2)
{
    func_800C1B60(arg2->unk4)->unk2 = (s16) (arg1 | 0x8000);
    return 0x20;
}
