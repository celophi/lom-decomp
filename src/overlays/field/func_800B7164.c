#include "common.h"

typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
} Obj2;

typedef struct
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[0x10 - 5];
    Obj2 *unk10;
    u8 pad14[0x1C - 0x14];
    u16 arr1C[4];
    u8 arr24[4];
} Obj;

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x1C - 0x18];
    u8 *unk1C;
    u8 pad20[0x24 - 0x20];
    Obj *unk24;
} FieldState;

extern FieldState *D_80123FB0;

s32 func_8008ADB4(s32 arg0);
s32 func_800B2D34(u8 *arg0, s32 arg1);

/**
 * @brief Resolve and scale an actor state value selected by the caller.
 * @param arg0 Value selector passed to the actor-state lookup.
 * @param arg1 Output location that receives the resolved value.
 */
void func_800B7164(s32 arg0, s32 *arg1)
{
    Obj *obj;
    Obj *obj3;
    s32 status;
    s32 mult_val;
    u8 idx;

    status = func_8008ADB4(D_80123FB0->unk24->unk4);
    mult_val = func_800B2D34((u8 *)D_80123FB0->unk24, arg0);
    obj = D_80123FB0->unk24;
    if ((obj->unk10->unkC & 2) || status == 0x31)
    {
        *arg1 = 0;
    }
    else if ((u32)(status - 0xA) < 2)
    {
        D_80123FB0->unk14 |= 1;
        idx = *D_80123FB0->unk1C >> 6;
        obj3 = D_80123FB0->unk24;
        *arg1 = obj3->arr1C[idx] + obj3->arr24[idx];
    }
    else
    {
        idx = *D_80123FB0->unk1C >> 6;
        *arg1 = obj->arr1C[idx];
    }
    *arg1 = (u32)(*arg1 * (mult_val + 0x32)) / 50;
}
