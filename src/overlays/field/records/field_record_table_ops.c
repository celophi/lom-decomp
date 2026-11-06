/**
 * @file field_record_table_ops.c
 * @brief Land placement flags, land distance, item keys, money and item values.
 */

#include "common.h"
#include <abs.h>
#include <rand.h>

#include "../internal/field_calls.h"
#include "../internal/field_records.h"
#include "main/audio/game_audio.h"
#include "main/main.h"

/** @brief Most lands that can be active before only the first requested land is placed. */
#define FIELD_ARTIFACTS_IN_HAND_LIMIT 3

/** @brief Terminator of a placed-land list. */
#define FIELD_LAND_LIST_END 0xFF

/** @brief Largest land distance used to index g_field_land_distance_items. */
#define FIELD_LAND_DISTANCE_MAX 31

/** @brief Resource id of the item value tables. */
#define FIELD_RESOURCE_ITEM_VALUES 0x11

/** @brief Special id slot that holds no special. */
#define FIELD_NO_SPECIAL 0xFF

/** @brief Item types per category in FieldItemValueTables::type_values. */
#define FIELD_ITEM_TYPES_PER_CATEGORY 16

/** @brief Hero item records scanned for a used key: equipment[] runs on into instruments[]. */
#define FIELD_HERO_ITEM_RECORD_COUNT 8

/** @brief field_get_land_state results. */
enum
{
    FIELD_LAND_STATE_LOCKED = 0,   /**< Artifact not unlocked, or the index is out of range. */
    FIELD_LAND_STATE_UNLOCKED = 1, /**< Artifact unlocked but not obtained. */
    FIELD_LAND_STATE_OBTAINED = 2, /**< Artifact obtained, land not placed. */
    FIELD_LAND_STATE_PLACED = 3,   /**< Land placed, FIELD_LAND_FLAG_04 clear. */
    FIELD_LAND_STATE_FLAG_04 = 4   /**< Land placed with FIELD_LAND_FLAG_04. */
};

/** @brief Item value tables (resource FIELD_RESOURCE_ITEM_VALUES). */
typedef struct
{
    u8 pad0[4];
    /** @brief Indexed by category * FIELD_ITEM_TYPES_PER_CATEGORY + type. */
    u16 type_values[0x24];
    /** @brief Indexed by the item subtype (info bits 16-21). */
    u16 subtype_values[0x40];
    /** @brief Indexed by a special id. */
    u16 special_values[1];
} FieldItemValueTables;

/** @brief Item kind given by field_award_land_artifacts when no artifact could be awarded, by land distance. */
extern u8 g_field_land_distance_items[];

static s32 field_try_award_land_artifact(s32 land_index);
static s32 field_is_item_key_used(FieldItemKey* key);

/**
 * @brief Award up to two land artifacts and write the lands whose artifacts were given to a list.
 *
 * When neither artifact can be awarded, the item for the current land's
 * distance from land 0 is given instead.
 *
 * @param first_land Land whose artifact to award first.
 * @param second_land Second land; only tried while fewer than FIELD_ARTIFACTS_IN_HAND_LIMIT artifacts are obtained but not placed.
 * @param awarded Output list of lands whose artifacts were awarded, terminated with FIELD_LAND_LIST_END.
 * @return Number of artifacts awarded.
 * @note JP indexes the distance item table without clamping the distance.
 */
s32 field_award_land_artifacts(s32 first_land, s32 second_land, s32* awarded)
{
    s32* cursor;
    s32 awarded_count;
    s32 artifacts_in_hand;
    s32 i;
#if !defined(VERSION_JP)
    u32 table_index;
#endif
    u32 flags;

    cursor = awarded;
    artifacts_in_hand = 0;

    for (i = 0; i < FIELD_LAND_COUNT; i++)
    {
        flags = g_field_game_state->lands[i].flags;

        if ((flags & FIELD_LAND_ARTIFACT_OBTAINED) && !((flags >> 1) & 1))
        {
            artifacts_in_hand++;
        }
    }

    awarded_count = 0;

    if (artifacts_in_hand < FIELD_ARTIFACTS_IN_HAND_LIMIT)
    {
        if (field_try_award_land_artifact(first_land) >= 0)
        {
            awarded_count = 1;
            *cursor = first_land;
            cursor++;
        }

        if (field_try_award_land_artifact(second_land) >= 0)
        {
            awarded_count++;
            *cursor = second_land;
            cursor++;
        }
    }
    else if (field_try_award_land_artifact(first_land) >= 0)
    {
        awarded_count = 1;
        *cursor = first_land;
        cursor++;
    }

    if (awarded_count == 0)
    {
#if defined(VERSION_JP)
        i = field_get_land_distance(g_music_track_index);
        field_receive_item(g_field_land_distance_items[i]);
#else
        i = field_get_land_distance(g_music_track_index);
        table_index = FIELD_LAND_DISTANCE_MAX;

        if (i <= FIELD_LAND_DISTANCE_MAX)
        {
            table_index = i;
        }

        field_receive_item(g_field_land_distance_items[table_index]);
#endif
    }

    *cursor = FIELD_LAND_LIST_END;
    return awarded_count;
}

/**
 * @brief Award a land's artifact if it is unlocked and not obtained yet, recording its artifact order.
 * @param land_index Land index.
 * @return @p land_index on success, -1 when out of range or the artifact cannot be awarded.
 */
static s32 field_try_award_land_artifact(s32 land_index)
{
    u8 flags;

    if (land_index < FIELD_LAND_COUNT)
    {
        flags = g_field_game_state->lands[land_index].flags;

        if ((flags & FIELD_LAND_ARTIFACT_OBTAINED) != 0 || !((flags >> 3) & 1))
        {
            return -1;
        }

        g_field_game_state->control.fields.artifact_count++;
        g_field_game_state->lands[land_index].flags |= FIELD_LAND_ARTIFACT_OBTAINED;
        g_field_game_state->lands[land_index].artifact_order = g_field_game_state->control.fields.artifact_count;
        return land_index;
    }

    return -1;
}

/**
 * @brief Unlock a land's artifact so that it can be awarded.
 * @param land_index Land index; ignored when >= FIELD_LAND_COUNT.
 */
void field_unlock_land_artifact(s32 land_index)
{
    if (land_index < FIELD_LAND_COUNT)
    {
        g_field_game_state->lands[land_index].flags |= FIELD_LAND_ARTIFACT_UNLOCKED;
    }
}

/**
 * @brief Classify a land's flags.
 * @param land_index Land index; >= FIELD_LAND_COUNT records a diagnostic.
 * @return A FIELD_LAND_STATE_* value.
 */
s32 field_get_land_state(s32 land_index)
{
    u8 flags;
    u32 bits;

    if (land_index < FIELD_LAND_COUNT)
    {
        flags = g_field_game_state->lands[land_index].flags;
        bits = flags;

        if ((bits >> 3) & 1)
        {
            if (!(flags & FIELD_LAND_ARTIFACT_OBTAINED))
            {
                return FIELD_LAND_STATE_UNLOCKED;
            }

            if (!((bits >> 1) & 1))
            {
                return FIELD_LAND_STATE_OBTAINED;
            }

            if (((bits >> 2) & 1) == 0)
            {
                return FIELD_LAND_STATE_PLACED;
            }

            return FIELD_LAND_STATE_FLAG_04;
        }
    }
    else
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_LAND, land_index, 0);
    }

    return FIELD_LAND_STATE_LOCKED;
}

/**
 * @brief Map grid distance from land 0 to a land, plus the land's placement order.
 * @param land_index Land to measure.
 * @return |dx| + |dz| between the two lands' grid cells plus the land's placement order.
 */
s32 field_get_land_distance(s32 land_index)
{
    s32 dx;
    s32 dz;
    u8 extra;

    dx = g_field_game_state->lands[land_index].x - g_field_game_state->lands[0].x;
    dx = abs(dx);

    dz = g_field_game_state->lands[land_index].z - g_field_game_state->lands[0].z;
    extra = g_field_game_state->lands[land_index].placement_order;
    dz = abs(dz);

    return extra + dx + dz;
}

/**
 * @brief Check whether a key is already used by an inventory item or one of the hero's item records.
 * @param key Key to look up.
 * @return 1 when the key is in use, 0 otherwise.
 */
static s32 field_is_item_key_used(FieldItemKey* key)
{
    s32 i;
    s32 first;
    s32 second;

    first = key->first;
    second = key->second;

    for (i = 0; i < FIELD_ITEM_COUNT; i++)
    {
        if ((g_field_game_state->items[i].name[0] != 0) && (g_field_game_state->items[i].key.first == first) &&
            (g_field_game_state->items[i].key.second == second))
        {
            return 1;
        }
    }

    for (i = 0; i < FIELD_HERO_ITEM_RECORD_COUNT; i++)
    {
        if ((g_field_game_state->characters[FIELD_PARTY_HERO].equipment[i].name[0] != 0) &&
            (g_field_game_state->characters[FIELD_PARTY_HERO].equipment[i].key.first == first) &&
            (g_field_game_state->characters[FIELD_PARTY_HERO].equipment[i].key.second == second))
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Generate a random item key, seeded by nibble masks, that no item uses yet.
 * @param seed Nibble pattern mixed into both halves of the key.
 * @param out Receives the unique key.
 */
void field_generate_item_key(u32 seed, FieldItemKey* out)
{
    FieldItemKey key;
    u32 value;
    u32 high_nibbles;
    u32 low_nibbles;
    u32 mask_high;
    u32 mask_low;

    mask_high = 0xF0F0F0F0;
    high_nibbles = seed & mask_high;
    mask_low = 0x0F0F0F0F;
    low_nibbles = seed & mask_low;

    do
    {
        value = rand();
        value += rand() << 16;
        key.first = high_nibbles | (value & mask_low);
        key.second = low_nibbles | (value & mask_high);
    } while (field_is_item_key_used(&key) != 0);

    out->first = key.first;
    out->second = key.second;
}

/**
 * @brief Add money, saturating at SAVED_MONEY_MAX.
 * @param amount Amount to add.
 * @return Always 1.
 */
s32 field_receive_money(s32 amount)
{
    u32 money;

    money = g_field_game_state->money + amount;
    g_field_game_state->money = money;

    if (money > SAVED_MONEY_MAX)
    {
        g_field_game_state->money = SAVED_MONEY_MAX;
    }

    return 1;
}

/**
 * @brief Spend money when more than the amount is available.
 * @param amount Amount to remove.
 * @return 1 when the amount was removed, 0 when there was not enough money.
 */
s32 field_spend_money(u32 amount)
{
    u32 money;

    money = g_field_game_state->money;

    if (amount < money)
    {
        g_field_game_state->money = money - amount;
        return 1;
    }

    return 0;
}

/**
 * @brief Compute an item's value from its type, subtype and specials.
 * @param item Item record.
 * @return Type value times subtype value, plus the value of every special.
 */
s32 field_get_item_value(FieldItemRecord* item)
{
    FieldItemValueTables* tables;
    u32 info;
    s32 value;
    u32 i;

    tables = field_find_resource(FIELD_RESOURCE_ITEM_VALUES);
    i = 0;
    info = item->info.word;
    value =
        tables->type_values[FIELD_ITEM_CATEGORY(info) * FIELD_ITEM_TYPES_PER_CATEGORY + FIELD_ITEM_TYPE(info)] * tables->subtype_values[(info >> 16) & 0x3F];
    for (; i < FIELD_ITEM_SPECIAL_COUNT; i++)
    {
        if (item->special_ids[i] != FIELD_NO_SPECIAL)
        {
            value += tables->special_values[item->special_ids[i]];
        }
    }

    return value;
}

/**
 * @brief Compute the cached value of every inventory item that does not have one yet.
 */
void field_cache_inventory_values(void)
{
    u32 i;

    for (i = 0; i < FIELD_ITEM_COUNT; i++)
    {
        if (g_field_game_state->items[i].name[0] != 0 && g_field_game_state->items[i].value == 0)
        {
            g_field_game_state->items[i].value = field_get_item_value(&g_field_game_state->items[i]);
        }
    }
}
