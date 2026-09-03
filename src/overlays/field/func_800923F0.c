#include "common.h"

typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x2A - 8];
    s16 unk2A;
    u8 pad2C[0x2E - 0x2C];
    u16 unk2E;
    s16 unk30;
    u8 pad32[0x3A - 0x32];
    u8 unk3A;
} FieldRecord;

typedef struct
{
    u8 pad0[0x174];
    s32 unk174;
    u8 pad178[0x18D - 0x178];
    u8 unk18D;
    u8 pad18E[0x23C - 0x18E];
} FieldState;

extern FieldState D_80105AE0[];
void func_800952DC(FieldRecord *record, s32 value);
void func_80096334(FieldRecord *record);
void func_800A2DD8(u8 index);

void func_800923F0(FieldRecord *arg0)
{
    s32 temp;
    FieldState *base;
    FieldState *slot;

    temp = arg0->unk4;
    if (temp < 0) {
        temp += 0x800;
        arg0->unk4 = temp;
        if (temp > 0) {
            arg0->unk4 = 0;
        }
    }

    if (arg0->unk2E == 0) {
        arg0->unk4 = 0;
        arg0->unk2A = 0;
        func_800952DC(arg0, 1);
        base = D_80105AE0;
        slot = &base[arg0->unk3A];
        slot->unk174 &= ~0x1800;
        func_80096334(arg0);
        if (arg0->unk3A < 2) {
            func_800A2DD8(arg0->unk3A);
            arg0->unk30 = 0;
            base[arg0->unk3A].unk18D = 0;
        }
    }
}

/**
 * @brief Initializes an inactive field record and clears two state flags.
 *
 * @param record Field record whose state entry is selected by byte 0x3A.
 */
void func_800924D8(FieldRecord *record)
{
    if (record->unk2E == 0)
    {
        record->unk2A = 0;
        func_800952DC(record, 1);
        D_80105AE0[record->unk3A].unk174 &= ~0x1800;
        func_80096334(record);
    }
}
