#include "common.h"

typedef struct
{
    s32 unk0;
} SomeRec;

extern SomeRec *func_80087F0C(s32 arg0);
extern void func_800B3114(SomeRec *counter, s32 delta);

void func_800C1B20(s32 arg0, s32 arg1)
{
    SomeRec *rec;
    s32 product;

    rec = func_80087F0C(arg0);
    product = rec->unk0 * arg1;
    func_800B3114(rec, (u32) product >> 8);
}

extern s32 D_80122B78;

s32 func_800C1B60(void)
{
    s32 var_v0;

    var_v0 = func_800C1B98();
    if (var_v0 == 0)
    {
        var_v0 = D_80122B78 + 0xE04;
    }
    return var_v0;
}
