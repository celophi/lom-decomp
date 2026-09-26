/**
 * @file field_slot_pool_ops.c
 * @brief Item staging helpers: slot chain moves, pool-paid level changes,
 *        stat modifier clamping and staged script emission.
 */

#include "common.h"
#include "field_calls.h"
#include "field_records.h"
#include "field_script.h"
#include "sdk/abs.h"

/** @brief Slot values that stay in slot 4 unless the release flag is set. */
#define FIELD_SLOT_IS_HIGH_CLASS(value) ((value) >= 0x51 && (value) <= 0x57)

/** @brief Slot values that stay in slot 4 while the keep flag is set. */
#define FIELD_SLOT_IS_LOW_CLASS(value) ((value) >= 0x3E && (value) <= 0x4B)

/** @brief Slot values at or above this have no slot script. */
#define FIELD_SLOT_SCRIPT_LIMIT 0xA0

/** @brief Slot whose values may be pinned by the staging flags. */
#define FIELD_PINNED_SLOT 4

/** @brief FieldItemStaging::stats byte: modifier index (low nibble) and limits row (high nibble). */
#define STAGED_STAT_MODIFIER_MASK 0x0F
#define STAGED_STAT_LIMITS_MASK 0xF0
#define STAGED_STAT_LIMITS_SHIFT 4

extern FieldItemStaging* D_80123FC4;
extern FieldItemTables* D_80123FC0;
extern s8 D_800F0C38[];
/** @brief Lowest and highest modifier index a stat may have, per limits row. */
extern u8 g_field_stat_modifier_limits[][2];

void field_script_run(FieldScriptContext* context);

static s32 field_shift_slot_chain(s32 slot);
static void field_clamp_staged_stats(void);
static void field_apply_flags2c_mask(void);
static void field_apply_flags2d_mask(void);

/**
 * @brief Queue a script from the item generation table on the active script context.
 *
 * Opens a new record when the current one already has a program, points it at
 * @p offset inside the loaded table, clears its wait word and the owner's
 * event argument, and runs the script.
 *
 * @param offset Byte offset of the script inside the loaded table; only the low 16 bits are used.
 */
void field_run_item_script(s32 offset)
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
    field_write_script_var(context->status.owner_id, FIELD_VAR_EVENT_ARGUMENT << 16, 0);
    field_script_run(g_field_script);
}

/**
 * @brief Run the slot scripts of the staged slots, from the last slot to the first.
 */
void field_run_slot_scripts(void)
{
    s32 slot;

    field_shift_slot_chain(0);
    if (D_80123FC4->slots[5] < FIELD_SLOT_SCRIPT_LIMIT)
    {
        field_run_item_script(D_80123FC0->item.slot_values[D_80123FC4->slots[5]].scripts[3]);
    }
    for (slot = 4; slot >= 3; slot--)
    {
        if (D_80123FC4->slots[slot] < FIELD_SLOT_SCRIPT_LIMIT)
        {
            field_run_item_script(D_80123FC0->item.slot_values[D_80123FC4->slots[slot]].scripts[2]);
        }
    }
    if (D_80123FC4->slots[2] < FIELD_SLOT_SCRIPT_LIMIT)
    {
        field_run_item_script(D_80123FC0->item.slot_values[D_80123FC4->slots[2]].scripts[1]);
    }
    if (D_80123FC4->slots[1] < FIELD_SLOT_SCRIPT_LIMIT)
    {
        field_run_item_script(D_80123FC0->item.slot_values[D_80123FC4->slots[1]].scripts[0]);
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
static s32 field_shift_slot_chain(s32 slot)
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
    if (field_shift_slot_chain(next) != 0)
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
 * @param cost Pool cost checked by field_can_pay_staged_cost.
 * @param value Slot value to find.
 * @param replacement Value written over the first match.
 * @return Index of the replaced slot, or 0xFF when nothing was replaced.
 */
s32 field_replace_slot_value(s32 cost, s32 value, s32 replacement)
{
    s32 slot;

    if (field_can_pay_staged_cost(cost) != 0)
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
void field_finish_staged_item(void)
{
    field_apply_flags2c_mask();
    field_apply_flags2d_mask();
    field_clamp_staged_stats();
}

/**
 * @brief Pick each stat's stronger modifier and clamp it to the stat's limits.
 *
 * Of the stat's own modifier and its base modifier, the one whose D_800F0C38
 * value has the larger magnitude wins (the own modifier on a tie); the winner
 * is then clamped to the stat's row of g_field_stat_modifier_limits.
 */
static void field_clamp_staged_stats(void)
{
    s32 i;
    s32 modifier;
    s32 base_modifier;
    s32 own_modifier;
    s32 maximum;
    u8 result;
    s8* modifier_values;
    u8* limits;
    FieldItemStaging* entry;

    for (i = 0; i < FIELD_STAGING_STAT_COUNT; i++)
    {
        /* set inside the loop: hoisting it changes the loop preheader order */
        modifier_values = D_800F0C38;
        base_modifier = D_80123FC4->base_stats[i];
        own_modifier = D_80123FC4->stats.bytes[i] & STAGED_STAT_MODIFIER_MASK;
        if (abs(modifier_values[own_modifier]) < abs(modifier_values[base_modifier]))
        {
            modifier = base_modifier;
        }
        else
        {
            modifier = own_modifier;
        }

        /* the clamp works through a shifted view; indexing stats.bytes[i] again changes register use */
        entry = FIELD_STAGING_AT(D_80123FC4, i);
        limits = g_field_stat_modifier_limits[entry->stats.bytes[0] >> STAGED_STAT_LIMITS_SHIFT];
        result = limits[0];
        if (modifier >= result)
        {
            maximum = limits[1];
            result = maximum;
            if (modifier <= maximum)
            {
                result = modifier;
            }
        }
        entry->stats.bytes[0] = (entry->stats.bytes[0] & STAGED_STAT_LIMITS_MASK) | (result & STAGED_STAT_MODIFIER_MASK);
    }
}

/**
 * @brief Apply every pending level increase of the staged level entries.
 */
void field_apply_pending_levels(void)
{
    s32 i;

    for (i = 0; i < FIELD_STAGING_LEVEL_COUNT; i++)
    {
        while (D_80123FC4->pending_levels[i] != 0)
        {
            field_raise_staged_level(i);
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
void field_raise_staged_level(s32 index)
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
static void field_apply_flags2c_mask(void)
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
static void field_apply_flags2d_mask(void)
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
void field_lower_staged_level(s32 index)
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
s32 field_can_pay_staged_cost(s32 cost)
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
