#include "common.h"
#include "field_calls.h"
#include "game_audio.h"
#include "field_records.h"
#include "field_types.h"
#include "sdk/rand.h"

enum
{
    FIELD_STATUS_PRIMARY_RECORD_COUNT = 3,
    FIELD_STATUS_EFFECT_COUNT = 15,
    FIELD_STATUS_RESULT_ROW_COUNT = 36,
    FIELD_STATUS_MAX_DURATION = 240
};

enum
{
    FIELD_STATUS_APPLY_IGNORE_IMMUNITY = 1 << 0,
    FIELD_STATUS_APPLY_ALLOW_ACTIVE = 1 << 1
};

/** @brief Stat selectors of func_800B2D64 beyond the eight single stats. */
enum
{
    FIELD_STAT_SELECT_ALL = 9,
    FIELD_STAT_SELECT_GROUP_A = 10,
    FIELD_STAT_SELECT_GROUP_B = 11
};

/** @brief Status slot ids FIELD_STATUS_SLOT_ID_BASE + n grant immunity to effect n. */
#define FIELD_STATUS_SLOT_ID_BASE 0x60
#define FIELD_STATUS_SLOT_ID_COUNT 16
#define FIELD_STATUS_TIMED_EFFECT_FLAGS_MASK 0xFFFF

/** @brief Status record that belongs to the third party slot. */
#define FIELD_COMPANION_RECORD_ID 2

/** @brief Item type (FieldItemRecord::info bits 10-15) that adds to a character's footprint strength. */
#define FIELD_ITEM_TYPE_FOOTPRINT_BONUS 7

/** @brief Character type (low seven bits of the info byte) that publishes its region. */
#define FIELD_CHARACTER_TYPE_COMPANION 3

/** @brief Highest level func_800B3670 returns. */
#define FIELD_LEVEL_MAX 99

/** @brief Size of the battle context cleared by func_800B3580. */
#define FIELD_BATTLE_CONTEXT_SIZE 0x4A4

extern FieldGameState* D_80122B74;
extern FieldBattleContext* D_80123FB0;
extern FieldBattleContext D_80123B08;
extern u8* D_80123FAC;
extern s32 D_8010D020;
extern u16 g_music_track_index;

/** @brief Per-effect immunity masks; also indexed by status slot id - FIELD_STATUS_SLOT_ID_BASE. */
extern u8 D_800F0B28[];
/** @brief Per-effect stat pair: high nibble selects the source stat, low nibble the target stat. */
extern u8 D_800F0B38[];
extern u8 D_800F0B48[];
extern u8 D_800F0B50[];
extern u8 D_800EF8C0[];
extern u8 D_800F0AE8[];

s32 func_8008B288(s32 actor_id);
s32 func_80087F44(s32 actor_id, VECTOR* out);
s32 func_80089D44();
FieldStatusState* func_80087F0C(s32 actor_id);
s32 func_8008B500(s32 record_id, s32 signal_id);
u32 func_800BD414(s32 owner_id, s32 variable_id);
void func_800BD520(s32 owner_id, u32 variable_id, s32 value);
void func_800C1EC8(s32 value, void* buffer, s32 size);
u8* func_800C1E40(s32 resource_id);
void func_800B3580(void);
s32 func_800B3670(s32 use_hero_level);
s32 func_800B37D4(void);
void func_800B3D84(void);

/**
 * @brief Find a status record by its identifier.
 * @param record_id Identifier to find.
 * @return Matching status record, or null when no record has the identifier.
 */
FieldStatusRecord* func_800B2A9C(s32 record_id)
{
    s32 i;

    for (i = 0; i < FIELD_BATTLE_RECORD_COUNT; i++)
    {
        if (record_id == D_80123FB0->records[i].meta.bytes.id)
        {
            return &D_80123FB0->records[i];
        }
    }
    record_game_diagnostic(0x8001, 0x68, record_id, -1);
    return NULL;
}

/**
 * @brief Find the first available secondary status record.
 * @return First secondary record that is not active, or null when none is available.
 */
FieldStatusRecord* func_800B2B08(void)
{
    s32 i;

    for (i = FIELD_STATUS_PRIMARY_RECORD_COUNT; i < FIELD_BATTLE_RECORD_COUNT; i++)
    {
        if (!D_80123FB0->records[i].meta.bits.active)
        {
            return &D_80123FB0->records[i];
        }
    }
    return NULL;
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
void func_800B2B54(FieldStatusRecord* source, FieldStatusRecord* target, s32 apply_flags, s32 effect_index, s32 chance_threshold, s32 duration)
{
    s32 effect_mask;
    s32 slot_index;
    s32 attack;
    s32 defense;
    s32 scaled_duration;
    FieldStatusRecord* timer_base;

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
        if (target->immunity_flags & D_800F0B28[effect_index])
        {
            return;
        }
        effect_mask = 0;
        for (slot_index = 0; slot_index < FIELD_STATUS_SLOT_COUNT; slot_index++)
        {
            if (target->status_slots[slot_index] >= FIELD_STATUS_SLOT_ID_BASE &&
                target->status_slots[slot_index] < FIELD_STATUS_SLOT_ID_BASE + FIELD_STATUS_SLOT_ID_COUNT)
            {
                effect_mask |= D_800F0B28[target->status_slots[slot_index] - FIELD_STATUS_SLOT_ID_BASE];
            }
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
    attack = func_800B2D34(source, D_800F0B38[effect_index] >> 4);
    defense = func_800B2D34(target, D_800F0B38[effect_index] & 0xF);
    scaled_duration = duration * attack / defense;
    /* The timer address is formed before the clamp: the record shifted by the timer index. */
    timer_base = (FieldStatusRecord*)((u8*)target + effect_index * sizeof(u16));
    if (scaled_duration > FIELD_STATUS_MAX_DURATION)
    {
        scaled_duration = FIELD_STATUS_MAX_DURATION;
    }
    timer_base->status_timers[0] = scaled_duration;
}

/**
 * @brief Read a status stat with a minimum value of one.
 * @param record Status record to read.
 * @param stat_index Stat index.
 * @return Selected stat, or one when the stat is zero or the index is out of range.
 */
s32 func_800B2D34(FieldStatusRecord* record, s32 stat_index)
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
 * @brief Scale one or more status stats from their base values and optionally signal the actor.
 * @param record Status record to update.
 * @param stat_selector Stat index, FIELD_STAT_SELECT_ALL, or a grouped-stat selector.
 * @param scale Scale in eighths; above eight raises the stats, otherwise lowers them.
 * @param emit_signal Nonzero to store the change signal and notify the actor.
 * @return 0 when the record's state has no current value; the other paths return no defined value.
 */
s32 func_800B2D64(FieldStatusRecord* record, u32 stat_selector, u32 scale, s32 emit_signal)
{
    u32 value;
    s32 stat_index;

    if (record->state->current == 0)
    {
        return 0;
    }
    if (stat_selector < FIELD_STATUS_STAT_COUNT)
    {
        value = (record->base_stats[stat_selector] * scale) >> 3;
        if (scale > 8)
        {
            if (value < record->stats[stat_selector])
            {
                value = record->stats[stat_selector];
            }
            record->stats[stat_selector] = value;
            if (emit_signal != 0)
            {
                record->state->status_signal = D_800F0B50[scale - 8];
                if (record->state->status_signal != 0)
                {
                    func_8008B500(record->meta.bytes.id, stat_selector + 0x9E);
                }
            }
        }
        else
        {
            if (record->stats[stat_selector] < value)
            {
                value = record->stats[stat_selector];
            }
            record->stats[stat_selector] = value;
            if (emit_signal != 0)
            {
                record->state->status_signal = D_800F0B50[8 - scale];
                if (record->state->status_signal != 0)
                {
                    func_8008B500(record->meta.bytes.id, stat_selector + 0xA7);
                }
            }
        }
    }
    else
    {
        switch (stat_selector)
        {
        case FIELD_STAT_SELECT_ALL:
            for (stat_index = 0; stat_index < FIELD_STATUS_STAT_COUNT; stat_index++)
            {
                func_800B2D64(record, stat_index, scale, 0);
            }
            if (emit_signal != 0)
            {
                if (scale > 8)
                {
                    record->state->status_signal = D_800F0B50[scale - 8];
                    func_8008B500(record->meta.bytes.id, 0x9D);
                }
                else
                {
                    record->state->status_signal = D_800F0B50[8 - scale];
                    func_8008B500(record->meta.bytes.id, 0xA6);
                }
            }
            break;
        case FIELD_STAT_SELECT_GROUP_A:
            func_800B2D64(record, 0, scale, 0);
            func_800B2D64(record, 1, scale, 0);
            func_800B2D64(record, 2, scale, 0);
            break;
        case FIELD_STAT_SELECT_GROUP_B:
            func_800B2D64(record, 3, scale, 0);
            func_800B2D64(record, 5, scale, 0);
            func_800B2D64(record, 6, scale, 0);
            break;
        }
    }
}

/**
 * @brief Roll a random byte against the record's final status stat.
 * @param record Status record supplying the threshold.
 * @return 1 when the random byte is below the threshold, otherwise 0.
 */
s32 func_800B2FF8(FieldStatusRecord* record)
{
    u32 threshold;

    threshold = func_800B2D34(record, FIELD_STATUS_STAT_COUNT - 1);
    return (rand() & 0xFF) < threshold;
}

/**
 * @brief Tell whether the first actor is on the side the second actor faces.
 * @param first_actor_id Actor whose position is tested.
 * @param second_actor_id Actor whose position and facing are the reference.
 * @return -1 when the first actor is on the faced side, otherwise 0.
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
        if (direction < 0x40 || direction > 0xC0)
        {
            return 0;
        }
        return -1;
    }
    else
    {
        if (direction < 0x40 || direction > 0xC0)
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
void func_800B30B8(FieldStatusState* state, s32 amount)
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
void saturating_counter_add(FieldStatusState* state, s32 delta)
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
void func_800B313C(FieldStatusRecord* record)
{
    func_80089D44(record->meta.bytes.id);
}

/**
 * @brief Clear one timed status effect or all timed status effects.
 * @param record Status record to update.
 * @param index Effect index, or an out-of-range value to clear every timed effect.
 */
void field_clear_record_state(FieldStatusRecord* record, u32 index)
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
    u8 unused[32]; /* never used; the original stack frame reserves it */
    u32 count;
    u32 index;

    chance = D_80122B74->characters[2].unk150[0].derived.bytes[2];
    if (rand() % 100 < chance)
    {
        func_800BD520(2, 0xD028, 100);
    }
    else
    {
        count = D_80122B74->characters[2].unk150[0].derived.bytes[0] >> 4;
        if (count < 4 || count >= 8)
        {
            record_game_diagnostic(0x74, count, 0, 0);
            func_800BD520(2, 0xD028, 99);
        }
        index = func_800C9ED4(actor_id);
        if (index >= count)
        {
            index = count - 1;
        }
        func_800BD520(2, 0xD028, ((func_800B2A9C(FIELD_COMPANION_RECORD_ID)->state->status_intensity * count) >> 8) * 6 + index);
    }
}

/**
 * @brief Write a three-value field script result from chance or a configured result row.
 * @param row_index Result-table row; out-of-range rows use the fixed fallback values.
 */
void func_800B32FC(s32 row_index)
{
    s32 chance;

    chance = D_80122B74->characters[2].unk150[0].derived.bytes[2];
    if (rand() * 100 / (RAND_MAX + 1) < chance)
    {
        func_800BD520(2, 0xD030, 129);
        func_800BD520(2, 0xD038, 0);
        func_800BD520(2, 0xD040, 100);
    }
    else if (row_index < FIELD_STATUS_RESULT_ROW_COUNT)
    {
        func_800BD520(2, 0xD030, D_80122B74->result_rows[row_index][0]);
        func_800BD520(2, 0xD038, D_80122B74->result_rows[row_index][1]);
        func_800BD520(2, 0xD040, D_80122B74->result_rows[row_index][2]);
    }
    else
    {
        func_800BD520(2, 0xD030, 129);
        func_800BD520(2, 0xD038, 0);
        func_800BD520(2, 0xD040, 99);
    }
}

/**
 * @brief Advance the companion's status intensity with a multiplier that decreases each quarter.
 * @param amount Base increment.
 */
void func_800B3420(s32 amount)
{
    FieldStatusRecord* record;
    FieldStatusState* state;
    u32 value;

    record = func_800B2A9C(FIELD_COMPANION_RECORD_ID);
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

/**
 * @brief Rebuild the battle context and publish the party and monster counts, or call func_800B4390 when @p group is 0.
 *
 * Script variables 0x4280 and 0x4284 hold the party and monster counts
 * that func_800B48B8 updates and func_800B62D8 tests for zero. With
 * D_8010D020 set both are written as 1.
 *
 * @param group Monster group to build; 0 selects func_800B4390 instead.
 */
void func_800B34D0(s32 group)
{
    s32 count;

    if (group != 0)
    {
        func_800B3580();
        count = func_800B37D4();
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4280, 1);
        }
        else
        {
            func_800BD520(0, 0x4280, count);
        }
        count = func_800B3DF4(group);
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4284, 1);
        }
        else
        {
            func_800BD520(0, 0x4284, count);
        }
    }
    else
    {
        func_800B4390();
    }
}

/**
 * @brief Clear the battle context, then fill its header from the current land and resource 1.
 */
void func_800B3580(void)
{
    s32 i;
    u8* resource;

    D_80123FAC = D_800EF8C0;
    D_80123FB0 = &D_80123B08;
    func_800C1EC8(0, &D_80123B08, FIELD_BATTLE_CONTEXT_SIZE);
    D_80123FB0->action = NULL;
    D_80123FB0->state.level = func_800B3670(0);

    for (i = 0; i < 8; i++)
    {
        D_80123FB0->element_levels[i] = D_800F0B48[D_80122B74->lands[g_music_track_index].levels[i]];
    }

    resource = func_800C1E40(1);
    D_80123FB0->templates = resource + *(s32*)(resource + 4);
    D_80123FB0->resources = resource + *(s32*)(resource + 8);
    func_800BD520(0, 0x428C, -1);
}

/**
 * @brief Compute the monster level from the hero's level or the land, clamped by script variables.
 *
 * The base comes from the hero's level when @p use_hero_level or bit 7 of
 * script variable 0x52F0 is set, otherwise from func_800C3688 for the
 * current land. Script variable 0x2938 adds 20 (mode 1) or forces 63
 * (mode 2). The result indexes D_800F0AE8 and is clamped to script
 * variables 0x52E0..0x52E8 and to FIELD_LEVEL_MAX.
 *
 * @param use_hero_level Nonzero to base the level on the hero's level.
 * @return Level in 0..99.
 */
s32 func_800B3670(s32 use_hero_level)
{
    s32 flag;
    s32 mode;
    s32 index;
    s32 value;
    u32 lo;
    u32 hi;

    flag = use_hero_level;
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
            index = D_80122B74->control.fields.hero_level + 20;
            break;
        case 2:
            index = 63;
            break;
        default:
            index = D_80122B74->control.fields.hero_level;
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
            index += 20;
            break;
        case 2:
            index = 63;
            break;
        }
    }

    if (index >= 64)
    {
        index = 63;
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

    if (value > FIELD_LEVEL_MAX)
    {
        value = FIELD_LEVEL_MAX;
    }
    return value;
}

/**
 * @brief Build the status records of the party characters.
 * @return Number of party slots in use.
 */
s32 func_800B37D4(void)
{
    s32 active_count;
    s32 index;
    s32 stat_index;
    s32 equipment_index;
    s32 ally;
    s8 stat;
    s32 party_index;
    u32 strength;
    u32 nibbles;

    party_index = 0;
    active_count = 0;
    do
    {
        if (D_80122B74->characters[party_index].name[0] != 0)
        {
            if (D_8010D020 != 0 && party_index == 0)
            {
                D_80123FB0->records[0].unk0 |= 0x40;
            }
            else
            {
                D_80123FB0->records[party_index].unk0 |= 0x80;
            }
            D_80123FB0->records[party_index].unk3 = 0xF;
            D_80123FB0->records[party_index].meta.bytes.id = party_index;
            D_80123FB0->records[party_index].meta.bits.active = 1;
            if (D_8010D020 != 0)
            {
                D_80123FB0->records[party_index].meta.bits.ally = (party_index == 0) ? 0xFF : 0;
            }
            else
            {
                D_80123FB0->records[party_index].meta.bits.ally = 1;
            }
            {
                FieldBattleContext* context = D_80123FB0;

                context->records[party_index].meta.bits.kind = D_80122B74->characters[party_index].info.bytes[0];
                context->records[party_index].counter = 5;
                context->records[party_index].meta.bytes.unk2 = 0;
            }
            D_80123FB0->records[party_index].counter_reset = 5;
            D_80123FB0->records[party_index].status_flags = 0;
            D_80123FB0->records[party_index].unkC = 0;
            D_80123FB0->records[party_index].state = func_80087F0C(party_index);
            stat_index = 0;
            D_80123FB0->records[party_index].unk18 = D_80122B74->characters[party_index].equipment->derived.values[0];
            D_80123FB0->records[party_index].unk1A = 25;
            do
            {
                index = 1;
                D_80123FB0->records[party_index].equipment_stats[stat_index] = 0;
                D_80123FB0->records[party_index].equipment_attributes[stat_index] = D_80122B74->characters[party_index].equipment->attributes[stat_index];
                do
                {
                    if (D_80122B74->characters[party_index].equipment[index].kind != 0)
                    {
                        FieldBattleContext* context = D_80123FB0;

                        context->records[party_index].equipment_stats[stat_index] +=
                            (D_80122B74->characters[party_index].equipment + index)->derived.values[stat_index];
                        context->records[party_index].equipment_attributes[stat_index] +=
                            (D_80122B74->characters[party_index].equipment + index)->attributes[stat_index];
                    }
                    index += 1;
                } while (index < FIELD_EQUIPMENT_SLOT_COUNT);
                stat_index += 1;
            } while (stat_index < 4);
            index = 0;
            func_800B4934(&D_80123FB0->records[party_index]);
            nibbles = D_80122B74->characters[party_index].equipment->bonus_nibbles.word;
            do
            {
                stat = func_800B7EE8(&D_80122B74->characters[party_index], index);
                {
                    FieldBattleContext* context = D_80123FB0;

                    context->records[party_index].base_stats[index] = stat;
                    context->records[party_index].stats[index] = stat;
                }
                D_80123FB0->records[party_index].unk3C[index] = nibbles & 0xF;
                nibbles >>= 4;
                D_80123FB0->records[party_index].unk3C[index] += D_80123FB0->element_levels[index];
                index += 1;
            } while (index < FIELD_STATUS_STAT_COUNT);
            equipment_index = 1;
            /* index is 8 here, so this clears unk4C (rewritten below). */
            D_80123FB0->records[party_index].unk44[index] = 0;
            D_80123FB0->records[party_index].immunity_flags = 0;
            D_80123FB0->records[party_index].unk39 = 0;
            D_80123FB0->records[party_index].unk3A = 0;
            do
            {
                if (D_80122B74->characters[party_index].equipment[equipment_index].kind != 0)
                {
                    nibbles = (D_80122B74->characters[party_index].equipment + equipment_index)->bonus_nibbles.word;
                    for (index = 0; index < FIELD_STATUS_STAT_COUNT; index++)
                    {
                        D_80123FB0->records[party_index].unk44[index] += nibbles & 0xF;
                        nibbles >>= 4;
                    }
                    D_80123FB0->records[party_index].immunity_flags |= (D_80122B74->characters[party_index].equipment + equipment_index)->flags2C;
                    D_80123FB0->records[party_index].unk3A |= (D_80122B74->characters[party_index].equipment + equipment_index)->flags2D;
                }
                equipment_index += 1;
            } while (equipment_index < FIELD_EQUIPMENT_SLOT_COUNT);
            D_80123FB0->records[party_index].template = NULL;
            D_80123FB0->records[party_index].unk4C = D_80122B74->characters[party_index].unk43;
            if (D_8010D020 != 0)
            {
                D_80123FB0->records[party_index].state->maximum = D_80122B74->characters[party_index].hp * 3;
                D_80123FB0->records[party_index].state->current = D_80122B74->characters[party_index].hp * 3;
                D_80123FB0->records[party_index].state->gauge.bits.value = D_80122B74->characters[party_index].hp * 3;
                D_80123FB0->records[party_index].state->gauge.bits.hud_bits = 0;
            }
            else
            {
                D_80123FB0->records[party_index].state->maximum = D_80122B74->characters[party_index].hp;
            }
            {
                FieldStatusState* state;

                strength = D_80123FB0->records[party_index].stats[3] * 2;
                if (((D_80122B74->characters[party_index].equipment[0].info.word >> 10) & 0x3F) == FIELD_ITEM_TYPE_FOOTPRINT_BONUS)
                {
                    strength += 0x80;
                }
                state = D_80123FB0->records[party_index].state;
                if (strength < 0x100)
                {
                    state->effect_footprint_strength = strength;
                }
                else
                {
                    state->effect_footprint_strength = 0xFF;
                }
            }
            if ((D_80122B74->characters[party_index].info.bytes[0] & 0x7F) == FIELD_CHARACTER_TYPE_COMPANION)
            {
                func_800B3D84();
            }
            active_count += 1;
        }
        party_index += 1;
    } while (party_index < FIELD_PARTY_SIZE);
    return active_count;
}

/**
 * @brief Report an out-of-range region index, then publish the current region's value.
 */
void func_800B3D84(void)
{
    if (D_80122B74->region_index < 0 || D_80122B74->region_index >= FIELD_REGION_COUNT)
    {
        record_game_diagnostic(0x8001, 0x75, D_80122B74->region_index, 0);
    }
    func_800BD520(2, 0xF020, D_80122B74->regions[D_80122B74->region_index].unk48.word);
}
