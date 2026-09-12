#include "common.h"

extern u8 *func_800C1E40(s32);
extern s32 rand(void);
extern u8 *D_80122B74;
/**
 * @brief Combine random table selections into a record effect and clear its payload.
 * @param arg0 Record index in the field context.
 * @param arg1 Effect destination index within the record.
 */
void func_800C0260(s32 arg0, s32 arg1)
{
    s32 temp_a1;
    s32 temp_s2;
    u8 *temp_s3;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_s0;
    s32 temp_s1;
    s32 var_s1;
    u8 *temp_a0;
    u8 *temp_a0_2;
    u8 *temp_a2;
    u8 *var_a0;

    temp_s3 = func_800C1E40(0xB);
    temp_s2 = arg0 * 0x8C;
    temp_a0 = D_80122B74 + temp_s2;
    if ((((u32) * (u32 *)(temp_a0 + 0x26E4) >> 0xC) & 0xF) == 1)
    {
        var_s0 = 0;
        temp_s1 =
            *(temp_s3 + ((rand() & 0xF) + ((D_80122B74[temp_s2 + 0x26EC] - 0x58) * 0x10)) + 4);
        var_s1 = temp_s1 |
                 *(temp_s3 + ((rand() & 0xF) + ((D_80122B74[temp_s2 + 0x26EC] - 0x58) * 0x10)) + 4);
    }
    else
    {
        var_s0 = 1;
        temp_v0 = rand();
        var_a0 = D_80122B74 + temp_s2;
        var_s1 = *(temp_s3 + ((temp_v0 & 0xF) + ((*(u8 *)(var_a0 + 0x26EC) - 0x58) * 0x10)) + 4);
        if ((((u32) * (u32 *)(var_a0 + 0x26E4) >> 0xC) & 0xF) > 1)
        {
            do
            {
                temp_v0 = rand() & 0xF;
                temp_v1 = var_s0 + temp_s2;
                var_s0 += 1;
                var_a0 = D_80122B74 + temp_s2;
                var_s1 |=
                    *(temp_s3 + (temp_v0 + ((D_80122B74[temp_v1 + 0x26EC] - 0x58) * 0x10)) + 4);
            } while (var_s0 < (((u32) * (u32 *)(var_a0 + 0x26E4) >> 0xC) & 0xF));
        }
        var_s0 = 0;
    }
    temp_v0_2 = arg1 * 0x10;
    temp_v1_2 = arg0 * 0x8C;
    temp_a1 = temp_v0_2 + temp_v1_2;
    temp_a2 = temp_s3 + var_s1;
    D_80122B74[temp_a1 + 0x26F4] = (s8)((temp_a2[0x84] & 0x3F) + 0x60);
    temp_a0_2 = D_80122B74 + temp_a1;
    *(u32 *)(temp_a0_2 + 0x26F4) =
        (s32)((*(u32 *)(temp_a0_2 + 0x26F4) & ~0x300) | (((u8)temp_a2[0x84] >> 6) << 8));
    do
    {
        temp_v0_3 = var_s0 + temp_v0_2;
        var_s0 += 1;
        D_80122B74[temp_v0_3 + temp_v1_2 + 0x26F8] = 0;
    } while (var_s0 < 8);
}

#define EFFECT_U8(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define EFFECT_U32(p, o) (*(u32 *)((u8 *)(p) + (o)))
#define EFFECT_RECORD_COUNT 5
#define EFFECT_RECORD_STRIDE 0x60
#define EFFECT_RECORD_HALF_STRIDE (EFFECT_RECORD_STRIDE / 2)
#define EFFECT_RECORD_BASE_OFFSET 0x2EF4
#define EFFECT_STATUS_OFFSET 0x2F38
#define EFFECT_SUBSLOT_COUNT 3
#define EFFECT_EMPTY_ID 0xFF

void func_800C0814(u8 *, s32, u8 *);

/** @brief D_80122B74 record; per-slot data at 0x8C stride, sub-slots at 0x10. */
typedef struct
{
    u8 pad[0x26F0];
    s32 unk26F0;
    u8 unk26F4;
} Rec;

typedef struct
{
    u8 low;
    u8 high;
} EffectThreshold;

typedef struct
{
    u8 unk00[4];
    EffectThreshold thresholds[256];
} EffectThresholdTable;

extern u8 *D_80122B74;

extern s32 func_800C0560(s32 arg0, s32 arg1, u8 *arg2);
extern void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

/**
 * @brief Populate the eight sub-slot handles for a field record, or fail audio.
 *
 * If func_800C1E40 reports no free channel, notifies the audio driver and
 * returns. Otherwise walks the eight 0x10-byte sub-slots of the @p arg0 record
 * (0x8C stride); each one still flagged 0xFF is resolved via func_800C0560 and
 * its handle stored into @c unk26F0.
 *
 * @param arg0 Field record index.
 * @see decomp.me (100%) TODO
 */
void func_800C0490(s32 arg0)
{
    s32 temp_v0;
    s32 sentinel;
    s32 var_s1;
    Rec *rec;

    temp_v0 = (s32)func_800C1E40(0x10);
    if (temp_v0 == 0)
    {
        akao_set_song_params(0x8001, 0x3E7, 0, 0);
        return;
    }
    var_s1 = 0;
    do
    {
        rec = (Rec *)(D_80122B74 + (arg0 * 0x8C + var_s1 * 0x10));
        if (rec->unk26F4 != (sentinel = 0xFF))
        {
            sentinel = temp_v0;
            ((Rec *)(D_80122B74 + (arg0 * 0x8C + var_s1 * 0x10)))->unk26F0 = func_800C0560(arg0, var_s1, (u8 *)sentinel);
        }
        var_s1 += 1;
    } while (var_s1 < 8);
}

/**
 * @brief Classify an effect slot against the selected effect thresholds.
 * @param arg0 Field record index.
 * @param arg1 Effect slot index within the record.
 * @param arg2 Effect threshold table.
 * @return Classification value from 0 through 3.
 */
s32 func_800C0560(s32 arg0, s32 arg1, u8 *arg2)
{
    s32 flag;
    s32 i;

    {
        s32 record_offset;
        s32 slot_offset;
        s32 table_offset;
        s32 scan_offset;
        u8 *base;

        i = 0;
        base = D_80122B74;
        slot_offset = arg1 * 0x10;
        record_offset = arg0 * 0x8C;
        table_offset = (*(base + (slot_offset + record_offset) + 0x26F4) - 0x60) * 8;
        flag = -1;
        for (i = 0; i < 8; i++)
        {
            scan_offset = i + slot_offset;
            if (*(base + (scan_offset + record_offset) + 0x26F8) < ((EffectThresholdTable *)arg2)->thresholds[*(arg2 + (i + table_offset) + 0x14)].high)
            {
                flag = 0;
                break;
            }
        }
    }
    if (flag != 0)
    {
        return 3;
    }

    {
        s32 record_offset;
        s32 slot_offset;
        s32 table_offset;
        s32 scan_offset;
        u8 *base;

        i = 0;
        base = D_80122B74;
        slot_offset = arg1 * 0x10;
        record_offset = arg0 * 0x8C;
        table_offset = (*(base + (slot_offset + record_offset) + 0x26F4) - 0x60) * 8;
        flag = -1;
        for (i = 0; i < 8; i++)
        {
            scan_offset = i + slot_offset;
            if (*(base + (scan_offset + record_offset) + 0x26F8) < ((EffectThresholdTable *)arg2)->thresholds[*(arg2 + (i + table_offset) + 0x14)].low)
            {
                flag = 0;
                break;
            }
        }
    }
    if (flag != 0)
    {
        return 2;
    }

    {
        s32 record_offset;
        s32 slot_offset;
        u8 *base;

        i = 0;
        base = D_80122B74;
        slot_offset = arg1 * 0x10;
        record_offset = arg0 * 0x8C;
        for (i = 0; i < 8; i++)
        {
            if (*(base + (i + slot_offset + record_offset) + 0x26F8) != 0)
            {
                flag = -1;
            }
        }
    }
    return -flag;
}

/**
 * @brief Apply each unprocessed effect in the five saved records.
 */
void func_800C06E8(void)
{
    s32 processed_mask;
    u8 *effect_table;
    s32 effect_index;
    s32 record_offset;
    s32 record_base_offset;
    s32 record_index;
    s32 inner_record_offset;
    s32 inner_record_base_offset;
    u32 packed_status;
    u8 *record_base;
    u8 *effect_byte;
    u8 *destination;
    u32 value;

    effect_table = func_800C1E40(0x12);
    record_index = 0;
    do
    {
        record_offset = (record_index << 1) * EFFECT_RECORD_HALF_STRIDE;
        record_base_offset = record_offset + EFFECT_RECORD_BASE_OFFSET;
        if (EFFECT_U8(D_80122B74 + record_offset, EFFECT_RECORD_BASE_OFFSET) != 0)
        {
            effect_index = 0;
            inner_record_offset = record_offset;
            inner_record_base_offset = record_base_offset;
            do
            {
                effect_byte = D_80122B74 + (effect_index + inner_record_offset);
                record_base = D_80122B74 + inner_record_offset;
                value = EFFECT_U8(effect_byte, EFFECT_STATUS_OFFSET);
                if (value != EFFECT_EMPTY_ID)
                {
                    packed_status = EFFECT_U32(record_base, EFFECT_STATUS_OFFSET);
                    processed_mask = (packed_status >> 24) & 7;
                    if (!((processed_mask >> effect_index) & 1))
                    {
                        destination = D_80122B74 + inner_record_base_offset;
                        value = (u32)((packed_status & 0xF8FFFFFF) | (((processed_mask | (1 << effect_index)) & 7) << 24));
                        EFFECT_U32(record_base, EFFECT_STATUS_OFFSET) = value;
                        func_800C0814(destination, EFFECT_U8(effect_byte, EFFECT_STATUS_OFFSET), effect_table);
                    }
                }
                effect_index += 1;
            } while (effect_index < EFFECT_SUBSLOT_COUNT);
        }
        record_index += 1;
    } while (record_index < EFFECT_RECORD_COUNT);
}

s32 rand(void); /* extern */

/**
 * @brief Apply packed stat adjustments and effect entries from a selected record.
 * @param arg0 Destination record containing stats and effect flags.
 * @param arg1 Record selector; values outside 0x60 through 0x87 are ignored.
 * @param arg2 Packed adjustment and effect table.
 */
void func_800C0814(u8 *arg0, s32 arg1, u8 *arg2)
{
    s32 temp_a0;
    s32 offset;
    s32 one;
    s32 var_a0;
    s32 var_s2;
    s32 var_s4;
    s32 var_v1;
    s32 var_v1_2;
    u32 temp_v1;
    s32 index;
    s32 temp_s0;
    s32 temp_s1;
    u8 temp_v1_2;
    u8 temp_v1_3;
    u8 *var_a1;
    u8 *var_a1_2;
    u8 *var_v0;

    temp_v1 = arg1 - 0x60;
    if (temp_v1 < 0x28U)
    {
        var_s2 = 0;
        do
        {
            index = temp_v1;
        } while (0);
        offset = index * 0x14;
        do
        {
            var_a1 = arg0 + var_s2;
            temp_a0 = var_a1[0x4C];
            temp_v1_2 = *(arg2 + (var_s2 + offset) + 4);
            temp_a0 = (((u32)temp_a0 >> 4) + (temp_v1_2 & 0xF)) - (temp_v1_2 >> 4);
            if (temp_a0 >= 0)
            {
                var_v1 = 0xF;
                if (temp_a0 < 0x10)
                {
                    var_v1 = temp_a0;
                }
            }
            else
            {
                var_v1 = 0;
            }
            var_s2 += 1;
            var_a1[0x4C] = (u8)((var_a1[0x4C] & 0xF) | (var_v1 * 0x10));
        } while (var_s2 < 8);
        var_s2 = 0;
        offset = index * 0x14;
        do
        {
            var_a1_2 = arg0 + var_s2;
            var_a0 = var_a1_2[0x54];
            temp_v1_3 = *(arg2 + (var_s2 + offset) + 0xC);
            var_a0 = (((u32)var_a0 >> 4) + (temp_v1_3 & 0xF)) - (temp_v1_3 >> 4);
            if (var_a0 >= 0)
            {
                var_v1_2 = 0xF;
                if (var_a0 < 0x10)
                {
                    var_v1_2 = var_a0;
                }
            }
            else
            {
                var_v1_2 = 0;
            }
            var_s2 += 1;
            var_a1_2[0x54] = (u8)((var_a1_2[0x54] & 0xF) | (var_v1_2 * 0x10));
        } while (var_s2 < 4);
        var_s2 = 0;
        one = 1;
        var_s4 = index * 0x14;
        do
        {
            var_v0 = arg2 + var_s4;
            temp_s1 = var_v0[0x10];
            temp_s0 = var_v0[0x11];
            if (index < 8)
            {
                var_a0 = rand() & 0xFF;
                if (var_a0 < (s32)temp_s0)
                {
                    arg0[0x48] = (u8)(arg0[0x48] | (one << temp_s1));
                }
                goto block_25;
            }
            if (index < 0x10)
            {
                var_a0 = rand() & 0xFF;
                if (var_a0 < (s32)temp_s0)
                {
                    arg0[0x48] = (u8)(arg0[0x48] & ~(one << (temp_s1 - 8)));
                }
                goto block_25;
            }
            switch (temp_s1)
            {
            case 0xF0:
                arg0[0x3C] = temp_s0;
                break;
            case 0xF1:
                arg0[0x3D] = temp_s0;
                break;
            }
        block_25:
            var_s4 += 2;
            var_s2 += 1;
        } while (var_s2 < 4);
    }
}
