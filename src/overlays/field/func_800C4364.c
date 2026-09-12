#include "common.h"

extern u8 *func_800C1E40(s32 arg0);
extern u32 D_80051C50[];
extern s8 D_800F0C38[];
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];

typedef struct
{
    u8 pad[0x2B0C];
    u8 unk2B0C;
} NameView;
typedef struct
{
    s32 a[27];
} LocalTableCopy;
typedef struct
{
    u8 pad[4];
    u8 value;
} ResourceByte;

typedef struct
{
    u8 pad[0x2B30];
    unsigned int a0 : 4;
    unsigned int a1 : 4;
    unsigned int a2 : 4;
    unsigned int a3 : 4;
    unsigned int a4 : 4;
    unsigned int a5 : 4;
    unsigned int a6 : 4;
    unsigned int a7 : 4;
    unsigned int a8 : 4;
    unsigned int a9 : 4;
    unsigned int a10 : 4;
    unsigned int a11 : 4;
    unsigned int a12 : 4;
    unsigned int a13 : 4;
    unsigned int a14 : 4;
    unsigned int a15 : 4;
} StatNibbles;

typedef struct
{
    u8 pad[0x2B22];
    u16 hp, stat0, stat1, stat2, stat3, stat4;
} OutputStats;
typedef struct
{
    u8 pad[0x2B48];
    u8 flags0, flags1, flags2, enabled;
    u32 zero;
    unsigned int low : 4;
    unsigned int high : 4;
    unsigned int rest : 24;
} GroupOutput;
typedef struct
{
    u8 pad[0x2B38];
    u16 resistance;
} ResistanceView;
typedef struct
{
    u8 pad[0xCF4];
    unsigned int id : 8;
    unsigned int type : 2;
    unsigned int category : 6;
    unsigned int rest : 16;
} ItemHeader;
#define U8(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define U16(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define U32(p, o) (*(u32 *)((u8 *)(p) + (o)))

/**
 * @brief Recomputes a party member's derived stat block from its gosub result set.
 *
 * Decodes the digit-name glyph runs for the member's level, sums element/attribute
 * counts and resistances across the selected inventory records, clamps each derived
 * value into range, and writes the packed stat fields back into the member's layout
 * record at @c arg0 * 0x14C inside the global menu layout buffer.
 *
 * @param arg0 Party member / layout record index.
 * @see decomp.me (100%)
 */
void func_800C4364(s32 arg0)
{
    s32 local_table[27];
    s32 accum[16];
    s32 count;
    s32 i;
    s32 digit;
    s32 value;
    s32 record_offset;
    s32 work_value;
    s32 type;
    u8 *base;
    u8 *out_base;
    u8 *tb0, *tb1, *tb2, *tb3, *tb4, *tb5;

    *(LocalTableCopy *)local_table = *(LocalTableCopy *)D_80051C50;

    count = 0;
    base = g_menuLayoutBuffer;
    accum[15] = (s32)base[0x29D5];
    digit = accum[15] / 50;
    digit += 11;
    {
        s32 low_index;
        s32 high_index;
        accum[0] =
            ((ResourceByte *)(func_800C1E40(0x100) + (low_index = digit * 2)))->value +
            (((ResourceByte *)(func_800C1E40(0x100) + ((high_index = digit * 2 + 1))))->value << 8);
    }
    {
        s32 low_index;
        s32 high_index;
        accum[1] =
            ((ResourceByte *)(func_800C1E40(0x100) + (low_index = (digit + 1) * 2)))->value +
            (((ResourceByte *)(func_800C1E40(0x100) + ((high_index = (digit + 1) * 2 + 1))))->value
             << 8);
    }
    i = accum[0];
    if (i < accum[1])
    {
        out_base = base;
        record_offset = arg0 * 0x14C;
        do
        {
            if (count < 21)
            {
                ((NameView *)((count + record_offset) + (u32)out_base))->unk2B0C =
                    ((ResourceByte *)(func_800C1E40(0x100) + i))->value;
            }
            count++;
            i++;
        } while (i < accum[1]);
    }

    value = accum[15];
    if (value < 200)
    {
        accum[15] = value % 50 + 1;
        {
            s32 hundreds_digit;
            hundreds_digit = accum[15] / 100;
            if (accum[15] >= 100)
            {
                {
                    s32 low_index;
                    s32 high_index;
                    accum[0] =
                        ((ResourceByte *)(func_800C1E40(0x100) + (low_index = hundreds_digit * 2)))
                            ->value +
                        (((ResourceByte *)(func_800C1E40(0x100) +
                                           ((high_index = hundreds_digit * 2 + 1))))
                             ->value
                         << 8);
                }
                {
                    s32 low_index;
                    s32 high_index;
                    accum[1] = ((ResourceByte *)(func_800C1E40(0x100) +
                                                 (low_index = (hundreds_digit + 1) * 2)))
                                   ->value +
                               (((ResourceByte *)(func_800C1E40(0x100) +
                                                  ((high_index = (hundreds_digit + 1) * 2 + 1))))
                                    ->value
                                << 8);
                }
                i = accum[0];
                if (i < accum[1])
                {
                    out_base = g_menuLayoutBuffer;
                    record_offset = arg0 * 0x14C;
                    do
                    {
                        if (count < 21)
                        {
                            ((NameView *)((count + record_offset) + (u32)out_base))->unk2B0C =
                                ((ResourceByte *)(func_800C1E40(0x100) + i))->value;
                        }
                        count++;
                        i++;
                    } while (i < accum[1]);
                }
            }
        }
        digit = (accum[15] % 100) / 10;
        if (accum[15] >= 10)
        {
            {
                s32 low_index;
                s32 high_index;
                accum[0] =
                    ((ResourceByte *)(func_800C1E40(0x100) + (low_index = digit * 2)))->value +
                    (((ResourceByte *)(func_800C1E40(0x100) + ((high_index = digit * 2 + 1))))
                         ->value
                     << 8);
            }
            {
                s32 low_index;
                s32 high_index;
                accum[1] =
                    ((ResourceByte *)(func_800C1E40(0x100) + (low_index = (digit + 1) * 2)))
                        ->value +
                    (((ResourceByte *)(func_800C1E40(0x100) + ((high_index = (digit + 1) * 2 + 1))))
                         ->value
                     << 8);
            }
            i = accum[0];
            if (i < accum[1])
            {
                out_base = g_menuLayoutBuffer;
                record_offset = arg0 * 0x14C;
                do
                {
                    if (count < 21)
                    {
                        ((NameView *)((count + record_offset) + (u32)out_base))->unk2B0C =
                            ((ResourceByte *)(func_800C1E40(0x100) + i))->value;
                    }
                    count++;
                    i++;
                } while (i < accum[1]);
            }
        }
        digit = accum[15] % 10;
        {
            s32 low_index;
            s32 high_index;
            accum[0] =
                ((ResourceByte *)(func_800C1E40(0x100) + (low_index = digit * 2)))->value +
                (((ResourceByte *)(func_800C1E40(0x100) + ((high_index = digit * 2 + 1))))->value
                 << 8);
        }
        {
            s32 low_index;
            s32 high_index;
            accum[1] =
                ((ResourceByte *)(func_800C1E40(0x100) + (low_index = (digit + 1) * 2)))->value +
                (((ResourceByte *)(func_800C1E40(0x100) + ((high_index = (digit + 1) * 2 + 1))))
                     ->value
                 << 8);
        }
        i = accum[0];
        if (i < accum[1])
        {
            out_base = g_menuLayoutBuffer;
            record_offset = arg0 * 0x14C;
            do
            {
                if (count < 21)
                {
                    ((NameView *)((count + record_offset) + (u32)out_base))->unk2B0C =
                        ((ResourceByte *)(func_800C1E40(0x100) + i))->value;
                }
                count++;
                i++;
            } while (i < accum[1]);
        }
        accum[0] = func_800C1E40(0x100)[0x18] + (func_800C1E40(0x100)[0x19] << 8);
        accum[1] = func_800C1E40(0x100)[0x1A] + (func_800C1E40(0x100)[0x1B] << 8);
        i = accum[0];
        if (i < accum[1])
        {
            out_base = g_menuLayoutBuffer;
            record_offset = arg0 * 0x14C;
            do
            {
                if (count < 21)
                {
                    ((NameView *)((count + record_offset) + (u32)out_base))->unk2B0C =
                        ((ResourceByte *)(func_800C1E40(0x100) + i))->value;
                }
                count++;
                i++;
            } while (i < accum[1]);
        }
    }
    if (count < 21)
    {
        u8 *end_base = g_menuLayoutBuffer;
        s32 end_offset = arg0 * 0x14C;
        ((NameView *)(end_base + (count + end_offset)))->unk2B0C = 0;
    }

    i = 0;
    accum[0] = 0;
    if (g_gosub_result_count > 0)
    {
        u8 *scan_base = g_menuLayoutBuffer;
        u8 *record_base = scan_base + 0xCE0;
        s32 result_count = g_gosub_result_count;
        s32 *results;
        results = g_gosub_result_values;
        do
        {
            work_value = *results << 6;
            if (((U32(scan_base, work_value + 0xCF4) >> 8) & 3) == 0)
            {
                accum[0] += U16(record_base, work_value + 0x24);
            }
            i++;
            results++;
        } while (i < result_count);
    }
    accum[0] = accum[0] < 10 ? 10 : accum[0] > 200 ? 200 : accum[0];
    ((OutputStats *)(g_menuLayoutBuffer + arg0 * 0x14C))->stat0 = (u16)accum[0];

    i = 0;
    accum[0] = 0;
    accum[1] = 0;
    accum[2] = 0;
    accum[3] = 0;
    if (g_gosub_result_count > 0)
    {
        s32 selected_type = 1;
        u8 *scan_base = g_menuLayoutBuffer;
        u8 *record_base = scan_base + 0xCE0;
        s32 result_count = g_gosub_result_count;
        s32 *results;
        results = g_gosub_result_values;
        do
        {
            work_value = *results << 6;
            if (((U32(scan_base, work_value + 0xCF4) >> 8) & 3) == selected_type)
            {
                accum[0] += U16(record_base, work_value + 0x24);
                accum[1] += U16(record_base, work_value + 0x26);
                accum[2] += U16(record_base, work_value + 0x28);
                accum[3] += U16(record_base, work_value + 0x2A);
            }
            i++;
            results++;
        } while (i < result_count);
    }
    {
        u8 *stat_base;
        i = 0;
        stat_base = g_menuLayoutBuffer;
        for (; i < 4; i++)
        {
            s32 output_offset;
            accum[i] = accum[i] < 0 ? 0 : accum[i] > 99 ? 99 : accum[i];
            ((OutputStats *)((output_offset = arg0 * 0x14C + i * 2) + (u32)stat_base))->stat1 =
                (u16)accum[i];
        }
    }

    accum[0] = 0;
    accum[1] = 0;
    accum[2] = 0;
    accum[3] = 0;
    accum[4] = 0;
    accum[5] = 0;
    accum[6] = 0;
    accum[7] = 0;
    accum[8] = 0;
    accum[9] = 0;
    accum[10] = 0;
    accum[11] = 0;
    accum[12] = 0;
    accum[13] = 0;
    accum[14] = 0;
    accum[15] = 0;
    i = 0;
    if (g_gosub_result_count > i)
    {
        u8 *item_base = g_menuLayoutBuffer;
        s32 result_count = g_gosub_result_count;
        s32 *results = g_gosub_result_values;
        do
        {
            u8 *item = (u8 *)((*results << 6) + (u32)item_base);
            type = (U32(item, 0xCF4) >> 8) & 3;
            if (type == 0)
            {
                accum[0] += U32(item, 0xCF8) & 0xF;
                accum[1] += U8(item, 0xCF8) >> 4;
                accum[2] += (U32(item, 0xCF8) >> 8) & 0xF;
                accum[3] += (U32(item, 0xCF8) >> 12) & 0xF;
                accum[4] += U16(item, 0xCFA) & 0xF;
                accum[5] += (U32(item, 0xCF8) >> 20) & 0xF;
                accum[6] += U8(item, 0xCFB) & 0xF;
                accum[7] += U32(item, 0xCF8) >> 28;
            }
            else if (type == 1)
            {
                accum[8] += U32(item, 0xCF8) & 0xF;
                accum[9] += U8(item, 0xCF8) >> 4;
                accum[10] += (U32(item, 0xCF8) >> 8) & 0xF;
                accum[11] += (U32(item, 0xCF8) >> 12) & 0xF;
                accum[12] += U16(item, 0xCFA) & 0xF;
                accum[13] += (U32(item, 0xCF8) >> 20) & 0xF;
                accum[14] += U8(item, 0xCFB) & 0xF;
                accum[15] += U32(item, 0xCF8) >> 28;
            }
            i++;
            results++;
        } while (i < result_count);
    }
    for (i = 0; i < 16; i++)
    {
        if (g_menuLayoutBuffer[0x29D5] >= 200)
        {
            accum[i] += 2;
        }
        accum[i] = accum[i] < 0 ? 0 : accum[i] > 9 ? 9 : accum[i];
    }

    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a0 = accum[0];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a1 = accum[1];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a2 = accum[2];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a3 = accum[3];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a4 = accum[4];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a5 = accum[5];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a6 = accum[6];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a7 = accum[7];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a8 = accum[8];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a9 = accum[9];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a10 = accum[10];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a11 = accum[11];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a12 = accum[12];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a13 = accum[13];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a14 = accum[14];
    ((StatNibbles *)(g_menuLayoutBuffer + arg0 * 0x14C))->a15 = accum[15];

    accum[0] = 0;
    accum[1] = 0;
    accum[2] = 0;
    accum[3] = 0;
    accum[4] = 0;
    accum[5] = 0;
    accum[6] = 0;
    accum[7] = 0;
    for (i = 0; i < g_gosub_result_count; i++)
    {
        s8 *resistance_table = D_800F0C38;
        u8 *item = g_menuLayoutBuffer + (work_value = g_gosub_result_values[i] << 6);
        accum[0] += resistance_table[U32(item, 0xCFC) & 0xF];
        accum[1] += resistance_table[U8(item, 0xCFC) >> 4];
        accum[2] += resistance_table[(U32(item, 0xCFC) >> 8) & 0xF];
        accum[3] += resistance_table[(U32(item, 0xCFC) >> 12) & 0xF];
        accum[4] += resistance_table[U16(item, 0xCFE) & 0xF];
        accum[5] += resistance_table[(U32(item, 0xCFC) >> 20) & 0xF];
        accum[6] += resistance_table[U8(item, 0xCFF) & 0xF];
        accum[7] += resistance_table[U32(item, 0xCFC) >> 28];
    }
    for (i = 0; i < 8; i++)
    {
        accum[i] = (accum[i] * 5) + 20;
        accum[i] = accum[i] < 20 ? 20 : accum[i] > 99 ? 99 : accum[i];
        ((ResistanceView *)(g_menuLayoutBuffer + arg0 * 0x14C + i * 2))->resistance &= 0xFE00;
        ((ResistanceView *)(g_menuLayoutBuffer + arg0 * 0x14C + i * 2))->resistance = (u16)accum[i]
                                                                                      << 9;
    }

    accum[0] = 0;
    i = 0;
    if (g_gosub_result_count > i)
    {
        s32 selected_type = 1;
        u8 *scan_base = g_menuLayoutBuffer;
        u8 *record_base = scan_base + 0xCE0;
        s32 result_count = g_gosub_result_count;
        s32 *results;
        results = g_gosub_result_values;
        do
        {
            work_value = *results << 6;
            if (((U32(scan_base, work_value + 0xCF4) >> 8) & 3) == selected_type)
            {
                accum[0] |= U8(record_base, work_value + 0x2c);
            }
            i++;
            results++;
        } while (i < result_count);
    }
    g_menuLayoutBuffer[arg0 * 0x14C + 0x2B48] = (u8)accum[0];
    accum[0] = 0;
    i = 0;
    if (g_gosub_result_count > 0)
    {
        u8 *scan_base = g_menuLayoutBuffer;
        u8 *record_base = scan_base + 0xCE0;
        s32 result_count = g_gosub_result_count;
        s32 *results;
        results = g_gosub_result_values;
        do
        {
            work_value = *results << 6;
            if (((U32(scan_base, work_value + 0xCF4) >> 8) & 3) == 0)
            {
                accum[0] |= U8(record_base, work_value + 0x2c);
            }
            i++;
            results++;
        } while (i < result_count);
    }
    g_menuLayoutBuffer[arg0 * 0x14C + 0x2B49] = (u8)accum[0];
    accum[0] = 0;
    i = 0;
    if (g_gosub_result_count > 0)
    {
        s32 selected_type = 1;
        u8 *scan_base = g_menuLayoutBuffer;
        u8 *record_base = scan_base + 0xCE0;
        s32 result_count = g_gosub_result_count;
        s32 *results;
        results = g_gosub_result_values;
        do
        {
            work_value = *results << 6;
            if (((U32(scan_base, work_value + 0xCF4) >> 8) & 3) == selected_type)
            {
                accum[0] |= U8(record_base, work_value + 0x2d);
            }
            i++;
            results++;
        } while (i < result_count);
    }

    tb0 = g_menuLayoutBuffer;
    (tb0 + arg0 * 0x14C)[0x2B4A] = (u8)accum[0];
    (tb0 + arg0 * 0x14C)[0x2B4B] = 1;
    U32((tb0 + arg0 * 0x14C), 0x2B4C) = 0;
    ((GroupOutput *)(tb0 + arg0 * 0x14C))->low = 0;
    for (i = 0; i < g_gosub_result_count; i++)
    {
        if (((ItemHeader *)(g_menuLayoutBuffer + (g_gosub_result_values[i] << 6)))->type == 0)
        {
            ((GroupOutput *)(tb0 + arg0 * 0x14C))->low = (u8)
                local_table[((ItemHeader *)(g_menuLayoutBuffer + (g_gosub_result_values[i] << 6)))
                                ->category];
        }
    }

    i = 0;
    accum[0] = 0;
    tb1 = g_menuLayoutBuffer;
    ((GroupOutput *)(tb1 + arg0 * 0x14C))->high = 4;
    {
        s32 result_count = g_gosub_result_count;
        s32 *results;
        if (result_count > 0)
        {
            u8 *scan_base = g_menuLayoutBuffer;
            s32 selected_type = 1;
            results = g_gosub_result_values;
            do
            {
                if (((U32(scan_base, (*results << 6) + 0xCF4) >> 8) & 3) == selected_type)
                {
                    accum[0]++;
                }
                i++;
                results++;
            } while (i < result_count);
        }
    }
    if (accum[0] == 2)
    {
        tb2 = g_menuLayoutBuffer;
        ((GroupOutput *)(tb2 + arg0 * 0x14C))->high = 5;
    }
    if (accum[0] == 3)
    {
        tb3 = g_menuLayoutBuffer;
        ((GroupOutput *)(tb3 + arg0 * 0x14C))->high = 6;
    }
    tb4 = g_menuLayoutBuffer;
    (tb4 + arg0 * 0x14C)[0x2B51] = 0;
    accum[0] = 75 - (((tb4 + arg0 * 0x14C)[0x2B50] >> 4) * 10);
    accum[0] = accum[0] < 0 ? 0 : accum[0] > 50 ? 50 : accum[0];
    tb5 = g_menuLayoutBuffer;
    (tb5 + arg0 * 0x14C)[0x2B52] = (u8)accum[0];
    (tb5 + arg0 * 0x14C)[0x2B53] = 0;
    U32((tb5 + arg0 * 0x14C), 0x2B54) = 0;
    accum[0] = U16((tb5 + arg0 * 0x14C), 0x2B24);
    accum[1] = U16((tb5 + arg0 * 0x14C), 0x2B26);
    accum[2] = U16((tb5 + arg0 * 0x14C), 0x2B28);
    accum[3] = U16((tb5 + arg0 * 0x14C), 0x2B2A);
    accum[4] = U16((tb5 + arg0 * 0x14C), 0x2B2C);
    count = accum[0] + accum[1] + accum[2] + accum[3] + accum[4];
    count = count * 5 >> 1;
    if (count >= 50)
    {
        work_value = 999;
        if (count < 1000)
        {
            work_value = count;
        }
    }
    else
    {
        work_value = 50;
    }
    {
        u8 *hp_base;
        hp_base = g_menuLayoutBuffer;
        U16((hp_base + arg0 * 0x14C), 0x2B22) = (s16)work_value;
    }
}
