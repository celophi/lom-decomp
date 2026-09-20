/** @file field_effect_transform.c
 * @brief Evaluate effect transforms and generate projected effect geometry.
 */

#include "common.h"
#include "field_effect_transform.h"
#include "field_effect_types.h"
#include "field_effect_render_state.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/rand.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define FIELD_RIBBON_MAX_SEGMENTS 20
#define FIELD_RIBBON_FRAME_COUNT 12
#define FIELD_RIBBON_UV_VARIANT 0x01
#define FIELD_RIBBON_FLIP_U 0x80
#define FIELD_RIBBON_FLIP_V 0x40
#define FIELD_EFFECT_OT_SIZE 4096
#define FIELD_EFFECT_OT_DEPTH_SHIFT 7
#define FIELD_EFFECT_CENTER_X 160
#define FIELD_EFFECT_CENTER_Y 112
#define FIELD_ANGLE_QUARTER_TURN (ONE / 4)
#define FIELD_ANGLE_HALF_TURN (ONE / 2)
#define FIELD_ANGLE_MASK (ONE - 1)
#define FIELD_TRACK_INDEX_MASK 0xF
#define FIELD_EFFECT_TRACK_COLOR 0x00008000
#define FIELD_EFFECT_LITERAL_COLOR 0x10000000
#define FIELD_PART_REVERSE_ROTATION_WITH_FACING 0x00020000
#define FIELD_PART_SCALE_DISTANCE_BY_TARGET 0x01000000
#define FIELD_PART_SCALE_DISTANCE_BY_OWNER 0x01000000
#define FIELD_PART_SCALE_Z_BY_OWNER 0x02000000
#define FIELD_PART_SCALE_X_BY_OWNER 0x04000000
#define FIELD_RIBBON_SCRATCH ((FieldRibbonScratch*)0x1F800000)
#define FIELD_MATRIX_SCRATCH ((FieldMatrixScratch*)0x1F8000C0)
#define FIELD_SPRITE_SCRATCH ((FieldSpriteScratch*)0x1F800100)

#define FIELD_GPU_ADDRESS_MASK 0x00FFFFFF
#define FIELD_GPU_LENGTH_MASK 0xFF000000
#define FIELD_TRACK_COUNT 16
#define FIELD_EFFECT_TARGET_PITCH (ONE / 12)
#define FIELD_EFFECT_DEFAULT_PITCH (ONE / 16)
#define FIELD_PART_SCALE_TRACK_SHIFT 7
#define FIELD_PART_ROTATION_MODE_SHIFT 6
#define FIELD_PART_ROTATION_TRACK_SHIFT 26
#define FIELD_PART_ROTATE_QUARTER_X_SHIFT 17
#define FIELD_PART_PROJECT_SCALE_SHIFT 23
#define FIELD_PART_ORIENTED_SHIFT 3
#define FIELD_PART_AIM_AT_TARGET_SHIFT 11
#define FIELD_PART_STRETCH_TO_TARGET_SHIFT 2
#define FIELD_PART_RGB_TRACKS_SHIFT 12
#define FIELD_PART_RANDOM_EXTENT_SHIFT 15
#define FIELD_PART_ADD_HALF_WIDTH_SHIFT 16
#define FIELD_PART_ADD_HALF_HEIGHT_SHIFT 17
#define FIELD_PART_ATTACHMENT_SHIFT 18
#define FIELD_PART_COLOR_TRACK_SHIFT 5
#define FIELD_PART_RED_TRACK 0x4
#define FIELD_PART_GREEN_TRACK 0x2
#define FIELD_PART_BLUE_TRACK 0x1
#define FIELD_SPRITE_TILT_BYTE 8
#define FIELD_ATTACHMENT_MASK 0x3F
#define FIELD_ATTACHMENT_GROUP_COUNT 8
#define FIELD_ATTACHMENT_LINKED_BOUNDS_COUNT 28
#define FIELD_PART_OWNER_BOUNDS 2

/** @brief Attachment category boundaries encoded in part placement flags. */
typedef enum
{
    FIELD_ATTACHMENT_LINKED_BOUNDS_FIRST = 10,
    FIELD_ATTACHMENT_POINT_FIRST = 20,
    FIELD_ATTACHMENT_POINT_SECOND_BIAS = 34,
    FIELD_ATTACHMENT_POINT_SECOND = 42,
    FIELD_ATTACHMENT_UNRESOLVED_FIRST = 55
} FieldEffectAttachmentCategory;

/** @brief Ribbon path vectors followed by one rotation matrix per segment. */
typedef struct
{
    FieldVector endpoint;
    FieldVector position;
    FieldVector midpoint;
    FieldVector half_delta;
    FieldVector delta;
    SVECTOR direction;
    FieldMatrix rotations[FIELD_RIBBON_MAX_SEGMENTS];
} FieldRibbonScratch;

/** @brief Temporary rotation and distance calculations for a part matrix. */
typedef struct
{
    SVECTOR direction;
    FieldVector scale;
    FieldVector delta;
    FieldVector squared_delta;
} FieldMatrixScratch;

/** @brief GTE output with unsigned low halfwords for GPU vertex additions. */
typedef union
{
    FieldVector vector;
    struct
    {
        u16 x;
        u16 x_high;
        u16 y;
        u16 y_high;
        u16 z;
        u16 z_high;
        u16 pad[2];
    } low;
} FieldSpriteProjection;

/** @brief Scratchpad vectors and local tilt matrix for sprite projection. */
typedef struct
{
    SVECTOR rotated_vertex;
    SVECTOR vertex;
    FieldSpriteProjection projected;
    MATRIX rotation;
} FieldSpriteScratch;

extern FieldMotionRecord g_field_actors[];
extern FieldObjectPlacement D_80105AE0[];
extern FieldActorState g_field_actor_slots[80];
extern s32 g_field_track_index;

s32 field_evaluate_parameter_track(FieldActorState* actor, s32 track);
s32 field_evaluate_parameter_track_at_time(FieldActorState* actor, u32 track, u16 time);

/**
 * @brief Render a textured ribbon from an effect to its resolved target.
 * @param effect Position, facing, color, age, and segment count source.
 * @param packet_cursor Receives consecutive POLY_FT4 packets.
 * @param ordering_table Depth-indexed GPU ordering table.
 * @return Cursor after the last emitted quad.
 * @note animation_active supplies the segment count for this effect kind.
 * @see decomp.me (99.94%) WIP
 */
u8* field_render_effect_ribbon(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    POLY_FT4* quad = (POLY_FT4*)packet_cursor;
    FieldActorPartDef* part;
    FieldActorState* state;
    FieldVector* position;
    FieldVector* midpoint;
    FieldVector* half_delta;
    FieldVector* delta;
    FieldMatrix* rotation;
    FieldVector* endpoint;
    s32 segment_count;
    s32 i;
    s32 arc_height;
    s32 step;
    s32 angle;
    s32 edge_x;
    SVECTOR* direction;
    s32 depth;
    u8 uv_flags;

    position = &FIELD_RIBBON_SCRATCH->position;
    midpoint = &FIELD_RIBBON_SCRATCH->midpoint;
    half_delta = &FIELD_RIBBON_SCRATCH->half_delta;
    delta = &FIELD_RIBBON_SCRATCH->delta;
    direction = &FIELD_RIBBON_SCRATCH->direction;

    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    state = &g_field_actor_slots[effect->actor_index];
    field_build_effect_part_matrix(effect, part, FIELD_RIBBON_SCRATCH->rotations, state);

    rotation = FIELD_RIBBON_SCRATCH->rotations;
    uv_flags = g_field_ribbon_frame_flags[((u16)effect->age) % FIELD_RIBBON_FRAME_COUNT];
    *(u16*)&quad->u0 = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 0];
    *(u16*)&quad->u1 = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 1];
    *(u16*)&quad->u2 = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 2];
    *(u16*)&quad->u3 = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 3];

    if (uv_flags & FIELD_RIBBON_FLIP_U)
    {
        i = quad->u0;
        quad->u0 = quad->u1;
        quad->u1 = i;
        i = quad->u2;
        quad->u2 = quad->u3;
        quad->u3 = i;
    }
    if (uv_flags & FIELD_RIBBON_FLIP_V)
    {
        i = quad->v0;
        quad->v0 = quad->v2;
        quad->v2 = i;
        i = quad->v1;
        quad->v1 = quad->v3;
        quad->v3 = i;
    }

    {
        u32 behavior_flags = part->behavior_flags;
        quad->clut = getClut(80, 492);
        quad->tpage = (u16)(((behavior_flags >> 17) & 0x60) | getTPage(0, 0, 448, 0));
    }

    gte_SetRotMatrix(rotation);

    {
        s32 view_x;
        s32 screen_x;
        s32 view_y;
        view_x = g_field_view_offset_x / 256;
        screen_x = effect->x / 256 + FIELD_EFFECT_CENTER_X;
        view_y = g_field_view_offset_y;
        quad->x0 = (s16)(view_x + screen_x);
        if (view_y < 0)
        {
            view_y += 255;
        }
        quad->y0 = (s16)(FIELD_EFFECT_CENTER_Y + (view_y >> 8) + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512);
    }

    field_resolve_effect_part_color(state, effect, part, &quad->r0);
    setPolyFT4(quad);
    setSemiTrans(quad, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

    rotation = FIELD_RIBBON_SCRATCH->rotations;
    segment_count = FIELD_RIBBON_MAX_SEGMENTS;
    if (effect->animation_active < FIELD_RIBBON_MAX_SEGMENTS)
    {
        segment_count = effect->animation_active;
    }
    endpoint = &FIELD_RIBBON_SCRATCH->endpoint;
    if (segment_count <= 0)
    {
        segment_count = 1;
    }
    step = FIELD_ANGLE_HALF_TURN / segment_count;

    field_resolve_effect_position(effect, part, &FIELD_RIBBON_SCRATCH->endpoint);
    {
        midpoint->vx = (FIELD_RIBBON_SCRATCH->endpoint.vx + effect->x) >> 1;
        midpoint->vy = FIELD_RIBBON_SCRATCH->endpoint.vy;
        midpoint->vz = (FIELD_RIBBON_SCRATCH->endpoint.vz + effect->z) >> 1;
        delta->vx = endpoint->vx - effect->x;
        half_delta->vx = delta->vx >> 1;
        delta->vy = effect->y - FIELD_RIBBON_SCRATCH->endpoint.vy;
        half_delta->vy = delta->vy;
        i = segment_count - 1;
        delta->vz = FIELD_RIBBON_SCRATCH->endpoint.vz - effect->z;
        half_delta->vz = delta->vz >> 1;
    }

    if (i > 0)
    {
        do
        {
            direction->vx = 0;
            direction->vy = (s16)((rand() << 12) >> 15);
            direction->vz = (s16)((rand() << 12) >> 16);
            RotMatrix_gte(direction, (MATRIX*)rotation);
            i--;
            rotation++;
        } while (i > 0);
    }

    rotation = FIELD_RIBBON_SCRATCH->rotations;

    if ((*(u8*)&part->behavior_flags) >> FIELD_PART_SCALE_TRACK_SHIFT)
    {
        direction->vx = 0;
        direction->vy = (s16)((part->behavior_flags >> 28) << 8);
        direction->vz = 0;
    }
    else
    {
        *(s32*)&direction->vz = 0;
        *(s32*)&direction->vx = 0;
    }

    if (((part->track_flags >> FIELD_PART_ROTATION_MODE_SHIFT) & 3) != 0)
    {
        arc_height = (part->track_flags >> FIELD_PART_ROTATION_TRACK_SHIFT) << 10;
    }
    else
    {
        arc_height = 0;
    }
    arc_height = (arc_height * rand()) >> 15;
    if (rand() & 1)
    {
        arc_height = -arc_height;
    }

    angle = ratan2(delta->vx, delta->vy + (delta->vz >> 1));
    edge_x = (rcos(angle) * part->footprint_scale_y) >> 12;
    angle = (rsin(angle) * part->footprint_scale_y) >> 12;
    if (uv_flags & FIELD_RIBBON_UV_VARIANT)
    {
        angle *= 2;
        edge_x *= 2;
    }

    i = segment_count - 1;
    quad->x2 = quad->x0 + edge_x;
    quad->y2 = quad->y0 + angle;
    quad->y0 -= angle;
    quad->x0 -= edge_x;

    if (i > 0)
    {
        s32 angle_step;
        s32 addr_mask = FIELD_GPU_ADDRESS_MASK;
        s32 tag_mask = FIELD_GPU_LENGTH_MASK;
        angle_step = i * step;
        do
        {
            depth = *(s32*)&quad->r0;
            *(s32*)&quad[1].r0 = depth;
            setPolyFT4(quad);
            setSemiTrans(quad, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

            gte_SetRotMatrix(rotation);
            gte_ldv0(direction);
            gte_rtv0();
            gte_stlvnl(delta);

            if (arc_height != 0)
            {
                position->vy = midpoint->vy + (half_delta->vy * i) / segment_count - ((arc_height * rsin(angle_step)) >> 12) + delta->vy;
            }
            else
            {
                position->vy = midpoint->vy + (half_delta->vy * i) / segment_count + delta->vy;
            }
            position->vx = ((half_delta->vx * rcos(angle_step)) >> 12) + midpoint->vx + delta->vx;
            position->vz = ((half_delta->vz * rcos(angle_step)) >> 12) + midpoint->vz + delta->vz;

            {
                s32 view_x;
                s32 screen_x;
                s32 view_y;
                view_x = g_field_view_offset_x / 256;
                screen_x = position->vx / 256 + FIELD_EFFECT_CENTER_X;
                view_y = g_field_view_offset_y;
                quad->x1 = (s16)(view_x + screen_x);
                if (view_y < 0)
                {
                    view_y += 255;
                }
                quad->y1 = (s16)(FIELD_EFFECT_CENTER_Y + (view_y >> 8) + position->vy / 256 - position->vz / 512 - g_field_view_offset_z / 512);
            }
            quad->x3 = quad->x1 + edge_x;
            quad->y3 = quad->y1 + angle;
            quad->y1 -= angle;
            quad->x1 -= edge_x;

            {
                u16 first_uv;
                u16 clut;
                first_uv = *(u16*)&quad->u0;
                *(u16*)&quad[1].u3 = *(u16*)&quad->u3;
                *(u16*)&quad[1].u0 = first_uv;
                quad[1].tpage = quad->tpage;
                *(s32*)&quad[1].x2 = *(s32*)&quad->x3;
                *(s32*)&quad[1].x0 = *(s32*)&quad->x1;
                clut = quad->clut;
                *(u16*)&quad[1].u1 = *(u16*)&quad->u1;
                *(u16*)&quad[1].u2 = *(u16*)&quad->u2;
                quad[1].clut = clut;
            }

            depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
            if (depth < 0)
            {
                s32 addr = (s32)quad & addr_mask;
                *(s32*)&quad->tag = (*(s32*)&quad->tag & tag_mask) | (ordering_table[0] & addr_mask);
                quad++;
                ordering_table[0] = (ordering_table[0] & tag_mask) | addr;
            }
            else if (depth >= FIELD_EFFECT_OT_SIZE)
            {
                s32 addr = (s32)quad & addr_mask;
                *(s32*)&quad->tag = (*(s32*)&quad->tag & tag_mask) | (ordering_table[FIELD_EFFECT_OT_SIZE - 1] & addr_mask);
                quad++;
                ordering_table[FIELD_EFFECT_OT_SIZE - 1] = (ordering_table[FIELD_EFFECT_OT_SIZE - 1] & tag_mask) | addr;
            }
            else
            {
                s32 addr;
                s32* entry;
                addr = (s32)quad & addr_mask;
                *(s32*)&quad->tag = (*(s32*)&quad->tag & tag_mask) | (ordering_table[depth] & addr_mask);
                entry = (s32*)(((s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
                quad++;
                *entry = (*entry & tag_mask) | addr;
            }
            angle_step -= step;
            i--;
            rotation++;
        } while (i > 0);
    }

    setPolyFT4(quad);
    setSemiTrans(quad, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

    {
        s32 view_x;
        s32 screen_x;
        s32 view_y;
        view_x = g_field_view_offset_x / 256;
        screen_x = endpoint->vx / 256 + FIELD_EFFECT_CENTER_X;
        view_y = g_field_view_offset_y;
        quad->x1 = (s16)(view_x + screen_x);
        if (view_y < 0)
        {
            view_y += 255;
        }
        {
            s32 depth;
            view_y = FIELD_EFFECT_CENTER_Y + (view_y >> 8) + endpoint->vy / 256;
            depth = endpoint->vz;
            view_y -= depth / 512;
            quad->y1 = view_y - g_field_view_offset_z / 512;
        }
    }
    quad->x3 = quad->x1 + edge_x;
    quad->y3 = quad->y1 + angle;
    quad->y1 -= angle;
    quad->x1 -= edge_x;

    depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
    if (depth < 0)
    {
        s32 addr = (s32)quad & FIELD_GPU_ADDRESS_MASK;
        setaddr(quad, getaddr(&ordering_table[0]));
        quad++;
        ordering_table[0] = (ordering_table[0] & FIELD_GPU_LENGTH_MASK) | addr;
    }
    else if (depth >= FIELD_EFFECT_OT_SIZE)
    {
        s32 addr = (s32)quad & FIELD_GPU_ADDRESS_MASK;
        setaddr(quad, getaddr(&ordering_table[FIELD_EFFECT_OT_SIZE - 1]));
        quad++;
        ordering_table[FIELD_EFFECT_OT_SIZE - 1] = (ordering_table[FIELD_EFFECT_OT_SIZE - 1] & FIELD_GPU_LENGTH_MASK) | addr;
    }
    else
    {
        s32 addr;
        s32* entry;
        s32 srcval;
        addr = (s32)quad & FIELD_GPU_ADDRESS_MASK;
        srcval = ordering_table[depth];
        setaddr(quad, srcval);
        entry = (s32*)(((s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
        quad++;
        *entry = (*entry & FIELD_GPU_LENGTH_MASK) | addr;
    }
    return (u8*)quad;
}

/**
 * @brief Build an effect part's rotation, target alignment, and animated scale.
 * @param effect Position, orientation, and track sampling age.
 * @param part Rotation, alignment, and scale selectors.
 * @param matrix Receives the resulting transform.
 * @param actor Owner supplying parameter tracks and object bindings.
 * @return Unspecified; callers use only the matrix output.
 * @see decomp.me (100%)
 */
s32 field_build_effect_part_matrix(FieldMotionRecord* effect, FieldActorPartDef* part, FieldMatrix* matrix, FieldActorState* actor)
{
    SVECTOR* direction = &FIELD_MATRIX_SCRATCH->direction;
    FieldVector* scale = &FIELD_MATRIX_SCRATCH->scale;
    FieldVector* delta = &FIELD_MATRIX_SCRATCH->delta;
    FieldVector* squared_delta = &FIELD_MATRIX_SCRATCH->squared_delta;
    s32 angle;
    s32 final_angle;
    s32 axis;
    s32 scale_mode;
    u32 distance;
    s32 rotation_mode;
    s32 target_component;
    s32 next_scale_track;
    u32 flags;

    /* FieldMatrix and FieldVector share the SDK layout; packed stores include the matrix pad. */
    *(u32*)&matrix->m[2][2] = ONE;
    *(u32*)&matrix->m[1][1] = ONE;
    *(u32*)&matrix->m[0][0] = ONE;
    matrix->t[2] = 0;
    matrix->t[1] = 0;
    matrix->t[0] = 0;
    *(u32*)&matrix->m[2][0] = 0;
    *(u32*)&matrix->m[0][2] = 0;

    flags = part->track_flags;
    rotation_mode = (flags >> FIELD_PART_ROTATION_MODE_SHIFT) & 3;
    if (rotation_mode != 0)
    {
        switch (rotation_mode)
        {
        case 1:
            angle = field_evaluate_parameter_track_at_time(actor, flags >> 26, (u16)effect->age) << 4;
            axis = ((u8*)&part->track_flags)[3];
            axis &= 3;
            break;
        case 2:
            angle = effect->heading;
            axis = 1;
            break;
        case 3:
            angle = (((flags >> FIELD_PART_ROTATION_TRACK_SHIFT) * (u16)effect->age) << 4) & FIELD_ANGLE_MASK;
            axis = flags >> 24;
            axis &= 3;
            break;
        default:
            break;
        }

        if (part->spawn_flags.word & FIELD_PART_REVERSE_ROTATION_WITH_FACING)
        {
            if (!(g_field_actors[actor->owner_object_index].facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
            {
                angle = -angle;
            }
        }

        switch (axis)
        {
        case 0:
            RotMatrixX(angle, (MATRIX*)matrix);
            break;
        case 1:
            RotMatrixY(angle, (MATRIX*)matrix);
            break;
        case 2:
            RotMatrixZ(angle, (MATRIX*)matrix);
            break;
        case 3:
            RotMatrixZ(angle, (MATRIX*)matrix);
            RotMatrixY(angle, (MATRIX*)matrix);
            RotMatrixX(angle, (MATRIX*)matrix);
            break;
        }
    }

    if ((part->track_flags >> FIELD_PART_ROTATE_QUARTER_X_SHIFT) & 1)
    {
        RotMatrixX(FIELD_ANGLE_QUARTER_TURN, (MATRIX*)matrix);
    }
    if (part->rotation_y_16 != 0)
    {
        RotMatrixY(part->rotation_y_16 << 4, (MATRIX*)matrix);
    }

    if ((part->behavior_flags >> FIELD_PART_ORIENTED_SHIFT) & 1)
    {
        RotMatrixZ(FIELD_ANGLE_QUARTER_TURN, (MATRIX*)matrix);
        RotMatrixY(FIELD_ANGLE_QUARTER_TURN, (MATRIX*)matrix);
        if ((part->placement_flags >> FIELD_PART_AIM_AT_TARGET_SHIFT) & 1)
        {
            field_resolve_effect_position(effect, part, scale);
            target_component = scale->vz;
            angle = ratan2(effect->z - target_component, scale->vx - effect->x);
            delta->vx = (scale->vx - effect->x) >> 8;
            delta->vy = (scale->vy - effect->y) >> 8;
            delta->vz = (scale->vz - effect->z) >> 8;
            gte_ldlvl(delta);
            gte_sqr0();
            gte_stlvnl(squared_delta);
            distance = SquareRoot0(squared_delta->vx + squared_delta->vz + squared_delta->vy);
            target_component = scale->vy;
            RotMatrixZ(ratan2(distance << 8, effect->y - target_component), (MATRIX*)matrix);
            RotMatrixY(angle, (MATRIX*)matrix);
            final_angle = FIELD_EFFECT_TARGET_PITCH;
        }
        else
        {
            RotMatrixZ(effect->pitch, (MATRIX*)matrix);
            RotMatrixY(effect->heading, (MATRIX*)matrix);
            RotMatrixZ(effect->rotation_z_16 << 4, (MATRIX*)matrix);
            RotMatrixY(effect->rotation_y_16 << 4, (MATRIX*)matrix);
            final_angle = FIELD_EFFECT_DEFAULT_PITCH;
        }
    }
    else
    {
        RotMatrixY(FIELD_ANGLE_QUARTER_TURN, (MATRIX*)matrix);
        final_angle = FIELD_ANGLE_QUARTER_TURN;
    }
    RotMatrixX(final_angle, (MATRIX*)matrix);

    if ((part->placement_flags >> FIELD_PART_STRETCH_TO_TARGET_SHIFT) & 1)
    {
        field_resolve_effect_position(effect, part, scale);
        delta->vx = (scale->vx - effect->x) >> 8;
        delta->vy = (scale->vy - effect->y) >> 8;
        delta->vz = (scale->vz - effect->z) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(squared_delta);
        distance = SquareRoot0(squared_delta->vx + squared_delta->vy + squared_delta->vz);
        if (distance == 0)
        {
            distance = 1;
        }
        scale->vy = ONE;
        scale->vz = ONE;
        if (effect->sprite_height_minus_one != 0)
        {
            scale->vx = (distance << 12) / effect->sprite_height_minus_one;
        }
        else
        {
            scale->vx = (distance << 12) >> 6;
        }
        ScaleMatrix((MATRIX*)matrix, (VECTOR*)scale);
    }

    if ((part->track_flags >> FIELD_PART_PROJECT_SCALE_SHIFT) & 1)
    {
        if ((part->behavior_flags >> FIELD_PART_ORIENTED_SHIFT) & 1)
        {
            direction->vx = -ONE;
            direction->vy = 0;
            direction->vz = 0;
            gte_SetRotMatrix(matrix);
            gte_ldv0(direction);
            gte_rtv0();
            gte_stlvnl(scale);
            direction->vx = 0;
            direction->vy = FIELD_ANGLE_QUARTER_TURN;
            direction->vz = FIELD_ANGLE_QUARTER_TURN;
            RotMatrix_gte(direction, (MATRIX*)matrix);
            RotMatrixZ(ratan2(scale->vy, scale->vx) + FIELD_ANGLE_QUARTER_TURN, (MATRIX*)matrix);
            gte_ldlvl(scale);
            gte_sqr0();
            gte_stlvnl(squared_delta);
            scale->vx = SquareRoot0(squared_delta->vx + squared_delta->vy);
            scale->vz = ONE;
            scale->vy = ONE;
            ScaleMatrix((MATRIX*)matrix, (VECTOR*)scale);
        }
    }

    scale->vy = ONE;
    if ((*(u32*)&part->unknown_0x1c) & FIELD_PART_SCALE_Z_BY_OWNER)
    {
        scale->vz = (part->footprint_scale_x - ((part->footprint_scale_x * ((s32)(256 - D_80105AE0[actor->owner_object_index].unknown_0x68) >> 6)) / 10)) << 6;
    }
    else
    {
        scale->vz = part->footprint_scale_x << 6;
    }
    if ((*(u32*)&part->unknown_0x1c) & FIELD_PART_SCALE_X_BY_OWNER)
    {
        scale->vx = (part->footprint_scale_y - ((part->footprint_scale_y * ((s32)(256 - D_80105AE0[actor->owner_object_index].unknown_0x68) >> 6)) / 10)) << 6;
    }
    else
    {
        scale->vx = part->footprint_scale_y << 6;
    }
    ScaleMatrix((MATRIX*)matrix, (VECTOR*)scale);

    g_field_effect_track_scale.x = ONE;
    g_field_effect_track_scale.y = ONE;
    g_field_effect_track_scale.z = ONE;

    if ((part->behavior_flags >> FIELD_PART_SCALE_TRACK_SHIFT) & 1)
    {
        scale->vz = ONE;
        scale->vy = ONE;
        scale->vx = ONE;
        flags = part->behavior_flags;
        scale_mode = (flags >> 20) & 3;
        switch (scale_mode)
        {
        case 0:
            scale->vz = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
            break;
        case 1:
            scale->vx = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
            break;
        case 2:
        {
            u32 value = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
            scale->vz = value;
            scale->vy = value;
            scale->vx = value;
            break;
        }
        case 3:
            scale->vz = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
            next_scale_track = (part->behavior_flags >> 28) + 1;
            scale->vx = field_evaluate_parameter_track_at_time(actor, next_scale_track % FIELD_TRACK_COUNT, (u16)effect->age) << 4;
            break;
        }
        g_field_effect_track_scale.x = (u16)scale->vx;
        g_field_effect_track_scale.y = (u16)scale->vy;
        g_field_effect_track_scale.z = (u16)scale->vz;
        ScaleMatrix((MATRIX*)matrix, (VECTOR*)scale);
    }
}

/**
 * @brief Resolve a part's literal or animated primitive color.
 * @param actor Owner supplying parameter tracks.
 * @param effect Color flags, packed literal color, and track sampling age.
 * @param part Literal channel values and track selectors.
 * @param out Four-byte color storage; the literal path also copies its fourth byte.
 * @see decomp.me (100%)
 */
void field_resolve_effect_part_color(FieldActorState* actor, FieldMotionRecord* effect, FieldActorPartDef* part, u8* out)
{
    s32 flags;
    u8 value;

    /* Packed reads include selectors spanning the adjacent byte fields. */
    flags = effect->flags;
    if (!(flags & FIELD_EFFECT_TRACK_COLOR))
    {
        out[0] = ((*(u32*)&part->unknown_0x2c >> FIELD_PART_COLOR_TRACK_SHIFT) & FIELD_PART_RED_TRACK)
                     ? field_evaluate_parameter_track_at_time(actor, part->red_or_track & FIELD_TRACK_INDEX_MASK, (u16)effect->age)
                     : part->red_or_track;
        out[1] = ((*(u32*)&part->unknown_0x2c >> FIELD_PART_COLOR_TRACK_SHIFT) & FIELD_PART_GREEN_TRACK)
                     ? field_evaluate_parameter_track_at_time(actor, part->green_or_track & FIELD_TRACK_INDEX_MASK, (u16)effect->age)
                     : part->green_or_track;
        out[2] = ((*(u32*)&part->unknown_0x2c >> FIELD_PART_COLOR_TRACK_SHIFT) & FIELD_PART_BLUE_TRACK)
                     ? field_evaluate_parameter_track_at_time(actor, part->blue_or_track & FIELD_TRACK_INDEX_MASK, (u16)effect->age)
                     : part->blue_or_track;
        return;
    }

    if (flags & FIELD_EFFECT_LITERAL_COLOR)
    {
        /* The fourth byte travels with RGB and is overwritten by the packet builder. */
        *(u32*)out = *(u32*)&effect->unknown_0x18;
        return;
    }

    {
        u32 part_flags = part->behavior_flags;
        if ((part_flags >> FIELD_PART_RGB_TRACKS_SHIFT) & 1)
        {
            out[0] = field_evaluate_parameter_track_at_time(actor, (part_flags >> FIELD_PART_ADD_HALF_WIDTH_SHIFT) & FIELD_TRACK_INDEX_MASK, (u16)effect->age);
            out[1] = field_evaluate_parameter_track_at_time(actor, (((u16*)&part->behavior_flags)[1] & FIELD_TRACK_INDEX_MASK) + 1, (u16)effect->age);
            out[2] = field_evaluate_parameter_track_at_time(actor, (((u16*)&part->behavior_flags)[1] & FIELD_TRACK_INDEX_MASK) + 2, (u16)effect->age);
            return;
        }

        value = field_evaluate_parameter_track_at_time(actor, (part_flags >> FIELD_PART_ADD_HALF_WIDTH_SHIFT) & FIELD_TRACK_INDEX_MASK, (u16)effect->age);
        out[0] = out[1] = out[2] = value;
    }
}

/**
 * @brief Append the effect's texture-page command at its clamped depth.
 * @param effect Supplies the ordering-table depth.
 * @param part Supplies the semi-transparency blend mode.
 * @param packet_cursor Receives one DR_TPAGE packet.
 * @param ordering_table Depth-indexed GPU ordering table.
 * @return Cursor immediately after the texture-page packet.
 * @see decomp.me (100%)
 */
u8* field_emit_effect_texture_page(FieldMotionRecord* effect, FieldActorPartDef* part, u8* packet_cursor, s32* ordering_table)
{
    s32 index;
    s32* entry;
    s32 srcval;

    setDrawTPage(packet_cursor, 0, 0, getTPage(0, (part->behavior_flags >> 22), 320, 0));

    index = effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
    if (index < 0)
    {
        addPrim(&ordering_table[0], packet_cursor);
        packet_cursor += sizeof(DR_TPAGE);
    }
    else if (index >= FIELD_EFFECT_OT_SIZE)
    {
        addPrim(&ordering_table[FIELD_EFFECT_OT_SIZE - 1], packet_cursor);
        packet_cursor += sizeof(DR_TPAGE);
    }
    else
    {
        srcval = ordering_table[index];
        setaddr(packet_cursor, srcval);
        entry = (s32*)((effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
        setaddr(entry, packet_cursor);
        packet_cursor += sizeof(DR_TPAGE);
    }
    return packet_cursor;
}

/**
 * @brief Project sprite corners, applying optional tilt and facing reversal.
 * @param effect Supplies the horizontal facing flag.
 * @param origin Unsigned screen-space origin components.
 * @param packet POLY_FT4 packet receiving projected coordinates.
 * @param width Sprite width.
 * @param height Sprite height.
 * @param x Horizontal offset from the anchor.
 * @param y Vertical offset from the anchor.
 * @param item Packed sprite record; byte 8 is the tilt in 16 GTE angle units per step.
 * @param matrix Caller rotation matrix, already loaded into the GTE on entry.
 * @see decomp.me (100%)
 */
void field_project_effect_sprite_quad(FieldMotionRecord* effect, u16* origin, u8* packet, s32 width, s32 height, s32 x, s32 y, u8* item, FieldMatrix* matrix)
{
    SVECTOR* rotated_vertex = &FIELD_SPRITE_SCRATCH->rotated_vertex;
    SVECTOR* vertex = &FIELD_SPRITE_SCRATCH->vertex;
    FieldSpriteProjection* out = &FIELD_SPRITE_SCRATCH->projected;
    MATRIX* tilt_matrix = &FIELD_SPRITE_SCRATCH->rotation;
    POLY_FT4* quad = (POLY_FT4*)packet;

    /* The output uses unsigned low halves, including wrapped negative coordinates. */
    if (item[FIELD_SPRITE_TILT_BYTE] == 0)
    {
        rotated_vertex->vx = y;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x0 = origin[0] + out->low.x;
        quad->y0 = origin[1] + out->low.y;

        rotated_vertex->vx = y;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x + width;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x1 = origin[0] + out->low.x;
        quad->y1 = origin[1] + out->low.y;

        rotated_vertex->vx = y + height;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x2 = origin[0] + out->low.x;
        quad->y2 = origin[1] + out->low.y;

        rotated_vertex->vx = y + height;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x + width;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x3 = origin[0] + out->low.x;
        quad->y3 = origin[1] + out->low.y;
        return;
    }

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        *(u32*)&tilt_matrix->m[2][2] = ONE;
        *(u32*)&tilt_matrix->m[1][1] = ONE;
        *(u32*)&tilt_matrix->m[0][0] = ONE;
        tilt_matrix->t[2] = 0;
        tilt_matrix->t[1] = 0;
        tilt_matrix->t[0] = 0;
        *(u32*)&tilt_matrix->m[2][0] = 0;
        *(u32*)&tilt_matrix->m[0][2] = 0;

        vertex->vx = y;
        vertex->vy = 0;
        vertex->vz = x + width;
        gte_SetRotMatrix(tilt_matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x1 = origin[0] + out->low.x;
        quad->y1 = origin[1] + out->low.y;

        vertex->vx = 0;
        vertex->vy = -(item[FIELD_SPRITE_TILT_BYTE] << 4);
        vertex->vz = 0;
        RotMatrix_gte(vertex, tilt_matrix);

        vertex->vx = 0;
        vertex->vy = 0;
        vertex->vz = -width;
        gte_SetRotMatrix(tilt_matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x0 = (u16)quad->x1 + out->low.x;
        quad->y0 = (u16)quad->y1 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = 0;
        gte_SetRotMatrix(tilt_matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x3 = (u16)quad->x1 + out->low.x;
        quad->y3 = (u16)quad->y1 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = -width;
        gte_SetRotMatrix(tilt_matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x2 = (u16)quad->x1 + out->low.x;
        quad->y2 = (u16)quad->y1 + out->low.y;
    }
    else
    {
        *(u32*)&tilt_matrix->m[2][2] = ONE;
        *(u32*)&tilt_matrix->m[1][1] = ONE;
        *(u32*)&tilt_matrix->m[0][0] = ONE;
        tilt_matrix->t[2] = 0;
        tilt_matrix->t[1] = 0;
        tilt_matrix->t[0] = 0;
        *(u32*)&tilt_matrix->m[2][0] = 0;
        *(u32*)&tilt_matrix->m[0][2] = 0;

        vertex->vx = y;
        vertex->vy = 0;
        vertex->vz = x;
        gte_SetRotMatrix(tilt_matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x0 = origin[0] + out->low.x;
        quad->y0 = origin[1] + out->low.y;

        vertex->vx = 0;
        vertex->vy = item[FIELD_SPRITE_TILT_BYTE] << 4;
        vertex->vz = 0;
        RotMatrix_gte(vertex, tilt_matrix);

        vertex->vx = 0;
        vertex->vy = 0;
        vertex->vz = width;
        gte_SetRotMatrix(tilt_matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x1 = (u16)quad->x0 + out->low.x;
        quad->y1 = (u16)quad->y0 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = 0;
        gte_SetRotMatrix(tilt_matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x2 = (u16)quad->x0 + out->low.x;
        quad->y2 = (u16)quad->y0 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = width;
        gte_SetRotMatrix(tilt_matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x3 = (u16)quad->x0 + out->low.x;
        quad->y3 = (u16)quad->y0 + out->low.y;
    }
}

/**
 * @brief Decode signed byte geometry corners and optionally mirror them.
 * @param out Receives four interleaved x/y pairs.
 * @param flip Nonzero to reverse corner order and negate x coordinates.
 * @param item Packed corner record; byte 7 is not a coordinate.
 * @see decomp.me (100%)
 */
void field_unpack_effect_quad_corners8(s16* out, s32 flip, u8* item)
{
    if (flip)
    {
        out[0] = -(s8)item[2];
        out[1] = (s8)item[3];
        out[2] = -(s8)item[0];
        out[3] = (s8)item[1];
        out[4] = -(s8)item[6];
        out[5] = (s8)item[8];
        out[6] = -(s8)item[4];
        out[7] = (s8)item[5];
    }
    else
    {
        out[0] = (s8)item[0];
        out[1] = (s8)item[1];
        out[2] = (s8)item[2];
        out[3] = (s8)item[3];
        out[4] = (s8)item[4];
        out[5] = (s8)item[5];
        out[6] = (s8)item[6];
        out[7] = (s8)item[8];
    }
}

/**
 * @brief Decode little-endian geometry corners and optionally mirror them.
 * @param out Receives four interleaved x/y pairs.
 * @param mirror Nonzero to reverse corner order and negate x coordinates.
 * @param item Packed corner record; byte 7 interrupts the second y component.
 * @see decomp.me (100%)
 */
void field_unpack_effect_quad_corners16(s16* out, s32 mirror, u8* item)
{
    if (mirror != 0)
    {
        out[0] = -(item[4] + (item[5] << 8));
        out[1] = item[6] + (item[8] << 8);
        out[2] = -(item[0] + (item[1] << 8));
        out[3] = item[2] + (item[3] << 8);
        out[4] = -(item[13] + (item[14] << 8));
        out[5] = item[15] + (item[16] << 8);
        out[6] = -(item[9] + (item[10] << 8));
        out[7] = item[11] + (item[12] << 8);
    }
    else
    {
        out[0] = item[0] + (item[1] << 8);
        out[1] = item[2] + (item[3] << 8);
        out[2] = item[4] + (item[5] << 8);
        out[3] = item[6] + (item[8] << 8);
        out[4] = item[9] + (item[10] << 8);
        out[5] = item[11] + (item[12] << 8);
        out[6] = item[13] + (item[14] << 8);
        out[7] = item[15] + (item[16] << 8);
    }
}

/**
 * @brief Resolve a part's extent from tracks, distance, randomness, and bounds.
 * @param actor Owner supplying tracks and linked object indices.
 * @param part Packed extent value, modifiers, and attachment category.
 * @return Extent after scaling and optional bounding-box half-extents.
 * @note TODO: categories 55-62 do not initialize the part index on this path.
 * @see decomp.me (100%)
 */
s32 field_resolve_effect_extent(FieldActorState* actor, FieldActorPartDef* part)
{
    FieldVector delta;
    FieldVector squared_delta;
    s32 extent;
    s32 part_index;
    s32 half_extent;
    u32 flags;
    FieldActorState* owner;

    {
        u32 value_flags = *(u32*)&part->unknown_0x20;
        s32 value_mode = (value_flags >> FIELD_PART_ROTATION_MODE_SHIFT) & 3;
        owner = actor;
        switch (value_mode)
        {
        case 0:
        {
            s32 value_low_bits = (u32)(*(u32*)&part->unknown_0x1c) >> 29;
            extent = ((value_flags & 0x3F) << 3) | value_low_bits;
            break;
        }
        case 1:
        {
            u32 value_low_bits = (*(u32*)&part->unknown_0x1c) >> 29;
            extent = field_evaluate_parameter_track(owner, (((value_flags & 0x3F) << 3) | value_low_bits) & FIELD_TRACK_INDEX_MASK);
            break;
        }
        case 2:
        {
            u32 value_low_bits = (*(u32*)&part->unknown_0x1c);
            extent = field_evaluate_parameter_track_at_time(owner, (((value_flags & 0x3F) << 3) | (value_low_bits >> 29)) & FIELD_TRACK_INDEX_MASK, 0);
            break;
        }
        }
    }

    if (part->effect_flags & FIELD_PART_SCALE_DISTANCE_BY_TARGET)
    {
        delta.vx = (g_field_actors[owner->track_object_indices[g_field_track_index]].x - g_field_actors[owner->owner_object_index].x) >> 8;
        delta.vy = (g_field_actors[owner->track_object_indices[g_field_track_index]].y - g_field_actors[owner->owner_object_index].y) >> 8;
        delta.vz = (g_field_actors[owner->track_object_indices[g_field_track_index]].z - g_field_actors[owner->owner_object_index].z) >> 8;
        gte_ldlvl(&delta);
        gte_sqr0();
        gte_stlvnl(&squared_delta);
        extent = (SquareRoot0(squared_delta.vx + squared_delta.vy + squared_delta.vz) * extent) / 100;
    }

    if ((part->placement_flags >> FIELD_PART_RANDOM_EXTENT_SHIFT) & 1)
    {
        extent = (extent * rand()) >> 15;
    }

    if ((*(u32*)&part->unknown_0x1c) & FIELD_PART_SCALE_DISTANCE_BY_OWNER)
    {
        extent = ((D_80105AE0[owner->owner_object_index].scale_percent & 0x3FF) * extent) / 100;
    }

    flags = part->placement_flags;
    {
        s32 category = (flags >> FIELD_PART_ATTACHMENT_SHIFT) & FIELD_ATTACHMENT_MASK;
        if (category < FIELD_ATTACHMENT_POINT_FIRST)
        {
            if ((flags >> FIELD_PART_ADD_HALF_WIDTH_SHIFT) & 1)
            {
                if ((u32)(category - FIELD_ATTACHMENT_LINKED_BOUNDS_FIRST) >= FIELD_ATTACHMENT_LINKED_BOUNDS_COUNT)
                {
                    half_extent = (D_80105AE0[owner->owner_object_index].bounds_right - D_80105AE0[owner->owner_object_index].bounds_left) >> 1;
                }
                else
                {
                    half_extent = (D_80105AE0[owner->track_object_indices[g_field_track_index]].bounds_right -
                                   D_80105AE0[owner->track_object_indices[g_field_track_index]].bounds_left) >>
                                  1;
                }
                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }
                extent += half_extent;
            }
            if ((part->placement_flags >> FIELD_PART_ADD_HALF_HEIGHT_SHIFT) & 1)
            {
                if ((u32)(((part->placement_flags >> FIELD_PART_ATTACHMENT_SHIFT) & FIELD_ATTACHMENT_MASK) - FIELD_ATTACHMENT_LINKED_BOUNDS_FIRST) >=
                    FIELD_ATTACHMENT_LINKED_BOUNDS_COUNT)
                {
                    half_extent = (D_80105AE0[owner->owner_object_index].bounds_bottom - D_80105AE0[owner->owner_object_index].bounds_top) >> 1;
                }
                else
                {
                    half_extent = (D_80105AE0[owner->track_object_indices[g_field_track_index]].bounds_bottom -
                                   D_80105AE0[owner->track_object_indices[g_field_track_index]].bounds_top) >>
                                  1;
                }
                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }
                extent += half_extent;
            }
        }
    }

    {
        s32 category = (part->placement_flags >> FIELD_PART_ATTACHMENT_SHIFT) & FIELD_ATTACHMENT_MASK;
        if ((u32)(category - FIELD_ATTACHMENT_POINT_SECOND) < FIELD_ATTACHMENT_GROUP_COUNT ||
            (u32)(category - FIELD_ATTACHMENT_POINT_FIRST) < FIELD_ATTACHMENT_GROUP_COUNT ||
            (u32)(category - FIELD_ATTACHMENT_UNRESOLVED_FIRST) < FIELD_ATTACHMENT_GROUP_COUNT)
        {
            s32 attachment_category = (part->placement_flags >> FIELD_PART_ATTACHMENT_SHIFT) & FIELD_ATTACHMENT_MASK;
            if (attachment_category < FIELD_ATTACHMENT_UNRESOLVED_FIRST)
            {
                part_index = attachment_category - FIELD_ATTACHMENT_POINT_FIRST;
                if (attachment_category >= FIELD_ATTACHMENT_POINT_SECOND)
                {
                    part_index = attachment_category - FIELD_ATTACHMENT_POINT_SECOND_BIAS;
                }
            }

            if ((((u16*)&part->placement_flags)[1]) & 1)
            {
                if (owner->parts[part_index].unknown_0xb == FIELD_PART_OWNER_BOUNDS)
                {
                    half_extent = (D_80105AE0[owner->owner_object_index].bounds_right - D_80105AE0[owner->owner_object_index].bounds_left) >> 1;
                }
                else
                {
                    half_extent = (D_80105AE0[owner->track_object_indices[g_field_track_index]].bounds_right -
                                   D_80105AE0[owner->track_object_indices[g_field_track_index]].bounds_left) >>
                                  1;
                }
                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }
                extent += half_extent;
            }

            if ((part->placement_flags >> FIELD_PART_ADD_HALF_HEIGHT_SHIFT) & 1)
            {
                if (owner->parts[part_index].unknown_0xb == FIELD_PART_OWNER_BOUNDS)
                {
                    half_extent = (D_80105AE0[owner->owner_object_index].bounds_bottom - D_80105AE0[owner->owner_object_index].bounds_top) >> 1;
                }
                else
                {
                    half_extent = (D_80105AE0[owner->track_object_indices[g_field_track_index]].bounds_bottom -
                                   D_80105AE0[owner->track_object_indices[g_field_track_index]].bounds_top) >>
                                  1;
                }
                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }
                extent += half_extent;
            }
        }
    }

    return extent;
}
