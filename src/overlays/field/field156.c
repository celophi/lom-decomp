#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad[0x8F];
    s32 unk90;
} UnkStruct800C1B60Ret;

extern UnkStruct800C1B60Ret *func_800C1B60(void);

void func_800C28B8(void)
{
    UnkStruct800C1B60Ret *p;

    s32 arg0;

    p = func_800C1B60();
    arg0 = p->unk0;
    p->unk90 &= 0xBFFFFFFF;
    func_800C1D14(arg0, 0);
}
