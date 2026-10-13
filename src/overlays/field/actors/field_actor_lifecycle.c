/**
 * @file field_actor_lifecycle.c
 * @brief Start, suspend and end a field battle, and look up monster templates.
 */

#include "main/audio/game_audio.h"
#include "main/audio/akao_cmd.h"
#include "common.h"
#include "../internal/field_calls.h"
#include "../internal/field_actor.h"
#include "../internal/field_records.h"
#include "main/main.h"

/** @brief Music fade-out length (ticks) when a battle ends to boss music. */
#define FIELD_BATTLE_MUSIC_FADE_TICKS 64

/*
 * Boss music (g_field_music_id, the MUSICnnn.SET resource number). Scene changes
 * into the boss arenas (the *_BS* scenes) select these tracks.
 */
/** @brief Boss music of most arenas, and of the BTL_SMP battle test scenes. */
#define FIELD_MUSIC_BOSS_BATTLE 3
/** @brief Boss music of the JUL_BS*, BON_BS00, CV1_BS00, MNT_BS30 and TWR_I10* arenas, among others. */
#define FIELD_MUSIC_BOSS_BATTLE_2 34
/** @brief Boss music of the WST_BS10, WD1_BS02, JGL_BS10, WAL_B071 and HTW_PMP* arenas. */
#define FIELD_MUSIC_BOSS_BATTLE_3 35
/** @brief Boss music of "Another Dimension" (DRL_BS30). */
#define FIELD_MUSIC_ANOTHER_DIMENSION 37
/** @brief Boss music of "The Sparkling Vortex" (JUL_BSS4). */
#define FIELD_MUSIC_SPARKLING_VORTEX 43
/** @brief Boss music of the TWR_BS*, LAK_BS01, GRB_BS00, HEL_BS00, SHP_BS10 and WAL_BS01 arenas; the WAL rooms also play it. */
#define FIELD_MUSIC_BOSS_BATTLE_4 45
/** @brief Music of the "Leaving Sorrow Behind" arena (ESL_BS00, ESL_BS01). */
#define FIELD_MUSIC_LEAVING_SORROW_BEHIND 46
/** @brief Boss music of the "Sanctuary of Mana" (SEI_BSS0). */
#define FIELD_MUSIC_SANCTUARY_OF_MANA 47

FieldStatusState* field_find_object_state(s32 actor_id);

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
            field_run_actor_event(g_field_runtime->actors[i].id, FIELD_EVENT_NOTIFY, FIELD_BATTLE_PHASE_START);
        }
    }

    context = g_field_runtime;
    context->state.bits.group_active = 1;
    context->state.bits.trigger_group = group;
    field_set_battle_group(group, context);

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        if (g_field_game_state->characters[i].info.bits.pad_controlled)
        {
            field_set_actor_control_mode(i, FIELD_CONTROL_PLAYER);
        }
        else
        {
            g_field_runtime->actors[i].enabled_events = FIELD_ALL_EVENTS;
            field_set_actor_control_mode(i, FIELD_CONTROL_SCRIPTED);
            field_run_actor_event(g_field_runtime->actors[i].id, FIELD_EVENT_START, 0);
        }
    }

    field_run_actor_event(FIELD_EVENT_ACTOR_ID_BASE, FIELD_EVENT_NOTIFY, FIELD_BATTLE_PHASE_START);
}

/**
 * @brief Suspend the battle, run the suspend event for the active group and refill the party's HP.
 */
void field_battle_suspend(void)
{
    s32 i;
    FieldStatusState* state;

    g_field_battle->state.flags |= FIELD_BATTLE_FINISHED;
    field_set_battle_group(0, g_field_battle);
    field_run_actor_event(FIELD_EVENT_ACTOR_ID_BASE, FIELD_EVENT_NOTIFY, FIELD_BATTLE_PHASE_SUSPEND);

    for (i = FIELD_PARTY_SIZE; i < g_field_runtime->state.actor_count; i++)
    {
        if (g_field_runtime->actors[i].flags.bits.trigger_group == g_field_runtime->state.bits.trigger_group)
        {
            field_run_actor_event(g_field_runtime->actors[i].id, FIELD_EVENT_NOTIFY, FIELD_BATTLE_PHASE_SUSPEND);
        }
    }

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        state = field_find_object_state(i);

        if (state != FIELD_STATUS_STATE_NONE)
        {
            state->current = state->maximum;
        }
    }
}

/**
 * @brief End the battle: return the party to field control, run the end event and fade the battle music.
 * @note Only boss music fades, and not after a duel (g_field_duel_mode).
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
            field_set_actor_control_mode(i, FIELD_CONTROL_PLAYER);
        }
        else
        {
            g_field_runtime->actors[i].enabled_events = 0;

            for (j = 0; j < FIELD_ACTOR_SCRIPT_COUNT; j++)
            {
                g_field_runtime->actors[i].scripts[j] = FIELD_NO_SCRIPT;
            }

            field_set_actor_control_mode(i, FIELD_CONTROL_FOLLOWER);
            field_stop_actor_script(i, 0);
        }
    }

    for (i = 0; i < g_field_runtime->state.actor_count; i++)
    {
        field_run_actor_event(g_field_runtime->actors[i].id, FIELD_EVENT_NOTIFY, FIELD_BATTLE_PHASE_END);
    }

    if (g_field_duel_mode != 0)
    {
        g_field_duel_mode = 0;
    }
    else
    {
        switch (g_field_music_id)
        {
        case FIELD_MUSIC_BOSS_BATTLE:
        case FIELD_MUSIC_BOSS_BATTLE_2:
        case FIELD_MUSIC_BOSS_BATTLE_3:
        case FIELD_MUSIC_ANOTHER_DIMENSION:
        case FIELD_MUSIC_SPARKLING_VORTEX:
        case FIELD_MUSIC_BOSS_BATTLE_4:
        case FIELD_MUSIC_LEAVING_SORROW_BEHIND:
        case FIELD_MUSIC_SANCTUARY_OF_MANA:
            akao_fade_song_volume(0, FIELD_BATTLE_MUSIC_FADE_TICKS, 0);
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

    record_game_diagnostic(DIAG_ERROR, DIAG_NO_MONSTER_TEMPLATE, id, -1);
    return NULL;
}
