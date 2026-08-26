#include "common.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
} UnkStruct_80122C0C;

extern UnkStruct_80122C0C D_80122C0C;

void func_800C6364(void)
{
    func_80087680(D_80122C0C.unk0, D_80122C0C.unk2, D_80122C0C.unk2, 0, 0, 0);
}
