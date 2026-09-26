/**
 * @file field_action_setup.c
 * @brief Resolve a field battle action: bind its records, check the target's
 *        guard and stance, and apply the outcome.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"
#include "sdk/rand.h"

/** @brief Event slot run on the attacker and the target when an action resolves. */
#define FIELD_ACTION_EVENT 0xC

/** @brief FIELD_ACTION_EVENT modes. */
enum
{
    FIELD_ACTION_EVENT_MISS = 0,     /**< Target: the action missed or was ignored. */
    FIELD_ACTION_EVENT_HURT = 1,     /**< Target: hit and still standing. */
    FIELD_ACTION_EVENT_DEFEATED = 2, /**< Target: hit and defeated. */
    FIELD_ACTION_EVENT_HIT = 4,      /**< Attacker: the hit landed. */
    FIELD_ACTION_EVENT_KILL = 5      /**< Attacker: the hit defeated the target. */
};

/** @brief Results of field_battle_check_target_guard. */
enum
{
    FIELD_GUARD_NONE = 0,
    FIELD_GUARD_IGNORED = 1,
    FIELD_GUARD_BLOCKED = 2
};

/** @brief Results of field_battle_check_target_stance. */
enum
{
    FIELD_STANCE_NONE = 0,
    FIELD_STANCE_WEAPON_GUARD = 1,
    FIELD_STANCE_IMMUNE = 2,
    FIELD_STANCE_REPEL = 3
};

/** @brief Kind of an action descriptor (low nibble of its first word). */
#define FIELD_DESCRIPTOR_KIND_MASK 0xF
#define FIELD_DESCRIPTOR_KIND(descriptor) ((descriptor)->info.word & FIELD_DESCRIPTOR_KIND_MASK)

/** @brief Status intensity shift of an action descriptor (bits 8-10). */
#define FIELD_DESCRIPTOR_SHIFT(descriptor) (((descriptor)->info.word >> 8) & 7)

/** @brief Both defense slot bits of an action descriptor (bits 6-7). */
#define FIELD_DESCRIPTOR_DEFENSE_SLOT_MASK 0xC0

/** @brief FieldStatusRecordMeta::bits.kind as a mask of the packed word. */
#define FIELD_STATUS_META_KIND_MASK 0xFC00
#define FIELD_STATUS_META_KIND_SHIFT 10

/** @brief Guard flags in FieldStatusRecordMeta::bytes.unk2. */
#define FIELD_GUARD_FLAG_DEFLECT 0x1000 /**< Deflects descriptor kinds 2 and 3. */
#define FIELD_GUARD_FLAG_COUNTER 0x2000 /**< Counters descriptor kind 0. */
#define FIELD_GUARD_FLAG_BLOCK 0x4000   /**< Blocks every action. */
#define FIELD_GUARD_FLAG_IGNORE 0x8000  /**< Ignores every action. */

/** @brief FieldStatusRecord::status_flags bits read here; meanings unknown. */
#define FIELD_RECORD_STATUS_10 0x10
#define FIELD_RECORD_STATUS_80 0x80
#define FIELD_RECORD_STATUS_100 0x100

/** @brief FieldStatusState::effect_flags bits read here. */
#define FIELD_EFFECT_NO_INTENSITY 0x200    /**< The attacker's intensity does not rise. */
#define FIELD_EFFECT_HOLD_MODIFIERS 0x8000 /**< The held action modifiers survive the action. */

/** @brief Status slot ids tested with field_count_status_slots. */
#define FIELD_SLOT_MONEY_PLUS_2 3       /**< Attacker adds FIELD_DEFEAT_MONEY_PLUS_2 to a defeated target. */
#define FIELD_SLOT_POWER_BOOST 4        /**< Kinds 0 and 1 hit with 1.5 times the power. */
#define FIELD_SLOT_QUICK_INTENSITY 7    /**< Attacker's intensity rises twice as fast. */
#define FIELD_SLOT_RARE_DROPS 9         /**< Attacker adds FIELD_DEFEAT_EXTRA_DROP_SLOTS and _NO_COMMON_DROPS. */
#define FIELD_SLOT_EXPERIENCE_PLUS_2 13 /**< Attacker adds FIELD_DEFEAT_EXPERIENCE_PLUS_2 to a defeated target. */
#define FIELD_SLOT_REPEL_KIND_4 0x34    /**< Target repels descriptor kind 4. */
#define FIELD_SLOT_REPEL_KIND_5 0x35    /**< Target repels descriptor kind 5. */

/** @brief Target animations (field_get_actor_animation) that change how an action lands. */
#define FIELD_ANIM_WEAPON_GUARD 0x22 /**< Kind 1 fails against a spear or staff. */
#define FIELD_ANIM_IMMUNE 0x3B       /**< Kind 1 always fails. */
#define FIELD_ANIM_REPEL_A 10        /**< Kind 4 is repelled with FIELD_RECORD_STATUS_10. */
#define FIELD_ANIM_REPEL_B 11

/**
 * @brief Weapon types (FIELD_ITEM_TYPE) that guard in FIELD_ANIM_WEAPON_GUARD.
 * @note Names assume the in-game weapon order (knife, sword, axe, ...).
 */
#define FIELD_WEAPON_SPEAR 6
#define FIELD_WEAPON_STAFF 7

/** @brief Effect animation (field_spawn_shared_animation_actor) of a target that repels an action. */
#define FIELD_EFFECT_ANIM_REPEL 0x90

/** @brief Status effect put on an attacker by a countering target. */
#define FIELD_COUNTER_EFFECT 4

/** @brief Status effect a countering target drops. */
#define FIELD_COUNTER_CLEARED_EFFECT 6

/** @brief Status effect put on a record whose action counter runs out. */
#define FIELD_EXHAUSTED_EFFECT 5

/** @brief field_apply_status_effect chance threshold that always passes the 8-bit roll. */
#define FIELD_CHANCE_ALWAYS 0x100

/** @brief Highest status intensity. */
#define FIELD_INTENSITY_MAX 255

/** @brief Evasion that always evades. */
#define FIELD_EVASION_ALWAYS 100

/** @brief Record ids below this are the player characters. */
#define FIELD_PLAYER_RECORD_COUNT 2

extern FieldBattleContext* g_field_battle;
extern FieldGameState* g_field_game_state;

FieldActionDescriptor* field_select_action_descriptor(void);
s32 field_get_actor_animation(s32 key);
s32 field_register_actor_hit(s32 key, s32 reaction);
s32 field_spawn_shared_animation_actor(s32 key, s32 resource_index);

static void field_battle_bind_action(FieldBattleAction* action, s32 resolve_descriptor);
static s32 field_battle_check_target_stance(void);
static s32 field_battle_check_target_guard(void);
static void field_battle_run_down_counters(s32 amount);
static void field_battle_raise_attacker_intensity(void);

/**
 * @brief Resolve an action and apply its outcome to the attacker and the target.
 * @param action Action to resolve.
 * @return 1 when there is nothing to resolve or the target repels the action,
 *         the guard or stance result when one stops it, otherwise 0.
 */
s32 field_battle_resolve_action(FieldBattleAction* action)
{
    s32 result;
    s32 effect;

    if (g_field_battle == NULL)
    {
        return 1;
    }
    if (g_field_battle->state.flags < 0)
    {
        return 1;
    }

    field_battle_bind_action(action, -1);
    if (g_field_battle->descriptor == NULL)
    {
        return 1;
    }

    result = field_battle_check_target_guard();
    if (result != FIELD_GUARD_NONE)
    {
        if (result == FIELD_GUARD_IGNORED)
        {
            field_run_actor_event(action->target_id, FIELD_ACTION_EVENT, FIELD_ACTION_EVENT_MISS);
        }
        g_field_battle->attacker->unkC &= ~FIELD_RECORD_ACTION_MODIFIERS;
        return result;
    }

    result = field_battle_check_target_stance();
    if (result != FIELD_STANCE_NONE)
    {
        if (result == FIELD_STANCE_REPEL)
        {
            field_spawn_shared_animation_actor(action->target_id, FIELD_EFFECT_ANIM_REPEL);
            return 1;
        }
        g_field_battle->attacker->unkC &= ~FIELD_RECORD_ACTION_MODIFIERS;
        return result;
    }

    if (!(g_field_battle->attacker->state->effect_flags & FIELD_EFFECT_HOLD_MODIFIERS))
    {
        g_field_battle->attacker->unkC &= ~FIELD_RECORD_HELD_MODIFIERS;
    }
    if (field_run_action_handler() != 0)
    {
        field_battle_raise_attacker_intensity();
        if (g_field_battle->target->state->current == 0)
        {
            if (!g_field_battle->action_flags.bits.follow_up)
            {
                field_run_actor_event(action->attacker_id, FIELD_ACTION_EVENT, FIELD_ACTION_EVENT_KILL);
            }
            field_run_actor_event(action->target_id, FIELD_ACTION_EVENT, FIELD_ACTION_EVENT_DEFEATED);
            if (field_count_status_slots(g_field_battle->attacker, FIELD_SLOT_MONEY_PLUS_2) != 0)
            {
                g_field_battle->target->unkC |= FIELD_DEFEAT_MONEY_PLUS_2;
            }
            if (field_count_status_slots(g_field_battle->attacker, FIELD_SLOT_EXPERIENCE_PLUS_2) != 0)
            {
                g_field_battle->target->unkC |= FIELD_DEFEAT_EXPERIENCE_PLUS_2;
            }
            if (field_count_status_slots(g_field_battle->attacker, FIELD_SLOT_RARE_DROPS) != 0)
            {
                g_field_battle->target->unkC |= FIELD_DEFEAT_EXTRA_DROP_SLOTS | FIELD_DEFEAT_NO_COMMON_DROPS;
            }
            if (field_roll_last_stat(g_field_battle->attacker) != 0)
            {
                g_field_battle->target->unkC |= FIELD_DEFEAT_NO_COMMON_DROPS;
            }
            if (g_field_battle->attacker->status_flags & FIELD_RECORD_STATUS_100)
            {
                g_field_battle->target->unkC |= FIELD_DEFEAT_MONEY_PLUS_1;
            }
            if (g_field_battle->attacker->status_flags & FIELD_RECORD_STATUS_80)
            {
                g_field_battle->target->unkC |= FIELD_DEFEAT_EXPERIENCE_PLUS_1;
            }
            effect = field_battle_handle_defeat(g_field_battle->target);
            if (effect != 0)
            {
                field_battle_finish(effect);
            }
        }
        else
        {
            field_battle_run_down_counters(1);
            field_run_actor_event(action->attacker_id, FIELD_ACTION_EVENT, FIELD_ACTION_EVENT_HIT);
            field_run_actor_event(action->target_id, FIELD_ACTION_EVENT, FIELD_ACTION_EVENT_HURT);
            g_field_battle->target->unkC &= ~FIELD_RECORD_DEFEAT_FLAGS;
            field_register_actor_hit(action->target_id, 0);
        }
        g_field_battle->attacker->unkC &= ~FIELD_RECORD_ACTION_MODIFIERS;
        return 0;
    }
    else
    {
        field_battle_run_down_counters(0);
        field_run_actor_event(action->target_id, FIELD_ACTION_EVENT, FIELD_ACTION_EVENT_MISS);
        g_field_battle->target->unkC &= ~FIELD_RECORD_DEFEAT_FLAGS;
        g_field_battle->attacker->unkC &= ~FIELD_RECORD_ACTION_MODIFIERS;
        return 0;
    }
}

/**
 * @brief Bind an action to the battle context and look up its records.
 * @param action Action to bind, or NULL to clear the binding.
 * @param resolve_descriptor Nonzero to select the action descriptor as well.
 */
static void field_battle_bind_action(FieldBattleAction* action, s32 resolve_descriptor)
{
    FieldActionDescriptor* descriptor;
    u32 info;

    g_field_battle->action = action;
    if (action == NULL)
    {
        record_game_diagnostic(0x8001, (s32)field_battle_bind_action, 0, 0);
        return;
    }
    g_field_battle->attacker = field_find_status_record(action->attacker_id);
    g_field_battle->target = field_find_status_record(action->target_id);
    g_field_battle->action_flags.word = 0;
    g_field_battle->action_flags.bits.side = field_is_actor_in_front(action->attacker_id, action->target_id);
    if (resolve_descriptor != 0)
    {
        g_field_battle->descriptor = field_select_action_descriptor();
        if (field_count_status_slots(g_field_battle->attacker, FIELD_SLOT_POWER_BOOST) != 0)
        {
            descriptor = g_field_battle->descriptor;
            info = descriptor->info.word;
            if ((info & FIELD_DESCRIPTOR_KIND_MASK) < 2)
            {
                descriptor->info.word = info | FIELD_DESCRIPTOR_DEFENSE_SLOT_MASK;
                g_field_battle->power = (g_field_battle->power * 3) >> 1;
            }
        }
    }
    else
    {
        g_field_battle->descriptor = NULL;
    }
}

/**
 * @brief Check the target's animation and status against the bound action.
 * @return A FIELD_STANCE_* result; FIELD_STANCE_REPEL also marks a follow-up.
 */
static s32 field_battle_check_target_stance(void)
{
    s32 animation;
    u32 kind;
    FieldStatusRecord* target;

    animation = field_get_actor_animation(g_field_battle->target->meta.bytes.id);
    switch (FIELD_DESCRIPTOR_KIND(g_field_battle->descriptor))
    {
    case 1:
        if (animation == FIELD_ANIM_IMMUNE)
        {
            return FIELD_STANCE_IMMUNE;
        }
        if (animation == FIELD_ANIM_WEAPON_GUARD)
        {
            target = g_field_battle->target;
            kind = target->meta.packed & FIELD_STATUS_META_KIND_MASK;
            if (kind == (0 << FIELD_STATUS_META_KIND_SHIFT) || kind == (1 << FIELD_STATUS_META_KIND_SHIFT))
            {
                u32 weapon_type =
                    FIELD_ITEM_TYPE(g_field_game_state->characters[target->meta.bytes.id].equipment[FIELD_WEAPON_SLOT].info.word);

                if (weapon_type == FIELD_WEAPON_SPEAR || weapon_type == FIELD_WEAPON_STAFF)
                {
                    return FIELD_STANCE_WEAPON_GUARD;
                }
            }
        }
        break;
    case 4:
        if (g_field_battle->action_flags.bits.side)
        {
            return FIELD_STANCE_IMMUNE;
        }
        if ((g_field_battle->target->status_flags & FIELD_RECORD_STATUS_10) &&
            (animation == FIELD_ANIM_REPEL_A || animation == FIELD_ANIM_REPEL_B))
        {
            g_field_battle->action_flags.bits.follow_up = 1;
            return FIELD_STANCE_REPEL;
        }
        if (field_count_status_slots(g_field_battle->target, FIELD_SLOT_REPEL_KIND_4))
        {
            g_field_battle->action_flags.bits.follow_up = 1;
            return FIELD_STANCE_REPEL;
        }
        break;
    case 5:
        if (field_count_status_slots(g_field_battle->target, FIELD_SLOT_REPEL_KIND_5))
        {
            g_field_battle->action_flags.bits.follow_up = 1;
            return FIELD_STANCE_REPEL;
        }
        break;
    case 2:
    case 3:
    case 6:
    default:
        break;
    }
    return FIELD_STANCE_NONE;
}

/**
 * @brief Check the target's guard flags before the action lands.
 * @return A FIELD_GUARD_* result.
 */
static s32 field_battle_check_target_guard(void)
{
    u16 flags;

    flags = g_field_battle->target->meta.bytes.unk2;
    if (flags & FIELD_GUARD_FLAG_IGNORE)
    {
        return FIELD_GUARD_IGNORED;
    }
    if (flags & FIELD_GUARD_FLAG_BLOCK)
    {
        return FIELD_GUARD_BLOCKED;
    }
    if (flags & FIELD_GUARD_FLAG_COUNTER)
    {
        if (FIELD_DESCRIPTOR_KIND(g_field_battle->descriptor) == 0)
        {
            field_apply_status_effect(g_field_battle->target, g_field_battle->attacker,
                          FIELD_STATUS_APPLY_IGNORE_IMMUNITY | FIELD_STATUS_APPLY_ALLOW_ACTIVE, FIELD_COUNTER_EFFECT,
                          FIELD_CHANCE_ALWAYS, 300);
            field_clear_status_effect(g_field_battle->target, FIELD_COUNTER_CLEARED_EFFECT);
            return FIELD_GUARD_BLOCKED;
        }
    }
    if (g_field_battle->target->meta.bytes.unk2 & FIELD_GUARD_FLAG_DEFLECT)
    {
        u32 kind = FIELD_DESCRIPTOR_KIND(g_field_battle->descriptor);

        if (kind == 2 || kind == 3)
        {
            field_spawn_shared_animation_actor(g_field_battle->target->meta.bytes.id, FIELD_EFFECT_ANIM_REPEL);
            return FIELD_GUARD_BLOCKED;
        }
    }
    return FIELD_GUARD_NONE;
}

/**
 * @brief Run down the attacker's and the target's action counters.
 * @param amount Amount taken from the target's counter.
 */
static void field_battle_run_down_counters(s32 amount)
{
    FieldStatusRecord* attacker;
    FieldStatusRecord* target;

    attacker = g_field_battle->attacker;
    if (attacker->counter <= 0)
    {
        attacker->counter = attacker->counter_reset;
        field_apply_status_effect(g_field_battle->attacker, g_field_battle->attacker,
                      FIELD_STATUS_APPLY_IGNORE_IMMUNITY | FIELD_STATUS_APPLY_ALLOW_ACTIVE, FIELD_EXHAUSTED_EFFECT,
                      FIELD_CHANCE_ALWAYS, 60);
    }
    if (g_field_battle->action->param != 0)
    {
        g_field_battle->target->counter -= amount;
        target = g_field_battle->target;
        if (target->counter <= 0)
        {
            target->counter = target->counter_reset;
            field_apply_status_effect(g_field_battle->target, g_field_battle->target,
                          FIELD_STATUS_APPLY_IGNORE_IMMUNITY | FIELD_STATUS_APPLY_ALLOW_ACTIVE, FIELD_EXHAUSTED_EFFECT,
                          FIELD_CHANCE_ALWAYS, 180);
        }
    }
}

/**
 * @brief Raise a player attacker's status intensity after a landed action.
 */
static void field_battle_raise_attacker_intensity(void)
{
    FieldStatusRecord* attacker;
    FieldStatusState* state;

    attacker = g_field_battle->attacker;
    if (attacker->meta.bytes.id < FIELD_PLAYER_RECORD_COUNT && !(attacker->state->effect_flags & FIELD_EFFECT_NO_INTENSITY))
    {
        if (field_count_status_slots(attacker, FIELD_SLOT_QUICK_INTENSITY) != 0)
        {
            g_field_battle->attacker->state->status_intensity += 8 << FIELD_DESCRIPTOR_SHIFT(g_field_battle->descriptor);
        }
        else
        {
            g_field_battle->attacker->state->status_intensity += 4 << FIELD_DESCRIPTOR_SHIFT(g_field_battle->descriptor);
        }
        state = g_field_battle->attacker->state;
        if (state->status_intensity > FIELD_INTENSITY_MAX)
        {
            state->status_intensity = FIELD_INTENSITY_MAX;
        }
    }
}

/**
 * @brief Roll whether the target evades an action.
 * @param action Action to bind before the roll.
 * @return -1 when the target always evades or the roll succeeds, otherwise 0.
 */
s32 field_battle_roll_evasion(FieldBattleAction* action)
{
    s32 roll;
    s32 percent;
    s32 chance;
    s32 evasion;

    evasion = g_field_battle->target->unk1A;
    if (evasion >= FIELD_EVASION_ALWAYS)
    {
        return -1;
    }

    roll = rand() & 0xFFFF;
    percent = roll % 100;
    field_battle_bind_action(action, 0);
    chance = field_get_status_stat(g_field_battle->attacker, 0);
    if (percent < (evasion * chance) / field_get_status_stat(g_field_battle->target, 4))
    {
        return -1;
    }

    return 0;
}
