#include "common.h"
#include "vector.h"
/** @brief State, timing, and movement fields in a 0x54-byte actor record. */
typedef struct
{
    u8 pad0[0x16];
    s16 divisor;
    u8 pad18[9];
    u8 state;
    u8 pad22[8];
    s16 value;
    u8 pad2c[2];
    u16 timer;
    u8 pad30[6];
    s8 movement;
    u8 pad37[3];
    u8 slot;
    u8 resource;
    u8 tail[0x18];
} FieldMotionActor;
/** @brief Track links, flags, and state in a 0x23C-byte actor slot. */
typedef struct
{
    u8 pad0[0xC];
    u32 flags;
    u8 pad10[0x2C];
    u32 value;
    u8 pad40[0x12F];
    u8 track;
    u8 parent;
    u8 pad171[3];
    u32 options;
    u32 state;
    u8 tail[0xC0];
} FieldMotionSlot;
/** @brief Movement scale in a 0x48-byte visual record. */
typedef struct
{
    u8 pad0[0x2E];
    u8 scale;
    u8 tail[0x19];
} FieldMotionVisual;
/** @brief Resource state byte in a 0x14-byte resource entry. */
typedef struct
{
    u8 pad0[8];
    u8 state;
    u8 tail[0xB];
} FieldMotionResource;
extern FieldMotionSlot D_80105AE0[];
extern FieldMotionVisual D_800FE3A0[];
extern FieldMotionResource g_field_resource_entries[];
extern void func_80092C98(FieldMotionActor *);
extern s32 func_80091728(s32, s32, FieldMotionActor *);
extern s32 field_object_has_active_actor_tracks(s32);
extern void func_800952DC(FieldMotionActor *, s32);
extern s32 func_80093AB8(FieldMotionActor *);
extern s32 func_80092AD8(FieldMotionActor *);
extern void func_80096334(FieldMotionActor *);
extern void func_8008BC5C(FieldMotionActor *);
extern s32 func_80097FA0(FieldMotionActor *, Vec3i *, s32);

/**
 * @brief Advance actor motion state or apply its remaining scaled displacement.
 * @param actor Actor supplying movement, state, visual slot, and resource index.
 * @param update Nonzero permits the initial timed/state-specific update.
 * @note Displacement is written to the three-component scratchpad vector.
 */
void func_800925EC(FieldMotionActor *actor, s32 update)
{
    Vec3i *scratch = (Vec3i *)0x1F800000;
    /* Preserve the target's eight-byte unused local allocation. */
    Vec2s unused_position[2];
    s32 state;
    s32 step;
    s32 delta;
    FieldMotionVisual *visual;

    if (update != 0 && (actor->timer != 0 || (actor->state & 0x7F) == 0x35))
    {
        func_80092C98(actor);
    }
    if (actor->timer == 0)
    {
        state = actor->state & 0x7F;
        if (state == 0xA || state == 0x31)
        {
            if (D_80105AE0[actor->slot].track < 12U)
            {
                if (func_80091728(actor->slot, D_80105AE0[actor->slot].track, actor) != 0)
                {
                    return;
                }
            }
        }
        if ((D_80105AE0[actor->slot].state >> 1) & 1)
        {
            if (field_object_has_active_actor_tracks(actor->slot) != 0)
            {
                return;
            }
            D_80105AE0[D_80105AE0[actor->slot].parent].flags &= ~0x2000;
        }
        D_80105AE0[actor->slot].flags &= ~0x4000;
        func_800952DC(actor, 1);
        D_80105AE0[actor->slot].options &= ~0x1800;
        state = actor->state & 0x7F;
        if (state == 0x37 || state == 0x3B)
        {
            actor->state ^= 0x80;
        }
        if (actor->slot < 2U && func_80093AB8(actor) != 0)
        {
            return;
        }
        if (func_80092AD8(actor) != 0)
        {
            actor->value = 0;
            actor->state &= 0x80;
            func_80096334(actor);
        }
        if (actor->value == 0)
        {
            D_80105AE0[actor->slot].value = 0xFFFF;
            func_8008BC5C(actor);
        }
    }
    else
    {
        step = actor->movement / actor->divisor;
        actor->movement = (u8)actor->movement - step;
        visual = &D_800FE3A0[actor->slot];
        if (actor->state & 0x80)
        {
            scratch->x = ((step << 8) * visual->scale) >> 6;
        }
        else
        {
            scratch->x = (-(step << 8) * visual->scale) >> 6;
        }
        scratch->y = 0;
        scratch->z = 0;
        func_80097FA0(actor, scratch, 1);
        if (g_field_resource_entries[actor->resource].state == 0)
        {
            D_80105AE0[actor->slot].options &= ~0x4000;
        }
    }
}
