#ifndef FIELD_ACTOR_BINDING_H
#define FIELD_ACTOR_BINDING_H

#include "common.h"

/**
 * @file field_actor_binding.h
 * @brief Bindings that tie the party objects to their animation actor slots.
 */

/** @brief Number of animation actor bindings (party members 0, 1 and everyone else). */
#define FIELD_ACTOR_BINDING_COUNT 3
/** @brief Binding shared by every object from FIELD_PLAYER_COUNT up. */
#define FIELD_SHARED_BINDING (FIELD_ACTOR_BINDING_COUNT - 1)

/** @brief FieldActorBinding::state values. */
#define FIELD_BINDING_IDLE 0
#define FIELD_BINDING_LOADING 1
#define FIELD_BINDING_READY 2

/** @brief Animation actor bound to a party object (0x1C bytes). */
typedef struct
{
    /** @brief FIELD_BINDING_IDLE, FIELD_BINDING_LOADING or FIELD_BINDING_READY. */
    s32 state;
    /** @brief Resource read queued for the animation, 0 once it has been unpacked. */
    s32 load_id;
    /** @brief Animation resource number (load_id - FIELD_ANIMATION_RESOURCE_BASE). */
    s32 resource_id;
    /** @brief Object that owns the bound animation. */
    s32 owner;
    s32 unk10;
    s32 unk14;
    /** @brief Animation actor slot (g_field_actor_slots) playing the bound animation. */
    s32 slot;
} FieldActorBinding;

extern FieldActorBinding g_field_actor_bindings[];

/**
 * @brief Binding used by object @p object_index (objects 2 and up share the last binding).
 * @param object_index Object index; only its low byte selects the binding.
 * @return The object's animation actor binding.
 */
static inline FieldActorBinding* field_object_binding(s32 object_index)
{
    FieldActorBinding* bindings;
    s32 offset;

    bindings = g_field_actor_bindings;
    if ((u8)object_index < FIELD_SHARED_BINDING)
    {
        offset = object_index * sizeof(FieldActorBinding);
    }
    else
    {
        offset = FIELD_SHARED_BINDING * sizeof(FieldActorBinding);
    }
    return (FieldActorBinding*)((u8*)bindings + offset);
}

#endif
