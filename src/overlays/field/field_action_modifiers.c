/**
 * @file field_action_modifiers.c
 * @brief Field battle action handlers: damage, status effects, defeat
 *        handling and the attack/defense modifiers they share.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_script.h"
#include "field_records.h"
#include "field_actor.h"


/** @brief field_battle_side_defeated results. */
#define FIELD_BATTLE_ONGOING 0
#define FIELD_BATTLE_PARTY_DEFEATED 1
#define FIELD_BATTLE_ENEMIES_DEFEATED 2

/** @brief FieldBattleContext::state flag: the battle is over. */
#define FIELD_BATTLE_FINISHED 0x80000000

/** @brief Record id whose script variables hold the companion values. */
#define FIELD_COMPANION_RECORD_ID 2

/** @brief Owner id of the first event record. */
#define FIELD_EVENT_OWNER 0x80

/** @brief Actor event run for an action, and the one run when the battle ends. */
#define FIELD_ACTION_EVENT 12
#define FIELD_BATTLE_END_EVENT 13

/** @brief FieldStatusRecordMeta::packed bit of bits.ally. */
#define FIELD_STATUS_META_ALLY 0x200

/** @brief FieldStatusRecordMeta::packed bits of bits.kind. */
#define FIELD_STATUS_META_KIND_MASK 0xFC00
#define FIELD_STATUS_META_KIND_SHIFT 10

/** @brief FieldStatusRecordMeta::bytes.unk2 bit: actions never apply to the record. */
#define FIELD_STATUS_META_UNTARGETABLE 0x4000

/** @brief FieldStatusRecordMeta::bytes.unk2 bit 0; a defeated monster keeps its knocked-out flag. */
#define FIELD_STATUS_META_UNK16 0x1

/** @brief FieldActorTemplate::flags bit 0; a defeated monster keeps its knocked-out flag. */
#define FIELD_TEMPLATE_FLAG_UNK0 0x1

/** @brief FieldStatusRecord::status_flags bits read on defeat. */
#define FIELD_RECORD_FLAG_AUTO_REVIVE 0x2
#define FIELD_RECORD_FLAG_SHORT_DEFEAT_TIMER 0x20
#define FIELD_RECORD_FLAG_NO_DEFEAT_TIMER 0x40

/** @brief FieldStatusState::effect_flags bits. */
#define FIELD_EFFECT_DEFENSELESS 0x2
#define FIELD_EFFECT_KNOCKED_OUT 0x200

/** @brief Effects 0-10, cleared when a record is defeated. */
#define FIELD_EFFECT_DEFEAT_CLEAR_MASK 0x7FF

/** @brief Timed effect ended by taking damage. */
#define FIELD_EFFECT_INDEX_CLEARED_BY_DAMAGE 6

/** @brief Frames before a defeated party record recovers. */
#define FIELD_DEFEAT_TIMER_NORMAL 900
#define FIELD_DEFEAT_TIMER_SHORT 450
#define FIELD_DEFEAT_TIMER_QUICK 15

/** @brief Effect played when a party record revives or recovers. */
#define FIELD_REVIVE_EFFECT 0x2C

/** @brief Status slot ids tested with field_count_status_slots. */
#define FIELD_STATUS_ID_RESIST_ALL 8
#define FIELD_STATUS_ID_DRAIN 10
#define FIELD_STATUS_ID_DAMAGE_IMMUNE 11
#define FIELD_STATUS_ID_QUICK_RECOVERY 12

/** @brief Bases of the status slot ids that scale damage, offset by race, kind or defense slot. */
#define FIELD_STATUS_ID_RACE_SLAYER 0x10
#define FIELD_STATUS_ID_RACE_GUARD 0x20
#define FIELD_STATUS_ID_KIND_GUARD 0x30
#define FIELD_STATUS_ID_KIND_BOOST 0x38
#define FIELD_STATUS_ID_SLOT_BOOST 0x40

/** @brief First and one-past-last status slot id passed on to the target by a hit. */
#define FIELD_STATUS_ID_ON_HIT_FIRST 0x50
#define FIELD_STATUS_ID_ON_HIT_END 0x60


/** @brief Number of party records; higher record ids are monsters. */
#define FIELD_PARTY_RECORD_COUNT 3

/** @brief Item id of the revive item consumed by field_consume_revive_item. */
#define FIELD_ITEM_REVIVE 0x58

/** @brief Value of an empty FieldItemRecord::special_ids slot. */
#define FIELD_ITEM_ID_NONE 0xFF

/** @brief FieldItemRecord::effect_index bit of an item carrying a revive. */
#define FIELD_ITEM_EFFECT_REVIVE 0x2

/** @brief Resource holding the coordinate panel labels. */
#define FIELD_RESOURCE_COORDINATE_LABELS 9

/** @brief Labels shown on the coordinate panel. */
#define FIELD_COORDINATE_LABEL_COUNT 3

/** @brief Number of handlers in g_field_action_handlers. */
#define FIELD_ACTION_HANDLER_COUNT 8

/** @brief Action kind whose extra effects need a target weak to its elements. */
#define FIELD_ACTION_KIND_ELEMENTAL 2

/** @brief Kind of an action descriptor (low nibble of its first word). */
#define FIELD_DESCRIPTOR_KIND(descriptor) ((descriptor)->info.word & 0xF)

/** @brief Side rule of an action descriptor (bits 4-5). */
#define FIELD_DESCRIPTOR_SIDE_RULE(descriptor) (((descriptor)->info.word >> 4) & 3)

/** @brief Defense slot of an action descriptor (bits 6-7). */
#define FIELD_DESCRIPTOR_DEFENSE_SLOT(descriptor) ((descriptor)->info.bytes.flags >> 6)

/** @brief Handler for one action descriptor kind; returns the damage dealt. */
typedef s32 (*FieldActionHandler)(void);

/** @brief Chance and duration of one on-hit status, indexed from FIELD_STATUS_ID_ON_HIT_FIRST. */
typedef struct FieldOnHitStatus
{
    u8 chance;
    u8 duration;
} FieldOnHitStatus;

extern FieldBattleContext* g_field_battle;
extern FieldGameState* g_field_game_state;

/** @brief Nonzero while a guest is in the party; selects the coordinate panel icon. */
extern s32 g_field_party_has_guest;

/** @brief Label strings drawn on the coordinate panel, NULL when missing. */
extern u8* g_field_coordinate_labels[FIELD_COORDINATE_LABEL_COUNT];

/** @brief Action handlers indexed by FieldActionDescriptor::info.bytes.handler. */
extern FieldActionHandler g_field_action_handlers[FIELD_ACTION_HANDLER_COUNT];

/** @brief element_defense slot matched against each element bit. */
extern u8 g_field_element_resist_slots[FIELD_ELEMENT_COUNT];

extern FieldOnHitStatus g_field_on_hit_statuses[];

u8* func_800C1E40(s32 resource_id);
s32 field_get_actor_animation(s32 key);
s32 field_start_actor_defeat_by_key(s32 key, s32 value);
s32 field_spawn_shared_animation_actor(s32 key, s32 resource_index);
s32 field_schedule_actor_revive(s32 key, s32 value1, s32 value2, s32 value3, s32 value0);
s32 rand(void);

s32 field_battle_side_defeated(FieldStatusRecord* record);
void field_consume_revive_item(FieldStatusRecord* record);
void field_compute_attack(s32 stat_index, s32* attack);
void field_compute_defense(s32 stat_index, s32* defense);
void field_apply_element_modifiers(s32 unused, s32 element_mask, s32* attack, s32* defense);
s32 field_apply_damage(u32 attack, u32 defense);
s32 field_apply_status_multipliers(s32 damage);
s32 field_apply_attacker_double(s32 damage);
void field_apply_on_hit_statuses(void);

/**
 * @brief Store a record id in the watched-record script variable.
 * @param record_id Record id to watch, or -1 for none.
 */
void field_battle_set_watched_record(s32 record_id)
{
    field_set_script_var(-1, FIELD_VAR_WATCHED_RECORD, record_id);
}

/**
 * @brief Run the action event in mode 3 on the three party records.
 */
void field_battle_run_party_event(void)
{
    s32 i;

    for (i = 0; i < FIELD_PARTY_RECORD_COUNT; i++)
    {
        field_run_actor_event(i, FIELD_ACTION_EVENT, 3);
    }
}

/**
 * @brief Check whether the bound action may hit its target.
 * @return -1 when the action applies to the target, otherwise 0.
 */
s32 field_action_target_applies(void)
{
    s32 opposed;
    s32 side_rule;

    if (g_field_battle->target->meta.bytes.unk2 & FIELD_STATUS_META_UNTARGETABLE)
    {
        return 0;
    }
    /* The u8 truncation happens before the switch in the target. */
    opposed = (u8)(g_field_battle->attacker->meta.bits.ally ^ g_field_battle->target->meta.bits.ally);
    side_rule = FIELD_DESCRIPTOR_SIDE_RULE(g_field_battle->descriptor);
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
s32 field_battle_side_defeated(FieldStatusRecord* record)
{
    if (record->meta.packed & FIELD_STATUS_META_ALLY)
    {
        if (field_get_script_var(0, FIELD_VAR_ALLY_COUNT) == 0)
        {
            return FIELD_BATTLE_PARTY_DEFEATED;
        }
    }
    else
    {
        if (field_get_script_var(0, FIELD_VAR_ENEMY_COUNT) == 0)
        {
            return FIELD_BATTLE_ENEMIES_DEFEATED;
        }
    }

    return FIELD_BATTLE_ONGOING;
}

/**
 * @brief Handle a record whose HP reached zero.
 * @param record Defeated record.
 * @return The field_battle_side_defeated result for the record's side.
 */
s32 field_battle_handle_defeat(FieldStatusRecord* record)
{
    s32 remaining;
    s32 duration;
    FieldActionDescriptor* descriptor;

    if (record->meta.bytes.id < FIELD_PARTY_RECORD_COUNT)
    {
        if (field_get_script_var(0, FIELD_VAR_WATCHED_RECORD) == record->meta.bytes.id)
        {
            field_set_script_var(0, FIELD_VAR_WATCHED_RECORD, -1);
        }
        if (record->meta.packed & FIELD_STATUS_META_ALLY)
        {
            remaining = field_get_script_var(0, FIELD_VAR_ALLY_COUNT);
        }
        else
        {
            remaining = field_get_script_var(0, FIELD_VAR_ENEMY_COUNT);
        }
        remaining--;
        if (remaining == 0 && (record->status_flags & FIELD_RECORD_FLAG_AUTO_REVIVE))
        {
            field_spawn_shared_animation_actor(record->meta.bytes.id, FIELD_REVIVE_EFFECT);
            remaining = 1;
            record->state->current = record->state->maximum;
            field_consume_revive_item(record);
        }
        else
        {
            record->state->effect_flags &= ~FIELD_EFFECT_DEFEAT_CLEAR_MASK;
            record->state->effect_flags |= FIELD_EFFECT_KNOCKED_OUT;
            field_start_actor_defeat_by_key(record->meta.bytes.id, -1);
            duration = FIELD_DEFEAT_TIMER_NORMAL;
            if (record->status_flags & FIELD_RECORD_FLAG_SHORT_DEFEAT_TIMER)
            {
                duration = FIELD_DEFEAT_TIMER_SHORT;
            }
            if (field_count_status_slots(record, FIELD_STATUS_ID_QUICK_RECOVERY) != 0)
            {
                descriptor = g_field_battle->descriptor;
                if (descriptor != NULL && FIELD_DESCRIPTOR_KIND(descriptor) < FIELD_ACTION_KIND_ELEMENTAL)
                {
                    duration = FIELD_DEFEAT_TIMER_QUICK;
                }
            }
            if (!(record->status_flags & FIELD_RECORD_FLAG_NO_DEFEAT_TIMER))
            {
                field_schedule_actor_revive(g_field_battle->action->target_id, 0x1E, FIELD_REVIVE_EFFECT, -1, duration);
            }
        }
        if (record->meta.packed & FIELD_STATUS_META_ALLY)
        {
            field_set_script_var(0, FIELD_VAR_ALLY_COUNT, remaining);
        }
        else
        {
            field_set_script_var(0, FIELD_VAR_ENEMY_COUNT, remaining);
        }
    }
    else
    {
        record->state->effect_flags &= ~FIELD_EFFECT_DEFEAT_CLEAR_MASK;
        if ((record->meta.bytes.unk2 & FIELD_STATUS_META_UNK16) || (record->template->flags & FIELD_TEMPLATE_FLAG_UNK0))
        {
            record->state->effect_flags |= FIELD_EFFECT_KNOCKED_OUT;
        }
        field_set_actor_record_script_only(record->meta.bytes.id, 0);
        record->meta.bits.active = 0;
        field_start_actor_defeat_by_key(record->meta.bytes.id, field_roll_defeat_drop(record));
        remaining = field_get_script_var(0, FIELD_VAR_ENEMY_COUNT) - 1;
        field_set_script_var(0, FIELD_VAR_ENEMY_COUNT, remaining);
    }
    return field_battle_side_defeated(record);
}

/**
 * @brief End the battle with a result and run the battle end event.
 * @param result Battle result stored in FIELD_VAR_BATTLE_RESULT.
 */
void field_battle_finish(s32 result)
{
    field_set_script_var(0, FIELD_VAR_BATTLE_RESULT, result);
    g_field_battle->state.flags |= FIELD_BATTLE_FINISHED;
    field_run_actor_event(FIELD_EVENT_OWNER, FIELD_BATTLE_END_EVENT, 1);
}

/**
 * @brief Select the coordinate panel labels for the current party.
 * @note The label resource is a 4-byte header and a u16 offset table; each
 *       group of three labels follows the previous one, the first group is
 *       used without a guest and group (guest id + 1) with one.
 */
void field_select_coordinate_labels(void)
{
    s32 first;
    u32 i;
    u8* resource;
    u16* offsets;

    g_field_party_has_guest = 0;
    first = 0;
    if (g_field_game_state->characters[1].name[0] != 0 && (g_field_game_state->characters[1].info.word & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_GUEST)
    {
        g_field_party_has_guest = 1;
        first = (g_field_game_state->characters[1].info.bytes[1] + 1) * FIELD_COORDINATE_LABEL_COUNT;
    }

    resource = func_800C1E40(FIELD_RESOURCE_COORDINATE_LABELS);
    i = 0;
    if (resource != NULL)
    {
        /* Integer sum: the target adds the index before the pointer. */
        offsets = (u16*)((first * 2) + (s32)resource);
        for (; i < FIELD_COORDINATE_LABEL_COUNT; i++)
        {
            g_field_coordinate_labels[i] = resource + (offsets[i + 2] + 4);
        }
    }
    else
    {
        for (; i < FIELD_COORDINATE_LABEL_COUNT; i++)
        {
            g_field_coordinate_labels[i] = NULL;
        }
    }
}

/**
 * @brief Make a record both the attacker and the target and settle its defeat.
 * @param record_id Record to look up.
 */
void field_battle_defeat_record(s32 record_id)
{
    FieldStatusRecord* record;
    s32 result;

    record = field_find_status_record(record_id);
    g_field_battle->attacker = record;
    g_field_battle->target = record;
    g_field_battle->descriptor = NULL;
    if (record != NULL)
    {
        result = field_battle_handle_defeat(record);
        if (result != FIELD_BATTLE_ONGOING)
        {
            field_battle_finish(result);
        }
    }
}

/**
 * @brief Consume the first equipped revive item of a party record.
 * @param record Party record that was revived.
 */
void field_consume_revive_item(FieldStatusRecord* record)
{
    s32 i;
    s32 j;
    FieldItemRecord* item;

    for (i = 0; i < FIELD_EQUIPMENT_SLOT_COUNT; i++)
    {
        item = &g_field_game_state->characters[record->meta.bytes.id].equipment[i];
        if (item->kind != 0 && (item->effect_index & FIELD_ITEM_EFFECT_REVIVE))
        {
            for (j = 0; j < FIELD_ITEM_SPECIAL_COUNT; j++)
            {
                if (item->special_ids[j] == FIELD_ITEM_REVIVE)
                {
                    item->special_ids[j] = FIELD_ITEM_ID_NONE;
                    item->effect_index = 0;
                    field_rebuild_equipment_status_flags(record);
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
s32 field_run_action_handler(void)
{
    FieldActionDescriptor* descriptor;

    descriptor = g_field_battle->descriptor;
    if (descriptor != NULL)
    {
        if (descriptor->info.bytes.handler < FIELD_ACTION_HANDLER_COUNT)
        {
            return g_field_action_handlers[descriptor->info.bytes.handler]();
        }
        record_game_diagnostic(0x8001, 0x65, descriptor->info.bytes.handler, g_field_battle->action->action_id);
        return 0;
    }
    return 0;
}

/**
 * @brief Handler 0: damage, then a status effect on the target.
 * @return Damage dealt.
 */
s32 field_action_damage_status(void)
{
    s32 attack;
    s32 defense;
    FieldActionParams params;
    FieldActionParams status;
    s32 element_mask;
    s32 damage;

    params = g_field_battle->descriptor->params;
    /* The status fields are read from a second copy, as in field_action_damage_stat_change. */
    status = params;
    field_compute_attack(params.status.attack_stat, &attack);
    field_compute_defense(params.status.defense_stat, &defense);
    /* Word shift, not the element_mask bitfield, which schedules the call setup differently. */
    element_mask = (params.word >> 8) & 0xFF;
    field_apply_element_modifiers(0, element_mask, &attack, &defense);
    damage = field_apply_damage(attack, defense);
    if ((g_field_battle->target->weak_elements & element_mask) || (FIELD_DESCRIPTOR_KIND(g_field_battle->descriptor) != FIELD_ACTION_KIND_ELEMENTAL))
    {
        field_apply_status_effect(g_field_battle->attacker, g_field_battle->target, 0, status.status.effect, (status.status.chance + 1) * 16, status.status.duration * 16);
    }
    field_apply_on_hit_statuses();
    return damage;
}

/**
 * @brief Handler 1: damage, then drain part of it into the attacker's HP.
 * @return Damage dealt, or 0 when the attacker has no HP left.
 */
s32 field_action_damage_drain(void)
{
    s32 attack;
    s32 defense;
    FieldActionParams params;
    s32 damage;

    params = g_field_battle->descriptor->params;
    field_compute_attack(params.roll.attack_stat, &attack);
    field_compute_defense(params.roll.defense_stat, &defense);
    field_apply_element_modifiers(0, 0, &attack, &defense);
    damage = field_apply_damage(attack, defense);

    if (g_field_battle->attacker->state->current == 0)
    {
        return 0;
    }

    if ((g_field_battle->target->weak_elements & params.roll.element_mask) || (FIELD_DESCRIPTOR_KIND(g_field_battle->descriptor) != FIELD_ACTION_KIND_ELEMENTAL))
    {
        if (params.roll.spread == 0)
        {
            params.roll.spread = 1;
        }
        field_heal_status(g_field_battle->attacker->state, (u32)(damage * (params.roll.base + rand() % params.roll.spread)) >> 7);
    }

    field_apply_on_hit_statuses();
    return damage;
}

/**
 * @brief Handler 2: damage, then stat changes on the attacker and the target.
 * @return Damage dealt.
 */
s32 field_action_damage_stat_change(void)
{
    s32 attack;
    s32 defense;
    FieldActionParams params;
    FieldActionParams changes;
    s32 element_mask;
    s32 damage;

    params = g_field_battle->descriptor->params;
    /* Same copy and word-shift element mask as field_action_damage_status. */
    changes = params;
    field_compute_attack(params.stat_change.attack_stat, &attack);
    field_compute_defense(params.stat_change.defense_stat, &defense);
    element_mask = (params.word >> 8) & 0xFF;
    field_apply_element_modifiers(0, element_mask, &attack, &defense);
    damage = field_apply_damage(attack, defense);
    if ((g_field_battle->target->weak_elements & element_mask) || (FIELD_DESCRIPTOR_KIND(g_field_battle->descriptor) != FIELD_ACTION_KIND_ELEMENTAL))
    {
        field_scale_status_stat(g_field_battle->attacker, changes.stat_change.attacker_stat, changes.stat_change.attacker_scale, -1);
        field_scale_status_stat(g_field_battle->target, changes.stat_change.target_stat, changes.stat_change.target_scale, -1);
    }
    field_apply_on_hit_statuses();
    return damage;
}

/**
 * @brief Handler 3: damage proportional to the attacker's current HP.
 * @return Damage dealt.
 */
s32 field_action_damage_from_hp(void)
{
    s32 attack;
    s32 defense;
    FieldActionParams params;
    s32 roll;
    s32 percent;
    s32 damage;

    params = g_field_battle->descriptor->params;
    if (params.roll.spread == 0)
    {
        params.roll.spread = 1;
    }

    roll = rand() % params.roll.spread;
    percent = params.roll.base + roll;
    defense = 0;
    attack = (u32)(g_field_battle->attacker->state->current * percent) >> 4;
    field_apply_element_modifiers(0, 0, &attack, &defense);
    damage = field_apply_damage(attack, defense);
    field_apply_on_hit_statuses();
    return damage;
}

/**
 * @brief Handler 4: damage plus a status effect when the target's race matches.
 * @return Damage dealt.
 */
s32 field_action_damage_conditional(void)
{
    FieldActionParams params;
    FieldActionParams unmatched;
    s32 attack;
    s32 defense;
    s32 race;
    s32 match;
    s32 damage;

    unmatched = g_field_battle->descriptor->params;
    params = unmatched;
    race = g_field_battle->target->race;
    match = (params.word >> 8) & 0xF;

    if (race == match)
    {
        record_game_diagnostic(0x8003, race, race, 1);

        field_compute_attack(params.conditional.attack_stat, &attack);
        if (params.conditional.doubled == 1)
        {
            attack <<= 1;
        }
        field_compute_defense(params.conditional.defense_stat, &defense);
        field_apply_element_modifiers(0, 0, &attack, &defense);
        damage = field_apply_damage(attack, defense);

        field_apply_status_effect(g_field_battle->attacker, g_field_battle->target, 0, params.conditional.effect, (params.conditional.chance + 1) << 4,
                      params.conditional.duration << 4);
    }
    else
    {
        unmatched = g_field_battle->descriptor->params;
        record_game_diagnostic(0x8003, match, race, 0);

        field_compute_attack(unmatched.conditional.attack_stat, &attack);
        field_compute_defense(unmatched.conditional.defense_stat, &defense);
        field_apply_element_modifiers(0, 0, &attack, &defense);
        damage = field_apply_damage(attack, defense);
    }

    field_apply_on_hit_statuses();
    return damage;
}

/**
 * @brief Handler 5: damage, then modify the target's record.
 * @return Damage dealt.
 */
s32 field_action_damage_modify(void)
{
    s32 attack;
    s32 defense;
    FieldActionParams params;
    s32 damage;

    params = g_field_battle->descriptor->params;
    field_compute_attack(params.modify.attack_stat, &attack);
    field_compute_defense(params.modify.defense_stat, &defense);
    field_apply_element_modifiers(0, 0, &attack, &defense);
    damage = field_apply_damage(attack, defense);
    field_apply_on_hit_statuses();
    switch (params.modify.operation)
    {
    case 0:
        g_field_battle->target->state->status_intensity = 0;
        break;
    case 1:
        g_field_battle->target->unkC |= params.modify.value << 16;
        break;
    case 2:
        g_field_battle->target->unkC |= params.modify.value;
        break;
    case 3:
        g_field_battle->target->weak_elements |= params.modify.value;
        break;
    }
    return damage;
}

/**
 * @brief Handler 6: scale the target's current HP by a random percentage.
 */
void field_action_scale_target_hp(void)
{
    FieldActionParams params;
    FieldStatusState* state;
    s32 roll;
    u32 hp;

    params = g_field_battle->descriptor->params;
    if (params.roll.spread == 0)
    {
        params.roll.spread = 1;
    }
    roll = rand() % params.roll.spread;
    state = g_field_battle->target->state;
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
s32 field_action_none(void)
{
    return 0;
}

/**
 * @brief Compute the attack value from an attacker stat and the action power.
 * @param stat_index Attacker stat.
 * @param attack Receives power * (stat + 50) / 50.
 */
void field_compute_attack(s32 stat_index, s32* attack)
{
    s32 stat;

    stat = field_get_status_stat(g_field_battle->attacker, stat_index);
    *attack = (u32)(g_field_battle->power * (stat + 50)) / 50;
}

/**
 * @brief Compute the defense value from a target stat and its equipment.
 * @param stat_index Target stat.
 * @param defense Receives the scaled defense, or 0 when the target cannot defend.
 */
void field_compute_defense(s32 stat_index, s32* defense)
{
    FieldStatusRecord* target;
    FieldStatusRecord* guard;
    s32 animation;
    s32 stat;
    u8 slot;

    animation = field_get_actor_animation(g_field_battle->target->meta.bytes.id);
    stat = field_get_status_stat(g_field_battle->target, stat_index);
    target = g_field_battle->target;
    if ((target->state->effect_flags & FIELD_EFFECT_DEFENSELESS) || animation == FIELD_ANIMATION_DEFENSELESS)
    {
        *defense = 0;
    }
    else if (animation == FIELD_ANIMATION_GUARD || animation == FIELD_ANIMATION_GUARD_ALT)
    {
        g_field_battle->action_flags.bits.guarded = 1;
        slot = FIELD_DESCRIPTOR_DEFENSE_SLOT(g_field_battle->descriptor);
        guard = g_field_battle->target;
        *defense = guard->equipment_stats[slot] + guard->equipment_attributes[slot];
    }
    else
    {
        slot = FIELD_DESCRIPTOR_DEFENSE_SLOT(g_field_battle->descriptor);
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
void field_apply_element_modifiers(s32 unused, s32 element_mask, s32* attack, s32* defense)
{
    s32 elements;
    s32 mask;
    s32 sum;
    s32 i;

    elements = element_mask | g_field_battle->power_flags;
    mask = elements & g_field_battle->target->weak_elements;

    if (mask != 0)
    {
        i = 0;
        sum = 0;
        for (; i < FIELD_ELEMENT_COUNT; i++)
        {
            if (mask & 1)
            {
                sum += g_field_battle->attacker->element_attack[i];
            }
            mask >>= 1;
        }
        *attack = (u32)(*attack * (sum + 5)) >> 2;
    }

    if (field_count_status_slots(g_field_battle->target, FIELD_STATUS_ID_RESIST_ALL) == 0)
    {
        mask = elements & g_field_battle->target->resist_elements;
    }
    else
    {
        mask = elements;
    }

    i = 0;
    if (mask != 0)
    {
        sum = 0;
        for (; i < FIELD_ELEMENT_COUNT; i++)
        {
            if (mask & 1)
            {
                sum += g_field_battle->target->element_defense[g_field_element_resist_slots[i]];
            }
            mask >>= 1;
        }

        if (sum >= 9)
        {
            *attack = (u32)*attack >> 2;
        }
        else
        {
            *attack = (u32)*attack >> 1;
        }
    }

    if (g_field_battle->target->meta.bytes.id != 0)
    {
        field_set_script_var(g_field_battle->target->meta.bytes.id, FIELD_VAR_RECORD_ELEMENTS, elements);
    }
}

/**
 * @brief Compute the damage of an attack and apply it to the target.
 * @param attack Attack value.
 * @param defense Defense value.
 * @return Damage dealt, or 0 when the target is immune or the action has no power.
 */
s32 field_apply_damage(u32 attack, u32 defense)
{
    s32 clamped;
    u32 half_attack;
    s32 bonus;
    u32 value;
    FieldStatusRecord* attacker;
    s32 quarter;

    if (field_count_status_slots(g_field_battle->target, FIELD_STATUS_ID_DAMAGE_IMMUNE) != 0)
    {
        return 0;
    }
    if (g_field_battle->descriptor->info.bytes.power == 0)
    {
        return 0;
    }

    if ((defense < attack) || (g_field_battle->action_flags.bits.guarded))
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
    /* Attacker of character type 4. */
    if ((g_field_battle->attacker->meta.packed & FIELD_STATUS_META_KIND_MASK) == (4 << FIELD_STATUS_META_KIND_SHIFT))
    {
        bonus = field_get_script_var(FIELD_COMPANION_RECORD_ID, FIELD_VAR_COMPANION_POWER_BONUS) * 4;
    }

    attack = (u32)((g_field_battle->descriptor->info.bytes.power + bonus) * attack) >> 4;
    attack = field_apply_status_multipliers(attack * g_field_battle->action->damage_scale);

    if ((field_count_status_slots(g_field_battle->attacker, FIELD_STATUS_ID_DRAIN) != 0) && (FIELD_DESCRIPTOR_KIND(g_field_battle->descriptor) < FIELD_ACTION_KIND_ELEMENTAL))
    {
        /* Open-coded attack / 4; value holds the state pointer because the
           target keeps it in the divisor's register. */
        attacker = g_field_battle->attacker;
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
        field_heal_status((FieldStatusState*)value, quarter >> 2);
    }

    attack = field_apply_attacker_double(attack);
    if (!(g_field_battle->action_flags.bits.guarded) && ((s32)attack <= 0))
    {
        attack = 1;
    }

    if (field_get_script_var(0, FIELD_VAR_DEBUG_LOG_DAMAGE) != 0)
    {
        record_game_diagnostic(0x8002, g_field_battle->attacker->meta.bytes.id, g_field_battle->target->meta.bytes.id, attack);
    }

    if (((field_get_script_var(0, FIELD_VAR_DEBUG_SPARE_PARTY) == 0) || (g_field_battle->target->meta.bytes.id < FIELD_PARTY_RECORD_COUNT)) &&
        ((field_get_script_var(0, FIELD_VAR_DEBUG_SPARE_ENEMIES) == 0) || (g_field_battle->target->meta.bytes.id >= FIELD_PARTY_RECORD_COUNT)))
    {
        field_clear_status_effect(g_field_battle->target, FIELD_EFFECT_INDEX_CLEARED_BY_DAMAGE);
        field_damage_status(g_field_battle->target->state, attack);
    }

    return attack;
}

/**
 * @brief Apply the attacker's and the target's status multipliers to damage.
 * @param damage Base damage.
 * @return Damage after the multipliers: x1.5 per boosting status, x0.5 per guarding one.
 */
s32 field_apply_status_multipliers(s32 damage)
{
    s32 count;
    s32 kind;

    count = field_count_status_slots(g_field_battle->attacker, FIELD_DESCRIPTOR_DEFENSE_SLOT(g_field_battle->descriptor) | FIELD_STATUS_ID_SLOT_BOOST);
    for (; count != 0; count--)
    {
        damage = (damage * 3) / 2;
    }

    kind = FIELD_DESCRIPTOR_KIND(g_field_battle->descriptor);
    count = field_count_status_slots(g_field_battle->attacker, kind + FIELD_STATUS_ID_KIND_BOOST);
    for (; count != 0; count--)
    {
        damage = (damage * 3) / 2;
    }

    kind = FIELD_DESCRIPTOR_KIND(g_field_battle->descriptor);
    count = field_count_status_slots(g_field_battle->target, kind + FIELD_STATUS_ID_KIND_GUARD);
    for (; count != 0; count--)
    {
        damage = damage / 2;
    }

    count = field_count_status_slots(g_field_battle->attacker, g_field_battle->target->race + FIELD_STATUS_ID_RACE_SLAYER);
    for (; count != 0; count--)
    {
        damage = (damage * 3) / 2;
    }

    count = field_count_status_slots(g_field_battle->target, g_field_battle->attacker->race + FIELD_STATUS_ID_RACE_GUARD);
    for (; count != 0; count--)
    {
        damage = damage / 2;
    }

    return damage;
}

/**
 * @brief Double the damage while bit 0 of the attacker's unkC flags is set.
 * @param damage Damage so far.
 * @return The possibly doubled damage.
 */
s32 field_apply_attacker_double(s32 damage)
{
    s32 result;

    result = damage;
    if (g_field_battle->attacker->unkC & 1)
    {
        result *= 2;
    }
    return result;
}

/**
 * @brief Pass the attacker's on-hit statuses to the target.
 */
void field_apply_on_hit_statuses(void)
{
    s32 status;
    FieldOnHitStatus* table;
    FieldOnHitStatus* entry;

    if (FIELD_DESCRIPTOR_KIND(g_field_battle->descriptor) != FIELD_ACTION_KIND_ELEMENTAL)
    {
        for (status = FIELD_STATUS_ID_ON_HIT_FIRST; status < FIELD_STATUS_ID_ON_HIT_END; status++)
        {
            if (field_count_status_slots(g_field_battle->attacker, status) != 0)
            {
                /* Indexing the table directly folds the -0xA0 bias into the address. */
                table = g_field_on_hit_statuses;
                entry = &table[status - FIELD_STATUS_ID_ON_HIT_FIRST];
                field_apply_status_effect(g_field_battle->attacker, g_field_battle->target, 0, status - FIELD_STATUS_ID_ON_HIT_FIRST, entry->chance, entry->duration * 16);
            }
        }
    }
}
