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
    u8 unused_16[2]; /**< High halfword of the angle_step parameter word; only the low halfword is read. */
    u16 shade_step;
    u8 unused_1a[2]; /**< High halfword of the shade_step parameter word; only the low halfword is read. */
    u16 target_shade;
    u8 unused_1e[2]; /**< High halfword of the target_shade parameter word; only the low halfword is read. */
    s32 depth;
    s32 texture_index;
} WmapTrailConfig;

/** @brief Spawn box, actor range, animation and shade settings for particles. */
typedef struct
{
    s32 count;
    s32 first;
    u16 x_min;
    u8 unused_0a[2];
    s32 x_range;
    u16 y_min;
    u8 unused_12[2];
    s32 y_range;
    u16 z_min;
    u8 unused_1a[2];
    s32 z_range;
    u16 target_shade;
    u8 unused_22[2];
    u16 shade;
    u8 unused_26[2];
    u16 shade_step;
    u8 unused_2a[2];
    s32 depth;
    s32 texture_index;
    u16 sequence;
    u8 unused_36[2];
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

/** @brief Particle settings that wmap_update_particle_system reads from the shared effect parameter words. */
typedef struct
{
    s32 unused_00; /**< Word 0 of the config; neither WMAP_SET_PARTICLE_CONFIG nor wmap_update_particle_system uses it. */
    s32 velocity_min;
    s32 velocity_range;
    s32 lifetime_min;
    s32 lifetime_range;
    /** @brief Frames between spawns, or -1 to stop spawning. */
    s32 spawn_interval;
    /** @brief Radius added each frame. */
    s32 radial_step;
    /** @brief First g_wmap_actor_motions slot of the particles. */
    s32 first_motion;
    s32 texture_index;
    s32 sequence;
    /** @brief Spawn radius; negative packs a minimum (bits 16-30) and a range (bits 0-15). */
    s32 radius;
} WmapParticleConfig;

/**
 * @brief Fill the WmapParticleConfig that starts at g_wmap_effect_params[@p base], except its first word.
 * @param base Index of the config's first word in g_wmap_effect_params.
 * @param velocity_min WmapParticleConfig.velocity_min.
 * @param velocity_range WmapParticleConfig.velocity_range.
 * @param lifetime_min WmapParticleConfig.lifetime_min.
 * @param lifetime_range WmapParticleConfig.lifetime_range.
 * @param spawn_interval WmapParticleConfig.spawn_interval.
 * @param radial_step WmapParticleConfig.radial_step.
 * @param first_motion WmapParticleConfig.first_motion.
 * @param texture_index WmapParticleConfig.texture_index.
 * @param sequence WmapParticleConfig.sequence.
 * @param radius WmapParticleConfig.radius.
 */
#define WMAP_SET_PARTICLE_CONFIG(base, velocity_min, velocity_range, lifetime_min, lifetime_range, spawn_interval, radial_step, first_motion, texture_index,   \
                                 sequence, radius)                                                                                                             \
    (g_wmap_effect_params[(base) + 1] = (velocity_min), g_wmap_effect_params[(base) + 2] = (velocity_range),                                                   \
     g_wmap_effect_params[(base) + 3] = (lifetime_min), g_wmap_effect_params[(base) + 4] = (lifetime_range),                                                   \
     g_wmap_effect_params[(base) + 5] = (spawn_interval), g_wmap_effect_params[(base) + 6] = (radial_step), g_wmap_effect_params[(base) + 7] = (first_motion), \
     g_wmap_effect_params[(base) + 8] = (texture_index), g_wmap_effect_params[(base) + 9] = (sequence), g_wmap_effect_params[(base) + 10] = (radius))

void wmap_update_particle_system(WmapSpriteActor* actors, WmapAnimationSlot* animations, s32 count, s32 peak_shade, s32 initial_shade, s32 shade_step, u32 mode,
                                 void* params);
void wmap_update_radial_burst(WmapSpriteActor* actors, WmapAnimationSlot* animations, s32 first, s32 end, s32 scale, s32 velocity_min, s32 velocity_range,
                              s32 lifetime_min, s32 lifetime_range, s32 initial_z, s32 frame, s32 spawn_interval);
void wmap_set_effect_model_transform(VECTOR* translation, SVECTOR* rotation);
void wmap_set_effect_camera_transform(void);
void wmap_update_flat_radial_burst(s32 first, s32 end, s32 frame, s32 depth, s32 velocity_base, s32 velocity_divisor, s32 lifetime_base, s32 sequence);
void wmap_update_falling_particles(s32 first, s32 end, s32 spawn_interval, s32 scale_override, s32 velocity_min, s32 velocity_range, s32 initial_z, s32 frame,
                                   s32 x_min, s32 x_range, s32 y_min, s32 y_range, s32 fade_z, s32 initial_scale_target, s32 initial_scale, s32 scale_step,
                                   s32 group);
void wmap_update_spiral_particles(s32 first, s32 end, s32 frame, s32 z_step, s32 depth);
void wmap_update_bouncing_particles(s32 first, s32 end, void* point_data, s32 frame, s32 depth);
void wmap_update_particle_trails(s32 first, s32 count, WmapTrailConfig* config, s32 expire_by_shade);
s32 wmap_run_effect_camera_move(s32 initialize);
void wmap_update_box_particles(WmapBoxParticleConfig* config);

extern MATRIX g_wmap_effect_camera_matrix;

/** @brief Particle and emitter parameter words of the running effect; WMAP_SET_PARTICLE_CONFIG fills them. */
extern s32* g_wmap_effect_params;

/** @brief Shade given to new particles; the fade-out steps lower it. */
extern s32 g_wmap_particle_shade;

#endif
