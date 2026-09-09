#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 *unk4;
    u8 *unk8;
    u8 unkC[8];
    s32 unk14;
    s32 unk18;
} StructB3580;

extern StructB3580 *D_80123FB0;

s32 func_800B3670(s32 arg0);

extern s32 func_800B42B4(u8 *);
extern u8 *func_800B4844(u32 *, s32);
extern u32 func_800BD414(s32, s32);
extern s32 func_800C19D0(s32, u32, s32);


/** @brief Partial HeaderB800B3DF4 layout used by func_800B3DF4. */
typedef struct
{
    u8 pad0[0x400];
    u16 unk400;
} HeaderB800B3DF4;

/** @brief Partial SlotB800B3DF4 layout used by func_800B3DF4. */
typedef struct
{
    u8 pad0[0x430];
    u8 unk430;
    u8 pad431[0x4C0 - 0x431];
    u32 unk4C0;
} SlotB800B3DF4;

extern u8 *D_80122B78;


void akao_set_song_params(s32, s32, s32, s32);

s32 func_80087F0C(s32 arg0);
/* Preserve word-width arguments at this caller. */
void func_800B3F1C(s32, u8 *, u8 *);

/**
 * @brief Build field records for actors whose status nibble matches the requested key.
 * @param arg0 Status key to match.
 * @return Number of matching actor records processed.
 * @note WIP: frame, allocation and control-flow differences remain.
 */
s32 func_800B3DF4(s32 arg0)
{
    s32 i;
    s32 count;
    s32 slot_off;
    s32 rec_off;
    s32 result;

    count = 0;
    slot_off = 0x1BC;
    rec_off = 0x160;
    for (i = 3; i < (s32) ((HeaderB800B3DF4 *) D_80122B78)->unk400; i++, slot_off += 0x94)
    {
        SlotB800B3DF4 *slot = (SlotB800B3DF4 *) (D_80122B78 + slot_off);
        if ((slot->unk4C0 & 0xF) == arg0)
        {
            result = func_80087F0C(slot->unk430);
            if (result == 0)
            {
                goto do_call;
            }
            if (result != -1)
            {
                goto skip_call;
            }
        do_call:
            akao_set_song_params(0x8001, 0x64, arg0, ((SlotB800B3DF4 *) (D_80122B78 + slot_off))->unk430);
        skip_call:
            count += 1;
            func_800B3F1C(((SlotB800B3DF4 *) (D_80122B78 + slot_off))->unk430, ((u8 *)D_80123FB0 + rec_off), (u8 *)result);
            rec_off += 0x68;
        }
    }
    return count;
}

/** @brief Initialize an actor and its counters from a resource template. */
void func_800B3F1C(s32 arg0, u8 *arg1, u8 *arg2)
{
    s32 temp_lo;
    s32 temp_lo_2;
    s32 temp_v1_4;
    s32 var_v0;
    u16 temp_s0;
    u16 temp_v1_3;
    u32 temp_v0_2;
    u32 temp_v0_3;
    u32 temp_v0_5;
    u32 temp_v0_6;
    u32 var_s0;
    u32 var_s0_2;
    u32 var_s0_3;
    u32 var_s2;
    u32 var_v1;
    u8 temp_v0_4;
    u8 *temp_v0;
    u8 *temp_v1;
    u8 *temp_v1_2;
    u8 *var_a0;
    u8 *var_a0_2;
    u8 *var_a1;

    arg1[4] = arg0;
    *(u8 *)(arg1 + 0x0) = 0x40;
    *(u8 * *)(arg1 + 0x10) = arg2;
    *(s32 *)(arg1 + 0x4) = (s32) ((((s32) *(s32 *)(arg1 + 0x4) | 0x100) & ~0x200 & 0xFFFF03FF) | 0x1400);
    *(u16 *)(arg1 + 0x6) = 0;
    temp_v0 = func_800B4844((u32 *)D_80123FB0->unk4, *(u8 *)(arg2 + 0x11));
    *(u8 * *)(arg1 + 0x14) = temp_v0;
    *(u8 *)(arg1 + 0x3) = (u8) *(u8 *)(temp_v0 + 0x1A);
    *(u8 *)(arg1 + 0x8) = (u8) *(u8 *)(temp_v0 + 0x1B);
    *(u16 *)(arg1 + 0xA) = 0;
    *(s32 *)(arg1 + 0xC) = 0;
    *(u8 *)(arg1 + 0x9) = (u8) *(u8 *)(temp_v0 + 0x1B);
    temp_v0_2 = func_800B42B4(temp_v0);
    var_v1 = 0x63;
    if (temp_v0_2 < 0x64U)
    {
        var_v1 = temp_v0_2;
    }
    var_s2 = var_v1;
    temp_v0_3 = func_800BD414(0, 0x2F78);
    if (temp_v0_3 != 0)
    {
        var_s2 = temp_v0_3;
    }
    var_s0 = 0;
    var_a0 = temp_v0;
    *(u16 *)(arg1 + 0x18) = (s16) ((u32) ((*(u16 *)(temp_v0 + 0x20) * 0x10) + (*(u16 *)(temp_v0 + 0x22) * var_s2)) >> 4);
    var_a1 = arg1;
    *(u8 *)(arg1 + 0x1A) = (u8) *(u8 *)(temp_v0 + 0x19);
    do
    {
        temp_lo = *(u8 *)(var_a0 + 0x25) * var_s2;
        temp_v1 = arg1 + var_s0;
        var_s0 += 1;
        temp_v0_4 = *(u8 *)(var_a0 + 0x24);
        var_a0 += 2;
        *(u16 *)(var_a1 + 0x1C) = (s16) ((u32) ((temp_v0_4 * 0x10) + temp_lo) >> 4);
        var_a1 += 2;
        *(u8 *)(temp_v1 + 0x24) = 0;
    } while (var_s0 < 4U);
    var_s0_2 = 0;
    var_a0_2 = temp_v0;
    do
    {
        temp_v1_2 = arg1 + var_s0_2;
        var_s0_2 += 1;
        *(u8 *)(temp_v1_2 + 0x3C) = 5;
        *(u8 *)(temp_v1_2 + 0x44) = 5;
        temp_v0_5 = (u32) ((*(u8 *)(var_a0_2 + 0x2C) * 4) + (*(u8 *)(var_a0_2 + 0x2D) * var_s2)) >> 2;
        *(u8 *)(temp_v1_2 + 0x30) = (s8) temp_v0_5;
        *(u8 *)(temp_v1_2 + 0x28) = (s8) temp_v0_5;
        var_a0_2 += 2;
    } while (var_s0_2 < 8U);
    *(u8 *)(arg1 + 0x38) = (u8) *(u8 *)(temp_v0 + 0x3C);
    *(u8 *)(arg1 + 0x39) = (u8) *(u8 *)(temp_v0 + 0x3D);
    *(u8 *)(arg1 + 0x3A) = (u8) *(u8 *)(temp_v0 + 0x3E);
    *(s32 *)(arg2 + 0x4C) = (s32) ((*(s32 *)(arg2 + 0x4C) & ~0xFE) | ((var_s2 & 0x7F) * 2));
    if (*(u8 *)(temp_v0 + 0x3F) & 2)
    {
        *(u8 * *)(arg2 + 0x64) = NULL;
    }
    else
    {
        *(u8 * *)(arg2 + 0x64) = temp_v0;
    }
    temp_v1_3 = *(u16 *)(temp_v0 + 0x1E);
    if (temp_v1_3 != 0xFFFF)
    {
        *(s32 *)(arg2 + 0x0) = (s32) (*(u16 *)(temp_v0 + 0x1C) + (temp_v1_3 * var_s2));
    }
    else
    {
        var_s0_3 = 1;
        *(s32 *)(arg2 + 0x0) = (s32) *(u16 *)(temp_v0 + 0x1C);
        if (var_s2 != 0)
        {
            do
            {
                temp_lo_2 = *(u8 *)(temp_v0 + 0x35) * var_s0_3;
                var_s0_3 += 1;
                *(s32 *)(arg2 + 0x0) = func_800C19D0(*(s32 *)(arg2 + 0x0), (u32) ((*(u8 *)(temp_v0 + 0x34) * 4) + temp_lo_2) >> 2, 1);
            } while (var_s2 >= var_s0_3);
        }
    }
    temp_v0_6 = func_800BD414(0, 0x2938);
    switch (temp_v0_6) {                            /* irregular */
    case 1:
        var_v0 = *(s32 *)(arg2 + 0x0) * 2;
block_20:
        *(s32 *)(arg2 + 0x0) = var_v0;
        break;
    case 2:
        var_v0 = *(s32 *)(arg2 + 0x0) * 3;
        goto block_20;
    }
    if (func_800BD414(0, 0xFFE) == 0)
    {
        if (*(s32 *)(arg2 + 0x0) == 0)
        {
            goto block_24;
        }
    }
    else
    {
block_24:
        *(s32 *)(arg2 + 0x0) = 1;
    }
    temp_v1_4 = ((*(s32 *)(arg2 + 0x8) & 0xFF000000) | (*(s32 *)(arg2 + 0x0) & 0xFFFFFF)) & 0x80FFFFFF;
    *(s32 *)(arg2 + 0x8) = temp_v1_4;
    *(s32 *)(arg2 + 0x4) = (s32) *(s32 *)(arg2 + 0x0);
    *(s32 *)(arg2 + 0x8) = (s32) ((temp_v1_4 & 0x7FFFFFFF) | (((u8) *(u8 *)(temp_v0 + 0x3F) >> 7) << 0x1F));
    temp_s0 = *(u8 *)(arg1 + 0x33) * 2;
    if (temp_s0 < 0x100U)
    {
        *(u16 *)(arg2 + 0x68) = temp_s0;
        return;
    }
    *(u16 *)(arg2 + 0x68) = 0xFFU;
}

/**
 * @brief Derive a track-tempo-scaled value from an 8-bit add/sub bitmask pair
 * against D_80123FB0's per-index byte table, clamped to a 4..10 range and
 * scaled by func_800B3670's result.
 * @param arg0 Record with add/sub bitmasks at 0x15/0x16 and a mode byte at 0x3F.
 * @return Value clamped to the range 1..99.
 */
s32 func_800B42B4(u8 *arg0)
{
    u32 add_mask;
    u32 sub_mask;
    u32 count;
    s32 value;
    s32 clamped;
    u32 scaled;

    count = 0;
    value = 8;
    add_mask = arg0[0x15];
    sub_mask = arg0[0x16];

    for (; count < 8; count++)
    {
        if (add_mask & 1)
        {
            value += D_80123FB0->unkC[count];
        }
        if (sub_mask & 1)
        {
            value -= D_80123FB0->unkC[count];
        }
        add_mask >>= 1;
        sub_mask >>= 1;
    }

    if (value < 5)
    {
        clamped = 4;
    }
    else
    {
        clamped = value;
        if (clamped >= 0xB)
        {
            clamped = 0xA;
        }
    }
    value = clamped;

    scaled = (u32)(func_800B3670(arg0[0x3F] & 4) * value) >> 3;

    if (scaled >= 2)
    {
        if (scaled >= 0x64)
        {
            scaled = 0x63;
        }
    }
    else
    {
        scaled = 1;
    }

    return scaled;
}
