#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80051EB4;

extern s16 D_80122C1C;

void func_800C9ED4();

void func_800C6DA0(void)
{
    func_800C9ED4(D_80122C1C);
}

extern UnkStruct80051EB4 D_80051EB4;
extern void func_800AAFEC(UnkStruct80051EB4 *arg0);

void func_800C6DC8(void)
{
    UnkStruct80051EB4 local;

    local = D_80051EB4;
    func_800AAFEC(&local);
}

void func_800AD030(s32 arg0);

void func_800C6E08(void)
{
    func_800AD030(0);
}
