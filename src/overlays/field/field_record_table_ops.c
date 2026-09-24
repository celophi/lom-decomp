/**
 * @file field_record_table_ops.c
 * @brief Land placement flags, land distance, item keys, money and item values.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Land flag: the land has been placed on the map. */
#define FIELD_LAND_PLACED 0x01

/** @brief Land flag: TODO meaning unknown; excludes a placed land from the active count. */
#define FIELD_LAND_FLAG_02 0x02

/** @brief Land flag: TODO meaning unknown. */
#define FIELD_LAND_FLAG_04 0x04

/** @brief Land flag: the land is available for placement. */
#define FIELD_LAND_AVAILABLE 0x08

/** @brief Most lands that can be active before only the first requested land is placed. */
#define FIELD_ACTIVE_LAND_LIMIT 3

/** @brief Terminator of a placed-land list. */
#define FIELD_LAND_LIST_END 0xFF

/** @brief Largest land distance used to index D_800F198C. */
#define FIELD_LAND_DISTANCE_MAX 31

/** @brief Money saturates at this amount. */
#define FIELD_MONEY_MAX 10000000

/** @brief Resource id of the item value tables. */
#define FIELD_RESOURCE_ITEM_VALUES 0x11

/** @brief Special id slot that holds no special. */
#define FIELD_NO_SPECIAL 0xFF

/** @brief Pair of words that identifies one item record. */
typedef struct FieldItemKey
{
    s32 first;
    s32 second;
} FieldItemKey;

/** @brief Item value tables (resource 0x11). */
typedef struct
{
    u8 pad0[4];
    /** @brief Indexed by category (info bits 8-9) * 16 + type (info bits 10-15). */
    u16 type_values[0x24];
    /** @brief Indexed by the item subtype (info bits 16-21). */
    u16 subtype_values[0x40];
    /** @brief Indexed by a special id. */
    u16 special_values[1];
} FieldItemValueTables;

void* func_800C1E40(s32 resource_id);
s32 func_800C3518(s32 land_index);
s32 func_800C3688(s32 land_index);
s32 rand(void);

extern FieldGameState* D_80122B74;
extern u8 D_800F198C[];
extern u16 g_music_track_index;

/**
 * @brief Place up to two lands and write the placed land ids to a list.
 * @param first_land First land to place.
 * @param second_land Second land to place; only tried while fewer than three lands are active.
 * @param placed Output list of placed lands, terminated with FIELD_LAND_LIST_END.
 * @return Number of lands placed.
 */
s32 func_800C33E4(s32 first_land, s32 second_land, s32* placed)
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
        flags = D_80122B74->lands[i].flags;
        if ((flags & FIELD_LAND_PLACED) && !((flags >> 1) & 1))
        {
            active_count += 1;
        }
    }

    placed_count = 0;
    if (active_count < FIELD_ACTIVE_LAND_LIMIT)
    {
        if (func_800C3518(first_land) >= 0)
        {
            placed_count = 1;
            *cursor = first_land;
            cursor++;
        }
        if (func_800C3518(second_land) >= 0)
        {
            placed_count += 1;
            *cursor = second_land;
            cursor++;
        }
    }
    else if (func_800C3518(first_land) >= 0)
    {
        placed_count = 1;
        *cursor = first_land;
        cursor++;
    }

    if (placed_count == 0)
    {
        /* i is reused for the distance; a separate local changes the register allocation. */
        i = func_800C3688(g_music_track_index);
        table_index = FIELD_LAND_DISTANCE_MAX;
        if (i <= FIELD_LAND_DISTANCE_MAX)
        {
            table_index = i;
        }
        func_800C2138(D_800F198C[table_index]);
    }
    *cursor = FIELD_LAND_LIST_END;
    return placed_count;
}

/**
 * @brief Place an available land that is not placed yet, recording its placement order.
 * @param land_index Land index.
 * @return @p land_index on success, -1 when out of range or not placeable.
 */
s32 func_800C3518(s32 land_index)
{
    u8 flags;

    if (land_index < FIELD_LAND_COUNT)
    {
        flags = D_80122B74->lands[land_index].flags;
        if ((flags & FIELD_LAND_PLACED) != 0 || !((flags >> 3) & 1))
        {
            return -1;
        }
        D_80122B74->control.fields.unk2E4 += 1;
        D_80122B74->lands[land_index].flags |= FIELD_LAND_PLACED;
        D_80122B74->lands[land_index].count = D_80122B74->control.fields.unk2E4;
        return land_index;
    }
    return -1;
}

/**
 * @brief Mark a land as available.
 * @param land_index Land index; ignored when >= FIELD_LAND_COUNT.
 */
void func_800C35AC(s32 land_index)
{
    if (land_index < FIELD_LAND_COUNT)
    {
        D_80122B74->lands[land_index].flags |= FIELD_LAND_AVAILABLE;
    }
}

/**
 * @brief Classify a land's flags.
 * @param land_index Land index; >= FIELD_LAND_COUNT records a diagnostic.
 * @return 0 when unavailable or out of range; for an available land 1 when not
 *         placed, 2 without flag 0x02, 3 without flag 0x04, otherwise 4.
 */
s32 func_800C35E4(s32 land_index)
{
    u8 flags;
    u32 bits;

    if (land_index < FIELD_LAND_COUNT)
    {
        flags = D_80122B74->lands[land_index].flags;
        bits = flags;
        if ((bits >> 3) & 1)
        {
            if (!(flags & FIELD_LAND_PLACED))
            {
                return 1;
            }
            if (!((bits >> 1) & 1))
            {
                return 2;
            }
            if (((bits >> 2) & 1) == 0)
            {
                return 3;
            }
            return 4;
        }
    }
    else
    {
        record_game_diagnostic(0x8001, 0x73, land_index, 0);
    }
    return 0;
}

/**
 * @brief Distance from land 0 to a land on the map grid, plus the land's unk2 byte.
 * @param land_index Land to measure.
 * @return |dx| + |dz| between the two lands' position nibbles plus the land's unk2 byte.
 * @note Reads lands[land_index].position and .unk2 and the first word of lands[0] by raw offset.
 */
s32 func_800C3688(s32 land_index)
{
    /* Kept lever: the do-while(0) blocks and the int-cast index chain; no typed form matches. */
    s32 dx;
    s32 dz;
    u8 extra;
    u8* base;
    u8* land;
    u8* land_address;
    FieldGameState** state_ptr;

    do
    {
        state_ptr = &D_80122B74;
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
s32 func_800C36F0(FieldItemKey* key)
{
    s32 i;
    s32 first;
    s32 second;

    first = key->first;
    second = key->second;

    for (i = 0; i < FIELD_ITEM_COUNT; i++)
    {
        if ((D_80122B74->items[i].kind != 0) && (D_80122B74->items[i].unk38 == first) && (D_80122B74->items[i].unk3C == second))
        {
            return 1;
        }
    }

    /* The hero's equipment[] runs on into unk150[]: eight item records. */
    for (i = 0; i < 8; i++)
    {
        if ((D_80122B74->characters[0].equipment[i].kind != 0) && (D_80122B74->characters[0].equipment[i].unk38 == first) &&
            (D_80122B74->characters[0].equipment[i].unk3C == second))
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
void func_800C37A8(u32 seed, FieldItemKey* out)
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
    } while (func_800C36F0(&key) != 0);

    out->first = key.first;
    out->second = key.second;
}

/**
 * @brief Add money, saturating at FIELD_MONEY_MAX.
 * @param amount Amount to add.
 * @return Always 1.
 */
s32 func_800C3860(s32 amount)
{
    u32 money;

    money = D_80122B74->money + amount;
    D_80122B74->money = money;
    if (money > FIELD_MONEY_MAX)
    {
        D_80122B74->money = FIELD_MONEY_MAX;
    }
    return 1;
}

/**
 * @brief Spend money when more than the amount is available.
 * @param amount Amount to remove.
 * @return 1 when the amount was removed, 0 when there was not enough money.
 */
s32 func_800C3894(u32 amount)
{
    u32 money;

    money = D_80122B74->money;
    if (amount < money)
    {
        D_80122B74->money = money - amount;
        return 1;
    }
    return 0;
}

/**
 * @brief Compute an item's value from its type, subtype and specials.
 * @param item Item record.
 * @return Type value times subtype value, plus the value of every special.
 */
s32 func_800C38C8(FieldItemRecord* item)
{
    FieldItemValueTables* tables;
    u32 info;
    s32 value;
    u32 i;
    u8 special;

    tables = func_800C1E40(FIELD_RESOURCE_ITEM_VALUES);
    i = 0;
    info = item->info.word;
    value = tables->type_values[((info >> 4) & 0x30) + ((info >> 10) & 0x3F)] * tables->subtype_values[(info >> 16) & 0x3F];
    do
    {
        special = item->special_ids[i];
        if (special != FIELD_NO_SPECIAL)
        {
            value += tables->special_values[item->special_ids[i]];
        }
        i++;
    } while (i < 4);
    return value;
}

/**
 * @brief Compute the cached value of every inventory item that does not have one yet.
 */
void func_800C396C(void)
{
    u32 i;

    for (i = 0; i < FIELD_ITEM_COUNT; i++)
    {
        if (D_80122B74->items[i].kind != 0 && D_80122B74->items[i].handle == 0)
        {
            D_80122B74->items[i].handle = func_800C38C8(&D_80122B74->items[i]);
        }
    }
}
