#ifndef WMAP_ACTOR_MOTION_H
#define WMAP_ACTOR_MOTION_H

#include "common.h"
#include "wmap_sprite_render.h"

/** @brief Spin rate stored as a word; the spiral particle setups write only its low halfword. */
typedef union
{
    s32 value;
    s16 low;
} WmapAngularVelocity;

/**
 * @brief Radial particle motion in one g_wmap_actor_motions slot.
 * @note A slot is free while @c state is zero.
 */
typedef struct
{
    s16 state;
    s16 angle;
    s32 velocity;
    s32 radius;
    s16 lifetime;
    s16 z;
    WmapAngularVelocity angular_velocity;
} WmapMotion;

/** @brief Radial trail motion that caches its projected screen position in place of the spin rate. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 velocity;
    s32 radius;
    s16 lifetime;
    s16 z;
    WmapScreenPosition screen_position;
} WmapTrailMotion;

/** @brief Motion slots, parallel to g_wmap_sprite_actors and g_wmap_actor_animations. */
extern WmapMotion g_wmap_actor_motions[];

#endif
