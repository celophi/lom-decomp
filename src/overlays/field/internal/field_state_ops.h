#ifndef FIELD_STATE_OPS_H
#define FIELD_STATE_OPS_H

#include "common.h"

#include "overlays/field/field_actor_records.h"

enum
{
    FIELD_STATUS_STAT_COUNT = 8,
    FIELD_STATUS_TIMER_COUNT = 12,
    FIELD_STATUS_SLOT_COUNT = 3,
    FIELD_ELEMENT_COUNT = 8
};

/** @brief Record kind stored in FieldStatusRecordMeta::bits.kind for a template-built monster. */
#define FIELD_STATUS_KIND_MONSTER 5
/** @brief FieldStatusRecordMeta::packed views of the active, ally and kind fields. */
#define FIELD_STATUS_META_ACTIVE 0x100
#define FIELD_STATUS_META_ALLY 0x200
#define FIELD_STATUS_META_KIND_SHIFT 10
#define FIELD_STATUS_META_KIND_MASK (0x3F << FIELD_STATUS_META_KIND_SHIFT)
/** @brief Highest level a record can reach. */
#define FIELD_LEVEL_MAX 99
/** @brief Element attack and defense level of a record with no modifiers. */
#define FIELD_ELEMENT_LEVEL_NEUTRAL 5

/** @brief Packed identifier and flags stored by each field status record. */
typedef union
{
    u32 packed;
    struct
    {
        u8 id;
        u8 flags;
        /** @brief FIELD_GUARD_FLAG_* bits. */
        u16 guard_flags;
    } bytes;
    struct
    {
        u32 id : 8;
        /** @brief Set while the record is in use. */
        u32 active : 1;
        /** @brief Set for records on the party's side. */
        u32 ally : 1;
        /** @brief Character type for party records, FIELD_STATUS_KIND_MONSTER for monsters. */
        u32 kind : 6;
        u32 guard_flags : 16;
    } bits;
} FieldStatusRecordMeta;

/** @brief Growth pair: a base value and a per-level increment. */
typedef struct FieldGrowthPair
{
    u8 base;
    u8 growth;
} FieldGrowthPair;

/** @brief Drop slots of a monster template; slot n is picked when bit n is the lowest set bit of a random mask. */
#define FIELD_ACTOR_DROP_SLOT_COUNT 8

/** @brief FieldActorTemplate::flags bit: a boss (fixed HUD panel, large pickups). */
#define FIELD_TEMPLATE_BOSS 0x80

/** @brief One drop slot of a monster template: a field_roll_defeat_drop handler and its argument. */
typedef struct FieldActorDrop
{
    u8 handler;
    u8 value;
} FieldActorDrop;

/** @brief Monster template returned by field_find_actor_template and linked from its status record. */
typedef struct FieldActorTemplate
{
    u8 pad0[0x15];
    /** @brief Bit n set: element level n raises the monster level. */
    u8 raise_mask;
    /** @brief Bit n set: element level n lowers the monster level. */
    u8 lower_mask;
    u8 pad17;
    /** @brief Identifier matched against FieldObjectState::template_index. */
    u8 id;
    /** @brief Evasion chance given to the monster's status record. */
    u8 evasion;
    /** @brief Monster race. */
    u8 race;
    u8 counter_reset;
    u16 hp_base;
    /** @brief Per-level HP increment, or 0xFFFF to grow HP along the stat 4 curve. */
    u16 hp_growth;
    u16 power_base;
    u16 power_growth;
    FieldGrowthPair equipment_stats[4];
    FieldGrowthPair stats[FIELD_STATUS_STAT_COUNT];
    u8 immunity_flags;
    u8 weak_elements;
    u8 resist_elements;
    /** @brief Bit 1: not linked from the object state; bit 2: level from the placed land count instead of the land's distance; bit 7: FIELD_TEMPLATE_BOSS. */
    u8 flags;
    FieldActorDrop drops[FIELD_ACTOR_DROP_SLOT_COUNT];
    s32 action_count;
    u8 actions[1][8];
} FieldActorTemplate;

/** @brief Monster template table: a count, then one offset per template from the table base. */
typedef struct FieldActorTemplateTable
{
    u32 count;
    u32 offsets[1];
} FieldActorTemplateTable;

/** @brief Rewards granted by field_grant_reward; kinds 0 to 3 also index FieldObjectState drop_counts. */
typedef enum
{
    FIELD_REWARD_EXPERIENCE_LARGE = 0,
    FIELD_REWARD_EXPERIENCE_SMALL = 1,
    FIELD_REWARD_MONEY_LARGE = 2,
    FIELD_REWARD_MONEY_SMALL = 3,
    FIELD_REWARD_ITEM = 4,
    FIELD_REWARD_RESTORE_QUARTER = 5,
    FIELD_REWARD_RESTORE_HALF = 6
} FieldRewardKind;

/** @brief facing_or_reward_kind of a pickup effect of reward kind 0; kind n is this plus n. */
#define FIELD_REWARD_EFFECT_KIND_FIRST 0x16

/** @brief Sentinel returned by field_find_object_state when it finds no object. */
#define FIELD_STATUS_STATE_NONE ((FieldObjectState*)-1)

/** @brief FieldStatusRecord::side bits: record on the monsters' side (also the duel leader), or on the party's side. */
#define FIELD_RECORD_MONSTER_SIDE 0x40
#define FIELD_RECORD_PARTY_SIDE 0x80

/** @brief Field actor status record containing stats, immunities, and timed effects. */
typedef struct FieldStatusRecord
{
    /** @brief Bit 6 set for monsters and the versus-mode leader, bit 7 for other party members. */
    u8 side;
    u8 pad1[2];
    /** @brief Monster race, matched by the race-dependent statuses and action handler 4. */
    u8 race;
    FieldStatusRecordMeta meta;
    s8 counter;
    u8 counter_reset;
    u16 status_flags;
    /** @brief FIELD_RECORD_ACTION_MODIFIERS, FIELD_RECORD_HELD_MODIFIERS and FIELD_RECORD_DEFEAT_FLAGS. */
    u32 modifier_flags;
    struct FieldObjectState* state;
    FieldActorTemplate* template;
    /** @brief Attack power: the weapon's for party records, the template's base and growth for monsters. */
    u16 power;
    /** @brief Evasion chance; 25 for party records. */
    u8 evasion;
    u8 pad1B;
    u16 equipment_stats[4];
    u8 equipment_attributes[4];
    u8 stats[FIELD_STATUS_STAT_COUNT];
    u8 base_stats[FIELD_STATUS_STAT_COUNT];
    u8 immunity_flags;
    /** @brief Element bits the record is weak to. */
    u8 weak_elements;
    /** @brief Element bits the record resists. */
    u8 resist_elements;
    u8 pad3B;
    /** @brief Attack level per element bit, 5 by default. */
    u8 element_attack[FIELD_ELEMENT_COUNT];
    /** @brief Defense level per element, 5 by default. */
    u8 element_defense[FIELD_ELEMENT_COUNT];
    /** @brief Companion ability copied from the character record. */
    u8 companion_ability;
    u8 status_slots[FIELD_STATUS_SLOT_COUNT];
    u16 status_timers[FIELD_STATUS_TIMER_COUNT];
} FieldStatusRecord;

/** @brief Fields of FieldStatusRecord::modifier_flags. */
#define FIELD_RECORD_ACTION_MODIFIERS 0xFF    /**< Cleared after every action. */
#define FIELD_RECORD_HELD_MODIFIERS 0xFF00    /**< Cleared unless FIELD_EFFECT_HOLD_MODIFIERS. */
#define FIELD_RECORD_DEFEAT_FLAGS 0xFF000000U /**< Set on the target when it is defeated. */

/** @brief FIELD_RECORD_DEFEAT_FLAGS bits: how the attacker changes the defeated record's drops. */
#define FIELD_DEFEAT_EXPERIENCE_PLUS_1 0x01000000    /**< One more experience pickup. */
#define FIELD_DEFEAT_EXPERIENCE_PLUS_2 0x02000000    /**< Two more experience pickups. */
#define FIELD_DEFEAT_EXTRA_DROP_SLOTS 0x04000000     /**< Two more drop slots take part in the drop roll. */
#define FIELD_DEFEAT_NO_COMMON_DROPS 0x08000000      /**< Drop slots 0 and 1, the likeliest, are never picked. */
#define FIELD_DEFEAT_MONEY_PLUS_1 0x10000000         /**< One more money pickup. */
#define FIELD_DEFEAT_MONEY_PLUS_2 0x20000000         /**< Two more money pickups. */
#define FIELD_DEFEAT_EXPERIENCE_AS_MONEY 0x80000000U /**< Every experience pickup becomes a money pickup. */

/** @brief apply_flags bits of field_apply_status_effect. */
enum
{
    FIELD_STATUS_APPLY_IGNORE_IMMUNITY = 1 << 0,
    FIELD_STATUS_APPLY_ALLOW_ACTIVE = 1 << 1
};

FieldStatusRecord* field_find_status_record(s32 record_id);
FieldStatusRecord* field_find_free_status_record(void);
void field_apply_status_effect(FieldStatusRecord* source, FieldStatusRecord* target, s32 apply_flags, s32 effect_index, s32 chance_threshold, s32 duration);
s32 field_get_status_stat(FieldStatusRecord* record, s32 stat_index);
s32 field_scale_status_stat(FieldStatusRecord* record, u32 stat_selector, u32 scale, s32 emit_signal);
s32 field_roll_last_stat(FieldStatusRecord* record);
s32 field_is_actor_in_front(s32 actor_id, s32 observer_id);
void field_damage_status(struct FieldObjectState* state, s32 damage);
void field_heal_status(struct FieldObjectState* state, s32 amount);
void field_revive_status_record(FieldStatusRecord* record);
void field_clear_status_effect(FieldStatusRecord* record, u32 effect_index);
void field_golem_select_logic_cell(s32 actor_id);
void field_golem_publish_logic_cell(s32 cell_index);
void field_raise_companion_intensity(s32 amount);
void field_battle_setup(s32 group);
s32 field_compute_base_monster_level(s32 use_land_count);

#endif
