/**
 * @file field_action_setup.c
 * @brief Resolve a field battle action: bind its records, classify it and
 *        apply its outcome.
 */

#include "game_audio.h"
#include "common.h"
#include "field_records.h"

/** @brief Event slot run on the attacker and the target when an action resolves. */
#define FIELD_ACTION_EVENT 0xC

/** @brief Signal sent to an actor that dodges or cancels an action. */
#define FIELD_ACTION_SIGNAL_CANCEL 0x90

/** @brief Kind of an action descriptor (low nibble of its first word). */
#define FIELD_DESCRIPTOR_KIND(descriptor) ((descriptor)->info.word & 0xF)

/** @brief Status intensity shift of an action descriptor (bits 8-10). */
#define FIELD_DESCRIPTOR_SHIFT(descriptor) (((descriptor)->info.word >> 8) & 7)

/** @brief Action id of the move that is always handled immediately. */
#define FIELD_ACTION_IMMEDIATE 0x3B

/** @brief Action id of the move that checks the attacker's weapon type. */
#define FIELD_ACTION_WEAPON_CHECK 0x22

extern FieldBattleContext* D_80123FB0;
extern FieldGameState* D_80122B74;

s32 func_800B4CE4(FieldStatusRecord* record, s32 status_id);
FieldActionDescriptor* func_800B50B8(void);
s32 func_800B28E0(s32 owner_id, s32 event_id, s32 mode);
s32 func_8008ADB4(s32 record_id);
void func_8008AB2C(s32 actor_id, s32 value);
void func_8008B500(s32 actor_id, s32 signal_id);
s32 func_800B6334(FieldStatusRecord* target);
void func_800B65CC(s32 effect);
s32 func_800B6808(void);
s32 rand(void);

void func_800B5948(FieldBattleAction* action, s32 resolve_descriptor);
s32 func_800B5A88(void);
s32 func_800B5C54(void);
void func_800B5D60(s32 amount);
void func_800B5E5C(void);

/**
 * @brief Resolve an action and apply its outcome to the attacker and the target.
 * @param action Action to resolve.
 * @return 1 when there is nothing to resolve or the action is cancelled, the
 *         nonzero result of func_800B5C54 or func_800B5A88, otherwise 0.
 */
s32 func_800B5534(FieldBattleAction* action)
{
    s32 result;
    s32 effect;

    if (D_80123FB0 == NULL)
    {
        return 1;
    }
    if (D_80123FB0->state.flags < 0)
    {
        return 1;
    }

    func_800B5948(action, -1);
    if (D_80123FB0->descriptor == NULL)
    {
        return 1;
    }

    result = func_800B5C54();
    if (result != 0)
    {
        if (result == 1)
        {
            func_800B28E0(action->target_id, FIELD_ACTION_EVENT, 0);
        }
        D_80123FB0->attacker->unkC &= ~0xFF;
        return result;
    }

    result = func_800B5A88();
    if (result != 0)
    {
        if (result == 3)
        {
            func_8008B500(action->target_id, FIELD_ACTION_SIGNAL_CANCEL);
            return 1;
        }
        D_80123FB0->attacker->unkC &= ~0xFF;
        return result;
    }

    if (!(D_80123FB0->attacker->state->effect_flags & 0x8000))
    {
        D_80123FB0->attacker->unkC &= 0xFFFF00FF;
    }
    if (func_800B6808() != 0)
    {
        func_800B5E5C();
        if (D_80123FB0->target->state->current == 0)
        {
            if (!D_80123FB0->action_flags.bits.follow_up)
            {
                func_800B28E0(action->attacker_id, FIELD_ACTION_EVENT, 5);
            }
            func_800B28E0(action->target_id, FIELD_ACTION_EVENT, 2);
            if (func_800B4CE4(D_80123FB0->attacker, 3) != 0)
            {
                D_80123FB0->target->unkC |= 0x20000000;
            }
            if (func_800B4CE4(D_80123FB0->attacker, 0xD) != 0)
            {
                D_80123FB0->target->unkC |= 0x02000000;
            }
            if (func_800B4CE4(D_80123FB0->attacker, 9) != 0)
            {
                D_80123FB0->target->unkC |= 0x0C000000;
            }
            if (func_800B2FF8(D_80123FB0->attacker) != 0)
            {
                D_80123FB0->target->unkC |= 0x08000000;
            }
            if (D_80123FB0->attacker->status_flags & 0x100)
            {
                D_80123FB0->target->unkC |= 0x10000000;
            }
            if (D_80123FB0->attacker->status_flags & 0x80)
            {
                D_80123FB0->target->unkC |= 0x01000000;
            }
            effect = func_800B6334(D_80123FB0->target);
            if (effect != 0)
            {
                func_800B65CC(effect);
            }
        }
        else
        {
            func_800B5D60(1);
            func_800B28E0(action->attacker_id, FIELD_ACTION_EVENT, 4);
            func_800B28E0(action->target_id, FIELD_ACTION_EVENT, 1);
            D_80123FB0->target->unkC &= 0xFFFFFF;
            func_8008AB2C(action->target_id, 0);
        }
        D_80123FB0->attacker->unkC &= ~0xFF;
        return 0;
    }
    else
    {
        func_800B5D60(0);
        func_800B28E0(action->target_id, FIELD_ACTION_EVENT, 0);
        D_80123FB0->target->unkC &= 0xFFFFFF;
        D_80123FB0->attacker->unkC &= ~0xFF;
        return 0;
    }
}

/**
 * @brief Bind an action to the battle context and look up its records.
 * @param action Action to bind, or NULL to clear the binding.
 * @param resolve_descriptor Nonzero to select the action descriptor as well.
 */
void func_800B5948(FieldBattleAction* action, s32 resolve_descriptor)
{
    FieldActionDescriptor* descriptor;
    u32 info;

    D_80123FB0->action = action;
    if (action == NULL)
    {
        record_game_diagnostic(0x8001, (s32)func_800B5948, 0, 0);
        return;
    }
    D_80123FB0->attacker = func_800B2A9C(action->attacker_id);
    D_80123FB0->target = func_800B2A9C(action->target_id);
    D_80123FB0->action_flags.word = 0;
    D_80123FB0->action_flags.bits.side = func_800B302C(action->attacker_id, action->target_id);
    if (resolve_descriptor != 0)
    {
        D_80123FB0->descriptor = func_800B50B8();
        if (func_800B4CE4(D_80123FB0->attacker, 4) != 0)
        {
            descriptor = D_80123FB0->descriptor;
            info = descriptor->info.word;
            if ((info & 0xF) < 2)
            {
                descriptor->info.word = info | 0xC0;
                D_80123FB0->power = (D_80123FB0->power * 3) >> 1;
            }
        }
    }
    else
    {
        D_80123FB0->descriptor = NULL;
    }
}

/**
 * @brief Classify the bound action and mark follow-up actions.
 * @return 0 for a normal action, 1 when the weapon check passes, 2 for
 *         immediate handling, or 3 when a follow-up action was marked.
 */
s32 func_800B5A88(void)
{
    s32 action_id;
    u32 kind;
    FieldStatusRecord* target;

    action_id = func_8008ADB4(D_80123FB0->target->meta.bytes.id);
    switch (FIELD_DESCRIPTOR_KIND(D_80123FB0->descriptor))
    {
    case 1:
        if (action_id == FIELD_ACTION_IMMEDIATE)
        {
            return 2;
        }
        if (action_id == FIELD_ACTION_WEAPON_CHECK)
        {
            target = D_80123FB0->target;
            kind = target->meta.packed & 0xFC00;
            if (kind == 0 || kind == 0x400)
            {
                u32 weapon_type = FIELD_ITEM_TYPE(D_80122B74->characters[target->meta.bytes.id].equipment[0].info.word);

                if (weapon_type == 6 || weapon_type == 7)
                {
                    return 1;
                }
            }
        }
        break;
    case 4:
        if (D_80123FB0->action_flags.bits.side)
        {
            return 2;
        }
        if ((D_80123FB0->target->status_flags & 0x10) && (action_id == 10 || action_id == 11))
        {
            D_80123FB0->action_flags.bits.follow_up = 1;
            return 3;
        }
        if (func_800B4CE4(D_80123FB0->target, 0x34))
        {
            D_80123FB0->action_flags.bits.follow_up = 1;
            return 3;
        }
        break;
    case 5:
        if (func_800B4CE4(D_80123FB0->target, 0x35))
        {
            D_80123FB0->action_flags.bits.follow_up = 1;
            return 3;
        }
        break;
    case 2:
    case 3:
    case 6:
    default:
        break;
    }
    return 0;
}

/**
 * @brief Check the target's guard flags before the action lands.
 * @return 1 when the target's 0x8000 flag is set, 2 when the action is
 *         blocked or deflected, otherwise 0.
 */
s32 func_800B5C54(void)
{
    u16 flags;

    flags = D_80123FB0->target->meta.bytes.unk2;
    if (flags & 0x8000)
    {
        return 1;
    }
    if (flags & 0x4000)
    {
        return 2;
    }
    if (flags & 0x2000)
    {
        if (FIELD_DESCRIPTOR_KIND(D_80123FB0->descriptor) == 0)
        {
            func_800B2B54(D_80123FB0->target, D_80123FB0->attacker, 3, 4, 0x100, 300);
            field_clear_record_state(D_80123FB0->target, 6);
            return 2;
        }
    }
    if (D_80123FB0->target->meta.bytes.unk2 & 0x1000)
    {
        u32 kind = FIELD_DESCRIPTOR_KIND(D_80123FB0->descriptor);

        if (kind == 2 || kind == 3)
        {
            func_8008B500(D_80123FB0->target->meta.bytes.id, FIELD_ACTION_SIGNAL_CANCEL);
            return 2;
        }
    }
    return 0;
}

/**
 * @brief Run down the attacker's and the target's action counters.
 * @param amount Amount taken from the target's counter.
 */
void func_800B5D60(s32 amount)
{
    FieldStatusRecord* attacker;
    FieldStatusRecord* target;

    attacker = D_80123FB0->attacker;
    if (attacker->counter <= 0)
    {
        attacker->counter = attacker->counter_reset;
        func_800B2B54(D_80123FB0->attacker, D_80123FB0->attacker, 3, 5, 0x100, 60);
    }
    if (D_80123FB0->action->param != 0)
    {
        D_80123FB0->target->counter -= amount;
        target = D_80123FB0->target;
        if (target->counter <= 0)
        {
            target->counter = target->counter_reset;
            func_800B2B54(D_80123FB0->target, D_80123FB0->target, 3, 5, 0x100, 180);
        }
    }
}

/**
 * @brief Raise a party attacker's status intensity after a landed action.
 */
void func_800B5E5C(void)
{
    FieldStatusRecord* attacker;
    FieldStatusState* state;

    attacker = D_80123FB0->attacker;
    if (attacker->meta.bytes.id < 2 && !(attacker->state->effect_flags & 0x200))
    {
        if (func_800B4CE4(attacker, 7) != 0)
        {
            D_80123FB0->attacker->state->status_intensity += 8 << FIELD_DESCRIPTOR_SHIFT(D_80123FB0->descriptor);
        }
        else
        {
            D_80123FB0->attacker->state->status_intensity += 4 << FIELD_DESCRIPTOR_SHIFT(D_80123FB0->descriptor);
        }
        state = D_80123FB0->attacker->state;
        if (state->status_intensity >= 0x100)
        {
            state->status_intensity = 0xFF;
        }
    }
}

/**
 * @brief Roll whether the target evades an action.
 * @param action Action to bind before the roll.
 * @return -1 when the target always evades or the roll succeeds, otherwise 0.
 */
s32 func_800B5F60(FieldBattleAction* action)
{
    s32 roll;
    s32 percent;
    s32 chance;
    s32 evasion;

    evasion = D_80123FB0->target->unk1A;
    if (evasion >= 100)
    {
        return -1;
    }

    roll = rand() & 0xFFFF;
    percent = roll % 100;
    func_800B5948(action, 0);
    chance = func_800B2D34(D_80123FB0->attacker, 0);
    if (percent < (evasion * chance) / func_800B2D34(D_80123FB0->target, 4))
    {
        return -1;
    }

    return 0;
}
