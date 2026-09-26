/** @file field_mesh_transform.c
 * @brief Transform mesh vertices, normals, and actor-part matrices.
 */

#include "common.h"
#include "field_effect_types.h"
#include "field_object_state.h"
#include "field_mesh.h"
#include "field_mesh_transform.h"
#include "sdk/libgte.h"
#include "sdk/libetc.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

extern s32 g_field_track_index;

/* Local view: the definition in field_actor_runtime.c takes the animation data it reads. */
u32 field_evaluate_parameter_track_at_time(FieldActorState *actor, s32 track, s32 time);

/** @brief FieldActorPartDef track_flags fields of mesh morphing. */
#define FIELD_PART_MESH_MORPH(flags) (((flags) >> 18) & 1)
#define FIELD_PART_MESH_MORPH_TRACK(flags) (((flags) >> 2) & 15)
#define FIELD_PART_MESH_MORPH_TARGET(flags) (((flags) >> 19) & 3)
/** @brief Morph target value that selects per-face offsets instead of another mesh. */
#define FIELD_MESH_MORPH_OFFSETS 3
/** @brief FieldActorPartDef palette_extent bits: scale the part by the owner's footprint strength. */
#define FIELD_PART_FOOTPRINT_SCALES_XZ 0x02000000
#define FIELD_PART_FOOTPRINT_SCALES_Y 0x04000000

/**
 * @brief Transform an actor part's triangle vertices into screen-coordinate buffers.
 *
 * Depending on the part flags, blend with another mesh, apply a shared offset
 * to each triangle, or transform the original vertices directly. The loaded
 * GTE matrix supplies the transform; fixed scratchpad vectors hold the results.
 *
 * @param actor Owner of the mesh table and parameter tracks.
 * @param record Actor record supplying the current parameter-track time.
 * @param part Flags selecting the interpolation or offset mode.
 * @param index Mesh entry to transform.
 */
void field_transform_mesh_vertices(FieldActorState *actor, FieldMotionRecord *record, FieldActorPartDef *part,
                   s32 index)
{
    s32 flags; /* GTE FLAG register, stored after every transform and never read */
    VECTOR *transformed = (VECTOR *)getScratchAddr(16);
    SVECTOR *work = (SVECTOR *)getScratchAddr(0);
    s16 *out = g_field_mesh_screen_vertices;
    s32 *depth = g_field_mesh_depth_offsets;
    SVECTOR *src = FIELD_ACTOR_MESH(actor, index)->vertices;
    SVECTOR *other;
    s32 count;
    s32 amount;
    s32 mode;
    s32 dx, dy, dz;
    u32 part_flags = part->track_flags.word;
    if (FIELD_PART_MESH_MORPH(part_flags))
    {
        amount = field_evaluate_parameter_track_at_time(actor, FIELD_PART_MESH_MORPH_TRACK(part_flags), (u16)record->age);
        mode = FIELD_PART_MESH_MORPH_TARGET(part->track_flags.word);
        if (mode < FIELD_MESH_MORPH_OFFSETS)
        {
            /* Interpolate corresponding vertices with a 12-bit fractional weight. */
            count = FIELD_ACTOR_MESH(actor, index)->face_count;
            other = FIELD_ACTOR_MESH(actor, mode)->vertices;
            while (count != 0)
            {
                work[0].vx = (u16)src[0].vx + (((other[0].vx - src[0].vx) * amount) >> 12);
                work[0].vy = (u16)src[0].vy + (((other[0].vy - src[0].vy) * amount) >> 12);
                work[0].vz = (u16)src[0].vz + (((other[0].vz - src[0].vz) * amount) >> 12);
                work[1].vx = (u16)src[1].vx + (((other[1].vx - src[1].vx) * amount) >> 12);
                work[1].vy = (u16)src[1].vy + (((other[1].vy - src[1].vy) * amount) >> 12);
                work[1].vz = (u16)src[1].vz + (((other[1].vz - src[1].vz) * amount) >> 12);
                work[2].vx = (u16)src[2].vx + (((other[2].vx - src[2].vx) * amount) >> 12);
                work[2].vy = (u16)src[2].vy + (((other[2].vy - src[2].vy) * amount) >> 12);
                work[2].vz = (u16)src[2].vz + (((other[2].vz - src[2].vz) * amount) >> 12);
                gte_ldv0(&work[0]);
                gte_rtv0tr();
                gte_stlvnl(&transformed[0]);
                gte_stflg(&flags);
                out[0] = (u16)transformed[0].vx * 2;
                out[1] = (u16)transformed[0].vz + ((u16)transformed[0].vy * 2);
                gte_ldv0(&work[1]);
                gte_rtv0tr();
                gte_stlvnl(&transformed[1]);
                gte_stflg(&flags);
                out[2] = (u16)transformed[1].vx * 2;
                out[3] = (u16)transformed[1].vz + ((u16)transformed[1].vy * 2);
                gte_ldv0(&work[2]);
                gte_rtv0tr();
                gte_stlvnl(&transformed[2]);
                gte_stflg(&flags);
                out[4] = (u16)transformed[2].vx * 2;
                out[5] = (u16)transformed[2].vz + ((u16)transformed[2].vy * 2);
                count--;
                src += 3;
                other += 3;
                out += 6;
                *depth = -(transformed[0].vz * 2);
                depth++;
            }
        }
        else
        {
            count = FIELD_ACTOR_MESH(actor, index)->face_count;
            /* The offset mode uses one displacement vector per triangle. */
            other = FIELD_ACTOR_MESH(actor, index)->offsets;
            while (count != 0)
            {
                dx = (other->vx * amount) >> 8;
                dy = (other->vy * amount) >> 8;
                dz = (other->vz * amount) >> 8;
                work[0].vx = src[0].vx + dx;
                work[0].vy = src[0].vy + dy;
                work[0].vz = src[0].vz + dz;
                work[1].vx = src[1].vx + dx;
                work[1].vy = src[1].vy + dy;
                work[1].vz = src[1].vz + dz;
                work[2].vx = src[2].vx + dx;
                work[2].vy = src[2].vy + dy;
                work[2].vz = src[2].vz + dz;
                gte_ldv0(&work[0]);
                gte_rtv0tr();
                gte_stlvnl(&transformed[0]);
                gte_stflg(&flags);
                out[0] = (u16)transformed[0].vx * 2;
                out[1] = (u16)transformed[0].vz + ((u16)transformed[0].vy * 2);
                gte_ldv0(&work[1]);
                gte_rtv0tr();
                gte_stlvnl(&transformed[1]);
                gte_stflg(&flags);
                out[2] = (u16)transformed[1].vx * 2;
                out[3] = (u16)transformed[1].vz + ((u16)transformed[1].vy * 2);
                gte_ldv0(&work[2]);
                gte_rtv0tr();
                gte_stlvnl(&transformed[2]);
                gte_stflg(&flags);
                out[4] = (u16)transformed[2].vx * 2;
                out[5] = (u16)transformed[2].vz + ((u16)transformed[2].vy * 2);
                count--;
                other++;
                src += 3;
                out += 6;
                *depth = -(transformed[0].vz * 2);
                depth++;
            }
        }
    }
    else
    {
        count = FIELD_ACTOR_MESH(actor, index)->face_count;
        while (count != 0)
        {
            gte_ldv0(&src[0]);
            gte_rtv0tr();
            gte_stlvnl(&transformed[0]);
            gte_stflg(&flags);
            out[0] = (u16)transformed[0].vx * 2;
            out[1] = (u16)transformed[0].vz + ((u16)transformed[0].vy * 2);
            gte_ldv0(&src[1]);
            gte_rtv0tr();
            gte_stlvnl(&transformed[0]);
            gte_stflg(&flags);
            out[2] = (u16)transformed[0].vx * 2;
            out[3] = (u16)transformed[0].vz + ((u16)transformed[0].vy * 2);
            gte_ldv0(&src[2]);
            gte_rtv0tr();
            gte_stlvnl(&transformed[0]);
            gte_stflg(&flags);
            out[4] = (u16)transformed[0].vx * 2;
            out[5] = (u16)transformed[0].vz + ((u16)transformed[0].vy * 2);
            count--;
            out += 6;
            src += 3;
            *depth = -(transformed[0].vz * 2);
            depth++;
        }
    }
}


/**
 * @brief Rotate mesh normals, optionally blending toward another normal set.
 * @param actor Actor supplying the normal sets and animation tracks.
 * @param record Actor instance supplying the current animation time.
 * @param part Part flags selecting interpolation and its track/normal set.
 * @param index Source mesh normal-set index.
 * @param matrix Rotation matrix applied through the GTE.
 * @note The interpolation branch writes output slots from count down through one.
 */
void field_transform_mesh_normals(FieldActorState *actor, FieldMotionRecord *record, FieldActorPartDef *part, s32 index, MATRIX *matrix)
{
    SVECTOR *output;
    SVECTOR *source;
    SVECTOR *scratch = (SVECTOR *)getScratchAddr(0);
    s32 factor;
    s32 selected;
    s32 count;

    output = g_field_mesh_transformed_normals;
    source = FIELD_ACTOR_MESH(actor, index)->normals;
    if (FIELD_PART_MESH_MORPH(part->track_flags.word))
    {
        factor = field_evaluate_parameter_track_at_time(actor, FIELD_PART_MESH_MORPH_TRACK(part->track_flags.word), (u16)record->age);
        selected = FIELD_PART_MESH_MORPH_TARGET(part->track_flags.word);
        if (selected < FIELD_MESH_MORPH_OFFSETS)
        {
            count = FIELD_ACTOR_MESH(actor, index)->face_count;
            /* output walks the blend-target normals here; results go by count. */
            output = FIELD_ACTOR_MESH(actor, selected)->normals;
            while (count != 0)
            {
                scratch->vx = (u16)source->vx + (((output->vx - source->vx) * factor) >> 12);
                scratch->vy = (u16)source->vy + (((output->vy - source->vy) * factor) >> 12);
                scratch->vz = (u16)source->vz + (((output->vz - source->vz) * factor) >> 12);
                gte_SetRotMatrix(matrix);
                gte_ldv0(scratch);
                gte_rtv0();
                gte_stsv(&g_field_mesh_transformed_normals[count]);
                count--;
                source++;
                output++;
            }
        }
        else
        {
            count = FIELD_ACTOR_MESH(actor, index)->face_count;
            while (count != 0)
            {
                gte_SetRotMatrix(matrix);
                gte_ldv0(source);
                gte_rtv0();
                gte_stsv(output);
                count--;
                source++;
                output++;
            }
        }
    }
    else
    {
        count = FIELD_ACTOR_MESH(actor, index)->face_count;
        while (count != 0)
        {
            gte_SetRotMatrix(matrix);
            gte_ldv0(source);
            gte_rtv0();
            gte_stsv(output);
            count--;
            source++;
            output++;
        }
    }
}


/**
 * @brief Build the transformed and scaled matrix for an actor part.
 *
 * Applies track-driven rotation, optional facing and distance transforms,
 * the supplied base matrix, and the part's fixed and animated scale factors.
 * The three work vectors occupy the original GTE scratchpad addresses.
 *
 * @param actor Owner whose parameter tracks and slot state are consulted.
 * @param record Actor position, orientation, and current track time.
 * @param part Part descriptor specifying rotation and scale behavior.
 * @param matrix Destination matrix initialized and updated by this function.
 * @param base_matrix Matrix passed to the base-transform composition helper.
 * @return Nothing meaningful; callers ignore it.
 * @note Declared int without a return statement; as void it compiles differently.
 */
s32 field_build_part_matrix(FieldActorState *actor, FieldMotionRecord *record, FieldActorPartDef *part,
                   MATRIX *matrix, MATRIX *base_matrix)
{
    VECTOR *scale;
    VECTOR *delta = (VECTOR *)getScratchAddr(4);
    VECTOR *square = (VECTOR *)getScratchAddr(8);
    s32 axis;
    s32 facing_angle;
    s32 rotation;
    s32 horizontal_scale;
    s32 base_scale;
    s32 track_scale_xz;
    s32 track_scale_xyz;
    s32 track_scale_pair;
    s32 component;
    s32 distance_xz;
    s32 scale_mode;
    s32 distance;
    u32 rotation_flags;
    u32 scale_flags;
    u32 track_index;
    u8 scale_xz;
    u8 scale_y;

    /* Identity rotation and zero translation, written as words. */
    ((s32 *)matrix)[4] = ONE;
    ((s32 *)matrix)[2] = ONE;
    /* The loop notes keep the ONE constant after the prologue; without them it is scheduled one slot earlier. */
    do
    {
        ((s32 *)matrix)[0] = ONE;
    } while (0);
    scale = (VECTOR *)getScratchAddr(0);
    ((s32 *)matrix)[7] = 0;
    ((s32 *)matrix)[6] = 0;
    ((s32 *)matrix)[5] = 0;
    ((s32 *)matrix)[3] = 0;
    ((s32 *)matrix)[1] = 0;
    rotation_flags = part->track_flags.word;
    component = (rotation_flags >> 6) & 3;
    if (component != 0)
    {
        switch (component)
        {
        case 1:
            rotation =
                field_evaluate_parameter_track_at_time(actor, rotation_flags >> 26, (u16)record->age)
                << 4;
            axis = part->track_flags.bytes.high;
            axis &= 3;
            break;
        case 2:
            RotMatrixX(ONE / 4, matrix);
            /* Pairs with the loop above; without it record and matrix swap registers. */
            do
            {
                rotation = record->heading;
            } while (0);
            axis = 2;
            break;
        case 3:
            rotation = (((rotation_flags >> 26) * (u16)record->age) << 4) & 0xfff;
            axis = rotation_flags >> 24;
            axis &= 3;
            break;
        }
        switch (axis)
        {
        case 1:
            RotMatrixZ(rotation, matrix);
            break;
        case 0:
            RotMatrixY(rotation, matrix);
            break;
        case 3:
            RotMatrixZ(rotation, matrix);
            RotMatrixY(rotation, matrix);
            RotMatrixX(rotation, matrix);
            break;
        case 2:
            RotMatrixX(rotation, matrix);
            break;
        }
    }
    if (part->rotation_y_16 != 0)
    {
        RotMatrixZ(part->rotation_y_16 << 4, matrix);
    }
    if ((part->behavior_flags.word >> 3) & 1)
    {
        if ((part->placement_flags.word >> 0xB) & 1)
        {
            field_resolve_effect_position(record, part, scale);
            component = scale->vz;
            facing_angle = ratan2(record->z - component, scale->vx - record->x);
            delta->vx = (scale->vx - record->x) >> 8;
            delta->vy = (scale->vy - record->y) >> 8;
            delta->vz = (scale->vz - record->z) >> 8;
            /* Square the direction components before measuring horizontal distance. */
            gte_ldlvl(delta);
            gte_sqr0();
            gte_stlvnl(square);
            distance_xz = SquareRoot0(square->vx + square->vz);
            component = scale->vy;
            RotMatrixZ(ratan2(distance_xz << 8, record->y - component), matrix);
            RotMatrixY(-facing_angle, matrix);
        }
        else
        {
            RotMatrixZ(record->pitch, matrix);
            RotMatrixY(-record->heading, matrix);
            RotMatrixZ(-(s32)record->rotation_z_16 << 4, matrix);
            RotMatrixY(-(s32)record->rotation_y_16 << 4, matrix);
        }
    }
    field_copy_matrix_rotation(base_matrix, matrix);
    if ((part->placement_flags.word >> 2) & 1)
    {
        g_field_track_index = (s32)record->track_index;
        field_resolve_effect_position(record, part, scale);
        scale->vx -= record->x;
        scale->vy -= record->y;
        scale->vz -= record->z;
        scale->vx >>= 8;
        scale->vz >>= 8;
        scale->vy >>= 8;
        gte_ldlvl(scale);
        gte_sqr0();
        gte_stlvnl(delta);
        distance = SquareRoot0(delta->vx + delta->vy + delta->vz);
        if (distance == 0)
        {
            distance = 1;
        }
        scale->vz = ONE;
        scale->vx = ONE;
        scale->vy = distance << 6;
        ScaleMatrix(matrix, scale);
    }
    if (part->palette_extent.word & FIELD_PART_FOOTPRINT_SCALES_XZ)
    {
        scale_xz = part->appearance.fields.footprint_scale_x;
        horizontal_scale =
            (scale_xz - ((scale_xz * ((s32)(FIELD_OBJECT_FOOTPRINT_STRENGTH_RANGE - g_field_object_states[actor->owner_object_index].effect_footprint_strength) >> 6)) / 10))
            << 6;
        scale->vz = horizontal_scale;
        scale->vx = horizontal_scale;
    }
    else
    {
        base_scale = part->appearance.fields.footprint_scale_x << 6;
        scale->vz = base_scale;
        scale->vx = base_scale;
    }
    if (part->palette_extent.word & FIELD_PART_FOOTPRINT_SCALES_Y)
    {
        scale_y = part->footprint_scale_y;
        scale->vy =
            (scale_y - ((scale_y * ((s32)(FIELD_OBJECT_FOOTPRINT_STRENGTH_RANGE - g_field_object_states[actor->owner_object_index].effect_footprint_strength) >> 6)) / 10))
            << 6;
    }
    else
    {
        scale->vy = part->footprint_scale_y << 6;
    }
    ScaleMatrix(matrix, scale);
    if ((part->behavior_flags.bytes.low >> 7) != 0)
    {
        scale->vz = ONE;
        scale->vy = ONE;
        scale->vx = ONE;
        scale_flags = part->behavior_flags.word;
        scale_mode = (scale_flags >> 0x14) & 3;
        switch (scale_mode)
        {
        case 0:
            track_scale_xz =
                field_evaluate_parameter_track_at_time(actor, scale_flags >> 0x1C, (u16)record->age) *
                16;
            scale->vz = track_scale_xz;
            scale->vx = track_scale_xz;
            break;
        case 1:
            track_index = scale_flags >> 0x1C;
            scale->vy =
                field_evaluate_parameter_track_at_time(actor, track_index, (u16)record->age) * 16;
            break;
        case 2:
            track_scale_xyz =
                field_evaluate_parameter_track_at_time(actor, scale_flags >> 0x1C, (u16)record->age) *
                16;
            scale->vz = track_scale_xyz;
            scale->vy = track_scale_xyz;
            scale->vx = track_scale_xyz;
            break;
        case 3:
            track_scale_pair =
                field_evaluate_parameter_track_at_time(actor, scale_flags >> 0x1C, (u16)record->age) *
                16;
            scale->vz = track_scale_pair;
            scale->vx = track_scale_pair;
            track_index = (part->behavior_flags.word >> 0x1C) + 1;
            scale->vy =
                field_evaluate_parameter_track_at_time(actor, track_index, (u16)record->age) * 16;
            break;
        }
        ScaleMatrix(matrix, scale);
    }
}
