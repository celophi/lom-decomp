#ifndef FIELD_EFFECT_RENDER_STATE_H
#define FIELD_EFFECT_RENDER_STATE_H

#include "common.h"
#include "main/display.h"

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

/** @brief Camera-relative bounds (eight fractional bits) of the visible play area. */
#define FIELD_VIEW_MARGIN_LEFT 0xA00
#define FIELD_VIEW_MARGIN_RIGHT 0x13600
#define FIELD_VIEW_MARGIN_BOTTOM 0x1B600

#endif
