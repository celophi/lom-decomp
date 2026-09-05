#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    s32 unk8;
} UnkStruct80123FB8;

extern UnkStruct80123FB8* g_field_script;

void func_800BB54C(void)
{
    UnkStruct80123FB8* p = g_field_script;
    s32 idx = p->unk4;

    p += idx;
    p->unk8 += 1;
}
