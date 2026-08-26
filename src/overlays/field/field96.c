#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    s32 unk8;
} UnkStruct800B892C;

extern UnkStruct800B892C *D_80123FB8;

void func_800B892C(void)
{
    UnkStruct800B892C *temp_v1;

    temp_v1 = D_80123FB8 + D_80123FB8->unk4;
    temp_v1->unk8 = temp_v1->unk8 + 1;
}
