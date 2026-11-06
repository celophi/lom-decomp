#ifndef WMAP_SEQUENCE_RUNTIME_H
#define WMAP_SEQUENCE_RUNTIME_H

#include "common.h"
#include <libgte.h>

#include "wmap_sprite_render.h"

/**
 * @brief Sequence callback: nonzero initializes; zero advances one update.
 *
 * Land effects, land events and special effects build their animation from step
 * sequences of this type. Each sequence owns a step global and a timer global, and
 * a runner that calls entry `step` of its step table once per frame while `step` is
 * in range. Their names follow one pattern, per source file:
 * - `<file>_run`, `<file>_run_timeline`, `<file>_run_sequence_K`: the runners;
 *   `g_<file>[_timeline|_sequence_K]_step` and `_timer` are their globals.
 * - `_reset` (entry 0) restarts at step 1 with a one-frame timer.
 * - `_wait_NN` counts the timer down, then advances; `_wait_idle` waits for
 *   g_wmap_sequence_busy to clear.
 * - `_end` (last entry) advances past the table, which stops the runner.
 * - `_step_NN` is any other step; NN is its index in the step table.
 * The timeline is the sequence that clears g_wmap_sequence_busy when it finishes.
 * @see wmap_step_sequence.h for the macros that define the runners and the
 *      reset, wait, wait_idle and end steps.
 */
typedef s32 (*WmapSequenceCallback)(s32 initialize);

/** @brief Entry of a sequence's step table; the runner calls the entry for the current step. */
typedef void (*WmapSequenceStep)(void);

/**
 * @brief Nonzero while a blocking sequence runs (the land focus or an effect timeline).
 * @note The step that starts the sequence sets it; the sequence's last step clears it.
 */
extern s32 g_wmap_sequence_busy;

s32 wmap_run_land_focus(s32 reset);
void wmap_update_sequences(void);
void wmap_start_sequence(WmapSequenceCallback callback);
void wmap_update_callbacks(void);
void wmap_install_callback(WmapSequenceCallback callback);
s32 wmap_step_actor_animation(void* actor_data, void* resource_data);
void wmap_init_sequences(void);
void wmap_draw_model_default(u8* resource_table, s32 resource_index, s32 ot_index, s32 tpage, s32 clut, s32 blend_mode, s32 color_scale);
void wmap_project_focus_cell(void);
s32 wmap_scale_color(CVECTOR color, s32 scale);
void wmap_set_model_transform(VECTOR* translation, SVECTOR* rotation);
void wmap_update_default_particles_from_settings(void* actors, WmapAnimationSlot* animations, s32 count, s32 peak_shade, s32 initial_shade, s32 shade_step);
void wmap_update_particles_from_settings(void* actors, WmapAnimationSlot* animations, s32 count, s32 peak_shade, s32 initial_shade, s32 shade_step, s32 mode);
s32 wmap_find_land_cell(s32 value, s32* row_out, s32* column_out);
void wmap_set_map_rotation(SVECTOR* rotation);
void wmap_reset_focus_screen_position(void);
s32 wmap_run_land_entry(s32 reset);

/* Constant transforms (all zero; never written). */
/** @brief Zero rotation: the plain map rotation, and the value effects reset their model rotations to. */
extern SVECTOR g_wmap_zero_rotation;
/** @brief Zero translation used for model matrices and to reset effect positions. */
extern VECTOR g_wmap_zero_translation;

/* Effect model slots a-h. Land effects share them because only one effect runs at a time;
 * slots a and b hold the model drops, c and d the spins (see wmap_special_effect_35.c). */
/** @brief Rotation of effect model slot a. */
extern SVECTOR g_wmap_effect_model_a_rotation;
/** @brief Position of effect model slot a; a drop starts it at the camera translation and lowers .vz. */
extern VECTOR g_wmap_effect_model_a_position;
/**
 * @brief Rotation of effect model slot b.
 * @note The exit transition reuses it as the screen panels' per-frame rotation step.
 */
extern SVECTOR g_wmap_effect_model_b_rotation;
/**
 * @brief Position of effect model slot b.
 * @note The exit transition reuses it as the screen panels' per-frame translation step.
 */
extern VECTOR g_wmap_effect_model_b_position;
/** @brief Rotation of effect model slot c; the exit transition reuses it for the first screen panel. */
extern SVECTOR g_wmap_effect_model_c_rotation;
/** @brief Position of effect model slot c; the exit transition reuses it for the first screen panel. */
extern VECTOR g_wmap_effect_model_c_position;
/** @brief Rotation of effect model slot d; the exit transition reuses it for the second screen panel. */
extern SVECTOR g_wmap_effect_model_d_rotation;
/** @brief Position of effect model slot d; the exit transition reuses it for the second screen panel. */
extern VECTOR g_wmap_effect_model_d_position;
/** @brief Rotation of effect model slot e, drawn at the camera translation; the exit transition reuses it as a panel corner. */
extern SVECTOR g_wmap_effect_model_e_rotation;
/** @brief Rotation of effect model slot f, drawn at the camera translation. */
extern SVECTOR g_wmap_effect_model_f_rotation;
/** @brief Rotation of effect model slot g. */
extern SVECTOR g_wmap_effect_model_g_rotation;
/** @brief Position of effect model slot g. */
extern VECTOR g_wmap_effect_model_g_position;
/** @brief Rotation of effect model slot h. */
extern SVECTOR g_wmap_effect_model_h_rotation;
/** @brief Position of effect model slot h. */
extern VECTOR g_wmap_effect_model_h_position;
/** @brief Effect model fade level (draw color scale), usually for slot a. */
extern s32 g_wmap_effect_fade_a;
/** @brief Effect model fade level (draw color scale), usually for slot b. */
extern s32 g_wmap_effect_fade_b;
/** @brief Effect model fade level (draw color scale), usually for slot c. */
extern s32 g_wmap_effect_fade_c;
/** @brief Effect model fade level (draw color scale), usually for slot d. */
extern s32 g_wmap_effect_fade_d;
/** @brief Auxiliary fade level shared by model, sprite and particle sequences. */
extern s32 g_wmap_aux_effect_fade_a;
/** @brief Auxiliary fade level shared by model, sprite and particle sequences. */
extern s32 g_wmap_aux_effect_fade_b;

/* Map state the land effects, events and the main loop share. */
/** @brief Map cell x (first g_wmap_cells index) of the land being placed or focused. */
extern s32 g_wmap_focus_cell_x;
/** @brief Map cell y (second g_wmap_cells index) of the land being placed or focused. */
extern s32 g_wmap_focus_cell_y;
/** @brief Nonzero while a scripted map event (placement, land effect, travel, story sequence) owns the map. */
extern s32 g_wmap_event_active;
/** @brief Nonzero once a land effect has taken over the map: hides the carried artifact and the placement highlights. */
extern s32 g_wmap_placement_overlay_hidden;
/** @brief When 1, the transition mesh is neither animated nor drawn. */
extern s32 g_wmap_transition_mesh_hidden;
/** @brief Frames rendered since the world map started; bit 0 picks the draw buffer. */
extern s32 g_wmap_frame_count;
/** @brief Emission level of the shared particle pool: live-particle cap for emitters, and the burst's scale. */
extern s32 g_wmap_particle_intensity;

/** @brief Per-group spawn timers and remaining spawn counts of wmap_update_falling_particles. */
extern s32 g_wmap_falling_spawn_timers[4];
extern s32 g_wmap_falling_spawn_counts[4];

/**
 * @brief Particle settings that wmap_update_particles_from_settings copies into a WmapParticleConfig.
 * @note Effects that run no particles reuse these words as their own counters, frame indexes and fade levels.
 */
extern s32 g_wmap_particle_velocity_min;
extern s32 g_wmap_particle_velocity_range;
extern s32 g_wmap_particle_lifetime_min;
extern s32 g_wmap_particle_lifetime_range;
extern s32 g_wmap_particle_spawn_interval;
extern s32 g_wmap_particle_radial_step;
extern s32 g_wmap_particle_first_motion;
extern s32 g_wmap_particle_texture_index;
extern s32 g_wmap_particle_sequence;
extern s32 g_wmap_particle_radius;

/**
 * @brief Set all ten particle settings read by wmap_update_particles_from_settings.
 * @param velocity_min g_wmap_particle_velocity_min.
 * @param velocity_range g_wmap_particle_velocity_range.
 * @param lifetime_min g_wmap_particle_lifetime_min.
 * @param lifetime_range g_wmap_particle_lifetime_range.
 * @param spawn_interval g_wmap_particle_spawn_interval.
 * @param radial_step g_wmap_particle_radial_step.
 * @param first_motion g_wmap_particle_first_motion.
 * @param texture_index g_wmap_particle_texture_index.
 * @param sequence g_wmap_particle_sequence.
 * @param radius g_wmap_particle_radius.
 */
#define WMAP_SET_PARTICLE_SETTINGS(velocity_min, velocity_range, lifetime_min, lifetime_range, spawn_interval, radial_step, first_motion, texture_index,       \
                                   sequence, radius)                                                                                                           \
    (g_wmap_particle_velocity_min = (velocity_min), g_wmap_particle_velocity_range = (velocity_range), g_wmap_particle_lifetime_min = (lifetime_min),          \
     g_wmap_particle_lifetime_range = (lifetime_range), g_wmap_particle_spawn_interval = (spawn_interval), g_wmap_particle_radial_step = (radial_step),        \
     g_wmap_particle_first_motion = (first_motion), g_wmap_particle_texture_index = (texture_index), g_wmap_particle_sequence = (sequence),                    \
     g_wmap_particle_radius = (radius))

/** @brief Effect model fade levels (draw color scale) that land effects share; fade e also serves as a frame counter. */
extern s32 g_wmap_effect_fade_e;
extern s32 g_wmap_effect_fade_f;
/** @brief Auxiliary fade level shared by model, sprite and particle sequences. */
extern s32 g_wmap_aux_effect_fade_c;
/** @brief Flash fade levels; effect 18 restarts flash b from flash a, which falls with each flash. */
extern s32 g_wmap_effect_flash_fade_a;
extern s32 g_wmap_effect_flash_fade_b;
/** @brief Index into g_wmap_cosine_wave that wraps at 256; some effects use it as a fade level instead. */
extern s32 g_wmap_effect_wave_phase;
/** @brief Model frame drawn by wmap_draw_model_default in the model loop effects. */
extern s32 g_wmap_effect_model_frame;

/**
 * @brief Arc settings of wmap_update_particle_system modes 1-3.
 * @note Mode 2 (parabola): height = base + peak - (lifetime - apex_lifetime)^2 / divisor.
 * @note Mode 1 (rise then fall): while lifetime > apex_lifetime the particle rises by peak plus a random part of divisor,
 *       then falls by base plus a random part of g_wmap_particle_fall_range.
 * @note Mode 3: height = (apex_lifetime - lifetime)^2 / peak.
 */
extern s32 g_wmap_particle_arc_apex_lifetime;
extern s32 g_wmap_particle_arc_peak;
extern s32 g_wmap_particle_arc_divisor;
extern s32 g_wmap_particle_arc_base;
extern s32 g_wmap_particle_fall_range;

/**
 * @brief Screen positions of effect sprites; event 17 uses a-i for its nine scattered sprites.
 * @note Effect 34 keeps sprite b's path in 1/16 pixels and draws it at position a.
 */
extern WmapScreenPosition g_wmap_effect_sprite_position_a;
extern WmapScreenPosition g_wmap_effect_sprite_position_b;
extern WmapScreenPosition g_wmap_effect_sprite_position_c;
extern s32 g_wmap_carousel_artifacts[];
extern s32 g_wmap_models_use_effect_camera;
/** @brief Last map-game score, or the active count of land effects 1 and 13; nothing reads it. */
extern s32 g_wmap_unused_tally;
/** @brief 127 * cos(2 * pi * i / 128): two full periods over 256 entries. */
extern s8 g_wmap_cosine_wave[256];
extern s32 g_wmap_sequence_count;

#endif
