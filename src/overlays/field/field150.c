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
