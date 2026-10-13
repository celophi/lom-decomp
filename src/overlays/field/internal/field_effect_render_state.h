#ifndef FIELD_EFFECT_RENDER_STATE_H
#define FIELD_EFFECT_RENDER_STATE_H

#include "common.h"
#include "main/display.h"
#include "common/vector.h"

/** @brief Track-driven scale in GTE fixed-point units, without SVECTOR padding. */
typedef struct
{
    s16 x;
    s16 y;
    s16 z;
} FieldEffectTrackScale;

extern FieldEffectTrackScale g_field_effect_track_scale;
extern u8 g_field_ribbon_frame_flags[];
extern u16 g_field_ribbon_uv_corners[];

/** @brief Camera translation added to effect coordinates, with eight fractional bits. */
extern s32 g_field_view_offset_x;
extern s32 g_field_view_offset_y;
extern s32 g_field_view_offset_z;

/** @brief Screen position of the camera origin when a world position is projected. */
#define FIELD_SCREEN_CENTER_X (SCREEN_WIDTH / 2)
#define FIELD_SCREEN_CENTER_Y (VRAM_DRAW_HEIGHT / 2)

/** @brief On-screen movement of the field view during the last camera update, in pixels. */
extern Vec2s g_field_screen_scroll;

/** @brief Camera offset added to the scrolled view (whole units). */
extern s32 g_field_camera_offset_x;
extern s32 g_field_camera_offset_y;
extern s32 g_field_camera_offset_z;
/** @brief Average party position the camera aims at, and the follow point that moves toward it. */
extern s32 g_field_camera_target_x;
extern s32 g_field_camera_target_z;
extern s32 g_field_camera_follow_x;
extern s32 g_field_camera_follow_z;

/** @brief Horizontal scroll range of an actor group, in pixels. */
typedef struct
{
    u16 left;
    u16 width;
} FieldGroupBounds;

/** @brief Scroll range of each actor group (group n uses entry n - 1), and the number of entries. */
extern FieldGroupBounds g_field_group_bounds[];
extern s32 g_field_group_bounds_count;

/** @brief Scroll limits of the view (pixels) and the targets they ease toward. */
extern s32 g_field_scroll_limit_left;
extern s32 g_field_scroll_limit_right;
extern s32 g_field_scroll_limit_left_target;
extern s32 g_field_scroll_limit_right_target;

/** @brief Scripted scroll: frames left, current position and target position (pixels). */
extern s32 g_field_scripted_scroll_frames;
extern s32 g_field_scripted_scroll_x;
extern s32 g_field_scripted_scroll_z;
extern s32 g_field_scripted_scroll_target_x;
extern s32 g_field_scripted_scroll_target_z;

/** @brief Camera-relative bounds (eight fractional bits) of the visible play area. */
#define FIELD_VIEW_MARGIN_LEFT 0xA00
#define FIELD_VIEW_MARGIN_RIGHT 0x13600
#define FIELD_VIEW_MARGIN_BOTTOM 0x1B600

#endif
