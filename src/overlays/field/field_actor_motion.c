/**
 * @file field_actor_motion.c
 * @brief Advance timed actor motion and test proposed movement against screen bounds.
 */

#include "common.h"
#include "field_effect_render_state.h"
#include "vector.h"
#include "field_types.h"

/** @brief Scratchpad vector that receives the actor displacement. */
#define FIELD_MOTION_SCRATCH ((Vec3i*)0x1F800000)

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
/** @brief Proposed field position and its signed screen projection. */
typedef struct
{
    FieldVector position;
    Vec2s screen;
} FieldBoundsProjection;

extern FieldMotionSlot g_field_object_states[];
extern FieldMotionVisual g_field_object_parts[];
extern FieldMotionResource g_field_resource_entries[];
extern void func_80092C98(FieldMotionActor*);
extern s32 func_80091728(s32, s32, FieldMotionActor*);
extern s32 field_object_has_active_actor_tracks(s32);
extern void field_update_sequence_actor_binding(FieldMotionActor*, s32);
extern s32 func_80093AB8(FieldMotionActor*);
extern s32 func_80092AD8(FieldMotionActor*);
extern void field_restart_sequence_animation(FieldMotionActor*);
extern void func_8008BC5C(FieldMotionActor*);
extern s32 field_resolve_actor_movement(FieldMotionActor*, Vec3i*, s32);

/**
 * @brief Advance actor motion state or apply its remaining scaled displacement.
 * @param actor Actor supplying movement, state, visual slot, and resource index.
 * @param update Nonzero permits the initial timed/state-specific update.
 * @return Nothing meaningful; the original declares an int return but never sets it.
 * @note Displacement is written to the three-component scratchpad vector.
 */
s32 func_800925EC(FieldMotionActor* actor, s32 update)
{
    Vec3i* scratch = FIELD_MOTION_SCRATCH;
    s32 unused[2]; /* never used; the original stack frame reserves it */
    s32 state;
    s32 step;
    s32 delta;
    FieldMotionVisual* visual;

    if (update != 0 && (actor->timer != 0 || (actor->state & 0x7F) == 0x35))
    {
        func_80092C98(actor);
    }
    if (actor->timer == 0)
    {
        state = actor->state & 0x7F;
        if (state == 0xA || state == 0x31)
        {
            if (g_field_object_states[actor->slot].track < 12U)
            {
                if (func_80091728(actor->slot, g_field_object_states[actor->slot].track, actor) != 0)
                {
                    return;
                }
            }
        }
        if ((g_field_object_states[actor->slot].state >> 1) & 1)
        {
            if (field_object_has_active_actor_tracks(actor->slot) != 0)
            {
                return;
            }
            g_field_object_states[g_field_object_states[actor->slot].parent].flags &= ~0x2000;
        }
        g_field_object_states[actor->slot].flags &= ~0x4000;
        field_update_sequence_actor_binding(actor, 1);
        g_field_object_states[actor->slot].options &= ~0x1800;
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
            field_restart_sequence_animation(actor);
        }
        if (actor->value == 0)
        {
            g_field_object_states[actor->slot].value = 0xFFFF;
            func_8008BC5C(actor);
        }
    }
    else
    {
        step = actor->movement / actor->divisor;
        actor->movement = (u8)actor->movement - step;
        visual = &g_field_object_parts[actor->slot];
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
        field_resolve_actor_movement(actor, scratch, 1);
        if (g_field_resource_entries[actor->resource].state == 0)
        {
            g_field_object_states[actor->slot].options &= ~0x4000;
        }
    }
}

/**
 * @brief Test whether a proposed displacement crosses a screen boundary.
 * @param position Current fixed-point field position.
 * @param delta Proposed displacement; only the X and Z components are tested.
 * @return One when the displacement crosses its corresponding screen edge.
 * @note The projected coordinates are truncated to 16 bits before the offsets are added.
 */
s32 func_80092988(Vec3i* position, Vec3i* delta)
{
    FieldBoundsProjection local;

    local.position.vx = position->x + delta->x;
    local.position.vy = position->y;
    local.position.vz = position->z + delta->z;
    local.screen.x = g_field_view_offset_x / 256 + (s16)(local.position.vx / 256 + 160);
    local.screen.y = g_field_view_offset_y / 256 + (s16)(local.position.vy / 256 + 112) - local.position.vz / 512 - g_field_view_offset_z / 512;
    if (delta->x < 0)
    {
        if (local.screen.x < 6)
        {
            return 1;
        }
    }
    else if (delta->x > 0)
    {
        if (local.screen.x >= 314)
        {
            return 1;
        }
    }
    if (delta->z > 0)
    {
        if (local.screen.y < 9)
        {
            return 1;
        }
    }
    else if (delta->z < 0)
    {
        if (local.screen.y >= 224)
        {
            return 1;
        }
    }
    return 0;
}
