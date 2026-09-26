/**
 * @file field_record_growth_ops.c
 * @brief Money, experience and level-up growth for party characters and
 *        stored companion records.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Money saturates at this amount. */
#define FIELD_MONEY_MAX 10000000

/** @brief Largest experience value of a progress word. */
#define FIELD_EXPERIENCE_MAX 9999999

/** @brief Progress word with the maximum experience and a zero level byte. */
#define FIELD_EXPERIENCE_MAX_WORD ((u32)FIELD_EXPERIENCE_MAX << 8)

/** @brief Largest counter value in FieldGameState::words. */
#define FIELD_COUNTER_MAX 9999999

/** @brief First FieldGameState::words entry of the per-character counters. */
#define FIELD_CHARACTER_COUNTER_BASE 0x68

/** @brief Largest base stat value (FIELD_STAT_BASE_MASK bits of a stat halfword). */
#define FIELD_STAT_MAX 0x18C

/** @brief Reward recipients below this one are party members. */
#define FIELD_REWARD_PARTY_LIMIT 3

/** @brief FieldStatusRecord::status_flags bit: the recipient shares its experience with the party. */
#define FIELD_RECORD_FLAG_SHARE_EXPERIENCE 0x4

/** @brief FieldRegionRecord::status bit 30: the stored companion gains experience. */
#define FIELD_REGION_GAINS_EXPERIENCE_SHIFT 30

/** @brief FieldRegionRecord::status bits 24-26: pending effects already applied. */
#define FIELD_REGION_APPLIED_MASK 0x07000000

/** @brief Pending effect slots of a stored companion record. */
#define FIELD_REGION_PENDING_EFFECT_COUNT 3

/** @brief Stored companions get an eighth of the experience awarded to the party. */
#define FIELD_REGION_EXPERIENCE_SHIFT 3

/** @brief Signal passed to field_spawn_shared_animation_actor after a level-up. */
#define FIELD_SIGNAL_LEVEL_UP 0x23

/** @brief Stat whose base value (quarter units) sets the hp increase of a level-up. */
#define FIELD_STAT_HP_GROWTH 4

/** @brief field_add_stat_increase flags: damp increases above 5, cap the result at 999. */
#define FIELD_INCREASE_DAMPED 0x1
#define FIELD_INCREASE_CAPPED 0x2

/** @brief Increases up to this amount are never damped. */
#define FIELD_INCREASE_UNDAMPED_MAX 5

/** @brief Largest value of a capped increase. */
#define FIELD_INCREASE_CAP 999

/** @brief A growth accumulator carries into its total at bit 3 (bit 7 of the byte); keeping bits 0-2 clears the carry. */
#define FIELD_GROWTH_CARRY_SHIFT 7
#define FIELD_GROWTH_ACCUMULATOR_KEEP 7

/** @brief Diagnostic codes of this file. */
#define DIAG_BAD_COMPANION_SLOT 0x79   /**< The active stored companion index is out of range. */
#define DIAG_BAD_REGION_SLOT 0x1F3     /**< A stored companion level-up names a slot out of range. */

extern FieldGameState* g_field_game_state;

/** @brief Per item type: eight four-bit stat increases applied on a level-up. */
extern u32 g_field_item_type_stat_growth[];

FieldStatusState* field_find_object_state(s32 actor_id);
s32 field_spawn_shared_animation_actor(s32 index, s32 value);

static void field_add_guest_counter(s32 amount);
static void field_award_region_experience(u32 amount);
static void field_grow_character_stats(FieldCharacterRecord* character);
static void field_grow_companion_stats(FieldCharacterRecord* character);

/**
 * @brief Add money, saturating at FIELD_MONEY_MAX.
 * @param recipient Reward recipient; only party members are credited.
 * @param amount Money to add.
 */
void field_add_money(s32 recipient, s32 amount)
{
    if (recipient < FIELD_REWARD_PARTY_LIMIT)
    {
        g_field_game_state->money += amount;
        if (g_field_game_state->money > FIELD_MONEY_MAX)
        {
            g_field_game_state->money = FIELD_MONEY_MAX;
        }
    }
}

/**
 * @brief Award experience, optionally shared among the living party members.
 * @param record_index Status record id of the recipient (a party member).
 * @param amount Experience to award.
 * @note With FIELD_RECORD_FLAG_SHARE_EXPERIENCE the amount is split evenly (at least 1)
 *       among the party members that are present and alive. On overflow the shared path
 *       clamps the recipient's record, not the member that overflowed.
 */
void field_award_experience(s32 record_index, s32 amount)
{
    s32 eligible_count;
    s32 index;
    u32 packed_value;
    u32 updated_value;
    FieldStatusRecord* entry;
    u32 distributed_value;
    u32 clamped_value;

    if ((record_index < FIELD_REWARD_PARTY_LIMIT) && (entry = func_800B2A9C(record_index), (entry != NULL)))
    {
        switch (entry->meta.bytes.id)
        {
        case FIELD_PARTY_HERO:
            field_award_region_experience(amount);
            break;
        case FIELD_PARTY_GUEST:
            field_add_guest_counter(amount);
            break;
        }
        index = 0;
        if (entry->status_flags & FIELD_RECORD_FLAG_SHARE_EXPERIENCE)
        {
            eligible_count = 0;
            for (; index < FIELD_PARTY_SIZE; index++)
            {
                if ((g_field_game_state->characters[index].name[0] != 0) && (field_find_object_state(index)->current != 0))
                {
                    eligible_count += 1;
                }
            }
            if (eligible_count <= 0)
            {
                eligible_count = 1;
            }
            amount /= eligible_count;
            amount = amount == 0 ? 1 : amount;
            for (index = 0; index < FIELD_PARTY_SIZE; index++)
            {
                if ((g_field_game_state->characters[index].name[0] != 0) && (field_find_object_state(index)->current != 0))
                {
                    if (index == FIELD_PARTY_GUEST)
                    {
                        field_add_guest_counter(amount);
                    }
                    packed_value = g_field_game_state->characters[index].progress.word;
                    distributed_value = (packed_value & 0xFF) | (((packed_value >> 8) + amount) << 8);
                    g_field_game_state->characters[index].progress.word = distributed_value;
                    if ((s32)(distributed_value >> 8) > FIELD_EXPERIENCE_MAX)
                    {
                        g_field_game_state->characters[record_index].progress.word =
                            g_field_game_state->characters[record_index].progress.level | FIELD_EXPERIENCE_MAX_WORD;
                    }
                    field_apply_character_level_ups(index, 1);
                }
            }
            return;
        }
        packed_value = g_field_game_state->characters[record_index].progress.word;
        updated_value = (packed_value & 0xFF) | (((packed_value >> 8) + amount) << 8);
        g_field_game_state->characters[record_index].progress.word = updated_value;
        if ((s32)(updated_value >> 8) > FIELD_EXPERIENCE_MAX)
        {
            clamped_value = updated_value & 0xFF;
            clamped_value |= FIELD_EXPERIENCE_MAX_WORD;
            g_field_game_state->characters[record_index].progress.word = clamped_value;
        }
        field_apply_character_level_ups(record_index, 1);
    }
}

/**
 * @brief Add to the counter word selected by the guest, saturating at FIELD_COUNTER_MAX.
 * @param amount Amount to add.
 * @note The counter is words[FIELD_CHARACTER_COUNTER_BASE + byte 1 of the guest's info].
 */
static void field_add_guest_counter(s32 amount)
{
    FieldGameState* state;

    state = g_field_game_state;
    state->words[state->characters[FIELD_PARTY_GUEST].info.bytes[1] + FIELD_CHARACTER_COUNTER_BASE] += amount;
    if ((u32)state->words[state->characters[FIELD_PARTY_GUEST].info.bytes[1] + FIELD_CHARACTER_COUNTER_BASE] > FIELD_COUNTER_MAX)
    {
        state->words[state->characters[FIELD_PARTY_GUEST].info.bytes[1] + FIELD_CHARACTER_COUNTER_BASE] = FIELD_COUNTER_MAX;
    }
}

/**
 * @brief Give an eighth of @p amount as experience to every stored companion that gains experience.
 * @param amount Experience awarded to the party.
 */
static void field_award_region_experience(u32 amount)
{
    u32 region_index;
    u32 packed_value;
    u32 updated_value;
    FieldGameState* state;

    amount >>= FIELD_REGION_EXPERIENCE_SHIFT;
    state = g_field_game_state;
    for (region_index = 0; region_index < FIELD_REGION_COUNT; region_index++)
    {
        if ((state->regions[region_index].name[0] != 0) && ((state->regions[region_index].status.word >> FIELD_REGION_GAINS_EXPERIENCE_SHIFT) & 1))
        {
            packed_value = state->regions[region_index].progress.word;
            updated_value = (packed_value & 0xFF) | (((packed_value >> 8) + amount) << 8);
            state->regions[region_index].progress.word = updated_value;
            if ((s32)(updated_value >> 8) > FIELD_EXPERIENCE_MAX)
            {
                state->regions[region_index].progress.word = (updated_value & 0xFF) | FIELD_EXPERIENCE_MAX_WORD;
            }
        }
    }
}

/**
 * @brief Apply every level-up a party member's experience allows.
 * @param index Party member index.
 * @param notify Nonzero to signal each level-up (see field_try_character_level_up).
 */
void field_apply_character_level_ups(s32 index, s32 notify)
{
    while (field_try_character_level_up(index, notify) != 0)
    {
    }
}

/**
 * @brief Apply every level-up a stored companion record's experience allows.
 * @param slot Companion record index (below FIELD_REGION_COUNT).
 * @note Each level adds the stat growth accumulators, rebuilds the effective stat
 *       bits, carries the total accumulators, recomputes hp and clears the
 *       pending effects.
 */
void field_apply_region_level_ups(s32 slot)
{
    s32 level;
    s32 i;
    s32 pending;
    u16 stat;
    u32 low;
    u32 base_stat;
    u32 threshold;
    u32 next_level;
    u32 experience;
    FieldRegionRecord* record;

    if (slot >= FIELD_REGION_COUNT)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_REGION_SLOT, slot, 0);
        return;
    }
    record = &g_field_game_state->regions[slot];
    pending = -1;
    if (record->name[0] != 0)
    {
        do
        {
            level = record->progress.level;
            threshold = field_level_threshold(level);
            experience = record->progress.word >> 8;
            if ((experience != 0) && (experience >= threshold))
            {
                next_level = level + 1;
                record->progress.level = next_level;
                if ((next_level & 0xFF) > FIELD_LEVEL_MAX)
                {
                    record->progress.level = FIELD_LEVEL_MAX;
                    pending = 0;
                    continue;
                }
                for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
                {
                    stat = record->stats[i];
                    low = record->stat_growth[i].bits.accumulator + (stat & FIELD_STAT_BASE_MASK);
                    low &= FIELD_STAT_BASE_MASK;
                    stat = (stat & FIELD_STAT_EFFECTIVE_MASK) | low;
                    record->stats[i] = stat;
                    if ((stat & FIELD_STAT_BASE_MASK) > FIELD_STAT_MAX)
                    {
                        record->stats[i] = (stat & FIELD_STAT_EFFECTIVE_MASK) | FIELD_STAT_MAX;
                    }
                    base_stat = record->stats[i] & FIELD_STAT_BASE_MASK;
                    record->stats[i] = base_stat | ((base_stat >> 2) << FIELD_STAT_EFFECTIVE_SHIFT);
                    record->stat_growth[i].bits.accumulator = record->stat_growth[i].bits.rate;
                }
                for (i = 0; i < 4; i++)
                {
                    record->equipment_totals[i] += record->total_growth[i].byte >> FIELD_GROWTH_CARRY_SHIFT;
                    record->total_growth[i].bits.accumulator &= FIELD_GROWTH_ACCUMULATOR_KEEP;
                    record->total_growth[i].bits.accumulator += record->total_growth[i].bits.rate;
                }
                record->unk1E += record->extra_growth.bytes[0] >> FIELD_GROWTH_CARRY_SHIFT;
                record->extra_growth.growth.accumulator &= FIELD_GROWTH_ACCUMULATOR_KEEP;
                record->extra_growth.growth.accumulator += record->extra_growth.growth.rate;
                record->hp = field_add_stat_increase(record->hp, (u32)(record->stats[FIELD_STAT_HP_GROWTH] & FIELD_STAT_BASE_MASK) >> 2,
                                                     FIELD_INCREASE_DAMPED | FIELD_INCREASE_CAPPED);
                record->extra_growth.halves[1] += record->extra_growth.bytes[1];
                record->status.word &= ~FIELD_REGION_APPLIED_MASK;
                for (i = 0; i < FIELD_REGION_PENDING_EFFECT_COUNT; i++)
                {
                    record->status.effects[i] = FIELD_NO_EFFECT;
                }
            }
            else
            {
                pending = 0;
            }
        } while (pending != 0);
    }
}

/**
 * @brief Advance a party member one level when its experience reaches the threshold.
 * @param index Party member index.
 * @param notify Nonzero to send FIELD_SIGNAL_LEVEL_UP after advancing.
 * @return -1 when the member advanced a level, otherwise 0.
 * @note A stored companion in the party grows through field_grow_companion_stats, the
 *       others through field_grow_character_stats.
 */
s32 field_try_character_level_up(s32 index, s32 notify)
{
    s32 level;

    level = g_field_game_state->characters[index].progress.level;
    if ((s32)(g_field_game_state->characters[index].progress.word >> 8) >= field_level_threshold(level))
    {
        g_field_game_state->characters[index].progress.level = level + 1;
        if (g_field_game_state->characters[index].progress.level > FIELD_LEVEL_MAX)
        {
            g_field_game_state->characters[index].progress.level = FIELD_LEVEL_MAX;
            return 0;
        }
        if ((g_field_game_state->characters[index].info.bytes[0] & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_COMPANION)
        {
            field_grow_companion_stats(&g_field_game_state->characters[index]);
        }
        else
        {
            field_grow_character_stats(&g_field_game_state->characters[index]);
        }
        func_800B7C58(index);
        if (notify != 0)
        {
            field_spawn_shared_animation_actor(index, FIELD_SIGNAL_LEVEL_UP);
        }
        return -1;
    }
    return 0;
}

/**
 * @brief Level-up growth of a party member that is not a stored companion.
 * @param character Party member record.
 * @note Adds the eight four-bit increases of the weapon's item type to the base stats,
 *       clamped to FIELD_STAT_MAX, then recomputes hp.
 */
static void field_grow_character_stats(FieldCharacterRecord* character)
{
    s32 i;
    u32 deltas;
    u16 stat;
    u32 low;

    deltas = g_field_item_type_stat_growth[character->equipment[FIELD_WEAPON_SLOT].info.bits.item_type];
    for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
    {
        stat = character->stats[i];
        low = (stat & FIELD_STAT_BASE_MASK) + (deltas & 0xF);
        low &= FIELD_STAT_BASE_MASK;
        stat = (stat & FIELD_STAT_EFFECTIVE_MASK) | low;
        character->stats[i] = stat;
        if ((stat & FIELD_STAT_BASE_MASK) > FIELD_STAT_MAX)
        {
            character->stats[i] = (stat & FIELD_STAT_EFFECTIVE_MASK) | FIELD_STAT_MAX;
        }
        deltas >>= 4;
    }
    character->hp = field_add_stat_increase(character->hp, (u32)(character->stats[FIELD_STAT_HP_GROWTH] & FIELD_STAT_BASE_MASK) >> 2,
                                            FIELD_INCREASE_DAMPED | FIELD_INCREASE_CAPPED);
}

/**
 * @brief Level-up growth of the stored companion in the party, mirrored into its stored record.
 * @param character Party member record (characters[2]).
 * @note Same algorithm as field_apply_region_level_ups, applied to the party record and
 *       the stored record regions[region_index].
 */
static void field_grow_companion_stats(FieldCharacterRecord* character)
{
    s32 i;
    u16 stat;
    u16 low;
    u16 total;
    u32 slot;

    slot = g_field_game_state->region_index;
    if (slot >= FIELD_REGION_COUNT)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_COMPANION_SLOT, slot, 0);
    }
    for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
    {
        stat = character->stats[i];
        low = g_field_game_state->regions[g_field_game_state->region_index].stat_growth[i].bits.accumulator + (stat & FIELD_STAT_BASE_MASK);
        low &= FIELD_STAT_BASE_MASK;
        stat &= FIELD_STAT_EFFECTIVE_MASK;
        stat |= low;
        character->stats[i] = stat;
        if ((stat & FIELD_STAT_BASE_MASK) > FIELD_STAT_MAX)
        {
            character->stats[i] = (stat & FIELD_STAT_EFFECTIVE_MASK) | FIELD_STAT_MAX;
        }
        g_field_game_state->regions[g_field_game_state->region_index].stat_growth[i].bits.accumulator =
            g_field_game_state->regions[g_field_game_state->region_index].stat_growth[i].bits.rate;
    }
    total = character->unk26 + (g_field_game_state->regions[g_field_game_state->region_index].extra_growth.bytes[0] >> FIELD_GROWTH_CARRY_SHIFT);
    character->unk26 = total;
    g_field_game_state->regions[g_field_game_state->region_index].unk1E = total;
    character->equipment[FIELD_WEAPON_SLOT].derived.values[0] = character->unk26;
    g_field_game_state->regions[g_field_game_state->region_index].extra_growth.growth.accumulator &= FIELD_GROWTH_ACCUMULATOR_KEEP;
    g_field_game_state->regions[g_field_game_state->region_index].extra_growth.growth.accumulator +=
        g_field_game_state->regions[g_field_game_state->region_index].extra_growth.growth.rate;
    for (i = 0; i < 4; i++)
    {
        total = character->equipment_totals[i] + (g_field_game_state->regions[g_field_game_state->region_index].total_growth[i].byte >> FIELD_GROWTH_CARRY_SHIFT);
        character->equipment_totals[i] = total;
        g_field_game_state->regions[g_field_game_state->region_index].equipment_totals[i] = total;
        /* Pointer form: indexing equipment[1] lets loop.c fold this store into the equipment_totals walker. */
        (character->equipment + 1)->derived.values[i] = character->equipment_totals[i];
        g_field_game_state->regions[g_field_game_state->region_index].total_growth[i].bits.accumulator &= FIELD_GROWTH_ACCUMULATOR_KEEP;
        g_field_game_state->regions[g_field_game_state->region_index].total_growth[i].bits.accumulator +=
            g_field_game_state->regions[g_field_game_state->region_index].total_growth[i].bits.rate;
    }
    character->hp = field_add_stat_increase(character->hp, (u32)(character->stats[FIELD_STAT_HP_GROWTH] & FIELD_STAT_BASE_MASK) >> 2,
                                            FIELD_INCREASE_DAMPED | FIELD_INCREASE_CAPPED);
    g_field_game_state->regions[g_field_game_state->region_index].progress.level = character->progress.level;
    g_field_game_state->regions[g_field_game_state->region_index].hp = character->hp;
    g_field_game_state->regions[g_field_game_state->region_index].status.word &= ~FIELD_REGION_APPLIED_MASK;
    for (i = 0; i < FIELD_REGION_PENDING_EFFECT_COUNT; i++)
    {
        g_field_game_state->regions[g_field_game_state->region_index].status.effects[i] = FIELD_NO_EFFECT;
    }
}

/**
 * @brief Add a stat-derived increase to a value, with optional damping and cap.
 * @param value Starting value.
 * @param increase Amount to add.
 * @param flags FIELD_INCREASE_DAMPED: increases above FIELD_INCREASE_UNDAMPED_MAX add only
 *        half of the excess; FIELD_INCREASE_CAPPED: cap the result at FIELD_INCREASE_CAP.
 * @return The increased value.
 */
s32 field_add_stat_increase(s32 value, s32 increase, s32 flags)
{
    s32 result;

    if ((u32)increase > FIELD_INCREASE_UNDAMPED_MAX && (flags & FIELD_INCREASE_DAMPED))
    {
        result = value + ((u32)(increase - FIELD_INCREASE_UNDAMPED_MAX) >> 1) + FIELD_INCREASE_UNDAMPED_MAX;
    }
    else
    {
        result = value + increase;
    }
    if ((flags & FIELD_INCREASE_CAPPED) && (u32)result > FIELD_INCREASE_CAP)
    {
        result = FIELD_INCREASE_CAP;
    }
    return result;
}
