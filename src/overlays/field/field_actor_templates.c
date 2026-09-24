#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

extern FieldBattleContext *D_80123FB0;
extern FieldRuntimeContext *D_80122B78;

s32 func_800B3670(s32 arg0);
s32 func_800B42B4(FieldActorTemplate *template);
extern FieldActorTemplate *func_800B4844(u32 *, s32);
extern u32 func_800BD414(s32, s32);
s32 func_80087F0C(s32 arg0);

/**
 * @brief Build status records for the field actors whose trigger group matches the requested key.
 * @param group Trigger group to match.
 * @return Number of matching actor records processed.
 */
s32 func_800B3DF4(s32 group)
{
    s32 i;
    s32 count;
    s32 result;

    count = 0;
    for (i = 3; i < (s32)D_80122B78->state.actor_count; i++)
    {
        if ((D_80122B78->actors[i].flags.word & 0xF) != group)
        {
            continue;
        }

        result = func_80087F0C(D_80122B78->actors[i].id);
        /* Kept gotos: if/else-if on result swaps two saved registers (99.30%). */
        if (result == 0)
        {
            goto report_error;
        }
        if (result != -1)
        {
            goto build_record;
        }
    report_error:
        record_game_diagnostic(0x8001, 0x64, group, D_80122B78->actors[i].id);
    build_record:
        func_800B3F1C(D_80122B78->actors[i].id, &D_80123FB0->records[3 + count], (FieldStatusState *)result);
        count++;
    }
    return count;
}

/**
 * @brief Initialize a monster's status record and runtime state from its template.
 * @param actor_id Actor identifier stored in the record.
 * @param record Status record to initialize.
 * @param state Runtime status state to initialize.
 */
void func_800B3F1C(s32 actor_id, FieldStatusRecord *record, FieldStatusState *state)
{
    s32 flags;
    s32 level;
    u32 clamped;
    u32 work;
    u32 stat;
    FieldActorTemplate *template;
    FieldBattleContext *ctx;

    record->meta.bytes.id = actor_id;
    record->unk0 = 0x40;
    record->state = state;
    flags = record->meta.packed;
    flags |= 0x100;
    flags &= ~0x200;
    flags &= 0xFFFF03FF;
    ctx = D_80123FB0;
    flags |= 0x1400;
    record->meta.packed = flags;
    record->meta.bytes.unk2 = 0;
    template = func_800B4844((u32 *)ctx->templates, state->template_index);
    record->template = template;
    record->unk3 = template->unk1A;
    record->counter = template->counter_reset;
    record->counter_reset = template->counter_reset;
    record->status_flags = 0;
    record->unkC = 0;
    level = func_800B42B4(template);
    clamped = 0x63;
    if ((u32)level < 0x64U)
    {
        clamped = level;
    }
    level = clamped;
    work = func_800BD414(0, 0x2F78);
    if (work != 0)
    {
        level = work;
    }
    record->unk18 = (u32)((template->unk20_base * 0x10) + (template->unk20_growth * level)) >> 4;
    record->unk1A = template->unk19;
    for (work = 0; work < 4; work++)
    {
        record->equipment_stats[work] = (u32)((template->equipment_stats[work].base * 0x10) + (template->equipment_stats[work].growth * level)) >> 4;
        record->equipment_attributes[work] = 0;
    }
    for (work = 0; work < FIELD_STATUS_STAT_COUNT; work++)
    {
        stat = (u32)((template->stats[work].base * 4) + (template->stats[work].growth * level)) >> 2;
        record->unk3C[work] = 5;
        record->unk44[work] = 5;
        record->base_stats[work] = stat;
        record->stats[work] = stat;
    }
    record->immunity_flags = template->immunity_flags;
    record->unk39 = template->unk3D;
    record->unk3A = template->unk3E;
    state->level.word = (state->level.word & ~0xFE) | ((level & 0x7F) * 2);
    if (template->flags & 2)
    {
        state->template = NULL;
    }
    else
    {
        state->template = template;
    }
    if (template->hp_growth != 0xFFFF)
    {
        state->maximum = template->hp_base + (template->hp_growth * level);
    }
    else
    {
        work = 1;
        state->maximum = template->hp_base;
        if (level != 0)
        {
            do
            {
                state->maximum = func_800C19D0(state->maximum, (u32)((template->stats[4].base * 4) + (template->stats[4].growth * work)) >> 2, 1);
                work++;
            } while ((u32)level >= work);
        }
    }
    switch (func_800BD414(0, 0x2938))
    {
    case 1:
        state->maximum *= 2;
        break;
    case 2:
        state->maximum *= 3;
        break;
    }
    if ((func_800BD414(0, 0xFFE) != 0) || (state->maximum == 0))
    {
        state->maximum = 1;
    }
    state->current = state->maximum;
    state->gauge.bits.value = state->maximum;
    state->gauge.bits.hud_bits = 0;
    state->gauge.bits.hud_flag = template->flags >> 7;
    work = record->base_stats[3] * 2;
    if (work < 0x100U)
    {
        state->effect_footprint_strength = work;
        return;
    }
    state->effect_footprint_strength = 0xFF;
}

/**
 * @brief Compute a monster's level from the battle element levels its template reacts to.
 * @param template Template with raise/lower masks and the flag byte.
 * @return Level clamped to 1..99.
 */
s32 func_800B42B4(FieldActorTemplate *template)
{
    u32 add_mask;
    u32 sub_mask;
    u32 count;
    s32 value;
    s32 clamped;
    u32 scaled;

    count = 0;
    value = 8;
    add_mask = template->raise_mask;
    sub_mask = template->lower_mask;

    for (; count < 8; count++)
    {
        if (add_mask & 1)
        {
            value += D_80123FB0->element_levels[count];
        }
        if (sub_mask & 1)
        {
            value -= D_80123FB0->element_levels[count];
        }
        add_mask >>= 1;
        sub_mask >>= 1;
    }

    if (value < 5)
    {
        clamped = 4;
    }
    else
    {
        clamped = value;
        if (clamped >= 0xB)
        {
            clamped = 0xA;
        }
    }
    value = clamped;

    scaled = (u32)(func_800B3670(template->flags & 4) * value) >> 3;

    if (scaled >= 2)
    {
        if (scaled >= 0x64)
        {
            scaled = 0x63;
        }
    }
    else
    {
        scaled = 1;
    }

    return scaled;
}
