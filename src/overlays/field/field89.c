#include "common.h"

typedef struct {
    u8 pad0[4];
    u8 unk4;
} UnkStruct800B313C;

void func_80089D44(u8 arg0);

void func_800B313C(UnkStruct800B313C *arg0)
{
    func_80089D44(arg0->unk4);
}
