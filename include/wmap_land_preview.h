#ifndef WMAP_LAND_PREVIEW_H
#define WMAP_LAND_PREVIEW_H

#include "common.h"
#include "sdk/libgpu.h"

#define WMAP_PREVIEW_QUADS_PER_BUFFER 2

/** @brief Last preview layers for each of the two display buffers. */
extern POLY_FT4 g_wmap_preview_saved_quads[2 * WMAP_PREVIEW_QUADS_PER_BUFFER];
extern s32 g_wmap_preview_shape;
extern s32 g_wmap_preview_x;
extern s32 g_wmap_preview_y;
extern s32 g_wmap_preview_draw_x;
extern s32 g_wmap_preview_draw_y;
extern s32 g_wmap_preview_bob_frame;
extern s32 g_wmap_preview_texture;

extern s32 g_wmap_artifact_transfer_frame;
extern s32 g_wmap_artifact_transfer_end;
extern s32 g_wmap_preview_travel_frame;
extern s32 g_wmap_preview_travel_end;
extern s32 g_wmap_preview_artifact_visible;

void wmap_update_land_preview(void);

#endif
