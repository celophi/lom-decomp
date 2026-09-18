#include "common.h"

extern u8 *D_80122B74;
extern u8 *D_80123FC4;
extern u8 *D_80123FC0;
void func_800BFE70(s32, s32, u8 *);
void func_800C37A8(u32, void *);
#define INIT_U8(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define INIT_U16(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define INIT_U32(p, o) (*(u32 *)((u8 *)(p) + (o)))
#define INIT_PTR(p, o) (*(u8 **)((u8 *)(p) + (o)))

/** @brief Two-byte staged entry whose value byte feeds a packed record nibble. */
typedef struct
{
    u8 unk0;
    u8 value;
} StagedNibbleEntry;

/**
 * @brief Initialize a generated record's text and packed attribute fields.
 */
void func_800BFA34(void)
{
    s32 copy_index;
    u8 copy_value;
    u8 nibble_0;
    u8 nibble_1;
    u8 nibble_2;
    u8 nibble_3;
    u8 nibble_4;
    u8 nibble_5;
    u8 nibble_6;
    u8 nibble_7;
    u8 *record_14_1;
    u8 *record_18_7;
    u8 *record_1c_0;
    u8 *record_1c_1;
    u8 *record_1c_2;
    u8 *record_1c_3;
    u8 *record_1c_4;
    u8 *record_1c_5;
    u8 *record_1c_6;
    u8 *record_1c_7;
    u8 *record_14_2;
    u8 *record_18_0;
    u8 *record_18_1;
    u8 *record_18_2;
    u8 *record_18_3;
    u8 *record_18_4;
    u8 *record_18_5;
    u8 *record_18_6;
    u8 *record;
    u8 *record_14_0;
    u8 *copy_destination;
    StagedNibbleEntry *staged_nibbles;

    record = INIT_PTR(D_80123FC4, 0);
    if (INIT_U8(record, 0x0) == 0)
    {
        func_800C37A8(INIT_U16(D_80122B74, 0xD8), record + 0x38);
        func_800BFE70((INIT_U8(D_80123FC4, 0x4) * 0x10) + INIT_U8(D_80123FC4, 0x5), INIT_U8(D_80123FC4, 0x6) + 0x24, INIT_PTR(D_80123FC4, 0));
    }
    else if (INIT_U32(record, 0x38) == 0)
    {
        if (INIT_U32(record, 0x3C) == 0)
        {
            func_800C37A8(INIT_U16(D_80122B74, 0xD8), record + 0x38);
        }
    }
    record_14_0 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(record_14_0, 0x14) = (s32) ((INIT_U32(record_14_0, 0x14) & ~0x300) | ((INIT_U8(D_80123FC4, 0x4) & 3) << 8));
    record_14_1 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(record_14_1, 0x14) = (s32) ((INIT_U32(record_14_1, 0x14) & 0xFFFF03FF) | ((INIT_U8(D_80123FC4, 0x5) & 0x3F) << 0xA));
    record_14_2 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(record_14_2, 0x14) = (s32) ((INIT_U32(record_14_2, 0x14) & 0xFFC0FFFF) | ((INIT_U8(D_80123FC4, 0x6) & 0x3F) << 0x10));
    record_18_0 = INIT_PTR(D_80123FC4, 0);
    nibble_0 = 0xF;
    if ((u8)INIT_U8(D_80123FC4, 0xD) < 0x10U)
    {
        staged_nibbles = (StagedNibbleEntry *)(D_80123FC4 + 0xC);
        nibble_0 = staged_nibbles[0].value;
    }
    INIT_U32(record_18_0, 0x18) = (s32) ((INIT_U32(record_18_0, 0x18) & ~0xF) | (nibble_0 & 0xF));
    record_18_1 = INIT_PTR(D_80123FC4, 0);
    nibble_1 = 0xF;
    if ((u8)INIT_U8(D_80123FC4, 0xF) < 0x10U)
    {
        staged_nibbles = (StagedNibbleEntry *)(D_80123FC4 + 0xC);
        nibble_1 = staged_nibbles[1].value;
    }
    INIT_U32(record_18_1, 0x18) = (s32) ((INIT_U32(record_18_1, 0x18) & ~0xF0) | ((nibble_1 & 0xF) * 0x10));
    record_18_2 = INIT_PTR(D_80123FC4, 0);
    nibble_2 = 0xF;
    if ((u8)INIT_U8(D_80123FC4, 0x11) < 0x10U)
    {
        staged_nibbles = (StagedNibbleEntry *)(D_80123FC4 + 0xC);
        nibble_2 = staged_nibbles[2].value;
    }
    INIT_U32(record_18_2, 0x18) = (s32) ((INIT_U32(record_18_2, 0x18) & ~0xF00) | ((nibble_2 & 0xF) << 8));
    record_18_3 = INIT_PTR(D_80123FC4, 0);
    nibble_3 = 0xF;
    if ((u8)INIT_U8(D_80123FC4, 0x13) < 0x10U)
    {
        staged_nibbles = (StagedNibbleEntry *)(D_80123FC4 + 0xC);
        nibble_3 = staged_nibbles[3].value;
    }
    INIT_U32(record_18_3, 0x18) = (s32) ((INIT_U32(record_18_3, 0x18) & 0xFFFF0FFF) | ((nibble_3 & 0xF) << 0xC));
    record_18_4 = INIT_PTR(D_80123FC4, 0);
    nibble_4 = 0xF;
    if ((u8)INIT_U8(D_80123FC4, 0x15) < 0x10U)
    {
        staged_nibbles = (StagedNibbleEntry *)(D_80123FC4 + 0xC);
        nibble_4 = staged_nibbles[4].value;
    }
    INIT_U32(record_18_4, 0x18) = (s32) ((INIT_U32(record_18_4, 0x18) & 0xFFF0FFFF) | ((nibble_4 & 0xF) << 0x10));
    record_18_5 = INIT_PTR(D_80123FC4, 0);
    nibble_5 = 0xF;
    if ((u8)INIT_U8(D_80123FC4, 0x17) < 0x10U)
    {
        staged_nibbles = (StagedNibbleEntry *)(D_80123FC4 + 0xC);
        nibble_5 = staged_nibbles[5].value;
    }
    INIT_U32(record_18_5, 0x18) = (s32) ((INIT_U32(record_18_5, 0x18) & 0xFF0FFFFF) | ((nibble_5 & 0xF) << 0x14));
    record_18_6 = INIT_PTR(D_80123FC4, 0);
    nibble_6 = 0xF;
    if ((u8)INIT_U8(D_80123FC4, 0x19) < 0x10U)
    {
        staged_nibbles = (StagedNibbleEntry *)(D_80123FC4 + 0xC);
        nibble_6 = staged_nibbles[6].value;
    }
    INIT_U32(record_18_6, 0x18) = (s32) ((INIT_U32(record_18_6, 0x18) & 0xF0FFFFFF) | ((nibble_6 & 0xF) << 0x18));
    record_18_7 = INIT_PTR(D_80123FC4, 0);
    nibble_7 = 0xF;
    if ((u8)INIT_U8(D_80123FC4, 0x1B) < 0x10U)
    {
        staged_nibbles = (StagedNibbleEntry *)(D_80123FC4 + 0xC);
        nibble_7 = staged_nibbles[7].value;
    }
    INIT_U32(record_18_7, 0x18) = (s32) ((INIT_U32(record_18_7, 0x18) & 0x0FFFFFFF) | (nibble_7 << 0x1C));
    record_1c_0 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(record_1c_0, 0x1C) = (s32) ((INIT_U32(record_1c_0, 0x1C) & ~0xF) | (INIT_U8(D_80123FC4, 0x20) & 0xF));
    record_1c_1 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(record_1c_1, 0x1C) = (s32) ((INIT_U32(record_1c_1, 0x1C) & ~0xF0) | (((u32) INIT_U32(D_80123FC4, 0x20) >> 4) & 0xF0));
    record_1c_2 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(record_1c_2, 0x1C) = (s32) ((INIT_U32(record_1c_2, 0x1C) & ~0xF00) | ((INIT_U16(D_80123FC4, 0x22) & 0xF) << 8));
    record_1c_3 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(record_1c_3, 0x1C) = (s32) ((INIT_U32(record_1c_3, 0x1C) & 0xFFFF0FFF) | ((INIT_U8(D_80123FC4, 0x23) & 0xF) << 0xC));
    record_1c_4 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(record_1c_4, 0x1C) = (s32) ((INIT_U32(record_1c_4, 0x1C) & 0xFFF0FFFF) | ((INIT_U8(D_80123FC4, 0x24) & 0xF) << 0x10));
    record_1c_5 = INIT_PTR(D_80123FC4, 0);
    copy_index = 0;
    INIT_U32(record_1c_5, 0x1C) = (s32) ((INIT_U32(record_1c_5, 0x1C) & 0xFF0FFFFF) | ((((u32) INIT_U32(D_80123FC4, 0x24) >> 8) & 0xF) << 0x14));
    record_1c_6 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(record_1c_6, 0x1C) = (s32) ((INIT_U32(record_1c_6, 0x1C) & 0xF0FFFFFF) | ((INIT_U16(D_80123FC4, 0x26) & 0xF) << 0x18));
    record_1c_7 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(record_1c_7, 0x1C) = (s32) ((INIT_U32(record_1c_7, 0x1C) & 0x0FFFFFFF) | (INIT_U8(D_80123FC4, 0x27) << 0x1C));
    do
    {
        copy_value = INIT_U8((u8 *)((s32)copy_index + (s32)D_80123FC4), 0x2A);
        copy_destination = INIT_PTR(D_80123FC4, 0) + copy_index;
        copy_index += 1;
        INIT_U8(copy_destination, 0x20) = copy_value;
    } while (copy_index < 3);
    INIT_U8(INIT_PTR(D_80123FC4, 0), 0x23) = (u8) INIT_U8(D_80123FC4, 0x29);
    INIT_U32(INIT_PTR(D_80123FC4, 0), 0x34) = 0;
}


typedef struct
{
    u8 pad0[4];
    u16 unk4;
} TableEntryB800BFE70;

u8* func_800C1E40(s32 arg0);

/**
 * @brief Copy two encoded text entries into a destination buffer.
 * @param arg0 Index of the second source entry.
 * @param arg1 Index of the first source entry.
 * @param arg2 Destination buffer.
 */
void func_800BFE70(s32 arg0, s32 arg1, u8* arg2)
{
    u8* base;
    u8* src;
    u8* dst;
    s32 c2;

    dst = arg2;
    base = func_800C1E40(8);

    src = base + (((TableEntryB800BFE70*)(base + (arg1 << 1)))->unk4 + 4);
    if (*src != 0)
    {
        s32 c1;

        do
        {
            c1 = *src;
            if (c1 < 0x20)
            {
                if (c1 >= 0x1D)
                {
                    *dst = c1;
                    src += 1;
                    dst += 1;
                }
            }
            c2 = *src;
            src += 1;
            *dst = c2;
            dst += 1;
        } while (*src != 0);
    }

    src = base + (((TableEntryB800BFE70*)(base + (arg0 << 1)))->unk4 + 4);
    if (*src != 0)
    {
        s32 c1;

        do
        {
            c1 = *src;
            if (c1 < 0x20)
            {
                if (c1 >= 0x1D)
                {
                    *dst = c1;
                    src += 1;
                    dst += 1;
                }
            }
            c2 = *src;
            src += 1;
            *dst = c2;
            dst += 1;
        } while (*src != 0);
    }

    *dst = 0;
}


/** @brief Byte fields used to derive the result stats. */
typedef struct
{
    u8 pad[5];
    u8 type, level;
    u8 pad7[6];
    u8 bonus;
    u8 pad_e[14];
    u8 value;
    u8 pad_1d[17];
    u8 stats[6];
    u8 extra;
    u8 pad35[7];
    u8 weights[4];
    u8 multipliers[4];
} Source;
/** @brief Access view for type weights and level divisors. */
typedef struct
{
    u8 pad[8];
    u8 weight;
    u8 pad9[3];
    u8 factor;
    u8 pad_d[0x179];
    u16 divisor;
} Table;
/** @brief Destination fields populated from the selected source. */
typedef struct
{
    u8 pad[0x24];
    u16 power;
    u8 stats[6];
    u8 extra;
    u8 pad_2d;
    u16 value;
    u8 scaled[4];
} Result;


/**
 * @brief Compute capped power, copy stats, and scale four source multipliers.
 * @param result Destination for the derived fields.
 */
void func_800BFF90(Result *result)
{
    s32 i, sum, bonus, offset, power;
    Table *level;
    u16 divisor;
    Source *source;
    i = 0;
    sum = 0;
    do
    {
        offset = i + ((Source *)D_80123FC4)->type * 12;
        sum += ((Table *)((u8 *)D_80123FC0 + offset))->weight * ((Source *)D_80123FC4)->weights[i];
        i++;
    } while (i < 4);
    i = 0;
    bonus = 0;
    source = (Source *)D_80123FC4;
    do
    {
        bonus += ((Source *)((u8 *)source + i * 2))->bonus;
        i++;
    } while (i < 8);
    level = (Table *)((u8 *)D_80123FC0 + ((Source *)D_80123FC4)->level * 20);
    divisor = level->divisor;
    power = (sum * (bonus + divisor) / divisor) >> 7;
    result->power = power;
    i = 0;
    if ((u32)(power & 0xFFFF) >= 1000)
    {
        result->power = 999;
    }
    do
    {
        result->stats[i] = ((Source *)D_80123FC4)->stats[i];
        i++;
    } while (i < 6);
    i = 0;
    result->extra = ((Source *)D_80123FC4)->extra;
    result->value = ((Source *)D_80123FC4)->value;
    do
    {
        level = (Table *)((u8 *)D_80123FC0 + (i + ((Source *)D_80123FC4)->type * 12));
        result->scaled[i] = (level->factor * ((Source *)D_80123FC4)->multipliers[i]) >> 6;
        i++;
    } while (i < 4);
}


extern u8 *D_80123FC0;
extern u8 *D_80123FC4;

/**
 * @brief Populate the mode-1 derived fields in a field record.
 * @param record Destination record to update from the active field tables.
 */
void func_800C015C(u8 *record)
{
    s32 i;
    s32 scaled_value;
    s32 product;

    i = 0;
    do
    {
        {
            u8 *source;

            source = D_80123FC0 + (i + D_80123FC4[5] * 0xC);
            scaled_value = (source[0xC8] * (D_80123FC4 + i)[0x40]) >> 6;
        }

        *(s16 *)(record + 0x24 + i * 2) = (s16)scaled_value;
        if ((u32)scaled_value >= 0x3E8)
        {
            *(s16 *)(record + 0x24 + i * 2) = 0x3E7;
        }

        {
            u8 *source;

            source = D_80123FC0 + (i + D_80123FC4[5] * 0xC);
            product = source[0xCC] * (D_80123FC4 + i)[0x40];
        }

        {
            u8 *output;

            output = record + i;
            i += 1;
            output[0x30] = (s8)(product >> 6);
        }
    } while (i < 4);

    record[0x2C] = D_80123FC4[0x36];
    record[0x2D] = D_80123FC4[0x35];
    *(s16 *)(record + 0x2E) = D_80123FC4[0x1C];
}
