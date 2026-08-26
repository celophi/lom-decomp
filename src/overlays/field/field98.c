#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    s32 unk8;
} UnkStruct80123FB8;

extern UnkStruct80123FB8* D_80123FB8;

void func_800BB54C(void)
{
    UnkStruct80123FB8* p = D_80123FB8;
    s32 idx = p->unk4;

    p += idx;
    p->unk8 += 1;
}
