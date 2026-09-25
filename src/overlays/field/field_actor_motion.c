/**
 * @file field_actor_motion.c
 * @brief Per-frame update of a field actor's action animation and the
 *        screen-edge test for proposed movement.
 */

#include "common.h"
#include "display.h"
#include "field_actor_tables.h"
#include "field_calls.h"
#include "field_effect_render_state.h"
#include "field_types.h"
#include "vector.h"

/** @brief Scratchpad vector that receives the actor displacement. */
#define FIELD_SCRATCH_DISPLACEMENT ((Vec3i*)0x1F800000)

/** @brief Action animations that continue while the action's buttons are held (10 is the guard). */
#define FIELD_ANIMATION_GUARD 10
#define FIELD_ANIMATION_HELD_31 0x31
/** @brief Action animation whose handler also runs after the animation has finished. */
#define FIELD_ANIMATION_35 0x35
/** @brief Action animations that leave the actor facing the other way. */
#define FIELD_ANIMATION_TURN_37 0x37
#define FIELD_ANIMATION_TURN_3B 0x3B

/** @brief Number of per-object actions that can be bound to buttons. */
#define FIELD_BOUND_ACTION_COUNT 12
/** @brief Objects below this index are the two players. */
#define FIELD_PLAYER_COUNT 2
/** @brief FieldObjectState::action_parameter value of an object without an action. */
#define FIELD_ACTION_PARAMETER_NONE 0xFFFF

/** @brief FieldObjectState::flags bit set on an object while another object is linked to it. */
#define FIELD_OBJECT_FLAG_LINK_TARGET 0x2000
/** @brief FieldObjectState::flags bit cleared when an action ends (placeholder name). */
#define FIELD_OBJECT_FLAG_4000 0x4000
/** @brief FieldObjectState::movement sequence bits cleared when an action ends. */
#define FIELD_MOVEMENT_SEQUENCE_0800 0x0800
#define FIELD_MOVEMENT_SEQUENCE_1000 0x1000
/** @brief FieldObjectState::movement bit: the object overlaps another one. */
#define FIELD_MOVEMENT_OVERLAPPING 0x4000

/** @brief The slide displacement is scaled by FieldObjectPart::scale_z / 64 (0x40 = full size). */
#define FIELD_PART_SCALE_SHIFT 6

/** @brief Screen margins a moving actor may not cross. */
#define FIELD_SCREEN_EDGE_LEFT 6
#define FIELD_SCREEN_EDGE_RIGHT (SCREEN_WIDTH - 6)
#define FIELD_SCREEN_EDGE_TOP 9
#define FIELD_SCREEN_EDGE_BOTTOM VRAM_DRAW_HEIGHT

/** @brief Proposed field position and its screen projection. */
typedef struct
{
    FieldVector position;
    Vec2s screen;
} FieldScreenProbe;

/* The owner header include/field_actor_sequence_runtime.h declares its own
 * g_field_actor_bindings type, which conflicts with field_actor_tables.h. */
void field_update_sequence_actor_binding(FieldActor* actor, s32 release_actor);
void field_restart_sequence_animation(FieldActor* actor);

/* include/field_contact_geometry.h pulls in the FieldMotionRecord view of the
 * actor tables, which conflicts with field_actor_tables.h. */
s32 field_resolve_actor_movement(FieldActor* actor, Vec3i* delta, s32 mode);

void field_release_object_link(FieldActor* actor);

/**
 * @brief Run one frame of an actor's action command.
 * @param actor Actor running the action.
 * @param update_action Non-zero runs the action state handler first.
 * @return Never set; the function is declared int but callers ignore the value.
 * @note While the animation plays, the remaining slide speed moves the actor
 *       along its facing; once it has finished, the action is wound down.
 */
s32 field_update_actor_action(FieldActor* actor, s32 update_action)
{
    Vec3i* displacement = FIELD_SCRATCH_DISPLACEMENT;
    s32 unused[2]; /* never used, but the original stack frame reserves it */
    s32 animation;
    s32 step;
    FieldObjectPart* part;

    if (update_action != 0 &&
        (actor->animation_state != 0 || (actor->animation & FIELD_ANIMATION_INDEX_MASK) == FIELD_ANIMATION_35))
    {
        func_80092C98((struct FieldMotionRecord*)actor);
    }
    if (actor->animation_state == 0)
    {
        animation = actor->animation & FIELD_ANIMATION_INDEX_MASK;
        if (animation == FIELD_ANIMATION_GUARD || animation == FIELD_ANIMATION_HELD_31)
        {
            if (g_field_object_states[actor->object_index].action < FIELD_BOUND_ACTION_COUNT)
            {
                if (field_get_held_action_buttons(actor->object_index, g_field_object_states[actor->object_index].action, actor) != 0)
                {
                    return;
                }
            }
        }
        if (g_field_object_states[actor->object_index].contact.bits.linked)
        {
            if (field_object_has_active_actor_tracks(actor->object_index) != 0)
            {
                return;
            }
            g_field_object_states[g_field_object_states[actor->object_index].linked_object_index].flags &= ~FIELD_OBJECT_FLAG_LINK_TARGET;
        }
        g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_FLAG_4000;
        field_update_sequence_actor_binding(actor, 1);
        g_field_object_states[actor->object_index].movement.word &= ~(FIELD_MOVEMENT_SEQUENCE_0800 | FIELD_MOVEMENT_SEQUENCE_1000);
        animation = actor->animation & FIELD_ANIMATION_INDEX_MASK;
        if (animation == FIELD_ANIMATION_TURN_37 || animation == FIELD_ANIMATION_TURN_3B)
        {
            actor->animation ^= FIELD_ANIMATION_FACING;
        }
        if (actor->object_index < FIELD_PLAYER_COUNT && func_80093AB8((struct FieldMotionRecord*)actor) != 0)
        {
            return;
        }
        if (func_80092AD8((struct FieldMotionRecord*)actor) != 0)
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            actor->animation &= FIELD_ANIMATION_FACING;
            field_restart_sequence_animation(actor);
        }
        if (actor->command == FIELD_ACTOR_COMMAND_NONE)
        {
            g_field_object_states[actor->object_index].action_parameter = FIELD_ACTION_PARAMETER_NONE;
            field_release_object_link(actor);
        }
    }
    else
    {
        step = actor->speed_accumulator / actor->frame_timer;
        actor->speed_accumulator -= step;
        part = &g_field_object_parts[actor->object_index];
        if (actor->animation & FIELD_ANIMATION_FACING)
        {
            displacement->x = ((step << 8) * part->scale_z) >> FIELD_PART_SCALE_SHIFT;
        }
        else
        {
            displacement->x = (-(step << 8) * part->scale_z) >> FIELD_PART_SCALE_SHIFT;
        }
        displacement->y = 0;
        displacement->z = 0;
        field_resolve_actor_movement(actor, displacement, 1);
        if (g_field_resource_entries[actor->resource_index].unk8 == 0)
        {
            g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_OVERLAPPING;
        }
    }
}

/**
 * @brief Test whether a proposed move takes an actor past the edge of the screen.
 * @param actor Actor whose fixed-point position is tested.
 * @param delta Proposed displacement; only X and Z are tested.
 * @return 1 when the move crosses the screen edge it heads for, otherwise 0.
 * @note The projected coordinates are truncated to 16 bits before the view offsets are added.
 */
s32 field_move_leaves_screen(FieldActor* actor, Vec3i* delta)
{
    FieldScreenProbe probe;

    probe.position.vx = actor->x + delta->x;
    probe.position.vy = actor->y;
    probe.position.vz = actor->z + delta->z;
    probe.screen.x = g_field_view_offset_x / 256 + (s16)(probe.position.vx / 256 + SCREEN_WIDTH / 2);
    probe.screen.y = g_field_view_offset_y / 256 + (s16)(probe.position.vy / 256 + VRAM_DRAW_HEIGHT / 2) - probe.position.vz / 512 -
                     g_field_view_offset_z / 512;
    if (delta->x < 0)
    {
        if (probe.screen.x < FIELD_SCREEN_EDGE_LEFT)
        {
            return 1;
        }
    }
    else if (delta->x > 0)
    {
        if (probe.screen.x >= FIELD_SCREEN_EDGE_RIGHT)
        {
            return 1;
        }
    }
    if (delta->z > 0)
    {
        if (probe.screen.y < FIELD_SCREEN_EDGE_TOP)
        {
            return 1;
        }
    }
    else if (delta->z < 0)
    {
        if (probe.screen.y >= FIELD_SCREEN_EDGE_BOTTOM)
        {
            return 1;
        }
    }
    return 0;
}
