#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
/** @brief Partial actor state exposing the owning slot index. */
typedef struct
{
    u8 pad0[0x228];
    u8 unk228;
} FieldActorState;
/** @brief Partial actor record containing position, rotation, and track time. */
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
/** @brief Partial actor-part descriptor containing transform flags and scales. */
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
/** @brief Actor-slot layout exposing the 0x68 transform attenuation value. */
typedef struct
{
    u8 pad0[0x68];
    u16 unk68;
    u8 pad6a[0x23C - 0x6A];
} FieldTransformSlot;
extern FieldTransformSlot D_80105AE0[];
extern s32 g_field_track_index;
s32 field_evaluate_parameter_track_at_time(FieldActorState *, u32, u16);
void func_80073F7C(Struct_D800FDF58 *, FieldActorPartDef *, VECTOR *);
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
 */
void func_80082C90(FieldActorState *actor, Struct_D800FDF58 *record, FieldActorPartDef *part,
                   MATRIX *matrix, MATRIX *base_matrix)
{
    VECTOR *scale = (VECTOR *)0x1F800000;
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
    ((s32 *)matrix)[0] = 0x1000;
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
            rotation = record->unk12;
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
            func_80073F7C(record, part, scale);
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
        func_80073F7C(record, part, scale);
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
