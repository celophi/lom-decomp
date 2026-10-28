#include "common.h"

#include "common/saved_game.h"
#include "internal/zukan_category.h"
#include "overlays/field/field_actor_records.h"

#define ZUKAN_HISTORY_GROUP_COUNT 6
#define ZUKAN_DISPLAY_ORDER_END 0x400
/* Entry indices. The Techniques list also shows the ability entries from ZUKAN_CHARACTER_END, and
 * the artifacts' second pages follow the World History entries the list shows. */
#define ZUKAN_FIRST_LAND_ENTRY 1
#define ZUKAN_FIRST_ARTIFACT_ENTRY 65
#define ZUKAN_FIRST_MONSTER_ENTRY 230
#define ZUKAN_ALWAYS_SHOWN_MONSTER_ENTRY 309
#define ZUKAN_FIRST_LATE_MONSTER_ENTRY 310
#define ZUKAN_FIRST_CHARACTER_ENTRY 350
#define ZUKAN_CHARACTER_END 454
#define ZUKAN_WORLD_HISTORY_START 480
#define ZUKAN_WORLD_HISTORY_END 576
#define ZUKAN_FIRST_ARTIFACT_ALT_ENTRY ZUKAN_WORLD_HISTORY_END
#define ZUKAN_FIRST_CACTUS_DIARY_ENTRY 640
#define ZUKAN_FIRST_TECHNIQUE_ENTRY 750
#define ZUKAN_EXTRA_TECHNIQUE_COUNT 26

/* Encyclopedia bits of the first entry of each unlock range; the lands and artifacts use the
 * FIELD_ENCYCLOPEDIA_* ranges. */
#define ZUKAN_MONSTER_BIT 0
#define ZUKAN_CHARACTER_BIT 100
#define ZUKAN_CACTUS_DIARY_BIT 200
#define ZUKAN_HISTORY_GROUP_UNLOCK_BIT 290
#define ZUKAN_LATE_MONSTER_BIT 300
#define ZUKAN_TECHNIQUE_BIT 0x2C0
#define ZUKAN_ABILITY_BIT 0x3C8

/* Bits of the ability entries: Jump, Defend, then Lunge to Slide in ability order, then Flip Kick. */
#define ZUKAN_JUMP_BIT ZUKAN_ABILITY_BIT
#define ZUKAN_DEFEND_BIT (ZUKAN_ABILITY_BIT + 1)
#define ZUKAN_LUNGE_BIT (ZUKAN_ABILITY_BIT + 2)
#define ZUKAN_FLIP_KICK_BIT (ZUKAN_LUNGE_BIT + FIELD_ABILITY_SLIDE + 1 - FIELD_ABILITY_LUNGE)

/* Named views of fields in SavedGameLayout at their fixed game-state addresses. */
extern u32 g_saved_technique_bits[FIELD_WEAPON_CATEGORY_COUNT];
extern u32 g_saved_ability_bits[FIELD_ABILITY_WORD_COUNT];
extern FieldLandWords g_saved_lands[FIELD_LAND_COUNT];

/** @brief Nonzero when encyclopedia entry bit @p bit is set in the saved game. */
#define ZUKAN_ENTRY_UNLOCKED(bit) SAVED_BIT_IS_SET(g_saved_encyclopedia_bits, bit)

/** @brief Set encyclopedia entry bit @p bit in the saved game. */
#define ZUKAN_UNLOCK_ENTRY(bit) SAVED_BIT_SET(g_saved_encyclopedia_bits, bit)

/** @brief Encyclopedia bit of @p index in a range whose index @p first_index has bit @p first_bit. */
#define ZUKAN_RANGE_BIT(index, first_index, first_bit) ((index) + ((first_bit) - (first_index)))

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
    s32 category_ranges[] = {
#include "zukan_category/category_ranges.inc"
    };
    s32 group_ranges[] = {
#include "zukan_category/group_ranges.inc"
    };
    s16 resource_values[] = {
#include "zukan_category/resource_values.inc"
    };
    u16 display_order[] = {
#include "zukan_category/display_order.inc"
    };

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
        for (i = 0; i < FIELD_ENCYCLOPEDIA_LAND_COUNT; i++)
        {
            if ((g_saved_lands[i].record.flags >> 1) & 1)
            {
                ZUKAN_UNLOCK_ENTRY(i + FIELD_ENCYCLOPEDIA_LAND_BIT);
            }
        }

        if (g_saved_lands[FIELD_LAND_LUCEMIA_2].word & FIELD_LAND_FLAG_04)
        {
            ZUKAN_UNLOCK_ENTRY(FIELD_ENCYCLOPEDIA_LAND_BIT + FIELD_LAND_LUCEMIA);
        }

        for (i = 0; i < count; i++)
        {
            if (!ZUKAN_ENTRY_UNLOCKED(ZUKAN_RANGE_BIT(entry_indices[i], ZUKAN_FIRST_LAND_ENTRY, FIELD_ENCYCLOPEDIA_LAND_BIT)))
            {
                entry_values[i] = 0;
                entry_indices[i] = 0;
            }
        }
    }
    else if (category == ZUKAN_CATEGORY_ARTIFACTS)
    {
        for (i = 0; i < FIELD_ENCYCLOPEDIA_LAND_COUNT; i++)
        {
            if (g_saved_lands[i].record.flags & FIELD_LAND_PLACED)
            {
                ZUKAN_UNLOCK_ENTRY(i + FIELD_ENCYCLOPEDIA_ARTIFACT_BIT);
            }

            if ((g_saved_lands[i].record.flags >> 1) & 1)
            {
                ZUKAN_UNLOCK_ENTRY(i + FIELD_ENCYCLOPEDIA_ARTIFACT_FLAG_02_BIT);
            }
        }

        if (g_saved_lands[FIELD_LAND_LUCEMIA_2].word & FIELD_LAND_FLAG_04)
        {
            ZUKAN_UNLOCK_ENTRY(FIELD_ENCYCLOPEDIA_ARTIFACT_FLAG_02_BIT + FIELD_LAND_LUCEMIA);
        }

        for (i = 0; i < count; i++)
        {
            if (!ZUKAN_ENTRY_UNLOCKED(ZUKAN_RANGE_BIT(entry_indices[i], ZUKAN_FIRST_ARTIFACT_ENTRY, FIELD_ENCYCLOPEDIA_ARTIFACT_BIT)))
            {
                if (!ZUKAN_ENTRY_UNLOCKED(ZUKAN_RANGE_BIT(entry_indices[i], ZUKAN_FIRST_ARTIFACT_ENTRY, FIELD_ENCYCLOPEDIA_ARTIFACT_FLAG_02_BIT)))
                {
                    entry_values[i] = 0;
                    entry_indices[i] = 0;
                }
            }

            if (ZUKAN_ENTRY_UNLOCKED(ZUKAN_RANGE_BIT(entry_indices[i], ZUKAN_FIRST_ARTIFACT_ENTRY, FIELD_ENCYCLOPEDIA_ARTIFACT_FLAG_02_BIT)))
            {
                entry_values[i] = resource_values[entry_indices[i] + (ZUKAN_FIRST_ARTIFACT_ALT_ENTRY - ZUKAN_FIRST_ARTIFACT_ENTRY)];
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
            if (entry_indices[i] != ZUKAN_ALWAYS_SHOWN_MONSTER_ENTRY)
            {
                if (entry_indices[i] < ZUKAN_FIRST_LATE_MONSTER_ENTRY)
                {
                    if (!ZUKAN_ENTRY_UNLOCKED(ZUKAN_RANGE_BIT(entry_indices[i], ZUKAN_FIRST_MONSTER_ENTRY, ZUKAN_MONSTER_BIT)))
                    {
                        entry_values[i] = 0;
                        entry_indices[i] = 0;
                    }
                }
                else
                {
                    if (!ZUKAN_ENTRY_UNLOCKED(ZUKAN_RANGE_BIT(entry_indices[i], ZUKAN_FIRST_LATE_MONSTER_ENTRY, ZUKAN_LATE_MONSTER_BIT)))
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
            if (!ZUKAN_ENTRY_UNLOCKED(ZUKAN_RANGE_BIT(entry_indices[i], ZUKAN_FIRST_CHARACTER_ENTRY, ZUKAN_CHARACTER_BIT)))
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
            if (!ZUKAN_ENTRY_UNLOCKED(ZUKAN_RANGE_BIT(entry_indices[i], ZUKAN_FIRST_CACTUS_DIARY_ENTRY, ZUKAN_CACTUS_DIARY_BIT)))
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
        for (i = 0; i < FIELD_ABILITY_WORD_COUNT * SAVED_BITS_PER_WORD; i++)
        {
            if ((g_saved_ability_bits[i / SAVED_BITS_PER_WORD] >> (i % SAVED_BITS_PER_WORD)) & 1)
            {
                if (i == FIELD_ABILITY_JUMP)
                {
                    ZUKAN_UNLOCK_ENTRY(ZUKAN_JUMP_BIT);
                }

                if (i == FIELD_ABILITY_DEFEND)
                {
                    ZUKAN_UNLOCK_ENTRY(ZUKAN_DEFEND_BIT);
                }

                if ((i >= FIELD_ABILITY_LUNGE) && (i <= FIELD_ABILITY_SLIDE))
                {
                    ZUKAN_UNLOCK_ENTRY(ZUKAN_RANGE_BIT(i, FIELD_ABILITY_LUNGE, ZUKAN_LUNGE_BIT));
                }

                if (i == FIELD_ABILITY_FLIP_KICK)
                {
                    ZUKAN_UNLOCK_ENTRY(ZUKAN_FLIP_KICK_BIT);
                }
            }
        }

        for (i = 0; i < FIELD_WEAPON_CATEGORY_COUNT * SAVED_BITS_PER_WORD; i++)
        {
            if ((i % SAVED_BITS_PER_WORD) < FIELD_TECHNIQUES_PER_WEAPON)
            {
                if ((g_saved_technique_bits[i / SAVED_BITS_PER_WORD] >> (i % SAVED_BITS_PER_WORD)) & 1)
                {
                    j = i - ((i / SAVED_BITS_PER_WORD) * (SAVED_BITS_PER_WORD - FIELD_TECHNIQUES_PER_WEAPON));
                    ZUKAN_UNLOCK_ENTRY(j + ZUKAN_TECHNIQUE_BIT);
                }
            }
        }

        for (i = 0; i < count; i++)
        {
            if ((entry_indices[i] >= ZUKAN_CHARACTER_END) && (entry_indices[i] < ZUKAN_FIRST_TECHNIQUE_ENTRY))
            {
                if (!ZUKAN_ENTRY_UNLOCKED(ZUKAN_RANGE_BIT(entry_indices[i], ZUKAN_CHARACTER_END, ZUKAN_ABILITY_BIT)))
                {
                    entry_values[i] = 0;
                    entry_indices[i] = 0;
                }
            }
            else
            {
                if (!ZUKAN_ENTRY_UNLOCKED(ZUKAN_RANGE_BIT(entry_indices[i], ZUKAN_FIRST_TECHNIQUE_ENTRY, ZUKAN_TECHNIQUE_BIT)))
                {
                    entry_values[i] = 0;
                    entry_indices[i] = 0;
                }
            }
        }
    }

    return count;
}
