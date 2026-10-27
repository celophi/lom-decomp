#ifndef FIELD_RESOURCE_ACTIONS_H
#define FIELD_RESOURCE_ACTIONS_H

#include "common.h"

#include "common/saved_game.h"

/**
 * @file field_resource_actions.h
 * @brief Action slots supplied by each loaded actor resource.
 */

/** @brief Number of action slots in one resource's action row. */
#define FIELD_RESOURCE_ACTION_COUNT 50
/** @brief Action slots of the two abilities a player assigns to buttons. */
#define FIELD_RESOURCE_ACTION_ABILITY_0 0
#define FIELD_RESOURCE_ACTION_ABILITY_1 1
/** @brief Action slot of the Quick Attack combo (the "Quick Attacks" button action). */
#define FIELD_RESOURCE_ACTION_COMBO 2
/** @brief Action slot of the Power Attack (the "Pow Attacks" button action). */
#define FIELD_RESOURCE_ACTION_POWER_ATTACK 3
/** @brief Power Attack follow-ups: after a Lunge or Defensive Lunge, after a Power Attack chained into a jump, and after a Tackle or Somersault. */
#define FIELD_RESOURCE_ACTION_DASH_POWER_ATTACK 8
#define FIELD_RESOURCE_ACTION_JUMP_POWER_ATTACK 9
#define FIELD_RESOURCE_ACTION_TACKLE_POWER_ATTACK 10
/** @brief Action slot that action chains program; a slot-11 action runs the command stored there. */
#define FIELD_RESOURCE_ACTION_CHAIN 11
/** @brief Number of action slots that can be bound to buttons (the slots below it). */
#define FIELD_RESOURCE_ACTION_BUTTON_COUNT 12
/** @brief First action blocked while g_field_actions_limited is set. */
#define FIELD_ACTION_BLOCKABLE_FIRST 4
/** @brief Target filter of an action with no ground effect, which collects no targets. */
#define FIELD_ACTION_TARGET_NONE 0xFF

/** @brief FieldActionSlot::command bit: the action runs a technique sequence. */
#define FIELD_ACTION_TECHNIQUE 0x8000
/** @brief FieldActionSlot::command bits holding the technique index. */
#define FIELD_ACTION_TECHNIQUE_MASK 0x7FFF
/** @brief First technique sequence id. */
#define FIELD_TECHNIQUE_SEQUENCE_BASE 0x88
/** @brief FieldActionSlot::animation value that starts no animation. */
#define FIELD_ACTION_NO_ANIMATION 0xFFFF

/** @brief Animation request bit: the request plays on the bound animation actor. */
#define FIELD_REQUEST_BOUND 0x8000
/** @brief Animation request bit: the request selects layers of the bound actor. */
#define FIELD_REQUEST_LAYERED 0x4000
/** @brief Animation request bit: the request needs targets. */
#define FIELD_REQUEST_TARGETED 0x0800
/** @brief Animation request bit: the layers play together instead of one layer. */
#define FIELD_REQUEST_ALL_LAYERS 0x0400
/** @brief Shift of the two animation request layer bits. */
#define FIELD_REQUEST_LAYER_SHIFT 12
/** @brief Animation request layer mask (after the shift). */
#define FIELD_REQUEST_LAYER_MASK 3
/** @brief Animation request bits holding the animation resource. */
#define FIELD_REQUEST_ANIMATION_MASK 0x3FF

/** @brief Flag halfword of an action slot. */
typedef struct
{
    u16 target_filter : 8; /**< Ground effect (GroundEffectKind) whose area selects the targets, FIELD_ACTION_TARGET_NONE for none. */
    u16 target_group : 2;  /**< Target group mode (opposite / same group). */
    u16 instrument : 1;    /**< Action plays an instrument (charge) animation. */
    u16 unknown_0xb : 5;
} FieldActionFlags;

/** @brief One eight-byte action slot of a resource's action row. */
typedef struct FieldActionSlot
{
    u16 command;            /**< Action command; bit 15 marks a technique. */
    FieldActionFlags flags;
    u16 animation;          /**< Animation started by the action, 0 for none. */
    u16 parameter;          /**< Animation request, sequence id or effect flags, depending on the command. */
} FieldActionSlot;

/** @brief Action row of one resource (g_field_resource_actions). */
typedef struct
{
    FieldActionSlot slots[FIELD_RESOURCE_ACTION_COUNT];
} FieldActionRow;

/** @brief Action rows indexed by the loaded actor resource. */
extern FieldActionRow g_field_resource_actions[];

/** @brief Default action animation and parameter of each ability, as byte pairs at ability * 2 (animation) and ability * 2 + 1 (parameter). */
extern u8 g_field_action_animation_parameters[];

#endif
