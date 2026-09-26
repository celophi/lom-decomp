/**
 * @file field_record_table_ops.c
 * @brief Land placement flags, land distance, item keys, money and item values.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Most lands that can be active before only the first requested land is placed. */
#define FIELD_ACTIVE_LAND_LIMIT 3

/** @brief Terminator of a placed-land list. */
#define FIELD_LAND_LIST_END 0xFF

/** @brief Largest land distance used to index g_field_land_distance_items. */
#define FIELD_LAND_DISTANCE_MAX 31

/** @brief Money saturates at this amount. */
#define FIELD_MONEY_MAX 10000000

/** @brief Resource id of the item value tables. */
#define FIELD_RESOURCE_ITEM_VALUES 0x11

/** @brief Special id slot that holds no special. */
#define FIELD_NO_SPECIAL 0xFF

/** @brief Item types per category in FieldItemValueTables::type_values. */
#define FIELD_ITEM_TYPES_PER_CATEGORY 16

/** @brief Hero item records scanned for a used key: equipment[] runs on into unk150[]. */
#define FIELD_HERO_ITEM_RECORD_COUNT 8

/** @brief field_get_land_state results. */
enum
{
    FIELD_LAND_STATE_UNAVAILABLE = 0, /**< Not available, or the index is out of range. */
    FIELD_LAND_STATE_AVAILABLE = 1,   /**< Available but not placed. */
    FIELD_LAND_STATE_PLACED = 2,      /**< Placed, FIELD_LAND_FLAG_02 clear. */
    FIELD_LAND_STATE_FLAG_02 = 3,     /**< Placed with FIELD_LAND_FLAG_02, FIELD_LAND_FLAG_04 clear. */
    FIELD_LAND_STATE_FLAG_04 = 4      /**< Placed with FIELD_LAND_FLAG_02 and FIELD_LAND_FLAG_04. */
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

void* func_800C1E40(s32 resource_id);
s32 rand(void);

extern FieldGameState* g_field_game_state;
/** @brief Item kind given by field_place_lands when no land could be placed, by land distance. */
extern u8 g_field_land_distance_items[];
extern u16 g_music_track_index;

static s32 field_try_place_land(s32 land_index);
static s32 field_is_item_key_used(FieldItemKey* key);

/**
 * @brief Place up to two lands and write the placed land ids to a list.
 *
 * When neither land can be placed, the item for the current land's
 * distance from land 0 is given instead.
 *
 * @param first_land First land to place.
 * @param second_land Second land to place; only tried while fewer than FIELD_ACTIVE_LAND_LIMIT lands are active.
 * @param placed Output list of placed lands, terminated with FIELD_LAND_LIST_END.
 * @return Number of lands placed.
 */
s32 field_place_lands(s32 first_land, s32 second_land, s32* placed)
{
    s32* cursor;
    s32 placed_count;
    s32 active_count;
    s32 i;
    u32 table_index;
    u32 flags;

    cursor = placed;
    active_count = 0;
    for (i = 0; i < FIELD_LAND_COUNT; i++)
    {
        flags = g_field_game_state->lands[i].flags;
        if ((flags & FIELD_LAND_PLACED) && !((flags >> 1) & 1))
        {
            active_count += 1;
        }
    }

    placed_count = 0;
    if (active_count < FIELD_ACTIVE_LAND_LIMIT)
    {
        if (field_try_place_land(first_land) >= 0)
        {
            placed_count = 1;
            *cursor = first_land;
            cursor++;
        }
        if (field_try_place_land(second_land) >= 0)
        {
            placed_count += 1;
            *cursor = second_land;
            cursor++;
        }
    }
    else if (field_try_place_land(first_land) >= 0)
    {
        placed_count = 1;
        *cursor = first_land;
        cursor++;
    }

    if (placed_count == 0)
    {
        /* i doubles as the distance: a separate local changes the register allocation. */
        i = field_get_land_distance(g_music_track_index);
        table_index = FIELD_LAND_DISTANCE_MAX;
        if (i <= FIELD_LAND_DISTANCE_MAX)
        {
            table_index = i;
        }
        field_receive_item(g_field_land_distance_items[table_index]);
    }
    *cursor = FIELD_LAND_LIST_END;
    return placed_count;
}

/**
 * @brief Place an available land that is not placed yet, recording its placement order.
 * @param land_index Land index.
 * @return @p land_index on success, -1 when out of range or not placeable.
 */
static s32 field_try_place_land(s32 land_index)
{
    u8 flags;

    if (land_index < FIELD_LAND_COUNT)
    {
        flags = g_field_game_state->lands[land_index].flags;
        if ((flags & FIELD_LAND_PLACED) != 0 || !((flags >> 3) & 1))
        {
            return -1;
        }
        g_field_game_state->control.fields.placed_land_count += 1;
        g_field_game_state->lands[land_index].flags |= FIELD_LAND_PLACED;
        g_field_game_state->lands[land_index].count = g_field_game_state->control.fields.placed_land_count;
        return land_index;
    }
    return -1;
}

/**
 * @brief Mark a land as available.
 * @param land_index Land index; ignored when >= FIELD_LAND_COUNT.
 */
void field_make_land_available(s32 land_index)
{
    if (land_index < FIELD_LAND_COUNT)
    {
        g_field_game_state->lands[land_index].flags |= FIELD_LAND_AVAILABLE;
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
        /* A single flags variable loses the andi 0xFF the original applies to the copy. */
        flags = g_field_game_state->lands[land_index].flags;
        bits = flags;
        if ((bits >> 3) & 1)
        {
            if (!(flags & FIELD_LAND_PLACED))
            {
                return FIELD_LAND_STATE_AVAILABLE;
            }
            if (!((bits >> 1) & 1))
            {
                return FIELD_LAND_STATE_PLACED;
            }
            if (((bits >> 2) & 1) == 0)
            {
                return FIELD_LAND_STATE_FLAG_02;
            }
            return FIELD_LAND_STATE_FLAG_04;
        }
    }
    else
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_LAND, land_index, 0);
    }
    return FIELD_LAND_STATE_UNAVAILABLE;
}

/**
 * @brief Map grid distance from land 0 to a land, plus the land's unk2 byte.
 * @param land_index Land to measure.
 * @return |dx| + |dz| between the two lands' grid cells plus the land's unk2 byte.
 * @note Reads the land's cell byte (low nibble x, high nibble z) and unk2 by
 *       byte offset and lands[0] as one word. TODO: typed lands[] access
 *       compiles to a different register allocation; the index chain in one
 *       variable and the one-pass do-while wrappers reproduce the original.
 */
s32 field_get_land_distance(s32 land_index)
{
    s32 dx;
    s32 dz;
    u8 extra;
    u8* base;
    u8* land;
    u8* land_address;
    FieldGameState** state_ptr;

    do
    {
        state_ptr = &g_field_game_state;
    } while (0);
    land = (u8*)(land_index << 1);
    land = (u8*)((s32)land + land_index);
    base = (u8*)*state_ptr;
    land = (u8*)((s32)land << 2);
    land_address = base + (s32)land;
    land = land_address;
    dz = land[0x2F1];
    do
    {
        land_index = *(u32*)(base + 0x2F0);
    } while (0);
    dx = dz & 0xF;
    dx -= ((u32)land_index >> 8) & 0xF;
    if (dx < 0)
    {
        dx = -dx;
    }
    dz = (u32)dz >> 4;
    land_index = (u32)land_index >> 12;
    land_index &= 0xF;
    dz -= land_index;
    do
    {
        extra = land[0x2F2];
    } while (0);
    do
    {
        do
        {
            if (dz < 0)
            {
                dz = -dz;
            }
        } while (0);
    } while (0);
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
        if ((g_field_game_state->items[i].kind != 0) && (g_field_game_state->items[i].key.first == first) &&
            (g_field_game_state->items[i].key.second == second))
        {
            return 1;
        }
    }

    for (i = 0; i < FIELD_HERO_ITEM_RECORD_COUNT; i++)
    {
        if ((g_field_game_state->characters[FIELD_PARTY_HERO].equipment[i].kind != 0) &&
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
 * @brief Add money, saturating at FIELD_MONEY_MAX.
 * @param amount Amount to add.
 * @return Always 1.
 */
s32 field_receive_money(s32 amount)
{
    u32 money;

    money = g_field_game_state->money + amount;
    g_field_game_state->money = money;
    if (money > FIELD_MONEY_MAX)
    {
        g_field_game_state->money = FIELD_MONEY_MAX;
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

    tables = func_800C1E40(FIELD_RESOURCE_ITEM_VALUES);
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
        if (g_field_game_state->items[i].kind != 0 && g_field_game_state->items[i].value == 0)
        {
            g_field_game_state->items[i].value = field_get_item_value(&g_field_game_state->items[i]);
        }
    }
}
