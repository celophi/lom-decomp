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

/** @brief Effect camera rotation offset that wmap_run_effect_camera_move steps toward. */
extern SVECTOR g_wmap_effect_camera_rotation_target;
/** @brief Per-frame rotation step of the effect camera move. */
extern SVECTOR g_wmap_effect_camera_rotation_step;
/** @brief Effect camera translation offset that wmap_run_effect_camera_move steps toward. */
extern VECTOR g_wmap_effect_camera_translation_target;
/** @brief Per-frame translation step of the effect camera move. */
extern VECTOR g_wmap_effect_camera_translation_step;
/** @brief Nonzero while wmap_run_effect_camera_move runs; clearing it stops the move and resets the offsets. */
extern s32 g_wmap_effect_camera_move_active;

void wmap_update_particle_system(WmapSpriteActor* actors, WmapAnimationSlot* animations, s32 count, s32 peak_shade,
                                 s32 initial_shade, s32 shade_step, u32 mode, void* params);
void wmap_update_radial_burst(WmapSpriteActor* actors, WmapAnimationSlot* animations, s32 first, s32 end, s32 scale,
                   s32 velocity_min, s32 velocity_range, s32 lifetime_min, s32 lifetime_range, s32 initial_z,
                   s32 frame, s32 spawn_interval);
void wmap_set_effect_model_transform(VECTOR* translation, SVECTOR* rotation);
void wmap_set_effect_camera_transform(void);
void wmap_update_flat_radial_burst(s32 first, s32 end, s32 frame, s32 depth,
                   s32 velocity_base, s32 velocity_divisor, s32 lifetime_base, s32 sequence);
void wmap_update_falling_particles(s32 first, s32 end, s32 spawn_interval, s32 scale_override, s32 velocity_min, s32 velocity_range, s32 initial_z, s32 frame, s32 x_min, s32 x_range, s32 y_min, s32 y_range, s32 fade_z, s32 initial_scale_target, s32 initial_scale, s32 scale_step, s32 group);
void wmap_update_spiral_particles(s32 first, s32 end, s32 frame, s32 z_step, s32 depth);
void wmap_update_bouncing_particles(s32 first, s32 end, void* point_data, s32 frame, s32 depth);
void wmap_update_particle_trails(s32 first, s32 count, WmapTrailConfig* config, s32 expire_by_shade);
s32 wmap_run_effect_camera_move(s32 initialize);
void wmap_update_box_particles(WmapBoxParticleConfig* config);

#endif
