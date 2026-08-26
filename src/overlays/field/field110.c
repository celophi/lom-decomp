#include "common.h"

typedef struct
{
    s8 pad[0x2C];
    s32 unk2C;
} UnkStruct80122B74;

extern UnkStruct80122B74* D_80122B74;

void func_800BCAA8(s32 arg0)
{
    func_800BD520(0, arg0, D_80122B74->unk2C);
}
