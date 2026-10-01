#include "zukan_category.h"
#include "saved_game.h"

#define ZUKAN_CATEGORY_RANGE_COUNT 13
#define ZUKAN_HISTORY_GROUP_RANGE_COUNT 7
#define ZUKAN_ENTRY_VALUE_COUNT 1018
#define ZUKAN_DISPLAY_ORDER_COUNT 719
#define ZUKAN_DISPLAY_ORDER_END 0x400
#define ZUKAN_LAND_UNLOCK_COUNT 33
#define ZUKAN_CHARACTER_END 454
#define ZUKAN_WORLD_HISTORY_END 576
#define ZUKAN_EXTRA_TECHNIQUE_COUNT 26

/** @brief Start index of each category and the final end index. */
typedef struct
{
    s32 values[ZUKAN_CATEGORY_RANGE_COUNT];
} ZukanCategoryRangeTable;

/** @brief Start index of each World History group and the final end index. */
typedef struct
{
    s32 values[ZUKAN_HISTORY_GROUP_RANGE_COUNT];
} ZukanGroupRangeTable;

/** @brief CD resource values for every encyclopedia entry. */
typedef struct
{
    s16 values[ZUKAN_ENTRY_VALUE_COUNT];
} ZukanEntryValueTable;

/** @brief Entry display order followed by its terminal sentinel. */
typedef struct
{
    u16 values[ZUKAN_DISPLAY_ORDER_COUNT];
} ZukanDisplayOrderTable;

extern ZukanCategoryRangeTable g_zukan_category_ranges;
extern ZukanGroupRangeTable g_zukan_group_ranges;
extern ZukanEntryValueTable g_zukan_entry_values;
extern ZukanDisplayOrderTable g_zukan_display_order;

/* Named views of fields in SavedGameLayout at their fixed game-state addresses. */
extern u32 g_saved_technique_bits[FIELD_WEAPON_CATEGORY_COUNT];
extern u32 g_saved_ability_bits[FIELD_ABILITY_WORD_COUNT];
extern FieldLandWords g_saved_lands[FIELD_LAND_COUNT];
extern u32 g_saved_encyclopedia_bits[FIELD_ENCYCLOPEDIA_BIT_WORDS];

/**
 * @brief Build and filter the ZUKAN entry list for the requested category.
 * @param category Category index to process.
 * @param entry_values Output array of entry resource values.
 * @param entry_indices Output array of entry indices.
 * @return Number of list rows; locked entries remain as empty rows.
 * @note Unlock bits are read from the saved game's encyclopedia, land,
 *       ability, and technique records.
 */
s32 zukan_build_category_entries(s32 category, s32* entry_values, s32* entry_indices)
{
    s32 count;
    s32 i;
    s32 j;
    s32 unused;
    s32 swap_index;
    s32 visible_count;
    s32 category_ranges[ZUKAN_CATEGORY_RANGE_COUNT];
    s32 group_ranges[ZUKAN_HISTORY_GROUP_RANGE_COUNT];
    s16 resource_values[ZUKAN_ENTRY_VALUE_COUNT];
    u16 display_order[ZUKAN_DISPLAY_ORDER_COUNT];

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
        category_ranges[category + 1] = ZUKAN_WORLD_HISTORY_END;
    }
    if (category == 7)
    {
        category_ranges[category + 1] = ZUKAN_CHARACTER_END;
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
        while (i < count + ZUKAN_EXTRA_TECHNIQUE_COUNT)
        {
            entry_indices[i] = i - ({ count - ZUKAN_CHARACTER_END; });
            i++;
        }
        count += ZUKAN_EXTRA_TECHNIQUE_COUNT;
    }

    visible_count = 0;
    i = 0;
    while (display_order[i] != ZUKAN_DISPLAY_ORDER_END)
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
        while (i < ZUKAN_LAND_UNLOCK_COUNT)
        {
            if (((g_saved_lands[i].record.flags >> 1) & 1) & 0xFF)
            {
                g_saved_encyclopedia_bits[(i + 0x200) / 32] = g_saved_encyclopedia_bits[(i + 0x200) / 32] | (1 << ((i + 0x200) % 32));
            }
            i++;
        }
        if (g_saved_lands[33].word & FIELD_LAND_FLAG_04)
        {
            g_saved_encyclopedia_bits[16] |= 0x01000000;
        }
        i = 0;
        while (i < count)
        {
            if (!(g_saved_encyclopedia_bits[(entry_indices[i] + 0x1FF) / 32] & (1 << ((entry_indices[i] + 0x1FF) % 32))))
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
        while (i < ZUKAN_LAND_UNLOCK_COUNT)
        {
            if ((g_saved_lands[i].record.flags & 1) & 0xFF)
            {
                g_saved_encyclopedia_bits[(i + 0x240) / 32] = g_saved_encyclopedia_bits[(i + 0x240) / 32] | (1 << ((i + 0x240) % 32));
            }
            if (((g_saved_lands[i].record.flags >> 1) & 1) & 0xFF)
            {
                g_saved_encyclopedia_bits[(i + 0x280) / 32] = g_saved_encyclopedia_bits[(i + 0x280) / 32] | (1 << ((i + 0x280) % 32));
            }
            i++;
        }
        if (g_saved_lands[33].word & FIELD_LAND_FLAG_04)
        {
            g_saved_encyclopedia_bits[20] |= 0x01000000;
        }
        i = 0;
        while (i < count)
        {
            if (!(g_saved_encyclopedia_bits[(entry_indices[i] + 0x1FF) / 32] & (1 << ((entry_indices[i] + 0x1FF) % 32))))
            {
                if (!(g_saved_encyclopedia_bits[(entry_indices[i] + 0x23F) / 32] & (1 << ((entry_indices[i] + 0x23F) % 32))))
                {
                    entry_values[i] = 0;
                    entry_indices[i] = 0;
                }
            }
            if (g_saved_encyclopedia_bits[(entry_indices[i] + 0x23F) / 32] & (1 << ((entry_indices[i] + 0x23F) % 32)))
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
                    if (!(g_saved_encyclopedia_bits[(entry_indices[i] - 0xE6) / 32] & (1 << ((entry_indices[i] - 0xE6) % 32))))
                    {
                        entry_values[i] = 0;
                        entry_indices[i] = 0;
                    }
                }
                else
                {
                    if (!(g_saved_encyclopedia_bits[(entry_indices[i] - 0xA) / 32] & (1 << ((entry_indices[i] - 0xA) % 32))))
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
            if (!(g_saved_encyclopedia_bits[(entry_indices[i] - 0xFA) / 32] & (1 << ((entry_indices[i] - 0xFA) % 32))))
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
            if (!(g_saved_encyclopedia_bits[(j + 0x122) / 32] & (1 << ((j + 0x122) % 32))))
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
            if (!(g_saved_encyclopedia_bits[(entry_indices[i] - 0x1B8) / 32] & (1 << ((entry_indices[i] - 0x1B8) % 32))))
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
            if ((g_saved_ability_bits[i / 32] >> (i % 32)) & 1)
            {
                if (i == 8)
                {
                    g_saved_encyclopedia_bits[30] |= 0x100;
                }
                if (i == 0xA)
                {
                    g_saved_encyclopedia_bits[30] |= 0x200;
                }
                if ((i >= 0x2F) && (i < 0x46))
                {
                    g_saved_encyclopedia_bits[(i + 0x39B) / 32] = g_saved_encyclopedia_bits[(i + 0x39B) / 32] | (1 << ((i + 0x39B) % 32));
                }
                if (i == 0x4F)
                {
                    g_saved_encyclopedia_bits[31] |= 2;
                }
            }
            i++;
        }

        i = 0;
        while (i < 0x160)
        {
            if ((i % 32) < 0x18)
            {
                if ((g_saved_technique_bits[i / 32] >> (i % 32)) & 1)
                {
                    j = i - ((i / 32) * 8);
                    g_saved_encyclopedia_bits[(j + 0x2C0) / 32] = g_saved_encyclopedia_bits[(j + 0x2C0) / 32] | (1 << ((j + 0x2C0) % 32));
                }
            }
            i++;
        }

        i = 0;
        while (i < count)
        {
            if ((entry_indices[i] >= 0x1C6) && (entry_indices[i] < 0x2EE))
            {
                if (!(g_saved_encyclopedia_bits[(entry_indices[i] + 0x202) / 32] & (1 << ((entry_indices[i] + 0x202) % 32))))
                {
                    entry_values[i] = 0;
                    entry_indices[i] = 0;
                }
            }
            else
            {
                if (!(g_saved_encyclopedia_bits[(entry_indices[i] - 0x2E) / 32] & (1 << ((entry_indices[i] - 0x2E) % 32))))
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
