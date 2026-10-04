#ifndef FIELD_HELD_ACTION_BUTTONS_H
#define FIELD_HELD_ACTION_BUTTONS_H

/**
 * @file field_held_action_buttons.h
 * @brief Action button bindings, and the JP inline copy of field_get_held_action_buttons.
 */

#include "common.h"
#include "main/main.h"
#include "main/controller_internal.h"
#include "field_actor.h"

/** @brief Number of action buttons a player can bind (four face buttons, four shoulder buttons). */
#define FIELD_BUTTON_BINDING_COUNT 8

/** @brief Pad button mask of each bindable button, in binding order. */
extern u8 g_field_binding_buttons[FIELD_BUTTON_BINDING_COUNT];
/** @brief Action binding code of each action. */
extern u8 g_field_action_binding_codes[];

#if defined(VERSION_JP) && !defined(FIELD_HELD_ACTION_BUTTONS_OUT_OF_LINE)
/**
 * @brief Collect the held buttons that are bound to an action (JP inline copy).
 * @param player Controller port and saved character index.
 * @param action Action whose bound buttons are collected.
 * @param actor Actor that must be a party member in control mode 0.
 * @return The held bound buttons, or zero when input is unavailable.
 * @note JP expands this in the actor update units; the out-of-line definition is
 *       in field_actor_input_actions.c.
 */
extern inline s32 field_get_held_action_buttons(s32 player, s32 action, FieldActor* actor)
{
    s32 mask;
    s32 i;
    u8 code;
    ControllerPortState* ports = CONTROLLER_STATE->ports;
    ControllerSample* sample;

    if (actor->object_index >= FIELD_PARTY_COUNT || (actor->control.word & FIELD_CONTROL_MODE_MASK) != 0)
    {
    no_buttons:
        return 0;
    }
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
    if (sample->device_type >= CONTROLLER_DEVICE_CONFIGURING)
    {
        goto no_buttons;
    }
    return ((u32)sample->held_buttons >> 8) & mask;
}
#endif

#endif
