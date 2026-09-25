#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

extern FieldBattleContext *g_field_battle;
extern FieldRuntimeContext *g_field_runtime;

/** @brief Runtime actor flag bits holding the trigger group. */
#define FIELD_TRIGGER_GROUP_MASK 0xF
/** @brief FieldStatusRecord::unk0 value of a record on the monsters' side. */
#define FIELD_RECORD_MONSTER_SIDE 0x40
/** @brief FieldActorTemplate::flags bits. */
#define FIELD_TEMPLATE_UNLINKED 0x2   /**< The object state keeps no link to the template. */
#define FIELD_TEMPLATE_HERO_LEVEL 0x4 /**< The level follows the hero's level instead of the land's. */

s32 func_800B3670(s32 use_hero_level);
s32 field_compute_monster_level(FieldActorTemplate *template);
/* Defined without a result; the value is the one it leaves in the return register. */
u32 func_800BD414(s32 owner_id, s32 variable_id);
FieldStatusState *field_find_object_state(s32 key);

/**
 * @brief Build the battle status records of the monsters in a trigger group.
 * @param group Trigger group whose actors join the battle.
 * @return Number of monster records built (they follow the party records).
 */
s32 field_build_group_monster_records(s32 group)
{
    s32 i;
    s32 count;
    FieldStatusState *state;

    count = 0;
    for (i = FIELD_PARTY_SIZE; i < (s32)g_field_runtime->state.actor_count; i++)
    {
        if ((g_field_runtime->actors[i].flags.word & FIELD_TRIGGER_GROUP_MASK) != group)
        {
            continue;
        }

        state = field_find_object_state(g_field_runtime->actors[i].id);
        /* The gotos keep the two saved registers in the original's order; if/else swaps them. */
        if (state == NULL)
        {
            goto report_error;
        }
        if (state != (FieldStatusState *)-1)
        {
            goto build_record;
        }
    report_error:
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_MONSTER_OBJECT, group, g_field_runtime->actors[i].id);
    build_record:
        field_init_monster_record(g_field_runtime->actors[i].id, &g_field_battle->records[FIELD_PARTY_SIZE + count], state);
        count++;
    }
    return count;
}

/**
 * @brief Initialize a monster's status record and object state from its template.
 * @param actor_id Actor identifier stored in the record.
 * @param record Status record to initialize.
 * @param state Object state of the monster.
 */
void field_init_monster_record(s32 actor_id, FieldStatusRecord *record, FieldStatusState *state)
{
    s32 flags;
    s32 level;
    u32 clamped;
    u32 work;
    u32 stat;
    FieldActorTemplate *template;
    FieldBattleContext *ctx;

    record->meta.bytes.id = actor_id;
    record->unk0 = FIELD_RECORD_MONSTER_SIDE;
    record->state = state;
    flags = record->meta.packed;
    flags |= FIELD_STATUS_META_ACTIVE;
    flags &= ~FIELD_STATUS_META_ALLY;
    flags &= ~FIELD_STATUS_META_KIND_MASK;
    ctx = g_field_battle;
    flags |= FIELD_STATUS_KIND_MONSTER << FIELD_STATUS_META_KIND_SHIFT;
    record->meta.packed = flags;
    record->meta.bytes.unk2 = 0;
    template = field_find_actor_template(ctx->templates, state->template_index);
    record->template = template;
    record->race = template->unk1A;
    record->counter = template->counter_reset;
    record->counter_reset = template->counter_reset;
    record->status_flags = 0;
    record->unkC = 0;
    level = field_compute_monster_level(template);
    clamped = FIELD_LEVEL_MAX;
    if ((u32)level < FIELD_LEVEL_MAX + 1U)
    {
        clamped = level;
    }
    level = clamped;
    work = func_800BD414(0, FIELD_VAR_MONSTER_LEVEL);
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
        record->element_attack[work] = FIELD_ELEMENT_LEVEL_NEUTRAL;
        record->element_defense[work] = FIELD_ELEMENT_LEVEL_NEUTRAL;
        record->base_stats[work] = stat;
        record->stats[work] = stat;
    }
    record->immunity_flags = template->immunity_flags;
    record->weak_elements = template->weak_elements;
    record->resist_elements = template->resist_elements;
    state->level.word = (state->level.word & ~0xFE) | ((level & 0x7F) * 2);
    if (template->flags & FIELD_TEMPLATE_UNLINKED)
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
    switch (func_800BD414(0, FIELD_VAR_DIFFICULTY))
    {
    case 1:
        state->maximum *= 2;
        break;
    case 2:
        state->maximum *= 3;
        break;
    }
    if ((func_800BD414(0, FIELD_VAR_DEBUG_ONE_HP_MONSTERS) != 0) || (state->maximum == 0))
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
 * @brief Compute a monster's level from the land's element levels its template reacts to.
 * @param template Template with the raise and lower element masks.
 * @return Level from 1 to FIELD_LEVEL_MAX.
 */
s32 field_compute_monster_level(FieldActorTemplate *template)
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
            value += g_field_battle->element_levels[count];
        }
        if (sub_mask & 1)
        {
            value -= g_field_battle->element_levels[count];
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

    scaled = (u32)(func_800B3670(template->flags & FIELD_TEMPLATE_HERO_LEVEL) * value) >> 3;

    if (scaled >= 2)
    {
        if (scaled > FIELD_LEVEL_MAX)
        {
            scaled = FIELD_LEVEL_MAX;
        }
    }
    else
    {
        scaled = 1;
    }

    return scaled;
}
