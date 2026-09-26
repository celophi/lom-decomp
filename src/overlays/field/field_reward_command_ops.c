/**
 * @file field_reward_command_ops.c
 * @brief Defeat drops of monsters and the rewards their pickups grant.
 *
 * The drop handlers are only called through g_field_drop_handlers, a data table.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_effect_types.h"
#include "field_records.h"
#include "sdk/rand.h"

/** @brief Drop handlers of field_roll_defeat_drop. */
#define FIELD_DROP_HANDLER_COUNT 4

/** @brief g_field_drop_slots_by_level has one entry per 16 monster levels. */
#define FIELD_DROP_LEVEL_BAND_SHIFT 4
/** @brief Drop slots added by FIELD_DEFEAT_EXTRA_DROP_SLOTS. */
#define FIELD_DROP_EXTRA_SLOTS 2

/** @brief Monster level above which the experience reward grows at half the rate. */
#define FIELD_REWARD_LEVEL_KNEE 10
/** @brief Experience of a large pickup, in small pickups. */
#define FIELD_REWARD_LARGE_EXPERIENCE_SCALE 10
/** @brief Money of a large and a small money pickup. */
#define FIELD_REWARD_MONEY_LARGE_AMOUNT 50
#define FIELD_REWARD_MONEY_SMALL_AMOUNT 10
/** @brief Share of the maximum restored by a restore pickup, out of 256. */
#define FIELD_REWARD_RESTORE_QUARTER_SHARE 64
#define FIELD_REWARD_RESTORE_HALF_SHARE 128

/** @brief Diagnostic for an unknown reward kind. */
#define DIAG_BAD_REWARD_KIND 300

/** @brief Item pickup codes of a template drop: 16 per monster template. */
#define FIELD_TEMPLATE_PICKUP_CODES 16

/** @brief Drop handler (g_field_drop_handlers): returns the FieldPickupAction the defeated monster leaves. */
typedef s32 (*FieldDropHandler)(s32 handler, s32 value, FieldStatusRecord* record);

FieldActorRecord* field_find_actor_record_or_default(s32 id);

extern FieldDropHandler g_field_drop_handlers[FIELD_DROP_HANDLER_COUNT];
/** @brief Drop slots taking part in the drop roll, per 16 monster levels. */
extern u8 g_field_drop_slots_by_level[];
extern FieldBattleContext* g_field_battle;

/**
 * @brief Roll the drop of a defeated monster and run the drop slot's handler.
 * @param record Status record of the defeated monster, or NULL.
 * @return The FieldPickupAction to leave, or -1 for none.
 * @note Slot n is picked when bit n is the lowest set bit of a random mask (probability 1/2^(n+1)),
 *       and the last slot taking part in the roll when none is.
 */
s32 field_roll_defeat_drop(FieldStatusRecord* record)
{
    s32 slot_count;
    s32 mask;
    s32 random;
    s32 slot;
    FieldActorTemplate* template;

    if (record == NULL)
    {
        return -1;
    }
    slot_count = g_field_drop_slots_by_level[record->state->level.bits.level >> FIELD_DROP_LEVEL_BAND_SHIFT];
    if (record->unkC & FIELD_DEFEAT_EXTRA_DROP_SLOTS)
    {
        slot_count += FIELD_DROP_EXTRA_SLOTS;
    }
    if (slot_count >= FIELD_ACTOR_DROP_SLOT_COUNT)
    {
        slot_count = FIELD_ACTOR_DROP_SLOT_COUNT - 1;
    }
    random = rand();
    mask = random & 0xFFFF;
    if (record->unkC & FIELD_DEFEAT_NO_COMMON_DROPS)
    {
        mask = random & 0xFFFC;
    }
    for (slot = 0; slot < slot_count; slot++)
    {
        if (mask & 1)
        {
            break;
        }
        mask >>= 1;
    }
    template = record->template;
    if (template->drops[slot].handler < FIELD_DROP_HANDLER_COUNT)
    {
        return g_field_drop_handlers[template->drops[slot].handler](template->drops[slot].handler, template->drops[slot].value, record);
    }
    return -1;
}

/**
 * @brief Grant the reward of a pickup.
 * @param recipient Status record id of the party member that picked it up.
 * @param owner Status record id of the actor that left it.
 * @param kind FieldRewardKind.
 */
void field_grant_reward(s32 recipient, s32 owner, u32 kind)
{
    s32 level;

    switch (kind)
    {
    case FIELD_REWARD_EXPERIENCE_LARGE:
        level = g_field_battle->state.level;
        if (level > FIELD_REWARD_LEVEL_KNEE)
        {
            level = ((level - FIELD_REWARD_LEVEL_KNEE) / 2) + FIELD_REWARD_LEVEL_KNEE;
        }
        field_award_experience(recipient, level * FIELD_REWARD_LARGE_EXPERIENCE_SCALE);
        return;
    case FIELD_REWARD_EXPERIENCE_SMALL:
        level = g_field_battle->state.level;
        if (level > FIELD_REWARD_LEVEL_KNEE)
        {
            level = ((level - FIELD_REWARD_LEVEL_KNEE) / 2) + FIELD_REWARD_LEVEL_KNEE;
        }
        field_award_experience(recipient, level);
        return;
    case FIELD_REWARD_MONEY_LARGE:
        field_add_money(recipient, FIELD_REWARD_MONEY_LARGE_AMOUNT);
        return;
    case FIELD_REWARD_MONEY_SMALL:
        field_add_money(recipient, FIELD_REWARD_MONEY_SMALL_AMOUNT);
        return;
    case FIELD_REWARD_ITEM:
        field_grant_actor_pickup((void*)recipient, owner);
        return;
    case FIELD_REWARD_RESTORE_QUARTER:
        field_restore_actor_capacity_fraction(recipient, FIELD_REWARD_RESTORE_QUARTER_SHARE);
        return;
    case FIELD_REWARD_RESTORE_HALF:
        field_restore_actor_capacity_fraction(recipient, FIELD_REWARD_RESTORE_HALF_SHARE);
        return;
    default:
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_REWARD_KIND, owner, kind);
        return;
    }
}

/**
 * @brief Drop handler 0: scatter experience and money pickups.
 * @param handler Drop handler index (unused).
 * @param value Experience pickups in the high nibble, money pickups in the low nibble.
 * @param record Status record of the defeated monster.
 * @return FIELD_PICKUP_EXPERIENCE_OR_CURRENCY.
 */
s32 field_drop_pickups(s32 handler, s32 value, FieldStatusRecord* record)
{
    s32 flags;
    s32 experience;

    experience = value >> 4;
    flags = record->unkC;
    value = value & 0xF;
    if (flags & FIELD_DEFEAT_EXPERIENCE_AS_MONEY)
    {
        value += experience;
        experience = 0;
    }
    if (flags & FIELD_DEFEAT_MONEY_PLUS_2)
    {
        value += 2;
    }
    if (flags & FIELD_DEFEAT_MONEY_PLUS_1)
    {
        value += 1;
    }
    if (flags & FIELD_DEFEAT_EXPERIENCE_PLUS_2)
    {
        experience += 2;
    }
    if (flags & FIELD_DEFEAT_EXPERIENCE_PLUS_1)
    {
        experience += 1;
    }
    if (record->template->flags & FIELD_TEMPLATE_BOSS)
    {
        record->state->signal.drop_counts[FIELD_REWARD_EXPERIENCE_LARGE] = experience;
        record->state->signal.drop_counts[FIELD_REWARD_EXPERIENCE_SMALL] = 0;
        record->state->signal.drop_counts[FIELD_REWARD_MONEY_LARGE] = value;
        record->state->signal.drop_counts[FIELD_REWARD_MONEY_SMALL] = 0;
    }
    else
    {
        record->state->signal.drop_counts[FIELD_REWARD_EXPERIENCE_LARGE] = 0;
        record->state->signal.drop_counts[FIELD_REWARD_EXPERIENCE_SMALL] = experience;
        record->state->signal.drop_counts[FIELD_REWARD_MONEY_LARGE] = 0;
        record->state->signal.drop_counts[FIELD_REWARD_MONEY_SMALL] = value;
    }

    return FIELD_PICKUP_EXPERIENCE_OR_CURRENCY;
}

/**
 * @brief Drop handler 1: leave a restore pickup, a half restore with probability value / 256.
 * @param handler Drop handler index (unused).
 * @param value Chance of the half restore, out of 256.
 * @param record Status record of the defeated monster (unused).
 * @return FIELD_PICKUP_RESTORE_HALF or FIELD_PICKUP_RESTORE_QUARTER.
 */
s32 field_drop_restore(s32 handler, s32 value, FieldStatusRecord* record)
{
    if ((rand() & 0xFF) < value)
    {
        return FIELD_PICKUP_RESTORE_HALF;
    }

    return FIELD_PICKUP_RESTORE_QUARTER;
}

/**
 * @brief Drop handler 2: leave an item pickup for a counter.
 * @param handler Drop handler index (unused).
 * @param value Counter index.
 * @param record Status record of the defeated monster.
 * @return FIELD_PICKUP_ITEM.
 */
s32 field_drop_counter(s32 handler, s32 value, FieldStatusRecord* record)
{
    field_find_actor_record_or_default(record->meta.bytes.id)->pickup = value | FIELD_PICKUP_COUNTER;
    return FIELD_PICKUP_ITEM;
}

/**
 * @brief Drop handler 3: leave an item pickup from the monster template's reward keys.
 * @param handler Drop handler index (unused).
 * @param value Reward key within the template's FIELD_TEMPLATE_PICKUP_CODES.
 * @param record Status record of the defeated monster.
 * @return FIELD_PICKUP_ITEM.
 */
s32 field_drop_template_item(s32 handler, s32 value, FieldStatusRecord* record)
{
    field_find_actor_record_or_default(record->meta.bytes.id)->pickup = value + record->template->id * FIELD_TEMPLATE_PICKUP_CODES;
    return FIELD_PICKUP_ITEM;
}
