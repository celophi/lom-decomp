#include "common.h"

/** @brief Layout buffer view for active records, identity words, and packed metadata. */
typedef struct FieldMenuRecordLayout
{
    u8 pad0[0x640];
    u8 unk640;
    u8 pad641[0x37];
    u32 unk678, unk67C;
    u8 pad680[0x660];
    u8 unkCE0;
    u8 padCE1[0x37];
    u32 unkD18, unkD1C;
    u8 padD20[0x2440];
    u8 unk3160;
    u8 pad3161[0x13];
    union
    {
        u32 word;
        u16 halves[2];
    } packed;
    u8 pad3178[0x20];
    u32 unk3198, unk319C;
} FieldMenuRecordLayout;

extern u8 g_menuLayoutBuffer[], D_80122A08[], D_800F0E98[];
extern u8 D_80122C02, D_80122C03, D_80122C04, D_80122C0C;
void func_800B2844();

/**
 * @brief Validate a shared equipment record and prepare its display parameters.
 * @note Nonmatching m2c translation; retains byte, halfword, and word views
 * of the packed record, and the extra fourth argument at two dispatch sites.
 */
void func_800C8A2C(void)
{
    u8 *var_a0;
    u8 *var_a0_2;
    u8 *var_a0_3;
    s32 temp_s5;
    s32 temp_v0_2;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a1_3;
    s32 var_s0;
    s32 var_t0;
    s8 temp_s2;
    s8 var_s3;
    s8 var_v1;
    u16 var_s1;
    u32 temp_a0;
    u32 temp_a1;
    u32 temp_v0;
    u8 var_s4;
    u8 *temp_a2;
    u8 *temp_a2_2;
    u8 *temp_a2_3;
    u8 *temp_v1;
    u8 *temp_v1_2;

    (&D_80122C02)[1] = 0;
    temp_v1 = (D_80122C02 << 6) + D_80122A08;
    var_t0 = 0;
    if (*(u8 *)(temp_v1 + 0x0) == 0)
    {
        (&D_80122C02)[1] = 1;
        return;
    }
    var_a1 = 0;
    var_a0 = g_menuLayoutBuffer;
loop_6:
    if ((*(u8 *)(var_a0 + 0xCE0) == 0) || (*(u32 *)(var_a0 + 0xD18) != *(u32 *)(temp_v1 + 0x38)) || (*(u32 *)(var_a0 + 0xD1C) != *(u32 *)(temp_v1 + 0x3C)))
    {
        var_a1 += 1;
        var_a0 += 0x40;
        if (var_a1 < 0x64)
        {
            goto loop_6;
        }
    }
    else
    {
        var_t0 = 1;
    }
    var_a1_2 = 0;
    temp_a2 = (D_80122C02 << 6) + D_80122A08;
    var_a0_2 = g_menuLayoutBuffer;
loop_11:
    if ((*(u8 *)(var_a0_2 + 0x640) == 0) || (*(u32 *)(var_a0_2 + 0x678) != *(u32 *)(temp_a2 + 0x38)) || (*(u32 *)(var_a0_2 + 0x67C) != *(u32 *)(temp_a2 + 0x3C)))
    {
        var_a1_2 += 1;
        var_a0_2 += 0x40;
        if (var_a1_2 < 8)
        {
            goto loop_11;
        }
    }
    else
    {
        var_t0 = 1;
    }
    var_a1_3 = 0;
    temp_a2_2 = (D_80122C02 << 6) + D_80122A08;
    var_a0_3 = g_menuLayoutBuffer;
loop_16:
    if ((*(u8 *)(var_a0_3 + 0x3160) == 0) || (*(u32 *)(var_a0_3 + 0x3198) != *(u32 *)(temp_a2_2 + 0x38)) || (*(u32 *)(var_a0_3 + 0x319C) != *(u32 *)(temp_a2_2 + 0x3C)))
    {
        var_a1_3 += 1;
        var_a0_3 += 0x40;
        if (var_a1_3 < 4)
        {
            goto loop_16;
        }
    }
    else
    {
        var_t0 = 1;
    }
    if (var_t0 == 0)
    {
        temp_v0 = *(u32 *)((D_80122C02 << 6) + D_80122A08 + 0x14);
        temp_s2 = (temp_v0 >> 8) & 3;
        var_s3 = (temp_v0 >> 0xA) & 0x3F;
        if (temp_s2 == 1)
        {
            var_s3 += 0xB;
        }
    else if (temp_s2 == 2)
    {
            var_s3 += 0x17;
        }
        temp_v1_2 = (D_80122C02 << 6) + D_80122A08;
        temp_s5 = *(u16 *)(temp_v1_2 + 0x16) & 0x3F;
        if (temp_s2 == 0)
        {
            var_s1 = *(u16 *)(temp_v1_2 + 0x24);
        }
    else if (temp_s2 == 1)
    {
            var_s1 = *(u16 *)(temp_v1_2 + 0x24) + *(u16 *)(temp_v1_2 + 0x26) + *(u16 *)(temp_v1_2 + 0x28) + *(u16 *)(temp_v1_2 + 0x2A);
        }
    else
    {
            var_s1 = (u16) temp_v1_2[0x26];
        }
        temp_a2_3 = (D_80122C02 << 6) + D_80122A08;
        temp_a0 = *(u32 *)(temp_a2_3 + 0x18);
        temp_a1 = *(u32 *)(temp_a2_3 + 0x1C);
        var_s4 = ((temp_a0 & 0xF) + ((temp_a0 >> 4) & 0xF) + ((temp_a0 >> 8) & 0xF) + ((temp_a0 >> 0xC) & 0xF) + ((temp_a0 >> 0x10) & 0xF) + ((temp_a0 >> 0x14) & 0xF) + ((temp_a0 >> 0x18) & 0xF) + (temp_a0 >> 0x1C) + (temp_a1 & 0xF) + ((temp_a1 >> 4) & 0xF) + ((temp_a1 >> 8) & 0xF) + ((temp_a1 >> 0xC) & 0xF) + ((temp_a1 >> 0x10) & 0xF) + ((temp_a1 >> 0x14) & 0xF) + ((temp_a1 >> 0x18) & 0xF) + (temp_a1 >> 0x1C)) >= 0x29;
        if (temp_s2 == 2)
        {
            var_s4 = temp_a2_3[0x24];
        }
        var_s0 = *(s32 *)(temp_a2_3 + 0x34);
        func_800B2844(0, temp_a2_3, 0xFF, D_80122C02);
        *(u8 *)((u8 *)&D_80122C04 + 0) = temp_s2;
        temp_v0_2 = temp_s5 * 2;
        *(u8 *)((u8 *)&D_80122C04 + 1) = var_s3;
        func_800B2844(1, *(temp_v0_2 + D_800F0E98) + (*(temp_v0_2 + 1 + D_800F0E98) << 8) + D_800F0E98, 0xFF);
        *(u16 *)((u8 *)&D_80122C04 + 2) = var_s1;
        if (var_s4 != 0)
        {
            *(u16 *)((u8 *)&D_80122C04 + 2) = (u16) (var_s1 - 0x8000);
        }
        *(s32 *)((u8 *)&D_80122C04 + 4) = var_s0;
        var_v1 = 0;
        do

{
            var_s0 /= 0xA;
            var_v1 += 1;
        } while (var_s0 != 0);
        D_80122C0C = var_v1;
        return;
    }
    D_80122C03 = 2;
    func_800B2844(0, (D_80122C02 << 6) + D_80122A08, 0xFF, D_80122C02);
}


void func_800A8F8C(void *, void *);
extern u8 g_menuLayoutBuffer[];

/** @brief Remove spent records and compact the four-entry pending table. */
void func_800C8E2C(void)
{
    u8 *rec;
    s32 outer_i;
    s32 src_off;
    u8 *scan;
    u8 *base;
    u8 *dest_base;
    s32 inner_i;
    s32 off;
    u8 *a1;
    u8 *s0;
    s32 dest_off;

    outer_i = 0;
    rec = g_menuLayoutBuffer;
clear_records:
    {
        if (rec[0x3160] != 0 && *(s32 *)(rec + 0x3194) == 0)
        {
            rec[0x3160] = 0;
        }
        outer_i += 1;
        rec += 0x40;
    }
    if (outer_i < 4)
    {
        goto clear_records;
    }

    outer_i = 0;
    base = g_menuLayoutBuffer;
    dest_base = base + 0x3160;
    src_off = outer_i;
    scan = base;
compact_records:
    {
        if (scan[0x3160] == 0 && *(s32 *)(scan + 0x3194) == 0)
        {
            inner_i = outer_i + 1;
            if (inner_i < 4)
            {
                off = inner_i << 6;
                dest_off = src_off;
                a1 = (u8 *)((u32)off + (u32)dest_base);
                s0 = (u8 *)((u32)off + (u32)base);
            loop_10:
                inner_i += 1;
                if (s0[0x3160] != 0)
                {
                    do
                    {
                        func_800A8F8C((void *)((u32)dest_off + (u32)dest_base), a1);
                    } while (0);
                    s0[0x3160] = 0;
                    *(s32 *)(s0 + 0x3194) = 0;
                }
                else
                {
                    a1 += 0x40;
                    s0 += 0x40;
                    if (inner_i < 4)
                    {
                        goto loop_10;
                    }
                }
            }
        }
        src_off += 0x40;
        outer_i += 1;
        scan += 0x40;
    }
    if (outer_i < 4)
    {
        goto compact_records;
    }
}


typedef struct
{
    u8 pad0[0x3160];
    u8 unk3160;   /* 0x3160 */
    u8 pad3161[0x3194 - 0x3161];
    s32 unk3194;  /* 0x3194 */
} BigStruct;

extern u8 D_80122C02;
extern u8 D_80046138[];


extern u8 *func_800A9060(void);
extern /** @brief Remove spent records and compact the four-entry pending table. */
void func_800C8E2C(void);

/** @brief Copy the selected pending record out and compact the table. */
void func_800C8F4C(void)
{
    s32 idx;
    s32 offset;
    u8 *handle;
    BigStruct *rec;

    idx = D_80122C02;
    handle = func_800A9060();
    offset = idx << 6;
    func_800A8F8C(handle, &D_80046138[offset]);
    rec = (BigStruct *) (D_80046138 - 0x3160 + offset);
    rec->unk3160 = 0;
    rec->unk3194 = 0;
    func_800C8E2C();
}


extern u8 g_menuLayoutBuffer[];
extern u8 D_80122C02;
extern s32 D_80122C08;

extern /** @brief Remove spent records and compact the four-entry pending table. */
void func_800C8E2C(void);

/** @brief Restore the first pending result record and expose its result. */
void func_800C8FA8(void)
{
    s32 i;
    s32 result;
    u8 *base;
    u8 *arg;
    u8 *p;

    result = 0;
    D_80122C02 = 0xFF;
    i = 0;
    p = g_menuLayoutBuffer;
    arg = p + 0x3160;
    base = p;

loop:
    if (base[0x3160] == 0 && *(s32 *)&base[0x3194] != 0)
    {
        base[0x3160] = base[0x3180];
        func_800B2844(0, arg, 0xFF);
        result = *(s32 *)&base[0x3194];
        *(s32 *)&base[0x3194] = 0;
        D_80122C02 = i;
    }
    else
    {
        arg += 0x40;
        i++;
        base += 0x40;
        if (i < 4)
        {
            goto loop;
        }
    }

    D_80122C08 = result;
    if (result == 0)
    {
        func_800C8E2C();
    }
}

extern void func_800B2844(s32, u8 *, s32);

extern u8 D_80046138[], D_800F0E98[], g_menuLayoutBuffer[];
extern u8 D_80122C02, D_80122C03, D_80122C04, D_80122C06;
/**
 * @brief Count available records, detect duplicate identities, and prepare the selected record display.
 */
void func_800C905C(void)
{
    s32 layout_base;
    s32 second_search_base;
    s32 display_base;
    s32 metadata_index;
    s32 lookup_offset;
    s32 selected_offset;
    s32 work_index;
    s32 duplicate_found;
    s32 record_type;
    s32 work_value;
    s32 lookup_row;
    u32 packed_metadata;
    u8 *search_record;
    u8 *selected_record;
    u8 *lookup_base;
    u8 *record_base;
    s32 selected_index;

    selected_index = D_80122C02;
    work_value = 0;
    work_index = work_value;
    layout_base = (s32)g_menuLayoutBuffer + work_index * 0x40;
    do
    {
        if (((FieldMenuRecordLayout *)layout_base)->unk3160 != 0)
        {
            work_value += 1;
        }
        work_index += 1;
        layout_base = (s32)g_menuLayoutBuffer + work_index * 0x40;
    } while (work_index < 4);

    work_index = (s32)&D_80122C06;
    *(u8 *)work_index = work_value;
    work_value = (s32)g_menuLayoutBuffer;
    selected_record = (selected_index << 6) + g_menuLayoutBuffer;
    ((u8 *)work_index)[-3] = 0;
    if (((FieldMenuRecordLayout *)selected_record)->unk3160 == 0)
    {
        ((u8 *)work_index)[-3] = 1;
        return;
    }

    duplicate_found = 0;
    goto search_first;

found_first:
    duplicate_found = 1;
    goto search_second;

found_second:
    duplicate_found = 1;
    goto searches_done;

search_first:
    work_index = duplicate_found;
    search_record = selected_record;
first_loop:
    if (((FieldMenuRecordLayout *)work_value)->unkCE0 != 0 &&
        ((FieldMenuRecordLayout *)work_value)->unkD18 == ((FieldMenuRecordLayout *)search_record)->unk3198 &&
        ((FieldMenuRecordLayout *)work_value)->unkD1C == ((FieldMenuRecordLayout *)search_record)->unk319C)
    {
        goto found_first;
    }
    work_index++;
    work_value += 0x40;
    if (work_index < 100)
    {
        goto first_loop;
    }

search_second:
    work_index = 0;
    second_search_base = (s32)g_menuLayoutBuffer;
    search_record = (u8 *)((selected_index << 6) + second_search_base);
    work_value = second_search_base;
second_loop:
    if (((FieldMenuRecordLayout *)work_value)->unk640 != 0 &&
        ((FieldMenuRecordLayout *)work_value)->unk678 == ((FieldMenuRecordLayout *)search_record)->unk3198 &&
        ((FieldMenuRecordLayout *)work_value)->unk67C == ((FieldMenuRecordLayout *)search_record)->unk319C)
    {
        goto found_second;
    }
    work_index++;
    work_value += 0x40;
    if (work_index < 8)
    {
        goto second_loop;
    }

searches_done:
    if (duplicate_found == 0)
    {
        s32 row_offset = selected_index << 6;

        layout_base = (s32)g_menuLayoutBuffer;
        packed_metadata = ((FieldMenuRecordLayout *)(row_offset + layout_base))->packed.word;
        record_type = (packed_metadata >> 8) & 3;
        lookup_row = (packed_metadata >> 0xA) & 0x3F;
        if (record_type == 1)
        {
            lookup_row += 0xB;
            work_value = 0;
        }
        else if (record_type == 2)
        {
            lookup_row += 0x17;
            work_value = 0;
        }
        else
        {
            work_value = 0;
        }

        display_base = (s32)g_menuLayoutBuffer;
        selected_offset = selected_index << 6;
        metadata_index = ((FieldMenuRecordLayout *)(selected_offset + display_base))->packed.halves[1] & 0x3F;
        record_base = (u8 *)display_base + 0x3160;
        func_800B2844(work_value, selected_offset + record_base, 0xFF);
        D_80122C04 = record_type;
        lookup_base = D_800F0E98;
        lookup_offset = metadata_index * 2;
        (&D_80122C04)[1] = lookup_row;
        func_800B2844(1, lookup_base[lookup_offset] + (D_800F0E98[lookup_offset + 1] << 8) + D_800F0E98, 0xFF);
        return;
    }

    func_800B2844(0, (selected_index << 6) + D_80046138, 0xFF);
    D_80122C03 = 2;
}
