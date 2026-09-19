#ifndef FIELD_STATE_OPS_H
#define FIELD_STATE_OPS_H

#include "common.h"

enum
{
    FIELD_STATUS_STAT_COUNT = 8,
    FIELD_STATUS_TIMER_COUNT = 12,
    FIELD_STATUS_SLOT_COUNT = 3
};

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
} FieldStatusRecordMeta;

/** @brief Runtime status values shared by a field actor and its status record. */
typedef struct FieldStatusState
{
    u32 maximum;
    s32 current;
    u8 pad8[4];
    u32 effect_flags;
    u8 pad10[4];
    s32 actor_id;
    u8 pad18[0x48 - 0x18];
    u16 status_intensity;
    u8 pad4A[0x60 - 0x4A];
    u8 status_signal;
} FieldStatusState;

/** @brief Field actor status record containing stats, immunities, and timed effects. */
typedef struct FieldStatusRecord
{
    u32 unk0;
    FieldStatusRecordMeta meta;
    u8 pad8[8];
    FieldStatusState *state;
    u8 pad14[0x28 - 0x14];
    u8 stats[FIELD_STATUS_STAT_COUNT];
    u8 base_stats[FIELD_STATUS_STAT_COUNT];
    u8 immunity_flags;
    u8 pad39[0x4D - 0x39];
    volatile u8 status_slots[FIELD_STATUS_SLOT_COUNT];
    u16 status_timers[FIELD_STATUS_TIMER_COUNT];
} FieldStatusRecord;

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
