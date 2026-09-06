#include "common.h"

typedef struct
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[0x39 - 5];
    u8 unk39;
    u8 unk3A;
    u8 pad3B[0x44 - 0x3B];
    u8 unk44;
} SubStruct24;

typedef struct
{
    u8 pad0[0x20];
    u8 *unk20;
    SubStruct24 *unk24;
    u8 pad28[0x4A2 - 0x28];
    u8 unk4A2;
} FieldStateView729C;

extern FieldStateView729C *D_80123FB0;
extern u8 D_800F0BB8[];

u32 func_800B4CE4(SubStruct24 *arg0, s32 arg1);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Apply active field-state modifiers to the supplied value pair.
 * @param arg0 Unused operation selector.
 * @param arg1 Additional modifier mask.
 * @param arg2 Primary value adjusted by active modifiers.
 * @param arg3 Secondary value passed through unchanged.
 */
void func_800B729C(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3)
{
    s32 combined;
    s32 mask;
    s32 sum;
    s32 i;
    u8 *base24;

    combined = arg1 | D_80123FB0->unk4A2;
    mask = combined & D_80123FB0->unk24->unk39;

    if (mask != 0)
    {
        i = 0;
        sum = 0;
        do
        {
            if (mask & 1)
            {
                sum += *(D_80123FB0->unk20 + i + 0x3C);
            }
            i++;
            mask >>= 1;
        } while (i < 8);
        *arg2 = (u32)(*arg2 * (sum + 5)) >> 2;
    }

    if (func_800B4CE4(D_80123FB0->unk24, 8) == 0)
    {
        mask = combined & D_80123FB0->unk24->unk3A;
    }
    else
    {
        mask = combined;
    }

    i = 0;
    if (mask != 0)
    {
        sum = 0;
        do
        {
            if (mask & 1)
            {
                base24 = (u8 *)D_80123FB0->unk24;
                sum += *(base24 + D_800F0BB8[i] + 0x44);
            }
            i++;
            mask >>= 1;
        } while (i < 8);

        if (sum >= 9)
        {
            *arg2 = (u32)*arg2 >> 2;
        }
        else
        {
            *arg2 = (u32)*arg2 >> 1;
        }
    }

    if (D_80123FB0->unk24->unk4 != 0)
    {
        func_800BD520(D_80123FB0->unk24->unk4, 0xD008, combined);
    }
}
