#include "game_audio.h"
#include "field_state_ops.h"
#include "field_types.h"
#include "sdk/rand.h"


extern u8 * D_80122B74;
extern u8 * D_80123FB0;
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

#define FIELD_STATE_CONFIG (*(FieldStateConfig **)&D_80122B74)

s32 func_8008B288(s32 actor_id);
s32 func_80087F44(s32 actor_id, VECTOR *out);
s32 func_80089D44();

void func_800BD520(s32 owner_id, u32 variable_id, s32 value);
u32 func_800C9ED4(s32 actor_id);
s32 func_8008B500(s32 record_id, s32 signal_id);

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
    record_game_diagnostic(0x8001, 0x68, record_id, -1);
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
    VECTOR first_position;
    VECTOR second_position;

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
 * @param amount Amount to subtract; negative values are reported as diagnostics.
 */
void func_800B30B8(FieldStatusState *state, s32 amount)
{
    s32 remaining;

    if (amount < 0)
    {
        record_game_diagnostic(0x8001, 0x7A, state->actor_id, amount);
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
            record_game_diagnostic(0x74, count, 0, 0);
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

#include "game_audio.h"
#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 *unk4;
    u8 *unk8;
    u8 unkC[8];
    s32 unk14;
    s32 unk18;
} StructB3580;

/** @brief View of the D_80122B74 block: a byte at 0x2E5 and 0xC-byte rows at 0x2F4. */
typedef struct
{
    u8 pad[0x2E5];
    u8 unk2E5;
    u8 pad2E6[0x2F4 - 0x2E6];
    u8 unk2F4[1][0xC];
} StructB74;

#define FIELD_B74 ((StructB74 *)D_80122B74)

s32 func_800B37D4(void);
s32 func_800B3DF4(s32);
void func_800B4390(void);
void func_800C1EC8(s32, void *, s32);
u8 *func_800C1E40(s32);
u32 func_800BD414(s32, s32);
s32 func_800C3688(s32);
void func_800B3580(void);
s32 func_800B3670(s32);

extern s32 D_8010D020;
extern u8 D_800EF8C0[];
extern u8 D_800F0B48[];
extern u8 D_800F0AE8[];
extern StructB3580 D_80123B08;
extern u8 *D_80123FAC;
extern u16 g_music_track_index;

/**
 * @brief Rebuild the D_80123B08 block and write script variables 0x4280 and 0x4284, or call func_800B4390 when arg0 is 0.
 *
 * With D_8010D020 set both variables are written as 1 instead of the
 * computed values. 0x4280 and 0x4284 are the counter pair that func_800B48B8
 * increments and func_800B62D8 tests for zero.
 *
 * @param arg0 Nonzero selects the rebuild path and is forwarded to func_800B3DF4.
 * @see decomp.me (100%)
 */
void func_800B34D0(s32 arg0)
{
    s32 value;

    if (arg0 != 0)
    {
        func_800B3580();
        value = func_800B37D4();
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4280, 1);
        }
        else
        {
            func_800BD520(0, 0x4280, value);
        }
        value = func_800B3DF4(arg0);
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4284, 1);
        }
        else
        {
            func_800BD520(0, 0x4284, value);
        }
    }
    else
    {
        func_800B4390();
    }
}

/**
 * @brief Zero the D_80123B08 block, then fill it from the current track's 0xC-byte layout record and resource 1.
 * @see decomp.me (100%)
 */
void func_800B3580(void)
{
    s32 i;
    u8 *p;

    D_80123FAC = D_800EF8C0;
    (*(StructB3580 **)&D_80123FB0) = &D_80123B08;
    func_800C1EC8(0, &D_80123B08, 0x4A4);
    (*(StructB3580 **)&D_80123FB0)->unk18 = 0;
    (*(StructB3580 **)&D_80123FB0)->unk0 = func_800B3670(0);

    for (i = 0; i < 8; i++)
    {
        (*(StructB3580 **)&D_80123FB0)->unkC[i] = D_800F0B48[FIELD_B74->unk2F4[g_music_track_index][i]];
    }

    p = func_800C1E40(1);
    (*(StructB3580 **)&D_80123FB0)->unk4 = p + *(s32 *)(p + 4);
    (*(StructB3580 **)&D_80123FB0)->unk8 = p + *(s32 *)(p + 8);
    func_800BD520(0, 0x428C, -1);
}

/**
 * @brief Look up D_800F0AE8 by a 0..0x3F index and clamp the result to script variables 0x52E0..0x52E8 and 0x63.
 *
 * The index comes from the byte at 0x2E5 of the layout buffer when @p arg0 or
 * bit 7 of script variable 0x52F0 is set, otherwise from func_800C3688 for the
 * current track. Script variable 0x2938 adds 0x14 (mode 1) or forces 0x3F
 * (mode 2).
 *
 * @param arg0 Nonzero selects the byte-at-0x2E5 index.
 * @return Value in 0..0x63.
 * @see decomp.me (100%)
 */
s32 func_800B3670(s32 arg0)
{
    s32 flag;
    s32 mode;
    s32 index;
    s32 value;
    u32 lo;
    u32 hi;

    flag = arg0;
    if (func_800BD414(0, 0x52F0) & 0x80)
    {
        flag = 1;
    }
    mode = func_800BD414(0, 0x2938);

    if (flag != 0)
    {
        switch (mode)
        {
            case 1:
                index = FIELD_B74->unk2E5 + 0x14;
                break;
            case 2:
                index = 0x3F;
                break;
            default:
                index = FIELD_B74->unk2E5;
                break;
        }
        index = (index * 3) / 2;
    }
    else
    {
        index = func_800C3688(g_music_track_index);
        switch (mode)
        {
            case 1:
                index += 0x14;
                break;
            case 2:
                index = 0x3F;
                break;
        }
    }

    if (index >= 0x40)
    {
        index = 0x3F;
    }

    value = D_800F0AE8[index];
    lo = func_800BD414(0, 0x52E0);
    hi = func_800BD414(0, 0x52E8);
    if (value < lo)
    {
        value = lo;
    }
    else if (value > hi)
    {
        value = hi;
    }

    if (value >= 0x64)
    {
        value = 0x63;
    }
    return value;
}



extern s32 D_8010D020;
extern u8 *func_80087F0C(s32);
extern void func_800B3D84(void);
extern void func_800B4934(u8 *);
extern s32 func_800B7EE8(u8 *, s32);

typedef struct
{
    u8 pad0[0xC];
    u8 base_attributes[8];
    u8 pad14[0x14];
    u8 flags;
    u8 pad29[2];
    u8 unk2B;
    union
    {
        u32 word;
        struct
        {
            u8 index;
            u8 flags;
            u16 upper;
        } parts;
    } config;
    u8 unk30;
    u8 unk31;
    u16 unk32;
    u32 unk34;
    u8 *actor;
    u32 unk3C;
    u16 unk40;
    u8 unk42;
    u8 pad43;
    u16 equipment_stats[4];
    u8 equipment_attributes[4];
    u8 attributes[8];
    u8 base_values[8];
    u8 flags60;
    u8 flags61;
    u8 flags62;
    u8 pad63;
    u8 modifiers[8];
    u8 bonuses[8];
    u8 unk74;
} PartyActorView;
typedef struct
{
    u8 pad0[0x5F0];
    u8 active;
    u8 pad5F1[0x17];
    u8 type;
    u8 pad609[0xB];
    u16 hp;
    u8 pad616[0x1D];
    u8 unk633;
    u8 pad634[0xC];
    u8 equipment_active;
    u8 pad641[0x13];
    u32 equipment_config;
    u32 equipment_modifiers;
    u8 pad65C[8];
    u16 equipment_stat;
    u8 pad666[6];
    u8 flags66C;
    u8 flags66D;
    u8 pad66E[2];
    u8 equipment_attribute;
} PartySaveView;
typedef struct
{
    u8 pad0[0x24];
    u16 stat;
    u8 pad26[0xA];
    u8 attribute;
} PartyEquipmentView;
typedef struct
{
    s32 hp;
    s32 max_hp;
    u32 flags;
    u8 padC[0x5C];
    u16 effect_footprint_strength;
} PartyLiveActorView;

/**
 * @brief Initialize the three party actor records and their derived attributes.
 * @see decomp.me (100%)
 */
s32 func_800B37D4(void)
{
    s32 active_count;
    s32 equipment_address;
    s32 attribute_offset;
    s32 active_flags;
    s32 config_flags;
    u32 actor_type;
    s32 index;
    s32 stat_index;
    s32 equipment_index;
    s32 player_control;
    s8 attribute_value;
    s32 party_index;
    s8 actor_flags;
    u32 capacity;
    u32 packed_modifiers;
    u8 *config_record;
    u8 *linked_record;
    u8 *modifier_record;
    u8 *flags60_record;
    u8 *flags62_record;
    u8 *live_flags;
    u8 *setup_record;
    u8 *stat_record;
    u8 *hp_source;
    u8 *capacity_record;
    u8 *attribute_record;
    u8 *hp_record;
    u8 *live_hp;
    u8 *actor;
    u8 *value_record;
    u8 *final_record;
    u8 *live_capacity;
    u8 *flags_record;
    party_index = 0;
    active_count = 0;
    do
    {
        if (((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->active != 0)
        {
            if ((D_8010D020 != 0) && (party_index == 0))
            {
                flags_record = (u8 *)(*(StructB3580 **)&D_80123FB0);
                actor_flags = ((PartyActorView *)flags_record)->flags | 0x40;
            }
            else
            {
                flags_record = (u8 *)(*(StructB3580 **)&D_80123FB0);
                flags_record += party_index * 0x68;
                actor_flags = ((PartyActorView *)flags_record)->flags | 0x80;
            }
            ((PartyActorView *)flags_record)->flags = actor_flags;
            ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + ((party_index * 0xD) << 3)))->unk2B = 0xF;
            ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + ((party_index * 0xD) << 3)))->config.parts.index =
                party_index;
            config_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + (party_index * 0x68);
            config_flags = ((PartyActorView *)config_record)->config.word;
            active_flags = config_flags | 0x100;
            ((PartyActorView *)config_record)->config.word = active_flags;
            if (D_8010D020 != 0)
            {
                player_control = 0;
                if (party_index == 0)
                {
                    player_control = 0xFF;
                }
                ((PartyActorView *)config_record)->config.word =
                    (s32)((active_flags & (~0x200)) | ((player_control & 1) << 9));
            }
            else
            {
                ((PartyActorView *)config_record)->config.word = (s32)(config_flags | 0x300);
            }
            setup_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + (party_index * 0x68);
            actor_type = ((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->type;
            ((PartyActorView *)setup_record)->config.word =
                (s32)((((PartyActorView *)setup_record)->config.word & 0xFFFF03FF) |
                      ((actor_type & 0x3F) << 10));
            ((PartyActorView *)setup_record)->unk30 = 5;
            ((PartyActorView *)setup_record)->config.parts.upper = 0;
            ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + ((party_index * 0xD) << 3)))->unk31 = 5;
            actor = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + (party_index * 0x68);
            ((PartyActorView *)actor)->unk32 = 0;
            ((PartyActorView *)actor)->unk34 = 0;
            actor = func_80087F0C(party_index);
            stat_index = 0;
            linked_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + (party_index * 0x68);
            ((PartyActorView *)linked_record)->actor = actor;
            ((PartyActorView *)linked_record)->unk40 =
                (u16)((PartySaveView *)(((party_index * 0x25) << 4) + ((s32)D_80122B74)))->equipment_stat;
            ((PartyActorView *)linked_record)->unk42 = 0x19;
            do
            {
                index = 1;
                ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + (((party_index * 0xD) << 3) + (stat_index << 1))))
                    ->equipment_stats[0] = 0;
                ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + (((party_index * 0xD) << 3) + stat_index)))
                    ->equipment_attributes[0] =
                    (u8)((PartySaveView *)((((party_index * 0x25) << 4) + ((s32)D_80122B74)) + stat_index))
                        ->equipment_attribute;
                do
                {
                    if (((PartySaveView *)(D_80122B74 + (((party_index * 0x25) << 4) + (index * 0x40))))
                            ->equipment_active != 0)
                    {
                        stat_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + ((stat_index << 1) + (party_index * 0x68));
                        attribute_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + ((party_index * 0x68) + stat_index);
                        equipment_address =
                            (s32)(((((party_index * 0x25) << 4) + ((s32)D_80122B74)) + (index << 6)) + 0x640);
                        ((PartyActorView *)stat_record)->equipment_stats[0] =
                            (u16)(((PartyActorView *)stat_record)->equipment_stats[0] +
                                  ((PartyEquipmentView *)(equipment_address + (stat_index << 1)))->stat);
                        ((PartyActorView *)attribute_record)->equipment_attributes[0] =
                            (u8)(((PartyActorView *)attribute_record)->equipment_attributes[0] +
                                 ((PartyEquipmentView *)(equipment_address + stat_index))->attribute);
                    }
                    index += 1;
                } while (index < 4);
                stat_index += 1;
            } while (stat_index < 4);
            index = 0;
            func_800B4934(((u8 *)(*(StructB3580 **)&D_80123FB0)) + ((party_index * 0x68) + 0x28));
            packed_modifiers =
                ((PartySaveView *)(((party_index * 0x25) << 4) + ((s32)D_80122B74)))->equipment_modifiers;
            do
            {
                attribute_value = func_800B7EE8(D_80122B74 + (((party_index * 0x25) << 4) + 0x5F0), index);
                attribute_offset = index + ((party_index * 0xD) << 3);
                value_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + attribute_offset;
                ((PartyActorView *)value_record)->base_values[0] = attribute_value;
                ((PartyActorView *)value_record)->attributes[0] = attribute_value;
                ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + attribute_offset))->modifiers[0] =
                    (s8)(packed_modifiers & 0xF);
                packed_modifiers = packed_modifiers >> 4;
                modifier_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + attribute_offset;
                ((PartyActorView *)modifier_record)->modifiers[0] +=
                    ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + index))->base_attributes[0];
                index += 1;
            } while (index < 8);
            equipment_index = 1;
            ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + (index + ((party_index * 0xD) << 3))))->bonuses[0] = 0;
            ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + ((party_index * 0xD) << 3)))->flags60 = 0;
            ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + ((party_index * 0xD) << 3)))->flags61 = 0;
            ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + ((party_index * 0xD) << 3)))->flags62 = 0;
            do
            {
                if (((PartySaveView *)(D_80122B74 + (((party_index * 0x25) << 4) + (equipment_index << 6))))
                        ->equipment_active != 0)
                {
                    packed_modifiers = ((PartySaveView *)((((party_index * 0x25) << 4) + ((s32)D_80122B74)) +
                                                          (equipment_index << 6)))
                                           ->equipment_modifiers;
                    for (index = 0; index < 8; index++)
                    {
                        ((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) - (-(index + (party_index * 0x68)))))
                            ->bonuses[0] += packed_modifiers & 0xF;
                        packed_modifiers >>= 4;
                    }

                    flags60_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + (party_index * 0x68);
                    ((PartyActorView *)flags60_record)->flags60 =
                        (u8)(((PartyActorView *)flags60_record)->flags60 |
                             ((PartySaveView *)((((party_index * 0x25) << 4) + ((s32)D_80122B74)) +
                                                (equipment_index << 6)))
                                 ->flags66C);
                    flags62_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + (party_index * 0x68);
                    ((PartyActorView *)flags62_record)->flags62 =
                        (u8)(((PartyActorView *)flags62_record)->flags62 |
                             ((PartySaveView *)((((party_index * 0x25) << 4) + ((s32)D_80122B74)) +
                                                (equipment_index << 6)))
                                 ->flags66D);
                }
                equipment_index += 1;
            } while (equipment_index < 4);
            final_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + (party_index * 0x68);
            ((PartyActorView *)final_record)->unk3C = 0;
            ((PartyActorView *)final_record)->unk74 =
                (u8)((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->unk633;
            if (D_8010D020 != 0)
            {
                hp_source = D_80122B74 + ((party_index * 0x25) << 4);
                hp_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + (party_index * 0x68);
                *((s32 *)((PartyActorView *)hp_record)->actor) = (s32)(((PartySaveView *)hp_source)->hp * 3);
                ((PartyLiveActorView *)((PartyActorView *)hp_record)->actor)->max_hp =
                    (s32)(((PartySaveView *)hp_source)->hp * 3);
                live_hp = ((PartyActorView *)hp_record)->actor;
                ((PartyLiveActorView *)live_hp)->flags =
                    (s32)((((PartyLiveActorView *)live_hp)->flags & 0xFF000000) |
                          (((PartySaveView *)hp_source)->hp * 3));
                live_flags = ((PartyActorView *)hp_record)->actor;
                ((PartyLiveActorView *)live_flags)->flags =
                    (s32)(((PartyLiveActorView *)live_flags)->flags & 0x80FFFFFF);
            }
            else
            {
                *((s32 *)((PartyActorView *)(((u8 *)(*(StructB3580 **)&D_80123FB0)) + ((party_index * 0xD) << 3)))->actor) =
                    (s32)((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->hp;
            }
            capacity_record = ((u8 *)(*(StructB3580 **)&D_80123FB0)) + (party_index * 0x68);
            capacity = ((PartyActorView *)capacity_record)->attributes[3] * 2;
            if (((((u32)((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->equipment_config) >>
                  0xA) &
                 0x3F) == 7)
            {
                capacity += 0x80;
            }
            live_capacity = ((PartyActorView *)capacity_record)->actor;
            if (capacity < 0x100U)
            {
                ((PartyLiveActorView *)live_capacity)->effect_footprint_strength = capacity;
            }
            else
            {
                ((PartyLiveActorView *)live_capacity)->effect_footprint_strength = 0xFFU;
            }
            if ((((PartySaveView *)(D_80122B74 + ((party_index * 0x25) << 4)))->type & 0x7F) == 3)
            {
                func_800B3D84();
            }
            active_count += 1;
        }
        party_index += 1;
    } while (party_index < 3);
    return active_count;
}





/**
 * @brief Report an invalid scene index, then forward the selected scene value.
 *
 * Reads the active scene index at offset 0x2EF0 of the D_80122B74 buffer; if it
 * is 5 or greater it records a diagnostic, then forwards the scene
 * entry's 0x2F3C word (stride 0x60) to func_800BD520.
 *
 * Matches under GCC 2.8.0. The pre-diagnostic scene index and the index
 * reloaded afterward are distinct value webs; materializing the second
 * index's 0x60-byte offset reproduces the target allocation exactly.
 */
void func_800B3D84(void)
{
    s32 idx1;
    s32 idx2;
    s32 off;

    idx1 = *(s32 *)(D_80122B74 + 0x2EF0);
    if ((u32)idx1 >= 5)
    {
        record_game_diagnostic(0x8001, 0x75, idx1, 0);
    }

    idx2 = *(s32 *)(D_80122B74 + 0x2EF0);
    off = idx2 * 0x60;
    func_800BD520(2, 0xF020, *(s32 *)(D_80122B74 + off + 0x2F3C));
}
