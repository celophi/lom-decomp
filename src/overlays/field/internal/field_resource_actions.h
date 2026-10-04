#ifndef FIELD_RESOURCE_ACTIONS_H
#define FIELD_RESOURCE_ACTIONS_H

#include "common.h"

/**
 * @file field_resource_actions.h
 * @brief Action slots supplied by each loaded actor resource.
 */

/** @brief Number of action slots in one resource's action row. */
#define FIELD_RESOURCE_ACTION_COUNT 50
/** @brief Target filter of an action without a target predicate. */
#define FIELD_ACTION_TARGET_NONE 0xFF

/** @brief Flag halfword of an action slot. */
typedef struct
{
    u16 target_filter : 8; /**< Target predicate index, FIELD_ACTION_TARGET_NONE for none. */
    u16 target_group : 2;  /**< Target group mode (opposite / same group). */
    u16 instrument : 1;    /**< Action plays an instrument (charge) animation. */
    u16 unknown_0xb : 5;
} FieldActionFlags;

/** @brief One eight-byte action slot of a resource's action row. */
typedef struct
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

#endif
