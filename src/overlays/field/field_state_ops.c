#include "field_state_ops.h"
#include "field_types.h"
#include "sdk/rand.h"

enum
{
    FIELD_STATUS_PRIMARY_RECORD_COUNT = 3,
    FIELD_STATUS_RECORD_COUNT = 11,
    FIELD_STATUS_EFFECT_COUNT = 15,
    FIELD_STATUS_RESULT_ROW_COUNT = 36,
    FIELD_STATUS_MAX_DURATION = 240
};

enum
{
    FIELD_STATUS_APPLY_IGNORE_IMMUNITY = 1 << 0,
    FIELD_STATUS_APPLY_ALLOW_ACTIVE = 1 << 1
};

enum
{
    FIELD_STATUS_RECORD_ACTIVE = 1 << 0
};

#define FIELD_STATUS_SLOT_ID_BASE 0x60
#define FIELD_STATUS_SLOT_ID_COUNT 16
#define FIELD_STATUS_TIMED_EFFECT_FLAGS_MASK 0xFFFF

/** @brief Header preceding the status-record array in the field runtime context. */
typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 *unk4;
    u8 *unk8;
    u8 unkC[8];
    s32 unk14;
    s32 unk18;
    u8 pad1C[0x28 - 0x1C];
} FieldStatusContextHeader;

#define FIELD_STATUS_RECORDS_OFFSET sizeof(FieldStatusContextHeader)

/** @brief Field configuration values used to generate script results. */
typedef struct
{
    u8 pad0[0xC04];
    u8 count_flags;
    u8 padC05;
    u8 chance_percent;
    u8 padC07[0x2A7C - 0xC07];
    u8 result_rows[FIELD_STATUS_RESULT_ROW_COUNT][4];
} FieldStateConfig;

#define FIELD_STATE_CONFIG D_80122B74

s32 func_8008B288(s32 actor_id);
s32 func_80087F44(s32 actor_id, FieldVector *out);
s32 func_80089D44();
void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
void func_800BD520(s32 owner_id, u32 variable_id, s32 value);
u32 func_800C9ED4(s32 actor_id);
s32 func_8008B500(s32 record_id, s32 signal_id);

extern FieldStateConfig *D_80122B74;
extern u8 *D_80123FB0;
extern u8 D_800F0B28[];
extern u8 D_800F0B38[];
extern u8 D_800F0B50[];

/**
 * @brief Find a status record by its identifier.
 * @param record_id Identifier to find.
 * @return Matching status record, or null when no record has the identifier.
 */
FieldStatusRecord *func_800B2A9C(s32 record_id)
{
    s32 record_offset;
    s32 record_index;
    u8 *scan_base;
    u8 *context_base;

    record_index = 0;
    context_base = (u8 *)D_80123FB0;
    record_offset = FIELD_STATUS_RECORDS_OFFSET;
    scan_base = context_base;
    do
    {
        record_index++;
        if (record_id != ((FieldStatusRecord *)(scan_base + FIELD_STATUS_RECORDS_OFFSET))->meta.bytes.id)
        {
            record_offset += sizeof(FieldStatusRecord);
            scan_base += sizeof(FieldStatusRecord);
        }
        else
        {
            return (FieldStatusRecord *)(context_base + record_offset);
        }
    } while (record_index < FIELD_STATUS_RECORD_COUNT);
    akao_set_song_params(0x8001, 0x68, record_id, -1);
    return 0;
}

/**
 * @brief Find the first available secondary status record.
 * @return First secondary record whose active flag is clear, or null when none is available.
 */
FieldStatusRecord *func_800B2B08(void)
{
    s32 record_offset;
    s32 record_index;
    u8 *scan_base;
    u8 *context_base;
    record_index = FIELD_STATUS_PRIMARY_RECORD_COUNT;
    context_base = (u8 *)D_80123FB0;
    record_offset = FIELD_STATUS_RECORDS_OFFSET + (FIELD_STATUS_PRIMARY_RECORD_COUNT * sizeof(FieldStatusRecord));
    scan_base = context_base + (FIELD_STATUS_PRIMARY_RECORD_COUNT * sizeof(FieldStatusRecord));
    do
    {
        record_index++;
        if ((((FieldStatusRecord *)(scan_base + FIELD_STATUS_RECORDS_OFFSET))->meta.packed >> 8) & FIELD_STATUS_RECORD_ACTIVE)
        {
            record_offset += sizeof(FieldStatusRecord);
            scan_base += sizeof(FieldStatusRecord);
        }
        else
        {
            return (FieldStatusRecord *)(context_base + record_offset);
        }
    } while (record_index < FIELD_STATUS_RECORD_COUNT);
    return 0;
}

/**
 * @brief Apply a permitted effect with a chance check and scaled duration.
 * @param source Actor providing the offensive scale.
 * @param target Actor receiving the effect.
 * @param apply_flags Controls immunity checks and whether an active effect may be replaced.
 * @param effect_index Effect to apply.
 * @param chance_threshold Threshold compared against an eight-bit random value.
 * @param duration Base duration scaled by the actors and capped at 240.
 */
void func_800B2B54(FieldStatusRecord *source, FieldStatusRecord *target, s32 apply_flags, s32 effect_index, s32 chance_threshold, s32 duration)
{
    s32 effect_mask, slot_index, attack, defense, scaled_duration;
    u8 *immunity_masks, *stat_pair;
    FieldStatusRecord *timer_cursor;

    if (effect_index >= FIELD_STATUS_EFFECT_COUNT)
    {
        return;
    }
    if (target->state->current == 0)
    {
        return;
    }
    if (!(apply_flags & FIELD_STATUS_APPLY_IGNORE_IMMUNITY))
    {
        immunity_masks = D_800F0B28;
        if (target->immunity_flags & immunity_masks[effect_index])
        {
            return;
        }
        effect_mask = 0;
        for (slot_index = 0; slot_index < FIELD_STATUS_SLOT_COUNT; slot_index++)
        {
            do
            {
                if ((u32)(target->status_slots[slot_index] - FIELD_STATUS_SLOT_ID_BASE) < FIELD_STATUS_SLOT_ID_COUNT)
                {
                    effect_mask |= immunity_masks[target->status_slots[slot_index] - FIELD_STATUS_SLOT_ID_BASE];
                }
            } while (0);
        }
        if (effect_mask & D_800F0B28[effect_index])
        {
            return;
        }
    }
    effect_mask = 1 << effect_index;
    if (!(apply_flags & FIELD_STATUS_APPLY_ALLOW_ACTIVE) && (target->state->effect_flags & effect_mask))
    {
        return;
    }
    if ((rand() & 0xFF) >= chance_threshold)
    {
        return;
    }
    target->state->effect_flags |= effect_mask;
    stat_pair = &D_800F0B38[effect_index];
    attack = func_800B2D34(source, *stat_pair >> 4);
    defense = func_800B2D34(target, *stat_pair & 0xF);
    scaled_duration = duration * attack / defense;
    timer_cursor = (FieldStatusRecord *)((u8 *)target + effect_index * sizeof(u16));
    if (scaled_duration > FIELD_STATUS_MAX_DURATION)
    {
        scaled_duration = FIELD_STATUS_MAX_DURATION;
    }
    timer_cursor->status_timers[0] = scaled_duration;
}

/**
 * @brief Read a status stat with a minimum value of one.
 * @param record Status record to read.
 * @param stat_index Stat index.
 * @return Selected stat, or one when the stat is zero or the index is out of range.
 */
s32 func_800B2D34(FieldStatusRecord *record, s32 stat_index)
{
    u32 value;

    if (stat_index < FIELD_STATUS_STAT_COUNT)
    {
        if (record->stats[stat_index] == 0)
        {
            value = 1;
        }
        else
        {
            value = record->stats[stat_index];
        }
        return value;
    }

    return 1;
}

/**
 * @brief Scale one or more status stats and optionally notify the associated actor.
 * @param record Status record to update.
 * @param stat_selector Stat index, all-stats selector, or predefined grouped-stat selector.
 * @param scale Scale factor centered around eight.
 * @param emit_signal Nonzero to notify the associated actor when applicable.
 * @return Result produced by the selected stat operation or actor notification.
 */
s32 func_800B2D64(FieldStatusRecord *record, u32 stat_selector, u32 scale, s32 emit_signal)
{
    s32 grouped_selector;
    s32 stat_index;
    u32 scaled_value;
    u32 current_word;
    u8 current_byte;
    u32 result;
    FieldStatusRecord *stat_cursor;

    if (record->state->current == 0)
    {
        return 0;
    }
    stat_cursor = (FieldStatusRecord *)((u8 *)record + stat_selector);
    if (stat_selector < FIELD_STATUS_STAT_COUNT)
    {
        scaled_value = (u32)(stat_cursor->base_stats[0] * scale) >> 3;
        if (scale >= 9U)
        {
            current_byte = stat_cursor->stats[0];
            result = scaled_value < current_byte;
            if (result != 0)
            {
                scaled_value = (u32)current_byte;
            }
            stat_cursor->stats[0] = (u8)scaled_value;
            if (emit_signal != 0)
            {
                record->state->status_signal = (u8)D_800F0B50[scale - 8];
                result = record->state->status_signal;
                if (result != 0)
                {
                    return func_8008B500(record->meta.bytes.id, stat_selector + 0x9E);
                }
            }
            return result;
        }
        current_word = stat_cursor->stats[0];
        result = current_word < scaled_value;
        if (result != 0)
        {
            scaled_value = (u32)current_word;
        }
        stat_cursor->stats[0] = (u8)scaled_value;
        if (emit_signal != 0)
        {
            record->state->status_signal = (u8)D_800F0B50[8 - scale];
            result = record->state->status_signal;
            if (result != 0)
            {
                return func_8008B500(record->meta.bytes.id, stat_selector + 0xA7);
            }
        }
        return result;
    }
    if (stat_selector != 9)
    {
        result = stat_selector < 9U;
        if (result == 0)
        {
            if (stat_selector == 10)
            {
                goto case_10;
            }
            if (stat_selector == 11)
            {
                goto case_11;
            }
            return 11;
        }
        return result;
    }

    stat_index = 0;
    do
    {
        func_800B2D64(record, stat_index, scale, 0);
        stat_index += 1;
        result = stat_index < FIELD_STATUS_STAT_COUNT;
    } while (result != 0);
    if (emit_signal != 0)
    {
        if (scale >= 9U)
        {
            record->state->status_signal = (u8)D_800F0B50[scale - 8];
            return func_8008B500(record->meta.bytes.id, 0x9D);
        }
        record->state->status_signal = (u8)D_800F0B50[8 - scale];
        return func_8008B500(record->meta.bytes.id, 0xA6);
    }
    return result;

case_10:
    func_800B2D64(record, 0, scale, 0);
    func_800B2D64(record, 1, scale, 0);
    stat_cursor = record;
    grouped_selector = 2;
    goto recursive_tail;

case_11:
    func_800B2D64(record, 3, scale, 0);
    func_800B2D64(record, 5, scale, 0);
    stat_cursor = record;
    grouped_selector = 6;

recursive_tail:
    result = func_800B2D64(stat_cursor, grouped_selector, scale, 0);
    return result;
}

/**
 * @brief Roll a random byte against the record's final status stat.
 * @param record Status record supplying the threshold.
 * @return 1 when the random byte is below the threshold, otherwise 0.
 */
s32 func_800B2FF8(FieldStatusRecord *record)
{
    s32 threshold;

    threshold = func_800B2D34(record, FIELD_STATUS_STAT_COUNT - 1);
    return (u32)(rand() & 0xFF) < (u32)threshold;
}

/**
 * @brief Compare two actors' horizontal positions using the second actor's facing range.
 * @param first_actor_id First actor to compare.
 * @param second_actor_id Second actor to compare and query for facing.
 * @return -1 for the selected side of the second actor, otherwise 0.
 */
s32 func_800B302C(s32 first_actor_id, s32 second_actor_id)
{
    s32 direction;
    FieldVector first_position;
    FieldVector second_position;

    direction = func_8008B288(second_actor_id);
    func_80087F44(first_actor_id, &first_position);
    func_80087F44(second_actor_id, &second_position);
    if (first_position.vx - second_position.vx < 0)
    {
        if ((u32)(direction - 0x40) >= 0x81)
        {
            return 0;
        }
        return -1;
    }
    else
    {
        if ((u32)(direction - 0x40) >= 0x81)
        {
            return -1;
        }
        return 0;
    }
}

/**
 * @brief Subtract from a status value while clamping it at zero.
 * @param state Status state to update.
 * @param amount Amount to subtract; negative values are reported to the audio driver instead.
 */
void func_800B30B8(FieldStatusState *state, s32 amount)
{
    s32 remaining;

    if (amount < 0)
    {
        akao_set_song_params(0x8001, 0x7A, state->actor_id, amount);
    }
    else
    {
        remaining = state->current - amount;
        if (remaining >= 0)
        {
            state->current = remaining;
        }
        else
        {
            state->current = 0;
        }
    }
}

/**
 * @brief Add to a counter and clamp it to its cap on overflow.
 * @param state Status state containing the counter and its maximum.
 * @param delta Amount to add to the counter's value.
 */
void saturating_counter_add(FieldStatusState *state, s32 delta)
{
    u32 maximum;
    u32 sum;

    maximum = state->maximum;
    sum = state->current + delta;
    state->current = sum;
    if (maximum < sum)
    {
        state->current = maximum;
    }
}

/**
 * @brief Forward a status record identifier to the actor-state helper.
 * @param record Status record whose identifier is forwarded.
 */
void func_800B313C(FieldStatusRecord *record)
{
    func_80089D44(record->meta.bytes.id);
}

/**
 * @brief Clear one timed status effect or all timed status effects.
 * @param record Status record to update.
 * @param index Effect index, or an out-of-range value to clear every timed effect.
 */
void field_clear_record_state(FieldStatusRecord *record, u32 index)
{
    s32 timer_index;

    if (index < FIELD_STATUS_TIMER_COUNT)
    {
        record->state->effect_flags &= ~(1 << index);
        record->status_timers[index] = 0;
        return;
    }
    record->state->effect_flags &= ~FIELD_STATUS_TIMED_EFFECT_FLAGS_MASK;
    for (timer_index = FIELD_STATUS_TIMER_COUNT - 1; timer_index >= 0; timer_index--)
    {
        record->status_timers[timer_index] = 0;
    }
}

/**
 * @brief Choose and write the field script result derived from chance, actor distance, and status intensity.
 * @param actor_id Actor used for the distance-weighted selection.
 */
void func_800B31CC(s32 actor_id)
{
    u32 chance;
    u8 scratch[32];
    u32 count;
    u32 index;

    chance = FIELD_STATE_CONFIG->chance_percent;
    if ((u32)(rand() % 100) < chance)
    {
        func_800BD520(2, 0xD028, 100);
    }
    else
    {
        count = FIELD_STATE_CONFIG->count_flags >> 4;
        if ((count < 4) || (count >= 8))
        {
            akao_set_song_params(0x74, count, 0, 0);
            func_800BD520(2, 0xD028, 99);
        }
        index = func_800C9ED4(actor_id);
        if (index >= count)
        {
            index = count - 1;
        }
        func_800BD520(2, 0xD028, (((func_800B2A9C(2)->state->status_intensity * count) >> 8) * 6) + index);
    }
}

/**
 * @brief Write a three-value field script result from chance or a configured result row.
 * @param row_index Result-table row; out-of-range rows use the fixed fallback values.
 */
void func_800B32FC(s32 row_index)
{
    s32 chance;

    chance = FIELD_STATE_CONFIG->chance_percent;
    if (((rand() * 100) / (RAND_MAX + 1)) < chance)
    {
        func_800BD520(2, 0xD030, 129);
        func_800BD520(2, 0xD038, 0);
        func_800BD520(2, 0xD040, 100);
    }
    else if (row_index < FIELD_STATUS_RESULT_ROW_COUNT)
    {
        func_800BD520(2, 0xD030, FIELD_STATE_CONFIG->result_rows[row_index][0]);
        func_800BD520(2, 0xD038, FIELD_STATE_CONFIG->result_rows[row_index][1]);
        func_800BD520(2, 0xD040, FIELD_STATE_CONFIG->result_rows[row_index][2]);
    }
    else
    {
        func_800BD520(2, 0xD030, 129);
        func_800BD520(2, 0xD038, 0);
        func_800BD520(2, 0xD040, 99);
    }
}

/**
 * @brief Advance status intensity with a multiplier that decreases each quarter.
 * @param amount Base increment applied to status record 2.
 */
void func_800B3420(s32 amount)
{
    FieldStatusRecord *record;
    FieldStatusState *state;
    u32 value;

    record = func_800B2A9C(2);
    state = record->state;
    value = state->status_intensity;

    switch (value >> 6)
    {
    case 0:
        state->status_intensity = value + (amount * 4);
        break;
    case 1:
        state->status_intensity = value + (amount * 3);
        break;
    case 2:
        state->status_intensity = value + (amount * 2);
        break;
    case 3:
        state->status_intensity = value + amount;
        break;
    }

    if (record->state->status_intensity >= 256)
    {
        record->state->status_intensity = 0;
    }
}
