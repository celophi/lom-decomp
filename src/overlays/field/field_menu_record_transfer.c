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

/** @brief Shared 64-byte record used by the menu record transfer path. */
typedef struct FieldSharedRecord
{
    u8 unk0;
    u8 pad1[0x13];
    union
    {
        u32 word;
        u16 halves[2];
    } packed_metadata;
    u32 unk18;
    u32 unk1C;
    u8 pad20[4];
    u16 unk24;
    u16 unk26;
    u16 unk28;
    u16 unk2A;
    u8 pad2C[8];
    u32 unk34;
    u32 unk38;
    u32 unk3C;
} FieldSharedRecord;

extern u8 g_menuLayoutBuffer[], D_80122A08[], D_800F0E98[];
extern u8 D_80122C02, D_80122C03, D_80122C04, D_80122C0C;
void func_800B2844();

/**
 * @brief Validate the selected shared record and prepare its menu display state.
 */
void func_800C8A2C(void)
{
    s32 work_index;
    s32 duplicate_found;
    s32 selected_index;
    u8 *initial_record;
    u8 *selected_record;
    u8 *detail_record;
    u8 *search_record;
    u8 *scan;
    u32 packed_metadata;
    s32 record_type;
    s32 lookup_row;
    s32 metadata_index;
    s32 display_value;
    u32 low_word;
    u32 high_word;
    s32 nibble_sum;
    s32 special_flag;
    u32 amount;
    s32 digits;
    u8 *lookup_base;
    s32 lookup_offset;
    u8 *display;
    u8 *status;
    u8 *initial_base;

    status = &D_80122C02;
    status[1] = 0;
    selected_index = D_80122C02;
    initial_base = D_80122A08;
    initial_record = (selected_index << 6) + initial_base;
    if (initial_record[0] == 0)
    {
        status[1] = 1;
        return;
    }

    duplicate_found = 0;

    goto search_first;

found_first:
    duplicate_found = 1;
    goto search_second;

found_second:
    duplicate_found = 1;
    goto search_third;

found_third:
    duplicate_found = 1;
    goto searches_done;

search_first:
    work_index = duplicate_found;
    search_record = initial_record;
    scan = g_menuLayoutBuffer;
first_loop:
    if (((FieldMenuRecordLayout *)scan)->unkCE0 != 0 &&
        ((FieldMenuRecordLayout *)scan)->unkD18 == *(u32 *)(search_record + 0x38) &&
        ((FieldMenuRecordLayout *)scan)->unkD1C == *(u32 *)(search_record + 0x3C))
    {
        goto found_first;
    }
    work_index++;
    scan += 0x40;
    if (work_index < 100)
    {
        goto first_loop;
    }

search_second:
    work_index = 0;
    {
        u8 *record_base;
        record_base = D_80122A08;
        search_record = record_base + (selected_index << 6);
    }
    scan = g_menuLayoutBuffer;
second_loop:
    if (((FieldMenuRecordLayout *)scan)->unk640 != 0 &&
        ((FieldMenuRecordLayout *)scan)->unk678 == *(u32 *)(search_record + 0x38) &&
        ((FieldMenuRecordLayout *)scan)->unk67C == *(u32 *)(search_record + 0x3C))
    {
        goto found_second;
    }
    work_index++;
    scan += 0x40;
    if (work_index < 8)
    {
        goto second_loop;
    }

search_third:
    work_index = 0;
    {
        u8 *record_base;
        record_base = D_80122A08;
        search_record = record_base + (selected_index << 6);
    }
    scan = g_menuLayoutBuffer;
third_loop:
    if (((FieldMenuRecordLayout *)scan)->unk3160 != 0 &&
        ((FieldMenuRecordLayout *)scan)->unk3198 == *(u32 *)(search_record + 0x38) &&
        ((FieldMenuRecordLayout *)scan)->unk319C == *(u32 *)(search_record + 0x3C))
    {
        goto found_third;
    }
    work_index++;
    scan += 0x40;
    if (work_index < 4)
    {
        goto third_loop;
    }

searches_done:
    if (duplicate_found == 0)
    {
        packed_metadata = ((FieldSharedRecord *)D_80122A08)[selected_index].packed_metadata.word;
        record_type = (packed_metadata >> 8) & 3;
        lookup_row = (packed_metadata >> 10) & 0x3F;
        if (record_type == 1)
        {
            lookup_row += 0xB;
        }
        else if (record_type == 2)
        {
            lookup_row += 0x17;
        }

        {
            u8 *detail_base;
            detail_base = D_80122A08;
            detail_record = (selected_index << 6) + detail_base;
        }
        metadata_index = *(u16 *)(detail_record + 0x16) & 0x3F;
        if (record_type == 0)
        {
            do
            {
                display_value = *(u16 *)(detail_record + 0x24);
            } while (0);
        }
        else if (record_type == 1)
        {
            do
            {
                display_value = *(u16 *)(detail_record + 0x24);
                display_value += *(u16 *)(detail_record + 0x26);
                display_value += *(u16 *)(detail_record + 0x28);
                display_value += *(u16 *)(detail_record + 0x2A);
            } while (0);
        }
        else
        {
            do
            {
                display_value = detail_record[0x26];
            } while (0);
        }

        {
            u8 *final_base;
            final_base = D_80122A08;
            selected_record = (selected_index << 6) + final_base;
        }
        low_word = *(u32 *)(selected_record + 0x18);
        high_word = *(u32 *)(selected_record + 0x1C);
        nibble_sum = (low_word & 0xF) + ((low_word >> 4) & 0xF) + ((low_word >> 8) & 0xF) + ((low_word >> 12) & 0xF) +
                     ((low_word >> 16) & 0xF) + ((low_word >> 20) & 0xF) + ((low_word >> 24) & 0xF) + (low_word >> 28) +
                     (high_word & 0xF) + ((high_word >> 4) & 0xF) + ((high_word >> 8) & 0xF) + ((high_word >> 12) & 0xF) +
                     ((high_word >> 16) & 0xF) + ((high_word >> 20) & 0xF) + ((high_word >> 24) & 0xF) + (high_word >> 28);
        special_flag = nibble_sum >= 0x29;
        if (record_type == 2)
        {
            special_flag = selected_record[0x24];
        }

        amount = *(u32 *)(selected_record + 0x34);
        func_800B2844(0, selected_record, 0xFF);
        D_80122C04 = record_type;
        display = &D_80122C04;
        lookup_base = D_800F0E98;
        lookup_offset = metadata_index * 2;
        display[1] = lookup_row;
        func_800B2844(1, lookup_base[lookup_offset] + (D_800F0E98[lookup_offset + 1] << 8) + D_800F0E98, 0xFF);
        *(u16 *)(display + 2) = display_value;
        if (special_flag != 0)
        {
            *(u16 *)(display + 2) = display_value - 0x8000;
        }
        *(u32 *)(display + 4) = amount;
        digits = 0;
        do
        {
            do
            {
                amount /= 10;
            } while (0);
            digits++;
        } while (amount != 0);
        D_80122C0C = digits;
        return;
    }

    D_80122C03 = 2;
    func_800B2844(0, (selected_index << 6) + D_80122A08, 0xFF);
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
