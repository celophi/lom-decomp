#ifndef WMAP_EFFECT_PRIMITIVES_H
#define WMAP_EFFECT_PRIMITIVES_H

#include "common.h"
#include <libgte.h>
#include "wmap_sprite_render.h"

/** @brief Trail timing and rendering view of the shared effect parameters. */
typedef struct
{
    s32 spawn_interval;
    s32 next_slot;
    s32 spawn_timer;
    s32 initial_angle;
    s32 radial_step;
    u16 angle_step;
    u8 unknown_16[2];
    u16 shade_step;
    u8 unknown_1a[2];
    u16 target_shade;
    u8 unknown_1e[2];
    s32 depth;
    s32 texture_index;
} WmapTrailConfig;

/** @brief Spawn box, actor range, animation and shade settings for particles. */
typedef struct
{
    s32 count;
    s32 first;
    u16 x_min;
    u8 unknown_0a[2];
    s32 x_range;
    u16 y_min;
    u8 unknown_12[2];
    s32 y_range;
    u16 z_min;
    u8 unknown_1a[2];
    s32 z_range;
    u16 target_shade;
    u8 unknown_22[2];
    u16 shade;
    u8 unknown_26[2];
    u16 shade_step;
    u8 unknown_2a[2];
    s32 depth;
    s32 texture_index;
    u16 sequence;
    u8 unknown_36[2];
} WmapBoxParticleConfig;

void func_8006A2FC(void* arg0, WmapAnimationSlot* animations, s32 arg2, s32 arg3, s32 arg4_value, s32 arg5_value, u32 arg6, void* arg7);
void func_8006A9C4(WmapSpriteActor* actors, WmapAnimationSlot* animations, s32 first, s32 end, s32 scale,
                   s32 velocity_min, s32 velocity_range, s32 lifetime_min, s32 lifetime_range, s32 initial_z,
                   s32 frame, s32 spawn_interval);
void wmap_set_effect_model_transform(VECTOR* translation, SVECTOR* rotation);
void wmap_set_effect_camera_transform(void);
void func_8006AFAC(s32 first, s32 end, s32 frame, s32 depth,
                   s32 velocity_base, s32 velocity_divisor, s32 lifetime_base, s32 sequence);
void func_8006B328(s32 first, s32 end, s32 spawn_interval, s32 scale_override, s32 velocity_min, s32 velocity_range, s32 initial_z, s32 frame, s32 x_min, s32 x_range, s32 y_min, s32 y_range, s32 fade_z, s32 initial_scale_target, s32 initial_scale, s32 scale_step, s32 group);
void func_8006B6EC(s32 first, s32 end, s32 frame, s32 z_step, s32 depth);
void func_8006B998(s32 first, s32 end, void* point_data, s32 frame, s32 depth);
void func_8006BC44(s32 first, s32 count, WmapTrailConfig* config, s32 expire_by_shade);
s32 func_8006C0EC(void);
void func_8006C448(WmapBoxParticleConfig* config);

#endif
