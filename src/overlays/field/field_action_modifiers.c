/**
 * @file field_action_modifiers.c
 * @brief Field battle action handlers: damage, status effects, defeat
 *        handling and the attack/defense modifiers they share.
 */

#include "game_audio.h"
#include "common.h"
#include "field_records.h"

/** @brief Script variable: number of party records still standing. */
#define FIELD_VAR_ALLY_COUNT 0x4280

/** @brief Script variable: number of monster records still standing. */
#define FIELD_VAR_ENEMY_COUNT 0x4284

/** @brief Script variable: battle result reported by func_800B65CC. */
#define FIELD_VAR_BATTLE_RESULT 0x4288

/** @brief Script variable: record id watched by func_800B6334 (-1 when none). */
#define FIELD_VAR_WATCHED_RECORD 0x428C

/** @brief Per-record script variable receiving the element mask of an action. */
#define FIELD_VAR_RECORD_ELEMENTS 0xD008

/** @brief Debug flags: log damage, and spare the party or the monsters. */
#define FIELD_VAR_DEBUG_LOG_DAMAGE 0xFFC
#define FIELD_VAR_DEBUG_SPARE_PARTY 0xFFA
#define FIELD_VAR_DEBUG_SPARE_ENEMIES 0xFFB

/** @brief func_800B62D8 results. */
#define FIELD_BATTLE_ONGOING 0
#define FIELD_BATTLE_PARTY_DEFEATED 1
#define FIELD_BATTLE_ENEMIES_DEFEATED 2

/** @brief FieldStatusRecordMeta::packed bit of bits.ally. */
#define FIELD_STATUS_META_ALLY 0x200

/** @brief Number of party records; higher record ids are monsters. */
#define FIELD_PARTY_RECORD_COUNT 3

/** @brief Item id of the revive item consumed by func_800B6744. */
#define FIELD_ITEM_REVIVE 0x58

/** @brief Value of an empty FieldItemRecord::special_ids slot. */
#define FIELD_ITEM_ID_NONE 0xFF

/** @brief Number of handlers in D_800F0B98. */
#define FIELD_ACTION_HANDLER_COUNT 8

/** @brief First and one-past-last status id applied by func_800B78C0. */
#define FIELD_ON_HIT_STATUS_FIRST 0x50
#define FIELD_ON_HIT_STATUS_END 0x60

/** @brief Kind of an action descriptor (low nibble of its first word). */
#define FIELD_DESCRIPTOR_KIND(descriptor) ((descriptor)->info.word & 0xF)

/** @brief Defense slot of an action descriptor (bits 6-7). */
#define FIELD_DESCRIPTOR_DEFENSE_SLOT(descriptor) ((descriptor)->info.bytes.flags >> 6)

/** @brief Handler for one action descriptor kind; returns the damage dealt. */
typedef s32 (*FieldActionHandler)(void);

/** @brief Chance and duration of one on-hit status, indexed from FIELD_ON_HIT_STATUS_FIRST. */
typedef struct FieldOnHitStatus
{
    u8 chance;
    u8 duration;
} FieldOnHitStatus;

extern FieldBattleContext* D_80123FB0;
extern FieldGameState* D_80122B74;

/** @brief Nonzero while the party includes the character of type 2. */
extern s32 D_80122698;

/** @brief Three pointers into resource 9 selected by func_800B661C. */
extern u8* D_801228F8[];

/** @brief Action handlers indexed by FieldActionDescriptor::info.bytes.handler. */
extern FieldActionHandler D_800F0B98[];

/** @brief Resistance slot of each element bit, indexes FieldStatusRecord::unk44. */
extern u8 D_800F0BB8[];

extern FieldOnHitStatus D_800F0BC0[];

/* Unprototyped: called with three arguments here and four in func_800B65CC. */
void func_800B28E0();
s32 func_800B4CE4(FieldStatusRecord* record, s32 status_id);
void func_800B4934(FieldStatusRecord* record);
s32 func_800BD414(s32 owner, s32 variable);
void func_800BD520(s32 owner, s32 variable, s32 value);
u8* func_800C1E40(s32 resource_id);
s32 func_800C0A38(FieldStatusRecord* record);
void func_800C2848(s32 actor_id, s32 value);
s32 func_8008ADB4(s32 record_id);
s32 func_8008AE14(s32 actor_id, s32 animation_id);
s32 func_8008B500(s32 actor_id, s32 signal_id);
s32 func_80089BE8(s32 actor_id, s32 arg1, s32 arg2, s32 arg3, s32 duration);
s32 rand(void);

s32 func_800B62D8(FieldStatusRecord* record);
s32 func_800B6334(FieldStatusRecord* record);
void func_800B65CC(s32 result);
void func_800B6744(FieldStatusRecord* record);
void func_800B70F4(s32 stat_index, s32* attack);
void func_800B7164(s32 stat_index, s32* defense);
void func_800B729C(s32 unused, s32 element_mask, s32* attack, s32* defense);
s32 func_800B742C(u32 attack, u32 defense);
s32 func_800B76F8(s32 damage);
s32 func_800B788C(s32 damage);
void func_800B78C0(void);

/**
 * @brief Store a record id in the watched-record script variable.
 * @param record_id Record id to watch, or -1 for none.
 */
void func_800B61C4(s32 record_id)
{
    func_800BD520(-1, FIELD_VAR_WATCHED_RECORD, record_id);
}

/**
 * @brief Run event 12 in mode 3 on the three party records.
 */
void func_800B61EC(void)
{
    s32 i;

    for (i = 0; i < FIELD_PARTY_RECORD_COUNT; i++)
    {
        func_800B28E0(i, 0xC, 3);
    }
}

/**
 * @brief Check whether the bound action may hit its target.
 * @return -1 when the action applies to the target, otherwise 0.
 */
s32 func_800B622C(void)
{
    s32 opposed;
    s32 side_rule;

    if (D_80123FB0->target->meta.bytes.unk2 & 0x4000)
    {
        return 0;
    }
    /* The u8 truncation happens before the switch in the target. */
    opposed = (u8)(D_80123FB0->attacker->meta.bits.ally ^ D_80123FB0->target->meta.bits.ally);
    side_rule = (D_80123FB0->descriptor->info.word >> 4) & 3;
    switch (side_rule)
    {
    case 0:
        if (opposed != 0)
        {
            return -1;
        }
        return 0;
    case 1:
        if (opposed != 0)
        {
            return -1;
        }
        return 0;
    case 2:
    case 3:
        return -1;
    }
}

/**
 * @brief Report whether the side of a defeated record has no one left.
 * @param record Record that was just defeated.
 * @return FIELD_BATTLE_PARTY_DEFEATED, FIELD_BATTLE_ENEMIES_DEFEATED or
 *         FIELD_BATTLE_ONGOING.
 */
s32 func_800B62D8(FieldStatusRecord* record)
{
    if (record->meta.packed & FIELD_STATUS_META_ALLY)
    {
        if (func_800BD414(0, FIELD_VAR_ALLY_COUNT) == 0)
        {
            return FIELD_BATTLE_PARTY_DEFEATED;
        }
    }
    else
    {
        if (func_800BD414(0, FIELD_VAR_ENEMY_COUNT) == 0)
        {
            return FIELD_BATTLE_ENEMIES_DEFEATED;
        }
    }

    return FIELD_BATTLE_ONGOING;
}

/**
 * @brief Handle a record whose HP reached zero.
 * @param record Defeated record.
 * @return The func_800B62D8 result for the record's side.
 */
s32 func_800B6334(FieldStatusRecord* record)
{
    s32 remaining;
    s32 duration;
    FieldActionDescriptor* descriptor;

    if (record->meta.bytes.id < FIELD_PARTY_RECORD_COUNT)
    {
        if (func_800BD414(0, FIELD_VAR_WATCHED_RECORD) == record->meta.bytes.id)
        {
            func_800BD520(0, FIELD_VAR_WATCHED_RECORD, -1);
        }
        if (record->meta.packed & FIELD_STATUS_META_ALLY)
        {
            remaining = func_800BD414(0, FIELD_VAR_ALLY_COUNT);
        }
        else
        {
            remaining = func_800BD414(0, FIELD_VAR_ENEMY_COUNT);
        }
        remaining--;
        if (remaining == 0 && (record->status_flags & 2))
        {
            func_8008B500(record->meta.bytes.id, 0x2C);
            remaining = 1;
            record->state->current = record->state->maximum;
            func_800B6744(record);
        }
        else
        {
            record->state->effect_flags &= ~0x7FF;
            record->state->effect_flags |= 0x200;
            func_8008AE14(record->meta.bytes.id, -1);
            duration = 900;
            if (record->status_flags & 0x20)
            {
                duration = 450;
            }
            if (func_800B4CE4(record, 0xC) != 0)
            {
                descriptor = D_80123FB0->descriptor;
                if (descriptor != NULL && FIELD_DESCRIPTOR_KIND(descriptor) < 2)
                {
                    duration = 15;
                }
            }
            if (!(record->status_flags & 0x40))
            {
                func_80089BE8(D_80123FB0->action->target_id, 0x1E, 0x2C, -1, duration);
            }
        }
        if (record->meta.packed & FIELD_STATUS_META_ALLY)
        {
            func_800BD520(0, FIELD_VAR_ALLY_COUNT, remaining);
        }
        else
        {
            func_800BD520(0, FIELD_VAR_ENEMY_COUNT, remaining);
        }
    }
    else
    {
        record->state->effect_flags &= ~0x7FF;
        if ((record->meta.bytes.unk2 & 1) || (record->template->flags & 1))
        {
            record->state->effect_flags |= 0x200;
        }
        func_800C2848(record->meta.bytes.id, 0);
        record->meta.bits.active = 0;
        func_8008AE14(record->meta.bytes.id, func_800C0A38(record));
        remaining = func_800BD414(0, FIELD_VAR_ENEMY_COUNT) - 1;
        func_800BD520(0, FIELD_VAR_ENEMY_COUNT, remaining);
    }
    return func_800B62D8(record);
}

/**
 * @brief End the battle with a result and run event 13 on the battle owner.
 * @param result Battle result stored in FIELD_VAR_BATTLE_RESULT.
 */
void func_800B65CC(s32 result)
{
    func_800BD520(0, FIELD_VAR_BATTLE_RESULT, result);
    D_80123FB0->state.flags |= 0x80000000;
    func_800B28E0(0x80, 0xD, 1, D_80123FB0);
}

/**
 * @brief Select the three resource 9 entries for the current party.
 * @note The entry group moves by (character type byte + 1) when the second
 *       party character is of type 2.
 */
void func_800B661C(void)
{
    s32 first;
    s32 i;
    u8* resource;
    u16* offsets;

    D_80122698 = 0;
    first = 0;
    if (D_80122B74->characters[1].name[0] != 0 && (D_80122B74->characters[1].info.word & 0x7F) == 2)
    {
        D_80122698 = 1;
        first = (D_80122B74->characters[1].info.bytes[1] + 1) * 3;
    }

    resource = func_800C1E40(9);
    i = 0;
    if (resource != NULL)
    {
        u8** out;
        out = D_801228F8;
        offsets = (u16*)((first * 2) + (s32)resource);
        do
        {
            *out = resource + (offsets[2] + 4);
            offsets++;
            i++;
            out++;
        } while ((u32)i < 3);
    }
    else
    {
        u8** out;
        out = D_801228F8;
        do
        {
            *out = NULL;
            i++;
            out++;
        } while ((u32)i < 3);
    }
}

/**
 * @brief Make a record both the attacker and the target and settle its defeat.
 * @param record_id Record to look up.
 */
void func_800B66F0(s32 record_id)
{
    FieldStatusRecord* record;
    s32 result;

    record = func_800B2A9C(record_id);
    D_80123FB0->attacker = record;
    D_80123FB0->target = record;
    D_80123FB0->descriptor = NULL;
    if (record != NULL)
    {
        result = func_800B6334(record);
        if (result != 0)
        {
            func_800B65CC(result);
        }
    }
}

/**
 * @brief Consume the first equipped revive item of a party record.
 * @param record Party record that was revived.
 */
void func_800B6744(FieldStatusRecord* record)
{
    s32 i;
    s32 j;
    FieldItemRecord* item;

    for (i = 0; i < FIELD_EQUIPMENT_SLOT_COUNT; i++)
    {
        item = &D_80122B74->characters[record->meta.bytes.id].equipment[i];
        if (item->kind != 0 && (item->effect_index & 2))
        {
            for (j = 0; j < 4; j++)
            {
                if (item->special_ids[j] == FIELD_ITEM_REVIVE)
                {
                    item->special_ids[j] = FIELD_ITEM_ID_NONE;
                    item->effect_index = 0;
                    func_800B4934(record);
                    return;
                }
            }
        }
    }
}

/**
 * @brief Run the handler of the bound action descriptor.
 * @return The handler's damage, or 0 without a descriptor or for an unknown handler.
 */
s32 func_800B6808(void)
{
    FieldActionDescriptor* descriptor;

    descriptor = D_80123FB0->descriptor;
    if (descriptor != NULL)
    {
        if (descriptor->info.bytes.handler < FIELD_ACTION_HANDLER_COUNT)
        {
            return D_800F0B98[descriptor->info.bytes.handler]();
        }
        record_game_diagnostic(0x8001, 0x65, descriptor->info.bytes.handler, D_80123FB0->action->action_id);
        return 0;
    }
    return 0;
}

/**
 * @brief Handler 0: damage, then a status effect on the target.
 * @return Damage dealt.
 */
s32 func_800B6890(void)
{
    s32 attack;
    s32 defense;
    FieldActionParams params;
    FieldActionParams status;
    s32 element_mask;
    s32 damage;

    params = D_80123FB0->descriptor->params;
    /* The status fields are read from a second copy, as in func_800B6B28. */
    status = params;
    func_800B70F4(params.status.attack_stat, &attack);
    func_800B7164(params.status.defense_stat, &defense);
    /* Word shift, not the element_mask bitfield, which schedules the call setup differently. */
    element_mask = (params.word >> 8) & 0xFF;
    func_800B729C(0, element_mask, &attack, &defense);
    damage = func_800B742C(attack, defense);
    if ((D_80123FB0->target->unk39 & element_mask) || (FIELD_DESCRIPTOR_KIND(D_80123FB0->descriptor) != 2))
    {
        func_800B2B54(D_80123FB0->attacker, D_80123FB0->target, 0, status.status.effect, (status.status.chance + 1) * 16, status.status.duration * 16);
    }
    func_800B78C0();
    return damage;
}

/**
 * @brief Handler 1: damage, then drain part of it into the attacker's HP.
 * @return Damage dealt, or 0 when the attacker has no HP left.
 */
s32 func_800B69B0(void)
{
    s32 attack;
    s32 defense;
    FieldActionParams params;
    s32 damage;

    params = D_80123FB0->descriptor->params;
    func_800B70F4(params.roll.attack_stat, &attack);
    func_800B7164(params.roll.defense_stat, &defense);
    func_800B729C(0, 0, &attack, &defense);
    damage = func_800B742C(attack, defense);

    if (D_80123FB0->attacker->state->current == 0)
    {
        return 0;
    }

    if ((D_80123FB0->target->unk39 & params.roll.element_mask) || (FIELD_DESCRIPTOR_KIND(D_80123FB0->descriptor) != 2))
    {
        if (params.roll.spread == 0)
        {
            params.roll.spread = 1;
        }
        saturating_counter_add(D_80123FB0->attacker->state, (u32)(damage * (params.roll.base + rand() % params.roll.spread)) >> 7);
    }

    func_800B78C0();
    return damage;
}

/**
 * @brief Handler 2: damage, then stat changes on the attacker and the target.
 * @return Damage dealt.
 */
s32 func_800B6B28(void)
{
    s32 attack;
    s32 defense;
    FieldActionParams params;
    FieldActionParams changes;
    s32 element_mask;
    s32 damage;

    params = D_80123FB0->descriptor->params;
    /* Same copy and word-shift element mask as func_800B6890. */
    changes = params;
    func_800B70F4(params.stat_change.attack_stat, &attack);
    func_800B7164(params.stat_change.defense_stat, &defense);
    element_mask = (params.word >> 8) & 0xFF;
    func_800B729C(0, element_mask, &attack, &defense);
    damage = func_800B742C(attack, defense);
    if ((D_80123FB0->target->unk39 & element_mask) || (FIELD_DESCRIPTOR_KIND(D_80123FB0->descriptor) != 2))
    {
        func_800B2D64(D_80123FB0->attacker, changes.stat_change.attacker_stat, changes.stat_change.attacker_scale, -1);
        func_800B2D64(D_80123FB0->target, changes.stat_change.target_stat, changes.stat_change.target_scale, -1);
    }
    func_800B78C0();
    return damage;
}

/**
 * @brief Handler 3: damage proportional to the attacker's current HP.
 * @return Damage dealt.
 */
s32 func_800B6C48(void)
{
    s32 attack;
    s32 defense;
    FieldActionParams params;
    s32 roll;
    s32 percent;
    s32 damage;

    params = D_80123FB0->descriptor->params;
    if (params.roll.spread == 0)
    {
        params.roll.spread = 1;
    }

    roll = rand() % params.roll.spread;
    percent = params.roll.base + roll;
    defense = 0;
    attack = (u32)(D_80123FB0->attacker->state->current * percent) >> 4;
    func_800B729C(0, 0, &attack, &defense);
    damage = func_800B742C(attack, defense);
    func_800B78C0();
    return damage;
}

/**
 * @brief Handler 4: damage plus a status effect when the target matches.
 * @return Damage dealt.
 */
s32 func_800B6D3C(void)
{
    FieldActionParams params;
    FieldActionParams unmatched;
    s32 attack;
    s32 defense;
    s32 target_class;
    s32 match;
    s32 damage;

    params = D_80123FB0->descriptor->params;
    target_class = D_80123FB0->target->unk3;
    match = (params.word >> 8) & 0xF; /* word shift, as in func_800B6890 */
    unmatched = params;

    if (target_class == match)
    {
        record_game_diagnostic(0x8003, target_class, target_class, 1);

        /* Net-zero pairs keep params and unmatched in separate registers. */
        params.word++;
        params.word--;
        unmatched.word--;
        unmatched.word++;
        func_800B70F4(params.conditional.attack_stat, &attack);
        if (params.conditional.doubled == 1)
        {
            attack <<= 1;
        }
        func_800B7164(params.conditional.defense_stat, &defense);
        func_800B729C(0, 0, &attack, &defense);
        damage = func_800B742C(attack, defense);

        func_800B2B54(D_80123FB0->attacker, D_80123FB0->target, 0, params.conditional.effect, (params.conditional.chance + 1) << 4,
                      params.conditional.duration << 4);
    }
    else
    {
        unmatched = D_80123FB0->descriptor->params;
        record_game_diagnostic(0x8003, match, target_class, 0);

        func_800B70F4(unmatched.conditional.attack_stat, &attack);
        func_800B7164(unmatched.conditional.defense_stat, &defense);
        func_800B729C(0, 0, &attack, &defense);
        damage = func_800B742C(attack, defense);
    }

    func_800B78C0();
    return damage;
}

/**
 * @brief Handler 5: damage, then modify the target's record.
 * @return Damage dealt.
 */
s32 func_800B6EC0(void)
{
    s32 attack;
    s32 defense;
    FieldActionParams params;
    s32 damage;

    params = D_80123FB0->descriptor->params;
    func_800B70F4(params.modify.attack_stat, &attack);
    func_800B7164(params.modify.defense_stat, &defense);
    func_800B729C(0, 0, &attack, &defense);
    damage = func_800B742C(attack, defense);
    func_800B78C0();
    switch (params.modify.operation)
    {
    case 0:
        D_80123FB0->target->state->status_intensity = 0;
        break;
    case 1:
        D_80123FB0->target->unkC |= params.modify.value << 16;
        break;
    case 2:
        D_80123FB0->target->unkC |= params.modify.value;
        break;
    case 3:
        D_80123FB0->target->unk39 |= params.modify.value;
        break;
    }
    return damage;
}

/**
 * @brief Handler 6: scale the target's current HP by a random percentage.
 */
void func_800B7020(void)
{
    FieldActionParams params;
    FieldStatusState* state;
    s32 roll;
    u32 hp;

    params = D_80123FB0->descriptor->params;
    if (params.roll.spread == 0)
    {
        params.roll.spread = 1;
    }
    roll = rand() % params.roll.spread;
    state = D_80123FB0->target->state;
    hp = params.roll.base + roll;
    hp = (u32)(state->current * hp) >> 4;
    if (hp == 0)
    {
        hp = 1;
    }
    state->current = hp;
}

/**
 * @brief Handler 7: does nothing.
 * @return 0.
 */
s32 func_800B70EC(void)
{
    return 0;
}

/**
 * @brief Compute the attack value from an attacker stat and the action power.
 * @param stat_index Attacker stat.
 * @param attack Receives power * (stat + 50) / 50.
 */
void func_800B70F4(s32 stat_index, s32* attack)
{
    s32 stat;

    stat = func_800B2D34(D_80123FB0->attacker, stat_index);
    *attack = (u32)(D_80123FB0->power * (stat + 50)) / 50;
}

/**
 * @brief Compute the defense value from a target stat and its equipment.
 * @param stat_index Target stat.
 * @param defense Receives the scaled defense, or 0 when the target cannot defend.
 */
void func_800B7164(s32 stat_index, s32* defense)
{
    FieldStatusRecord* target;
    FieldStatusRecord* guard;
    s32 action_id;
    s32 stat;
    u8 slot;

    action_id = func_8008ADB4(D_80123FB0->target->meta.bytes.id);
    stat = func_800B2D34(D_80123FB0->target, stat_index);
    target = D_80123FB0->target;
    if ((target->state->effect_flags & 2) || action_id == 0x31)
    {
        *defense = 0;
    }
    else if (action_id == 10 || action_id == 11)
    {
        D_80123FB0->action_flags.bits.unk0 = 1;
        slot = FIELD_DESCRIPTOR_DEFENSE_SLOT(D_80123FB0->descriptor);
        guard = D_80123FB0->target;
        *defense = guard->equipment_stats[slot] + guard->equipment_attributes[slot];
    }
    else
    {
        slot = FIELD_DESCRIPTOR_DEFENSE_SLOT(D_80123FB0->descriptor);
        *defense = target->equipment_stats[slot];
    }
    *defense = (u32)(*defense * (stat + 50)) / 50;
}

/**
 * @brief Apply the target's element weaknesses and resistances to the attack.
 * @param unused Not read.
 * @param element_mask Elements added to the action's own element flags.
 * @param attack Attack value, scaled by the weaknesses and resistances hit.
 * @param defense Defense value, left unchanged.
 */
void func_800B729C(s32 unused, s32 element_mask, s32* attack, s32* defense)
{
    s32 elements;
    s32 mask;
    s32 sum;
    s32 i;

    elements = element_mask | D_80123FB0->power_flags;
    mask = elements & D_80123FB0->target->unk39;

    if (mask != 0)
    {
        i = 0;
        sum = 0;
        do
        {
            if (mask & 1)
            {
                sum += D_80123FB0->attacker->unk3C[i];
            }
            i++;
            mask >>= 1;
        } while (i < 8);
        *attack = (u32)(*attack * (sum + 5)) >> 2;
    }

    if (func_800B4CE4(D_80123FB0->target, 8) == 0)
    {
        mask = elements & D_80123FB0->target->unk3A;
    }
    else
    {
        mask = elements;
    }

    i = 0;
    if (mask != 0)
    {
        sum = 0;
        do
        {
            if (mask & 1)
            {
                sum += D_80123FB0->target->unk44[D_800F0BB8[i]];
            }
            i++;
            mask >>= 1;
        } while (i < 8);

        if (sum >= 9)
        {
            *attack = (u32)*attack >> 2;
        }
        else
        {
            *attack = (u32)*attack >> 1;
        }
    }

    if (D_80123FB0->target->meta.bytes.id != 0)
    {
        func_800BD520(D_80123FB0->target->meta.bytes.id, FIELD_VAR_RECORD_ELEMENTS, elements);
    }
}

/**
 * @brief Compute the damage of an attack and apply it to the target.
 * @param attack Attack value.
 * @param defense Defense value.
 * @return Damage dealt, or 0 when the target is immune or the action has no power.
 */
s32 func_800B742C(u32 attack, u32 defense)
{
    s32 clamped;
    u32 half_attack;
    s32 bonus;
    u32 value;
    FieldStatusRecord* attacker;
    s32 quarter;

    if (func_800B4CE4(D_80123FB0->target, 0xB) != 0)
    {
        return 0;
    }
    if (D_80123FB0->descriptor->info.bytes.power == 0)
    {
        return 0;
    }

    if ((defense < attack) || (D_80123FB0->action_flags.word & 1))
    {
        attack -= defense >> 1;
        clamped = attack;
        if ((s32)attack < 0)
        {
            clamped = 0;
        }
        attack = clamped;
    }
    else
    {
        value = defense >> 1;
        if (value == 0)
        {
            value = 1;
        }
        half_attack = (attack >> 1) & 0xFFFF;
        attack = (half_attack * half_attack) / value;
    }

    bonus = 0;
    if ((D_80123FB0->attacker->meta.packed & 0xFC00) == 0x1000)
    {
        bonus = func_800BD414(2, 0xD038) * 4;
    }

    attack = (u32)((D_80123FB0->descriptor->info.bytes.power + bonus) * attack) >> 4;
    attack = func_800B76F8(attack * D_80123FB0->action->damage_scale);

    if ((func_800B4CE4(D_80123FB0->attacker, 0xA) != 0) && (FIELD_DESCRIPTOR_KIND(D_80123FB0->descriptor) < 2))
    {
        /* Open-coded attack / 4; value shares a register with the divisor above. */
        attacker = D_80123FB0->attacker;
        if ((s32)attack < 0)
        {
            quarter = attack + 3;
            value = (u32)attacker->state;
        }
        else
        {
            quarter = attack;
            value = (u32)attacker->state;
        }
        saturating_counter_add((FieldStatusState*)value, quarter >> 2);
    }

    attack = func_800B788C(attack);
    if (!(D_80123FB0->action_flags.word & 1) && ((s32)attack <= 0))
    {
        attack = 1;
    }

    if (func_800BD414(0, FIELD_VAR_DEBUG_LOG_DAMAGE) != 0)
    {
        record_game_diagnostic(0x8002, D_80123FB0->attacker->meta.bytes.id, D_80123FB0->target->meta.bytes.id, attack);
    }

    if (((func_800BD414(0, FIELD_VAR_DEBUG_SPARE_PARTY) == 0) || (D_80123FB0->target->meta.bytes.id < FIELD_PARTY_RECORD_COUNT)) &&
        ((func_800BD414(0, FIELD_VAR_DEBUG_SPARE_ENEMIES) == 0) || (D_80123FB0->target->meta.bytes.id >= FIELD_PARTY_RECORD_COUNT)))
    {
        field_clear_record_state(D_80123FB0->target, 6);
        func_800B30B8(D_80123FB0->target->state, attack);
    }

    return attack;
}

/**
 * @brief Apply the attacker's and the target's status multipliers to damage.
 * @param damage Base damage.
 * @return Damage after the multipliers.
 */
s32 func_800B76F8(s32 damage)
{
    s32 count;
    s32 kind;

    count = func_800B4CE4(D_80123FB0->attacker, FIELD_DESCRIPTOR_DEFENSE_SLOT(D_80123FB0->descriptor) | 0x40);
    if (count != 0)
    {
        do
        {
            damage = (damage * 3) / 2;
        } while (--count != 0);
    }

    kind = FIELD_DESCRIPTOR_KIND(D_80123FB0->descriptor);
    count = func_800B4CE4(D_80123FB0->attacker, kind + 0x38);
    if (count != 0)
    {
        do
        {
            damage = (damage * 3) / 2;
        } while (--count != 0);
    }

    kind = FIELD_DESCRIPTOR_KIND(D_80123FB0->descriptor);
    count = func_800B4CE4(D_80123FB0->target, kind + 0x30);
    if (count != 0)
    {
        do
        {
            damage = damage / 2;
        } while (--count != 0);
    }

    count = func_800B4CE4(D_80123FB0->attacker, D_80123FB0->target->unk3 + 0x10);
    if (count != 0)
    {
        do
        {
            damage = (damage * 3) / 2;
        } while (--count != 0);
    }

    count = func_800B4CE4(D_80123FB0->target, D_80123FB0->attacker->unk3 + 0x20);
    if (count != 0)
    {
        do
        {
            damage = damage / 2;
        } while (--count != 0);
    }

    return damage;
}

/**
 * @brief Double the damage while bit 0 of the attacker's unkC flags is set.
 * @param damage Damage so far.
 * @return The possibly doubled damage.
 */
s32 func_800B788C(s32 damage)
{
    s32 result;

    result = damage;
    if (D_80123FB0->attacker->unkC & 1)
    {
        result *= 2;
    }
    return result;
}

/**
 * @brief Pass the attacker's on-hit statuses to the target.
 */
void func_800B78C0(void)
{
    s32 status;
    FieldOnHitStatus* table;
    FieldOnHitStatus* entry;

    status = FIELD_ON_HIT_STATUS_FIRST;
    if (FIELD_DESCRIPTOR_KIND(D_80123FB0->descriptor) != 2)
    {
        do
        {
            if (func_800B4CE4(D_80123FB0->attacker, status) != 0)
            {
                /* Indexing D_800F0BC0 directly folds the -0xA0 bias into the address. */
                table = D_800F0BC0;
                entry = &table[status - FIELD_ON_HIT_STATUS_FIRST];
                func_800B2B54(D_80123FB0->attacker, D_80123FB0->target, 0, status - FIELD_ON_HIT_STATUS_FIRST, entry->chance, entry->duration * 16);
            }
            status += 1;
        } while (status < FIELD_ON_HIT_STATUS_END);
    }
}
