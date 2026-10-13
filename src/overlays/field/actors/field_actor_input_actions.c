/**
 * @file field_actor_input_actions.c
 * @brief Map controller input to actor actions, action commands and the run button.
 */

#include "common.h"
#include "main/main.h"
#include "common/pad.h"
#include "main/controller_internal.h"
#include "../internal/field_actor_tables.h"
#include "../internal/field_calls.h"
#include "overlays/field/field_text.h"
#include "overlays/field/field_sound.h"
#include "main/audio/akao.h"
#include "../internal/field_contact_geometry.h"
#define FIELD_HELD_ACTION_BUTTONS_OUT_OF_LINE
#include "../internal/field_held_action_buttons.h"

/** @brief Actions 2 and 3 stay available while FIELD_OBJECT_FLAG_MINI is set. */
#define FIELD_ACTION_RESTRICTED_FIRST 2
#define FIELD_ACTION_RESTRICTED_LAST 3

/** @brief First action blocked while g_field_actions_limited is set. */
#define FIELD_ACTION_BLOCKABLE_FIRST 4

/** @brief Buttons that confirm (start an interaction). */
#define FIELD_CONFIRM_BUTTONS (PAD_BTN_CROSS | PAD_BTN_L3)

/** @brief The four shoulder buttons, which keep their bits through the face-button remap. */
#define FIELD_SHOULDER_BUTTONS (PAD_BTN_L2 | PAD_BTN_R2 | PAD_BTN_L1 | PAD_BTN_R1)

/** @brief Buttons that make the actor run while held. */
#define FIELD_RUN_BUTTONS (PAD_BTN_CIRCLE | PAD_BTN_L1)

/**
 * @brief Collect the held buttons that are bound to an action.
 * @param player Controller port and saved character index.
 * @param action Action whose bound buttons are collected.
 * @param actor Actor that must be a party member in control mode 0.
 * @return The held bound buttons, or zero when input is unavailable.
 * @note JP returns the face buttons as read (no swap).
 */
s32 field_get_held_action_buttons(s32 player, s32 action, FieldActor* actor)
{
    s32 mask;
    s32 i;
    u8 code;
    ControllerPortState* ports;
    ControllerSample* sample;
#if !defined(VERSION_JP)
    u32 buttons;
#endif

    ports = CONTROLLER_STATE->ports;

    if (actor->object_index < FIELD_PARTY_COUNT)
    {
        if ((actor->control.word & FIELD_CONTROL_MODE_MASK) == 0)
        {
            mask = 0;
            code = g_field_action_binding_codes[action];

            for (i = 0; i < FIELD_BUTTON_BINDING_COUNT; i++)
            {
                if (code == g_saved_game_ctx->characters[player].button_actions[i])
                {
                    mask |= g_field_binding_buttons[i];
                }
            }

            sample = &ports[player].published_sample;

            if (sample->device_type < CONTROLLER_DEVICE_CONFIGURING)
            {
#if defined(VERSION_JP)
                return (sample->held_buttons >> 8) & mask;
#else
                buttons = sample->held_buttons >> 8;
                return (((buttons >> 1) & PAD_BTN_CROSS) | ((buttons & PAD_BTN_CROSS) << 1) | ((buttons >> 3) & PAD_BTN_SQUARE) |
                        ((buttons & PAD_BTN_SQUARE) << 3) | (buttons & FIELD_SHOULDER_BUTTONS)) &
                       mask;
#endif
            }
        }
    }

    return 0;
}

/**
 * @brief Let the leader probe for an interaction when confirm is pressed and nothing blocks it.
 * @note JP checks only for an active interaction before reading the text windows.
 */
void field_poll_leader_interaction(void)
{
    s32 first_status;
    s32 second_status;

#if defined(VERSION_JP)
    if ((g_field_interaction_active == 0) && (g_field_buffered_input & FIELD_CONFIRM_BUTTONS))
#else
    if ((g_field_active_group == 0) && (g_field_actors[0].command == FIELD_ACTOR_COMMAND_NONE) && (g_field_dialog_screen_mode == 0) &&
        (g_field_interaction_active == 0) && (g_field_buffered_input & FIELD_CONFIRM_BUTTONS))
#endif
    {
        first_status = field_text_get_status(0);
        second_status = field_text_get_status(1);

        if ((g_field_active_group == 0) && (first_status == FIELD_TEXT_STATUS_CLOSED) && (second_status == first_status))
        {
            field_probe_actor_interaction(g_field_actors);
        }
    }
}

/**
 * @brief Turn the player's queued action into an enabled actor command word.
 * @param actor Player actor; its action chain is reset when no action is queued.
 * @param player Player index: command history and action command map.
 * @return Command word from the player's action command map, or zero when none is available.
 */
s32 field_resolve_action_command(FieldActor* actor, s32 player)
{
    s32 action;

    action = field_command_history_match(player, (actor->animation >> FIELD_ANIMATION_FACING_SHIFT) ^ 1, 0);

    if (action != FIELD_HISTORY_COMMAND_NONE)
    {
        g_field_action_context = (g_field_action_context & ~FIELD_ACTION_CONTEXT_ACTION_MASK) | action;
    }
    else if ((actor->animation & FIELD_ANIMATION_INDEX_MASK) != FIELD_ABILITY_DOUBLE_JUMP)
    {
        actor->variant = 0;
        g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_CHAINING;
        g_field_object_states[actor->object_index].retry_count = 0;
    }

    if (g_field_object_states[actor->object_index].flags & FIELD_OBJECT_FLAG_MINI)
    {
        if (action != FIELD_ACTION_RESTRICTED_FIRST && action != FIELD_ACTION_RESTRICTED_LAST)
        {
            if (action != FIELD_HISTORY_COMMAND_NONE)
            {
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            }

            return 0;
        }
    }

    if (action < FIELD_ACTION_MAP_ENTRY_COUNT)
    {
        if (g_field_actions_limited == 0 || action < FIELD_ACTION_BLOCKABLE_FIRST)
        {
            if (g_field_action_command_maps[player].commands[action] != 0 && g_field_action_command_maps[player].disabled[action] == 0)
            {
                return g_field_action_command_maps[player].commands[action];
            }
        }
    }

    return 0;
}

/**
 * @brief Set or clear the actor's running flag while a run button is held.
 * @param actor Player actor; resources with an action table never run.
 * @param player Controller port to read.
 * @note JP tests the buttons as read (no PAD_REMAP_FACE_BITS swap).
 */
void field_update_actor_run_button(FieldActor* actor, s32 player)
{
    u16 raw;
    u32 buttons;
    ControllerPortState* ports;

    ports = CONTROLLER_STATE->ports;

    if (!(g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS))
    {
#if defined(VERSION_JP)
        if (ports[player].published_sample.device_type >= CONTROLLER_DEVICE_CONFIGURING)
        {
            buttons = 0;
        }
        else
        {
            raw = ports[player].published_sample.held_buttons;
            buttons = ((raw << 8) & 0xFF00) | (raw >> 8);
        }
#else
        buttons = 0;

        if (ports[player].published_sample.device_type < CONTROLLER_DEVICE_CONFIGURING)
        {
            raw = ports[player].published_sample.held_buttons;
            buttons = ((raw << 8) & 0xFF00) | (raw >> 8);
        }

        buttons = PAD_REMAP_FACE_BITS(buttons);
#endif

        if (buttons & FIELD_RUN_BUTTONS)
        {
            if (actor->running == 0)
            {
                actor->running = 1;
                actor->animation_active = 0;
            }
        }
        else if (actor->running != 0)
        {
            actor->running = 0;
            actor->animation_active = 0;
        }
    }
}
