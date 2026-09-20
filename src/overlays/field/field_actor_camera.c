#include "common.h"
#include "field_effect_render_state.h"
#include "display.h"

/**
 * @file field_actor_camera.c
 * @brief Field camera target tracking, map clamping, and horizontal
 *        interpolation-bound setup.
 *
 * Groups the four camera-control routines that share the D_8010AExx
 * interpolation-bound globals and the D_8010D01x smoothed target:
 * func_80091BC8 (target derive + approach), func_80091D7C (update + clamp +
 * screen motion), func_800920FC (reset lower bound), and func_80092124
 * (horizontal bound select).
 */

/** @brief Actor position and presence fields in a 0x54-byte record. */
typedef struct
{
    s32 x;
    s32 pad4;
    s32 z;
    u8 pad_c[0x19];
    u8 presence;
    u8 pad26[0x2E];
} Actor;

/** @brief Actor slot flags and vertical offset in a 0x23C-byte record. */
typedef struct
{
    u8 pad0[12];
    s32 flags;
    u8 pad10[0x166];
    s16 height;
    u8 pad178[0xC4];
} Slot;

/** @brief Screen coordinate scratch record retaining 16-bit components. */
typedef struct
{
    u16 x, y, z, pad;
} Point;

/** @brief Fixed-point offset used when projecting camera motion. */
typedef struct
{
    s32 x, y, z, pad;
} Vector;

/** @brief Scene identifier word followed by fixed-point camera coordinates. */
typedef struct
{
    s32 scene, x, y, z;
} Camera;

/** @brief Map dimensions used to clamp the camera. */
typedef struct
{
    u16 width, height;
} Bounds;

/** @brief Interpolation bound thresholds for a camera mode (min, span). */
typedef struct
{
    u16 min;
    u16 span;
} FieldThreshold;

/** @brief Fixed-point camera coordinates at 0x801ED480. */
typedef struct
{
    u8 _pad[4];
    s32 x;
    s32 y;
    s32 z;
} FieldCamera;

extern int abs(int);

extern void func_80092200(void);
extern void func_800922B8(void);

extern Actor g_field_actors[];
extern Slot g_field_object_states[];
extern Point D_801077FC;
extern FieldThreshold g_field_group_bounds[];

extern s32 D_800F2278;
extern s32 D_800F227C;
extern s32 D_800F2280;

extern s32 g_field_active_group;
extern s32 g_field_group_bounds_count;
extern s32 g_field_camera_target_x;
extern s32 g_field_camera_target_z;
extern s32 D_8010AE58;
extern s32 D_8010AE60;
extern s32 D_8010AE68;
extern s32 D_8010AE6C;
extern s32 D_8010AE70;
extern s32 D_8010AE7C;
extern s32 D_8010AE80;
extern s32 g_field_camera_follow_x;
extern s32 g_field_camera_follow_z;

/**
 * @brief Update the actor-derived target and approach each coordinate by at most 0x800.
 */
void func_80091BC8(void)
{
    s32 divisor;
    s32 target_z;
    s32 target_x;
    s32 count;
    s32 index;
    Actor *actor;
    Slot *slot;

    count = 0;
    target_z = 0;
    target_x = 0;
    index = 1;
    do
    {
        actor = &g_field_actors[index];
        slot = &g_field_object_states[index];
        if ((actor->presence != 0xFF) && !(slot->flags & 0x23E4))
        {
            target_x -= actor->x;
            target_z -= actor->z;
            target_z += slot->height << 9;
            if (count != 0)
            {
                divisor = count + 1;
                target_x = target_x / divisor;
                target_z = target_z / divisor;
            }
            count += 1;
        }
        index -= 1;
    } while (index >= 0);
    if (count != 0)
    {
        g_field_camera_target_x = target_x;
        g_field_camera_target_z = target_z;
    }

    {
        s32 target = g_field_camera_target_x;
        s32 current = g_field_camera_follow_x;

        if (target != current)
        {
            s32 delta = target - current;
            s32 magnitude = abs(delta);

            if (magnitude < 0x800)
            {
                g_field_camera_follow_x = target;
            }
            else
            {
                s32 value;
                s32 *write_position = &g_field_camera_follow_x;

                if (delta < 0)
                {
                    value = current - 0x800;
                }
                else
                {
                    value = current + 0x800;
                }
                *write_position = value;
            }
        }
    }

    {
        s32 target = g_field_camera_target_z;
        s32 current = g_field_camera_follow_z;

        if (target != current)
        {
            s32 delta = target - current;
            s32 magnitude = abs(delta);

            if (magnitude < 0x800)
            {
                g_field_camera_follow_z = target;
            }
            else
            {
                s32 value;
                s32 *write_position = &g_field_camera_follow_z;

                if (delta < 0)
                {
                    value = current - 0x800;
                }
                else
                {
                    value = current + 0x800;
                }
                *write_position = value;
            }
        }
    }
}

/**
 * @brief Update the camera, clamp it to map bounds, and record screen motion.
 *
 * Camera coordinates use eight fractional bits; projecting depth divides by
 * 512. The temporary points and zero offset preserve the original stack layout.
 */
void func_80091D7C(void)
{
    Point before;
    Point after;
    Vector offset;
    Camera *camera = (Camera *)0x801ED480;
    Bounds *bounds = (Bounds *)0x801ED400;
    s32 screen_x;
    s32 camera_screen_x, camera_screen_y, offset_screen_x, offset_screen_y;
    s32 screen_y;
    s32 follow_x;
    s32 follow_z;
    s32 clamp_x;
    s32 clamp_z;

    func_80091BC8();
    follow_x = g_field_camera_follow_x;
    follow_z = g_field_camera_follow_z;
    func_800922B8();
    func_80092200();
    offset.x = 0;
    offset.y = 0;
    offset.z = 0;
    before.x = g_field_view_offset_x / 256 + 160;
    before.y = g_field_view_offset_y / 256 + 112 - g_field_view_offset_z / 512;
    if (follow_x < (g_field_view_offset_x - 0x2000))
    {
        g_field_view_offset_x = follow_x + 0x2000;
    }
    if ((g_field_view_offset_x + 0x2000) < follow_x)
    {
        g_field_view_offset_x = follow_x - 0x2000;
    }
    if (follow_z < (g_field_view_offset_z - 0x2000))
    {
        g_field_view_offset_z = follow_z + 0x2000;
    }
    if ((g_field_view_offset_z + 0x2000) < follow_z)
    {
        g_field_view_offset_z = follow_z - 0x2000;
    }
    camera->x = g_field_view_offset_x + (D_800F2278 << 8) + 0xA000;
    g_field_view_offset_y = camera->y = D_800F227C << 8;
    camera->z = g_field_view_offset_z + (D_800F2280 << 9) + 0xE000;
    if (camera->x > -(D_8010AE60 << 8))
    {
        camera->x = -(D_800F2278 << 8) - (D_8010AE60 << 8);
        g_field_view_offset_x = camera->x - (D_800F2278 << 8) - 0xA000;
    }
    if (camera->x < -(D_8010AE68 << 8) + 0x14000)
    {
        clamp_x = (D_800F2278 << 8) - 0x14000;
        clamp_x = -(D_8010AE68 << 8) - clamp_x;
        camera->x = clamp_x;
        g_field_view_offset_x = camera->x - (D_800F2278 << 8) - 0xA000;
    }
    if (camera->z > 0)
    {
        camera->z = -(D_800F2280 << 9);
        g_field_view_offset_z = camera->z - (D_800F2280 << 9) - 0xE000;
    }
    if (camera->z < -((s32)(bounds->height << 16) >> 8) + 0x1C000)
    {
        clamp_z = (D_800F2280 << 9) - 0x1C000;
        clamp_z = -((s32)(bounds->height << 16) >> 8) - clamp_z;
        camera->z = clamp_z;
        g_field_view_offset_z = camera->z - (D_800F2280 << 9) - 0xE000;
    }
    if (D_8010AE7C != 0 || D_8010AE80 != 0)
    {
        camera->x = -(D_8010AE7C << 8);
        g_field_view_offset_x = -(D_8010AE7C + 0xA0) << 8;
        camera->z = -(D_8010AE80 << 9);
        g_field_view_offset_z = -(D_8010AE80 + 0x70) << 9;
    }
    camera_screen_x = g_field_view_offset_x / 256;
    offset_screen_x = offset.x / 256 + 160;
    screen_x = camera_screen_x + offset_screen_x;
    after.x = screen_x;
    camera_screen_y = g_field_view_offset_y / 256;
    offset_screen_y = offset.y / 256 + 112;
    screen_y = camera_screen_y + offset_screen_y - offset.z / 512 - g_field_view_offset_z / 512;
    after.y = screen_y;
    D_801077FC.x = screen_x - before.x;
    D_801077FC.y = screen_y - before.y;
}

/**
 * @brief Reset the interpolation lower bound and seed the upper bound from the field bounds block.
 */
void func_800920FC(void)
{
    D_8010AE6C = 0;
    D_8010AE58 = 0x20;
    D_8010AE70 = *(s16*)0x801ED400;
}

/**
 * @brief Configure horizontal camera interpolation bounds for the active field camera mode.
 */
void func_80092124(void)
{
    s32 camera_mode;
    FieldThreshold* camera_thresholds;
    FieldThreshold* threshold;
    FieldCamera* camera;
    s32 threshold_index;
    s32 lower_bound;

    camera = (FieldCamera*)0x801ED480;
    camera_mode = g_field_active_group;
    if (camera_mode == 0)
    {
        D_8010AE6C = 0;
        D_8010AE58 = 32;
        D_8010AE70 = *(s16*)0x801ED400;
        return;
    }

    if (g_field_group_bounds_count < camera_mode)
    {
        goto camera_bounds;
    }

    camera_thresholds = g_field_group_bounds;
    threshold_index = camera_mode - 1;
    threshold = &camera_thresholds[threshold_index];
    if (threshold->span < SCREEN_WIDTH)
    {
camera_bounds:
        D_8010AE58 = 16;
        lower_bound = D_8010AE6C = -(camera->x >> 8);
        D_8010AE70 = lower_bound + SCREEN_WIDTH;
        return;
    }

    D_8010AE58 = 32;
    D_8010AE6C = threshold->min;
    D_8010AE70 = threshold->min + threshold->span;
}

extern s32 D_8010AE74;
extern s32 D_8010CFD8;
extern s32 D_8010CFDC;

/**
 * @brief Step the D_8010AE60 / D_8010AE68 pair toward D_8010AE6C / D_8010AE70 over the remaining D_8010AE58 frames.
 */
void func_80092200(void)
{
    s32 temp_a1;
    s32 temp_v1;

    if (D_8010AE58 != 0) {
        s32 *cur60 = &D_8010AE60;
        temp_a1 = (D_8010AE6C - *cur60) / D_8010AE58;
        temp_v1 = (D_8010AE70 - D_8010AE68) / D_8010AE58;
        D_8010AE58 -= 1;
        D_8010AE60 += temp_a1;
        D_8010AE68 += temp_v1;
    }
}

/**
 * @brief Step the D_8010AE7C / D_8010AE80 pair toward D_8010CFD8 / D_8010CFDC over the remaining D_8010AE74 frames, snapping when none remain.
 */
void func_800922B8(void)
{
    s32 temp_a1;
    s32 temp_v1;

    if (D_8010AE74 != 0) {
        s32 *cur = &D_8010AE7C;
        temp_a1 = (D_8010CFD8 - *cur) / D_8010AE74;
        temp_v1 = (D_8010CFDC - D_8010AE80) / D_8010AE74;
        D_8010AE74 -= 1;
        D_8010AE7C += temp_a1;
        D_8010AE80 += temp_v1;
        return;
    }
    D_8010AE7C = D_8010CFD8;
    D_8010AE80 = D_8010CFDC;
}

/**
 * @brief Clear the interpolation state and seed both active bounds from the field bounds block.
 */
void func_80092394(void)
{
    s32 value;

    value = *(s16*)0x801ED400;

    D_8010AE6C = 0;
    D_8010AE60 = 0;
    D_8010AE58 = 0;
    D_8010AE80 = 0;
    D_8010AE7C = 0;
    D_8010CFDC = 0;
    D_8010CFD8 = 0;
    D_8010AE74 = 0;
    D_8010AE68 = value;
    D_8010AE70 = value;
}
