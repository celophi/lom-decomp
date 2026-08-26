#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80051EB4;

extern UnkStruct80051EB4 D_80051EB4;
extern void func_800AAFEC(UnkStruct80051EB4 *arg0);

void func_800C8220(void)
{
    UnkStruct80051EB4 local;

    local = D_80051EB4;
    func_800AAFEC(&local);
}
