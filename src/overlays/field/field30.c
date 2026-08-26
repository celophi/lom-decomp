#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
} UnkStruct14;

void func_800832F0(UnkStruct14 *arg0, UnkStruct14 *arg1)
{
    arg0->unk0 = arg1->unk0;
    arg0->unk4 = arg1->unk4;
    arg0->unk8 = arg1->unk8;
    arg0->unkC = arg1->unkC;
    arg0->unk10 = arg1->unk10;
}
