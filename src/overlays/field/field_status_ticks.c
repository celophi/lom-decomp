#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

extern FieldBattleContext *D_80123FB0;
extern FieldRuntimeContext *D_80122B78;

s32 func_800BD414(s32 arg0, s32 arg1);
void func_800BD520(s32 arg0, u32 arg1, s32 arg2);

/**
 * @brief Count one more battle for the ally or the enemy side in script variables 0x4280 / 0x4284.
 */
void func_800B48B8(void)
{
    s32 count;

    if ((D_80123FB0 != NULL) && (D_80123FB0->state.flags >= 0))
    {
        /* No argument: the original leaves $a0 as the caller set it. 0x200 is meta.bits.ally. */
        if (((FieldStatusRecord *(*)(void))func_800B2A9C)()->meta.packed & 0x200)
        {
            count = func_800BD414(0, 0x4280);
            func_800BD520(0, 0x4280, count + 1);
        }
        else
        {
            count = func_800BD414(0, 0x4284);
            func_800BD520(0, 0x4284, count + 1);
        }
    }
}

extern u16 D_800F0B58[];
extern u8 *D_80122B74;

/**
 * @brief Rebuild an actor's 16-bit status mask from its four sub-entries.
 *
 * Clears the actor's 0xA mask, then walks the four 0x40-byte records that begin
 * at offset 0x5F0 of the actor's per-index block (stride 0x250) inside the table
 * pointed to by @c D_80122B74. For each active record (byte 0 non-zero) it ORs in
 * the 16-bit flag looked up in @c D_800F0B58 by the record's 0x2E field.
 *
 * @param record Status record whose id selects the block and whose status_flags are rebuilt.
 */
void func_800B4934(FieldStatusRecord *record)
{
    s32 i;
    s32 off;
    u8 *base;
    u8 *rec;
    u16 *tbl;

    i = 0;
    tbl = D_800F0B58;
    record->status_flags = 0;
    off = 0x50;
    base = D_80122B74 + (record->meta.bytes.id * 0x250 + 0x5F0);
    do
    {
        rec = base + off;
        if (*rec != 0)
        {
            record->status_flags |= tbl[*(u16 *)(rec + 0x2E)];
        }
        i += 1;
        off += 0x40;
    } while (i < 4);
}

void func_800B4B44(void);
void func_800B4D1C(FieldStatusRecord *record);
void func_800B4DF0(FieldStatusRecord *record);
void func_800B4E60(FieldStatusRecord *record);
void func_800B4F38(FieldStatusRecord *record);
void func_800B4F80(FieldStatusRecord *record);

/**
 * @brief Run two update passes over the active field status records.
 */
void func_800B49C0(void)
{
    s32 i;
    s32 keep;

    if ((D_80123FB0 != NULL) && (D_80123FB0->state.flags >= 0))
    {
        i = 0;
        func_800B4B44();
        do
        {
            if (D_80123FB0->records[i].meta.bits.active)
            {
                if (func_800BD414(0, 0xFFD) == 0)
                {
                    keep = i < 3;
                }
                else
                {
                    if (D_80123FB0->records[i].meta.bytes.id < 2)
                    {
                        D_80123FB0->records[i].state->status_intensity = 0xFF;
                    }
                    keep = i < 3;
                }
                if (keep)
                {
                    func_800B4D1C(&D_80123FB0->records[i]);
                    func_800B4F80(&D_80123FB0->records[i]);
                }
                func_800B4DF0(&D_80123FB0->records[i]);
            }
            i++;
        } while (i < 11);

        i = 0;
        if ((D_80122B78->frame_count & 0xF) == 0)
        {
            do
            {
                if (D_80123FB0->records[i].meta.bits.active)
                {
                    func_800B4E60(&D_80123FB0->records[i]);
                    func_800B4F38(&D_80123FB0->records[i]);
                }
                i++;
            } while (i < 11);
        }
    }
}

u32 func_800B4CE4(FieldStatusRecord *record, s32 status);

/**
 * @brief Rebuild indexed field-state byte mappings and trigger dependent handlers.
 */
void func_800B4B44(void)
{
    s32 i;
    s32 j;
    u8 *list;
    u8 *other;
    s32 first;
    s32 second;
    s32 current;

    i = 0;
    do
    {
        j = 0;
        do
        {
            D_80123FB0->records[i].status_slots[j] = 0;
            j++;
        } while (j < 3);
        i++;
    } while (i < 3);

    i = 0;
    list = func_800A2E34();
    while (*list != 0xFF)
    {
        other = list + 1;
        current = *list;
        first = *other;
        D_80123FB0->records[current].status_slots[i] = D_80123FB0->records[first].unk4C;
        second = *other;
        other += 2;
        first = *list;
        list += 2;
        D_80123FB0->records[second].status_slots[i] = D_80123FB0->records[first].unk4C;
        i++;
    }

    if (func_800B4CE4(&D_80123FB0->records[1], 0) < 3)
    {
        func_800B28E0(1, 0xC, 6);
    }
    if (func_800B4CE4(&D_80123FB0->records[2], 0) < 3)
    {
        func_800B28E0(2, 0xC, 6);
    }
    if (i >= 4)
    {
        record_game_diagnostic(0x8001, 0x6F, i, 0);
    }
}

/**
 * @brief Count the status slots of a record that hold a given status.
 * @param record Status record to scan.
 * @param status Status id to count.
 * @return Number of the FIELD_STATUS_SLOT_COUNT slots equal to @p status.
 */
u32 func_800B4CE4(FieldStatusRecord *record, s32 status)
{
    u32 count;
    u32 i;

    i = 0;
    count = i;
    for (; i < 3; i++)
    {
        if (record->status_slots[i] == status)
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief Clear selected record state and apply active field record actions.
 * @param record Status record whose active statuses are processed.
 */
void func_800B4D1C(FieldStatusRecord *record)
{
    s32 i;
    s32 j;

    if (func_800B4CE4(record, 5) != 0)
    {
        field_clear_record_state(record, 0xFF);
    }

    i = 0x60;
    do
    {
        if (func_800B4CE4(record, i) != 0)
        {
            field_clear_record_state(record, i - 0x60);
        }
        i++;
    } while (i < 0x6C);

    if (func_800B4CE4(record, 6) != 0)
    {
        if (record->meta.bytes.id == 0)
        {
            record->state->status_intensity = 0xFF;
        }
    }

    j = 0x70;
    do
    {
        if (func_800B4CE4(record, j) != 0)
        {
            func_800B2D64(record, j - 0x70, 0xA, 0);
        }
        j++;
    } while (j < 0x80);
}

/**
 * @brief Ticks a record's twelve state timers and clears any that expire.
 *
 * For each of the twelve half-word timers at record offset 0x50, decrements a
 * nonzero timer and, when it reaches zero or below, clears that state via
 * field_clear_record_state.
 *
 * @param record Status record whose timers are ticked.
 */
void func_800B4DF0(FieldStatusRecord *record)
{
    s32 i;

    for (i = 0; i < 12; i++)
    {
        if ((s16)record->status_timers[i] != 0)
        {
            s16 remaining = record->status_timers[i] - 1;
            record->status_timers[i] = remaining;
            if (remaining <= 0)
            {
                field_clear_record_state(record, i);
            }
        }
    }
}

/**
 * @brief Drain a record's current value by its per-tick cost while a draining effect is active.
 * @param record Status record to update; the cost is maximum >> 5, or >> 8 for party
 *               members whose template shows the HP gauge.
 * @note The current value never drops below 1.
 */
void func_800B4E60(FieldStatusRecord *record)
{
    s32 current;
    u32 value;
    s32 cost;
    FieldStatusState *state_again;
    FieldStatusState *state;

    value = record->meta.bytes.id;
    state = record->state;
    value = value < 3U;
    current = state->current;
    if (value != 0)
{
        value = state->effect_flags;
        value &= 0x190;
        if (value != 0)
{
            value = state->maximum;
            value >>= 5;
            do {
                cost = 1;
                if (value != 0)
{
                    cost = value;
                }
                current -= cost;
            } while (0);
            value = 1;
            if (current > 0)
{
                state->current = current;
                return;
            }
            state->current = value;
        }
    } else {
        value = state->effect_flags;
        value &= 0x191;
        if (value != 0)
{
            value = record->template->flags;
            value &= 0x80;
            do {
                cost = 1;
                if (value != 0)
{
                    value = state->maximum;
                    value >>= 8;
                } else {
                    value = state->maximum;
                    value >>= 5;
                }
                if (value != 0)
{
                    cost = value;
                }
                current -= cost;
            } while (0);
            state_again = record->state;
            value = 1;
            if (current > 0)
{
                state_again->current = current;
                return;
            }
            state_again->current = value;
        }
    }
}

/**
 * @brief Step each current stat one point toward its base value.
 * @param record Status record whose stats are updated.
 */
void func_800B4F38(FieldStatusRecord *record)
{
    s32 i;
    u8 current;
    u8 target;

    for (i = 0; i < FIELD_STATUS_STAT_COUNT; i++)
    {
        current = record->stats[i];
        target = record->base_stats[i];
        if (target < current)
        {
            record->stats[i] = current - 1;
        }
        else if (current < target)
        {
            record->stats[i] = current + 1;
        }
    }
}

s32 func_8008ADB4(u8 arg0);

/**
 * @brief Update the record's saturating counters when its growth interval elapses.
 * @param record Status record whose linked state counter grows.
 */
void func_800B4F80(FieldStatusRecord *record)
{
    u16 flags;
    s32 multiplier;
    s32 scaled_remaining;
    s32 divisor;
    s32 classification;

    flags = record->status_flags;
    if (flags & 1)
    {
        return;
    }
    if (record->state->effect_flags & 0x391)
    {
        return;
    }
    if ((flags & 8) || func_800B4CE4(record, 2) != 0)
    {
        multiplier = 8;
    }
    else
    {
        classification = func_8008ADB4(record->meta.bytes.id);
        if (classification < 0)
        {
            return;
        }
        if (classification < 2)
        {
            multiplier = 4;
        }
        else if (classification != 0x31)
        {
            return;
        }
        else
        {
            multiplier = 2;
        }
    }

    scaled_remaining = (0x64 - func_800B2D34(record, 4)) * multiplier;
    if (scaled_remaining < 0)
    {
        scaled_remaining += 0xF;
    }
    divisor = scaled_remaining >> 4;
    if (divisor <= 0)
    {
        divisor = 1;
    }
    if ((u32)D_80122B78->frame_count % (u32)divisor == 0)
    {
        saturating_counter_add(record->state, 1);
        if (record->meta.bytes.id < 3)
        {
            saturating_counter_add(record->state, func_800B4CE4(record, 1));
        }
    }
}
