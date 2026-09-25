/**
 * @file field_actor_input_actions.c
 * @brief Map controller input to actor actions, action commands and the run button.
 */

#include "common.h"
#include "main.h"
#include "pad.h"
#include "controller_internal.h"
#include "field_actor_tables.h"
#include "field_calls.h"
#include "field_text.h"

/** @brief Number of action buttons a player can bind (four face buttons, four shoulder buttons). */
#define FIELD_BUTTON_BINDING_COUNT 8

/** @brief Number of party objects (two players and a companion). */
#define FIELD_PARTY_COUNT 3

/** @brief Number of actions with an entry in an action command map. */
#define FIELD_ACTION_MAP_ENTRY_COUNT 11

/** @brief Action returned by the command history when no action is queued. */
#define FIELD_ACTION_NONE 0xFF

/** @brief Actions 2 and 3 stay available while FIELD_OBJECT_ACTIONS_RESTRICTED is set. */
#define FIELD_ACTION_RESTRICTED_FIRST 2
#define FIELD_ACTION_RESTRICTED_LAST 3

/** @brief First action blocked while g_field_actions_limited is set. */
#define FIELD_ACTION_BLOCKABLE_FIRST 4

/** @brief Shift of the mirror bit in an actor's animation byte. */
#define FIELD_ANIMATION_FACING_SHIFT 7

/** @brief Animation that keeps the action chain alive while no action is queued. */
#define FIELD_ANIMATION_UNK3D 0x3D

/** @brief Object flag: only the restricted actions are accepted. */
#define FIELD_OBJECT_ACTIONS_RESTRICTED 0x400

/** @brief Object flag: an action chain is running (cleared with the retry count). */
#define FIELD_OBJECT_CHAINING 0x8000

/** @brief Sound played when a restricted object tries another action, and its pan. */
#define FIELD_SOUND_ACTION_REFUSED 0x78
#define FIELD_SOUND_PAN_CENTRE 0x80

/** @brief Buttons that confirm (start an interaction). */
#define FIELD_CONFIRM_BUTTONS (PAD_BTN_CROSS | PAD_BTN_L3)

/** @brief The four shoulder buttons, which keep their bits through the face-button remap. */
#define FIELD_SHOULDER_BUTTONS (PAD_BTN_L2 | PAD_BTN_R2 | PAD_BTN_L1 | PAD_BTN_R1)

/** @brief Buttons that make the actor run while held. */
#define FIELD_RUN_BUTTONS (PAD_BTN_CIRCLE | PAD_BTN_L1)

/** @brief Status returned by field_text_get_status for a closed text window. */
#define FIELD_TEXT_CLOSED (-1)

/** @brief Saved character record of the pad context (SAVED_CHARACTER_SIZE bytes). */
typedef struct
{
    u8 unk0[0x48];
    /** @brief Action binding code assigned to each bindable button. */
    u8 button_actions[FIELD_BUTTON_BINDING_COUNT];
    u8 unk50[SAVED_CHARACTER_SIZE - 0x50];
} FieldSavedCharacter;

/** @brief Pad context view holding the three saved characters. */
typedef struct
{
    u8 unk0[0x5F0];
    FieldSavedCharacter characters[FIELD_PARTY_COUNT];
} FieldSavedParty;

/** @brief The live saved game, viewed through FieldSavedParty. */
#define FIELD_SAVED_PARTY ((FieldSavedParty*)g_pad_ctx)

/** @brief One action command map: eleven actor command words and their disable flags. */
typedef struct
{
    u16 commands[FIELD_ACTION_MAP_ENTRY_COUNT];
    u8 disabled[FIELD_ACTION_MAP_ENTRY_COUNT];
    u8 pad;
} FieldActionMap;

/** @brief Pad button mask of each bindable button, in binding order. */
extern u8 g_field_binding_buttons[FIELD_BUTTON_BINDING_COUNT];
/** @brief Action binding code of each action. */
extern u8 g_field_action_binding_codes[];
extern s32 g_field_active_group;
extern s32 g_field_dialog_screen_mode;
extern s32 g_field_interaction_active;
extern s32 g_field_buffered_input;
extern FieldActionMap g_field_action_command_maps[];
extern s32 g_field_action_context;
extern s32 g_field_actions_limited;

/* field_contact_geometry.h cannot be included next to field_actor_tables.h (two g_field_object_states types). */
void field_probe_actor_interaction(struct FieldMotionRecord* entry);

/**
 * @brief Collect the held buttons that are bound to an action.
 * @param player Controller port and saved character index.
 * @param action Action whose bound buttons are collected.
 * @param actor Actor that must be a party member in control mode 0.
 * @return The held bound buttons, or zero when input is unavailable.
 */
s32 field_get_held_action_buttons(s32 player, s32 action, FieldActor* actor)
{
    s32 mask;
    s32 i;
    u8 code;
    ControllerPortState* ports;
    ControllerSample* sample;
    u32 buttons;

    ports = CONTROLLER_STATE->ports;
    if (actor->object_index < FIELD_PARTY_COUNT)
    {
        if ((actor->control.word & FIELD_CONTROL_MODE_MASK) == 0)
        {
            mask = 0;
            code = g_field_action_binding_codes[action];
            for (i = 0; i < FIELD_BUTTON_BINDING_COUNT; i++)
            {
                if (code == FIELD_SAVED_PARTY->characters[player].button_actions[i])
                {
                    mask |= g_field_binding_buttons[i];
                }
            }
            sample = &ports[player].published_sample;
            if (sample->device_type < CONTROLLER_DEVICE_CONFIGURING)
            {
                buttons = (u32)sample->held_buttons >> 8;
                return (((buttons >> 1) & PAD_BTN_CROSS) | ((buttons & PAD_BTN_CROSS) << 1) | ((buttons >> 3) & PAD_BTN_SQUARE) | ((buttons & PAD_BTN_SQUARE) << 3) | (buttons & FIELD_SHOULDER_BUTTONS)) & mask;
            }
        }
    }
    return 0;
}

/**
 * @brief Let the leader probe for an interaction when confirm is pressed and nothing blocks it.
 */
void field_poll_leader_interaction(void)
{
    s32 first_status;
    s32 second_status;

    if ((g_field_active_group == 0) && (g_field_actors[0].command == 0) && (g_field_dialog_screen_mode == 0) && (g_field_interaction_active == 0) && (g_field_buffered_input & FIELD_CONFIRM_BUTTONS))
    {
        first_status = field_text_get_status(0);
        second_status = field_text_get_status(1);
        if ((g_field_active_group == 0) && (first_status == FIELD_TEXT_CLOSED) && (second_status == first_status))
        {
            field_probe_actor_interaction((struct FieldMotionRecord*)g_field_actors);
        }
    }
}

/**
 * @brief Turn the player's queued action into an enabled actor command word.
 * @param actor Player actor; its action chain is reset when no action is queued.
 * @param player Player index: command history and action command map.
 * @return Command word from the player's action command map, or zero when none is available.
 */
u16 field_resolve_action_command(FieldActor* actor, s32 player)
{
    s32 action;

    action = func_800A29F8(player, (actor->animation >> FIELD_ANIMATION_FACING_SHIFT) ^ 1, 0);
    if (action != FIELD_ACTION_NONE)
    {
        g_field_action_context = (g_field_action_context & ~0xFF) | action;
    }
    else if ((actor->animation & FIELD_ANIMATION_INDEX_MASK) != FIELD_ANIMATION_UNK3D)
    {
        actor->variant = 0;
        g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_CHAINING;
        g_field_object_states[actor->object_index].retry_count = 0;
    }
    if (g_field_object_states[actor->object_index].flags & FIELD_OBJECT_ACTIONS_RESTRICTED)
    {
        if (action != FIELD_ACTION_RESTRICTED_FIRST && action != FIELD_ACTION_RESTRICTED_LAST)
        {
            if (action != FIELD_ACTION_NONE)
            {
                func_800A3938(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
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
 */
void field_update_actor_run_button(FieldActor* actor, s32 player)
{
    u16 raw;
    u32 buttons;
    ControllerPortState* ports;

    ports = CONTROLLER_STATE->ports;
    if (!(g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS))
    {
        buttons = 0;
        if (ports[player].published_sample.device_type < CONTROLLER_DEVICE_CONFIGURING)
        {
            raw = ports[player].published_sample.held_buttons;
            buttons = ((raw << 8) & 0xFF00) | (raw >> 8);
        }

        buttons = PAD_REMAP_FACE_BITS(buttons);
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
