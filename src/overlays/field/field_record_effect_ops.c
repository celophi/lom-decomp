/**
 * @file field_record_effect_ops.c
 * @brief Menu slot effects (rolling and grading) and pending effects of the stored companion records.
 */

#include "game_audio.h"
#include "common.h"
#include "sdk/rand.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief First item id with a row of random effect picks in resource 0xB. */
#define FIELD_EFFECT_ITEM_BASE 0x58

/** @brief Items with a row of random effect picks. */
#define FIELD_EFFECT_ITEM_COUNT 8

/** @brief Random effect picks per item row. */
#define FIELD_EFFECT_PICK_COUNT 16

/** @brief Effect code bits 0-5: effect id minus FIELD_EFFECT_ID_BASE; bits 6-7: result type. */
#define FIELD_EFFECT_CODE_ID_MASK 0x3F
#define FIELD_EFFECT_CODE_TYPE_SHIFT 6

/** @brief Threshold pairs of resource 0x10. */
#define FIELD_EFFECT_THRESHOLD_COUNT 8

/** @brief Effect ids below this one set a flag bit, the next ones up to FIELD_EFFECT_CLEAR_END clear one. */
#define FIELD_EFFECT_SET_END 8
#define FIELD_EFFECT_CLEAR_END 16

/** @brief Effect kinds of the remaining effect ids. */
#define FIELD_EFFECT_KIND_WEAPON 0xF0
#define FIELD_EFFECT_KIND_ARMOR 0xF1

/** @brief Four-bit growth accumulators are clamped to 0 to this value. */
#define FIELD_GROWTH_ACCUMULATOR_MAX 15

/** @brief Pending effect slots of a stored companion record. */
#define FIELD_REGION_PENDING_EFFECT_COUNT 3

/** @brief FieldRegionRecord::status bits 24-26: pending effects already applied. */
#define FIELD_REGION_APPLIED_SHIFT 24
#define FIELD_REGION_APPLIED_MASK 7

/** @brief Resource ids of the effect tables. */
#define FIELD_RESOURCE_EFFECT_PICKS 0xB
#define FIELD_RESOURCE_EFFECT_THRESHOLDS 0x10
#define FIELD_RESOURCE_REGION_EFFECTS 0x12

/** @brief Resource 0xB: random effect picks per item row, then the effect codes. */
typedef struct
{
    u8 header[4];
    /** @brief Sixteen candidate effects for each item FIELD_EFFECT_ITEM_BASE and up. */
    u8 picks[FIELD_EFFECT_ITEM_COUNT][FIELD_EFFECT_PICK_COUNT];
    /** @brief FIELD_EFFECT_CODE_* packed effect id and result type, indexed by the combined picks. */
    u8 codes[1];
} FieldSlotEffectTable;

/** @brief Resource 0x12 row applied to a region record (0x14 bytes). */
typedef struct
{
    /** @brief High nibble subtracted, low nibble added to each stat growth accumulator. */
    u8 stat_deltas[FIELD_CHARACTER_STAT_COUNT];
    /** @brief Same for the four total growth accumulators. */
    u8 total_deltas[4];
    /** @brief Pairs of (kind, value): a flag bit and its chance, or a FIELD_EFFECT_KIND_* and an equipment id. */
    u8 effects[4][2];
} FieldRegionEffectRow;

/** @brief Resource 0x12: a header, then one row per effect id. */
typedef struct
{
    u8 header[4];
    FieldRegionEffectRow rows[FIELD_EFFECT_COUNT];
} FieldRegionEffectTable;

/** @brief Low/high threshold pair of resource 0x10. */
typedef struct
{
    u8 low;
    u8 high;
} FieldEffectThreshold;

/** @brief Resource 0x10: threshold pairs, then per effect id the pair each slot counter is graded against. */
typedef struct
{
    u8 header[4];
    FieldEffectThreshold thresholds[FIELD_EFFECT_THRESHOLD_COUNT];
    u8 threshold_indexes[FIELD_EFFECT_COUNT][FIELD_MENU_SLOT_COUNTER_COUNT];
} FieldEffectThresholdTable;

extern void* func_800C1E40(s32);
extern FieldGameState* g_field_game_state;

static s32 field_classify_menu_slot(s32 group_index, s32 slot_index, FieldEffectThresholdTable* table);
static void field_apply_region_effect(FieldRegionRecord* record, s32 effect, FieldRegionEffectTable* table);

/**
 * @brief Roll a random effect for a menu slot from the items of its group and reset the slot's counters.
 * @param group_index Menu slot group index.
 * @param slot_index Slot within the group that receives the effect.
 * @note Every item adds one random pick; a group with a single item rolls it twice. The picks
 *       are OR-ed into an index of the effect codes.
 */
void field_roll_menu_slot_effect(s32 group_index, s32 slot_index)
{
    FieldSlotEffectTable* table;
    s32 random_value;
    s32 index;
    s32 effect_index;
    s32 selection;

    table = func_800C1E40(FIELD_RESOURCE_EFFECT_PICKS);
    if (g_field_game_state->menu_slots[group_index].flags.bits.item_count == 1)
    {
        index = 0;
        random_value = rand();
        effect_index = table->picks[g_field_game_state->menu_slots[group_index].items[0] - FIELD_EFFECT_ITEM_BASE][random_value & 0xF];
        random_value = rand();
        selection = table->picks[g_field_game_state->menu_slots[group_index].items[0] - FIELD_EFFECT_ITEM_BASE][random_value & 0xF];
        effect_index |= selection;
    }
    else
    {
        index = 1;
        random_value = rand();
        effect_index = table->picks[g_field_game_state->menu_slots[group_index].items[0] - FIELD_EFFECT_ITEM_BASE][random_value & 0xF];
        for (; index < (s32)g_field_game_state->menu_slots[group_index].flags.bits.item_count; index++)
        {
            random_value = rand();
            selection = table->picks[g_field_game_state->menu_slots[group_index].items[index] - FIELD_EFFECT_ITEM_BASE][random_value & 0xF];
            effect_index |= selection;
        }
        index = 0;
    }
    g_field_game_state->menu_slots[group_index].slots[slot_index].entry.index = (table->codes[effect_index] & FIELD_EFFECT_CODE_ID_MASK) + FIELD_EFFECT_ID_BASE;
    g_field_game_state->menu_slots[group_index].slots[slot_index].entry.bits.result_type = table->codes[effect_index] >> FIELD_EFFECT_CODE_TYPE_SHIFT;
    /* Both paths leave index at 0. */
    do
    {
        g_field_game_state->menu_slots[group_index].slots[slot_index].counters[index] = 0;
        index++;
    } while (index < FIELD_MENU_SLOT_COUNTER_COUNT);
}

/**
 * @brief Grade every used slot of a menu slot group against the effect thresholds.
 * @param group_index Menu slot group index.
 */
void field_classify_menu_slots(s32 group_index)
{
    FieldEffectThresholdTable* table;
    s32 slot_index;

    table = func_800C1E40(FIELD_RESOURCE_EFFECT_THRESHOLDS);
    if (table == NULL)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_MISSING_EFFECT_THRESHOLDS, 0, 0);
        return;
    }
    for (slot_index = 0; slot_index < FIELD_MENU_GROUP_SLOT_COUNT; slot_index++)
    {
        if (g_field_game_state->menu_slots[group_index].slots[slot_index].entry.index != FIELD_MENU_ENTRY_EMPTY)
        {
            g_field_game_state->menu_slots[group_index].slots[slot_index].handle = field_classify_menu_slot(group_index, slot_index, table);
        }
    }
}

/**
 * @brief Grade a slot's counters against the threshold pairs of its effect.
 * @param group_index Menu slot group index.
 * @param slot_index Slot within the group.
 * @param table Threshold table (resource 0x10).
 * @return FIELD_MENU_SLOT_INDEXED when every counter reaches its high threshold,
 *         FIELD_MENU_SLOT_ITEM when every counter reaches its low one,
 *         FIELD_MENU_SLOT_PLAIN when any counter is nonzero, otherwise FIELD_MENU_SLOT_UNUSED.
 */
static s32 field_classify_menu_slot(s32 group_index, s32 slot_index, FieldEffectThresholdTable* table)
{
    s32 flag;
    s32 i;

    i = 0;
    flag = -1;
    for (; i < FIELD_MENU_SLOT_COUNTER_COUNT; i++)
    {
        s32 effect = g_field_game_state->menu_slots[group_index].slots[slot_index].entry.index - FIELD_EFFECT_ID_BASE;

        if (g_field_game_state->menu_slots[group_index].slots[slot_index].counters[i] < table->thresholds[table->threshold_indexes[effect][i]].high)
        {
            flag = 0;
            break;
        }
    }
    if (flag != 0)
    {
        return FIELD_MENU_SLOT_INDEXED;
    }

    i = 0;
    flag = -1;
    for (; i < FIELD_MENU_SLOT_COUNTER_COUNT; i++)
    {
        s32 effect = g_field_game_state->menu_slots[group_index].slots[slot_index].entry.index - FIELD_EFFECT_ID_BASE;

        if (g_field_game_state->menu_slots[group_index].slots[slot_index].counters[i] < table->thresholds[table->threshold_indexes[effect][i]].low)
        {
            flag = 0;
            break;
        }
    }
    if (flag != 0)
    {
        return FIELD_MENU_SLOT_ITEM;
    }

    for (i = 0; i < FIELD_MENU_SLOT_COUNTER_COUNT; i++)
    {
        if (g_field_game_state->menu_slots[group_index].slots[slot_index].counters[i] != 0)
        {
            flag = -1;
        }
    }
    return -flag;
}

/**
 * @brief Apply each pending, not yet applied effect of the stored companion records.
 */
void field_apply_pending_region_effects(void)
{
    FieldRegionEffectTable* table;
    s32 record_index;
    s32 effect_index;
    u32 status;
    u32 value;
    s32 applied;
    FieldRegionRecord* record;

    table = func_800C1E40(FIELD_RESOURCE_REGION_EFFECTS);
    for (record_index = 0; record_index < FIELD_REGION_COUNT; record_index++)
    {
        if (g_field_game_state->regions[record_index].name[0] != 0)
        {
            for (effect_index = 0; effect_index < FIELD_REGION_PENDING_EFFECT_COUNT; effect_index++)
            {
                value = g_field_game_state->regions[record_index].status.effects[effect_index];
                if (value != FIELD_NO_EFFECT)
                {
                    status = g_field_game_state->regions[record_index].status.word;
                    applied = (status >> FIELD_REGION_APPLIED_SHIFT) & FIELD_REGION_APPLIED_MASK;
                    if (!((applied >> effect_index) & 1))
                    {
                        record = &g_field_game_state->regions[record_index];
                        value = (status & ~(FIELD_REGION_APPLIED_MASK << FIELD_REGION_APPLIED_SHIFT)) |
                                (((applied | (1 << effect_index)) & FIELD_REGION_APPLIED_MASK) << FIELD_REGION_APPLIED_SHIFT);
                        g_field_game_state->regions[record_index].status.word = value;
                        field_apply_region_effect(record, g_field_game_state->regions[record_index].status.effects[effect_index], table);
                    }
                }
            }
        }
    }
}

/**
 * @brief Apply one effect row to a stored companion record: growth deltas, then flag and equipment changes.
 * @param record Companion record to update.
 * @param effect Effect id; ids without an effect table row are ignored. Turned into the row index.
 * @param table Effect table (resource 0x12).
 */
static void field_apply_region_effect(FieldRegionRecord* record, s32 effect, FieldRegionEffectTable* table)
{
    s32 i;
    s32 value;
    s32 clamped;
    u8 stat_delta;
    u8 total_delta;
    s32 kind;
    s32 amount;
    FieldRegionRecord* view;

    if ((u32)(effect - FIELD_EFFECT_ID_BASE) < FIELD_EFFECT_COUNT)
    {
        effect -= FIELD_EFFECT_ID_BASE;
        for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
        {
            view = FIELD_REGION_AT(record, i);
            value = view->stat_growth[0].byte;
            stat_delta = table->rows[effect].stat_deltas[i];
            value = ((u32)value >> 4) + (stat_delta & 0xF) - (stat_delta >> 4);
            if (value >= 0)
            {
                clamped = FIELD_GROWTH_ACCUMULATOR_MAX;
                if (value <= FIELD_GROWTH_ACCUMULATOR_MAX)
                {
                    clamped = value;
                }
            }
            else
            {
                clamped = 0;
            }
            view->stat_growth[0].byte = (view->stat_growth[0].byte & 0xF) | (clamped << 4);
        }
        for (i = 0; i < 4; i++)
        {
            view = FIELD_REGION_AT(record, i);
            value = view->total_growth[0].byte;
            total_delta = table->rows[effect].total_deltas[i];
            value = ((u32)value >> 4) + (total_delta & 0xF) - (total_delta >> 4);
            if (value >= 0)
            {
                clamped = FIELD_GROWTH_ACCUMULATOR_MAX;
                if (value <= FIELD_GROWTH_ACCUMULATOR_MAX)
                {
                    clamped = value;
                }
            }
            else
            {
                clamped = 0;
            }
            view->total_growth[0].byte = (view->total_growth[0].byte & 0xF) | (clamped << 4);
        }
        for (i = 0; i < 4; i++)
        {
            kind = table->rows[effect].effects[i][0];
            amount = table->rows[effect].effects[i][1];
            if (effect < FIELD_EFFECT_SET_END)
            {
                value = rand() & 0xFF;
                if (value < amount)
                {
                    record->unk48.flags |= 1 << kind;
                }
            }
            else if (effect < FIELD_EFFECT_CLEAR_END)
            {
                value = rand() & 0xFF;
                if (value < amount)
                {
                    record->unk48.flags &= ~(1 << (kind - FIELD_EFFECT_SET_END));
                }
            }
            else
            {
                switch (kind)
                {
                case FIELD_EFFECT_KIND_WEAPON:
                    record->weapon_id = amount;
                    break;
                case FIELD_EFFECT_KIND_ARMOR:
                    record->armor_ids[0] = amount;
                    break;
                }
            }
        }
    }
}
