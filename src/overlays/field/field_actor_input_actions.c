/**
 * @file field_actor_input_actions.c
 * @brief Map controller input to actor actions, animation IDs, and input-state changes.
 */

#include "common.h"
#include "field_calls.h"
#include "controller_internal.h"
#include "field_contact_geometry.h"
#include "field_object_state.h"
#include "field_text.h"

/** @brief Stride of one player's block in the pad context. */
#define FIELD_PAD_PLAYER_STRIDE 0x250

/** @brief Offset of a player's eight action bindings in the pad context. */
#define FIELD_PAD_ACTION_BINDINGS 0x638

/** @brief Number of action bindings per player. */
#define FIELD_ACTION_BINDING_COUNT 8

/** @brief Number of actions with an animation-map entry. */
#define FIELD_ACTION_MAP_ENTRY_COUNT 11

/** @brief Action returned by func_800A29F8 when no action is queued. */
#define FIELD_ACTION_NONE 0xFF

/** @brief One action-animation map: eleven animation ids and their disable flags. */
typedef struct
{
    u16 animations[FIELD_ACTION_MAP_ENTRY_COUNT];
    u8 disabled[FIELD_ACTION_MAP_ENTRY_COUNT];
    u8 pad;
} FieldActionMap;

/** @brief Actor fields updated from the held buttons (0x54-byte actor record). */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x33 - 0x25];
    u8 button_held;
    u8 pad34[0x3B - 0x34];
    u8 resource_index;
} FieldInputActor;

/** @brief Resource entry flags; bit 0 disables input handling. */
typedef struct
{
    u8 pad0[0x10];
    u32 flags;
} FieldInputResource;

extern u8 D_800EB23C[];
extern u8 D_800EB244[];
extern u8* g_pad_ctx;
extern FieldMotionRecord g_field_actors[];
extern s32 g_field_active_group;
extern s32 D_800F229C;
extern s32 D_8010AE78;
extern s32 g_field_buffered_input;
extern FieldActionMap g_field_action_animation_maps[];
extern s32 g_field_action_context;
extern s32 D_8010AE54;
extern FieldInputResource g_field_resource_entries[];

/**
 * @brief Build the held-button mask of the buttons bound to an action.
 * @param player Controller port and pad-context player index.
 * @param action Action whose bound buttons are collected.
 * @param actor Actor that must be a party member with no active motion flags.
 * @return The held bound buttons, or zero when input is unavailable.
 */
s32 func_80091728(s32 player, s32 action, FieldMotionRecord* actor)
{
    s32 mask;
    s32 i;
    u8 binding;
    ControllerPortState* ports;
    ControllerSample* sample;
    u32 buttons;
    u8* player_block;

    ports = CONTROLLER_STATE->ports;
    if (actor->source_object_index < 3)
    {
        if ((actor->flags & 0x1FF) == 0)
        {
            mask = 0;
            i = 0;
            binding = D_800EB244[action];
            player_block = g_pad_ctx + player * FIELD_PAD_PLAYER_STRIDE;
            do
            {
                if (binding == *(player_block + i + FIELD_PAD_ACTION_BINDINGS))
                {
                    mask |= D_800EB23C[i];
                }
                i += 1;
            } while (i < FIELD_ACTION_BINDING_COUNT);
            sample = &ports[player].published_sample;
            if (sample->device_type < CONTROLLER_DEVICE_CONFIGURING)
            {
                buttons = (u32)sample->held_buttons >> 8;
                return (((buttons >> 1) & 0x20) | ((buttons & 0x20) * 2) | ((buttons >> 3) & 0x10) | ((buttons & 0x10) * 8) | (buttons & 0xF)) & mask;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}

/**
 * @brief Start a leader interaction probe when the confirm input is buffered and nothing blocks it.
 */
void func_8009184C(void)
{
    s32 first_status;
    s32 second_status;

    if ((g_field_active_group == 0) && (g_field_actors[0].motion_parameter == 0) && (D_800F229C == 0) && (D_8010AE78 == 0) && (g_field_buffered_input & 0x220))
    {
        first_status = field_text_get_status(0);
        second_status = field_text_get_status(1);
        if ((g_field_active_group == 0) && (first_status == -1) && (second_status == first_status))
        {
            field_probe_actor_interaction(g_field_actors);
        }
    }
}

/**
 * @brief Resolve an actor action into an enabled animation ID.
 * @param actor Actor state to query and update.
 * @param map_index Action-animation map passed to the action selector.
 * @return Enabled animation ID, or zero when no animation is available.
 */
u16 func_80091914(FieldMotionRecord* actor, s32 map_index)
{
    s32 action;

    action = func_800A29F8(map_index, (actor->facing_or_reward_kind >> 7) ^ 1, 0);
    if (action != FIELD_ACTION_NONE)
    {
        g_field_action_context = (g_field_action_context & ~0xFF) | action;
    }
    else if ((actor->facing_or_reward_kind & 0x7F) != 0x3D)
    {
        actor->reference_index = 0;
        g_field_object_states[actor->source_object_index].object_flags &= 0xFFFF7FFF;
        g_field_object_states[actor->source_object_index].targets[13] = 0;
    }
    if (g_field_object_states[actor->source_object_index].object_flags & 0x400)
    {
        if (action != 2 && action != 3)
        {
            if (action != FIELD_ACTION_NONE)
            {
                func_800A3938(0x78, 0x80);
            }
            return 0;
        }
    }
    if (action < FIELD_ACTION_MAP_ENTRY_COUNT)
    {
        if (D_8010AE54 == 0 || action < 4)
        {
            if (g_field_action_animation_maps[map_index].animations[action] != 0 && g_field_action_animation_maps[map_index].disabled[action] == 0)
            {
                return g_field_action_animation_maps[map_index].animations[action];
            }
        }
    }
    return 0;
}

/**
 * @brief Track whether an actor's controller holds one of the 0x44 buttons.
 * @param actor Actor state to update.
 * @param port Controller port to read.
 */
void func_80091AC8(FieldInputActor* actor, s32 port)
{
    u16 raw;
    s32 input;
    ControllerPortState* ports;

    ports = CONTROLLER_STATE->ports;
    if (!(g_field_resource_entries[actor->resource_index].flags & 1))
    {
        input = 0;
        if (ports[port].published_sample.device_type < CONTROLLER_DEVICE_CONFIGURING)
        {
            raw = ports[port].published_sample.held_buttons;
            input = ((raw << 8) & 0xFF00) | (raw >> 8);
        }

        input = ((u32)(input & 0x40) >> 1) | ((input & 0x20) * 2) | ((u32)(input & 0x80) >> 3) | ((input & 0x10) * 8) | (input & ~0xF0);
        if (input & 0x44)
        {
            if (actor->button_held == 0)
            {
                actor->button_held = 1;
                actor->unk24 = 0;
            }
        }
        else if (actor->button_held != 0)
        {
            actor->button_held = 0;
            actor->unk24 = 0;
        }
    }
}
