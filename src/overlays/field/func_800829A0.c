#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Normal count and array pointer in a 0x18-byte actor mesh record. */
typedef struct
{
    u16 count;
    u8 pad2[10];
    SVECTOR *normals;
    u8 tail[8];
} FieldMeshNormalSet;
/** @brief FieldNormalActor prefix containing the mesh table pointer at offset 0x18. */
typedef struct
{
    u8 pad[0x18];
    FieldMeshNormalSet *meshes;
} FieldNormalActor;
/** @brief Animation-time field at offset 0x2C in an actor instance record. */
typedef struct
{
    u8 pad[0x2C];
    u16 time;
} FieldNormalAnimationRecord;
extern SVECTOR *D_80105870;
extern s32 field_evaluate_parameter_track_at_time(FieldNormalActor *actor, u32 track, u16 time);

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
void func_800829A0(FieldNormalActor *actor, FieldNormalAnimationRecord *record, u32 *part, s32 index, MATRIX *matrix)
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
        factor = field_evaluate_parameter_track_at_time(actor, (*part >> 2) & 15, record->time);
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
