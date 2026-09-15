/** @file field_mesh_transform.c
 * @brief Transform mesh vertices, normals, and actor-part matrices.
 */

#include "common.h"
#include "sdk/libgte.h"

typedef struct
{
    u16 count;
    u8 pad2[6];
    SVECTOR *triangles;
    SVECTOR *normals;
    SVECTOR *offsets;
    u8 pad14[4];
} FieldTransformMesh;
typedef struct
{
    u8 pad0[0x18];
    FieldTransformMesh *meshes;
    u8 pad1C[0x228 - 0x1C];
    u8 unk228;
} FieldActorState;
typedef struct
{
    s32 unk0, unk4, unk8;
    u8 padc[6];
    s16 unk12, unk14;
    u8 pad16[0x29 - 0x16];
    u8 unk29;
    u8 pad2a[2];
    u16 unk2C;
    u8 pad2e[4];
    u8 unk32, unk33;
} Struct_D800FDF58;
typedef struct
{
    u32 unk0, unk4;
    u8 pad8[0x13 - 8];
    u8 unk13;
    u8 pad14[8];
    u32 unk1C;
    u8 pad20[8];
    u32 unk28;
    u8 pad2c[2];
    u8 unk2E;
    u8 pad2f[4];
    u8 unk33;
} FieldActorPartDef;
typedef struct
{
    u8 pad0[0x68];
    u16 unk68;
    u8 pad6a[0x23C - 0x6A];
} FieldTransformSlot;

/* func_800822A4 */
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
/** @brief Twenty-four-byte mesh entry with triangle and per-triangle offset arrays. */

/** @brief Partial actor state exposing its mesh table. */

/** @brief Partial actor record exposing the parameter-track time. */

/** @brief FieldActorPartDef descriptor flags controlling vertex interpolation and offsets. */

extern s16 *D_80105790;
extern s32 *D_80105878;
s32 field_evaluate_parameter_track_at_time(FieldActorState *, u32, u16);
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
void func_800822A4(FieldActorState *actor, Struct_D800FDF58 *record, FieldActorPartDef *part,
                   s32 index)
{
    s32 flags; /* GTE status destination retained by the original transform sequence. */
    VECTOR *transformed = (VECTOR *)0x1F800040;
    SVECTOR *work = (SVECTOR *)0x1F800000;
    s16 *out = D_80105790;
    s32 *depth = D_80105878;
    SVECTOR *src = actor->meshes[index].triangles;
    SVECTOR *other;
    s32 count;
    s32 amount;
    s32 mode;
    s32 dx, dy, dz;
    u32 part_flags = part->unk0;
    if ((part_flags >> 18) & 1)
    {
        amount =
            field_evaluate_parameter_track_at_time(actor, (part_flags >> 2) & 15, record->unk2C);
        mode = (part->unk0 >> 19) & 3;
        if (mode < 3)
        {
            /* Interpolate corresponding vertices with a 12-bit fractional weight. */
            count = actor->meshes[index].count;
            other = actor->meshes[mode].triangles;
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
            count = actor->meshes[index].count;
            /* The offset mode uses one displacement vector per triangle. */
            other = actor->meshes[index].offsets;
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
        count = actor->meshes[index].count;
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


/* func_800829A0 */
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Normal count and array pointer in a 0x18-byte actor mesh record. */

/** @brief FieldActorState prefix containing the mesh table pointer at offset 0x18. */

/** @brief Animation-time field at offset 0x2C in an actor instance record. */

extern SVECTOR *D_80105870;
extern s32 field_evaluate_parameter_track_at_time(FieldActorState *actor, u32 track, u16 time);

/**
 * @brief Rotate mesh normals, optionally blending toward another normal set.
 * @param actor Actor supplying the normal sets and animation tracks.
 * @param record Actor instance supplying the current animation time.
 * @param part Part flags selecting interpolation and its track/normal set.
 * @param index Source mesh normal-set index.
 * @param matrix Rotation matrix applied through the GTE.
 * @note The interpolation branch writes output slots from count down through one.
 * @note 100% match with GCC 2.7.2 CDK: 188 instructions, 752 bytes.
 */
void func_800829A0(FieldActorState *actor, Struct_D800FDF58 *record, u32 *part, s32 index, MATRIX *matrix)
{
    SVECTOR *output;
    SVECTOR *source;
    SVECTOR *scratch = (SVECTOR *)0x1F800000;
    s32 factor;
    s32 selected;
    s32 count;

    output = D_80105870;
    source = actor->meshes[index].normals;
    if ((*part >> 18) & 1)
    {
        factor = field_evaluate_parameter_track_at_time(actor, (*part >> 2) & 15, record->unk2C);
        selected = (*part >> 19) & 3;
        if (selected < 3)
        {
            count = actor->meshes[index].count;
            /* Reuse the destination cursor for the alternate source normals. */
            output = actor->meshes[selected].normals;
            while (count != 0)
            {
                scratch->vx = (u16)source->vx + (((output->vx - source->vx) * factor) >> 12);
                scratch->vy = (u16)source->vy + (((output->vy - source->vy) * factor) >> 12);
                scratch->vz = (u16)source->vz + (((output->vz - source->vz) * factor) >> 12);
                gte_SetRotMatrix(matrix);
                gte_ldv0(scratch);
                gte_rtv0();
                gte_stsv(&D_80105870[count]);
                count--;
                source++;
                output++;
            }
        }
        else
        {
            count = actor->meshes[index].count;
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
        count = actor->meshes[index].count;
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


/* func_80082C90 */
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
/** @brief Partial actor state exposing the owning slot index. */

/** @brief Partial actor record containing position, rotation, and track time. */

/** @brief Partial actor-part descriptor containing transform flags and scales. */

/** @brief Actor-slot layout exposing the 0x68 transform attenuation value. */

extern FieldTransformSlot D_80105AE0[];
extern s32 g_field_track_index;
s32 field_evaluate_parameter_track_at_time(FieldActorState *, u32, u16);
void field_resolve_effect_position(Struct_D800FDF58 *, FieldActorPartDef *, VECTOR *);
void func_800832F0(MATRIX *, MATRIX *);
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
 * @return Unspecified; callers ignore the value.
 */
s32 func_80082C90(FieldActorState *actor, Struct_D800FDF58 *record, FieldActorPartDef *part,
                   MATRIX *matrix, MATRIX *base_matrix)
{
    VECTOR *scale;
    VECTOR *delta = (VECTOR *)0x1F800010;
    VECTOR *square = (VECTOR *)0x1F800020;
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

    /* Initialize the packed rotation and translation words to identity. */
    ((s32 *)matrix)[4] = 0x1000;
    ((s32 *)matrix)[2] = 0x1000;
    do
    {
        ((s32 *)matrix)[0] = 0x1000;
    } while (0);
    scale = (VECTOR *)0x1F800000;
    ((s32 *)matrix)[7] = 0;
    ((s32 *)matrix)[6] = 0;
    ((s32 *)matrix)[5] = 0;
    ((s32 *)matrix)[3] = 0;
    ((s32 *)matrix)[1] = 0;
    rotation_flags = part->unk0;
    component = (rotation_flags >> 6) & 3;
    if (component != 0)
    {
        switch (component)
        {
        case 1:
            rotation =
                field_evaluate_parameter_track_at_time(actor, rotation_flags >> 26, record->unk2C)
                << 4;
            axis = ((u8 *)part)[3];
            axis &= 3;
            break;
        case 2:
            RotMatrixX(0x400, matrix);
            do
            {
                rotation = record->unk12;
            } while (0);
            axis = 2;
            break;
        case 3:
            rotation = (((rotation_flags >> 26) * record->unk2C) << 4) & 0xfff;
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
    if (part->unk13 != 0)
    {
        RotMatrixZ(part->unk13 << 4, matrix);
    }
    if (((u32)part->unk4 >> 3) & 1)
    {
        if (((u32)part->unk28 >> 0xB) & 1)
        {
            field_resolve_effect_position(record, part, scale);
            component = scale->vz;
            facing_angle = ratan2(record->unk8 - component, scale->vx - record->unk0);
            delta->vx = (s32)((s32)(scale->vx - record->unk0) >> 8);
            delta->vy = (s32)((s32)(scale->vy - record->unk4) >> 8);
            delta->vz = (s32)((s32)(scale->vz - record->unk8) >> 8);
            /* Square the direction components before measuring horizontal distance. */
            gte_ldlvl(delta);
            gte_sqr0();
            gte_stlvnl(square);
            distance_xz = SquareRoot0(square->vx + square->vz);
            component = scale->vy;
            RotMatrixZ(ratan2(distance_xz << 8, record->unk4 - component), matrix);
            RotMatrixY(-facing_angle, matrix);
        }
        else
        {
            RotMatrixZ(record->unk14, matrix);
            RotMatrixY(-record->unk12, matrix);
            RotMatrixZ(-(s32)record->unk32 << 4, matrix);
            RotMatrixY(-(s32)record->unk33 << 4, matrix);
        }
    }
    func_800832F0(base_matrix, matrix);
    if (((u32)part->unk28 >> 2) & 1)
    {
        g_field_track_index = (s32)record->unk29;
        field_resolve_effect_position(record, part, scale);
        /* Keep subtraction and normalization as separate scratchpad updates. */
        scale->vx -= record->unk0;
        scale->vy -= record->unk4;
        scale->vz -= record->unk8;
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
        scale->vz = 0x1000;
        scale->vx = 0x1000;
        scale->vy = distance << 6;
        ScaleMatrix(matrix, scale);
    }
    if (part->unk1C & 0x02000000)
    {
        scale_xz = part->unk2E;
        horizontal_scale =
            (scale_xz - ((scale_xz * ((s32)(0x100 - D_80105AE0[actor->unk228].unk68) >> 6)) / 10))
            << 6;
        scale->vz = horizontal_scale;
        scale->vx = horizontal_scale;
    }
    else
    {
        base_scale = part->unk2E << 6;
        scale->vz = base_scale;
        scale->vx = base_scale;
    }
    if (part->unk1C & 0x04000000)
    {
        scale_y = part->unk33;
        scale->vy =
            (scale_y - ((scale_y * ((s32)(0x100 - D_80105AE0[actor->unk228].unk68) >> 6)) / 10))
            << 6;
    }
    else
    {
        scale->vy = part->unk33 << 6;
    }
    ScaleMatrix(matrix, scale);
    if ((((u8 *)part)[4] >> 7) != 0)
    {
        scale->vz = 0x1000;
        scale->vy = 0x1000;
        scale->vx = 0x1000;
        scale_flags = part->unk4;
        scale_mode = (scale_flags >> 0x14) & 3;
        switch (scale_mode)
        {
        case 0:
            track_scale_xz =
                field_evaluate_parameter_track_at_time(actor, scale_flags >> 0x1C, record->unk2C) *
                0x10;
            scale->vz = track_scale_xz;
            scale->vx = track_scale_xz;
            break;
        case 1:
            track_index = scale_flags >> 0x1C;
            scale->vy =
                field_evaluate_parameter_track_at_time(actor, track_index, record->unk2C) * 0x10;
            break;
        case 2:
            track_scale_xyz =
                field_evaluate_parameter_track_at_time(actor, scale_flags >> 0x1C, record->unk2C) *
                0x10;
            scale->vz = track_scale_xyz;
            scale->vy = track_scale_xyz;
            scale->vx = track_scale_xyz;
            break;
        case 3:
            track_scale_pair =
                field_evaluate_parameter_track_at_time(actor, scale_flags >> 0x1C, record->unk2C) *
                0x10;
            scale->vz = track_scale_pair;
            scale->vx = track_scale_pair;
            track_index = ((u32)part->unk4 >> 0x1C) + 1;
            scale->vy =
                field_evaluate_parameter_track_at_time(actor, track_index, record->unk2C) * 0x10;
            break;
        }
        ScaleMatrix(matrix, scale);
    }
}
