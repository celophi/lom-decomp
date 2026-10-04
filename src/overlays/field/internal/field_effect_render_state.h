#ifndef FIELD_EFFECT_RENDER_STATE_H
#define FIELD_EFFECT_RENDER_STATE_H

#include "common.h"

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

#endif
