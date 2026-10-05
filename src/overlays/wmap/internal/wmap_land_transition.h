#ifndef WMAP_LAND_TRANSITION_H
#define WMAP_LAND_TRANSITION_H

#include "common.h"
#include "wmap_frame_render.h"
#include <libgte.h>

#define WMAP_ARTIFACT_SLOTS 12
#define WMAP_PLACEMENT_RESOURCE_BASE 0x10CE
#define WMAP_CAROUSEL_STEP_FRAMES 4
#define WMAP_CAROUSEL_FRAMES (WMAP_ARTIFACT_SLOTS * WMAP_CAROUSEL_STEP_FRAMES)

#define WMAP_CELL_SPACING 48
#define WMAP_VIEW_COLUMNS 3
#define WMAP_PREVIEW_FRAME_BOUND_COUNT (WMAP_VIEW_COLUMNS * WMAP_VIEW_COLUMNS + 1)
#define WMAP_PREVIEW_CENTER_CELL_INDEX (WMAP_VIEW_COLUMNS * WMAP_VIEW_COLUMNS / 2)
#define WMAP_ARTIFACT_TPAGE 0xAE
#define WMAP_ARTIFACT_CLUT 0x7FEC
#define WMAP_NO_ARTIFACT (-1)

/** @brief Carousel slide and visibility modes. */
enum WmapCarouselMode
{
    WMAP_CAROUSEL_VISIBLE,
    WMAP_CAROUSEL_SLIDE_OUT,
    WMAP_CAROUSEL_SLIDE_IN,
    WMAP_CAROUSEL_HIDDEN
};

/** @brief Map selection panel animation phases. */
enum WmapSelectionPhase
{
    WMAP_SELECTION_MAP,
    WMAP_SELECTION_OPENING,
    WMAP_SELECTION_ARTIFACTS,
    WMAP_SELECTION_SHOW_ARTIFACTS,
    WMAP_SELECTION_SHOW_MAP
};

/** @brief Carousel rotation angle and its stored padding. */
typedef struct
{
    u16 angle;
    u16 pad;
} WmapCarouselAngle;

/** @brief Artifact texture size and offsets for carousel, shadow, and map drawing. */
typedef struct
{
    u8 u;
    u8 pad_01;
    u8 v;
    u8 pad_03;
    u16 width;
    u16 height;
    u16 carousel_x;
    u16 carousel_y;
    u16 shadow_x;
    u16 shadow_y;
    s16 offset_x;
    s16 offset_y;
    s16 anchor_x;
    s16 anchor_y;
} WmapArtifactImage;

extern s32 g_wmap_carousel_frame;
extern WmapCarouselAngle g_wmap_carousel_angles[];
extern SVECTOR g_wmap_carousel_rotation;
extern WmapArtifactImage g_wmap_artifact_images[];

void wmap_update_artifact_selection(void);
void wmap_draw_artifact_carousel(void);

/** @brief Map and artifact selection-panel animation phase. */
extern s32 g_wmap_selection_phase;
/** @brief Visibility and slide direction of the artifact carousel. */
extern s32 g_wmap_carousel_mode;

/** @brief Preview travel-frame boundaries for the nine map view cells. */
extern const s32 g_wmap_preview_travel_frame_bounds[WMAP_PREVIEW_FRAME_BOUND_COUNT];

#endif
