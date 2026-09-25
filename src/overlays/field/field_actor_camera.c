/**
 * @file field_actor_camera.c
 * @brief Field camera: party tracking, scroll limits, scripted scrolling and
 *        the per-frame camera update.
 *
 * Positions keep eight fractional bits on x and y and nine on z (the map is
 * drawn at half depth). The view scrolls between a left and a right scroll
 * limit that ease toward the bounds of the active actor group; a script can
 * take over the scroll position instead.
 */

#include "common.h"
#include "sdk/libgte.h"
#include "field_calls.h"
#include "field_actor_tables.h"
#include "field_effect_render_state.h"
#include "display.h"
#include "scene_state.h"

/** @brief Width of the field view in pixels. */
#define FIELD_VIEW_WIDTH SCREEN_WIDTH

/** @brief Height of the field view in pixels. */
#define FIELD_VIEW_HEIGHT VRAM_DRAW_HEIGHT

/** @brief Horizontal centre of the field view in pixels. */
#define FIELD_VIEW_CENTER_X (FIELD_VIEW_WIDTH / 2)

/** @brief Vertical centre of the field view in pixels. */
#define FIELD_VIEW_CENTER_Y (FIELD_VIEW_HEIGHT / 2)

/** @brief Number of party objects the camera follows (objects 0 and 1). */
#define FIELD_CAMERA_TRACKED_COUNT 2

/* Object state flag bits with unknown meaning; together they keep an object out of the camera average. */
#define FIELD_OBJECT_FLAG_0004 0x0004
#define FIELD_OBJECT_FLAG_0020 0x0020
#define FIELD_OBJECT_FLAG_0040 0x0040
#define FIELD_OBJECT_FLAG_0080 0x0080
#define FIELD_OBJECT_FLAG_0100 0x0100
#define FIELD_OBJECT_FLAG_0200 0x0200
#define FIELD_OBJECT_FLAG_2000 0x2000
#define FIELD_CAMERA_IGNORE_FLAGS                                                                                                     \
    (FIELD_OBJECT_FLAG_0004 | FIELD_OBJECT_FLAG_0020 | FIELD_OBJECT_FLAG_0040 | FIELD_OBJECT_FLAG_0080 | FIELD_OBJECT_FLAG_0100 |     \
     FIELD_OBJECT_FLAG_0200 | FIELD_OBJECT_FLAG_2000)

/** @brief Largest per-frame step of the camera follow point, in position units. */
#define FIELD_CAMERA_MAX_STEP 0x800

/** @brief Distance the follow point may lead the view before the view scrolls, in position units. */
#define FIELD_CAMERA_DEAD_ZONE 0x2000

/** @brief Frames to ease the scroll limits to a group's bounds. */
#define FIELD_SCROLL_EASE_FRAMES 32

/** @brief Frames to ease the scroll limits onto the current screen. */
#define FIELD_SCROLL_LOCK_FRAMES 16

/** @brief Screen position, in pixels (only x and y are written). */
typedef struct
{
    u16 x;
    u16 y;
    u16 z;
    u16 pad;
} FieldScreenPoint;

/** @brief On-screen movement of the field view during the last update. */
typedef struct
{
    s16 x;
    s16 y;
} FieldScreenDelta;

/** @brief Horizontal scroll range of an actor group, in pixels. */
typedef struct
{
    u16 left;
    u16 width;
} FieldGroupBounds;

extern int abs(int);

static void field_camera_step_scroll_limits(void);
static void field_camera_step_scripted_scroll(void);

extern FieldScreenDelta g_field_screen_scroll;
extern FieldGroupBounds g_field_group_bounds[];
extern s32 g_field_group_bounds_count;
extern s32 g_field_active_group;

extern s32 g_field_camera_offset_x;
extern s32 g_field_camera_offset_y;
extern s32 g_field_camera_offset_z;

extern s32 g_field_camera_target_x;
extern s32 g_field_camera_target_z;
extern s32 g_field_camera_follow_x;
extern s32 g_field_camera_follow_z;

extern s32 D_8010AE58;
extern s32 g_field_scroll_limit_left;
extern s32 g_field_scroll_limit_right;
extern s32 g_field_scroll_limit_left_target;
extern s32 g_field_scroll_limit_right_target;

extern s32 g_field_scripted_scroll_frames;
extern s32 g_field_scripted_scroll_x;
extern s32 g_field_scripted_scroll_z;
extern s32 g_field_scripted_scroll_target_x;
extern s32 g_field_scripted_scroll_target_z;

/**
 * @brief Screen x of a view-relative position.
 * @param position View-relative position.
 * @return Screen x in pixels.
 */
static inline s32 field_camera_screen_x(VECTOR* position)
{
    s32 view_x = g_field_view_offset_x / 256;
    s32 offset_x = position->vx / 256 + FIELD_VIEW_CENTER_X;

    return view_x + offset_x;
}

/**
 * @brief Screen y of a view-relative position.
 * @param position View-relative position.
 * @return Screen y in pixels.
 */
static inline s32 field_camera_screen_y(VECTOR* position)
{
    s32 view_y = g_field_view_offset_y / 256;
    s32 offset_y = position->vy / 256 + FIELD_VIEW_CENTER_Y;

    return view_y + offset_y - position->vz / 512 - g_field_view_offset_z / 512;
}

/**
 * @brief Ease the scroll limits onto the current screen.
 * @param camera Scene camera.
 */
static inline void field_camera_lock_scroll(SceneState* camera)
{
    s32 left;

    D_8010AE58 = FIELD_SCROLL_LOCK_FRAMES;
    left = g_field_scroll_limit_left_target = -(camera->camera_x >> 8);
    g_field_scroll_limit_right_target = left + FIELD_VIEW_WIDTH;
}

/**
 * @brief Aim the camera at the average party position and move the follow point toward it.
 */
void field_camera_track_party(void)
{
    s32 divisor;
    s32 target_z;
    s32 target_x;
    s32 count;
    s32 index;

    count = 0;
    target_z = 0;
    target_x = 0;
    for (index = FIELD_CAMERA_TRACKED_COUNT - 1; index >= 0; index--)
    {
        if ((g_field_actors[index].presence != FIELD_ACTOR_UNUSED) && !(g_field_object_states[index].flags & FIELD_CAMERA_IGNORE_FLAGS))
        {
            target_x -= g_field_actors[index].x;
            target_z -= g_field_actors[index].z;
            target_z += g_field_object_states[index].movement.half.hi << 9;
            if (count != 0)
            {
                divisor = count + 1;
                target_x = target_x / divisor;
                target_z = target_z / divisor;
            }
            count += 1;
        }
    }
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

            if (abs(delta) < FIELD_CAMERA_MAX_STEP)
            {
                g_field_camera_follow_x = target;
            }
            else
            {
                s32 value;
                s32* follow = &g_field_camera_follow_x; /* storing through the global directly changes the codegen */

                if (delta < 0)
                {
                    value = current - FIELD_CAMERA_MAX_STEP;
                }
                else
                {
                    value = current + FIELD_CAMERA_MAX_STEP;
                }
                *follow = value;
            }
        }
    }

    {
        s32 target = g_field_camera_target_z;
        s32 current = g_field_camera_follow_z;

        if (target != current)
        {
            s32 delta = target - current;

            if (abs(delta) < FIELD_CAMERA_MAX_STEP)
            {
                g_field_camera_follow_z = target;
            }
            else
            {
                s32 value;
                s32* follow = &g_field_camera_follow_z; /* storing through the global directly changes the codegen */

                if (delta < 0)
                {
                    value = current - FIELD_CAMERA_MAX_STEP;
                }
                else
                {
                    value = current + FIELD_CAMERA_MAX_STEP;
                }
                *follow = value;
            }
        }
    }
}

/**
 * @brief Scroll the view after the follow point, clamp it to the scroll limits and record the screen motion.
 */
void field_camera_update(void)
{
    FieldScreenPoint before;
    FieldScreenPoint after;
    VECTOR origin;
    SceneState* camera = SCENE_STATE;
    FieldMapBounds* bounds = FIELD_MAP_BOUNDS;
    s32 follow_x;
    s32 follow_z;
    s32 clamp_x;
    s32 clamp_z;

    field_camera_track_party();
    follow_x = g_field_camera_follow_x;
    follow_z = g_field_camera_follow_z;
    field_camera_step_scripted_scroll();
    field_camera_step_scroll_limits();
    origin.vx = 0;
    origin.vy = 0;
    origin.vz = 0;
    before.x = field_camera_screen_x(&origin);
    before.y = field_camera_screen_y(&origin);
    if (follow_x < (g_field_view_offset_x - FIELD_CAMERA_DEAD_ZONE))
    {
        g_field_view_offset_x = follow_x + FIELD_CAMERA_DEAD_ZONE;
    }
    if ((g_field_view_offset_x + FIELD_CAMERA_DEAD_ZONE) < follow_x)
    {
        g_field_view_offset_x = follow_x - FIELD_CAMERA_DEAD_ZONE;
    }
    if (follow_z < (g_field_view_offset_z - FIELD_CAMERA_DEAD_ZONE))
    {
        g_field_view_offset_z = follow_z + FIELD_CAMERA_DEAD_ZONE;
    }
    if ((g_field_view_offset_z + FIELD_CAMERA_DEAD_ZONE) < follow_z)
    {
        g_field_view_offset_z = follow_z - FIELD_CAMERA_DEAD_ZONE;
    }
    camera->camera_x = g_field_view_offset_x + (g_field_camera_offset_x << 8) + (FIELD_VIEW_CENTER_X << 8);
    g_field_view_offset_y = camera->camera_y = g_field_camera_offset_y << 8;
    camera->camera_z = g_field_view_offset_z + (g_field_camera_offset_z << 9) + (FIELD_VIEW_CENTER_Y << 9);
    if (camera->camera_x > -(g_field_scroll_limit_left << 8))
    {
        camera->camera_x = -(g_field_camera_offset_x << 8) - (g_field_scroll_limit_left << 8);
        g_field_view_offset_x = camera->camera_x - (g_field_camera_offset_x << 8) - (FIELD_VIEW_CENTER_X << 8);
    }
    if (camera->camera_x < -(g_field_scroll_limit_right << 8) + (FIELD_VIEW_WIDTH << 8))
    {
        clamp_x = (g_field_camera_offset_x << 8) - (FIELD_VIEW_WIDTH << 8);
        clamp_x = -(g_field_scroll_limit_right << 8) - clamp_x;
        camera->camera_x = clamp_x;
        g_field_view_offset_x = camera->camera_x - (g_field_camera_offset_x << 8) - (FIELD_VIEW_CENTER_X << 8);
    }
    if (camera->camera_z > 0)
    {
        camera->camera_z = -(g_field_camera_offset_z << 9);
        g_field_view_offset_z = camera->camera_z - (g_field_camera_offset_z << 9) - (FIELD_VIEW_CENTER_Y << 9);
    }
    if (camera->camera_z < -((s16)bounds->depth << 8) + (FIELD_VIEW_HEIGHT << 9))
    {
        clamp_z = (g_field_camera_offset_z << 9) - (FIELD_VIEW_HEIGHT << 9);
        clamp_z = -((s16)bounds->depth << 8) - clamp_z;
        camera->camera_z = clamp_z;
        g_field_view_offset_z = camera->camera_z - (g_field_camera_offset_z << 9) - (FIELD_VIEW_CENTER_Y << 9);
    }
    if (g_field_scripted_scroll_x != 0 || g_field_scripted_scroll_z != 0)
    {
        camera->camera_x = -(g_field_scripted_scroll_x << 8);
        g_field_view_offset_x = -(g_field_scripted_scroll_x + FIELD_VIEW_CENTER_X) << 8;
        camera->camera_z = -(g_field_scripted_scroll_z << 9);
        g_field_view_offset_z = -(g_field_scripted_scroll_z + FIELD_VIEW_CENTER_Y) << 9;
    }
    after.x = field_camera_screen_x(&origin);
    after.y = field_camera_screen_y(&origin);
    g_field_screen_scroll.x = after.x - before.x;
    g_field_screen_scroll.y = after.y - before.y;
}

/**
 * @brief Ease the scroll limits back to the whole map width.
 */
inline void field_camera_release_scroll_limits(void)
{
    g_field_scroll_limit_left_target = 0;
    D_8010AE58 = FIELD_SCROLL_EASE_FRAMES;
    g_field_scroll_limit_right_target = FIELD_MAP_BOUNDS->width;
}

/**
 * @brief Ease the scroll limits to the bounds of the active actor group.
 * @note An unknown group, or one narrower than the view, locks the scroll to the current screen.
 */
void field_camera_select_scroll_limits(void)
{
    SceneState* camera;
    FieldGroupBounds* entry;
    s32 group;
    FieldGroupBounds* group_bounds;
    s32 entry_index;

    camera = SCENE_STATE;
    group = g_field_active_group;
    if (group == 0)
    {
        field_camera_release_scroll_limits();
        return;
    }

    if (g_field_group_bounds_count < group)
    {
        field_camera_lock_scroll(camera);
        return;
    }

    /* indexing g_field_group_bounds[group - 1] directly changes the codegen */
    group_bounds = g_field_group_bounds;
    entry_index = group - 1;
    entry = &group_bounds[entry_index];
    if (entry->width < FIELD_VIEW_WIDTH)
    {
        field_camera_lock_scroll(camera);
        return;
    }

    D_8010AE58 = FIELD_SCROLL_EASE_FRAMES;
    g_field_scroll_limit_left_target = entry->left;
    g_field_scroll_limit_right_target = entry->left + entry->width;
}

/**
 * @brief Move the scroll limits one step toward their targets.
 */
static void field_camera_step_scroll_limits(void)
{
    if (D_8010AE58 != 0)
    {
        g_field_scroll_limit_left += (g_field_scroll_limit_left_target - g_field_scroll_limit_left) / D_8010AE58;
        g_field_scroll_limit_right += (g_field_scroll_limit_right_target - g_field_scroll_limit_right) / D_8010AE58;
        D_8010AE58 -= 1;
    }
}

/**
 * @brief Move the scripted scroll position one step toward its target, snapping when no frames remain.
 */
static void field_camera_step_scripted_scroll(void)
{
    if (g_field_scripted_scroll_frames != 0)
    {
        g_field_scripted_scroll_x += (g_field_scripted_scroll_target_x - g_field_scripted_scroll_x) / g_field_scripted_scroll_frames;
        g_field_scripted_scroll_z += (g_field_scripted_scroll_target_z - g_field_scripted_scroll_z) / g_field_scripted_scroll_frames;
        g_field_scripted_scroll_frames -= 1;
    }
    else
    {
        g_field_scripted_scroll_x = g_field_scripted_scroll_target_x;
        g_field_scripted_scroll_z = g_field_scripted_scroll_target_z;
    }
}

/**
 * @brief Reset the scroll limits to the whole map width and clear the scripted scroll.
 */
void field_camera_reset(void)
{
    s32 map_width;

    map_width = FIELD_MAP_BOUNDS->width;

    g_field_scroll_limit_left_target = 0;
    g_field_scroll_limit_left = 0;
    D_8010AE58 = 0;
    g_field_scripted_scroll_z = 0;
    g_field_scripted_scroll_x = 0;
    g_field_scripted_scroll_target_z = 0;
    g_field_scripted_scroll_target_x = 0;
    g_field_scripted_scroll_frames = 0;
    g_field_scroll_limit_right = map_width;
    g_field_scroll_limit_right_target = map_width;
}
