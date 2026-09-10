#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
/** @brief Twenty-four-byte mesh entry with triangle and per-triangle offset arrays. */
typedef struct
{
    u16 count;
    u8 pad2[6];
    SVECTOR *triangles;
    u8 padc[4];
    SVECTOR *offsets;
    u8 pad14[4];
} FieldTransformMesh;
/** @brief Partial actor state exposing its mesh table. */
typedef struct
{
    u8 pad0[0x18];
    FieldTransformMesh *meshes;
} FieldActorState;
/** @brief Partial actor record exposing the parameter-track time. */
typedef struct
{
    u8 pad0[0x2C];
    u16 time;
} Struct_D800FDF58;
/** @brief FieldActorPartDef descriptor flags controlling vertex interpolation and offsets. */
typedef struct
{
    u32 flags;
} FieldActorPartDef;
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
    u32 part_flags = part->flags;
    if ((part_flags >> 18) & 1)
    {
        amount =
            field_evaluate_parameter_track_at_time(actor, (part_flags >> 2) & 15, record->time);
        mode = (part->flags >> 19) & 3;
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
