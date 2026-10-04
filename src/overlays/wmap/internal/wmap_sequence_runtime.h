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
void func_8006CFE4(void* actor, WmapAnimationSlot* resource, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void func_8006D014(void* actor, WmapAnimationSlot* resource, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);
s32 wmap_find_land_cell(s32 value, s32* row_out, s32* column_out);
void wmap_set_map_rotation(SVECTOR* rotation);
void wmap_reset_focus_screen_position(void);
s32 wmap_run_land_entry(s32 reset);

/* Constant transforms (all zero; never written). */
/** @brief Zero rotation: the plain map rotation, and the value effects reset their model rotations to. */
extern SVECTOR g_wmap_zero_rotation;
/** @brief Zero translation used for model matrices and to reset effect positions. */
extern VECTOR g_wmap_zero_translation;

/* Effect model slots a-d. Land effects share them because only one effect runs at a time;
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

#endif
