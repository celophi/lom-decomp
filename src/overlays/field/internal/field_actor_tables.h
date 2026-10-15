#ifndef FIELD_ACTOR_TABLES_H
#define FIELD_ACTOR_TABLES_H

#include "common.h"
#include "field_actor.h"
#include "field_actor_binding.h"
#include "field_player_records.h"
#include "field_resource_actions.h"
#include "overlays/field/field_actor_records.h"

/**
 * @file field_actor_tables.h
 * @brief Field actor record, object state, animation slot and binding tables
 *        shared by the actor command interpreters.
 *
 * The field keeps three parallel per-object tables indexed the same way:
 * g_field_actors (script and motion state), g_field_object_states (runtime
 * state; its key word identifies the object to scripts) and
 * g_field_object_parts (model part data). Animation actors live in the
 * separate g_field_actor_slots pool; g_field_actor_bindings ties up to three
 * of them to the party objects.
 */

/** @brief Resting value of the HUD shake counters (FieldPlayerRecord::hit_state). */
#define FIELD_HUD_SHAKE_IDLE 0xFF
/** @brief First frame of the HUD panel shake started by a hit (counts down to 0). */
#define FIELD_HUD_SHAKE_START 5
/** @brief HUD shake counter of the boss panel (FIELD_HUD_SHAKE_IDLE when still). */
extern s32 g_field_boss_hud_shake_frame;

/** @brief Number of general-purpose animation actor slots. */
#define FIELD_ACTOR_SLOT_COUNT 48

/** @brief First animation actor slot owned by an object (one per object, in object order). */
#define FIELD_OBJECT_SLOT_BASE FIELD_ACTOR_SLOT_COUNT
/** @brief Each object's effect animations play in animation actor slot 64 + object index. */
#define FIELD_OBJECT_EFFECT_SLOT_BASE 64

/** @brief Total number of animation actor slots (general, per-object and per-object effect slots). */
#define FIELD_ACTOR_SLOT_TOTAL 80


/** @brief FieldObjectState::interaction_kind of a dropped item the player can pick up. */
#define FIELD_INTERACTION_ITEM 1

/** @brief Sentinel returned by the actor lookups when no object matches. */
#define FIELD_ACTOR_NONE ((FieldActor*)-1)

/** @brief Script index of an actor without a running script. */
#define FIELD_SCRIPT_NONE 0xFF

/** @brief Script index of an actor running its object state's private script. */
#define FIELD_SCRIPT_OBJECT 0xFE

/** @brief First resource read id of the streamed animation resources. */
#define FIELD_ANIMATION_RESOURCE_BASE 0x2DC

/**
 * @brief Map size words of the fixed field map header block at 0x801ED400.
 * @note Width and depth are screen pixels; world positions have FIELD_POSITION_SHIFT fractional bits, with z projected at half scale.
 */
typedef struct
{
    s16 width;
    u16 depth;
    /** Bit 1 of the selected object's background flags (field_select_object). */
    u8 unk4;
} FieldMapBounds;

/** @brief Fixed address of the field map header block. */
#define FIELD_MAP_BOUNDS ((FieldMapBounds*)MAP_BOUNDS_ADDRESS)

/** @brief FieldObjectState::hud flag (FieldStatusState::level byte) showing the technique gauge. */
#define FIELD_HUD_SHOW_TECHNIQUE_GAUGE 0x01

/** @brief Sentinel returned by field_find_object_state when no object matches. */
#define FIELD_OBJECT_STATE_NONE ((FieldObjectState*)-1)


/** @brief Parameter curve count of an animation (curve selectors are four bits). */
#define FIELD_CURVE_COUNT 16
/** @brief Curve selector of an unused render-state track. */
#define FIELD_CURVE_NONE 0xFF
/** @brief Neutral global colour scale. */
#define FIELD_COLOR_SCALE_NEUTRAL 0x100


/** @brief FieldActorSlot::status bit 0: the slot plays a streamed animation bound to its owner. */
#define FIELD_SLOT_OWNER_LINKED 0x1
/** @brief FieldActorSlot::status bits holding the action element (stored shifted by one); all set means none. */
#define FIELD_SLOT_ELEMENT_BITS 0x1E
#define FIELD_SLOT_ELEMENT_SHIFT 1


extern FieldObjectState g_field_scene_object_states[];
extern FieldObjectPart g_field_object_parts[];

/**
 * @brief Find the field actor whose object state carries @p key.
 * @param key Object key compared against each object state's key word.
 * @return The matching actor record, or FIELD_ACTOR_NONE.
 */
static inline FieldActor* field_find_actor(s32 key)
{
    FieldObjectState* state;
    FieldActor* actor;
    s32 i;

    actor = g_field_actors;
    state = g_field_object_states;
    for (i = 0; i < FIELD_ACTOR_COUNT; i++, state++, actor++)
    {
        if (state->key == key)
        {
            return actor;
        }
    }
    return FIELD_ACTOR_NONE;
}

#endif
