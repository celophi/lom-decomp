#include "common.h"
#include "field_calls.h"
#include "field_script.h"
#include "game_audio.h"
#include "main.h"
#include "field_records.h"
#include "field_types.h"
#include "sdk/rand.h"

/** @brief Number of status effects; effect n owns status timer n and effect flag bit n. */
#define FIELD_STATUS_EFFECT_COUNT 15
/** @brief Longest duration a status effect can be given. */
#define FIELD_STATUS_MAX_DURATION 240
/** @brief Effect flag bits that have a status timer. */
#define FIELD_STATUS_TIMED_EFFECT_FLAGS_MASK 0xFFFF

/** @brief Status slot ids FIELD_STATUS_SLOT_ID_BASE + n grant immunity to effect n. */
#define FIELD_STATUS_SLOT_ID_BASE 0x60
#define FIELD_STATUS_SLOT_ID_COUNT 16

/** @brief Stat scale that leaves a stat unchanged; field_scale_status_stat scales in eighths. */
#define FIELD_STAT_SCALE_UNIT 8

/** @brief Stat selectors of field_scale_status_stat beyond the eight single stats. */
enum
{
    FIELD_STAT_SELECT_ALL = 9,
    FIELD_STAT_SELECT_GROUP_A = 10,
    FIELD_STAT_SELECT_GROUP_B = 11
};

/** @brief Shared animations played for a stat change: all stats, or the first of eight per-stat ones. */
#define FIELD_ANIMATION_ALL_STATS_UP 0x9D
#define FIELD_ANIMATION_STAT_UP 0x9E
#define FIELD_ANIMATION_ALL_STATS_DOWN 0xA6
#define FIELD_ANIMATION_STAT_DOWN 0xA7

/** @brief Facing angles (256 per turn) from FIELD_FACING_NEGATIVE_X_MIN to _MAX face towards negative X. */
#define FIELD_FACING_NEGATIVE_X_MIN 0x40
#define FIELD_FACING_NEGATIVE_X_MAX 0xC0

/** @brief Item type of a weapon that adds FIELD_FOOTPRINT_WEAPON_BONUS to a character's footprint strength. */
#define FIELD_ITEM_TYPE_FOOTPRINT_BONUS 7
#define FIELD_FOOTPRINT_WEAPON_BONUS 0x80
/** @brief Largest footprint strength. */
#define FIELD_FOOTPRINT_STRENGTH_MAX 0xFF

/** @brief Race of the party characters' status records. */
#define FIELD_PARTY_RACE 15
/** @brief HP multiplier of the party characters in a duel. */
#define FIELD_DUEL_HP_SCALE 3
/** @brief Counter and counter reset of a new party record. */
#define FIELD_PARTY_COUNTER_START 5

/** @brief Rows of g_field_monster_level_by_rank. */
#define FIELD_LEVEL_RANK_COUNT 64
/** @brief Ranks added on FIELD_DIFFICULTY_HARD. */
#define FIELD_LEVEL_RANK_HARD_BONUS 20

/** @brief Status intensity range; field_raise_companion_intensity wraps it to 0 at this value. */
#define FIELD_INTENSITY_RANGE 256
/** @brief Intensity range with one increment multiplier in field_raise_companion_intensity. */
#define FIELD_INTENSITY_QUARTER (FIELD_INTENSITY_RANGE / 4)

/** @brief Valid grid_bound values of the golem companion. */
#define FIELD_LOGIC_GRID_BOUND_MIN 4
#define FIELD_LOGIC_GRID_BOUND_MAX 7
/** @brief Logic values written when the random-action roll succeeds, or when no grid cell applies. */
#define FIELD_LOGIC_RANDOM 100
#define FIELD_LOGIC_INVALID 99
/** @brief Block id written when no grid cell supplies one. */
#define FIELD_LOGIC_FALLBACK_BLOCK 129

/** @brief Companion script variables written by the golem logic functions. */
#define FIELD_VAR_LOGIC_CELL 0xD028
#define FIELD_VAR_LOGIC_BLOCK 0xD030
#define FIELD_VAR_LOGIC_EDGE 0xD040
/** @brief Companion script variable receiving the active stored companion's flag word. */
#define FIELD_VAR_COMPANION_FLAGS 0xF020

/** @brief Resource with the monster templates and the battle reward table. */
#define FIELD_RESOURCE_BATTLE 1

/** @brief Diagnostic codes of this file. */
#define DIAG_BAD_STATUS_RECORD 0x68   /**< No status record has the requested id. */
#define DIAG_BAD_LOGIC_GRID_SIZE 0x74 /**< The companion's grid_bound is outside 4 to 7 (passed as the status). */
#define DIAG_BAD_REGION_INDEX 0x75    /**< The active stored companion index is out of range. */
#define DIAG_NEGATIVE_DAMAGE 0x7A     /**< A negative damage amount. */

/** @brief Header of the FIELD_RESOURCE_BATTLE resource: byte offsets of its tables from the resource start. */
typedef struct FieldBattleResource
{
    s32 unk0;
    s32 templates_offset;
    s32 rewards_offset;
} FieldBattleResource;

extern FieldGameState* g_field_game_state;
extern FieldBattleContext* g_field_battle;
extern FieldBattleContext g_field_battle_context;
extern FieldActionBank* g_field_action_bank;
extern FieldActionBank g_field_default_action_bank;
extern s32 g_field_duel_mode;

/** @brief Per-effect immunity masks; also indexed by status slot id - FIELD_STATUS_SLOT_ID_BASE. */
extern u8 g_field_status_immunity_masks[FIELD_STATUS_SLOT_ID_COUNT];
/** @brief Per-effect stats scaling the duration: high nibble the source's stat, low nibble the target's. */
extern u8 g_field_status_duration_stats[FIELD_STATUS_SLOT_ID_COUNT];
extern u8 g_field_element_level_by_land_level[];
/** @brief Status signal stored for a stat change, indexed by the scale's distance from FIELD_STAT_SCALE_UNIT. */
extern u8 g_field_stat_change_signals[];
/** @brief Base monster level of each level rank. */
extern u8 g_field_monster_level_by_rank[FIELD_LEVEL_RANK_COUNT];

s32 field_get_actor_facing(s32 actor_id);
s32 field_get_actor_position(s32 actor_id, Vec3i* position);
/* Declared without parameters: field_revive_status_record passes only the key. */
s32 field_revive_actor();
FieldStatusState* field_find_object_state(s32 actor_id);
s32 field_spawn_shared_animation_actor(s32 key, s32 resource_index);
void func_800C1EC8(s32 value, void* buffer, s32 size);
u8* func_800C1E40(s32 resource_id);

static void field_battle_reset_context(void);
static s32 field_build_party_records(void);
static void field_publish_companion_flags(void);

/**
 * @brief Find the battle status record with an identifier.
 * @param record_id Record identifier (party slot for the party records).
 * @return Matching record, or NULL after a diagnostic when no record has the identifier.
 */
FieldStatusRecord* field_find_status_record(s32 record_id)
{
    s32 i;

    for (i = 0; i < FIELD_BATTLE_RECORD_COUNT; i++)
    {
        if (record_id == g_field_battle->records[i].meta.bytes.id)
        {
            return &g_field_battle->records[i];
        }
    }
    record_game_diagnostic(DIAG_ERROR, DIAG_BAD_STATUS_RECORD, record_id, -1);
    return NULL;
}

/**
 * @brief Find the first unused status record after the party records.
 * @return First inactive non-party record, or NULL when all are in use.
 */
FieldStatusRecord* field_find_free_status_record(void)
{
    s32 i;

    for (i = FIELD_PARTY_SIZE; i < FIELD_BATTLE_RECORD_COUNT; i++)
    {
        if (!g_field_battle->records[i].meta.bits.active)
        {
            return &g_field_battle->records[i];
        }
    }
    return NULL;
}

/**
 * @brief Roll a timed status effect onto a record and set its duration.
 *
 * The effect fails on a defeated target, on an immunity (the target's own
 * immunity_flags or one granted by its status slots), on an effect already
 * active, and when the random byte is not below @p chance_threshold. The
 * duration is scaled by one stat of each record (g_field_status_duration_stats)
 * and capped at FIELD_STATUS_MAX_DURATION.
 *
 * @param source Record causing the effect.
 * @param target Record receiving the effect.
 * @param apply_flags FIELD_STATUS_APPLY_* bits.
 * @param effect_index Effect to apply, below FIELD_STATUS_EFFECT_COUNT.
 * @param chance_threshold Success threshold for a random value from 0 to 255.
 * @param duration Base duration in status ticks.
 */
void field_apply_status_effect(FieldStatusRecord* source, FieldStatusRecord* target, s32 apply_flags, s32 effect_index, s32 chance_threshold, s32 duration)
{
    s32 effect_mask;
    s32 slot_index;
    s32 attack;
    s32 defense;
    s32 scaled_duration;
    FieldStatusRecord* timer_record;

    if (effect_index >= FIELD_STATUS_EFFECT_COUNT)
    {
        return;
    }
    if (target->state->current == 0)
    {
        return;
    }
    if (!(apply_flags & FIELD_STATUS_APPLY_IGNORE_IMMUNITY))
    {
        if (target->immunity_flags & g_field_status_immunity_masks[effect_index])
        {
            return;
        }
        effect_mask = 0;
        for (slot_index = 0; slot_index < FIELD_STATUS_SLOT_COUNT; slot_index++)
        {
            if (target->status_slots[slot_index] >= FIELD_STATUS_SLOT_ID_BASE &&
                target->status_slots[slot_index] < FIELD_STATUS_SLOT_ID_BASE + FIELD_STATUS_SLOT_ID_COUNT)
            {
                effect_mask |= g_field_status_immunity_masks[target->status_slots[slot_index] - FIELD_STATUS_SLOT_ID_BASE];
            }
        }
        if (effect_mask & g_field_status_immunity_masks[effect_index])
        {
            return;
        }
    }
    effect_mask = 1 << effect_index;
    if (!(apply_flags & FIELD_STATUS_APPLY_ALLOW_ACTIVE) && (target->state->effect_flags & effect_mask))
    {
        return;
    }
    if ((rand() & 0xFF) >= chance_threshold)
    {
        return;
    }
    target->state->effect_flags |= effect_mask;
    attack = field_get_status_stat(source, g_field_status_duration_stats[effect_index] >> 4);
    defense = field_get_status_stat(target, g_field_status_duration_stats[effect_index] & 0xF);
    scaled_duration = duration * attack / defense;
    /* A shifted record, not &status_timers[effect_index]: the original forms this address before the clamp. */
    timer_record = (FieldStatusRecord*)((u8*)target + effect_index * sizeof(u16));
    if (scaled_duration > FIELD_STATUS_MAX_DURATION)
    {
        scaled_duration = FIELD_STATUS_MAX_DURATION;
    }
    timer_record->status_timers[0] = scaled_duration;
}

/**
 * @brief Read a battle stat of a record, never less than 1 (it is used as a divisor).
 * @param record Status record to read.
 * @param stat_index Stat index, below FIELD_STATUS_STAT_COUNT.
 * @return The stat, or 1 when it is 0 or @p stat_index is out of range.
 */
s32 field_get_status_stat(FieldStatusRecord* record, s32 stat_index)
{
    if (stat_index < FIELD_STATUS_STAT_COUNT)
    {
        return (record->stats[stat_index] != 0) ? record->stats[stat_index] : 1;
    }

    return 1;
}

/**
 * @brief Raise or lower one or more battle stats to a multiple of their base values.
 *
 * A raise never lowers a stat and a lowering never raises one. With
 * @p emit_signal the record's status signal is set from
 * g_field_stat_change_signals and the matching stat-change animation plays.
 *
 * @param record Status record to update.
 * @param stat_selector Stat index, or a FIELD_STAT_SELECT_* group.
 * @param scale New value in eighths of the base value; above FIELD_STAT_SCALE_UNIT raises.
 * @param emit_signal Nonzero to set the signal and play the animation.
 * @return 0 for a defeated record; the other paths return no value.
 */
s32 field_scale_status_stat(FieldStatusRecord* record, u32 stat_selector, u32 scale, s32 emit_signal)
{
    u32 value;
    s32 stat_index;

    if (record->state->current == 0)
    {
        return 0;
    }
    if (stat_selector < FIELD_STATUS_STAT_COUNT)
    {
        value = record->base_stats[stat_selector] * scale / FIELD_STAT_SCALE_UNIT;
        if (scale > FIELD_STAT_SCALE_UNIT)
        {
            if (value < record->stats[stat_selector])
            {
                value = record->stats[stat_selector];
            }
            record->stats[stat_selector] = value;
            if (emit_signal != 0)
            {
                record->state->signal.stat_change = g_field_stat_change_signals[scale - FIELD_STAT_SCALE_UNIT];
                if (record->state->signal.stat_change != 0)
                {
                    field_spawn_shared_animation_actor(record->meta.bytes.id, FIELD_ANIMATION_STAT_UP + stat_selector);
                }
            }
        }
        else
        {
            if (record->stats[stat_selector] < value)
            {
                value = record->stats[stat_selector];
            }
            record->stats[stat_selector] = value;
            if (emit_signal != 0)
            {
                record->state->signal.stat_change = g_field_stat_change_signals[FIELD_STAT_SCALE_UNIT - scale];
                if (record->state->signal.stat_change != 0)
                {
                    field_spawn_shared_animation_actor(record->meta.bytes.id, FIELD_ANIMATION_STAT_DOWN + stat_selector);
                }
            }
        }
    }
    else
    {
        switch (stat_selector)
        {
        case FIELD_STAT_SELECT_ALL:
            for (stat_index = 0; stat_index < FIELD_STATUS_STAT_COUNT; stat_index++)
            {
                field_scale_status_stat(record, stat_index, scale, 0);
            }
            if (emit_signal != 0)
            {
                if (scale > FIELD_STAT_SCALE_UNIT)
                {
                    record->state->signal.stat_change = g_field_stat_change_signals[scale - FIELD_STAT_SCALE_UNIT];
                    field_spawn_shared_animation_actor(record->meta.bytes.id, FIELD_ANIMATION_ALL_STATS_UP);
                }
                else
                {
                    record->state->signal.stat_change = g_field_stat_change_signals[FIELD_STAT_SCALE_UNIT - scale];
                    field_spawn_shared_animation_actor(record->meta.bytes.id, FIELD_ANIMATION_ALL_STATS_DOWN);
                }
            }
            break;
        case FIELD_STAT_SELECT_GROUP_A:
            field_scale_status_stat(record, 0, scale, 0);
            field_scale_status_stat(record, 1, scale, 0);
            field_scale_status_stat(record, 2, scale, 0);
            break;
        case FIELD_STAT_SELECT_GROUP_B:
            field_scale_status_stat(record, 3, scale, 0);
            field_scale_status_stat(record, 5, scale, 0);
            field_scale_status_stat(record, 6, scale, 0);
            break;
        }
    }
}

/**
 * @brief Roll a random value from 0 to 255 against the record's last battle stat.
 * @param record Status record supplying the threshold.
 * @return 1 when the roll is below the stat, otherwise 0.
 */
s32 field_roll_last_stat(FieldStatusRecord* record)
{
    u32 threshold;

    threshold = field_get_status_stat(record, FIELD_STATUS_STAT_COUNT - 1);
    return (rand() & 0xFF) < threshold;
}

/**
 * @brief Tell whether an actor is on the side of the X axis that another actor faces.
 * @param actor_id Actor whose position is tested.
 * @param observer_id Actor whose position and facing are the reference.
 * @return -1 when @p actor_id is in front of @p observer_id, otherwise 0.
 */
s32 field_is_actor_in_front(s32 actor_id, s32 observer_id)
{
    s32 facing;
    Vec3i actor_position;
    Vec3i observer_position;

    facing = field_get_actor_facing(observer_id);
    field_get_actor_position(actor_id, &actor_position);
    field_get_actor_position(observer_id, &observer_position);
    if (actor_position.x - observer_position.x < 0)
    {
        if (facing < FIELD_FACING_NEGATIVE_X_MIN || facing > FIELD_FACING_NEGATIVE_X_MAX)
        {
            return 0;
        }
        return -1;
    }
    else
    {
        if (facing < FIELD_FACING_NEGATIVE_X_MIN || facing > FIELD_FACING_NEGATIVE_X_MAX)
        {
            return -1;
        }
        return 0;
    }
}

/**
 * @brief Take damage from a status state's HP, stopping at 0.
 * @param state Status state to update.
 * @param damage Damage to take; a negative value is only reported.
 */
void field_damage_status(FieldStatusState* state, s32 damage)
{
    s32 remaining;

    if (damage < 0)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_NEGATIVE_DAMAGE, state->actor_id, damage);
    }
    else
    {
        remaining = state->current - damage;
        if (remaining >= 0)
        {
            state->current = remaining;
        }
        else
        {
            state->current = 0;
        }
    }
}

/**
 * @brief Restore a status state's HP, stopping at its maximum.
 * @param state Status state to update.
 * @param amount HP to restore.
 */
void field_heal_status(FieldStatusState* state, s32 amount)
{
    u32 maximum;
    u32 sum;

    maximum = state->maximum;
    sum = state->current + amount;
    state->current = sum;
    if (maximum < sum)
    {
        state->current = maximum;
    }
}

/**
 * @brief Revive the actor of a status record.
 * @param record Status record whose identifier is the actor key.
 */
void field_revive_status_record(FieldStatusRecord* record)
{
    field_revive_actor(record->meta.bytes.id);
}

/**
 * @brief Clear one timed status effect, or all of them.
 * @param record Status record to update.
 * @param effect_index Effect to clear; FIELD_STATUS_TIMER_COUNT or more clears every timed effect.
 */
void field_clear_status_effect(FieldStatusRecord* record, u32 effect_index)
{
    s32 timer_index;

    if (effect_index < FIELD_STATUS_TIMER_COUNT)
    {
        record->state->effect_flags &= ~(1 << effect_index);
        record->status_timers[effect_index] = 0;
        return;
    }
    record->state->effect_flags &= ~FIELD_STATUS_TIMED_EFFECT_FLAGS_MASK;
    for (timer_index = FIELD_STATUS_TIMER_COUNT - 1; timer_index >= 0; timer_index--)
    {
        record->status_timers[timer_index] = 0;
    }
}

/**
 * @brief Choose the golem companion's logic grid cell and store it in FIELD_VAR_LOGIC_CELL.
 *
 * With the companion's random-action chance (byte 2 of its golem data, in
 * percent) the result is FIELD_LOGIC_RANDOM. Otherwise the row follows the
 * companion's status intensity and the column the distance bucket of
 * @p actor_id, both within the golem's grid_bound.
 *
 * @param actor_id Actor whose distance selects the column.
 */
void field_golem_select_logic_cell(s32 actor_id)
{
    u32 chance;
    u8 unused[32]; /* never used; the original stack frame reserves it */
    u32 grid_bound;
    u32 column;
    u32 row;

    chance = g_field_game_state->characters[FIELD_PARTY_COMPANION].unk150[0].derived.bytes[2];
    if (rand() % 100 < chance)
    {
        field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_LOGIC_CELL, FIELD_LOGIC_RANDOM);
    }
    else
    {
        grid_bound = g_field_game_state->characters[FIELD_PARTY_COMPANION].unk150[0].derived.bytes[0] >> 4;
        if (grid_bound < FIELD_LOGIC_GRID_BOUND_MIN || grid_bound > FIELD_LOGIC_GRID_BOUND_MAX)
        {
            record_game_diagnostic(DIAG_BAD_LOGIC_GRID_SIZE, grid_bound, 0, 0);
            field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_LOGIC_CELL, FIELD_LOGIC_INVALID);
        }
        column = func_800C9ED4(actor_id);
        if (column >= grid_bound)
        {
            column = grid_bound - 1;
        }
        row = field_find_status_record(FIELD_PARTY_COMPANION)->state->status_intensity * grid_bound / FIELD_INTENSITY_RANGE;
        field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_LOGIC_CELL, row * GOLEM_GRID_WIDTH + column);
    }
}

/**
 * @brief Store a golem logic grid cell in the companion's logic script variables.
 *
 * With the companion's random-action chance the fallback block and
 * FIELD_LOGIC_RANDOM are stored instead; an out-of-range cell stores the
 * fallback block and FIELD_LOGIC_INVALID.
 *
 * @param cell_index Grid cell, row * GOLEM_GRID_WIDTH + column.
 */
void field_golem_publish_logic_cell(s32 cell_index)
{
    s32 chance;

    chance = g_field_game_state->characters[FIELD_PARTY_COMPANION].unk150[0].derived.bytes[2];
    if (rand() * 100 / (RAND_MAX + 1) < chance)
    {
        field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_LOGIC_BLOCK, FIELD_LOGIC_FALLBACK_BLOCK);
        field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_COMPANION_POWER_BONUS, 0);
        field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_LOGIC_EDGE, FIELD_LOGIC_RANDOM);
    }
    else if (cell_index < GOLEM_GRID_CELL_COUNT)
    {
        field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_LOGIC_BLOCK, g_field_game_state->golem_grid[cell_index].block_id);
        field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_COMPANION_POWER_BONUS, g_field_game_state->golem_grid[cell_index].detail);
        field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_LOGIC_EDGE, g_field_game_state->golem_grid[cell_index].edge);
    }
    else
    {
        field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_LOGIC_BLOCK, FIELD_LOGIC_FALLBACK_BLOCK);
        field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_COMPANION_POWER_BONUS, 0);
        field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_LOGIC_EDGE, FIELD_LOGIC_INVALID);
    }
}

/**
 * @brief Raise the companion's status intensity, by less in each higher quarter of its range.
 * @param amount Increment in the top quarter; lower quarters add two, three and four times it.
 * @note The intensity wraps to 0 once it reaches FIELD_INTENSITY_RANGE.
 */
void field_raise_companion_intensity(s32 amount)
{
    FieldStatusRecord* record;
    FieldStatusState* state;
    u32 value;

    record = field_find_status_record(FIELD_PARTY_COMPANION);
    state = record->state;
    value = state->status_intensity;

    switch (value / FIELD_INTENSITY_QUARTER)
    {
    case 0:
        state->status_intensity = value + (amount * 4);
        break;
    case 1:
        state->status_intensity = value + (amount * 3);
        break;
    case 2:
        state->status_intensity = value + (amount * 2);
        break;
    case 3:
        state->status_intensity = value + amount;
        break;
    }

    if (record->state->status_intensity >= FIELD_INTENSITY_RANGE)
    {
        record->state->status_intensity = 0;
    }
}

/**
 * @brief Build the battle records of the party and a monster group and publish their counts.
 *
 * FIELD_VAR_ALLY_COUNT and FIELD_VAR_ENEMY_COUNT receive the record counts
 * (1 each in a duel); func_800B48B8 updates them and
 * field_battle_side_defeated tests them for zero.
 *
 * @param group Monster group to build; 0 only runs field_battle_scan_actor_objects.
 */
void field_battle_setup(s32 group)
{
    s32 count;

    if (group != 0)
    {
        field_battle_reset_context();
        count = field_build_party_records();
        if (g_field_duel_mode != 0)
        {
            field_set_script_var(0, FIELD_VAR_ALLY_COUNT, 1);
        }
        else
        {
            field_set_script_var(0, FIELD_VAR_ALLY_COUNT, count);
        }
        count = field_build_group_monster_records(group);
        if (g_field_duel_mode != 0)
        {
            field_set_script_var(0, FIELD_VAR_ENEMY_COUNT, 1);
        }
        else
        {
            field_set_script_var(0, FIELD_VAR_ENEMY_COUNT, count);
        }
    }
    else
    {
        field_battle_scan_actor_objects();
    }
}

/**
 * @brief Clear the battle context and fill its header from the current land and FIELD_RESOURCE_BATTLE.
 */
static void field_battle_reset_context(void)
{
    s32 i;
    FieldBattleResource* resource;

    g_field_action_bank = &g_field_default_action_bank;
    g_field_battle = &g_field_battle_context;
    func_800C1EC8(0, &g_field_battle_context, sizeof(FieldBattleContext));
    g_field_battle->action = NULL;
    g_field_battle->state.level = field_compute_base_monster_level(0);

    for (i = 0; i < FIELD_ELEMENT_COUNT; i++)
    {
        g_field_battle->element_levels[i] = g_field_element_level_by_land_level[g_field_game_state->lands[g_music_track_index].levels[i]];
    }

    resource = (FieldBattleResource*)func_800C1E40(FIELD_RESOURCE_BATTLE);
    g_field_battle->templates = (FieldActorTemplateTable*)((u8*)resource + resource->templates_offset);
    g_field_battle->resources = (u8*)resource + resource->rewards_offset;
    field_set_script_var(0, FIELD_VAR_WATCHED_RECORD, -1);
}

/**
 * @brief Compute the battle's base monster level from the hero's level or the land.
 *
 * The level rank is 1.5 times the hero's level when @p use_hero_level or
 * FIELD_LEVEL_FLAG_HERO is set, otherwise the current land's distance
 * (field_get_land_distance). FIELD_DIFFICULTY_HARD adds FIELD_LEVEL_RANK_HARD_BONUS
 * ranks and FIELD_DIFFICULTY_HARDEST uses the top rank. The rank's level
 * from g_field_monster_level_by_rank is clamped to FIELD_VAR_MONSTER_LEVEL_MIN
 * and _MAX, then to FIELD_LEVEL_MAX.
 *
 * @param use_hero_level Nonzero to base the level on the hero's level.
 * @return Base monster level, at most FIELD_LEVEL_MAX.
 */
s32 field_compute_base_monster_level(s32 use_hero_level)
{
    s32 use_hero;
    s32 difficulty;
    s32 rank;
    s32 level;
    u32 level_min;
    u32 level_max;

    use_hero = use_hero_level;
    if (field_get_script_var(0, FIELD_VAR_LEVEL_FLAGS) & FIELD_LEVEL_FLAG_HERO)
    {
        use_hero = 1;
    }
    difficulty = field_get_script_var(0, FIELD_VAR_DIFFICULTY);

    if (use_hero != 0)
    {
        switch (difficulty)
        {
        case FIELD_DIFFICULTY_HARD:
            rank = g_field_game_state->control.fields.hero_level + FIELD_LEVEL_RANK_HARD_BONUS;
            break;
        case FIELD_DIFFICULTY_HARDEST:
            rank = FIELD_LEVEL_RANK_COUNT - 1;
            break;
        default:
            rank = g_field_game_state->control.fields.hero_level;
            break;
        }
        rank = (rank * 3) / 2;
    }
    else
    {
        rank = field_get_land_distance(g_music_track_index);
        switch (difficulty)
        {
        case FIELD_DIFFICULTY_HARD:
            rank += FIELD_LEVEL_RANK_HARD_BONUS;
            break;
        case FIELD_DIFFICULTY_HARDEST:
            rank = FIELD_LEVEL_RANK_COUNT - 1;
            break;
        }
    }

    if (rank >= FIELD_LEVEL_RANK_COUNT)
    {
        rank = FIELD_LEVEL_RANK_COUNT - 1;
    }

    level = g_field_monster_level_by_rank[rank];
    level_min = field_get_script_var(0, FIELD_VAR_MONSTER_LEVEL_MIN);
    level_max = field_get_script_var(0, FIELD_VAR_MONSTER_LEVEL_MAX);
    if (level < level_min)
    {
        level = level_min;
    }
    else if (level > level_max)
    {
        level = level_max;
    }

    if (level > FIELD_LEVEL_MAX)
    {
        level = FIELD_LEVEL_MAX;
    }
    return level;
}

/**
 * @brief Build the battle status records of the party characters from their game-state records.
 * @return Number of party slots in use.
 */
static s32 field_build_party_records(void)
{
    s32 active_count;
    s32 index;
    s32 stat_index;
    s32 equipment_index;
    s8 stat;
    s32 party_index;
    u32 strength;
    u32 nibbles;

    active_count = 0;
    for (party_index = 0; party_index < FIELD_PARTY_SIZE; party_index++)
    {
        if (g_field_game_state->characters[party_index].name[0] != 0)
        {
            if (g_field_duel_mode != 0 && party_index == 0)
            {
                g_field_battle->records[0].unk0 |= FIELD_RECORD_MONSTER_SIDE;
            }
            else
            {
                g_field_battle->records[party_index].unk0 |= FIELD_RECORD_PARTY_SIDE;
            }
            g_field_battle->records[party_index].race = FIELD_PARTY_RACE;
            g_field_battle->records[party_index].meta.bytes.id = party_index;
            g_field_battle->records[party_index].meta.bits.active = 1;
            if (g_field_duel_mode != 0)
            {
                g_field_battle->records[party_index].meta.bits.ally = (party_index == 0) ? 0xFF : 0;
            }
            else
            {
                g_field_battle->records[party_index].meta.bits.ally = 1;
            }
            {
                FieldBattleContext* battle = g_field_battle;

                battle->records[party_index].meta.bits.kind = g_field_game_state->characters[party_index].info.bytes[0];
                battle->records[party_index].counter = FIELD_PARTY_COUNTER_START;
                battle->records[party_index].meta.bytes.unk2 = 0;
            }
            g_field_battle->records[party_index].counter_reset = FIELD_PARTY_COUNTER_START;
            g_field_battle->records[party_index].status_flags = 0;
            g_field_battle->records[party_index].unkC = 0;
            g_field_battle->records[party_index].state = field_find_object_state(party_index);
            g_field_battle->records[party_index].unk18 = g_field_game_state->characters[party_index].equipment->derived.values[0];
            g_field_battle->records[party_index].unk1A = 25;
            for (stat_index = 0; stat_index < 4; stat_index++)
            {
                g_field_battle->records[party_index].equipment_stats[stat_index] = 0;
                g_field_battle->records[party_index].equipment_attributes[stat_index] =
                    g_field_game_state->characters[party_index].equipment->attributes[stat_index];
                /* The armor slots are read as (equipment + index)->; equipment[index]. changes the address arithmetic. */
                for (index = 1; index < FIELD_EQUIPMENT_SLOT_COUNT; index++)
                {
                    if (g_field_game_state->characters[party_index].equipment[index].kind != 0)
                    {
                        FieldBattleContext* battle = g_field_battle;

                        battle->records[party_index].equipment_stats[stat_index] +=
                            (g_field_game_state->characters[party_index].equipment + index)->derived.values[stat_index];
                        battle->records[party_index].equipment_attributes[stat_index] +=
                            (g_field_game_state->characters[party_index].equipment + index)->attributes[stat_index];
                    }
                }
            }
            func_800B4934(&g_field_battle->records[party_index]);
            nibbles = g_field_game_state->characters[party_index].equipment->bonus_nibbles.word;
            for (index = 0; index < FIELD_STATUS_STAT_COUNT; index++)
            {
                stat = field_get_equipped_stat(&g_field_game_state->characters[party_index], index);
                g_field_battle->records[party_index].stats[index] = g_field_battle->records[party_index].base_stats[index] = stat;
                g_field_battle->records[party_index].element_attack[index] = nibbles & 0xF;
                nibbles >>= 4;
                g_field_battle->records[party_index].element_attack[index] += g_field_battle->element_levels[index];
            }
            /* index is 8 here, so this clears unk4C (rewritten below). */
            g_field_battle->records[party_index].element_defense[index] = 0;
            g_field_battle->records[party_index].immunity_flags = 0;
            g_field_battle->records[party_index].weak_elements = 0;
            g_field_battle->records[party_index].resist_elements = 0;
            for (equipment_index = 1; equipment_index < FIELD_EQUIPMENT_SLOT_COUNT; equipment_index++)
            {
                if (g_field_game_state->characters[party_index].equipment[equipment_index].kind != 0)
                {
                    /* Same (equipment + index)-> form as the armor totals above. */
                    nibbles = (g_field_game_state->characters[party_index].equipment + equipment_index)->bonus_nibbles.word;
                    for (index = 0; index < FIELD_STATUS_STAT_COUNT; index++)
                    {
                        g_field_battle->records[party_index].element_defense[index] += nibbles & 0xF;
                        nibbles >>= 4;
                    }
                    g_field_battle->records[party_index].immunity_flags |= (g_field_game_state->characters[party_index].equipment + equipment_index)->flags2C;
                    g_field_battle->records[party_index].resist_elements |= (g_field_game_state->characters[party_index].equipment + equipment_index)->flags2D;
                }
            }
            g_field_battle->records[party_index].template = NULL;
            g_field_battle->records[party_index].unk4C = g_field_game_state->characters[party_index].unk43;
            if (g_field_duel_mode != 0)
            {
                g_field_battle->records[party_index].state->maximum = g_field_game_state->characters[party_index].hp * FIELD_DUEL_HP_SCALE;
                g_field_battle->records[party_index].state->current = g_field_game_state->characters[party_index].hp * FIELD_DUEL_HP_SCALE;
                g_field_battle->records[party_index].state->gauge.bits.value = g_field_game_state->characters[party_index].hp * FIELD_DUEL_HP_SCALE;
                g_field_battle->records[party_index].state->gauge.bits.hud_bits = 0;
            }
            else
            {
                g_field_battle->records[party_index].state->maximum = g_field_game_state->characters[party_index].hp;
            }
            {
                FieldStatusState* state;

                strength = g_field_battle->records[party_index].stats[3] * 2;
                if (FIELD_ITEM_TYPE(g_field_game_state->characters[party_index].equipment[FIELD_WEAPON_SLOT].info.word) == FIELD_ITEM_TYPE_FOOTPRINT_BONUS)
                {
                    strength += FIELD_FOOTPRINT_WEAPON_BONUS;
                }
                state = g_field_battle->records[party_index].state;
                if (strength <= FIELD_FOOTPRINT_STRENGTH_MAX)
                {
                    state->effect_footprint_strength = strength;
                }
                else
                {
                    state->effect_footprint_strength = FIELD_FOOTPRINT_STRENGTH_MAX;
                }
            }
            if ((g_field_game_state->characters[party_index].info.bytes[0] & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_COMPANION)
            {
                field_publish_companion_flags();
            }
            active_count += 1;
        }
    }
    return active_count;
}

/**
 * @brief Store the active stored companion's flag word in FIELD_VAR_COMPANION_FLAGS.
 * @note An out-of-range region_index is reported but still used.
 */
static void field_publish_companion_flags(void)
{
    if (g_field_game_state->region_index < 0 || g_field_game_state->region_index >= FIELD_REGION_COUNT)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_REGION_INDEX, g_field_game_state->region_index, 0);
    }
    field_set_script_var(FIELD_PARTY_COMPANION, FIELD_VAR_COMPANION_FLAGS, g_field_game_state->regions[g_field_game_state->region_index].unk48.word);
}
