#include "common.h"

/** @brief Actor state record; the object index at 0x3A selects a D_80105AE0 slot. */
typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x21 - 8];
    u8 unk21;   /* 0x21 control state; bit 7 preserved */
    u8 pad22[0x2A - 0x22];
    s16 unk2A;  /* 0x2A */
    u8 pad2C[0x2E - 0x2C];
    u16 unk2E;  /* 0x2E gate flag */
    s16 unk30;  /* 0x30 */
    u8 pad32[0x3A - 0x32];
    u8 unk3A;   /* 0x3A object index */
} FieldRecord;

/** @brief Per-actor slot in D_80105AE0; stride 0x23C. */
typedef struct
{
    u8 pad0[0x174];
    s32 unk174;
    u8 pad178[0x18D - 0x178];
    u8 unk18D;
    u8 pad18E[0x23C - 0x18E];
} FieldState;

void func_800952DC(FieldRecord *record, s32 value);
void func_80096334(FieldRecord *record);
void func_800A2DD8(u8 index);

extern FieldState D_80105AE0[];
extern s32 D_8010AE58;
extern s32 D_8010AE60;
extern s32 D_8010AE68;
extern s32 D_8010AE6C;
extern s32 D_8010AE70;
extern s32 D_8010AE74;
extern s32 D_8010AE7C;
extern s32 D_8010AE80;
extern s32 D_8010CFD8;
extern s32 D_8010CFDC;
extern s16 D_801ED400;

/**
 * @brief Step the D_8010AE60 / D_8010AE68 pair toward D_8010AE6C / D_8010AE70 over the remaining D_8010AE58 frames.
 */
void func_80092200(void)
{
    s32 temp_a1;
    s32 temp_v1;

    if (D_8010AE58 != 0) {
        s32 *cur60 = &D_8010AE60;
        temp_a1 = (D_8010AE6C - *cur60) / D_8010AE58;
        temp_v1 = (D_8010AE70 - D_8010AE68) / D_8010AE58;
        D_8010AE58 -= 1;
        D_8010AE60 += temp_a1;
        D_8010AE68 += temp_v1;
    }
}

/**
 * @brief Step the D_8010AE7C / D_8010AE80 pair toward D_8010CFD8 / D_8010CFDC over the remaining D_8010AE74 frames, snapping when none remain.
 */
void func_800922B8(void)
{
    s32 temp_a1;
    s32 temp_v1;

    if (D_8010AE74 != 0) {
        s32 *cur = &D_8010AE7C;
        temp_a1 = (D_8010CFD8 - *cur) / D_8010AE74;
        temp_v1 = (D_8010CFDC - D_8010AE80) / D_8010AE74;
        D_8010AE74 -= 1;
        D_8010AE7C += temp_a1;
        D_8010AE80 += temp_v1;
        return;
    }
    D_8010AE7C = D_8010CFD8;
    D_8010AE80 = D_8010CFDC;
}

/**
 * @brief Clears the interpolation state above and seeds D_8010AE68 and D_8010AE70 from D_801ED400.
 *
 * @note 99.565% match. The only residue is the address register used to load
 *       `D_801ED400`; all 23 instructions and 92 bytes are otherwise exact.
 */
void func_80092394(void)
{
    s32 value;

    do
    {
        value = D_801ED400;
    } while (0);

    D_8010AE6C = 0;
    D_8010AE60 = 0;
    D_8010AE58 = 0;
    D_8010AE80 = 0;
    D_8010AE7C = 0;
    D_8010CFDC = 0;
    D_8010CFD8 = 0;
    D_8010AE74 = 0;
    D_8010AE68 = value;
    D_8010AE70 = value;
}

/**
 * @brief Decay a negative unk4 toward zero by 0x800, then idle the record when its gate flag is clear.
 *
 * Idling zeroes unk4 and unk2A, runs func_800952DC, clears the slot's 0x1800
 * flags, runs func_80096334, and for object indices below 2 also calls
 * func_800A2DD8 and clears unk30 and the slot byte at 0x18D.
 *
 * @param arg0 Actor state record.
 */
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

/**
 * @brief Idle an actor's animation when its gate flag is clear.
 *
 * When @c unk2E is 0, stops the actor's sound, zeroes @c unk2A, runs
 * func_800952DC, clears the 0x1800 bits of the slot's @c unk174, sets the
 * control state in @c unk21 to 2 (keeping bit 7), and notifies func_80096334.
 *
 * @param arg0 Actor state record.
 * @see decomp.me (100%) TODO
 */
void func_80092550(FieldRecord *arg0)
{
    if (arg0->unk2E == 0)
    {
        func_8006AA7C(arg0->unk3A);
        arg0->unk2A = 0;
        func_800952DC(arg0, 1);
        D_80105AE0[arg0->unk3A].unk174 = D_80105AE0[arg0->unk3A].unk174 & ~0x1800;
        arg0->unk21 = (u8)((arg0->unk21 & 0x80) + 2);
        func_80096334(arg0);
    }
}
