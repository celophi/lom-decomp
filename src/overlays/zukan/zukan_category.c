#include "zukan_category.h"

typedef struct
{
    s32 values[13];
} ZukanCategoryRangeTable;

typedef struct
{
    s32 values[7];
} ZukanGroupRangeTable;

typedef struct
{
    s16 values[1018];
} ZukanEntryValueTable;

typedef struct
{
    u16 values[719];
} ZukanDisplayOrderTable;

extern ZukanCategoryRangeTable g_zukan_category_ranges;
extern ZukanGroupRangeTable g_zukan_group_ranges;
extern ZukanEntryValueTable g_zukan_entry_values;
extern ZukanDisplayOrderTable g_zukan_display_order;

extern u32 D_8004300C[];
extern u32 D_80043038[];
extern u8 D_800432C8[];
extern s32 D_80043454;
extern u32 D_800460AC[];
extern u32 D_800460EC;
extern u32 D_800460FC;
extern u32 D_80046124;
extern u32 D_80046128;

/**
 * @brief Build and filter the ZUKAN entry list for the requested category.
 * @param category Category index to process.
 * @param entry_values Output array of entry resource values.
 * @param entry_indices Output array of entry indices.
 * @return Number of entries remaining after filtering.
 */
s32 zukan_build_category_entries(s32 category, s32* entry_values, s32* entry_indices)
{
    s32 count;
    s32 i;
    s32 j;
    s32 unused;
    s32 swap_index;
    s32 visible_count;
    s32 category_ranges[13];
    s32 group_ranges[7];
    s16 resource_values[1018];
    u16 display_order[719];

    __builtin_memcpy(category_ranges, category_ranges, 0);
    *(ZukanCategoryRangeTable*)category_ranges = g_zukan_category_ranges;
    __builtin_memcpy(group_ranges, group_ranges, 0);
    *(ZukanGroupRangeTable*)group_ranges = g_zukan_group_ranges;
    __builtin_memcpy(resource_values, resource_values, 0);
    *(ZukanEntryValueTable*)resource_values = g_zukan_entry_values;
    __builtin_memcpy(display_order, display_order, 0);
    *(ZukanDisplayOrderTable*)display_order = g_zukan_display_order;

    if (category == 8)
    {
        category_ranges[category + 1] = 0x240;
    }
    if (category == 7)
    {
        category_ranges[category + 1] = 0x1C6;
    }

    count = category_ranges[category + 1] - category_ranges[category];
    i = 0;
    while (i < count)
    {
        entry_indices[i] = i + category_ranges[category];
        i++;
    }

    if (category == 0xB)
    {
        i = count;
        while (i < count + 0x1A)
        {
            entry_indices[i] = i - ({ count - 0x1C6; });
            i++;
        }
        count += 0x1A;
    }

    visible_count = 0;
    i = 0;
    while (display_order[i] != 0x400)
    {
        j = visible_count;
        while (j < count)
        {
            if (display_order[i] == entry_indices[j])
            {
                swap_index = entry_indices[visible_count];
                entry_indices[visible_count] = entry_indices[j];
                entry_indices[j] = swap_index;
                entry_values[visible_count] = resource_values[entry_indices[visible_count]];
                visible_count++;
            }
            j++;
        }
        i++;
    }
    count = visible_count;

    if (category == 1)
    {
        i = 0;
        while (i < 0x21)
        {
            if (((D_800432C8[i * 12] >> 1) & 1) & 0xFF)
            {
                D_800460AC[(i + 0x200) / 32] = D_800460AC[(i + 0x200) / 32] | (1 << ((i + 0x200) % 32));
            }
            i++;
        }
        if (D_80043454 & 4)
        {
            D_800460EC |= 0x01000000;
        }
        i = 0;
        while (i < count)
        {
            if (!(D_800460AC[(entry_indices[i] + 0x1FF) / 32] & (1 << ((entry_indices[i] + 0x1FF) % 32))))
            {
                entry_values[i] = 0;
                entry_indices[i] = 0;
            }
            i++;
        }
    }
    else if (category == 2)
    {
        i = 0;
        while (i < 0x21)
        {
            if ((D_800432C8[i * 12] & 1) & 0xFF)
            {
                D_800460AC[(i + 0x240) / 32] = D_800460AC[(i + 0x240) / 32] | (1 << ((i + 0x240) % 32));
            }
            if (((D_800432C8[i * 12] >> 1) & 1) & 0xFF)
            {
                D_800460AC[(i + 0x280) / 32] = D_800460AC[(i + 0x280) / 32] | (1 << ((i + 0x280) % 32));
            }
            i++;
        }
        if (D_80043454 & 4)
        {
            D_800460FC |= 0x01000000;
        }
        i = 0;
        while (i < count)
        {
            if (!(D_800460AC[(entry_indices[i] + 0x1FF) / 32] & (1 << ((entry_indices[i] + 0x1FF) % 32))))
            {
                if (!(D_800460AC[(entry_indices[i] + 0x23F) / 32] & (1 << ((entry_indices[i] + 0x23F) % 32))))
                {
                    entry_values[i] = 0;
                    entry_indices[i] = 0;
                }
            }
            if (D_800460AC[(entry_indices[i] + 0x23F) / 32] & (1 << ((entry_indices[i] + 0x23F) % 32)))
            {
                entry_values[i] = resource_values[entry_indices[i] + 0x1FF];
            }
            i++;
        }
    }
    else if (category == 3)
    {
    }
    else if (category == 4)
    {
    }
    else if (category == 5)
    {
    }
    else if (category == 6)
    {
        i = 0;
        while (i < count)
        {
            if (entry_indices[i] != 0x135)
            {
                if (entry_indices[i] < 0x136)
                {
                    if (!(D_800460AC[(entry_indices[i] - 0xE6) / 32] & (1 << ((entry_indices[i] - 0xE6) % 32))))
                    {
                        entry_values[i] = 0;
                        entry_indices[i] = 0;
                    }
                }
                else
                {
                    if (!(D_800460AC[(entry_indices[i] - 0xA) / 32] & (1 << ((entry_indices[i] - 0xA) % 32))))
                    {
                        entry_values[i] = 0;
                        entry_indices[i] = 0;
                    }
                }
            }
            i++;
        }
    }
    else if (category == 7)
    {
        i = 0;
        while (i < count)
        {
            if (!(D_800460AC[(entry_indices[i] - 0xFA) / 32] & (1 << ((entry_indices[i] - 0xFA) % 32))))
            {
                entry_values[i] = 0;
                entry_indices[i] = 0;
            }
            i++;
        }
    }
    else if (category == 8)
    {
        j = 0;
        while (j < 6)
        {
            if (!(D_800460AC[(j + 0x122) / 32] & (1 << ((j + 0x122) % 32))))
            {
                i = 0;
                while (i < count)
                {
                    if ((entry_indices[i] >= group_ranges[j] + 0x1E0) && (entry_indices[i] < group_ranges[j + 1] + 0x1E0))
                    {
                        entry_values[i] = 0;
                        entry_indices[i] = 0;
                    }
                    i++;
                }
            }
            j++;
        }
    }
    else if (category == 9)
    {
        i = 0;
        while (i < count)
        {
            if (!(D_800460AC[(entry_indices[i] - 0x1B8) / 32] & (1 << ((entry_indices[i] - 0x1B8) % 32))))
            {
                entry_values[i] = 0;
                entry_indices[i] = 0;
            }
            i++;
        }
    }
    else if (category == 0xA)
    {
    }
    else if (category == 0xB)
    {
        i = 0;
        while (i < 0x60)
        {
            if ((D_80043038[i / 32] >> (i % 32)) & 1)
            {
                if (i == 8)
                {
                    D_80046124 |= 0x100;
                }
                if (i == 0xA)
                {
                    D_80046124 |= 0x200;
                }
                if ((i >= 0x2F) && (i < 0x46))
                {
                    D_800460AC[(i + 0x39B) / 32] = D_800460AC[(i + 0x39B) / 32] | (1 << ((i + 0x39B) % 32));
                }
                if (i == 0x4F)
                {
                    D_80046128 |= 2;
                }
            }
            i++;
        }

        i = 0;
        while (i < 0x160)
        {
            if ((i % 32) < 0x18)
            {
                if ((D_8004300C[i / 32] >> (i % 32)) & 1)
                {
                    j = i - ((i / 32) * 8);
                    D_800460AC[(j + 0x2C0) / 32] = D_800460AC[(j + 0x2C0) / 32] | (1 << ((j + 0x2C0) % 32));
                }
            }
            i++;
        }

        i = 0;
        while (i < count)
        {
            if ((entry_indices[i] >= 0x1C6) && (entry_indices[i] < 0x2EE))
            {
                if (!(D_800460AC[(entry_indices[i] + 0x202) / 32] & (1 << ((entry_indices[i] + 0x202) % 32))))
                {
                    entry_values[i] = 0;
                    entry_indices[i] = 0;
                }
            }
            else
            {
                if (!(D_800460AC[(entry_indices[i] - 0x2E) / 32] & (1 << ((entry_indices[i] - 0x2E) % 32))))
                {
                    entry_values[i] = 0;
                    entry_indices[i] = 0;
                }
            }
            i++;
        }
    }

    return count;
}
