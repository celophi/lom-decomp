#include "zukan_category.h"
#include "saved_game.h"

#define ZUKAN_CATEGORY_RANGE_COUNT 13
#define ZUKAN_HISTORY_GROUP_COUNT 6
#define ZUKAN_HISTORY_GROUP_RANGE_COUNT (ZUKAN_HISTORY_GROUP_COUNT + 1)
#define ZUKAN_ENTRY_VALUE_COUNT 1018
#define ZUKAN_DISPLAY_ORDER_COUNT 719
#define ZUKAN_DISPLAY_ORDER_END 0x400
#define ZUKAN_LAND_UNLOCK_COUNT 33
#define ZUKAN_CHARACTER_END 454
#define ZUKAN_WORLD_HISTORY_START 480
#define ZUKAN_WORLD_HISTORY_END 576
/** @brief Unlock bit of the first World History group; the other groups follow it. */
#define ZUKAN_HISTORY_GROUP_UNLOCK_BIT 0x122
/** @brief Techniques per weapon category in the saved technique bits. */
#define ZUKAN_TECHNIQUES_PER_WEAPON 24
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

/** @brief Nonzero when encyclopedia entry bit @p bit is set in the saved game. */
#define ZUKAN_ENTRY_UNLOCKED(bit) (g_saved_encyclopedia_bits[(bit) / 32] & (1 << ((bit) % 32)))

/** @brief Set encyclopedia entry bit @p bit in the saved game. */
#define ZUKAN_UNLOCK_ENTRY(bit) (g_saved_encyclopedia_bits[(bit) / 32] = g_saved_encyclopedia_bits[(bit) / 32] | (1 << ((bit) % 32)))

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

    if (category == ZUKAN_CATEGORY_WORLD_HISTORY)
    {
        category_ranges[category + 1] = ZUKAN_WORLD_HISTORY_END;
    }
    if (category == ZUKAN_CATEGORY_CHARACTERS)
    {
        category_ranges[category + 1] = ZUKAN_CHARACTER_END;
    }

    count = category_ranges[category + 1] - category_ranges[category];
    for (i = 0; i < count; i++)
    {
        entry_indices[i] = i + category_ranges[category];
    }

    if (category == ZUKAN_CATEGORY_TECHNIQUES)
    {
        for (i = count; i < count + ZUKAN_EXTRA_TECHNIQUE_COUNT; i++)
        {
            entry_indices[i] = i + ZUKAN_CHARACTER_END - count;
        }
        count += ZUKAN_EXTRA_TECHNIQUE_COUNT;
    }

    visible_count = 0;
    for (i = 0; display_order[i] != ZUKAN_DISPLAY_ORDER_END; i++)
    {
        for (j = visible_count; j < count; j++)
        {
            if (display_order[i] == entry_indices[j])
            {
                swap_index = entry_indices[visible_count];
                entry_indices[visible_count] = entry_indices[j];
                entry_indices[j] = swap_index;
                entry_values[visible_count] = resource_values[entry_indices[visible_count]];
                visible_count++;
            }
        }
    }
    count = visible_count;

    if (category == ZUKAN_CATEGORY_LANDS)
    {
        for (i = 0; i < ZUKAN_LAND_UNLOCK_COUNT; i++)
        {
            if ((g_saved_lands[i].record.flags >> 1) & 1)
            {
                ZUKAN_UNLOCK_ENTRY(i + 0x200);
            }
        }
        if (g_saved_lands[33].word & FIELD_LAND_FLAG_04)
        {
            ZUKAN_UNLOCK_ENTRY(0x218);
        }
        for (i = 0; i < count; i++)
        {
            if (!ZUKAN_ENTRY_UNLOCKED(entry_indices[i] + 0x1FF))
            {
                entry_values[i] = 0;
                entry_indices[i] = 0;
            }
        }
    }
    else if (category == ZUKAN_CATEGORY_ARTIFACTS)
    {
        for (i = 0; i < ZUKAN_LAND_UNLOCK_COUNT; i++)
        {
            if (g_saved_lands[i].record.flags & 1)
            {
                ZUKAN_UNLOCK_ENTRY(i + 0x240);
            }
            if ((g_saved_lands[i].record.flags >> 1) & 1)
            {
                ZUKAN_UNLOCK_ENTRY(i + 0x280);
            }
        }
        if (g_saved_lands[33].word & FIELD_LAND_FLAG_04)
        {
            ZUKAN_UNLOCK_ENTRY(0x298);
        }
        for (i = 0; i < count; i++)
        {
            if (!ZUKAN_ENTRY_UNLOCKED(entry_indices[i] + 0x1FF))
            {
                if (!ZUKAN_ENTRY_UNLOCKED(entry_indices[i] + 0x23F))
                {
                    entry_values[i] = 0;
                    entry_indices[i] = 0;
                }
            }
            if (ZUKAN_ENTRY_UNLOCKED(entry_indices[i] + 0x23F))
            {
                entry_values[i] = resource_values[entry_indices[i] + 0x1FF];
            }
        }
    }
    else if (category == ZUKAN_CATEGORY_EQUIPMENT)
    {
    }
    else if (category == ZUKAN_CATEGORY_ITEMS)
    {
    }
    else if (category == ZUKAN_CATEGORY_PRODUCE)
    {
    }
    else if (category == ZUKAN_CATEGORY_MONSTERS)
    {
        for (i = 0; i < count; i++)
        {
            if (entry_indices[i] != 0x135)
            {
                if (entry_indices[i] < 0x136)
                {
                    if (!ZUKAN_ENTRY_UNLOCKED(entry_indices[i] - 0xE6))
                    {
                        entry_values[i] = 0;
                        entry_indices[i] = 0;
                    }
                }
                else
                {
                    if (!ZUKAN_ENTRY_UNLOCKED(entry_indices[i] - 0xA))
                    {
                        entry_values[i] = 0;
                        entry_indices[i] = 0;
                    }
                }
            }
        }
    }
    else if (category == ZUKAN_CATEGORY_CHARACTERS)
    {
        for (i = 0; i < count; i++)
        {
            if (!ZUKAN_ENTRY_UNLOCKED(entry_indices[i] - 0xFA))
            {
                entry_values[i] = 0;
                entry_indices[i] = 0;
            }
        }
    }
    else if (category == ZUKAN_CATEGORY_WORLD_HISTORY)
    {
        for (j = 0; j < ZUKAN_HISTORY_GROUP_COUNT; j++)
        {
            if (!ZUKAN_ENTRY_UNLOCKED(j + ZUKAN_HISTORY_GROUP_UNLOCK_BIT))
            {
                for (i = 0; i < count; i++)
                {
                    if ((entry_indices[i] >= group_ranges[j] + ZUKAN_WORLD_HISTORY_START) &&
                        (entry_indices[i] < group_ranges[j + 1] + ZUKAN_WORLD_HISTORY_START))
                    {
                        entry_values[i] = 0;
                        entry_indices[i] = 0;
                    }
                }
            }
        }
    }
    else if (category == ZUKAN_CATEGORY_CACTUS_DIARIES)
    {
        for (i = 0; i < count; i++)
        {
            if (!ZUKAN_ENTRY_UNLOCKED(entry_indices[i] - 0x1B8))
            {
                entry_values[i] = 0;
                entry_indices[i] = 0;
            }
        }
    }
    else if (category == ZUKAN_CATEGORY_GOLEMOLOGY)
    {
    }
    else if (category == ZUKAN_CATEGORY_TECHNIQUES)
    {
        for (i = 0; i < FIELD_ABILITY_WORD_COUNT * 32; i++)
        {
            if ((g_saved_ability_bits[i / 32] >> (i % 32)) & 1)
            {
                if (i == 8)
                {
                    ZUKAN_UNLOCK_ENTRY(0x3C8);
                }
                if (i == 0xA)
                {
                    ZUKAN_UNLOCK_ENTRY(0x3C9);
                }
                if ((i >= 0x2F) && (i < 0x46))
                {
                    ZUKAN_UNLOCK_ENTRY(i + 0x39B);
                }
                if (i == 0x4F)
                {
                    ZUKAN_UNLOCK_ENTRY(0x3E1);
                }
            }
        }

        for (i = 0; i < FIELD_WEAPON_CATEGORY_COUNT * 32; i++)
        {
            if ((i % 32) < ZUKAN_TECHNIQUES_PER_WEAPON)
            {
                if ((g_saved_technique_bits[i / 32] >> (i % 32)) & 1)
                {
                    j = i - ((i / 32) * 8);
                    ZUKAN_UNLOCK_ENTRY(j + 0x2C0);
                }
            }
        }

        for (i = 0; i < count; i++)
        {
            if ((entry_indices[i] >= 0x1C6) && (entry_indices[i] < 0x2EE))
            {
                if (!ZUKAN_ENTRY_UNLOCKED(entry_indices[i] + 0x202))
                {
                    entry_values[i] = 0;
                    entry_indices[i] = 0;
                }
            }
            else
            {
                if (!ZUKAN_ENTRY_UNLOCKED(entry_indices[i] - 0x2E))
                {
                    entry_values[i] = 0;
                    entry_indices[i] = 0;
                }
            }
        }
    }

    return count;
}
