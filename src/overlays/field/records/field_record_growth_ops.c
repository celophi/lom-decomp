/**
 * @file field_record_growth_ops.c
 * @brief Money, experience and level-up growth for party characters and pet records.
 */

#include "main/audio/game_audio.h"
#include "common.h"
#include "../internal/field_calls.h"
#include "../internal/field_records.h"

/** @brief Largest base stat value: 99 in quarter units (FIELD_STAT_BASE_MASK bits of a stat halfword). */
#define FIELD_STAT_BASE_MAX 396

/** @brief FieldStatusRecord::status_flags bit: the recipient shares its experience with the party. */
#define FIELD_RECORD_FLAG_SHARE_EXPERIENCE 0x4

/** @brief Grazing pets get an eighth of the experience awarded to the hero. */
#define FIELD_GRAZING_EXPERIENCE_SHIFT 3

/** @brief Animation resource passed to field_spawn_shared_animation_actor after a level-up. */
#define FIELD_ANIMATION_LEVEL_UP 0x23

/** @brief Stat whose base value (quarter units) sets the hp increase of a level-up. */
#define FIELD_STAT_HP_GROWTH 4

/** @brief Increases up to this amount are never damped. */
#define FIELD_INCREASE_UNDAMPED_MAX 5

/** @brief Largest value of a capped increase. */
#define FIELD_INCREASE_CAP 999

/** @brief Shift of a growth byte's carry: accumulator bit 3, which a level-up adds to its total. */
#define FIELD_GROWTH_CARRY_SHIFT 7

/** @brief Accumulator bits kept once the carry has been added (bits 0-2). */
#define FIELD_GROWTH_ACCUMULATOR_KEEP 7

/** @brief Diagnostic codes of this file. */
#define DIAG_BAD_JOINED_PET 0x79 /**< SavedGameLayout::joined_pet is out of range. */
#define DIAG_BAD_PET_SLOT 0x1F3  /**< A pet level-up names a slot out of range. */

/** @brief Per item type: eight four-bit stat increases applied on a level-up. */
extern u32 g_field_item_type_stat_growth[];

FieldStatusState* field_find_object_state(s32 actor_id);
s32 field_spawn_shared_animation_actor(s32 key, s32 resource_index);

static void field_add_guest_experience(s32 amount);
static void field_award_grazing_pet_experience(u32 amount);
static void field_grow_character_stats(FieldCharacterRecord* character);
static void field_grow_joined_pet_stats(FieldCharacterRecord* character);

/**
 * @brief Add money, saturating at FIELD_MONEY_MAX.
 * @param recipient Reward recipient; only party members are credited.
 * @param amount Money to add.
 */
void field_add_money(s32 recipient, s32 amount)
{
    if (recipient < FIELD_PARTY_SIZE)
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
 * @param record_index Status record id of the recipient; only party members gain experience.
 * @param amount Experience to award.
 * @note Experience awarded to the hero also goes to the grazing pets, and experience awarded
 *       to the guest is also banked for the guest (field_add_guest_experience).
 * @note With FIELD_RECORD_FLAG_SHARE_EXPERIENCE the amount is split evenly (at least 1)
 *       among the party members that are present and alive. On overflow the shared path
 *       clamps the recipient's record, not the member that overflowed.
 */
void field_award_experience(s32 record_index, s32 amount)
{
    s32 eligible_count;
    s32 index;
    FieldStatusRecord* entry;

    if (record_index >= FIELD_PARTY_SIZE)
    {
        return;
    }

    entry = field_find_status_record(record_index);

    if (entry == NULL)
    {
        return;
    }

    switch (entry->meta.bytes.id)
    {
    case FIELD_PARTY_HERO:
        field_award_grazing_pet_experience(amount);
        break;
    case FIELD_PARTY_GUEST:
        field_add_guest_experience(amount);
        break;
    }

    if (entry->status_flags & FIELD_RECORD_FLAG_SHARE_EXPERIENCE)
    {
        for (index = 0, eligible_count = 0; index < FIELD_PARTY_SIZE; index++)
        {
            if ((g_field_game_state->characters[index].name[0] != 0) && (field_find_object_state(index)->current != 0))
            {
                eligible_count++;
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
                    field_add_guest_experience(amount);
                }

                g_field_game_state->characters[index].progress.bits.experience += amount;

                if (g_field_game_state->characters[index].progress.bits.experience > FIELD_EXPERIENCE_MAX)
                {
                    g_field_game_state->characters[record_index].progress.bits.experience = FIELD_EXPERIENCE_MAX;
                }

                field_apply_character_level_ups(index, 1);
            }
        }

        return;
    }

    g_field_game_state->characters[record_index].progress.bits.experience += amount;

    if (g_field_game_state->characters[record_index].progress.bits.experience > FIELD_EXPERIENCE_MAX)
    {
        g_field_game_state->characters[record_index].progress.bits.experience = FIELD_EXPERIENCE_MAX;
    }

    field_apply_character_level_ups(record_index, 1);
}

/**
 * @brief Bank experience for the guest in party slot 1, saturating at FIELD_EXPERIENCE_MAX.
 * @param amount Experience to add.
 * @note The bank is words[FIELD_GUEST_EXPERIENCE_WORD + byte 1 of the guest's info];
 *       field_join_guest adds the same word, indexed by guest id, when the guest rejoins.
 */
static void field_add_guest_experience(s32 amount)
{
    g_field_game_state->words[FIELD_GUEST_EXPERIENCE_WORD + g_field_game_state->characters[FIELD_PARTY_GUEST].info.bytes[1]] += amount;

    if ((u32)g_field_game_state->words[FIELD_GUEST_EXPERIENCE_WORD + g_field_game_state->characters[FIELD_PARTY_GUEST].info.bytes[1]] > FIELD_EXPERIENCE_MAX)
    {
        g_field_game_state->words[FIELD_GUEST_EXPERIENCE_WORD + g_field_game_state->characters[FIELD_PARTY_GUEST].info.bytes[1]] = FIELD_EXPERIENCE_MAX;
    }
}

/**
 * @brief Give an eighth of @p amount as experience to every pet grazing at the ranch.
 * @param amount Experience awarded to the hero.
 */
static void field_award_grazing_pet_experience(u32 amount)
{
    u32 pet_index;

    amount >>= FIELD_GRAZING_EXPERIENCE_SHIFT;

    for (pet_index = 0; pet_index < PET_RECORD_COUNT; pet_index++)
    {
        if ((g_field_game_state->pets[pet_index].name[0] != 0) && g_field_game_state->pets[pet_index].status.bits.grazing)
        {
            g_field_game_state->pets[pet_index].progress.bits.experience += amount;

            if (g_field_game_state->pets[pet_index].progress.bits.experience > FIELD_EXPERIENCE_MAX)
            {
                g_field_game_state->pets[pet_index].progress.bits.experience = FIELD_EXPERIENCE_MAX;
            }
        }
    }
}

/**
 * @brief Apply every level-up a party member's experience allows.
 * @param index Party member index.
 * @param notify Nonzero to play the level-up animation for each level (see field_try_character_level_up).
 */
void field_apply_character_level_ups(s32 index, s32 notify)
{
    while (field_try_character_level_up(index, notify) != 0);
}

/**
 * @brief Apply every level-up a pet record's experience allows.
 * @param slot Pet record index (below PET_RECORD_COUNT).
 * @note Each level adds the stat growth accumulators, rebuilds the effective stat
 *       bits, carries the total accumulators, recomputes hp and clears the
 *       pending effects.
 * @note JP differs only in field_level_threshold().
 */
void field_apply_pet_level_ups(s32 slot)
{
    s32 level;
    s32 i;
    s32 levels_pending;
    u16 stat;
    u32 base;
    u32 base_stat;
    u32 threshold;
    u32 experience;
    PetRecord* pet;

    if (slot >= PET_RECORD_COUNT)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_PET_SLOT, slot, 0);
        return;
    }

    pet = &g_field_game_state->pets[slot];

    if (pet->name[0] == 0)
    {
        return;
    }

    levels_pending = -1;

    while (levels_pending != 0)
    {
        level = pet->progress.level;
        threshold = field_level_threshold(level);
        experience = pet->progress.bits.experience;

        if ((experience != 0) && (experience >= threshold))
        {
            pet->progress.level = level + 1;

            if (pet->progress.level > FIELD_LEVEL_MAX)
            {
                pet->progress.level = FIELD_LEVEL_MAX;
                levels_pending = 0;
            }
            else
            {
                for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
                {
                    stat = pet->stats[i];
                    base = pet->stat_growth[i].bits.accumulator + (stat & FIELD_STAT_BASE_MASK);
                    base &= FIELD_STAT_BASE_MASK;
                    stat = (stat & FIELD_STAT_EFFECTIVE_MASK) | base;
                    pet->stats[i] = stat;

                    if ((stat & FIELD_STAT_BASE_MASK) > FIELD_STAT_BASE_MAX)
                    {
                        pet->stats[i] = (stat & FIELD_STAT_EFFECTIVE_MASK) | FIELD_STAT_BASE_MAX;
                    }

                    base_stat = pet->stats[i] & FIELD_STAT_BASE_MASK;
                    pet->stats[i] = base_stat | ((base_stat >> 2) << FIELD_STAT_EFFECTIVE_SHIFT);
                    pet->stat_growth[i].bits.accumulator = pet->stat_growth[i].bits.rate;
                }

                for (i = 0; i < FIELD_EQUIPMENT_TOTAL_COUNT; i++)
                {
                    pet->equipment_totals[i] += pet->total_growth[i].byte >> FIELD_GROWTH_CARRY_SHIFT;
                    pet->total_growth[i].bits.accumulator &= FIELD_GROWTH_ACCUMULATOR_KEEP;
                    pet->total_growth[i].bits.accumulator += pet->total_growth[i].bits.rate;
                }

                pet->power += pet->extra_growth.bytes[0] >> FIELD_GROWTH_CARRY_SHIFT;
                pet->extra_growth.growth.accumulator &= FIELD_GROWTH_ACCUMULATOR_KEEP;
                pet->extra_growth.growth.accumulator += pet->extra_growth.growth.rate;
                pet->hp = field_add_stat_increase(pet->hp, (pet->stats[FIELD_STAT_HP_GROWTH] & FIELD_STAT_BASE_MASK) >> 2,
                                                  FIELD_INCREASE_DAMPED | FIELD_INCREASE_CAPPED);
                pet->extra_growth.halves[1] += pet->extra_growth.bytes[1];
                pet->status.word &= ~FIELD_REGION_APPLIED_MASK;

                for (i = 0; i < FIELD_REGION_PENDING_EFFECT_COUNT; i++)
                {
                    pet->status.effects[i] = FIELD_NO_EFFECT;
                }
            }
        }
        else
        {
            levels_pending = 0;
        }
    }
}

/**
 * @brief Advance a party member one level when its experience reaches the threshold.
 * @param index Party member index.
 * @param notify Nonzero to play FIELD_ANIMATION_LEVEL_UP after advancing.
 * @return -1 when the member advanced a level, otherwise 0.
 * @note The pet in the party grows through field_grow_joined_pet_stats, the others
 *       through field_grow_character_stats.
 * @note JP differs only in field_level_threshold().
 */
s32 field_try_character_level_up(s32 index, s32 notify)
{
    s32 level;

    level = g_field_game_state->characters[index].progress.level;

    if (field_level_threshold(level) <= g_field_game_state->characters[index].progress.bits.experience)
    {
        g_field_game_state->characters[index].progress.level = level + 1;

        if (g_field_game_state->characters[index].progress.level > FIELD_LEVEL_MAX)
        {
            g_field_game_state->characters[index].progress.level = FIELD_LEVEL_MAX;
            return 0;
        }

        if (g_field_game_state->characters[index].info.bits.type == FIELD_CHARACTER_COMPANION)
        {
            field_grow_joined_pet_stats(&g_field_game_state->characters[index]);
        }
        else
        {
            field_grow_character_stats(&g_field_game_state->characters[index]);
        }

        field_refresh_party_member(index);

        if (notify != 0)
        {
            field_spawn_shared_animation_actor(index, FIELD_ANIMATION_LEVEL_UP);
        }

        return -1;
    }

    return 0;
}

/**
 * @brief Level-up growth of a party member that is not a pet.
 * @param character Party member record.
 * @note Adds the eight four-bit increases of the weapon's item type to the base stats,
 *       clamped to FIELD_STAT_BASE_MAX, then recomputes hp.
 */
static void field_grow_character_stats(FieldCharacterRecord* character)
{
    s32 i;
    u32 deltas;
    u16 stat;
    u32 base;

    deltas = g_field_item_type_stat_growth[character->equipment[FIELD_WEAPON_SLOT].info.bits.item_type];

    for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
    {
        stat = character->stats[i];
        base = (stat & FIELD_STAT_BASE_MASK) + (deltas & 0xF);
        base &= FIELD_STAT_BASE_MASK;
        stat = (stat & FIELD_STAT_EFFECTIVE_MASK) | base;
        character->stats[i] = stat;

        if ((stat & FIELD_STAT_BASE_MASK) > FIELD_STAT_BASE_MAX)
        {
            character->stats[i] = (stat & FIELD_STAT_EFFECTIVE_MASK) | FIELD_STAT_BASE_MAX;
        }

        deltas >>= 4;
    }

    character->hp = field_add_stat_increase(character->hp, (character->stats[FIELD_STAT_HP_GROWTH] & FIELD_STAT_BASE_MASK) >> 2,
                                            FIELD_INCREASE_DAMPED | FIELD_INCREASE_CAPPED);
}

/**
 * @brief Level-up growth of the pet in the party, mirrored into its pet record.
 * @param character Party member record (characters[2]).
 * @note Same algorithm as field_apply_pet_level_ups, applied to the party record and
 *       to pets[joined_pet].
 */
static void field_grow_joined_pet_stats(FieldCharacterRecord* character)
{
    s32 i;
    u16 stat;
    u16 base;
    u16 total;
    u32 slot;

    slot = g_field_game_state->joined_pet;

    if (slot >= PET_RECORD_COUNT)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_JOINED_PET, slot, 0);
    }

    for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
    {
        stat = character->stats[i];
        base = g_field_game_state->pets[g_field_game_state->joined_pet].stat_growth[i].bits.accumulator + (stat & FIELD_STAT_BASE_MASK);
        base &= FIELD_STAT_BASE_MASK;
        stat &= FIELD_STAT_EFFECTIVE_MASK;
        stat |= base;
        character->stats[i] = stat;

        if ((stat & FIELD_STAT_BASE_MASK) > FIELD_STAT_BASE_MAX)
        {
            character->stats[i] = (stat & FIELD_STAT_EFFECTIVE_MASK) | FIELD_STAT_BASE_MAX;
        }

        g_field_game_state->pets[g_field_game_state->joined_pet].stat_growth[i].bits.accumulator =
            g_field_game_state->pets[g_field_game_state->joined_pet].stat_growth[i].bits.rate;
    }

    total = character->power + (g_field_game_state->pets[g_field_game_state->joined_pet].extra_growth.bytes[0] >> FIELD_GROWTH_CARRY_SHIFT);
    character->power = total;
    g_field_game_state->pets[g_field_game_state->joined_pet].power = total;
    character->equipment[FIELD_WEAPON_SLOT].derived.weapon.power = character->power;
    g_field_game_state->pets[g_field_game_state->joined_pet].extra_growth.growth.accumulator &= FIELD_GROWTH_ACCUMULATOR_KEEP;
    g_field_game_state->pets[g_field_game_state->joined_pet].extra_growth.growth.accumulator +=
        g_field_game_state->pets[g_field_game_state->joined_pet].extra_growth.growth.rate;

    for (i = 0; i < FIELD_EQUIPMENT_TOTAL_COUNT; i++)
    {
        total = character->equipment_totals[i] + (g_field_game_state->pets[g_field_game_state->joined_pet].total_growth[i].byte >> FIELD_GROWTH_CARRY_SHIFT);
        character->equipment_totals[i] = total;
        g_field_game_state->pets[g_field_game_state->joined_pet].equipment_totals[i] = total;
        (&character->equipment[FIELD_ARMOR_SLOT])->derived.values[i] = character->equipment_totals[i];
        g_field_game_state->pets[g_field_game_state->joined_pet].total_growth[i].bits.accumulator &= FIELD_GROWTH_ACCUMULATOR_KEEP;
        g_field_game_state->pets[g_field_game_state->joined_pet].total_growth[i].bits.accumulator +=
            g_field_game_state->pets[g_field_game_state->joined_pet].total_growth[i].bits.rate;
    }

    character->hp = field_add_stat_increase(character->hp, (character->stats[FIELD_STAT_HP_GROWTH] & FIELD_STAT_BASE_MASK) >> 2,
                                            FIELD_INCREASE_DAMPED | FIELD_INCREASE_CAPPED);
    g_field_game_state->pets[g_field_game_state->joined_pet].progress.level = character->progress.level;
    g_field_game_state->pets[g_field_game_state->joined_pet].hp = character->hp;
    g_field_game_state->pets[g_field_game_state->joined_pet].status.word &= ~FIELD_REGION_APPLIED_MASK;

    for (i = 0; i < FIELD_REGION_PENDING_EFFECT_COUNT; i++)
    {
        g_field_game_state->pets[g_field_game_state->joined_pet].status.effects[i] = FIELD_NO_EFFECT;
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
s32 field_add_stat_increase(s32 value, u32 increase, s32 flags)
{
    u32 result;

    if (increase > FIELD_INCREASE_UNDAMPED_MAX && (flags & FIELD_INCREASE_DAMPED))
    {
        result = value + ((increase - FIELD_INCREASE_UNDAMPED_MAX) >> 1) + FIELD_INCREASE_UNDAMPED_MAX;
    }
    else
    {
        result = value + increase;
    }

    if ((flags & FIELD_INCREASE_CAPPED) && result > FIELD_INCREASE_CAP)
    {
        result = FIELD_INCREASE_CAP;
    }

    return result;
}
