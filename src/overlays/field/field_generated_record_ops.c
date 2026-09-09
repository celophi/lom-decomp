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

/** @brief Initializes a generated record name, key and packed attributes.
 * @note Initial nonmatching C recovered from assembly.
 */
void func_800BFA34(void)
{
    s32 var_a2;
    u8 temp_v0;
    u8 var_v1;
    u8 var_v1_2;
    u8 var_v1_3;
    u8 var_v1_4;
    u8 var_v1_5;
    u8 var_v1_6;
    u8 var_v1_7;
    u8 var_v1_8;
    u8 *temp_a0;
    u8 *temp_a0_10;
    u8 *temp_a0_11;
    u8 *temp_a0_12;
    u8 *temp_a0_13;
    u8 *temp_a0_14;
    u8 *temp_a0_15;
    u8 *temp_a0_16;
    u8 *temp_a0_17;
    u8 *temp_a0_18;
    u8 *temp_a0_2;
    u8 *temp_a0_3;
    u8 *temp_a0_4;
    u8 *temp_a0_5;
    u8 *temp_a0_6;
    u8 *temp_a0_7;
    u8 *temp_a0_8;
    u8 *temp_a0_9;
    u8 *temp_a1;
    u8 *temp_a1_2;
    u8 *temp_v1;

    temp_a1 = INIT_PTR(D_80123FC4, 0);
    if (INIT_U8(temp_a1, 0x0) == 0)
    {
        func_800C37A8(INIT_U16(D_80122B74, 0xD8), temp_a1 + 0x38);
        func_800BFE70((INIT_U8(D_80123FC4, 0x4) * 0x10) + INIT_U8(D_80123FC4, 0x5), INIT_U8(D_80123FC4, 0x6) + 0x24, INIT_PTR(D_80123FC4, 0));
    }
    else if (INIT_U32(temp_a1, 0x38) == 0)
    {
        if (INIT_U32(temp_a1, 0x3C) == 0)
        {
            func_800C37A8(INIT_U16(D_80122B74, 0xD8), temp_a1 + 0x38);
        }
    }
    temp_a1_2 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(temp_a1_2, 0x14) = (s32) ((INIT_U32(temp_a1_2, 0x14) & ~0x300) | ((INIT_U8(D_80123FC4, 0x4) & 3) << 8));
    temp_a0 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(temp_a0, 0x14) = (s32) ((INIT_U32(temp_a0, 0x14) & 0xFFFF03FF) | ((INIT_U8(D_80123FC4, 0x5) & 0x3F) << 0xA));
    temp_a0_2 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(temp_a0_2, 0x14) = (s32) ((INIT_U32(temp_a0_2, 0x14) & 0xFFC0FFFF) | ((INIT_U8(D_80123FC4, 0x6) & 0x3F) << 0x10));
    temp_a0_3 = INIT_PTR(D_80123FC4, 0);
    var_v1 = 0xF;
    if ((u8) INIT_U8(D_80123FC4, 0xD) < 0x10U)
    {
        var_v1 = INIT_U8(D_80123FC4, 0xD);
    }
    INIT_U32(temp_a0_3, 0x18) = (s32) ((INIT_U32(temp_a0_3, 0x18) & ~0xF) | (var_v1 & 0xF));
    temp_a0_4 = INIT_PTR(D_80123FC4, 0);
    var_v1_2 = 0xF;
    if ((u8) INIT_U8(D_80123FC4, 0xF) < 0x10U)
    {
        var_v1_2 = INIT_U8(D_80123FC4, 0xF);
    }
    INIT_U32(temp_a0_4, 0x18) = (s32) ((INIT_U32(temp_a0_4, 0x18) & ~0xF0) | ((var_v1_2 & 0xF) * 0x10));
    temp_a0_5 = INIT_PTR(D_80123FC4, 0);
    var_v1_3 = 0xF;
    if ((u8) INIT_U8(D_80123FC4, 0x11) < 0x10U)
    {
        var_v1_3 = INIT_U8(D_80123FC4, 0x11);
    }
    INIT_U32(temp_a0_5, 0x18) = (s32) ((INIT_U32(temp_a0_5, 0x18) & ~0xF00) | ((var_v1_3 & 0xF) << 8));
    temp_a0_6 = INIT_PTR(D_80123FC4, 0);
    var_v1_4 = 0xF;
    if ((u8) INIT_U8(D_80123FC4, 0x13) < 0x10U)
    {
        var_v1_4 = INIT_U8(D_80123FC4, 0x13);
    }
    INIT_U32(temp_a0_6, 0x18) = (s32) ((INIT_U32(temp_a0_6, 0x18) & 0xFFFF0FFF) | ((var_v1_4 & 0xF) << 0xC));
    temp_a0_7 = INIT_PTR(D_80123FC4, 0);
    var_v1_5 = 0xF;
    if ((u8) INIT_U8(D_80123FC4, 0x15) < 0x10U)
    {
        var_v1_5 = INIT_U8(D_80123FC4, 0x15);
    }
    INIT_U32(temp_a0_7, 0x18) = (s32) ((INIT_U32(temp_a0_7, 0x18) & 0xFFF0FFFF) | ((var_v1_5 & 0xF) << 0x10));
    temp_a0_8 = INIT_PTR(D_80123FC4, 0);
    var_v1_6 = 0xF;
    if ((u8) INIT_U8(D_80123FC4, 0x17) < 0x10U)
    {
        var_v1_6 = INIT_U8(D_80123FC4, 0x17);
    }
    INIT_U32(temp_a0_8, 0x18) = (s32) ((INIT_U32(temp_a0_8, 0x18) & 0xFF0FFFFF) | ((var_v1_6 & 0xF) << 0x14));
    temp_a0_9 = INIT_PTR(D_80123FC4, 0);
    var_v1_7 = 0xF;
    if ((u8) INIT_U8(D_80123FC4, 0x19) < 0x10U)
    {
        var_v1_7 = INIT_U8(D_80123FC4, 0x19);
    }
    INIT_U32(temp_a0_9, 0x18) = (s32) ((INIT_U32(temp_a0_9, 0x18) & 0xF0FFFFFF) | ((var_v1_7 & 0xF) << 0x18));
    temp_a0_10 = INIT_PTR(D_80123FC4, 0);
    var_v1_8 = 0xF;
    if ((u8) INIT_U8(D_80123FC4, 0x1B) < 0x10U)
    {
        var_v1_8 = INIT_U8(D_80123FC4, 0x1B);
    }
    INIT_U32(temp_a0_10, 0x18) = (s32) ((INIT_U32(temp_a0_10, 0x18) & 0x0FFFFFFF) | (var_v1_8 << 0x1C));
    temp_a0_11 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(temp_a0_11, 0x1C) = (s32) ((INIT_U32(temp_a0_11, 0x1C) & ~0xF) | (INIT_U8(D_80123FC4, 0x20) & 0xF));
    temp_a0_12 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(temp_a0_12, 0x1C) = (s32) ((INIT_U32(temp_a0_12, 0x1C) & ~0xF0) | (((u32) INIT_U32(D_80123FC4, 0x20) >> 4) & 0xF0));
    temp_a0_13 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(temp_a0_13, 0x1C) = (s32) ((INIT_U32(temp_a0_13, 0x1C) & ~0xF00) | ((INIT_U16(D_80123FC4, 0x22) & 0xF) << 8));
    temp_a0_14 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(temp_a0_14, 0x1C) = (s32) ((INIT_U32(temp_a0_14, 0x1C) & 0xFFFF0FFF) | ((INIT_U8(D_80123FC4, 0x23) & 0xF) << 0xC));
    temp_a0_15 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(temp_a0_15, 0x1C) = (s32) ((INIT_U32(temp_a0_15, 0x1C) & 0xFFF0FFFF) | ((INIT_U8(D_80123FC4, 0x24) & 0xF) << 0x10));
    temp_a0_16 = INIT_PTR(D_80123FC4, 0);
    var_a2 = 0;
    INIT_U32(temp_a0_16, 0x1C) = (s32) ((INIT_U32(temp_a0_16, 0x1C) & 0xFF0FFFFF) | ((((u32) INIT_U32(D_80123FC4, 0x24) >> 8) & 0xF) << 0x14));
    temp_a0_17 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(temp_a0_17, 0x1C) = (s32) ((INIT_U32(temp_a0_17, 0x1C) & 0xF0FFFFFF) | ((INIT_U16(D_80123FC4, 0x26) & 0xF) << 0x18));
    temp_a0_18 = INIT_PTR(D_80123FC4, 0);
    INIT_U32(temp_a0_18, 0x1C) = (s32) ((INIT_U32(temp_a0_18, 0x1C) & 0x0FFFFFFF) | (INIT_U8(D_80123FC4, 0x27) << 0x1C));
    do
    {
        temp_v0 = INIT_U8(D_80123FC4 + var_a2, 0x2A);
        temp_v1 = INIT_PTR(D_80123FC4, 0) + var_a2;
        var_a2 += 1;
        INIT_U8(temp_v1, 0x20) = temp_v0;
    } while (var_a2 < 3);
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
    s32 i, sum, bonus, product, power;
    Table *level;
    u16 divisor;
    Source *source;
    i = 0;
    sum = 0;
    do
    {
        sum += ((Table *)((u8 *)D_80123FC0 + (((Source *)D_80123FC4)->type * 12 + i)))->weight *
               ((Source *)D_80123FC4)->weights[i];
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
        level = (Table *)((u8 *)D_80123FC0 + (((Source *)D_80123FC4)->type * 12 + i));
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
