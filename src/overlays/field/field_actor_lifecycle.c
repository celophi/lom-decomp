/** @file field_actor_lifecycle.c
 * @brief Start, suspend and end a field battle, and look up monster templates.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"
#include "main.h"

/** @brief Owner id of the first event record (the field's own event scripts). */
#define FIELD_EVENT_OWNER 0x80

/** @brief Actor event run at each battle phase; its argument is a FIELD_BATTLE_PHASE_* value. */
#define FIELD_BATTLE_EVENT 13
#define FIELD_BATTLE_PHASE_START 0
#define FIELD_BATTLE_PHASE_END 1
#define FIELD_BATTLE_PHASE_SUSPEND 3

/** @brief Actor event a script-controlled party member runs when a battle starts. */
#define FIELD_PARTY_BATTLE_EVENT 15

/** @brief Every event of an actor record enabled. */
#define FIELD_ALL_EVENTS 0xFFFF

/** @brief FieldBattleContext::state flag: the battle is over or suspended. */
#define FIELD_BATTLE_FINISHED 0x80000000

/** @brief Party control modes set with field_set_actor_control_mode: pad input, following the leader, or scripts. */
#define FIELD_CONTROL_PAD 0
#define FIELD_CONTROL_FOLLOW 1
#define FIELD_CONTROL_SCRIPTED 2

/** @brief Music fade-out length (ticks) when a battle ends on a battle-music layout. */
#define FIELD_BATTLE_MUSIC_FADE_TICKS 64

/** @brief record_game_diagnostic arguments for a missing monster template. */
#define FIELD_DIAGNOSTIC_ERROR 0x8001
#define FIELD_DIAGNOSTIC_NO_TEMPLATE 0x67

extern FieldGameState* g_field_game_state;
extern FieldRuntimeContext* g_field_runtime;
extern FieldBattleContext* g_field_battle;
extern s32 g_field_duel_mode;

extern FieldStatusState* field_find_object_state(s32 actor_id);
extern s32 field_set_actor_control_mode(s32 party_index, s32 mode);
extern void field_stop_actor_script(s32 actor_id, s32 flags);
extern s32 akao_cmd_c1(s32 song, s32 fade_ticks, s32 volume);

/**
 * @brief Look up the object state of every non-party actor record.
 * @note The results are discarded; called instead of building a battle for group 0.
 */
void field_battle_scan_actor_objects(void)
{
    s32 i;

    for (i = FIELD_PARTY_SIZE; i < g_field_runtime->state.actor_count; i++)
    {
        field_find_object_state(g_field_runtime->actors[i].id);
    }
}

/**
 * @brief Start a battle: bind the group's actors, run their battle event and hand the party to battle control.
 * @param group Trigger group (monster group) matched against each actor record.
 */
void field_battle_start(s32 group)
{
    s32 i;
    FieldRuntimeContext* context;

    for (i = FIELD_PARTY_SIZE; i < g_field_runtime->state.actor_count; i++)
    {
        if (g_field_runtime->actors[i].flags.bits.trigger_group == group)
        {
            field_set_actor_group(g_field_runtime->actors[i].id, group);
            field_run_actor_event(g_field_runtime->actors[i].id, FIELD_BATTLE_EVENT, FIELD_BATTLE_PHASE_START);
        }
    }

    /* One load of g_field_runtime for the three uses; naming it directly each time reloads it. */
    context = g_field_runtime;
    context->state.bits.group_active = 1;
    context->state.bits.trigger_group = group;
    field_set_battle_group(group, context);

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        if (g_field_game_state->characters[i].info.bits.pad_controlled)
        {
            field_set_actor_control_mode(i, FIELD_CONTROL_PAD);
        }
        else
        {
            g_field_runtime->actors[i].enabled_events = FIELD_ALL_EVENTS;
            field_set_actor_control_mode(i, FIELD_CONTROL_SCRIPTED);
            field_run_actor_event(g_field_runtime->actors[i].id, FIELD_PARTY_BATTLE_EVENT, 0);
        }
    }

    field_run_actor_event(FIELD_EVENT_OWNER, FIELD_BATTLE_EVENT, FIELD_BATTLE_PHASE_START);
}

/**
 * @brief Suspend the battle, run the suspend event for the active group and refill the party's HP.
 */
void field_battle_suspend(void)
{
    s32 i;
    FieldStatusState* state;
    FieldBattleContext* battle;

    battle = g_field_battle;
    battle->state.flags |= FIELD_BATTLE_FINISHED;
    field_set_battle_group(0, battle);
    field_run_actor_event(FIELD_EVENT_OWNER, FIELD_BATTLE_EVENT, FIELD_BATTLE_PHASE_SUSPEND);

    for (i = FIELD_PARTY_SIZE; i < g_field_runtime->state.actor_count; i++)
    {
        if (g_field_runtime->actors[i].flags.bits.trigger_group == g_field_runtime->state.bits.trigger_group)
        {
            field_run_actor_event(g_field_runtime->actors[i].id, FIELD_BATTLE_EVENT, FIELD_BATTLE_PHASE_SUSPEND);
        }
    }

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        state = field_find_object_state(i);
        if (state != (FieldStatusState*)-1)
        {
            state->current = state->maximum;
        }
    }
}

/**
 * @brief End the battle: return the party to field control, run the end event and fade the battle music.
 * @note The music fades only on the listed layouts, and not after a duel (g_field_duel_mode).
 */
void field_battle_end(void)
{
    s32 i;
    s32 j;

    g_field_battle = NULL;
    g_field_runtime->state.bits.group_active = 0;
    g_field_runtime->state.bits.trigger_group = 0;

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        if (g_field_game_state->characters[i].info.bits.pad_controlled)
        {
            field_set_actor_control_mode(i, FIELD_CONTROL_PAD);
        }
        else
        {
            g_field_runtime->actors[i].enabled_events = 0;
            for (j = 0; j < FIELD_ACTOR_SCRIPT_COUNT; j++)
            {
                g_field_runtime->actors[i].scripts[j] = FIELD_NO_SCRIPT;
            }
            field_set_actor_control_mode(i, FIELD_CONTROL_FOLLOW);
            field_stop_actor_script(i, 0);
        }
    }

    for (i = 0; i < g_field_runtime->state.actor_count; i++)
    {
        field_run_actor_event(g_field_runtime->actors[i].id, FIELD_BATTLE_EVENT, FIELD_BATTLE_PHASE_END);
    }

    if (g_field_duel_mode != 0)
    {
        g_field_duel_mode = 0;
    }
    else
    {
        switch (g_layout_flag)
        {
        case 3:
        case 34:
        case 35:
        case 37:
        case 43:
        case 45:
        case 46:
        case 47:
            akao_cmd_c1(0, FIELD_BATTLE_MUSIC_FADE_TICKS, 0);
            break;
        }
    }
}

/**
 * @brief Find a monster template by id.
 * @param table Template table of the battle resource.
 * @param id Template id; only the low byte is compared.
 * @return The matching template, or NULL (after a diagnostic) when absent.
 */
FieldActorTemplate* field_find_actor_template(FieldActorTemplateTable* table, s32 id)
{
    u32 i;
    FieldActorTemplate* template;

    id &= 0xFF;
    for (i = 0; i < table->count; i++)
    {
        template = (FieldActorTemplate*)((u8*)table + table->offsets[i]);
        if (template->id == id)
        {
            return template;
        }
    }

    record_game_diagnostic(FIELD_DIAGNOSTIC_ERROR, FIELD_DIAGNOSTIC_NO_TEMPLATE, id, -1);
    return NULL;
}
