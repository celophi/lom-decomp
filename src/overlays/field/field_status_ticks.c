/**
 * @file field_status_ticks.c
 * @brief Per-frame upkeep of the battle status records: partner status slots,
 *        status timers, HP drain and regeneration, and stat recovery.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_script.h"
#include "field_records.h"

/** @brief Script variable: when non-zero, records 0 and 1 keep full status intensity. */
#define FIELD_VAR_DEBUG_FULL_INTENSITY 0xFFD

/** @brief Records below this id get full intensity from FIELD_VAR_DEBUG_FULL_INTENSITY. */
#define FULL_INTENSITY_RECORD_COUNT 2
/** @brief Status intensity given by FIELD_VAR_DEBUG_FULL_INTENSITY and STATUS_ID_FULL_INTENSITY. */
#define STATUS_INTENSITY_FULL 0xFF

/** @brief The status and regeneration updates of the second pass run every this many frames. */
#define SLOW_TICK_FRAME_MASK 0xF

/** @brief Status slot ids handled here (see field_count_status_slots). */
#define STATUS_ID_EMPTY 0
#define STATUS_ID_EXTRA_REGEN 1         /**< Each slot heals one more point per regeneration tick. */
#define STATUS_ID_FAST_REGEN 2          /**< Regeneration as with STATUS_FLAG_FAST_REGEN. */
#define STATUS_ID_CLEAR_EFFECTS 5       /**< Clears every status effect each frame. */
#define STATUS_ID_FULL_INTENSITY 6      /**< Keeps the hero (record 0) at full intensity. */
#define STATUS_ID_IMMUNITY_BASE 0x60    /**< + n: clears status effect n each frame. */
#define STATUS_ID_IMMUNITY_END 0x6C
#define STATUS_ID_STAT_BOOST_BASE 0x70  /**< + n: raises stat selector n to 10/8 each frame. */
#define STATUS_ID_STAT_BOOST_END 0x80

/** @brief field_clear_status_effect index that clears every effect. */
#define STATUS_EFFECT_ALL 0xFF
/** @brief Stat scale of a STATUS_ID_STAT_BOOST_BASE slot, in eighths. */
#define STATUS_STAT_BOOST_SCALE 10

/** @brief A record with fewer empty status slots than this has a partner. */
#define STATUS_PARTNER_EVENT_THRESHOLD 3
/** @brief Actor event run for a party member that has a partner, and its mode. */
#define STATUS_PARTNER_EVENT 0xC
#define STATUS_PARTNER_EVENT_MODE 6
/** @brief Number of partner pairs the status slots can hold. */
#define STATUS_PARTNER_PAIR_MAX FIELD_STATUS_SLOT_COUNT
/** @brief Diagnostic code for too many partner pairs (passed as the first argument). */
#define DIAG_TOO_MANY_PARTNER_PAIRS 0x6F
/** @brief End marker of the partner pair list. */
#define PARTNER_LIST_END 0xFF

/** @brief FieldStatusRecord::status_flags bits read here. */
#define STATUS_FLAG_NO_REGEN 0x1
#define STATUS_FLAG_FAST_REGEN 0x8

/** @brief FieldStatusState::effect_flags bits that drain HP: for party members, and for monsters. */
#define STATUS_EFFECTS_DRAIN_PARTY 0x190
#define STATUS_EFFECTS_DRAIN_MONSTER 0x191
/** @brief FieldStatusState::effect_flags bits that stop regeneration. */
#define STATUS_EFFECTS_NO_REGEN 0x391

/** @brief HP drained per tick, as a right shift of the maximum HP (bosses drain less). */
#define DRAIN_SHIFT 5
#define DRAIN_SHIFT_BOSS 8

/** @brief Stat that slows regeneration, and the value at which regeneration is slowest. */
#define REGEN_STAT 4
#define REGEN_STAT_RANGE 100
/** @brief Regeneration speed multipliers: fast, standing or walking, and animation REGEN_ANIMATION. */
#define REGEN_MULTIPLIER_FAST 8
#define REGEN_MULTIPLIER_IDLE 4
#define REGEN_MULTIPLIER_REST 2
/** @brief Animations below this one count as standing or walking. */
#define REGEN_ANIMATION_IDLE_END 2
/** @brief Animation that regenerates at REGEN_MULTIPLIER_REST. */
#define REGEN_ANIMATION_REST 0x31

extern FieldBattleContext* g_field_battle;
extern FieldRuntimeContext* g_field_runtime;
extern FieldGameState* g_field_game_state;
/** @brief Status flag bits granted by each equipment effect index. */
extern u16 g_field_equipment_status_flags[];

static void field_rebuild_partner_slots(void);
static void field_apply_status_slots(FieldStatusRecord* record);
static void field_tick_status_timers(FieldStatusRecord* record);
static void field_drain_status_hp(FieldStatusRecord* record);
static void field_recover_status_stats(FieldStatusRecord* record);
static void field_regenerate_status_hp(FieldStatusRecord* record);

s32 field_get_actor_animation(s32 key);

/**
 * @brief Count one more standing record on the side of a revived actor.
 *
 * Adds one to FIELD_VAR_ALLY_COUNT for a party-side record, else to
 * FIELD_VAR_ENEMY_COUNT; nothing happens outside a running battle.
 *
 * @param key Object key of the revived actor.
 */
void field_count_revived_record(s32 key)
{
    s32 count;

    if (g_field_battle != NULL && g_field_battle->state.flags >= 0)
    {
        if (field_find_status_record(key)->meta.packed & FIELD_STATUS_META_ALLY)
        {
            count = field_get_script_var(0, FIELD_VAR_ALLY_COUNT);
            field_set_script_var(0, FIELD_VAR_ALLY_COUNT, count + 1);
        }
        else
        {
            count = field_get_script_var(0, FIELD_VAR_ENEMY_COUNT);
            field_set_script_var(0, FIELD_VAR_ENEMY_COUNT, count + 1);
        }
    }
}

/**
 * @brief Rebuild a party record's status flags from its character's equipment.
 * @param record Status record whose id selects the character; its status_flags are rebuilt.
 */
void field_rebuild_equipment_status_flags(FieldStatusRecord* record)
{
    s32 i;
    FieldItemRecord* item;

    record->status_flags = 0;
    for (i = 0; i < FIELD_EQUIPMENT_SLOT_COUNT; i++)
    {
        item = &g_field_game_state->characters[record->meta.bytes.id].equipment[i];
        if (item->kind != 0)
        {
            record->status_flags |= g_field_equipment_status_flags[item->effect_index];
        }
    }
}

/**
 * @brief Per-frame update of the battle status records.
 *
 * Rebuilds the partner status slots, then applies the slots, the regeneration
 * and the status timers of every active record; every sixteenth frame it also
 * drains HP and moves the stats back toward their base values.
 */
void field_tick_battle_status(void)
{
    s32 i;

    if (g_field_battle != NULL && g_field_battle->state.flags >= 0)
    {
        field_rebuild_partner_slots();
        for (i = 0; i < FIELD_BATTLE_RECORD_COUNT; i++)
        {
            if (g_field_battle->records[i].meta.bits.active)
            {
                if (field_get_script_var(0, FIELD_VAR_DEBUG_FULL_INTENSITY) != 0 && g_field_battle->records[i].meta.bytes.id < FULL_INTENSITY_RECORD_COUNT)
                {
                    g_field_battle->records[i].state->status_intensity = STATUS_INTENSITY_FULL;
                }
                if (i < FIELD_PARTY_SIZE)
                {
                    field_apply_status_slots(&g_field_battle->records[i]);
                    field_regenerate_status_hp(&g_field_battle->records[i]);
                }
                field_tick_status_timers(&g_field_battle->records[i]);
            }
        }

        if ((g_field_runtime->frame_count & SLOW_TICK_FRAME_MASK) == 0)
        {
            for (i = 0; i < FIELD_BATTLE_RECORD_COUNT; i++)
            {
                if (g_field_battle->records[i].meta.bits.active)
                {
                    field_drain_status_hp(&g_field_battle->records[i]);
                    field_recover_status_stats(&g_field_battle->records[i]);
                }
            }
        }
    }
}

/**
 * @brief Give each partner pair's members the other's partner status.
 *
 * Clears the status slots of the party records, then for pair n of the
 * partner list stores each member's unk4C in slot n of the other member.
 * Party members 1 and 2 that now have a partner run STATUS_PARTNER_EVENT.
 */
static void field_rebuild_partner_slots(void)
{
    s32 i;
    s32 j;
    u8* pair;

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        for (j = 0; j < FIELD_STATUS_SLOT_COUNT; j++)
        {
            g_field_battle->records[i].status_slots[j] = STATUS_ID_EMPTY;
        }
    }

    i = 0;
    pair = field_pair_indicators_get_list();
    while (pair[0] != PARTNER_LIST_END)
    {
        g_field_battle->records[pair[0]].status_slots[i] = g_field_battle->records[pair[1]].unk4C;
        g_field_battle->records[pair[1]].status_slots[i] = g_field_battle->records[pair[0]].unk4C;
        pair += 2;
        i++;
    }

    if (field_count_status_slots(&g_field_battle->records[1], STATUS_ID_EMPTY) < STATUS_PARTNER_EVENT_THRESHOLD)
    {
        field_run_actor_event(1, STATUS_PARTNER_EVENT, STATUS_PARTNER_EVENT_MODE);
    }
    if (field_count_status_slots(&g_field_battle->records[2], STATUS_ID_EMPTY) < STATUS_PARTNER_EVENT_THRESHOLD)
    {
        field_run_actor_event(2, STATUS_PARTNER_EVENT, STATUS_PARTNER_EVENT_MODE);
    }
    if (i > STATUS_PARTNER_PAIR_MAX)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_TOO_MANY_PARTNER_PAIRS, i, 0);
    }
}

/**
 * @brief Count the status slots of a record that hold a given status.
 * @param record Status record to scan.
 * @param status Status id to count.
 * @return Number of the FIELD_STATUS_SLOT_COUNT slots equal to @p status.
 */
u32 field_count_status_slots(FieldStatusRecord* record, s32 status)
{
    u32 count;
    u32 i;

    i = 0;
    count = 0;
    for (; i < FIELD_STATUS_SLOT_COUNT; i++)
    {
        if (record->status_slots[i] == status)
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief Apply the per-frame effects of a party record's status slots.
 * @param record Status record whose slots are applied.
 */
static void field_apply_status_slots(FieldStatusRecord* record)
{
    s32 status;

    if (field_count_status_slots(record, STATUS_ID_CLEAR_EFFECTS) != 0)
    {
        field_clear_status_effect(record, STATUS_EFFECT_ALL);
    }

    for (status = STATUS_ID_IMMUNITY_BASE; status < STATUS_ID_IMMUNITY_END; status++)
    {
        if (field_count_status_slots(record, status) != 0)
        {
            field_clear_status_effect(record, status - STATUS_ID_IMMUNITY_BASE);
        }
    }

    if (field_count_status_slots(record, STATUS_ID_FULL_INTENSITY) != 0)
    {
        if (record->meta.bytes.id == 0)
        {
            record->state->status_intensity = STATUS_INTENSITY_FULL;
        }
    }

    for (status = STATUS_ID_STAT_BOOST_BASE; status < STATUS_ID_STAT_BOOST_END; status++)
    {
        if (field_count_status_slots(record, status) != 0)
        {
            field_scale_status_stat(record, status - STATUS_ID_STAT_BOOST_BASE, STATUS_STAT_BOOST_SCALE, 0);
        }
    }
}

/**
 * @brief Count down a record's status timers and clear the effects that run out.
 * @param record Status record whose timers are ticked.
 */
static void field_tick_status_timers(FieldStatusRecord* record)
{
    s32 i;
    s16 remaining;

    for (i = 0; i < FIELD_STATUS_TIMER_COUNT; i++)
    {
        if ((s16)record->status_timers[i] != 0)
        {
            remaining = record->status_timers[i] - 1;
            record->status_timers[i] = remaining;
            if (remaining <= 0)
            {
                field_clear_status_effect(record, i);
            }
        }
    }
}

/**
 * @brief Store drained HP, keeping at least 1.
 * @param state Status state to update.
 * @param hp HP after the drain; values below 1 store 1.
 */
static inline void field_store_drained_hp(FieldStatusState* state, s32 hp)
{
    if (hp > 0)
    {
        state->current = hp;
        return;
    }
    state->current = 1;
}

/**
 * @brief Drain a record's HP while a draining status effect is active.
 *
 * The drain is max_hp >> DRAIN_SHIFT (>> DRAIN_SHIFT_BOSS for boss monsters),
 * at least 1; HP never drops below 1.
 *
 * @param record Status record to update.
 */
static void field_drain_status_hp(FieldStatusRecord* record)
{
    FieldStatusState* state;
    s32 hp;
    u32 drain;
    s32 cost;

    state = record->state;
    hp = state->current;
    if (record->meta.bytes.id < FIELD_PARTY_SIZE)
    {
        if (state->effect_flags & STATUS_EFFECTS_DRAIN_PARTY)
        {
            cost = 1;
            drain = state->maximum >> DRAIN_SHIFT;
            if (drain != 0)
            {
                cost = drain;
            }
            hp -= cost;
            field_store_drained_hp(state, hp);
        }
    }
    else if (state->effect_flags & STATUS_EFFECTS_DRAIN_MONSTER)
    {
        if (record->template->flags & FIELD_TEMPLATE_BOSS)
        {
            cost = 1;
            drain = state->maximum >> DRAIN_SHIFT_BOSS;
            if (drain != 0)
            {
                cost = drain;
            }
        }
        else
        {
            cost = 1;
            drain = state->maximum >> DRAIN_SHIFT;
            if (drain != 0)
            {
                cost = drain;
            }
        }
        hp -= cost;
        field_store_drained_hp(record->state, hp);
    }
}

/**
 * @brief Move each stat one point toward its base value.
 * @param record Status record whose stats are updated.
 */
static void field_recover_status_stats(FieldStatusRecord* record)
{
    s32 i;
    u8 current;
    u8 target;

    for (i = 0; i < FIELD_STATUS_STAT_COUNT; i++)
    {
        current = record->stats[i];
        target = record->base_stats[i];
        if (target < current)
        {
            record->stats[i] = current - 1;
        }
        else if (current < target)
        {
            record->stats[i] = current + 1;
        }
    }
}

/**
 * @brief Regenerate one HP point when the record's regeneration interval has passed.
 *
 * The interval shrinks with the multiplier (fast regeneration, standing or
 * walking, or resting) and grows with stat REGEN_STAT; party members also heal
 * one more point per STATUS_ID_EXTRA_REGEN slot.
 *
 * @param record Status record to regenerate.
 */
static void field_regenerate_status_hp(FieldStatusRecord* record)
{
    u16 flags;
    s32 multiplier;
    s32 interval;
    s32 animation;

    flags = record->status_flags;
    if (flags & STATUS_FLAG_NO_REGEN)
    {
        return;
    }
    if (record->state->effect_flags & STATUS_EFFECTS_NO_REGEN)
    {
        return;
    }
    if ((flags & STATUS_FLAG_FAST_REGEN) || field_count_status_slots(record, STATUS_ID_FAST_REGEN) != 0)
    {
        multiplier = REGEN_MULTIPLIER_FAST;
    }
    else
    {
        animation = field_get_actor_animation(record->meta.bytes.id);
        if (animation < 0)
        {
            return;
        }
        if (animation < REGEN_ANIMATION_IDLE_END)
        {
            multiplier = REGEN_MULTIPLIER_IDLE;
        }
        else if (animation != REGEN_ANIMATION_REST)
        {
            return;
        }
        else
        {
            multiplier = REGEN_MULTIPLIER_REST;
        }
    }

    interval = (REGEN_STAT_RANGE - field_get_status_stat(record, REGEN_STAT)) * multiplier / 16;
    if (interval <= 0)
    {
        interval = 1;
    }
    if ((u32)g_field_runtime->frame_count % (u32)interval == 0)
    {
        field_heal_status(record->state, 1);
        if (record->meta.bytes.id < FIELD_PARTY_SIZE)
        {
            field_heal_status(record->state, field_count_status_slots(record, STATUS_ID_EXTRA_REGEN));
        }
    }
}
