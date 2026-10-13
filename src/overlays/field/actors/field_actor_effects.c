/**
 * @file field_actor_effects.c
 * @brief Ground effects drawn under field objects.
 *
 * field_start_object_ground_effect starts an object's ground effect and
 * field_draw_object_ground_effect draws it once per frame: it grows the
 * effect radius (the low ten movement bits of the object state), advances the
 * effect angle and emits the effect through one of the four packet builders
 * that follow. Every effect is made of Gouraud-shaded, additively blended
 * quads.
 */

#include "common.h"
#include "../internal/field_calls.h"
#include "main/controller_internal.h"
#include "main/display.h"
#include "../internal/field_effect_dispatch.h"
#include "../internal/field_effect_render_state.h"
#include "../internal/field_effect_types.h"
#include "main/main.h"
#include "common/pad.h"
#include <libgte.h>
#include <libgpu.h>
#include <inline_c.h>
#include "sdk/gte_dmpsx_compat.h"
#include <rand.h>
#include "common/gpu_packet.h"

/** @brief Number of scattered-dome points (FieldObjectState::ground_attachment_points). */
#define SCATTER_POINT_COUNT 3

/** @brief Minimum distance between two scattered-dome points, in whole units. */
#define SCATTER_MIN_DISTANCE 64

/** @brief Channel level of the brightest effect vertex. */
#define EFFECT_LEVEL 160

/** @brief Packed color word with only a blue channel; also clears the primitive code. */
#define EFFECT_BLUE(level) (((level) << 16) & 0xFFFFFF)

/** @brief Packed color word with only a green channel. */
#define EFFECT_GREEN(level) (((level) << 8) & 0xFF00)

/** @brief Strips per effect, and the segments each strip or arch is built from. */
#define STRIP_COUNT 4
#define STRIP_SEGMENT_COUNT 8

/** @brief Width of a curved strip, in fixed-point units. */
#define CURVE_STRIP_WIDTH 0x1400

/** @brief Width of a spiral strip, in radius units. */
#define SPIRAL_STRIP_WIDTH 20

/** @brief Radius increment between two spiral strips, and the radius wrap limit. */
#define SPIRAL_RADIUS_STEP 80
#define SPIRAL_RADIUS_LIMIT 320

/** @brief Radius range of each ground effect kind: the effect starts at the minimum and grows to the maximum. */
#define GROUND_EFFECT_DOME_MIN_RADIUS 30
#define GROUND_EFFECT_DOME_MAX_RADIUS 96
#define GROUND_EFFECT_STRIPS_MIN_RADIUS 64
#define GROUND_EFFECT_STRIPS_MAX_RADIUS 128
#define GROUND_EFFECT_SPIRAL_MIN_RADIUS 16
#define GROUND_EFFECT_SPIRAL_MAX_RADIUS 64
#define GROUND_EFFECT_CURVES_MIN_RADIUS 30
#define GROUND_EFFECT_CURVES_MAX_RADIUS 200
#define GROUND_EFFECT_SCATTERED_DOMES_MIN_RADIUS 30
#define GROUND_EFFECT_SCATTERED_DOMES_MAX_RADIUS 64
#define GROUND_EFFECT_DRIFTING_DOME_MIN_RADIUS 0
#define GROUND_EFFECT_DRIFTING_DOME_MAX_RADIUS 100
#define GROUND_EFFECT_AIMED_DOME_MIN_RADIUS 30
#define GROUND_EFFECT_AIMED_DOME_MAX_RADIUS 64

/** @brief Effect angle change per frame of the rotating effects. */
#define EFFECT_SPIN_STEP 0x80

/** @brief Phase step per frame of the strip effects, and the number of phases (the phase runs 0..79). */
#define EFFECT_PHASE_STEP 4
#define EFFECT_PHASE_COUNT 80

/** @brief Scratchpad word pair holding the GTE distance input vector. */
#define DISTANCE_DELTA ((VECTOR*)SCRATCHPAD_ADDRESS)

/** @brief Scratchpad words receiving the squared distance components. */
#define DISTANCE_SQUARES ((VECTOR*)SCRATCHPAD_AT(0x10))

/** @brief Runtime state of @p actor's object. */
#define OBJECT_STATE(actor) g_field_object_states[(actor)->source_object_index]

/** @brief Current effect radius of @p actor's object (low ten movement bits). */
#define EFFECT_RADIUS(actor) (OBJECT_STATE(actor).movement.half.flags & FIELD_MOVEMENT_SCALE_MASK)

/** @brief Replace the effect radius of @p actor's object, keeping the other movement bits. */
#define SET_EFFECT_RADIUS(actor, radius)                                                                                                                       \
    (OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~FIELD_MOVEMENT_SCALE_MASK) | ((radius) & FIELD_MOVEMENT_SCALE_MASK))

/** @brief Project world point @p _tmp into vertex @p _vert of quad @p _poly. */
#define PROJECT_POINT(_poly, _vert, _tmp)                                                                                                                      \
    (_poly)->x##_vert = (s16)(FIELD_SCREEN_CENTER_X + g_field_view_offset_x / 256 + (_tmp).vx / 256);                                                                \
    (_poly)->y##_vert = (s16)(FIELD_SCREEN_CENTER_Y + g_field_view_offset_y / 256 + (_tmp).vy / 256 - (_tmp).vz / 512 - g_field_view_offset_z / 512)

/**
 * @brief Link the packet at the cursor into the ordering table by depth and advance the cursor.
 * @note @p _bucket is evaluated up to three times.
 */
#define ADD_PRIM_BY_DEPTH(_bucket, _type)                                                                                                                      \
    if ((_bucket) < 0)                                                                                                                                         \
    {                                                                                                                                                          \
        addPrim(&ordering_table[0], (_type*)cursor);                                                                                                           \
        cursor += sizeof(_type);                                                                                                                               \
    }                                                                                                                                                          \
    else if ((_bucket) >= FIELD_WORLD_OT_SIZE)                                                                                                                    \
    {                                                                                                                                                          \
        addPrim(&ordering_table[FIELD_WORLD_OT_SIZE - 1], (_type*)cursor);                                                                                        \
        cursor += sizeof(_type);                                                                                                                               \
    }                                                                                                                                                          \
    else                                                                                                                                                       \
    {                                                                                                                                                          \
        addPrim(&ordering_table[(_bucket)], (_type*)cursor);                                                                                                   \
        cursor += sizeof(_type);                                                                                                                               \
    }

/**
 * @brief Rotate (@p _radius, 0, 0) by @p matrix and store position +/- the result in work.world.
 * @note Uses the locals work, matrix, position and facing of field_build_effect_spiral.
 */
#define STRIP_POINT(_radius)                                                                                                                                   \
    work.input.vx = (_radius);                                                                                                                                 \
    work.input.vy = 0;                                                                                                                                         \
    work.input.vz = 0;                                                                                                                                         \
    gte_SetRotMatrix(matrix);                                                                                                                                  \
    gte_ldv0(&work.input);                                                                                                                                     \
    gte_rtv0();                                                                                                                                                \
    gte_stlvnl(&work.rotated);                                                                                                                                 \
    if (facing != 0)                                                                                                                                           \
    {                                                                                                                                                          \
        work.world.vx = position->vx + (work.rotated.vx << 8);                                                                                                 \
        work.world.vy = position->vy + (work.rotated.vy << 8);                                                                                                 \
        work.world.vz = position->vz + (work.rotated.vz << 8);                                                                                                 \
    }                                                                                                                                                          \
    else                                                                                                                                                       \
    {                                                                                                                                                          \
        work.world.vx = position->vx - (work.rotated.vx << 8);                                                                                                 \
        work.world.vy = position->vy + (work.rotated.vy << 8);                                                                                                 \
        work.world.vz = position->vz + (work.rotated.vz << 8);                                                                                                 \
    }

/** @brief Ground effect kinds; the kind also selects the radius range. */
typedef enum GroundEffectKind
{
    GROUND_EFFECT_DOME = 0,            /**< Rotating dome of arches. */
    GROUND_EFFECT_STRIPS = 1,          /**< Four rotating arched strips. */
    GROUND_EFFECT_SPIRAL = 2,          /**< Four strips wound around a tilted axis. */
    GROUND_EFFECT_CURVES = 3,          /**< Curved strips running out to both sides. */
    GROUND_EFFECT_SCATTERED_DOMES = 4, /**< Domes at three random ground points. */
    GROUND_EFFECT_DRIFTING_DOME = 5,   /**< Dome drifting in the facing direction. */
    GROUND_EFFECT_AIMED_DOME = 6       /**< Dome steered with the object's controller. */
} GroundEffectKind;

/** @brief Rotation matrix that is also cleared and set up word by word. */
typedef union
{
    MATRIX matrix;
    s32 words[sizeof(MATRIX) / sizeof(s32)];
} MatrixWords;

/** @brief Stack workspace of the strip builders (field_build_effect_curves uses only the world vector). */
typedef struct
{
    VECTOR rotated;
    VECTOR world;
    VECTOR unused;
    SVECTOR input;
    MATRIX unused_matrices[2];
    MatrixWords rotation;
    MATRIX unused_matrices_after[2];
} FieldStripWorkspace;

void field_get_ground_effect_radius_limits(s32 kind, s32* min_radius, s32* max_radius);

static u8* field_build_effect_dome(u_long* ordering_table, u8* packet, VECTOR* position, s32 radius);
static u8* field_build_effect_strips(u_long* ordering_table, u8* packet, VECTOR* position, s32 radius);
static u8* field_build_effect_curves(u_long* ordering_table, u8* packet, VECTOR* position, s32 extent, s32 forward);
static u8* field_build_effect_spiral(u_long* ordering_table, u8* packet, VECTOR* position, s32 tilt, s32 facing);
extern s32 g_field_effect_angle;

/**
 * @brief Start an object's ground effect.
 * @param actor Actor whose object gets the effect.
 * @param kind Effect kind (GroundEffectKind).
 * @note The radius starts at the kind's minimum (see field_get_ground_effect_radius_limits).
 */
void field_start_object_ground_effect(FieldMotionRecord* actor, u32 kind)
{
    VECTOR* delta = DISTANCE_DELTA;
    VECTOR* squares = DISTANCE_SQUARES;
    s32 point_index;
    s32 compare_index;
    s32 needs_retry;

    OBJECT_STATE(actor).effect_angle = 0;

    switch (kind)
    {
    case GROUND_EFFECT_STRIPS:
        SET_EFFECT_RADIUS(actor, GROUND_EFFECT_STRIPS_MIN_RADIUS);
        return;
    case GROUND_EFFECT_SPIRAL:
        SET_EFFECT_RADIUS(actor, GROUND_EFFECT_SPIRAL_MIN_RADIUS);
        return;
    case GROUND_EFFECT_DOME:
        SET_EFFECT_RADIUS(actor, GROUND_EFFECT_DOME_MIN_RADIUS);
        return;
    case GROUND_EFFECT_CURVES:
        SET_EFFECT_RADIUS(actor, GROUND_EFFECT_CURVES_MIN_RADIUS);
        return;
    case GROUND_EFFECT_SCATTERED_DOMES:
        SET_EFFECT_RADIUS(actor, GROUND_EFFECT_SCATTERED_DOMES_MIN_RADIUS);

        /* Pick each point in -128..127 around the view until it is far enough from the earlier ones. */
        for (point_index = 0; point_index < SCATTER_POINT_COUNT; point_index++)
        {
            needs_retry = 1;

            do
            {
                OBJECT_STATE(actor).ground_attachment_points[point_index].x = (rand() >> 7) - 128;
                OBJECT_STATE(actor).ground_attachment_points[point_index].y = (rand() >> 7) - 128;
                OBJECT_STATE(actor).ground_attachment_points[point_index].x =
                    OBJECT_STATE(actor).ground_attachment_points[point_index].x - g_field_view_offset_x / 256 - actor->x / 256;
                OBJECT_STATE(actor).ground_attachment_points[point_index].y =
                    OBJECT_STATE(actor).ground_attachment_points[point_index].y - g_field_view_offset_z / 256 - actor->z / 256;
                for (compare_index = 0; compare_index < point_index; compare_index++)
                {
                    delta->vx = OBJECT_STATE(actor).ground_attachment_points[compare_index].x - OBJECT_STATE(actor).ground_attachment_points[point_index].x;
                    delta->vy = OBJECT_STATE(actor).ground_attachment_points[compare_index].y - OBJECT_STATE(actor).ground_attachment_points[point_index].y;
                    delta->vz = 0;
                    gte_ldlvl(delta);
                    gte_sqr0();
                    gte_stlvnl(squares);

                    if (SquareRoot0(squares->vx + squares->vy) < SCATTER_MIN_DISTANCE)
                    {
                        break;
                    }
                }

                if (compare_index == point_index)
                {
                    needs_retry = 0;
                }
            } while (needs_retry != 0);
        }

        return;
    case GROUND_EFFECT_DRIFTING_DOME:
        SET_EFFECT_RADIUS(actor, GROUND_EFFECT_DRIFTING_DOME_MIN_RADIUS);
        OBJECT_STATE(actor).ground_attachment_points[0].x = 0;
        OBJECT_STATE(actor).ground_attachment_points[0].y = 0;
        return;
    case GROUND_EFFECT_AIMED_DOME:
        SET_EFFECT_RADIUS(actor, GROUND_EFFECT_AIMED_DOME_MIN_RADIUS);
        OBJECT_STATE(actor).ground_attachment_points[0].x = 0;
        OBJECT_STATE(actor).ground_attachment_points[0].y = 0;
        return;
    }
}

/**
 * @brief Look up the radius range of a ground effect kind.
 * @param kind Effect kind (GroundEffectKind); other values leave both outputs untouched.
 * @param min_radius Receives the radius at which the effect intensity is zero.
 * @param max_radius Receives the radius at which the effect stops growing.
 * @note Declared inline: field_draw_object_ground_effect expands it in place.
 */
inline void field_get_ground_effect_radius_limits(s32 kind, s32* min_radius, s32* max_radius)
{
    switch (kind)
    {
    case GROUND_EFFECT_DOME:
        *min_radius = GROUND_EFFECT_DOME_MIN_RADIUS;
        *max_radius = GROUND_EFFECT_DOME_MAX_RADIUS;
        break;
    case GROUND_EFFECT_STRIPS:
        *min_radius = GROUND_EFFECT_STRIPS_MIN_RADIUS;
        *max_radius = GROUND_EFFECT_STRIPS_MAX_RADIUS;
        break;
    case GROUND_EFFECT_SPIRAL:
        *min_radius = GROUND_EFFECT_SPIRAL_MIN_RADIUS;
        *max_radius = GROUND_EFFECT_SPIRAL_MAX_RADIUS;
        break;
    case GROUND_EFFECT_CURVES:
        *min_radius = GROUND_EFFECT_CURVES_MIN_RADIUS;
        *max_radius = GROUND_EFFECT_CURVES_MAX_RADIUS;
        break;
    case GROUND_EFFECT_SCATTERED_DOMES:
        *min_radius = GROUND_EFFECT_SCATTERED_DOMES_MIN_RADIUS;
        *max_radius = GROUND_EFFECT_SCATTERED_DOMES_MAX_RADIUS;
        break;
    case GROUND_EFFECT_DRIFTING_DOME:
        *min_radius = GROUND_EFFECT_DRIFTING_DOME_MIN_RADIUS;
        *max_radius = GROUND_EFFECT_DRIFTING_DOME_MAX_RADIUS;
        break;
    case GROUND_EFFECT_AIMED_DOME:
        *min_radius = GROUND_EFFECT_AIMED_DOME_MIN_RADIUS;
        *max_radius = GROUND_EFFECT_AIMED_DOME_MAX_RADIUS;
        break;
    }
}

/**
 * @brief Draw an object's ground effect and advance its radius and angle.
 * @param actor Actor supplying the position, facing and object index.
 * @param kind Effect kind (GroundEffectKind).
 * @note The aimed dome moves with the d-pad or left stick of the object's
 *       controller, but not past the screen edge.
 */
void field_draw_object_ground_effect(FieldMotionRecord* actor, u32 kind)
{
    struct
    {
        VECTOR point;
        VECTOR direction;
        VECTOR target;
    } work;
    DVECTOR screen;
    VECTOR* position;
    ControllerState* controller = CONTROLLER_STATE;
    s32 intensity;
    s32 facing;
    s32 min_radius;
    s32 max_radius;
    s32 radius;
    s32 buttons;
    s32 raw_buttons;
    u16 held;
    s32 i;
    s32 draw;
    u8* packet;
    u_long* ordering_table;

    ordering_table = &g_field_render_half->ordering_table[FIELD_WORLD_OT_OFFSET];
    packet = g_field_render_half->primitive_cursor;
    field_get_ground_effect_radius_limits(kind, &min_radius, &max_radius);
    radius = EFFECT_RADIUS(actor);
    intensity = ((radius - min_radius) << 8) / (max_radius - min_radius);
    facing = actor->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED;
    OBJECT_STATE(actor).action_charge = intensity;
    /* x, y, z and the following word read as one VECTOR. */
    position = (VECTOR*)&actor->x;

    if (OBJECT_STATE(actor).action_charge > FIELD_ACTION_CHARGE_FULL)
    {
        OBJECT_STATE(actor).action_charge = FIELD_ACTION_CHARGE_FULL;
    }

    draw = 1;

    if (actor->source_object_index == FIELD_PARTY_COMPANION)
    {
        if ((g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].info.word & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_GOLEM)
        {
            draw = 0;
        }
    }

    g_field_effect_angle = OBJECT_STATE(actor).effect_angle;

    switch (kind)
    {
    case GROUND_EFFECT_DOME:
        if (draw != 0)
        {
            packet = field_build_effect_dome(ordering_table, packet, position, radius);
        }

        OBJECT_STATE(actor).effect_angle -= EFFECT_SPIN_STEP;

        if (EFFECT_RADIUS(actor) < max_radius)
        {
            SET_EFFECT_RADIUS(actor, EFFECT_RADIUS(actor) + 2);
        }

        break;
    case GROUND_EFFECT_STRIPS:
        if (draw != 0)
        {
            packet = field_build_effect_strips(ordering_table, packet, position, radius);
        }

        OBJECT_STATE(actor).effect_angle -= EFFECT_SPIN_STEP;

        if (EFFECT_RADIUS(actor) < max_radius)
        {
            SET_EFFECT_RADIUS(actor, EFFECT_RADIUS(actor) + 2);
        }

        break;
    case GROUND_EFFECT_SPIRAL:
        if (draw != 0)
        {
            packet = field_build_effect_spiral(ordering_table, packet, position, radius, facing);
        }

        /* For the strip effects the angle is an animation phase. */
        OBJECT_STATE(actor).effect_angle += EFFECT_PHASE_STEP;

        if (OBJECT_STATE(actor).effect_angle >= EFFECT_PHASE_COUNT)
        {
            OBJECT_STATE(actor).effect_angle = 0;
        }

        if (EFFECT_RADIUS(actor) < max_radius)
        {
            SET_EFFECT_RADIUS(actor, EFFECT_RADIUS(actor) + 1);
        }

        break;
    case GROUND_EFFECT_CURVES:
        if (OBJECT_STATE(actor).effect_angle < 0)
        {
            OBJECT_STATE(actor).effect_angle = 0;
        }

        if (draw != 0)
        {
            packet = field_build_effect_curves(ordering_table, field_build_effect_curves(ordering_table, packet, position, radius, 0), position, radius, 1);
        }

        OBJECT_STATE(actor).effect_angle += EFFECT_PHASE_STEP;

        if (OBJECT_STATE(actor).effect_angle >= EFFECT_PHASE_COUNT)
        {
            OBJECT_STATE(actor).effect_angle = 0;
        }

        if (EFFECT_RADIUS(actor) < max_radius)
        {
            SET_EFFECT_RADIUS(actor, EFFECT_RADIUS(actor) + 2);
        }

        break;
    case GROUND_EFFECT_SCATTERED_DOMES:
        for (i = 0; i < SCATTER_POINT_COUNT; i++)
        {
            work.point.vx = actor->x + (OBJECT_STATE(actor).ground_attachment_points[i].x << 8);
            work.point.vy = actor->y;
            work.point.vz = actor->z + (OBJECT_STATE(actor).ground_attachment_points[i].y << 8);

            if (draw != 0)
            {
                packet = field_build_effect_dome(ordering_table, packet, &work.point, radius);
            }
        }

        if (EFFECT_RADIUS(actor) < max_radius)
        {
            SET_EFFECT_RADIUS(actor, EFFECT_RADIUS(actor) + 2);
        }

        OBJECT_STATE(actor).effect_angle -= EFFECT_SPIN_STEP;
        break;
    case GROUND_EFFECT_DRIFTING_DOME:
        work.point.vx = actor->x + (OBJECT_STATE(actor).ground_attachment_points[0].x << 8);
        work.point.vy = actor->y;
        work.point.vz = actor->z + (OBJECT_STATE(actor).ground_attachment_points[0].y << 8);

        if (draw != 0)
        {
            packet = field_build_effect_dome(ordering_table, packet, &work.point, radius);
        }

        OBJECT_STATE(actor).effect_angle -= EFFECT_SPIN_STEP;

        if (EFFECT_RADIUS(actor) < max_radius)
        {
            SET_EFFECT_RADIUS(actor, EFFECT_RADIUS(actor) + 2);

            if (actor->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
            {
                OBJECT_STATE(actor).ground_attachment_points[0].x += 2;
            }
            else
            {
                OBJECT_STATE(actor).ground_attachment_points[0].x -= 2;
            }
        }

        break;
    case GROUND_EFFECT_AIMED_DOME:
        work.point.vx = actor->x + (OBJECT_STATE(actor).ground_attachment_points[0].x << 8);
        work.point.vy = actor->y;
        work.point.vz = actor->z + (OBJECT_STATE(actor).ground_attachment_points[0].y << 8);

        if (draw != 0)
        {
            packet = field_build_effect_dome(ordering_table, packet, &work.point, radius);
        }

        OBJECT_STATE(actor).effect_angle -= EFFECT_SPIN_STEP;

        if (EFFECT_RADIUS(actor) < max_radius)
        {
            SET_EFFECT_RADIUS(actor, EFFECT_RADIUS(actor) + 1);
        }

        if (actor->source_object_index < FIELD_PLAYER_COUNT)
        {
            if (controller->ports[actor->source_object_index].published_sample.device_type >= CONTROLLER_DEVICE_CONFIGURING)
            {
                raw_buttons = 0;
            }
            else
            {
                /* The driver stores the two button bytes swapped. */
                held = controller->ports[actor->source_object_index].published_sample.held_buttons;
                raw_buttons = (held << 8) | (held >> 8);
            }
#if defined(VERSION_JP)
            buttons = raw_buttons;
#else
            buttons = PAD_REMAP_FACE_BITS_16(raw_buttons);
#endif
            work.point.vz = 0;
            work.point.vy = 0;
            work.point.vx = 0;

            if (buttons & PAD_BTN_RIGHT)
            {
                work.point.vx = ONE;
            }

            if (buttons & PAD_BTN_LEFT)
            {
                work.point.vx -= ONE;
            }

            if (buttons & PAD_BTN_DOWN)
            {
                work.point.vy = -ONE;
            }

            if (buttons & PAD_BTN_UP)
            {
                work.point.vy += ONE;
            }

            if (controller->ports[actor->source_object_index].published_sample.device_type != CONTROLLER_DEVICE_DIGITAL)
            {
                work.point.vx += controller->ports[actor->source_object_index].published_sample.left_stick_x * 16;
                work.point.vy -= controller->ports[actor->source_object_index].published_sample.left_stick_y * 16;
            }

            if ((work.point.vx | work.point.vy) != 0)
            {
                /* work.direction = unit direction; the dome moves one unit per 0x400 of it. */
                VectorNormal(&work.point, &work.direction);
                work.target.vx = actor->x + ((OBJECT_STATE(actor).ground_attachment_points[0].x + (work.direction.vx >> 10)) << 8);
                work.target.vy = actor->y;
                work.target.vz = actor->z + ((OBJECT_STATE(actor).ground_attachment_points[0].y + (work.direction.vy >> 10)) << 8);
                screen.vx = FIELD_SCREEN_CENTER_X + g_field_view_offset_x / 256 + work.target.vx / 256;
                screen.vy = FIELD_SCREEN_CENTER_Y + g_field_view_offset_y / 256 + work.target.vy / 256 - work.target.vz / 512 - g_field_view_offset_z / 512;

                if ((screen.vx > 0 || work.direction.vx > 0) && (screen.vy > 0 || work.direction.vy < 0) &&
                    (screen.vx < SCREEN_WIDTH || work.direction.vx < 0) && (screen.vy < VRAM_DRAW_HEIGHT || work.direction.vy > 0))
                {
                    OBJECT_STATE(actor).ground_attachment_points[0].x += work.direction.vx >> 10;
                    OBJECT_STATE(actor).ground_attachment_points[0].y += work.direction.vy >> 10;
                }
            }
        }

        break;
    }

    g_field_render_half->primitive_cursor = packet;
}

/**
 * @brief Append a rotating dome of arched quads around a position.
 * @param ordering_table Ordering table with FIELD_WORLD_OT_SIZE depth buckets.
 * @param packet Next free primitive-buffer byte.
 * @param position World-space center in fixed-point coordinates.
 * @param radius Dome radius.
 * @return First free byte after the appended primitives.
 * @note The dome is rotated by the effect angle in g_field_effect_angle.
 */
static u8* field_build_effect_dome(u_long* ordering_table, u8* packet, VECTOR* position, s32 radius)
{
    VECTOR v[6];
    SVECTOR rot;
    MATRIX m0;
    MATRIX m1;
    s32 i;
    s32 x;
    s32 y;
    s32 initial_x;
    s32 initial_y;
    u32 center;
    u8* cursor;

    cursor = packet;

    rot.vx = 0;
    rot.vz = 0;
    rot.vy = g_field_effect_angle;
    RotMatrix_gte(&rot, &m0);

    initial_x = (rcos(0) >> 4) * radius;
    initial_y = (rsin(0) >> 4) * radius;

    rot.vx = 0;
    rot.vz = 0;
    rot.vy = g_field_effect_angle + 0x180;
    RotMatrix_gte(&rot, &m1);

    v[0].vx = initial_x;
    v[0].vy = initial_y;
    v[0].vz = 0;
    ApplyMatrixLV(&m0, &v[0], &v[1]);
    v[0].vx = initial_x;
    v[0].vy = initial_y;
    v[0].vz = 0;
    ApplyMatrixLV(&m1, &v[0], &v[4]);

    /* The first four quads share the center vertex; one per quarter turn. */
    v[0].vx = position->vx + v[1].vx;
    v[0].vy = position->vy + v[1].vy;
    v[0].vz = position->vz + v[1].vz;
    PROJECT_POINT((POLY_G4*)cursor, 0, v[0]);

    v[0].vx = position->vx - v[1].vx;
    v[0].vy = position->vy + v[1].vy;
    v[0].vz = position->vz - v[1].vz;
    PROJECT_POINT((POLY_G4*)cursor + 1, 0, v[0]);

    v[0].vx = position->vx - v[1].vz;
    v[0].vy = position->vy + v[1].vy;
    v[0].vz = position->vz + v[1].vx;
    PROJECT_POINT((POLY_G4*)cursor + 2, 0, v[0]);

    v[0].vx = position->vx + v[1].vz;
    v[0].vy = position->vy + v[1].vy;
    v[0].vz = position->vz - v[1].vx;
    PROJECT_POINT((POLY_G4*)cursor + 3, 0, v[0]);

    v[0].vx = position->vx;
    v[0].vy = position->vy;
    v[0].vz = position->vz;
    PROJECT_POINT((POLY_G4*)cursor, 1, v[0]);
    center = *(s32*)&((POLY_G4*)cursor)->x1;
    *(s32*)&((POLY_G4*)cursor)->x3 = center;
    *(s32*)&((POLY_G4*)cursor + 3)->x1 = center;
    *(s32*)&((POLY_G4*)cursor + 3)->x3 = center;
    *(s32*)&((POLY_G4*)cursor + 2)->x1 = center;
    *(s32*)&((POLY_G4*)cursor + 2)->x3 = center;
    *(s32*)&((POLY_G4*)cursor + 1)->x1 = center;
    *(s32*)&((POLY_G4*)cursor + 1)->x3 = center;

    v[0].vx = position->vx + v[4].vx;
    v[0].vy = position->vy + v[4].vy;
    v[0].vz = position->vz + v[4].vz;
    PROJECT_POINT((POLY_G4*)cursor, 2, v[0]);

    v[0].vx = position->vx - v[4].vx;
    v[0].vy = position->vy + v[4].vy;
    v[0].vz = position->vz - v[4].vz;
    PROJECT_POINT((POLY_G4*)cursor + 1, 2, v[0]);

    v[0].vx = position->vx - v[4].vz;
    v[0].vy = position->vy + v[4].vy;
    v[0].vz = position->vz + v[4].vx;
    PROJECT_POINT((POLY_G4*)cursor + 2, 2, v[0]);

    v[0].vx = position->vx + v[4].vz;
    v[0].vy = position->vy + v[4].vy;
    v[0].vz = position->vz - v[4].vx;
    PROJECT_POINT((POLY_G4*)cursor + 3, 2, v[0]);

    SET_BGR0_PACKED((POLY_G4*)cursor, EFFECT_BLUE(EFFECT_LEVEL - (v[1].vz >> 8)));
    SET_BGR0_PACKED((POLY_G4*)cursor + 1, EFFECT_GREEN((v[1].vz >> 8) + EFFECT_LEVEL));
    SET_BGR0_PACKED((POLY_G4*)cursor + 2, EFFECT_GREEN(EFFECT_LEVEL - (v[1].vx >> 8)));
    SET_BGR0_PACKED((POLY_G4*)cursor + 3, EFFECT_GREEN((v[1].vx >> 8) + EFFECT_LEVEL));

    SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor + 3, EFFECT_GREEN(EFFECT_LEVEL));
    SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor + 2, EFFECT_GREEN(EFFECT_LEVEL));
    SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor + 1, EFFECT_GREEN(EFFECT_LEVEL));
    SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor, EFFECT_GREEN(EFFECT_LEVEL));
    SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor + 1, 0);
    SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor + 1, 0);
    SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor + 2, 0);
    SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor + 2, 0);
    SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor + 3, 0);
    SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor + 3, 0);
    SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor, 0);
    SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor, 0);

    SetPolyG4((POLY_G4*)cursor);
    SetPolyG4((POLY_G4*)cursor + 1);
    SetPolyG4((POLY_G4*)cursor + 2);
    SetPolyG4((POLY_G4*)cursor + 3);
    setSemiTrans((POLY_G4*)cursor, 1);
    setSemiTrans((POLY_G4*)cursor + 1, 1);
    setSemiTrans((POLY_G4*)cursor + 2, 1);
    setSemiTrans((POLY_G4*)cursor + 3, 1);

    ADD_PRIM_BY_DEPTH(position->vz >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);
    ADD_PRIM_BY_DEPTH(position->vz >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);
    ADD_PRIM_BY_DEPTH(position->vz >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);
    ADD_PRIM_BY_DEPTH(position->vz >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);

    setDrawTPage((DR_TPAGE*)cursor, 0, 0, BLEND_ADD_TPAGE);
    ADD_PRIM_BY_DEPTH((position->vz + v[1].vz) >> FIELD_WORLD_OT_DEPTH_SHIFT, DR_TPAGE);

    /* Eight arch segments of 22.5 degrees, drawn for two opposite quarters. */
    for (i = 1; i <= STRIP_SEGMENT_COUNT; i++)
    {
        x = (rcos(i << 8) >> 4) * radius;
        y = -(rsin(i << 8) >> 4) * radius;
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[2]);
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m1, &v[0], &v[5]);

        v[0].vx = position->vx + v[1].vx;
        v[0].vy = position->vy + v[1].vy;
        v[0].vz = position->vz + v[1].vz;
        PROJECT_POINT((POLY_G4*)cursor, 0, v[0]);
        v[0].vx = position->vx + v[2].vx;
        v[0].vy = position->vy + v[2].vy;
        v[0].vz = position->vz + v[2].vz;
        PROJECT_POINT((POLY_G4*)cursor, 1, v[0]);
        v[0].vx = position->vx + v[4].vx;
        v[0].vy = position->vy + v[4].vy;
        v[0].vz = position->vz + v[4].vz;
        PROJECT_POINT((POLY_G4*)cursor, 2, v[0]);
        v[0].vx = position->vx + v[5].vx;
        v[0].vy = position->vy + v[5].vy;
        v[0].vz = position->vz + v[5].vz;
        PROJECT_POINT((POLY_G4*)cursor, 3, v[0]);

        SET_BGR0_PACKED((POLY_G4*)cursor, EFFECT_BLUE(EFFECT_LEVEL - (v[1].vz >> 8)));
        SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor, EFFECT_BLUE(EFFECT_LEVEL - (v[2].vz >> 8)));
        SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor, 0);
        SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor, 0);
        SetPolyG4((POLY_G4*)cursor);
        setSemiTrans((POLY_G4*)cursor, 1);
        ADD_PRIM_BY_DEPTH((position->vz + v[1].vz) >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);

        setDrawTPage((DR_TPAGE*)cursor, 0, 0, BLEND_ADD_TPAGE);
        ADD_PRIM_BY_DEPTH((position->vz + v[1].vz) >> FIELD_WORLD_OT_DEPTH_SHIFT, DR_TPAGE);

        v[0].vx = position->vx - v[1].vz;
        v[0].vy = position->vy + v[1].vy;
        v[0].vz = position->vz + v[1].vx;
        PROJECT_POINT((POLY_G4*)cursor, 0, v[0]);
        v[0].vx = position->vx - v[2].vz;
        v[0].vy = position->vy + v[2].vy;
        v[0].vz = position->vz + v[2].vx;
        PROJECT_POINT((POLY_G4*)cursor, 1, v[0]);
        v[0].vx = position->vx - v[4].vz;
        v[0].vy = position->vy + v[4].vy;
        v[0].vz = position->vz + v[4].vx;
        PROJECT_POINT((POLY_G4*)cursor, 2, v[0]);
        v[0].vx = position->vx - v[5].vz;
        v[0].vy = position->vy + v[5].vy;
        v[0].vz = position->vz + v[5].vx;
        PROJECT_POINT((POLY_G4*)cursor, 3, v[0]);

        SET_BGR0_PACKED((POLY_G4*)cursor, EFFECT_BLUE(EFFECT_LEVEL - (v[1].vz >> 8)));
        SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor, EFFECT_BLUE(EFFECT_LEVEL - (v[2].vz >> 8)));
        SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor, 0);
        SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor, 0);
        SetPolyG4((POLY_G4*)cursor);
        setSemiTrans((POLY_G4*)cursor, 1);
        ADD_PRIM_BY_DEPTH((position->vz + v[1].vz) >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);

        setDrawTPage((DR_TPAGE*)cursor, 0, 0, BLEND_ADD_TPAGE);
        ADD_PRIM_BY_DEPTH((position->vz + v[1].vz) >> FIELD_WORLD_OT_DEPTH_SHIFT, DR_TPAGE);

        v[1].vx = v[2].vx;
        v[1].vy = v[2].vy;
        v[1].vz = v[2].vz;
        v[4].vx = v[5].vx;
        v[4].vy = v[5].vy;
        v[4].vz = v[5].vz;
    }

    return cursor;
}

/**
 * @brief Append four rotating strips of arched quads.
 * @param ordering_table Ordering table with FIELD_WORLD_OT_SIZE depth buckets.
 * @param packet Next free primitive-buffer byte.
 * @param position World-space center in fixed-point coordinates.
 * @param radius Radius used to construct the strips.
 * @return First free byte after the appended primitives.
 */
static u8* field_build_effect_strips(u_long* ordering_table, u8* packet, VECTOR* position, s32 radius)
{
    VECTOR v[6];
    SVECTOR rot;
    MATRIX m0;
    MATRIX unused_matrix;
    VECTOR p0;
    VECTOR p1;
    s32 distance;
    s32 i;
    s32 j;
    s32 x;
    s32 y;
    s32 inner_x;
    s32 inner_y;
    s32 angle;
    u8* cursor;

    cursor = packet;

    for (i = 0; i < STRIP_COUNT; i++)
    {
        /* Each strip stands between two points of a circle, a sixteenth of a turn apart. */
        distance = (radius >> 1) + 64;
        p0.vx = position->vx;
        p0.vy = position->vy;
        p0.vz = position->vz;
        p1.vx = position->vx;
        p1.vy = position->vy;
        p1.vz = position->vz;
        angle = i << 10;
        x = (rcos(angle - g_field_effect_angle) >> 4) * distance;
        y = (rsin(angle - g_field_effect_angle) >> 4) * distance;
        p0.vx += x;
        p0.vz += y;
        x = (rcos(angle - g_field_effect_angle - 0x100) >> 4) * distance;
        y = (rsin(angle - g_field_effect_angle - 0x100) >> 4) * distance;
        p1.vx += x;
        p1.vz += y;
        rot.vx = 0;
        rot.vz = 0;
        rot.vy = g_field_effect_angle + angle;
        RotMatrix_gte(&rot, &m0);
        x = ((rcos(0) >> 4) * radius) >> 1;
        y = ((rsin(0) >> 4) * radius) >> 1;
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[1]);
        v[0].vx = -radius * 0x80;
        v[0].vy = 0;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[2]);
        v[0].vx = p0.vx + v[1].vx;
        v[0].vy = p0.vy + v[1].vy;
        v[0].vz = p0.vz + v[1].vz;
        PROJECT_POINT((POLY_G4*)cursor, 0, v[0]);
        v[0].vx = p0.vx + v[2].vx;
        v[0].vy = p0.vy + v[2].vy;
        v[0].vz = p0.vz + v[2].vz;
        PROJECT_POINT((POLY_G4*)cursor, 1, v[0]);
        v[0].vx = p1.vx + v[1].vx;
        v[0].vy = p1.vy + v[1].vy;
        v[0].vz = p1.vz + v[1].vz;
        PROJECT_POINT((POLY_G4*)cursor, 2, v[0]);
        v[0].vx = p1.vx + v[2].vx;
        v[0].vy = p1.vy + v[2].vy;
        v[0].vz = p1.vz + v[2].vz;
        PROJECT_POINT((POLY_G4*)cursor, 3, v[0]);
        SET_BGR0_PACKED((POLY_G4*)cursor, EFFECT_GREEN(EFFECT_LEVEL - (v[1].vz >> 8)));
        SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor, EFFECT_GREEN(EFFECT_LEVEL - (v[2].vz >> 8)));
        SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor, 0);
        SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor, 0);
        SetPolyG4((POLY_G4*)cursor);
        setSemiTrans((POLY_G4*)cursor, 1);
        ADD_PRIM_BY_DEPTH((p0.vz + v[1].vz) >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);
        setDrawTPage((DR_TPAGE*)cursor, 0, 0, BLEND_ADD_TPAGE);
        ADD_PRIM_BY_DEPTH((p0.vz + v[1].vz) >> FIELD_WORLD_OT_DEPTH_SHIFT, DR_TPAGE);

        for (j = 1; j <= STRIP_SEGMENT_COUNT; j++)
        {
            inner_x = ((rcos(j << 8) >> 4) * radius) >> 1;
            inner_y = (-(rsin(j << 8) >> 4) * radius) >> 1;
            v[0].vx = inner_x;
            v[0].vy = inner_y;
            v[0].vz = 0;
            ApplyMatrixLV(&m0, &v[0], &v[2]);
            v[0].vx = p0.vx + v[1].vx;
            v[0].vy = p0.vy + v[1].vy;
            v[0].vz = p0.vz + v[1].vz;
            PROJECT_POINT((POLY_G4*)cursor, 0, v[0]);
            v[0].vx = p0.vx + v[2].vx;
            v[0].vy = p0.vy + v[2].vy;
            v[0].vz = p0.vz + v[2].vz;
            PROJECT_POINT((POLY_G4*)cursor, 1, v[0]);
            v[0].vx = p1.vx + v[1].vx;
            v[0].vy = p1.vy + v[1].vy;
            v[0].vz = p1.vz + v[1].vz;
            PROJECT_POINT((POLY_G4*)cursor, 2, v[0]);
            v[0].vx = p1.vx + v[2].vx;
            v[0].vy = p1.vy + v[2].vy;
            v[0].vz = p1.vz + v[2].vz;
            PROJECT_POINT((POLY_G4*)cursor, 3, v[0]);
            SET_BGR0_PACKED((POLY_G4*)cursor, EFFECT_BLUE(EFFECT_LEVEL - (v[1].vz >> 8)));
            SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor, EFFECT_BLUE(EFFECT_LEVEL - (v[2].vz >> 8)));
            SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor, 0);
            SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor, 0);
            SetPolyG4((POLY_G4*)cursor);
            setSemiTrans((POLY_G4*)cursor, 1);
            ADD_PRIM_BY_DEPTH((p0.vz + v[1].vz) >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);
            setDrawTPage((DR_TPAGE*)cursor, 0, 0, BLEND_ADD_TPAGE);
            ADD_PRIM_BY_DEPTH((p0.vz + v[1].vz) >> FIELD_WORLD_OT_DEPTH_SHIFT, DR_TPAGE);
            v[1].vx = v[2].vx;
            v[1].vy = v[2].vy;
            v[1].vz = v[2].vz;
        }
    }

    return cursor;
}

/**
 * @brief Draw animated curved quad strips running out along X from a position.
 * @param ordering_table Ordering table with FIELD_WORLD_OT_SIZE depth buckets.
 * @param packet Destination for the generated GPU packets.
 * @param position World position in signed fixed-point coordinates.
 * @param extent Maximum horizontal extent, tested after each completed strip.
 * @param forward Nonzero runs toward positive X; zero toward negative X.
 * @return First byte after the emitted primitives and draw-page command.
 * @note Emits at least one strip and at most four, with nine quads per strip.
 */
static u8* field_build_effect_curves(u_long* ordering_table, u8* packet, VECTOR* position, s32 extent, s32 forward)
{
    FieldStripWorkspace work;
    s32 step;
    s32 strip_index;
    s32 angle;
    s32 offset;
    s32 first_xy;
    s32 second_xy;
    u8* cursor;

    cursor = packet;
    strip_index = 0;
    /* The strips scroll outward by the phase and repeat every two strip widths. */
    offset = (g_field_effect_angle % 40) << 8;
    do
    {
        if (forward != 0)
        {
            work.world.vx = position->vx + offset;
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        }
        else
        {
            work.world.vx = position->vx - offset;
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        }

        PROJECT_POINT((POLY_G4*)cursor, 0, work.world);
        /* Keep both starting vertices to close the strip after eight steps. */
        first_xy = *(s32*)&((POLY_G4*)cursor)->x0;

        if (forward != 0)
        {
            work.world.vx = position->vx + offset + CURVE_STRIP_WIDTH;
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        }
        else
        {
            work.world.vx = position->vx - offset - CURVE_STRIP_WIDTH;
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        }

        PROJECT_POINT((POLY_G4*)cursor, 1, work.world);
        second_xy = *(s32*)&((POLY_G4*)cursor)->x1;

        for (step = 1, angle = 0x100; step <= STRIP_SEGMENT_COUNT; step++, angle += 0x100)
        {
            if (forward != 0)
            {
                work.world.vx = position->vx + offset + angle;
                work.world.vy = position->vy - rsin(angle) * 2;
                work.world.vz = position->vz + rcos(angle) * 2;
            }
            else
            {
                work.world.vx = position->vx - offset - angle;
                work.world.vy = position->vy - rsin(angle) * 2;
                work.world.vz = position->vz + rcos(angle) * 2;
            }

            PROJECT_POINT((POLY_G4*)cursor, 2, work.world);
            *(s32*)&((POLY_G4*)cursor + 1)->x0 = *(s32*)&((POLY_G4*)cursor)->x2;

            if (forward != 0)
            {
                work.world.vx = position->vx + offset + angle + CURVE_STRIP_WIDTH;
                work.world.vy = position->vy - rsin(angle) * 2;
                work.world.vz = position->vz + rcos(angle) * 2;
            }
            else
            {
                work.world.vx = position->vx - offset - angle - CURVE_STRIP_WIDTH;
                work.world.vy = position->vy - rsin(angle) * 2;
                work.world.vz = position->vz + rcos(angle) * 2;
            }

            PROJECT_POINT((POLY_G4*)cursor, 3, work.world);
            SET_BGR0_PACKED((POLY_G4*)cursor, 0);
            SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor, EFFECT_BLUE(EFFECT_LEVEL));
            SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor, 0);
            SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor, EFFECT_BLUE(EFFECT_LEVEL));
            *(s32*)&((POLY_G4*)cursor + 1)->x1 = *(s32*)&((POLY_G4*)cursor)->x3;
            SetPolyG4((POLY_G4*)cursor);
            setSemiTrans((POLY_G4*)cursor, 1);
            ADD_PRIM_BY_DEPTH(position->vz >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);
        }

        *(s32*)&((POLY_G4*)cursor)->x2 = first_xy;
        *(s32*)&((POLY_G4*)cursor)->x3 = second_xy;
        SET_BGR0_PACKED((POLY_G4*)cursor, 0);
        SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor, EFFECT_GREEN(EFFECT_LEVEL));
        SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor, 0);
        SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor, EFFECT_GREEN(EFFECT_LEVEL));
        SetPolyG4((POLY_G4*)cursor);
        setSemiTrans((POLY_G4*)cursor, 1);
        ADD_PRIM_BY_DEPTH(position->vz >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);
        offset += 2 * CURVE_STRIP_WIDTH;
    } while (offset < (extent << 8) && ++strip_index < STRIP_COUNT);
    setDrawTPage((DR_TPAGE*)cursor, 0, 0, BLEND_ADD_TPAGE);
    ADD_PRIM_BY_DEPTH(position->vz >> FIELD_WORLD_OT_DEPTH_SHIFT, DR_TPAGE);
    return cursor;
}

/**
 * @brief Draw four strips of quads wound around a tilted axis.
 * @param ordering_table Ordering table with FIELD_WORLD_OT_SIZE depth buckets.
 * @param packet First free primitive packet.
 * @param position Fixed-point world-space origin of the effect.
 * @param tilt Effect radius; the axis leans by ratan2(tilt, 100).
 * @param facing Selects addition or subtraction of the rotated X offset.
 * @return First free packet following all quads and the draw-page command.
 * @note Each strip turns a half circle around X in eight steps while its
 *       radius grows; the phase in g_field_effect_angle is the radius of the first strip.
 */
static u8* field_build_effect_spiral(u_long* ordering_table, u8* packet, VECTOR* position, s32 tilt, s32 facing)
{
    FieldStripWorkspace work;
    s32 strip_index;
    s32 angle;
    s32 first_xy;
    s32 second_xy;
    MATRIX* matrix;
    s32 segment_index;
    s32 radius;
    s32 radius_sum;
    u8* cursor;

    cursor = packet;
    angle = ratan2(tilt, 100);
    strip_index = 0;
    matrix = &work.rotation.matrix;
    radius = g_field_effect_angle;
    do
    {
        /* Identity rotation with zero translation, written as words. */
        work.rotation.words[4] = ONE;
        work.rotation.words[2] = ONE;
        work.rotation.words[0] = ONE;
        work.rotation.words[7] = 0;
        work.rotation.words[6] = 0;
        work.rotation.words[5] = 0;
        work.rotation.words[3] = 0;
        work.rotation.words[1] = 0;
        RotMatrixY(angle, matrix);
        STRIP_POINT(radius);
        PROJECT_POINT((POLY_G4*)cursor, 0, work.world);
        first_xy = *(s32*)&((POLY_G4*)cursor)->x0;
        STRIP_POINT(radius + SPIRAL_STRIP_WIDTH);
        PROJECT_POINT((POLY_G4*)cursor, 1, work.world);
        second_xy = *(s32*)&((POLY_G4*)cursor)->x1;

        for (segment_index = 1, radius_sum = radius; segment_index <= STRIP_SEGMENT_COUNT; segment_index++, radius_sum += radius)
        {
            RotMatrixX(-0x100, matrix);
            STRIP_POINT(radius + (radius_sum >> 5));
            PROJECT_POINT((POLY_G4*)cursor, 2, work.world);
            *(s32*)&((POLY_G4*)cursor + 1)->x0 = *(s32*)&((POLY_G4*)cursor)->x2;
            STRIP_POINT(radius + (radius_sum >> 5) + SPIRAL_STRIP_WIDTH);
            PROJECT_POINT((POLY_G4*)cursor, 3, work.world);
            SET_BGR0_PACKED((POLY_G4*)cursor, 0);
            SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor, EFFECT_BLUE(EFFECT_LEVEL));
            SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor, 0);
            SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor, EFFECT_BLUE(EFFECT_LEVEL));
            *(s32*)&((POLY_G4*)cursor + 1)->x1 = *(s32*)&((POLY_G4*)cursor)->x3;
            SetPolyG4((POLY_G4*)cursor);
            setSemiTrans((POLY_G4*)cursor, 1);
            ADD_PRIM_BY_DEPTH(position->vz >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);
        }

        *(s32*)&((POLY_G4*)cursor)->x2 = first_xy;
        *(s32*)&((POLY_G4*)cursor)->x3 = second_xy;
        SET_BGR0_PACKED((POLY_G4*)cursor, 0);
        SET_POLY_G4_BGR1_PACKED((POLY_G4*)cursor, EFFECT_GREEN(EFFECT_LEVEL));
        SET_POLY_G4_BGR2_PACKED((POLY_G4*)cursor, 0);
        SET_POLY_G4_BGR3_PACKED((POLY_G4*)cursor, EFFECT_GREEN(EFFECT_LEVEL));
        SetPolyG4((POLY_G4*)cursor);
        setSemiTrans((POLY_G4*)cursor, 1);
        ADD_PRIM_BY_DEPTH(position->vz >> FIELD_WORLD_OT_DEPTH_SHIFT, POLY_G4);
        radius += SPIRAL_RADIUS_STEP;

        if (radius >= SPIRAL_RADIUS_LIMIT)
        {
            radius -= SPIRAL_RADIUS_LIMIT;
        }

        strip_index++;
    } while (strip_index < STRIP_COUNT);
    setDrawTPage((DR_TPAGE*)cursor, 0, 0, BLEND_ADD_TPAGE);
    ADD_PRIM_BY_DEPTH(position->vz >> FIELD_WORLD_OT_DEPTH_SHIFT, DR_TPAGE);
    return cursor;
}
