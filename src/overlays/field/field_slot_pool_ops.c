/**
 * @file field_slot_pool_ops.c
 * @brief Item staging helpers: slot chain moves, pool-paid level changes,
 *        stat modifier clamping and staged script emission.
 */

#include "common.h"
#include "field_calls.h"
#include "field_records.h"
#include "field_script.h"

/** @brief Slot values that stay in slot 4 unless the release flag is set. */
#define FIELD_SLOT_IS_HIGH_CLASS(value) ((value) >= 0x51 && (value) <= 0x57)

/** @brief Slot values that stay in slot 4 while the keep flag is set. */
#define FIELD_SLOT_IS_LOW_CLASS(value) ((value) >= 0x3E && (value) <= 0x4B)

/** @brief Slot values at or above this have no slot script. */
#define FIELD_SLOT_SCRIPT_LIMIT 0xA0

/** @brief Slot whose values may be pinned by the staging flags. */
#define FIELD_PINNED_SLOT 4

extern FieldItemStaging* D_80123FC4;
extern FieldItemTables* D_80123FC0;
extern s8 D_800F0C38[];
extern u8 D_800F0E88[][2];

void field_script_run(FieldScriptContext* context);

s32 func_800BF514(s32 slot);
void func_800BF730(void);
void func_800BF880(s32 index);
void func_800BF8E0(void);
void func_800BF944(void);
s32 func_800BF9F0(s32 cost);

/**
 * @brief Queue a script from the item generation table on the active script context.
 *
 * Opens a new record when the current one already has a program, points it at
 * @p offset inside the loaded table, clears its wait word and runs the script.
 *
 * @param offset Byte offset of the script inside the loaded table; only the low 16 bits are used.
 */
void func_800BF2F0(s32 offset)
{
    s32 depth;
    FieldScriptContext* context;

    depth = g_field_script->active_record;
    if (FIELD_SCRIPT_RECORD(depth)->pc != NULL)
    {
        g_field_script->active_record = depth + 1;
    }
    context = g_field_script;
    FIELD_SCRIPT_RECORD_STATE(context->active_record)->pc = D_80123FC0->bytes + (offset & 0xFFFF);
    FIELD_SCRIPT_RECORD_STATE(context->active_record)->wait.bits.resume = 0;
    FIELD_SCRIPT_RECORD_STATE(context->active_record)->wait.bits.frames = 0;
    field_write_script_var(context->status.owner_id, 0xD0000000, 0);
    field_script_run(g_field_script);
}

/**
 * @brief Run the slot scripts of the staged slots, from the last slot to the first.
 */
void func_800BF3D8(void)
{
    s32 slot;

    func_800BF514(0);
    if (D_80123FC4->slots[5] < FIELD_SLOT_SCRIPT_LIMIT)
    {
        func_800BF2F0(D_80123FC0->item.slot_values[D_80123FC4->slots[5]].scripts[3]);
    }
    for (slot = 4; slot >= 3; slot--)
    {
        if (D_80123FC4->slots[slot] < FIELD_SLOT_SCRIPT_LIMIT)
        {
            func_800BF2F0(D_80123FC0->item.slot_values[D_80123FC4->slots[slot]].scripts[2]);
        }
    }
    if (D_80123FC4->slots[2] < FIELD_SLOT_SCRIPT_LIMIT)
    {
        func_800BF2F0(D_80123FC0->item.slot_values[D_80123FC4->slots[2]].scripts[1]);
    }
    if (D_80123FC4->slots[1] < FIELD_SLOT_SCRIPT_LIMIT)
    {
        func_800BF2F0(D_80123FC0->item.slot_values[D_80123FC4->slots[1]].scripts[0]);
    }
}

/**
 * @brief Shift the slot chain from @p slot upward to make room.
 *
 * A slot value moves to the next slot when every later slot could make room.
 * Slot 4 keeps values pinned by the staging flags.
 *
 * @param slot First slot of the chain to shift.
 * @return -1 when @p slot is free or was vacated, 0 when its value must stay.
 */
s32 func_800BF514(s32 slot)
{
    s32 next;

    if (D_80123FC4->slots[slot] == FIELD_STAGING_SLOT_EMPTY)
    {
        return -1;
    }

    if (slot == FIELD_PINNED_SLOT)
    {
        if (FIELD_SLOT_IS_HIGH_CLASS(D_80123FC4->slots[FIELD_PINNED_SLOT]) && !D_80123FC4->flags.bits.release_high_slot)
        {
            return 0;
        }
        if (FIELD_SLOT_IS_LOW_CLASS(D_80123FC4->slots[slot]) && D_80123FC4->flags.bits.keep_low_slot)
        {
            return 0;
        }
    }

    next = slot + 1;
    if (func_800BF514(next) != 0)
    {
        D_80123FC4->slots[next] = D_80123FC4->slots[slot];
        D_80123FC4->slots[slot] = FIELD_STAGING_SLOT_EMPTY;
        return -1;
    }

    if (FIELD_SLOT_IS_HIGH_CLASS(D_80123FC4->slots[slot]) && !D_80123FC4->flags.bits.release_high_slot)
    {
        return 0;
    }
    if (FIELD_SLOT_IS_LOW_CLASS(D_80123FC4->slots[slot]) && D_80123FC4->flags.bits.keep_low_slot)
    {
        return 0;
    }

    return -1;
}

/**
 * @brief Replace a slot value in slots 4 down to 2, if the pool can pay for it.
 * @param cost Pool cost checked by func_800BF9F0.
 * @param value Slot value to find.
 * @param replacement Value written over the first match.
 * @return Index of the replaced slot, or 0xFF when nothing was replaced.
 */
s32 func_800BF68C(s32 cost, s32 value, s32 replacement)
{
    s32 slot;

    if (func_800BF9F0(cost) != 0)
    {
        for (slot = 4; slot >= 2; slot--)
        {
            if (D_80123FC4->slots[slot] == value)
            {
                D_80123FC4->slots[slot] = replacement;
                return slot;
            }
        }
    }

    return 0xFF;
}

/**
 * @brief Finish the staged flags and clamp the staged stat modifiers.
 */
void func_800BF700(void)
{
    func_800BF8E0();
    func_800BF944();
    func_800BF730();
}

/**
 * @brief Pick each stat's stronger modifier and clamp it to the stat's bounds.
 *
 * The modifier whose D_800F0C38 value has the larger magnitude wins between
 * the stat's own modifier and its base modifier; the winner is then clamped to
 * the stat's (minimum, maximum) row of D_800F0E88.
 */
void func_800BF730(void)
{
    s32 i;
    FieldItemStaging* entry;
    u8 packed;
    u8 result;
    s32 base_modifier;
    s32 magnitude;
    s32 base_magnitude;
    s32 modifier;
    s32 maximum;
    s32 own_modifier;
    s8* modifiers;
    s8* own_entry;
    s8* lookup;

    for (i = 0; i < FIELD_STAGING_STAT_COUNT; i++)
    {
        /* the do/while(0) blocks, the dead first own_entry and the integer sums are kept levers */
        modifiers = D_800F0C38;
        entry = FIELD_STAGING_AT(D_80123FC4, i);
        do
        {
            packed = entry->stats.bytes[0];
            base_modifier = entry->base_stats[0];
        } while (0);

        own_modifier = packed & 0xF;
        own_entry = (s8*)(own_modifier + (s32)modifiers);
        modifier = own_modifier;
        do
        {
            own_entry = (s8*)(modifier + (s32)modifiers);
        } while (0);
        lookup = (s8*)(base_modifier + (s32)modifiers);
        magnitude = *own_entry;
        base_magnitude = *lookup;
        if (magnitude < 0)
        {
            magnitude = -magnitude;
        }
        if (base_magnitude < 0)
        {
            base_magnitude = -base_magnitude;
        }
        magnitude = magnitude < base_magnitude;
        if (magnitude)
        {
            modifier = base_modifier;
        }

        lookup = (s8*)D_800F0E88[packed >> 4];
        result = ((u8*)lookup)[0];
        if (modifier >= result)
        {
            maximum = ((u8*)lookup)[1];
            result = maximum;
            if (modifier <= maximum)
            {
                result = modifier;
            }
        }

        entry->stats.bytes[0] = (entry->stats.bytes[0] & 0xF0) | (result & 0xF);
    }
}

/**
 * @brief Apply every pending level increase of the staged level entries.
 */
void func_800BF800(void)
{
    s32 i;

    for (i = 0; i < FIELD_STAGING_LEVEL_COUNT; i++)
    {
        while (D_80123FC4->pending_levels[i] != 0)
        {
            func_800BF880(i);
            D_80123FC4->pending_levels[i]--;
        }
    }
}

/**
 * @brief Raise one staged level if the pool can pay for it.
 *
 * The price is the entry's cost (at least 1) shifted left by its current
 * level; the level stops at FIELD_STAGING_LEVEL_MAX.
 *
 * @param index Level entry to raise.
 */
void func_800BF880(s32 index)
{
    FieldItemStaging* staging;
    FieldItemStaging* entry;
    s32 price;
    s32 pool;

    /* price first holds the entry's byte offset; the original reuses the variable */
    price = index * sizeof(FieldStagingLevel);
    staging = D_80123FC4;
    entry = FIELD_STAGING_AT(staging, price);
    price = 1;
    if (entry->levels[0].cost != 0)
    {
        price = entry->levels[0].cost;
    }

    pool = staging->pool;
    price <<= entry->levels[0].level;

    if (pool >= price && entry->levels[0].level < FIELD_STAGING_LEVEL_MAX)
    {
        staging->pool = pool - price;
        entry->levels[0].level++;
    }
}

/**
 * @brief Set the flags2C bits whose mask bit is set and whose level is nonzero.
 */
void func_800BF8E0(void)
{
    s32 mask;
    s32 i;

    for (i = 0, mask = 1; i < FIELD_STAGING_LEVEL_COUNT; i++, mask <<= 1)
    {
        if ((mask & D_80123FC4->flags2C_mask) && D_80123FC4->levels[i].level != 0)
        {
            D_80123FC4->flags2C |= mask;
        }
    }
}

/**
 * @brief Rebuild flags2D from the bits of flags2D_mask.
 */
void func_800BF944(void)
{
    s32 mask;
    s32 i;

    D_80123FC4->flags2D = 0;
    for (i = 0, mask = 1; i < 8; i++, mask <<= 1)
    {
        if (D_80123FC4->flags2D_mask & mask)
        {
            D_80123FC4->flags2D |= mask;
        }
    }
}

/**
 * @brief Lower one staged level and refund its price to the pool.
 * @param index Level entry to lower.
 */
void func_800BF9A0(s32 index)
{
    u8 level;

    level = D_80123FC4->levels[index].level;
    if (level != 0)
    {
        D_80123FC4->levels[index].level = level - 1;
        D_80123FC4->pool += D_80123FC4->levels[index].cost << D_80123FC4->levels[index].level;
    }
}

/**
 * @brief Test whether the pool can pay @p cost.
 * @param cost Price to test; treated as 0 while the staging slot class is 0.
 * @return -1 when the pool covers the price, else 0.
 */
s32 func_800BF9F0(s32 cost)
{
    FieldItemStaging* staging;

    staging = D_80123FC4;
    if (staging->flags.bits.slot_class == 0)
    {
        cost = 0;
    }
    if (staging->pool < cost)
    {
        return 0;
    }
    return -1;
}
