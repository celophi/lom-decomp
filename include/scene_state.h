#ifndef _SCENE_STATE_H
#define _SCENE_STATE_H

#include "common.h"

/** @brief Scene selection, camera position, and pixel lookup retained across overlays. */
typedef struct
{
    u16 map_id;
    u16 object_index;
    s32 camera_x;
    s32 camera_y;
    s32 camera_z;
    u32 pixel_lookup_selector;
} SceneState;

/** @brief Scene state block kept at a fixed RAM address across overlays. */
#define SCENE_STATE ((SceneState*)0x801ED480)

#endif
