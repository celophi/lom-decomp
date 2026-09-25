/** @file field_actor_behavior.c
 * @brief Party follower routes, controller movement, actor commands, and action
 *        animation dispatch.
 *
 * One translation unit: g_field_follower_animation_map (used only by the route
 * code) follows the behavior tables g_field_actor_turn_animations ..
 * g_field_action_sound_ids in .data, so route and behavior data share one object.
 */

#include "common.h"
#include "field_actor_tables.h"
#include "field_actor_routes.h"
#include "field_actor_behavior.h"
#include "field_text.h"
#include "field_effect_render_state.h"
#include "field_actor_runtime.h"
#include "field_calls.h"
#include "field_types.h"
#include "controller_internal.h"
#include "scene_state.h"
#include "sdk/libetc.h"
#include "sdk/memory.h"
#include "sdk/rand.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Route history entries between the leader and each follower. */
#define FIELD_ROUTE_FOLLOWER_SAMPLES (FIELD_ROUTE_HISTORY_LENGTH / 2)
/** @brief Minimum route distance between two samples. */
#define FIELD_ROUTE_MIN_SPACING 3
/** @brief Normalized directions carry twelve fractional bits. */
#define FIELD_DIRECTION_SHIFT 12
/** @brief Squared distance from the next route sample at which a follower runs. */
#define FIELD_ROUTE_RUN_DISTANCE_SQUARED 385
/** @brief Squared distance per follower slot at which a follower closes up on the leader. */
#define FIELD_ROUTE_FOLLOW_DISTANCE_SQUARED 3000

/** @brief Headings are 256 steps per turn (ratan2/rcos use 4096). */
#define FIELD_HEADING_STEPS 256
/** @brief Shift from a 4096-step angle to a heading. */
#define FIELD_HEADING_ANGLE_SHIFT 4
/** @brief Half of one eighth of a turn, added before a heading is split into eight sectors. */
#define FIELD_HEADING_HALF_SECTOR 16
/** @brief Shift from a heading to one of eight sectors. */
#define FIELD_HEADING_SECTOR_SHIFT 5
/** @brief Heading bits of a sector (the heading snapped to an eighth of a turn). */
#define FIELD_HEADING_SECTOR_MASK 0xE0

/** @brief FieldActor::control bit set while the actor moves. */
#define FIELD_CONTROL_MOVING 0x200
/** @brief FieldActor::control second movement bit (placeholder name; only ever cleared here). */
#define FIELD_CONTROL_MOVEMENT_UNK400 0x400
/** @brief FieldActor::control movement bits. */
#define FIELD_CONTROL_MOVEMENT_MASK (FIELD_CONTROL_MOVING | FIELD_CONTROL_MOVEMENT_UNK400)
/** @brief Shift of the two FieldActor::control movement bits. */
#define FIELD_CONTROL_MOVEMENT_SHIFT 9

/** @brief First walking animation. */
#define FIELD_ANIMATION_WALK 5
/** @brief First running animation. */
#define FIELD_ANIMATION_RUN 10
/** @brief Animation that starts a pending instrument action. */
#define FIELD_ANIMATION_INSTRUMENT_START 0x0E
/** @brief Animation of an instrument action being released. */
#define FIELD_ANIMATION_INSTRUMENT 0x0F
/** @brief Animation of an instrument action waiting for its target. */
#define FIELD_ANIMATION_INSTRUMENT_READY 0x10
/** @brief Terminator of g_field_actor_turn_animations. */
#define FIELD_TURN_ANIMATION_END 0xFF

/** @brief FieldObjectState::flags bit that mirrors the controller directions. */
#define FIELD_OBJECT_FLAG_REVERSED 0x08

/** @brief FieldObjectState::movement bit: the object overlaps another one. */
#define FIELD_MOVEMENT_OVERLAPPING 0x4000
/** @brief FieldObjectState::movement bit: a technique sequence is running. */
#define FIELD_MOVEMENT_TECHNIQUE 0x8000
/** @brief FieldObjectState::movement bit: the charge animation was started. */
#define FIELD_MOVEMENT_CHARGED 0x400
/** @brief FieldObjectState::movement low bits holding the effect scale. */
#define FIELD_MOVEMENT_SCALE_MASK 0x3FF

/** @brief FieldObjectState::contact bit that keeps an object out of the target list (placeholder name). */
#define FIELD_CONTACT_UNK20 0x20
/** @brief FieldObjectState::contact bit: the object is charging or waiting for an action. */
#define FIELD_CONTACT_ACTION_PENDING 0x40
/** @brief FieldObjectState::contact bit: the object is a target of a running action. */
#define FIELD_CONTACT_TARGETED 0x80
/** @brief FieldObjectState::contact bits cleared before an action is prepared. */
#define FIELD_CONTACT_ACTION_BITS 0x1C
/** @brief FieldObjectState::contact bit cleared when an action is accepted (placeholder name). */
#define FIELD_CONTACT_UNK02 0x02

/** @brief Value of FieldObjectState::contact.bytes.animation_actor_index without an actor. */
#define FIELD_ANIMATION_ACTOR_NONE 0xFF
/** @brief FieldObjectState::technique_gauge value of a full gauge. */
#define FIELD_TECHNIQUE_GAUGE_FULL 0xFF
/** @brief Gauge added by the gauge-up animation. */
#define FIELD_TECHNIQUE_GAUGE_STEP 0x20
/** @brief FieldObjectState::action_charge below which an action still charges. */
#define FIELD_ACTION_CHARGE_FULL 0xFF
/** @brief FieldObjectState::action_charge from which the charge animation starts. */
#define FIELD_ACTION_CHARGE_ANIMATION 0x41

/** @brief First resource action slot that is not a pad button action. */
#define FIELD_RESOURCE_ACTION_BUTTONS 12
/** @brief Action slot a mode-11 action redirects through. */
#define FIELD_RESOURCE_ACTION_REDIRECT 11
/** @brief Resource action slots of the two combo buttons. */
#define FIELD_RESOURCE_ACTION_COMBO 2

/** @brief FieldResourceAction::command bit: the action runs a technique sequence. */
#define FIELD_ACTION_TECHNIQUE 0x8000
/** @brief FieldResourceAction::command bits holding the technique index. */
#define FIELD_ACTION_TECHNIQUE_MASK 0x7FFF
/** @brief FieldResourceAction::flags bit: the action plays an instrument. */
#define FIELD_ACTION_INSTRUMENT 0x400
/** @brief FieldResourceAction::flags value without an action handler. */
#define FIELD_ACTION_NO_HANDLER 0xFF
/** @brief Shift of the two FieldResourceAction::flags target mode bits. */
#define FIELD_ACTION_TARGET_MODE_SHIFT 8
/** @brief FieldResourceAction::flags target mode mask (after the shift). */
#define FIELD_ACTION_TARGET_MODE_MASK 3
/** @brief FieldResourceAction::flags bits of the action element (after the handler byte). */
#define FIELD_ACTION_ELEMENT_MASK 0x0F
/** @brief FieldResourceAction::animation values that start no animation. */
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

/** @brief Maximum number of targets handed to an action animation. */
#define FIELD_ACTION_MAX_TARGETS 9
/** @brief Free actor slots an action needs before it may start. */
#define FIELD_ACTION_MIN_FREE_SLOTS 3
/** @brief Frames the combo timer runs after a combo action. */
#define FIELD_COMBO_WINDOW 10
/** @brief Weapon type without a variant animation for command 0x1F. */
#define FIELD_WEAPON_TYPE_NO_VARIANT 10
/** @brief Techniques per weapon type in the technique sequence table. */
#define FIELD_TECHNIQUES_PER_WEAPON 24
/** @brief First technique sequence id. */
#define FIELD_TECHNIQUE_SEQUENCE_BASE 0x88
/** @brief Sequence id bit marking a technique sequence. */
#define FIELD_SEQUENCE_TECHNIQUE 0x8000

/** @brief Sound played when an action cannot start. */
#define FIELD_SOUND_ACTION_REFUSED 0x78
/** @brief Centre pan for func_800A3938. */
#define FIELD_SOUND_PAN_CENTRE 0x80
/** @brief g_field_action_sound_ids value without a sound. */
#define FIELD_SOUND_NONE 0xFF

/** @brief Distance (squared, whole units) at which a path waypoint counts as reached while walking. */
#define FIELD_PATH_WALK_REACHED 50
/** @brief Manhattan distance (fixed point) at which a waypoint counts as reached while running. */
#define FIELD_PATH_RUN_REACHED 0x2000
/** @brief Manhattan distance (fixed point) at which a walking target counts as reached. */
#define FIELD_TARGET_WALK_REACHED 0x1000
/** @brief Manhattan distance (fixed point) at which a approaching target counts as reached. */
#define FIELD_TARGET_APPROACH_REACHED 0x2000
/** @brief Vertical step of the rise and sink commands. */
#define FIELD_VERTICAL_STEP 0x100
/** @brief Screen margin, relative to the camera, inside which a leaving actor keeps walking. */
#define FIELD_SCREEN_MARGIN_LEFT 0xA00
#define FIELD_SCREEN_MARGIN_RIGHT 0x13600
#define FIELD_SCREEN_MARGIN_BOTTOM 0x1B600

/** @brief Stick deflection of a digital direction button. */
#define FIELD_PAD_FULL_DEFLECTION 0x1000
/** @brief Screen centre used to project the actor position. */
#define FIELD_SCREEN_CENTRE_X 160
#define FIELD_SCREEN_CENTRE_Y 112
/** @brief Collision result bits of a fully blocked move. */
#define FIELD_COLLISION_BLOCKED 3
/** @brief FieldObjectPart::scale_z of a full-size object; full-size objects use the large footprint. */
#define FIELD_PART_FULL_SCALE 0x40
/** @brief Collision footprint of large and normal actors (width, depth), and the step height. */
#define FIELD_FOOTPRINT_LARGE_WIDTH 12
#define FIELD_FOOTPRINT_LARGE_DEPTH 8
#define FIELD_FOOTPRINT_WIDTH 9
#define FIELD_FOOTPRINT_DEPTH 6
#define FIELD_FOOTPRINT_STEP 16
/** @brief Collision mover mode bits cleared before a walk (see FIELD_COLLISION_MOVER_AIRBORNE in field_collision.c). */
#define FIELD_MOVER_AIRBORNE_LOW 0x10000
#define FIELD_MOVER_AIRBORNE_HIGH 0x20000
/** @brief field_find_actor_overlap result bits holding the overlapped object (0x8000 marks a hit). */
#define FIELD_OVERLAP_INDEX_MASK 0x7FFF
/** @brief func_8005B368 result without a blocking marker. */
#define FIELD_MARKER_NONE -1
/** @brief func_800AD7DC result without a combo. */
#define FIELD_COMBO_NONE 0xFF
/** @brief rand() results above this pick the second idle animation. */
#define FIELD_IDLE_ALTERNATE_THRESHOLD 0x6000
/** @brief Idle animation below which an idle actor is already settled. */
#define FIELD_ANIMATION_IDLE_COUNT 2
/** @brief Frames a stopped actor keeps its walking animation. */
#define FIELD_STOP_DELAY 3
/** @brief Heading sector (of eight) pointing straight up or down the screen. */
#define FIELD_SECTOR_UP 2
#define FIELD_SECTOR_DOWN 6

/**
 * @brief Object state placed so that its path[0] is the waypoint at byte @p offset of @p object_index's path.
 * @note The target adds the waypoint offset before the state base; indexing path[] adds it last.
 */
#define FIELD_WAYPOINT_STATE(states, object_index, offset) \
    ((FieldObjectState*)((offset) + (((object_index) * (s32)(sizeof(FieldObjectState) / sizeof(s32))) << 2) + (s32)(states)))

/**
 * @brief Action @p index of resource @p resource in g_field_resource_actions.
 * @note The target adds the row offset to the entry address (row offset first);
 *       only subtracting the negated address reproduces that operand order.
 */
#define FIELD_RESOURCE_ACTION(resource, index) \
    ((FieldResourceAction*)((resource) * (s32)sizeof(g_field_resource_actions[0]) - -(s32)&g_field_resource_actions[0][(index)]))

/** @brief Binding of a party object: members 0 and 1 own one each, everyone else shares the third. */
#define FIELD_OBJECT_BINDING(i) ((u32)(i) < FIELD_PLAYER_COUNT ? (i) : FIELD_PLAYER_COUNT)

/** @brief Scratchpad collision mover and probe. */
#define FIELD_COLLISION_MOVER ((struct FieldCollisionMover*)0x1F800000)
#define FIELD_COLLISION_PROBE ((struct FieldCollisionQuery*)0x1F800040)

/**
 * @brief Collision mover handed to func_8005B6AC (same layout as in field_collision.c).
 * @note The low half of mode_flags doubles as the footprint depth.
 */
struct FieldCollisionMover
{
    s32 x;
    s32 height;
    s32 z;
    s32 move_x;
    s32 move_height;
    s32 move_z;
    s32 resolved_height;
    s32 collision_node;
    s32 flags;
    u16 footprint_width;
    s16 height_bias;
    union
    {
        s32 mode_flags;
        s16 footprint_depth;
    } mode;
};

/** @brief Footprint query handed to func_8005B368 (same layout as in field_collision.c). */
struct FieldCollisionQuery
{
    s32 x;
    s32 y;
    s32 z;
    u16 width;
    s16 height_tolerance;
    u16 depth;
};

/** @brief One action of a resource's action table (eight bytes). */
typedef struct
{
    /** @brief Action command, or FIELD_ACTION_TECHNIQUE plus a technique index. */
    u16 command;
    /** @brief Low byte: action handler; bits 8-9: target mode; FIELD_ACTION_INSTRUMENT. */
    u16 flags;
    u16 animation;
    /** @brief Animation request (FIELD_REQUEST_* bits plus an animation resource). */
    u16 request;
} FieldResourceAction;

extern FieldResourceAction g_field_resource_actions[][FIELD_RESOURCE_ACTION_COUNT];
extern u8 g_field_route_animation_history[];
extern s32 g_field_direction_animation_modes[];
extern u8 g_field_follower_animation_map[];
extern s32 g_field_actor_walk_animations[];
extern s32 g_field_actor_diagonal_walk_animations[];
extern u8 g_field_actor_turn_animations[];
extern u8 g_field_action_animation_parameters[];
extern u8 g_field_action_sound_ids[];
extern s32 g_field_active_group;
extern u8 g_field_collision_disabled;
extern s32 g_field_pad_buttons;
extern s32 g_field_actions_limited;
extern s32 D_8010AE58;
extern s32 D_80122B20;

s32 func_8001CDAC(s32* in, s32* out);
void field_route_actor_to_object(FieldActor* actor, s32 target_index, s32 mode);
void field_restart_actor_animation(FieldActor* actor);
void field_stop_actor_animations_for_object(FieldActor* actor, s32 force);
s32 field_find_actor_overlap(void* record, void* position, s32 filter_group);
void field_start_actor_contact_interaction(void* record, s32 actor_index);
/* Defined as (void) in field_actor_slot_resources.c, but every call here passes the object index. */
s32 field_count_free_actor_slots(s32 object_index);
static s32 field_apply_action_animation(FieldActor* actor, FieldObjectState* state, FieldResourceAction* action);
static s32 field_actor_action_is_charging(FieldActor* actor);
static s32 field_start_action_animation(s32 object_index, s32 target_count, u8* targets, s32 request);
static void field_sample_actor_route(FieldRoutePoint* points, s32 remaining, FieldActor* actor, FieldActor* target, s32 heading_offset, s32 unused_limit);
static s32 field_get_route_heading_animation(FieldActor* destination, FieldActor* source);
/* Unprototyped: the original call passes a third argument (the bound actor slot) that the function ignores. */
static s32 field_filter_action_targets();

/**
 * @brief Route hook with an empty body.
 */
void func_8008C728(void)
{
}

/**
 * @brief Fill the leader's route history with its current position and animation.
 */
void field_reset_leader_position_history(void)
{
    FieldRoutePoint* point;
    s32 i;

    point = g_field_object_states[0].route_history;
    i = 0;
    do
    {
        point->x = g_field_actors[0].x / 256;
        point->z = g_field_actors[0].z / 256;
        g_field_route_animation_history[i] = g_field_actors[0].animation;
        i++;
        point++;
    } while (i < FIELD_ROUTE_HISTORY_LENGTH);
}

/**
 * @brief Rebuild the leader's route history when a follower is not on it.
 *
 * Each follower walks towards the leader's history entry at its route_index.
 * When a follower stands elsewhere (after a warp or a scripted move), the
 * history is resampled along a generated path from the follower to the leader.
 */
void field_refresh_party_routes(void)
{
    s32 first_x;
    s32 second_x;
    s32 first_only_x;
    s32 second_only_x;
    s32 first_z;
    s32 second_z;
    s32 first_only_z;
    s32 second_only_z;

    if ((g_field_actors[1].presence != FIELD_ACTOR_UNUSED) && (g_field_actors[1].control.word & FIELD_CONTROL_MODE_MASK))
    {
        if (g_field_actors[2].presence != FIELD_ACTOR_UNUSED)
        {
            first_x = g_field_actors[1].x / 256;
            if (first_x == g_field_object_states[0].route_history[g_field_object_states[1].route_index].x)
            {
                first_z = g_field_actors[1].z / 256;
                if (first_z == g_field_object_states[0].route_history[g_field_object_states[1].route_index].z)
                {
                    second_x = g_field_actors[2].x / 256;
                    if (second_x == g_field_object_states[0].route_history[g_field_object_states[2].route_index].x)
                    {
                        second_z = g_field_actors[2].z / 256;
                        if (second_z == g_field_object_states[0].route_history[g_field_object_states[2].route_index].z)
                        {
                            return;
                        }
                    }
                }
            }
            field_sample_actor_route(g_field_object_states[0].route_history, FIELD_ROUTE_FOLLOWER_SAMPLES, &g_field_actors[1], &g_field_actors[1], 0,
                                     FIELD_ROUTE_FOLLOWER_SAMPLES);
            field_sample_actor_route(&g_field_object_states[0].route_history[FIELD_ROUTE_FOLLOWER_SAMPLES], FIELD_ROUTE_FOLLOWER_SAMPLES - 1, &g_field_actors[1],
                                     &g_field_actors[0], FIELD_ROUTE_FOLLOWER_SAMPLES, FIELD_ROUTE_FOLLOWER_SAMPLES);
            g_field_object_states[1].route_index = FIELD_ROUTE_FOLLOWER_SAMPLES;
            field_sample_actor_route(g_field_object_states[0].route_history, FIELD_ROUTE_FOLLOWER_SAMPLES, &g_field_actors[2], &g_field_actors[1], 0,
                                     FIELD_ROUTE_FOLLOWER_SAMPLES);
            g_field_object_states[2].route_index = 0;
        }
        else
        {
            first_only_x = g_field_actors[1].x / 256;
            if (first_only_x == g_field_object_states[0].route_history[g_field_object_states[1].route_index].x)
            {
                first_only_z = g_field_actors[1].z / 256;
                if (first_only_z == g_field_object_states[0].route_history[g_field_object_states[1].route_index].z)
                {
                    return;
                }
            }
            field_sample_actor_route(g_field_object_states[0].route_history, FIELD_ROUTE_FOLLOWER_SAMPLES, &g_field_actors[1], &g_field_actors[1], 0,
                                     FIELD_ROUTE_FOLLOWER_SAMPLES);
            field_sample_actor_route(&g_field_object_states[0].route_history[FIELD_ROUTE_FOLLOWER_SAMPLES], FIELD_ROUTE_FOLLOWER_SAMPLES - 1, &g_field_actors[1],
                                     &g_field_actors[0], FIELD_ROUTE_FOLLOWER_SAMPLES, FIELD_ROUTE_FOLLOWER_SAMPLES);
            g_field_object_states[1].route_index = FIELD_ROUTE_FOLLOWER_SAMPLES;
        }
    }
    else if (g_field_actors[2].presence != FIELD_ACTOR_UNUSED)
    {
        second_only_x = g_field_actors[2].x / 256;
        if (second_only_x == g_field_object_states[0].route_history[g_field_object_states[2].route_index].x)
        {
            second_only_z = g_field_actors[2].z / 256;
            if (second_only_z == g_field_object_states[0].route_history[g_field_object_states[2].route_index].z)
            {
                return;
            }
        }
        field_sample_actor_route(g_field_object_states[0].route_history, FIELD_ROUTE_FOLLOWER_SAMPLES, &g_field_actors[2], &g_field_actors[2], 0,
                                 FIELD_ROUTE_FOLLOWER_SAMPLES);
        field_sample_actor_route(&g_field_object_states[0].route_history[FIELD_ROUTE_FOLLOWER_SAMPLES], FIELD_ROUTE_FOLLOWER_SAMPLES - 1, &g_field_actors[2],
                                 &g_field_actors[0], FIELD_ROUTE_FOLLOWER_SAMPLES, FIELD_ROUTE_FOLLOWER_SAMPLES);
        g_field_object_states[2].route_index = FIELD_ROUTE_FOLLOWER_SAMPLES;
    }
}

/**
 * @brief Sample the path from @p actor to @p target into evenly spaced route points.
 * @param points Output points (whole units).
 * @param remaining Number of points to write, including the start position; at least two.
 * @param actor Actor at the start of the path.
 * @param target Actor at the end of the path; also the heading reference.
 * @param heading_offset First entry of g_field_route_animation_history to write.
 * @param unused_limit Unused.
 * @note The pinned assembler inserts one extra nop; see
 *       docs/decompilation/maspsx-division-register-hazard.md.
 */
static void field_sample_actor_route(FieldRoutePoint* points, s32 remaining, FieldActor* actor, FieldActor* target, s32 heading_offset, s32 unused_limit)
{
    struct
    {
        VECTOR position;
        VECTOR delta;
        VECTOR direction;
    } work;
    s32 object_index;
    FieldObjectState* states;
    FieldObjectState* waypoint_state;
    s32 heading;
    s16* z_cursor;
    s16 sample_x;
    s16 sample_z;
    s32 spacing;
    s32 segment_length;
    s32 waypoint_heading_index;
    s32 sample_heading_index;
    s32 point_index;
    s32 total_distance;
    s32 waypoint_offset;
    s32 path_index;
    s32 segment_dx;
    s32 step_x;
    s32 step_z;
    s32 segment_dz;
    s32 waypoint_dx;
    s32 waypoint_dz;
    s32 sample_origin_x;
    s32 sample_origin_z;
    s32 initial_z;
    s32 initial_heading;

    /* Measure the generated path before choosing the sample spacing. */
    field_route_actor_to_object(actor, target->object_index, 0);
    work.position.vx = actor->x;
    total_distance = 0;
    work.position.vz = actor->z;
    path_index = 0;
    object_index = actor->object_index;
    if (g_field_object_states[object_index].path_length != 0)
    {
        do
        {
            segment_dx = g_field_object_states[object_index].path[path_index].x - work.position.vx;
            work.delta.vx = segment_dx / 256;
            segment_dz = g_field_object_states[actor->object_index].path[path_index].z - work.position.vz;
            work.delta.vz = segment_dz / 256;
            work.delta.vy = 0;
            gte_ldlvl(&work.delta);
            gte_sqr0();
            gte_stlvnl(&work.direction);
            segment_length = SquareRoot0(work.direction.vx + work.direction.vz);
            work.position.vx = g_field_object_states[actor->object_index].path[path_index].x;
            work.position.vz = g_field_object_states[actor->object_index].path[path_index].z;
            path_index += 1;
            total_distance += segment_length;
            object_index = actor->object_index;
        } while (path_index < g_field_object_states[object_index].path_length);
    }
    initial_heading = field_get_route_heading_animation(actor, target);
    points->x = actor->x / 256;
    /* The rounding division is written out here and below; "/ 256" schedules differently. */
    initial_z = actor->z;
    if (initial_z < 0)
    {
        initial_z += 255;
    }
    remaining -= 1;
    points->z = initial_z >> 8;
    points++;
    g_field_route_animation_history[heading_offset] = initial_heading;
    point_index = 1;
    /* Short paths begin with repeated start samples so the remaining steps stay useful. */
    while ((total_distance / remaining) < FIELD_ROUTE_MIN_SPACING)
    {
        g_field_route_animation_history[point_index + heading_offset] = initial_heading;
        points->x = actor->x / 256;
        points->z = actor->z / 256;
        points++;
        remaining -= 1;
        point_index += 1;
        if (remaining == 0)
        {
            return;
        }
    }
    work.position.vx = actor->x;
    work.position.vz = actor->z;
    spacing = total_distance / remaining;
    path_index = 0;
    if (g_field_object_states[actor->object_index].path_length != 0)
    {
        states = g_field_object_states;
        do
        {
            waypoint_offset = path_index * sizeof(FieldPathPoint);
            z_cursor = &points->z;
            do
            {
                waypoint_state = FIELD_WAYPOINT_STATE(states, actor->object_index, waypoint_offset);
                waypoint_dx = waypoint_state->path[0].x - work.position.vx;
                work.delta.vx = waypoint_dx / 256;
                waypoint_state = FIELD_WAYPOINT_STATE(states, actor->object_index, waypoint_offset);
                waypoint_dz = waypoint_state->path[0].z - work.position.vz;
                work.delta.vz = waypoint_dz / 256;
                work.delta.vy = 0;
                if (spacing >= SquareRoot0(func_8001CDAC((s32*)&work.delta, (s32*)&work.direction)))
                {
                    waypoint_state = FIELD_WAYPOINT_STATE(states, actor->object_index, waypoint_offset);
                    points->x = waypoint_state->path[0].x / 256;
                    waypoint_state = FIELD_WAYPOINT_STATE(states, actor->object_index, waypoint_offset);
                    *z_cursor = waypoint_state->path[0].z / 256;
                    work.position.vx = points->x << 8;
                    points++;
                    work.position.vz = *z_cursor << 8;
                    heading = field_get_route_heading_animation(actor, target);
                    remaining -= 1;
                    waypoint_heading_index = point_index + heading_offset;
                    point_index += 1;
                    g_field_route_animation_history[waypoint_heading_index] = heading;
                }
                else
                {
                    sample_origin_x = work.position.vx / 256;
                    step_x = work.direction.vx * spacing;
                    sample_x = sample_origin_x + (step_x / (1 << FIELD_DIRECTION_SHIFT));
                    points->x = sample_x;
                    sample_origin_z = work.position.vz;
                    work.position.vx = (s32)(sample_x << 16) >> 8;
                    if (sample_origin_z < 0)
                    {
                        sample_origin_z += 255;
                    }
                    sample_origin_z >>= 8;
                    step_z = work.direction.vz * spacing;
                    sample_z = sample_origin_z + (step_z / (1 << FIELD_DIRECTION_SHIFT));
                    *z_cursor = sample_z;
                    z_cursor += 2;
                    points++;
                    work.position.vz = (s32)(sample_z << 16) >> 8;
                    heading = field_get_route_heading_animation(actor, target);
                    remaining -= 1;
                    sample_heading_index = point_index + heading_offset;
                    point_index += 1;
                    g_field_route_animation_history[sample_heading_index] = heading;
                    continue;
                }
                break;
            } while (remaining != 0);
            path_index += 1;
            if (remaining == 0)
            {
                break;
            }
        } while (path_index < states[actor->object_index].path_length);
    }
}

/**
 * @brief Map the heading from @p source to @p destination onto a walking animation.
 * @param destination Point the heading is measured to.
 * @param source Point the heading is measured from.
 * @return Running animation for the heading's eighth of the circle.
 */
static s32 field_get_route_heading_animation(FieldActor* destination, FieldActor* source)
{
    s32 angle;
    s32 heading;

    angle = ratan2(destination->z - source->z, source->x - destination->x) >> FIELD_HEADING_ANGLE_SHIFT;
    heading = angle + FIELD_HEADING_HALF_SECTOR;
    if (heading < 0)
    {
        heading = angle + FIELD_HEADING_HALF_SECTOR + FIELD_HEADING_STEPS;
    }
    if (heading >= FIELD_HEADING_STEPS)
    {
        heading -= FIELD_HEADING_STEPS;
    }
    return g_field_direction_animation_modes[heading >> FIELD_HEADING_SECTOR_SHIFT] + FIELD_ANIMATION_RUN;
}

/**
 * @brief Push an actor's position onto its route history when it has moved.
 * @param record Actor to record; the leader also records its animation.
 */
void field_record_actor_position(FieldActor* record)
{
    s32 index;
    FieldRoutePoint* history;
    u8* animation_history;

    if ((record->x / 256) == g_field_object_states[record->object_index].route_history[FIELD_ROUTE_HISTORY_LENGTH - 1].x)
    {
        if ((record->z / 256) == g_field_object_states[record->object_index].route_history[FIELD_ROUTE_HISTORY_LENGTH - 1].z)
        {
            return;
        }
    }

    index = 0;
    history = g_field_object_states[record->object_index].route_history;
    /* Each X/Z pair moves as one word. */
    do
    {
        index++;
        *(s32*)history = *(s32*)(history + 1);
        history++;
    } while (index < FIELD_ROUTE_HISTORY_LENGTH - 1);

    history->x = record->x / 256;
    history->z = record->z / 256;

    if (record->object_index == 0)
    {
        animation_history = g_field_route_animation_history;
        index = 0;
        do
        {
            index++;
            animation_history[0] = animation_history[1];
            animation_history++;
        } while (index < FIELD_ROUTE_HISTORY_LENGTH - 1);
        *animation_history = record->animation;
    }
}

/**
 * @brief Walk a follower along the leader's route history.
 *
 * The follower moves towards the leader's history entry at its route_index,
 * advancing or holding that index to keep its distance, then resolves
 * collision and picks a walking or standing animation.
 *
 * @param actor Follower to move.
 * @param follower_index Place of the follower behind the leader (0 or 1).
 */
void field_follow_leader_route(FieldActor* actor, s32 follower_index)
{
    VECTOR square;
    VECTOR delta;
    FieldMapBounds* bounds = FIELD_MAP_BOUNDS;
    struct FieldCollisionMover* mover = FIELD_COLLISION_MOVER;
    FieldActor* scan;
    s16 frame_timer;
    s32 limit;
    s32 position_z;
    s32 animation_index;
    s32 actor_x;
    s32 advance_dx;
    s32 retreat_dx;
    s32 position_x;
    s32 leader_index;
    s32 dz;
    s32 dx;
    s32 leader_dx;
    s32 leader_dz;
    s32 speed_step;
    u8 route_index;
    s32 sample_index;
    u8 recorded_animation;
    u8 old_animation;
    u8 animation;
    FieldObjectState* sample_state;
    FieldActor* leader;
    FieldObjectState* advance_state;
    FieldObjectState* retreat_state;

    /* The leader is the first present actor under player control. */
    for (leader_index = 0; leader_index < FIELD_ACTOR_COUNT; leader_index++)
    {
        scan = &g_field_actors[leader_index];
        if (scan->presence != FIELD_ACTOR_UNUSED && !(scan->control.half[0] & FIELD_CONTROL_MODE_MASK))
        {
            break;
        }
    }
    if (leader_index == FIELD_ACTOR_COUNT)
    {
        leader_index = 0;
    }
    delta.vy = 0;
    delta.vz = 0;
    delta.vx = 0;
    frame_timer = actor->frame_timer;
    leader = &g_field_actors[leader_index];
    if (frame_timer != 0)
    {
        speed_step = actor->speed_accumulator / frame_timer;
    }
    else
    {
        speed_step = 0;
    }
    if (speed_step != 0)
    {
        actor->speed_accumulator -= speed_step;
    }
    if (actor->command != FIELD_ACTOR_COMMAND_NONE)
    {
        field_update_actor_command(actor);
        return;
    }
    actor_x = actor->x;
    {
        FieldObjectState* states = g_field_object_states;
        FieldObjectState* state = &states[actor->object_index];

        /* The leader's history entry at this follower's route index. */
        sample_state = (FieldObjectState*)((s32)g_field_object_states + (leader->object_index * (s32)(sizeof(FieldObjectState) / sizeof(s32)) + state->route_index) * 4);
    }
    if (sample_state->route_history[0].x != actor_x / 256 || sample_state->route_history[0].z != actor->z / 256)
    {
        limit = FIELD_ROUTE_FOLLOWER_SAMPLES;
        if (follower_index != 0)
        {
            limit = 0;
        }
        {
            FieldObjectState* states = g_field_object_states;
            retreat_state = &states[actor->object_index];
        }
        sample_index = retreat_state->route_index;
        dx = 0;
        if (limit < sample_index)
        {
            dz = dx;
            retreat_state->route_index = sample_index - 1;
            delta.vx = 0;
            delta.vz = 0;
        }
        else
        {
            retreat_dx = (g_field_object_states[leader->object_index].route_history[sample_index].x << 8) - actor->x;
            delta.vx = retreat_dx;
            dx = retreat_dx;
            delta.vz = (g_field_object_states[leader->object_index].route_history[g_field_object_states[actor->object_index].route_index].z << 8) - actor->z;
            dz = delta.vz;
            gte_ldlvl(&delta);
            gte_sqr12();
            gte_stlvnl(&square);
            if (((square.vx + square.vz) >= FIELD_ROUTE_RUN_DISTANCE_SQUARED) && (!(g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS)))
            {
                actor->running = 1;
            }
            else
            {
                actor->running = 0;
            }
        }
    }
    else
    {
        leader_dx = leader->x - actor_x;
        delta.vx = leader_dx / 256;
        leader_dz = leader->z - actor->z;
        delta.vz = leader_dz / 256;
        gte_ldlvl(&delta);
        gte_sqr0();
        gte_stlvnl(&square);
        dx = 0;
        if (((square.vx + square.vz) > ((follower_index + 1) * FIELD_ROUTE_FOLLOW_DISTANCE_SQUARED)) &&
            ((advance_state = &g_field_object_states[actor->object_index], route_index = advance_state->route_index) < (FIELD_ROUTE_HISTORY_LENGTH - 1U)))
        {
            advance_state->route_index = route_index + 1;
            advance_dx = (g_field_object_states[leader->object_index].route_history[g_field_object_states[actor->object_index].route_index].x << 8) - actor->x;
            delta.vx = advance_dx;
            dx = advance_dx;
            delta.vz = (g_field_object_states[leader->object_index].route_history[g_field_object_states[actor->object_index].route_index].z << 8) - actor->z;
            dz = delta.vz;
            gte_ldlvl(&delta);
            gte_sqr12();
            gte_stlvnl(&square);
            if (((square.vx + square.vz) >= FIELD_ROUTE_RUN_DISTANCE_SQUARED) && (!(g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS)))
            {
                actor->running = 1;
            }
            else
            {
                actor->running = 0;
            }
        }
        else
        {
            dz = dx;
            delta.vx = 0;
            delta.vz = 0;
        }
    }
    mover->height = actor->y;
    if (g_field_collision_disabled == 0)
    {
        position_x = actor->x;
        if ((position_x >= 0) && (position_x < (bounds->width << 8)) && ((position_z = actor->z) >= 0) && (position_z < ((s32)(bounds->depth << 16) >> 7)))
        {
            mover->x = position_x;
            mover->height = actor->y;
            mover->z = actor->z;
            mover->move_x = dx;
            mover->move_height = 0;
            mover->move_z = dz;
            if (g_field_object_parts[actor->object_index].scale_z == FIELD_PART_FULL_SCALE)
            {
                mover->footprint_width = FIELD_FOOTPRINT_LARGE_WIDTH;
                mover->mode.footprint_depth = FIELD_FOOTPRINT_LARGE_DEPTH;
            }
            else
            {
                mover->footprint_width = FIELD_FOOTPRINT_WIDTH;
                mover->mode.footprint_depth = FIELD_FOOTPRINT_DEPTH;
            }
            mover->height_bias = FIELD_FOOTPRINT_STEP;
            mover->mode.mode_flags &= ~FIELD_MOVER_AIRBORNE_HIGH;
            mover->mode.mode_flags &= ~FIELD_MOVER_AIRBORNE_LOW;
            mover->collision_node = g_field_object_states[actor->object_index].collision_node;
            mover->flags = g_field_object_states[actor->object_index].collision_flags;
            func_8005B6AC(mover);
            g_field_object_states[actor->object_index].collision_node = mover->collision_node;
            g_field_object_states[actor->object_index].collision_flags = mover->flags;
            actor->y = mover->height;
            g_field_object_states[actor->object_index].movement.half.hi = mover->resolved_height / 256;
        }
        else
        {
            g_field_object_states[actor->object_index].collision_node = -1;
            g_field_object_states[actor->object_index].collision_flags = 0;
            g_field_object_states[actor->object_index].movement.half.hi = 0;
        }
    }
    if ((dx | dz) != 0)
    {
        if (g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS)
        {
            recorded_animation = g_field_route_animation_history[g_field_object_states[actor->object_index].route_index];
            animation = g_field_follower_animation_map[recorded_animation & FIELD_ANIMATION_INDEX_MASK];
            animation |= recorded_animation & FIELD_ANIMATION_FACING;
        }
        else
        {
            animation = g_field_route_animation_history[g_field_object_states[actor->object_index].route_index];
        }
        if (actor->animation != animation)
        {
            actor->animation = animation;
            actor->animation_frame = 0;
            actor->animation_active = 1;
            field_restart_actor_animation(actor);
        }
    }
    else
    {
        old_animation = actor->animation;
        animation_index = old_animation & FIELD_ANIMATION_INDEX_MASK;
        if ((animation_index < FIELD_ANIMATION_WALK) || (g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS))
        {
            if ((animation_index != 0) && (g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS))
            {
                /* Writing the restart tail out here moves the shared block. */
                actor->animation = old_animation & FIELD_ANIMATION_FACING;
                goto restart_animation;
            }
        }
        else
        {
            actor->animation = (animation_index % FIELD_ANIMATION_DIRECTIONS) | (old_animation & FIELD_ANIMATION_FACING);
        restart_animation:
            actor->animation_frame = 0;
            actor->animation_active = 1;
            field_restart_actor_animation(actor);
        }
    }
    if ((dx | dz) != 0)
    {
        actor->x = actor->x + dx;
        actor->z = actor->z + dz;
    }
    if ((delta.vx | delta.vz) != 0)
    {
        actor->control.word = (actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK) | FIELD_CONTROL_MOVING;
    }
    else
    {
        actor->control.word = actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK;
    }
}

/**
 * @brief Move the player-controlled actor from the pad, resolve collision and contacts.
 * @param actor Controlled actor.
 * @param pad_index Controller port.
 * @return The two FieldActor::control movement bits (bit 0 = moving), or 0 when input is blocked.
 */
s32 field_update_actor_input(FieldActor* actor, s32 pad_index)
{
    /** @brief Pad direction, requested position and applied displacement. */
    typedef struct
    {
        FieldVector input, motion, change;
        s16 screen_x, screen_y;
    } MovementWork;

    FieldActor* movement_actor;
    MovementWork work;
    ControllerPortState* ports = CONTROLLER_STATE->ports;
    FieldMapBounds* bounds = FIELD_MAP_BOUNDS;
    struct FieldCollisionQuery* probe = FIELD_COLLISION_PROBE;
    struct FieldCollisionMover* mover = FIELD_COLLISION_MOVER;
    s16 command;
    s16 frame_timer;
    s32 input_z;
    s32 input_x;
    s32 motion_x;
    s32 status_or_contact;
    s32 buttons_port;
    s32 stick_port;
    s32 mover_x;
    s32 motion_z;
    s32 actor_x;
    s32 actor_z;
    s32 mover_z;
    s32 record_input;
    s32 text_status;
    s32 has_input;
    s32 speed_step;
    u16 held;
    u8 device_type;
    ControllerSample* pad;
    FieldObjectState* released_state;
    FieldObjectState* blocked_state;

    buttons_port = pad_index;
    if ((u8)ports[buttons_port].published_sample.device_type >= CONTROLLER_DEVICE_CONFIGURING)
    {
        g_field_pad_buttons = 0;
    }
    else
    {
        held = ports[buttons_port].published_sample.held_buttons;
        g_field_pad_buttons = ((held << 8) & 0xFF00) | (held >> 8);
    }
    /* Swap the up/left and right/down face buttons. */
    g_field_pad_buttons = ((u32)(g_field_pad_buttons & PADRdown) >> 1) | ((g_field_pad_buttons & PADRright) * 2) | ((u32)(g_field_pad_buttons & PADRleft) >> 3) |
                          ((g_field_pad_buttons & PADRup) * 8) | (g_field_pad_buttons & ~(PADRup | PADRright | PADRdown | PADRleft));
    command = actor->command;
    record_input = 0;
    if (command != FIELD_ACTOR_COMMAND_RECOVER)
    {
        record_input = command != FIELD_ACTOR_COMMAND_96;
    }
    func_800A2594(pad_index, record_input);
    if (D_80122B20 != 0)
    {
        g_field_pad_buttons = 0;
    }
    if (actor->command != FIELD_ACTOR_COMMAND_NONE)
    {
        field_update_actor_command(actor);
        return 0;
    }
    if (func_800B0850() != 0)
    {
        return 0;
    }
    work.input.vx = 0;
    work.input.vy = 0;
    work.input.vz = 0;
    actor->control.word = actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK;
    if ((D_80122B20 == 0) && (g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS))
    {
        actor->command = field_resolve_action_command(actor, pad_index);
    }
    if (func_800A6490() == 0)
    {
        text_status = field_text_get_status(0);
        status_or_contact = -1;
        if (text_status != status_or_contact)
        {
            return 0;
        }
        if (field_text_get_status(1) != status_or_contact)
        {
            return 0;
        }
    }
    input_z = 0;
    stick_port = pad_index;
    pad = &ports[stick_port].published_sample;
    device_type = ports[stick_port].published_sample.device_type;
    input_x = input_z;
    if (device_type != CONTROLLER_DEVICE_DIGITAL && D_80122B20 == 0)
    {
        if (device_type >= CONTROLLER_DEVICE_CONFIGURING)
        {
            work.input.vx = 0;
            work.input.vz = 0;
        }
        else
        {
            input_x = work.input.vx = pad->left_stick_x;
            input_z = work.input.vz = -pad->left_stick_y;
        }
    }
    has_input = input_x | input_z;
    if (has_input == 0)
    {
        if (g_field_pad_buttons & PADLright)
        {
            actor->control.word = (actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK) | FIELD_CONTROL_MOVING;
            input_x += FIELD_PAD_FULL_DEFLECTION;
            work.input.vx += FIELD_PAD_FULL_DEFLECTION;
        }
        if (g_field_pad_buttons & PADLleft)
        {
            actor->control.word = (actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK) | FIELD_CONTROL_MOVING;
            input_x -= FIELD_PAD_FULL_DEFLECTION;
            work.input.vx -= FIELD_PAD_FULL_DEFLECTION;
        }
        if (g_field_pad_buttons & PADLdown)
        {
            input_z -= FIELD_PAD_FULL_DEFLECTION;
            work.input.vz -= FIELD_PAD_FULL_DEFLECTION;
            actor->control.word = (actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK) | FIELD_CONTROL_MOVING;
        }
        if (g_field_pad_buttons & PADLup)
        {
            input_z += FIELD_PAD_FULL_DEFLECTION;
            work.input.vz += FIELD_PAD_FULL_DEFLECTION;
            actor->control.word = (actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK) | FIELD_CONTROL_MOVING;
        }
    }
    else
    {
        actor->control.word = (actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK) | FIELD_CONTROL_MOVING;
    }
    if (g_field_object_states[actor->object_index].flags & FIELD_OBJECT_FLAG_REVERSED)
    {
        input_x = -input_x;
        input_z = -input_z;
        work.input.vx = -work.input.vx;
        work.input.vz = -work.input.vz;
    }
    func_8001CDAC(&work.input.vx, &work.motion.vx);
    movement_actor = actor;
    frame_timer = movement_actor->frame_timer;
    if (frame_timer != 0)
    {
        speed_step = movement_actor->speed_accumulator / frame_timer;
        movement_actor->speed_accumulator -= speed_step;
    }
    else
    {
        speed_step = 0;
    }
    motion_x = (work.motion.vx * speed_step) >> 4;
    work.motion.vx = motion_x;
    motion_z = (work.motion.vz * speed_step) >> 4;
    work.motion.vz = motion_z;
    if (g_field_collision_disabled == 0)
    {
        if (field_move_leaves_screen(movement_actor, (Vec3i*)&work.motion.vx) == 0)
        {
            mover->x = movement_actor->x;
            mover->height = movement_actor->y;
            mover->z = movement_actor->z;
            actor_x = movement_actor->x;
            if ((actor_x >= 0) && (actor_x < (bounds->width << 8)) && (actor_z = movement_actor->z, (actor_z >= 0)) && (actor_z < ((s32)(bounds->depth << 16) >> 7)))
            {
                mover->move_height = 0;
                mover->height_bias = FIELD_FOOTPRINT_STEP;
                probe->height_tolerance = FIELD_FOOTPRINT_STEP;
                mover->move_x = work.motion.vx;
                mover->move_z = work.motion.vz;
                if (g_field_object_parts[movement_actor->object_index].scale_z == FIELD_PART_FULL_SCALE)
                {
                    mover->footprint_width = FIELD_FOOTPRINT_LARGE_WIDTH;
                    probe->width = FIELD_FOOTPRINT_LARGE_WIDTH;
                    mover->mode.footprint_depth = FIELD_FOOTPRINT_LARGE_DEPTH;
                    probe->depth = FIELD_FOOTPRINT_LARGE_DEPTH;
                }
                else
                {
                    mover->footprint_width = FIELD_FOOTPRINT_WIDTH;
                    probe->width = FIELD_FOOTPRINT_WIDTH;
                    mover->mode.footprint_depth = FIELD_FOOTPRINT_DEPTH;
                    probe->depth = FIELD_FOOTPRINT_DEPTH;
                }

                mover->mode.mode_flags &= ~FIELD_MOVER_AIRBORNE_HIGH;
                mover->mode.mode_flags &= ~FIELD_MOVER_AIRBORNE_LOW;
                mover->collision_node = g_field_object_states[movement_actor->object_index].collision_node;
                mover->flags = g_field_object_states[movement_actor->object_index].collision_flags;
                if ((func_8005B6AC(mover) & FIELD_COLLISION_BLOCKED) == FIELD_COLLISION_BLOCKED)
                {
                    movement_actor->control.word = movement_actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK;
                }
                g_field_object_states[movement_actor->object_index].collision_node = mover->collision_node;
                g_field_object_states[movement_actor->object_index].collision_flags = mover->flags;
                movement_actor->y = mover->height;
                g_field_object_states[movement_actor->object_index].movement.half.hi = mover->resolved_height / 256;
            }
            else
            {
                g_field_object_states[movement_actor->object_index].collision_node = -1;
                g_field_object_states[movement_actor->object_index].collision_flags = 0;
                g_field_object_states[movement_actor->object_index].movement.half.hi = 0;
            }
            mover_x = mover->x;
            mover_z = mover->z;
            work.motion.vx = mover_x;
            work.motion.vz = mover_z;
            work.change.vx = mover_x - movement_actor->x;
            work.change.vy = 0;
            work.change.vz = mover_z - movement_actor->z;
            if (field_move_leaves_screen(movement_actor, (Vec3i*)&work.change.vx) != 0)
            {
                work.motion.vx = movement_actor->x;
                work.motion.vz = movement_actor->z;
                movement_actor->control.word = movement_actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK;
            }
            probe->x = mover->x;
            probe->y = movement_actor->y;
            probe->z = mover->z;
            /* func_8005B368 is called as returning int: the original compares the s16 result unextended. */
            if ((g_field_active_group != 0) && (((s32(*)(struct FieldCollisionQuery*))func_8005B368)(probe) != FIELD_MARKER_NONE))
            {
                work.motion.vx = movement_actor->x;
                work.motion.vz = movement_actor->z;
                movement_actor->control.word &= ~FIELD_CONTROL_MOVEMENT_MASK;
            }
        }
        else
        {
            work.motion.vx = movement_actor->x;
            work.motion.vz = movement_actor->z;
            movement_actor->control.word = movement_actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK;
        }
    }
    else
    {
        work.motion.vx = motion_x + movement_actor->x;
        work.motion.vz = motion_z + movement_actor->z;
    }
    work.motion.vy = movement_actor->y;
    if (movement_actor->command == FIELD_ACTOR_COMMAND_NONE)
    {
        field_update_actor_movement_animation(movement_actor, input_x, input_z);
    }
    else
    {
        field_prepare_actor_action(movement_actor);
    }
    work.screen_x = g_field_view_offset_x / 256 + (s16)(work.motion.vx / 256 + FIELD_SCREEN_CENTRE_X);
    work.screen_y = g_field_view_offset_y / 256 + (s16)(work.motion.vy / 256 + FIELD_SCREEN_CENTRE_Y) - work.motion.vz / 512 - g_field_view_offset_z / 512;
    if (movement_actor->command == FIELD_ACTOR_COMMAND_NONE)
    {
        if ((g_field_object_states[movement_actor->object_index].movement.word >> 14) & 1)
        {
            movement_actor->x = work.motion.vx;
            movement_actor->z = work.motion.vz;
            if (field_find_actor_overlap(movement_actor, movement_actor, 0) == 0)
            {
                released_state = &g_field_object_states[movement_actor->object_index];
                released_state->movement.word = released_state->movement.word & ~FIELD_MOVEMENT_OVERLAPPING;
            }
        }
        else
        {
            status_or_contact = field_find_actor_overlap(movement_actor, &work.motion.vx, 0) & FIELD_OVERLAP_INDEX_MASK;
            if (status_or_contact < FIELD_PARTY_COUNT)
            {
                movement_actor->x = work.motion.vx;
                movement_actor->z = work.motion.vz;
            }
            else
            {
                if (field_find_actor_overlap(movement_actor, movement_actor, 0) != 0)
                {
                    blocked_state = &g_field_object_states[movement_actor->object_index];
                    blocked_state->movement.word = blocked_state->movement.word | FIELD_MOVEMENT_OVERLAPPING;
                }
                movement_actor->control.word = movement_actor->control.word & ~FIELD_CONTROL_MOVEMENT_MASK;
                field_start_actor_contact_interaction(movement_actor, status_or_contact);
            }
        }
    }
    field_update_actor_run_button(movement_actor, pad_index);
    if (movement_actor->command != FIELD_ACTOR_COMMAND_NONE)
    {
        field_update_actor_command(movement_actor);
    }
    return (movement_actor->control.word >> FIELD_CONTROL_MOVEMENT_SHIFT) & 3;
}

/**
 * @brief Validate the action requested by a FIELD_ACTOR_COMMAND_ACTION command and start it.
 *
 * The command's high byte selects the action in the actor's resource action
 * table. A refused action clears the command; an accepted one updates the
 * object's contact bits, starts the instrument, technique or animation, and
 * runs the action handler.
 *
 * @param actor Actor whose pending command is checked.
 */
void field_prepare_actor_action(FieldActor* actor)
{
    s32 animation_slot;
    s32 row_offset;
    s32 technique_base;
    s32 technique;
    u16 request;
    u16 animation;
    u8 object_index;
    u8 action_slot;
    FieldObjectState* state;
    FieldResourceAction* action;

    if ((u8)actor->command != FIELD_ACTOR_COMMAND_ACTION)
    {
        return;
    }
    g_field_object_states[actor->object_index].contact.word = g_field_object_states[actor->object_index].contact.word & ~FIELD_CONTACT_ACTION_BITS;
    if (!(g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS))
    {
        actor->command = FIELD_ACTOR_COMMAND_NONE;
        return;
    }
    g_field_object_states[actor->object_index].action = (u16)actor->command >> 8;
    row_offset = actor->resource_index * sizeof(g_field_resource_actions[0]);
    actor->command = (u8)actor->command;
    state = &g_field_object_states[actor->object_index];
    action_slot = state->action;
    {
        s32 action_address = (s32)&g_field_resource_actions[0][action_slot];
        action = (FieldResourceAction*)(row_offset + action_address);
    }
    if (action_slot == FIELD_RESOURCE_ACTION_REDIRECT)
    {
        state->action = (u8)action->command;
    }
    if (!(action->flags & FIELD_ACTION_INSTRUMENT) && (action->command == 0) && (action->animation == 0))
    {
        func_800A3938(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
        actor->command = FIELD_ACTOR_COMMAND_NONE;
        return;
    }
    if ((action->flags & FIELD_ACTION_INSTRUMENT) &&
        ((field_object_has_active_actor_tracks(actor->object_index) != 0) || (field_count_free_actor_slots(actor->object_index) < FIELD_ACTION_MIN_FREE_SLOTS) ||
         (actor->variant != 0)))
    {
        actor->command = FIELD_ACTOR_COMMAND_NONE;
        return;
    }
    g_field_object_states[actor->object_index].contact.word = g_field_object_states[actor->object_index].contact.word & ~FIELD_CONTACT_UNK02;
    if ((action->command & FIELD_ACTION_TECHNIQUE) && !(action->flags & FIELD_ACTION_INSTRUMENT))
    {
        if ((field_object_has_active_actor_tracks(actor->object_index) != 0) || (D_8010AE58 != 0) ||
            (field_count_free_actor_slots(actor->object_index) < FIELD_ACTION_MIN_FREE_SLOTS))
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            return;
        }
        object_index = actor->object_index;
        if (g_field_object_states[object_index].technique_gauge != FIELD_TECHNIQUE_GAUGE_FULL)
        {
            func_800A3938(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            return;
        }
        technique = action->command & FIELD_ACTION_TECHNIQUE_MASK;
        technique_base = (g_field_player_records[object_index].weapon_type * FIELD_TECHNIQUES_PER_WEAPON) + FIELD_TECHNIQUE_SEQUENCE_BASE;
        if (field_start_streamed_animation(object_index, technique + technique_base) == 0)
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            return;
        }
        g_field_object_states[actor->object_index].movement.word = g_field_object_states[actor->object_index].movement.word | FIELD_MOVEMENT_TECHNIQUE;
    }
    else
    {
        request = action->request;
        if ((request & FIELD_REQUEST_BOUND) && (field_start_streamed_animation(actor->object_index, request & FIELD_REQUEST_ANIMATION_MASK) == 0))
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            return;
        }
    }
    if (action->flags & FIELD_ACTION_INSTRUMENT)
    {
        g_field_object_states[actor->object_index].hud.word = g_field_object_states[actor->object_index].hud.word & ~1;
        g_field_object_states[actor->object_index].contact.word = g_field_object_states[actor->object_index].contact.word | FIELD_CONTACT_ACTION_PENDING;
        actor->animation_state = 1;
        actor->animation_active = 1;
        actor->animation_frame = 0;
        actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_INSTRUMENT_READY;
        g_field_object_states[actor->object_index].movement.word = g_field_object_states[actor->object_index].movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK;
        field_restart_actor_animation(actor);
        if (action->animation != 0)
        {
            g_field_object_states[actor->object_index].action_parameter = action->animation;
        }
        g_field_object_states[actor->object_index].movement.word = g_field_object_states[actor->object_index].movement.word & ~FIELD_MOVEMENT_CHARGED;
    }
    else if (!(action->command & FIELD_ACTION_TECHNIQUE))
    {
        animation = action->animation;
        if ((animation != FIELD_ACTION_NO_ANIMATION) && (animation != 0))
        {
            animation_slot = field_find_free_actor_slot(actor->object_index, 0);
            if ((animation_slot != -1) && (field_start_builtin_animation(actor->object_index, animation_slot, action->animation) != 0))
            {
                field_start_actor_animation(animation_slot, 0, 0);
                g_field_object_states[actor->object_index].contact.bytes.animation_actor_index = animation_slot;
            }
        }
    }
    field_start_object_ground_effect((struct FieldMotionRecord*)actor, (u8)action->flags);
}

/**
 * @brief Pick the walking animation for a pad displacement, or settle into idle.
 * @param record Actor whose facing and animation are updated.
 * @param delta_x Horizontal displacement.
 * @param delta_z Depth displacement.
 */
void field_update_actor_movement_animation(FieldActor* record, s32 delta_x, s32 delta_z)
{
    s32* vertical_entry;
    s32* direction_entry;
    s32 sector_index;
    s32 heading_or_animation;
    s32 heading;
    s32 old_animation;
    s32 direction_table_address;
    s32 sector_or_running;
    s32 direction_offset;
    u8 resource_index;
    u8 current_animation;
    s32 previous_animation;
    s32 animation_index;
    s32 idle_animation;
    s32 facing_mask;
    FieldActor* refresh_record;
    u8 idle_delay;
    u8 walk_delay;

    resource_index = record->resource_index;
    if (g_field_resource_entries[resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS)
    {
        if ((delta_x | delta_z) != 0)
        {
            heading_or_animation = ratan2(-delta_z, delta_x);
            heading_or_animation >>= FIELD_HEADING_ANGLE_SHIFT;
            heading_or_animation += FIELD_HEADING_HALF_SECTOR;
            if (heading_or_animation < 0)
            {
                heading_or_animation += FIELD_HEADING_STEPS;
            }
            if (heading_or_animation >= FIELD_HEADING_STEPS)
            {
                heading_or_animation -= FIELD_HEADING_STEPS;
            }
            sector_or_running = heading_or_animation >> FIELD_HEADING_SECTOR_SHIFT;
            if ((sector_or_running == FIELD_SECTOR_UP) || (sector_or_running == FIELD_SECTOR_DOWN))
            {
                s32* vertical_base;

                facing_mask = ~FIELD_ANIMATION_FACING;
                vertical_base = g_field_actor_walk_animations;
                vertical_entry = vertical_base + sector_or_running;
                if ((record->animation & facing_mask) != (*vertical_entry & facing_mask))
                {
                    record->direction = heading_or_animation & FIELD_HEADING_SECTOR_MASK;
                    heading_or_animation = (u8)*vertical_entry;
                    old_animation = record->animation;
                    record->animation_frame = 0;
                    record->animation_active = 1;
                    heading_or_animation |= old_animation & FIELD_ANIMATION_FACING;
                    record->animation = heading_or_animation;
                    field_restart_actor_animation(record);
                }
            }
            else
            {
                sector_index = sector_or_running;
                current_animation = record->animation;
                if (((current_animation != g_field_actor_walk_animations[sector_index]) && (delta_z == 0)) ||
                    ((current_animation != g_field_actor_diagonal_walk_animations[sector_index]) && (delta_z != 0)))
                {
                    record->direction = heading_or_animation & FIELD_HEADING_SECTOR_MASK;
                    if (delta_z != 0)
                    {
                        record->animation = g_field_actor_diagonal_walk_animations[heading_or_animation >> FIELD_HEADING_SECTOR_SHIFT];
                    }
                    else
                    {
                        record->animation = g_field_actor_walk_animations[heading_or_animation >> FIELD_HEADING_SECTOR_SHIFT];
                    }
                    record->animation_frame = 0;
                    record->animation_active = 1;
                    field_restart_actor_animation(record);
                }
            }
            record->stop_delay = FIELD_STOP_DELAY;
            record->animation_active = 1;
            return;
        }
        idle_delay = record->stop_delay;
        if (idle_delay != 0)
        {
            record->stop_delay = idle_delay - 1;
        }
        idle_animation = record->animation;
        idle_animation &= FIELD_ANIMATION_INDEX_MASK;
        if (((idle_animation >= FIELD_ANIMATION_IDLE_COUNT) && (record->stop_delay == 0)) || ((idle_animation < FIELD_ANIMATION_IDLE_COUNT) && (record->animation_state == 0)))
        {
            s32 random_value;

            /* delta_z is reused for the new idle animation. */
            delta_z = record->animation;
            delta_z &= FIELD_ANIMATION_FACING;
            random_value = rand();
            refresh_record = record;
            delta_z += random_value > FIELD_IDLE_ALTERNATE_THRESHOLD;
            refresh_record->animation = delta_z;
            refresh_record->animation_frame = 0;
            refresh_record->animation_state = 1;
            refresh_record->animation_active = 1;
            field_restart_actor_animation(refresh_record);
        }
    }
    else
    {
        if ((delta_x | delta_z) != 0)
        {
            heading = ratan2(-delta_z, delta_x);
            heading >>= FIELD_HEADING_ANGLE_SHIFT;
            heading += FIELD_HEADING_HALF_SECTOR;
            if (heading < 0)
            {
                heading += FIELD_HEADING_STEPS;
            }
            if (heading >= FIELD_HEADING_STEPS)
            {
                heading -= FIELD_HEADING_STEPS;
            }
            direction_offset = heading >> FIELD_HEADING_SECTOR_SHIFT;
            direction_table_address = (s32)g_field_direction_animation_modes;
            direction_offset *= 4;
            direction_entry = (s32*)(direction_table_address + direction_offset);
            if (record->animation != (*direction_entry + ((record->running & 1) * FIELD_ANIMATION_DIRECTIONS) + FIELD_ANIMATION_WALK))
            {
                record->direction = heading & FIELD_HEADING_SECTOR_MASK;
                heading_or_animation = (u8)*direction_entry;
                sector_or_running = record->running;
                record->animation_frame = 0;
                record->animation_active = 1;
                heading_or_animation += (sector_or_running & 1) * FIELD_ANIMATION_DIRECTIONS;
                heading_or_animation += FIELD_ANIMATION_WALK;
                record->animation = heading_or_animation;
                field_restart_actor_animation(record);
            }
            record->stop_delay = FIELD_STOP_DELAY;
            record->animation_active = 1;
            return;
        }
        walk_delay = record->stop_delay;
        if (walk_delay != 0)
        {
            record->stop_delay = walk_delay - 1;
        }
        previous_animation = record->animation;
        animation_index = previous_animation;
        animation_index &= FIELD_ANIMATION_INDEX_MASK;
        if ((animation_index >= FIELD_ANIMATION_WALK) && (record->stop_delay == 0))
        {
            refresh_record = record;
            refresh_record->animation_active = 1;
            refresh_record->animation_frame = 0;
            refresh_record->animation = (animation_index % FIELD_ANIMATION_DIRECTIONS) | (previous_animation & FIELD_ANIMATION_FACING);
            field_restart_actor_animation(refresh_record);
        }
    }
}

/**
 * @brief Run one frame of an actor's current command (FIELD_ACTOR_COMMAND_*).
 * @param actor Actor whose command is advanced.
 * @return Nothing useful; the value is undefined.
 */
s32 field_update_actor_command(FieldActor* actor)
{
    /** @brief Collected targets, the synthesized combo action and GTE vectors. */
    typedef struct
    {
        s32 targets[12];
        FieldResourceAction combo_action;
        VECTOR displacement;
        VECTOR squared;
    } FieldCommandWork;

    FieldCommandWork scratch;
    s32* target_cursor;
    s32 parameter_offset;
    s32 waypoint_x;
    s32 target_delta_x;
    s32 approach_delta_x;
    s32 sequence_dx;
    s32 jump_dx;
    s32 walk_dx;
    s32 run_dx;
    s32 screen_route_dx;
    s32 approach_dx;
    s32 timed_dx;
    s32 movement_dx;
    s32 target_count_or_slot;
    s32 animation_target_count;
    s32 target_index;
    s32 combo_or_slot;
    s32 eligible_count;
    s32 route_distance_z;
    s32 screen_route_delta_x;
    s32 screen_route_delta_z;
    s32 target_distance_z;
    s32 run_distance_z;
    s32 approach_distance_z;
    s32 route_delta_x;
    s32 route_delta_z;
    s32 heading;
    s32 route_distance_x;
    s32 target_distance_x;
    s32 run_distance_x;
    s32 approach_distance_x;
    SceneState* scene;
    FieldResourceAction* action;
    s32 next_waypoint;
    s32 next_run_waypoint;
    s32 next_screen_waypoint;
    u16 charge_animation;
    u32 sequence_flags;
    u8 action_object_index;
    u8 charge_object_index;
    s32 technique_object_index;
    s32 combo_object_index;
    u8 revive_object_index;
    u8 animation;
    u8 release_object_index;
    s32 combo_target_index;
    u8 turn_animation;
    u8 command_timer;
    FieldActor* followed_actors;
    FieldActor* followed_actor;
    FieldActorSlot* actor_slots;
    FieldObjectState* finish_states;
    FieldObjectState* cancel_states;
    FieldObjectState* instrument_states;
    FieldObjectState* charge_state;
    FieldResourceAction(*action_rows)[FIELD_RESOURCE_ACTION_COUNT];
    FieldObjectState* action_state;
    FieldObjectState* state;
    FieldObjectState* pending_states;
    FieldObjectState* charge_states;
    FieldPlayerRecord* combo_players;
    FieldPlayerRecord* timer_players;
    FieldPlayerRecord* revive_players;
    FieldPlayerRecord* reset_players;
    FieldPlayerRecord* combo_player;
    FieldPlayerRecord* timer_player;
    FieldPlayerRecord* revive_player;
    FieldPlayerRecord* completed_player;
    FieldPlayerRecord* destination_player;
    FieldObjectState* finished_state;
    FieldObjectState* pending_state;
    FieldObjectState* instrument_state;
    FieldObjectState* released_state;
    FieldObjectState* cancelled_state;
    FieldActorSlot* animation_slot;
    FieldObjectState* target_state;
    FieldObjectState* target_list_state;
    FieldObjectState* target_count_state;
    FieldObjectState* instrument_start_state;
    FieldResourceAction* combo_actions;
    FieldObjectState* checked_state;

    scene = SCENE_STATE;
    state = &g_field_object_states[actor->object_index];
    switch (actor->command)
    {
    case FIELD_ACTOR_COMMAND_WAIT_ANIMATION:
        if (actor->animation_state == 0)
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
        }
        scratch.displacement.vx = 0;
        scratch.displacement.vy = 0;
        scratch.displacement.vz = 0;
        field_resolve_actor_movement(actor, &scratch.displacement.vx, 0);
        return;
    case FIELD_ACTOR_COMMAND_WAIT:
        if (actor->animation_state == 0)
        {
            if ((actor->command_param == 0) || (--actor->command_param == 0))
            {
                actor->command = FIELD_ACTOR_COMMAND_NONE;
            }
        }
        return;
    case FIELD_ACTOR_COMMAND_STOP:

        actor->command = FIELD_ACTOR_COMMAND_NONE;
        return;
    case FIELD_ACTOR_COMMAND_RETIRE:
        if ((actor->removal_delay != 0) && (--actor->removal_delay != 0))
        {
            return;
        }
        if (field_object_has_active_actor_tracks(actor->object_index) != 0)
        {
            return;
        }
        actor->presence = FIELD_ACTOR_UNUSED;
        return;
    case FIELD_ACTOR_COMMAND_ATTACHED:
        if ((((actor->removal_delay == 0) || (--actor->removal_delay == 0)) && (field_object_has_active_actor_tracks(actor->object_index) == 0)) ||
            (followed_actors = g_field_actors, followed_actor = &followed_actors[actor->command_param], followed_actor->presence == FIELD_ACTOR_UNUSED))
        {
            field_stop_actor_animations_for_object(actor, 0);
            actor->presence = FIELD_ACTOR_UNUSED;
            return;
        }
        actor->x = followed_actor->x;
        actor->y = followed_actors[actor->command_param].y;
        actor->z = followed_actors[actor->command_param].z;
        return;
    case FIELD_ACTOR_COMMAND_RECOVER:
        if (actor->object_index < FIELD_PLAYER_COUNT)
        {
            combo_players = g_field_player_records;
            combo_player = &combo_players[actor->object_index];
            if ((combo_player->character_kind == FIELD_PLAYER_KIND_HERO) && (combo_player->combo_timer != 0))
            {
                if ((field_get_held_action_buttons(actor->object_index, 0, actor) != 0) && (field_get_held_action_buttons(actor->object_index, 1, actor) != 0))
                {
                    action_rows = g_field_resource_actions;
                    combo_actions = action_rows[actor->resource_index];
                    combo_or_slot = func_800AD7DC(combo_actions[0].command, combo_actions[1].command);
                    parameter_offset = combo_or_slot * 2;
                    if (combo_or_slot != FIELD_COMBO_NONE)
                    {
                        scratch.combo_action.command = combo_or_slot;
                        scratch.combo_action.animation = g_field_action_animation_parameters[parameter_offset];
                        scratch.combo_action.request = g_field_action_animation_parameters[parameter_offset + 1];
                        switch (combo_or_slot)
                        {
                        case 0x34:
                        case 0x3E:
                        case 0x45:
                        case 0x4E:
                        case 0x4F:
                        case 0x50:
                        case 0x51:
                            g_field_object_states[actor->object_index].action = combo_or_slot;
                            break;
                        }
                        if (scratch.combo_action.animation != FIELD_ACTION_NO_ANIMATION)
                        {
                            if (scratch.combo_action.animation != 0)
                            {
                                combo_or_slot = field_find_free_actor_slot(actor->object_index, 0);
                                if (combo_or_slot != -1)
                                {
                                    if (field_start_builtin_animation(actor->object_index, combo_or_slot, scratch.combo_action.animation) != 0)
                                    {
                                        field_start_actor_animation(combo_or_slot, 0, 0);
                                        g_field_object_states[actor->object_index].contact.bytes.animation_actor_index = combo_or_slot;
                                    }
                                }
                            }
                        }
                        field_apply_action_animation(actor, state, &scratch.combo_action);
                        g_field_player_records[actor->object_index].combo_timer = 0;
                        combo_or_slot = 0;
                        field_command_history_clear(actor->object_index);
                        field_release_object_link(actor);
                        combo_target_index = actor->object_index;
                        if (g_field_object_states[combo_target_index].contact.bytes.target_count != 0)
                        {
                            do
                            {
                                combo_target_index = g_field_object_states[combo_target_index].targets[combo_or_slot];
                                g_field_object_states[combo_target_index].contact.word &= ~FIELD_CONTACT_TARGETED;
                                combo_target_index = actor->object_index;
                                combo_or_slot += 1;
                            } while (combo_or_slot < g_field_object_states[combo_target_index].contact.bytes.target_count);
                        }
                        g_field_object_states[actor->object_index].contact.bytes.target_count = 0;
                        return;
                    }
                }
                timer_players = g_field_player_records;
                timer_player = &timer_players[actor->object_index];
                timer_player->combo_timer--;
            }
        }
        field_update_actor_action(actor, 1);
        return;
    case FIELD_ACTOR_COMMAND_96:
        field_update_actor_action(actor, 0);
        return;
    case FIELD_ACTOR_COMMAND_98:
    case FIELD_ACTOR_COMMAND_9B:
        field_update_actor_action(actor, 1);
        return;
    case FIELD_ACTOR_COMMAND_INSTRUMENT:
        field_update_instrument_command(actor);
        return;
    case FIELD_ACTOR_COMMAND_TECHNIQUE:
        field_update_technique_command(actor, state->sequence);
        return;
    case FIELD_ACTOR_COMMAND_TURN:
        if (!(g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS))
        {
            /* The target reads command_param twice back to back; plain reads fold into one load. */
            turn_animation = g_field_actor_turn_animations[((volatile FieldActor*)actor)->command_param];
            actor->command_param = (u8)(((volatile FieldActor*)actor)->command_param + 1);
            actor->animation = turn_animation;
            if (g_field_actor_turn_animations[actor->command_param] == FIELD_TURN_ANIMATION_END)
            {
                actor->command = FIELD_ACTOR_COMMAND_NONE;
                actor->command_param = 0;
            }
        }
        else
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            actor->command_param = 0;
            actor->animation ^= FIELD_ANIMATION_FACING;
        }
        actor->animation_state = 1;
        actor->animation_frame = 0;
        actor->animation_active = 1;
        field_restart_actor_animation(actor);
        return;
    case FIELD_ACTOR_COMMAND_IDLE_AFTER_RELOAD:
        field_idle_actor_after_reload(actor);
        return;
    case FIELD_ACTOR_COMMAND_IDLE_AFTER_ANIMATION:
        field_idle_actor_after_animation(actor);
        return;
    case FIELD_ACTOR_COMMAND_HIT:
        field_update_hit_reaction(actor);
        return;
    case FIELD_ACTOR_COMMAND_KNOCKED_DOWN:
    {
        u8 revive_index;
        FieldPlayerRecord* timed_player;
        s16 revive_delay;
        revive_players = g_field_player_records;
        revive_index = actor->object_index;
        timed_player = &revive_players[revive_index];
        revive_delay = timed_player->revive_delay;
        if (revive_delay != 0)
        {
            if (revive_index < FIELD_PARTY_COUNT)
            {
                if ((timed_player->revive_time >= revive_delay) ||
                    (timed_player->revive_time++, revive_player = &g_field_player_records[actor->object_index],
                     (revive_player->revive_time >= revive_player->revive_delay)))
                {
                    completed_player = &g_field_player_records[actor->object_index];
                    completed_player->revive_delay = 0;
                    completed_player->revive_time = 0;
                    revive_object_index = actor->object_index;
                    destination_player = &g_field_player_records[revive_object_index];
                    field_revive_actor(revive_object_index, destination_player->revive_animation, destination_player->revive_effect, destination_player->revive_sound);
                    return;
                }
            }
        }
        break;
    }
    case FIELD_ACTOR_COMMAND_DEFEATED:
        field_update_defeated(actor);
        return;
    case FIELD_ACTOR_COMMAND_DEFEAT_BOUND:
        if ((g_field_actor_bindings[FIELD_OBJECT_BINDING(actor->object_index)].state != 0) &&
            (g_field_actor_bindings[FIELD_OBJECT_BINDING(actor->object_index)].owner == actor->object_index))
        {
            field_start_defeat_bound_animation(actor);
            return;
        }
        field_start_streamed_animation(actor->object_index, g_field_resource_entries[actor->resource_index].unkE & FIELD_REQUEST_ANIMATION_MASK);
        return;
    case FIELD_ACTOR_COMMAND_DEFEAT_WAIT:
        if ((g_field_actor_bindings[FIELD_OBJECT_BINDING(actor->object_index)].state != 0) &&
            (g_field_actor_bindings[FIELD_OBJECT_BINDING(actor->object_index)].owner == actor->object_index))
        {
            field_start_defeat_wait_animation(actor);
            return;
        }

        field_start_streamed_animation(actor->object_index, g_field_resource_entries[actor->resource_index].unkE & FIELD_REQUEST_ANIMATION_MASK);
        return;
    case FIELD_ACTOR_COMMAND_DEFEAT_END:
        field_update_defeat_end(actor);
        return;
    case FIELD_ACTOR_COMMAND_DEFEAT_DELAY:
        command_timer = actor->command_param;
        if (command_timer != 0)
        {
            actor->command_param = command_timer - 1;
            return;
        }
        if (!(state->contact.word & FIELD_CONTACT_ANIMATION_HIDDEN))
        {
            field_collapse_defeated_actor(actor);
            return;
        }
        break;
    case FIELD_ACTOR_COMMAND_STEP:
        field_move_actor_step(actor, 0, 0, 0);
        return;
    case FIELD_ACTOR_COMMAND_SEQUENCE_STEP:
        sequence_dx = rcos(actor->direction * 16) >> 4;
        field_apply_sequence_displacement(actor, sequence_dx, 0, -rsin(actor->direction * 16) >> 4);
        return;
    case FIELD_ACTOR_COMMAND_JUMP:
        jump_dx = rcos(actor->direction * 16) >> 4;
        field_update_actor_jump(actor, jump_dx, 0, -rsin(actor->direction * 16) >> 4);
        return;
    case FIELD_ACTOR_COMMAND_FOLLOW_LEADER:
        field_follow_leader(actor);
        return;
    case FIELD_ACTOR_COMMAND_WALK_PATH:
        route_delta_x = state->path[state->path_index].x - actor->x;
        scratch.displacement.vx = route_delta_x / 256;
        route_delta_z = state->path[state->path_index].z - actor->z;
        scratch.displacement.vy = route_delta_z / 256;
        gte_ldlvl(&scratch.displacement);
        gte_sqr0();
        gte_stlvnl(&scratch.squared);
        if ((scratch.squared.vx + scratch.squared.vy) < FIELD_PATH_WALK_REACHED)
        {
            actor->x = state->path[state->path_index].x;
            actor->z = state->path[state->path_index].z;
            next_waypoint = state->path_index + 1;
            if (state->path_length == next_waypoint)
            {
                actor->command = FIELD_ACTOR_COMMAND_NONE;
            }
            else
            {
                state->path_index = next_waypoint;
            }
            return;
        }
        heading = ratan2(-state->path[state->path_index].z + actor->z, state->path[state->path_index].x - actor->x) >> FIELD_HEADING_ANGLE_SHIFT;
        actor->direction = heading;
        movement_dx = rcos(actor->direction * 16) >> 4;
        field_move_actor_step(actor, movement_dx, 0, -rsin(actor->direction * 16) >> 4);
        return;
    case FIELD_ACTOR_COMMAND_WALK:
        walk_dx = rcos(actor->direction * 16) >> 4;
        field_move_actor_step(actor, walk_dx, 0, -rsin(actor->direction * 16) >> 4);
        return;
    case FIELD_ACTOR_COMMAND_RUN_PATH:
        route_distance_z = state->path[state->path_index].z - actor->z;
        waypoint_x = state->path[state->path_index].x;
        route_distance_z = abs(route_distance_z);
        route_distance_x = waypoint_x - actor->x;
        route_distance_x = abs(route_distance_x);
        if ((route_distance_z + route_distance_x) < FIELD_PATH_RUN_REACHED)
        {
            actor->x = waypoint_x;
            actor->z = state->path[state->path_index].z;
            next_run_waypoint = state->path_index + 1;
            if (state->path_length != next_run_waypoint)
            {
                state->path_index = next_run_waypoint;
                return;
            }
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            actor->running = 0;
            return;
        }
        actor->running = 1;
        actor->direction = ratan2(-state->path[state->path_index].z + actor->z, state->path[state->path_index].x - actor->x) >> FIELD_HEADING_ANGLE_SHIFT;
        movement_dx = rcos(actor->direction * 16) >> 4;
        field_move_actor_step(actor, movement_dx, 0, -rsin(actor->direction * 16) >> 4);
        if (actor->command == FIELD_ACTOR_COMMAND_NONE)
        {
            actor->running = 0;
            return;
        }
        break;
    case FIELD_ACTOR_COMMAND_RUN:
        run_dx = rcos(actor->direction * 16) >> 4;
        field_move_actor_step(actor, run_dx, 0, -rsin(actor->direction * 16) >> 4);
        if (actor->command == FIELD_ACTOR_COMMAND_NONE)
        {
            actor->running = 0;
            return;
        }
        break;
    case FIELD_ACTOR_COMMAND_LEAVE_PATH:
        screen_route_delta_x = state->path[state->path_index].x - actor->x;
        scratch.displacement.vx = screen_route_delta_x / 256;
        screen_route_delta_z = state->path[state->path_index].z - actor->z;
        scratch.displacement.vy = screen_route_delta_z / 256;
        gte_ldlvl(&scratch.displacement);
        gte_sqr0();
        gte_stlvnl(&scratch.squared);
        if ((scratch.squared.vx + scratch.squared.vy) < FIELD_PATH_WALK_REACHED)
        {
            actor->x = state->path[state->path_index].x;
            actor->z = state->path[state->path_index].z;
            next_screen_waypoint = state->path_index + 1;
            if (state->path_length == next_screen_waypoint)
            {
                actor->command = FIELD_ACTOR_COMMAND_NONE;
            }
            else
            {
                state->path_index = next_screen_waypoint;
            }
        }
        else
        {
            actor->direction = ratan2(-state->path[state->path_index].z + actor->z, state->path[state->path_index].x - actor->x) >> FIELD_HEADING_ANGLE_SHIFT;
            screen_route_dx = rcos(actor->direction * 16) >> 4;
            field_move_actor_step(actor, screen_route_dx, 0, -rsin(actor->direction * 16) >> 4);
        }
        {
            s32 negative_view_offset;
            s32 position;
            s32 view_offset;
            view_offset = scene->camera_x;
            position = actor->x;
            negative_view_offset = -view_offset;
            if ((negative_view_offset + FIELD_SCREEN_MARGIN_LEFT) < position)
            {
                if (position < (negative_view_offset + FIELD_SCREEN_MARGIN_RIGHT))
                {
                    view_offset = scene->camera_z;
                    position = actor->z;
                    negative_view_offset = -view_offset;
                    if ((negative_view_offset + FIELD_SCREEN_MARGIN_LEFT) < position)
                    {
                        if (position < (negative_view_offset + FIELD_SCREEN_MARGIN_BOTTOM))
                        {
                            actor->command = FIELD_ACTOR_COMMAND_NONE;
                            return;
                        }
                    }
                }
            }
        }
        break;
    case FIELD_ACTOR_COMMAND_WALK_TO_TARGET:
        target_distance_z = state->target_z - actor->z;
        target_distance_z = abs(target_distance_z);
        target_delta_x = state->target_x - actor->x;
        target_distance_x = abs(target_delta_x);
        if ((target_distance_z + target_distance_x) >= FIELD_TARGET_WALK_REACHED)
        {
            heading = ratan2(actor->z - state->target_z, target_delta_x) >> FIELD_HEADING_ANGLE_SHIFT;
            actor->direction = heading;
            movement_dx = rcos(actor->direction * 16) >> 4;
            field_move_actor_step(actor, movement_dx, 0, -rsin(actor->direction * 16) >> 4);
            return;
        }
        actor->command = FIELD_ACTOR_COMMAND_NONE;
        return;
    case FIELD_ACTOR_COMMAND_RUN_TO_TARGET:
        run_distance_z = state->target_z - actor->z;
        run_distance_z = abs(run_distance_z);
        run_distance_x = state->target_x - actor->x;
        run_distance_x = abs(run_distance_x);
        if ((run_distance_z + run_distance_x) < FIELD_TARGET_WALK_REACHED)
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            actor->running = 0;
            return;
        }
        actor->running = 1;
        {
            s32 target_z = state->target_z;
            s32 current_z = actor->z;
            s32 target_x = state->target_x;
            s32 current_x = actor->x;
            actor->direction = ratan2(current_z - target_z, target_x - current_x) >> FIELD_HEADING_ANGLE_SHIFT;
        }
        movement_dx = rcos(actor->direction * 16) >> 4;
        field_move_actor_step(actor, movement_dx, 0, -rsin(actor->direction * 16) >> 4);
        if (actor->command == FIELD_ACTOR_COMMAND_NONE)
        {
            actor->running = 0;
            return;
        }
        break;
    case FIELD_ACTOR_COMMAND_WALK_FROM_TARGET:
    {
        s32 target_z;
        s32 current_z;
        s32 target_x;
        s32 current_x;
        target_z = state->target_z;
        current_z = actor->z;
        target_x = state->target_x;
        current_x = actor->x;
        heading = (ratan2(current_z - target_z, target_x - current_x) >> FIELD_HEADING_ANGLE_SHIFT) - FIELD_HEADING_STEPS / 2;
        actor->direction = heading;
        movement_dx = rcos(actor->direction * 16) >> 4;
        field_move_actor_step(actor, movement_dx, 0, -rsin(actor->direction * 16) >> 4);
        return;
    }
    case FIELD_ACTOR_COMMAND_RISE:
        field_move_actor_step(actor, 0, -FIELD_VERTICAL_STEP, 0);
        return;
    case FIELD_ACTOR_COMMAND_SINK:
        field_move_actor_step(actor, 0, FIELD_VERTICAL_STEP, 0);
        return;
    case FIELD_ACTOR_COMMAND_9C:
        field_update_actor_lift(actor, 1);
        return;
    case FIELD_ACTOR_COMMAND_9D:
        field_update_actor_lift(actor, 0);
        return;
    case FIELD_ACTOR_COMMAND_APPROACH_TARGET:
        approach_distance_z = state->target_z - actor->z;
        approach_distance_z = abs(approach_distance_z);
        approach_delta_x = state->target_x - actor->x;
        approach_distance_x = abs(approach_delta_x);
        if ((approach_distance_z + approach_distance_x) >= FIELD_TARGET_APPROACH_REACHED)
        {
            actor->direction = ratan2(actor->z - state->target_z, approach_delta_x) >> FIELD_HEADING_ANGLE_SHIFT;
            approach_dx = rcos(actor->direction * 16) >> 4;
            field_slide_actor(actor, approach_dx, -rsin(actor->direction * 16) >> 4);
            return;
        }
        actor->command = FIELD_ACTOR_COMMAND_NONE;
        return;
    case FIELD_ACTOR_COMMAND_TIMED_WALK:
        timed_dx = rcos(actor->direction * 16) >> 4;
        field_update_timed_walk(actor, timed_dx, -rsin(actor->direction * 16) >> 4);
        if ((state->command_timer == 0) || (--state->command_timer == 0))
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
        }
        return;
    case FIELD_ACTOR_COMMAND_TIMED_SLIDE:
        movement_dx = rcos(actor->direction * 16) >> 4;
        field_update_timed_slide(actor, movement_dx, -rsin(actor->direction * 16) >> 4);
        if ((state->command_timer == 0) || (--state->command_timer == 0))
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
        }
        return;
    case FIELD_ACTOR_COMMAND_WAIT_BOUND_ACTOR:
        if (g_field_actor_slots[g_field_actor_bindings[FIELD_OBJECT_BINDING(actor->object_index)].slot].active == 0)
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            return;
        }
        break;
    case FIELD_ACTOR_COMMAND_INSTRUMENT_END:
        if (actor->animation_state == 0)
        {
            animation = actor->animation;
            if ((animation & FIELD_ANIMATION_INDEX_MASK) == FIELD_ANIMATION_INSTRUMENT)
            {
                finish_states = g_field_object_states;
                actor->animation = (animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_DEFENSELESS;
                actor->animation_state = 1;
                actor->animation_frame = 0;
                actor->animation_active = 1;
                finished_state = &finish_states[actor->object_index];
                finished_state->movement.word = finished_state->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK;
                field_restart_actor_animation(actor);
                return;
            }
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            return;
        }
        break;
    case FIELD_ACTOR_COMMAND_ACTION_END:
        if (actor->animation_state == 0)
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            actor->animation_state = 1;
            actor->animation_active = 1;
            actor->animation = actor->animation & FIELD_ANIMATION_FACING;
            field_restart_actor_animation(actor);
            return;
        }
        break;
    case FIELD_ACTOR_COMMAND_SEQUENCE_WAIT:
        command_timer = actor->command_param;
        if (command_timer == 0)
        {
            field_restart_sequence_animation(actor);
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            field_command_history_clear(actor->object_index);
            return;
        }
        actor->command_param = command_timer - 1;
        return;
    case FIELD_ACTOR_COMMAND_ACTION:
    {
        FieldObjectState* action_states;
        action_states = g_field_object_states;
        action_states[actor->object_index].contact.bytes.target_count = 0;
        if (actor->object_index < FIELD_PARTY_COUNT)
        {
            reset_players = g_field_player_records;
            reset_players[actor->object_index].combo_timer = 0;
        }
        if (actor->control.word & FIELD_CONTROL_MODE_MASK)
        {
            action = FIELD_RESOURCE_ACTION(actor->resource_index, action_states[actor->object_index].action);
        }
        else
        {
            action_state = &action_states[actor->object_index];
            if (action_state->action < FIELD_RESOURCE_ACTION_BUTTONS)
            {
                action = FIELD_RESOURCE_ACTION(actor->resource_index, action_state->action);
            }
            else
            {
                action = &g_field_resource_actions[actor->resource_index][FIELD_RESOURCE_ACTION_REDIRECT];
            }
        }
    }
        if ((u8)action->flags != FIELD_ACTION_NO_HANDLER)
        {
            s32 action_pending;
            target_count_or_slot = field_collect_action_targets(actor->object_index, action, (action->flags >> FIELD_ACTION_TARGET_MODE_SHIFT) & FIELD_ACTION_TARGET_MODE_MASK,
                                                 state->movement.half.lo & FIELD_MOVEMENT_SCALE_MASK, scratch.targets);
            action_pending = 0;
            if (g_field_actions_limited == 0)
            {
                if (!(actor->control.word & FIELD_CONTROL_MODE_MASK))
                {
                    FieldObjectState* check_base = g_field_object_states;
                    action_object_index = actor->object_index;
                    checked_state = &check_base[action_object_index];
                    if (checked_state->action < FIELD_RESOURCE_ACTION_BUTTONS)
                    {
                        action_pending = field_get_held_action_buttons(action_object_index, checked_state->action, actor);
                    }
                }
                else
                {
                    action_pending = field_actor_action_is_charging(actor);
                }
            }
            if (action_pending != 0)
            {
                pending_states = g_field_object_states;
                pending_state = &pending_states[actor->object_index];
                pending_state->contact.word = pending_state->contact.word | FIELD_CONTACT_ACTION_PENDING;
                field_draw_object_ground_effect(actor, (u8)action->flags);
                if ((action->flags & FIELD_ACTION_INSTRUMENT) && (actor->animation_state == 0))
                {
                    actor->animation_state = 1;
                    actor->animation_active = 1;
                    actor->animation_frame = 0;
                    actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_INSTRUMENT_START;
                    instrument_state = &pending_states[actor->object_index];
                    instrument_state->movement.word = instrument_state->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK;
                    field_restart_actor_animation(actor);
                }
                charge_states = g_field_object_states;
                charge_state = &charge_states[actor->object_index];
                sequence_flags = charge_state->movement.word;
                if (!((sequence_flags >> 10) & 1) && (charge_state->action_charge >= FIELD_ACTION_CHARGE_ANIMATION))
                {
                    charge_animation = action->animation;
                    if ((charge_animation != FIELD_ACTION_NO_ANIMATION) && (charge_animation != 0))
                    {
                        charge_state->movement.word = sequence_flags | FIELD_MOVEMENT_CHARGED;
                        target_count_or_slot = field_find_free_actor_slot(actor->object_index, 0);
                        if ((target_count_or_slot != -1) &&
                            (field_start_builtin_animation(actor->object_index, target_count_or_slot, action->animation) != 0))
                        {
                            field_start_actor_animation(target_count_or_slot, 0, 0);
                            charge_states[actor->object_index].contact.bytes.animation_actor_index = target_count_or_slot;
                            charge_object_index = actor->object_index;
                            func_800A623C(charge_object_index,
                                          (0x80000000 | (charge_object_index << 0x10)) |
                                              (charge_states[charge_object_index].action - 4));
                            return;
                        }
                    }
                }
                break;
            }
            else
            {
                released_state = &g_field_object_states[actor->object_index];
                released_state->contact.word = released_state->contact.word & ~FIELD_CONTACT_ACTION_PENDING;
                release_object_index = actor->object_index;
                if (!((g_field_object_states[release_object_index].movement.word >> 10) & 1))
                {
                    g_field_actor_slots[g_field_actor_bindings[FIELD_OBJECT_BINDING(release_object_index)].slot].active = 0;
                    field_release_actor_binding(actor->object_index);
                    cancel_states = g_field_object_states;
                    actor->animation_state = 1;
                    actor->animation_active = 1;
                    actor->animation_frame = 0;
                    actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_INSTRUMENT;
                    cancelled_state = &cancel_states[actor->object_index];
                    cancelled_state->movement.word = cancelled_state->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK;
                    field_restart_actor_animation(actor);
                    actor->command = FIELD_ACTOR_COMMAND_INSTRUMENT_END;
                    return;
                }
                eligible_count = field_filter_action_targets(target_count_or_slot, scratch.targets,
                                                             g_field_actor_bindings[FIELD_OBJECT_BINDING(release_object_index)].slot);
                actor_slots = g_field_actor_slots;
                animation_slot = &actor_slots[g_field_actor_bindings[FIELD_OBJECT_BINDING(actor->object_index)].slot];
                animation_slot->status.word = (animation_slot->status.word & ~FIELD_SLOT_ELEMENT_BITS) | (((u8)action->flags & FIELD_ACTION_ELEMENT_MASK) * 2);
                if (!(action->request & FIELD_REQUEST_TARGETED))
                {
                    if (field_start_action_animation(actor->object_index, 0, NULL, action->request) == 0)
                    {
                        g_field_object_states[actor->object_index].contact.word |= FIELD_CONTACT_ACTION_PENDING;
                        return;
                    }
                }
                else if (eligible_count != 0)
                {
                    animation_target_count = eligible_count;
                    if (eligible_count > FIELD_ACTION_MAX_TARGETS)
                    {
                        eligible_count = FIELD_ACTION_MAX_TARGETS;
                        animation_target_count = FIELD_ACTION_MAX_TARGETS;
                    }
                    if (field_start_action_animation(actor->object_index, animation_target_count, scratch.targets,
                                                     action->request) != 0)
                    {
                        target_index = 0;
                        if (eligible_count > 0)
                        {
                            do
                            {
                                target_cursor = scratch.targets + target_index;
                                target_state = &g_field_object_states[*target_cursor];
                                target_state->contact.word = target_state->contact.word | FIELD_CONTACT_TARGETED;
                                target_list_state = &g_field_object_states[actor->object_index];
                                target_list_state->targets[target_list_state->contact.bytes.target_count] = *target_cursor;
                                target_index += 1;
                                target_count_state = &g_field_object_states[actor->object_index];
                                target_count_state->contact.bytes.target_count = target_count_state->contact.bytes.target_count + 1;
                            } while (target_index < eligible_count);
                        }
                    }
                    else
                    {
                        g_field_object_states[actor->object_index].contact.word |= FIELD_CONTACT_ACTION_PENDING;
                        return;
                    }
                }
                else
                {
                    g_field_actor_slots[g_field_actor_bindings[FIELD_OBJECT_BINDING(actor->object_index)].slot].active = 0;
                    field_release_actor_binding(actor->object_index);
                    g_field_object_states[actor->object_index].contact.bytes.animation_actor_index = FIELD_ANIMATION_ACTOR_NONE;
                }
            }
        }
        {
            if (action->flags & FIELD_ACTION_INSTRUMENT)
            {
                instrument_states = g_field_object_states;
                actor->command = FIELD_ACTOR_COMMAND_INSTRUMENT;
                actor->animation_state = 1;
                actor->animation_active = 1;
                actor->animation_frame = 0;
                actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_INSTRUMENT;
                instrument_start_state = &instrument_states[actor->object_index];
                instrument_start_state->movement.word = instrument_start_state->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK;
                field_restart_actor_animation(actor);
                actor->control.word |= FIELD_CONTROL_PLAY_ONCE;
                return;
            }
            if (action->command & FIELD_ACTION_TECHNIQUE)
            {
                g_field_object_states[actor->object_index].technique_gauge = 0;
                state->sequence_position = 0;
                state->sequence_id = action->request;
                state->action_parameter = action->animation;
                state->movement.word = state->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK;
                state->sequence_id = (((action->command & FIELD_ACTION_TECHNIQUE_MASK) + FIELD_TECHNIQUE_SEQUENCE_BASE) | FIELD_SEQUENCE_TECHNIQUE) +
                                     (g_field_player_records[actor->object_index].weapon_type * FIELD_TECHNIQUES_PER_WEAPON);
                if (actor->object_index < FIELD_PARTY_COUNT)
                {
                    technique_object_index = actor->object_index;
                    func_800A623C(technique_object_index,
                                  (action->command & FIELD_ACTION_TECHNIQUE_MASK) + (g_field_player_records[technique_object_index].weapon_type * FIELD_TECHNIQUES_PER_WEAPON));
                }
                field_execute_actor_sequence(actor, action->command & FIELD_ACTION_TECHNIQUE_MASK);
                state->sequence = action->command & FIELD_ACTION_TECHNIQUE_MASK;
                actor->command = FIELD_ACTOR_COMMAND_TECHNIQUE;
                field_restart_actor_animation(actor);
                actor->control.word = actor->control.word | FIELD_CONTROL_PLAY_ONCE;
                return;
            }
            if (actor->object_index < FIELD_PARTY_COUNT)
            {
                combo_object_index = actor->object_index;
                if (g_field_object_states[combo_object_index].action < FIELD_RESOURCE_ACTION_COMBO)
                {
                    g_field_player_records[combo_object_index].combo_timer = FIELD_COMBO_WINDOW;
                }
            }
            field_apply_action_animation(actor, state, action);
            return;
        }
        break;
    case FIELD_ACTOR_COMMAND_BA:
        break;
    default:
        scratch.displacement.vx = 0;
        scratch.displacement.vy = 0;
        scratch.displacement.vz = 0;
        field_resolve_actor_movement(actor, &scratch.displacement.vx, 0);
        actor->command = FIELD_ACTOR_COMMAND_NONE;
        return;
    }
}

/**
 * @brief Keep only the valid action targets, packed at the front of @p indices.
 * @param count Number of object indices to check.
 * @param indices Object indices; the valid ones are written back from the start.
 * @return Number of valid targets.
 */
static s32 field_filter_action_targets(s32 count, s32* indices)
{
    FieldActor* actor_base;
    FieldObjectState* state_base;
    u8* binding_base;
    s32 group;
    s32 sentinel;
    s32 accepted_indices[16];
    s16 command;
    s32* output;
    s32* input;
    s32* accepted;
    s32* copy;
    s32 owner_index;
    s32 object_index;
    s32 contact;
    s32 index;
    s32 candidate_index;
    s32 action_contact;
    s32 i;
    s32 accepted_count;
    s32 binding_offset;
    s32 owner_binding_offset;
    FieldActor* actor;
    FieldObjectState* state;

    output = indices;
    i = 0;
    accepted_count = i;
    if (count > 0)
    {
        sentinel = FIELD_ACTOR_UNUSED;
        actor_base = g_field_actors;
        state_base = g_field_object_states;
        group = g_field_active_group;
        binding_base = (u8*)g_field_actor_bindings;
        input = output;
        accepted = accepted_indices;
        do
        {
            candidate_index = *input;
            if (candidate_index != sentinel)
            {
                /* Index-first sums: the target adds the scaled index before the table base. */
                actor = (FieldActor*)(candidate_index * sizeof(FieldActor) + (s32)actor_base);
                state = (FieldObjectState*)(candidate_index * sizeof(FieldObjectState) + (s32)state_base);
                if ((actor->presence != sentinel) && (state->unk4.word != 0))
                {
                    contact = state->contact.word;
                    if (!(contact & FIELD_CONTACT_ANIMATION_HIDDEN) && ((i < FIELD_PARTY_COUNT) || ((state->group_flags & FIELD_OBJECT_GROUP_MASK) == group)))
                    {
                        command = actor->command;
                        if ((command != FIELD_ACTOR_COMMAND_TECHNIQUE) && (command != FIELD_ACTOR_COMMAND_DEFEAT_DELAY) && (command != FIELD_ACTOR_COMMAND_INSTRUMENT))
                        {
                            if (!(contact & FIELD_CONTACT_ACTION_PENDING))
                            {
                                /* A party member whose bound animation actor is busy is skipped. */
                                if (actor->object_index < FIELD_PLAYER_COUNT)
                                {
                                    binding_offset = actor->object_index * sizeof(FieldActorBinding);
                                }
                                else
                                {
                                    binding_offset = FIELD_PLAYER_COUNT * sizeof(FieldActorBinding);
                                }
                                object_index = actor->object_index;
                                actor = (FieldActor*)((FieldActorBinding*)(binding_base + binding_offset))->owner;
                                owner_index = (s32)actor;
                                if (owner_index == object_index)
                                {
                                    if ((u32)(owner_index & 0xFF) < FIELD_PLAYER_COUNT)
                                    {
                                        owner_binding_offset = owner_index * sizeof(FieldActorBinding);
                                    }
                                    else
                                    {
                                        owner_binding_offset = FIELD_PLAYER_COUNT * sizeof(FieldActorBinding);
                                    }
                                    if (((FieldActorBinding*)(binding_base + owner_binding_offset))->state != 0)
                                    {
                                        i += 1;
                                        input++;
                                        continue;
                                    }
                                }
                            }
                            if ((state->collision.word != 0) && !(state->flags & FIELD_OBJECT_UNTARGETABLE_FLAGS))
                            {
                                action_contact = state->contact.word;
                                /* The flag test goes through the low byte; testing the word changes the code. */
                                if (!(action_contact & FIELD_CONTACT_UNK20) && !((u8)action_contact & FIELD_CONTACT_TARGETED) && !(state->movement.word & FIELD_MOVEMENT_TECHNIQUE))
                                {
                                    accepted_count += 1;
                                    *accepted = *input;
                                    accepted++;
                                }
                            }
                        }
                    }
                }
            }
            i += 1;
            input++;
        } while (i < count);
    }
    i = 0;
    if (accepted_count > 0)
    {
        copy = accepted_indices;
        do
        {
            index = *copy;
            copy++;
            i += 1;
            *output = index;
            output++;
        } while (i < accepted_count);
    }
    return accepted_count;
}

/**
 * @brief Start an action's animation on an actor and apply its side effects.
 * @param actor Actor that performs the action.
 * @param state Object state of @p actor.
 * @param action Action to perform; its command selects the animation.
 * @return Nothing useful; the value is undefined.
 */
static s32 field_apply_action_animation(FieldActor* actor, FieldObjectState* state, FieldResourceAction* action)
{
    s32 animation_index;
    s32 masked_command;
    s32 command;
    u16 gauge;
    u16 next_gauge;

    state->action_parameter = action->request;
    command = action->command;
    if (((u32)(command - 0x2F) < 2U) || (masked_command = command & 0xFFFF, (masked_command == 0x44)) || (masked_command == 0x45))
    {
        state->flags = state->flags | 0x4000;
    }
    if ((action->command == 0x1F) && (actor->variant != 0) &&
        ((actor->object_index >= FIELD_PLAYER_COUNT) || (g_field_player_records[actor->object_index].weapon_type != FIELD_WEAPON_TYPE_NO_VARIANT)))
    {
        actor->animation = (u8)actor->variant + ((u8)action->command + (actor->animation & FIELD_ANIMATION_FACING));
    }
    else
    {
        actor->animation = (u8)action->command + (actor->animation & FIELD_ANIMATION_FACING);
    }
    actor->animation_state = 1;
    actor->animation_frame = 0;
    actor->animation_active = 1;
    state->movement.word = state->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK;
    field_restart_actor_animation(actor);
    animation_index = actor->animation & FIELD_ANIMATION_INDEX_MASK;
    switch (animation_index)
    {
    case 0x41:
        field_battle_set_watched_record(state->key);
        actor->command = FIELD_ACTOR_COMMAND_RECOVER;
        break;
    case 0x33:
        gauge = state->technique_gauge;
        next_gauge = gauge < FIELD_TECHNIQUE_GAUGE_FULL + 1 - FIELD_TECHNIQUE_GAUGE_STEP ? gauge + FIELD_TECHNIQUE_GAUGE_STEP : FIELD_TECHNIQUE_GAUGE_FULL;
        state->technique_gauge = next_gauge;
        actor->command = FIELD_ACTOR_COMMAND_RECOVER;
        break;
    case 0x08:
    case 0x0A:
    case 0x31:
    case 0x3D:
        actor->command = FIELD_ACTOR_COMMAND_RECOVER;
        actor->control.word = actor->control.word | FIELD_CONTROL_PLAY_ONCE;
        break;
    default:
        actor->command = FIELD_ACTOR_COMMAND_RECOVER;
        break;
    }
    if ((actor->object_index < FIELD_PLAYER_COUNT) && (g_field_action_sound_ids[actor->animation & FIELD_ANIMATION_INDEX_MASK] != FIELD_SOUND_NONE))
    {
        func_800A3938(g_field_action_sound_ids[actor->animation & FIELD_ANIMATION_INDEX_MASK], FIELD_SOUND_PAN_CENTRE);
    }
}

/**
 * @brief Report whether an actor's action is still charging.
 * @param actor Actor to check.
 * @return Non-zero while the action charge is below FIELD_ACTION_CHARGE_FULL.
 */
static s32 field_actor_action_is_charging(FieldActor* actor)
{
    return g_field_object_states[actor->object_index].action_charge < FIELD_ACTION_CHARGE_FULL;
}

/**
 * @brief Start the animation of an action request, on a new actor slot or the bound one.
 * @param object_index Object that performs the action.
 * @param target_count Number of entries in @p targets.
 * @param targets Target object indices.
 * @param request Animation request (FIELD_REQUEST_* bits plus an animation resource).
 * @return 1 when the request was handled, 0 when the bound actor was not ready.
 */
static s32 field_start_action_animation(s32 object_index, s32 target_count, u8* targets, s32 request)
{
    s32 animation_slot;

    if (request & FIELD_REQUEST_BOUND)
    {
        return field_start_bound_action_animation(object_index, target_count, targets, request);
    }

    animation_slot = field_find_free_actor_slot(object_index, 0);
    if ((animation_slot != -1) && (field_start_builtin_animation(object_index, animation_slot, request & FIELD_REQUEST_ANIMATION_MASK) != 0))
    {
        field_start_actor_animation(animation_slot, target_count, targets);
        g_field_object_states[object_index].contact.bytes.animation_actor_index = animation_slot;
    }
    return 1;
}

/**
 * @brief Start an action request on the object's bound animation actor.
 *
 * A layered request selects one animation of the bound actor, or copies the
 * actor into extra slots so several layers play together.
 *
 * @param object_index Object that performs the action.
 * @param target_count Number of entries in @p targets.
 * @param targets Target object indices.
 * @param request Animation request (FIELD_REQUEST_* bits).
 * @return 1 when the request was started, 0 when the bound actor was not ready.
 */
s32 field_start_bound_action_animation(s32 object_index, s32 target_count, u8* targets, s32 request)
{
    u16 layer_value;
    s32 remaining_layers;
    s32 result;
    s32 binding_index;
    s32 new_slot;
    s32 layer_index;
    FieldActorSlot* slot;
    FieldActorBinding* binding;
    FieldActorSlot* base_slot;
    FieldAnimationDef* animations;
    FieldActorSlot* default_slot;
    FieldActorSlot* selected_slot;
    FieldActorSlot* layer_slot;
    FieldActorSlot* initialized_slot;

    binding_index = object_index;
    if (object_index >= FIELD_ACTOR_BINDING_COUNT)
    {
        binding_index = FIELD_ACTOR_BINDING_COUNT - 1;
    }
    binding = &g_field_actor_bindings[binding_index];
    if (binding->owner != object_index)
    {
        return 0;
    }
    if (binding->state != FIELD_BINDING_READY)
    {
        return 0;
    }

    if (request & FIELD_REQUEST_LAYERED)
    {
        if (!(request & FIELD_REQUEST_ALL_LAYERS))
        {
            g_field_actor_slots[binding->slot].animation_index = (request >> FIELD_REQUEST_LAYER_SHIFT) & FIELD_REQUEST_LAYER_MASK;
            layer_slot = &g_field_actor_slots[binding->slot];
            layer_slot->duration = layer_slot->default_animation[layer_slot->animation_index].duration;
        }
        else
        {
            g_field_actor_slots[binding->slot].animation_index = 0;
            base_slot = &g_field_actor_slots[binding->slot];
            remaining_layers = (request >> FIELD_REQUEST_LAYER_SHIFT) & FIELD_REQUEST_LAYER_MASK;
            base_slot->duration = base_slot->default_animation->duration;
            layer_index = 1;
            g_field_actor_slots[binding->slot].unk2A = 1;
            if (remaining_layers != 0)
            {
                do
                {
                    new_slot = field_find_free_actor_slot(object_index, 0);
                    if (new_slot != -1)
                    {
                        slot = &g_field_actor_slots[new_slot];
                        bcopy((u8*)&g_field_actor_slots[binding->slot], (u8*)slot, sizeof(FieldActorSlot));
                        animations = slot->default_animation;
                        slot->slot_index = new_slot;
                        slot->animation_index = layer_index;
                        layer_value = animations->duration;
                        slot->track_interval = 0;
                        slot->animation = &animations[layer_index];
                        result = 1;
                        slot->unk2A = result;
                        slot->active = result;
                        slot->duration = layer_value;
                        field_start_actor_animation(new_slot, target_count, targets);
                    }
                    remaining_layers -= 1;
                    layer_index += 1;
                } while (remaining_layers != 0);
            }
        }
    }
    else
    {
        default_slot = &g_field_actor_slots[binding->slot];
        default_slot->duration = default_slot->default_animation->duration;
        g_field_actor_slots[binding->slot].animation_index = 0;
    }
    initialized_slot = &g_field_actor_slots[binding->slot];
    initialized_slot->track_interval = initialized_slot->default_animation[initialized_slot->animation_index].unk16;
    selected_slot = &g_field_actor_slots[binding->slot];
    selected_slot->animation = selected_slot->default_animation + selected_slot->animation_index;
    g_field_actor_slots[binding->slot].active = 1;
    field_start_actor_animation(binding->slot, target_count, targets);
    g_field_object_states[object_index].contact.bytes.animation_actor_index = binding->slot;
    result = 1;
    return result;
}
