#ifndef WMAP_LAND_TRANSITION_H
#define WMAP_LAND_TRANSITION_H

#include "common.h"
#include "sdk/libgte.h"

#define WMAP_ARTIFACT_SLOTS 12
#define WMAP_CAROUSEL_STEP_FRAMES 4
#define WMAP_CAROUSEL_FRAMES (WMAP_ARTIFACT_SLOTS * WMAP_CAROUSEL_STEP_FRAMES)

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

#endif
