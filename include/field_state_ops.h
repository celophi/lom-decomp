#ifndef FIELD_STATE_OPS_H
#define FIELD_STATE_OPS_H

#include "common.h"

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
        u16 unk2;
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
        u32 unk16 : 16;
    } bits;
} FieldStatusRecordMeta;

/** @brief Growth pair: a base value and a per-level increment. */
typedef struct FieldGrowthPair
{
    u8 base;
    u8 growth;
} FieldGrowthPair;

/** @brief Monster template returned by field_find_actor_template and linked from its status record. */
typedef struct FieldActorTemplate
{
    u8 pad0[0x15];
    /** @brief Bit n set: element level n raises the monster level. */
    u8 raise_mask;
    /** @brief Bit n set: element level n lowers the monster level. */
    u8 lower_mask;
    u8 pad17;
    /** @brief Identifier matched against FieldStatusState::template_index. */
    u8 id;
    u8 unk19;
    u8 unk1A;
    u8 counter_reset;
    u16 hp_base;
    /** @brief Per-level HP increment, or 0xFFFF to grow HP along the stat 4 curve. */
    u16 hp_growth;
    u16 unk20_base;
    u16 unk20_growth;
    FieldGrowthPair equipment_stats[4];
    FieldGrowthPair stats[FIELD_STATUS_STAT_COUNT];
    u8 immunity_flags;
    u8 weak_elements;
    u8 resist_elements;
    /** @brief Bit 1: not linked from the object state; bit 2: level from the hero's level instead of the land's; bit 7: see the HP gauge. */
    u8 flags;
    u8 pad40[0x50 - 0x40];
    s32 action_count;
    u8 actions[1][8];
} FieldActorTemplate;

/** @brief Monster template table: a count, then one offset per template from the table base. */
typedef struct FieldActorTemplateTable
{
    u32 count;
    u32 offsets[1];
} FieldActorTemplateTable;

/** @brief Displayed HP gauge (low 24 bits) and HUD bits. */
typedef union
{
    u32 word;
    struct
    {
        u32 value : 24;
        u32 hud_bits : 7;
        u32 hud_flag : 1;
    } bits;
} FieldStatusGauge;

/** @brief Runtime status values shared by a field actor and its status record. */
typedef struct FieldStatusState
{
    u32 maximum;
    s32 current;
    FieldStatusGauge gauge;
    u32 effect_flags;
    u8 unk10;
    /** @brief Template id passed to field_find_actor_template. */
    u8 template_index;
    u8 pad12[2];
    s32 actor_id;
    u8 pad18[0x48 - 0x18];
    u16 status_intensity;
    u8 pad4A[2];
    /** @brief Bits 1-7: level. */
    union
    {
        s32 word;
        struct
        {
            u32 unk0 : 1;
            u32 level : 7;
        } bits;
    } level;
    /** @brief Stored world position, compared with the live actor position. */
    s32 position_x;
    s32 position_y;
    s32 position_z;
    u8 pad5C[0x60 - 0x5C];
    u8 status_signal;
    u8 pad61[3];
    FieldActorTemplate* template;
    /** @brief Stat-derived footprint strength, saturated to 255 during setup. */
    u16 effect_footprint_strength;
} FieldStatusState;

/** @brief Field actor status record containing stats, immunities, and timed effects. */
typedef struct FieldStatusRecord
{
    /** @brief Bit 6 set for monsters and the versus-mode leader, bit 7 for other party members. */
    u8 unk0;
    u8 pad1[2];
    /** @brief Monster race, matched by the race-dependent statuses and action handler 4. */
    u8 race;
    FieldStatusRecordMeta meta;
    s8 counter;
    u8 counter_reset;
    u16 status_flags;
    u32 unkC;
    FieldStatusState *state;
    FieldActorTemplate *template;
    u16 unk18;
    u8 unk1A;
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
    u8 unk4C;
    u8 status_slots[FIELD_STATUS_SLOT_COUNT];
    u16 status_timers[FIELD_STATUS_TIMER_COUNT];
} FieldStatusRecord;

/** @brief apply_flags bits of func_800B2B54. */
enum
{
    FIELD_STATUS_APPLY_IGNORE_IMMUNITY = 1 << 0,
    FIELD_STATUS_APPLY_ALLOW_ACTIVE = 1 << 1
};

FieldStatusRecord *func_800B2A9C(s32 record_id);
FieldStatusRecord *func_800B2B08(void);
void func_800B2B54(FieldStatusRecord *source, FieldStatusRecord *target, s32 apply_flags, s32 effect_index, s32 chance_threshold, s32 duration);
s32 func_800B2D34(FieldStatusRecord *record, s32 stat_index);
s32 func_800B2D64(FieldStatusRecord *record, u32 stat_selector, u32 scale, s32 emit_signal);
s32 func_800B2FF8(FieldStatusRecord *record);
s32 func_800B302C(s32 first_actor_id, s32 second_actor_id);
void func_800B30B8(FieldStatusState *state, s32 amount);
void saturating_counter_add(FieldStatusState *state, s32 delta);
void func_800B313C(FieldStatusRecord *record);
void field_clear_record_state(FieldStatusRecord *record, u32 index);
void func_800B31CC(s32 actor_id);
void func_800B32FC(s32 row_index);
void func_800B3420(s32 amount);

#endif
