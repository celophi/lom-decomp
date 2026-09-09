#include "common.h"

typedef struct
{
    char pad[0x1C];
    s32 *unk1C;
    s32 unk20;
    s32 unk24;
} FieldB78C0State;

extern FieldB78C0State *D_80123FB0;
extern s32 func_800B4CE4(s32 a, s32 b);

/**
 * @brief Apply field-state modifiers to an input value.
 * @param arg0 Base value to modify.
 * @return Value after applying the active field-state modifiers.
 */
s32 func_800B76F8(s32 arg0)
{
    s32 s0;
    s32 count;
    s32 value;

    s0 = arg0;

    count = func_800B4CE4(D_80123FB0->unk20, (*(u8 *)D_80123FB0->unk1C >> 6) | 0x40);
    if (count != 0)
    {
        do
        {
            s0 = (s0 * 3) / 2;
        } while (--count != 0);
    }

    value = *D_80123FB0->unk1C & 0xF;
    count = func_800B4CE4(D_80123FB0->unk20, value + 0x38);
    if (count != 0)
    {
        do
        {
            s0 = (s0 * 3) / 2;
        } while (--count != 0);
    }

    value = *D_80123FB0->unk1C & 0xF;
    count = func_800B4CE4(D_80123FB0->unk24, value + 0x30);
    if (count != 0)
    {
        do
        {
            s0 = s0 / 2;
        } while (--count != 0);
    }

    count = func_800B4CE4(D_80123FB0->unk20, *(u8 *)(D_80123FB0->unk24 + 3) + 0x10);
    if (count != 0)
    {
        do
        {
            s0 = (s0 * 3) / 2;
        } while (--count != 0);
    }

    count = func_800B4CE4(D_80123FB0->unk24, *(u8 *)(D_80123FB0->unk20 + 3) + 0x20);
    if (count != 0)
    {
        do
        {
            s0 = s0 / 2;
        } while (--count != 0);
    }

    return s0;
}

typedef struct
{
    s8 pad[0xC];
    s32 flags;
} InnerStruct80123FB0;

typedef struct
{
    s8 pad[0x20];
    InnerStruct80123FB0* inner;
} OuterStruct80123FB0;



s32 func_800B788C(s32 arg0)
{
    s32 result;

    result = arg0;
    if (((OuterStruct80123FB0 *)D_80123FB0)->inner->flags & 1)
    {
        result *= 2;
    }
    return result;
}

typedef struct
{
    u8 unk0;
    u8 unk1;
} FieldB78C0Rec;



extern void func_800B2B54(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern s32 func_800B4CE4(s32 a, s32 b);
extern FieldB78C0Rec D_800F0BC0;
extern FieldB78C0State *D_80123FB0;

void func_800B78C0(void)
{
    s32 var_s0;
    FieldB78C0Rec *new_var;
    FieldB78C0Rec *temp_v1;

    var_s0 = 0x50;
    if ((*D_80123FB0->unk1C & 0xF) != 2)
    {
        do
        {
            if (func_800B4CE4(D_80123FB0->unk20, var_s0) != 0)
            {
                new_var = &D_800F0BC0;
                temp_v1 = &new_var[var_s0 - 0x50];
                func_800B2B54(D_80123FB0->unk20, D_80123FB0->unk24, 0, var_s0 - 0x50,
                              (s32)temp_v1->unk0, temp_v1->unk1 * 0x10);
            }
            var_s0 += 1;
        } while (var_s0 < 0x60);
    }
}
