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
