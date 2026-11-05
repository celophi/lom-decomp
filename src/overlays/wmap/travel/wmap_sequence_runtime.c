#include "common.h"
#include <inline_c.h>

#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_land_transition.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_map_display.h"
#include "../internal/wmap_model_render.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"
#include "common/saved_game.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Sequence and callback slot counts, and the number of sprite actors. */
#define WMAP_SEQUENCE_SLOTS 14
#define WMAP_CALLBACK_SLOTS 8

/** @brief Step counts of the land focus, land entry and land return sequences. */
#define WMAP_LAND_FOCUS_STEPS 10
#define WMAP_LAND_ENTRY_STEPS 4
#define WMAP_LAND_RETURN_STEPS 6

extern s32 g_wmap_sequence_active[];
extern WmapSequenceCallback g_wmap_sequences[];
extern u32 g_wmap_land_entry_step;
extern u32 g_wmap_land_focus_step;
extern s32 g_wmap_land_focus_timer;
extern WmapSequenceStep g_wmap_land_focus_steps[WMAP_LAND_FOCUS_STEPS];
extern s32 g_wmap_callback_active[];
extern WmapSequenceCallback g_wmap_callbacks[];

extern s32* g_wmap_effect_params;
extern s32 g_wmap_land_entry_timer;
extern WmapSequenceStep g_wmap_land_entry_steps[WMAP_LAND_ENTRY_STEPS];
extern u32 g_wmap_land_return_step;
extern s32 g_wmap_land_return_timer;
extern WmapSequenceStep g_wmap_land_return_steps[WMAP_LAND_RETURN_STEPS];

/* Sequence steps: reached through the step tables and from the step before them. */
void wmap_land_entry_start_focus(void);
void wmap_land_focus_reset(void);
void wmap_land_focus_wait_2(void);
void wmap_land_focus_fade_out(void);
void wmap_land_focus_wait_4(void);
void wmap_land_focus_hold(void);
void wmap_land_focus_wait_6(void);
void wmap_land_focus_lock(void);
void wmap_land_focus_wait_8(void);
void wmap_land_entry_reset(void);
void wmap_land_entry_wait(void);
void wmap_land_entry_finish(void);
void wmap_land_return_reset(void);
void wmap_land_return_start(void);
void wmap_land_return_wait_sequences(void);
void wmap_land_return_scroll_back(void);
void wmap_land_return_wait_scroll(void);
void wmap_land_return_finish(void);
void wmap_set_camera_model_transform(VECTOR* translation, SVECTOR* rotation);
static s32 wmap_run_land_return(s32 reset);

/**
 * @brief Install and initialize a counted sequence in the first free slot.
 * @param callback Sequence callback, called with 1 to initialize and 0 to update.
 */
static inline void wmap_start_sequence_slot(WmapSequenceCallback callback)
{
    s32 i;

    for (i = 0; i < WMAP_SEQUENCE_SLOTS; i++)
    {
        if (g_wmap_sequence_active[i] == 0)
        {
            g_wmap_sequences[i] = callback;
            g_wmap_sequence_active[i] = 1;
            (g_wmap_sequences[i])(1);
            g_wmap_sequence_count++;
            break;
        }
    }

    if (i == WMAP_SEQUENCE_SLOTS)
    {
        func_80064F14();
    }
}

/**
 * @brief Land entry step 1: start the land focus sequence and mark the land effect as running.
 */
void wmap_land_entry_start_focus(void)
{
    wmap_start_sequence_slot(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_entry_step++;
    wmap_land_entry_wait();
}

/**
 * @brief Run the current step of the land focus sequence (scroll to the land, fade, project it).
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
WMAP_STEP_RUNNER(wmap_run_land_focus, g_wmap_land_focus_steps, WMAP_LAND_FOCUS_STEPS, g_wmap_land_focus_step, g_wmap_land_focus_timer)

/**
 * @brief Land focus step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_land_focus_reset, g_wmap_land_focus_step, g_wmap_land_focus_timer)

/**
 * @brief Land focus step 2: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_land_focus_wait_2, g_wmap_land_focus_step, g_wmap_land_focus_timer)

/**
 * @brief Land focus step 3: start the screen fade (not for FIELD_LAND_TREE_OF_MANA) and dim the spirit panel.
 */
void wmap_land_focus_fade_out(void)
{
    if (g_wmap_selected_artifact != FIELD_LAND_TREE_OF_MANA)
    {
        g_wmap_screen_fade_mode = WMAP_FADE_DECREASE;
    }

    g_wmap_status_panel_mode = 3;
    g_wmap_spirit_target_brightness = 0;
    g_wmap_land_focus_timer = 2;
    g_wmap_land_focus_step++;
}

/**
 * @brief Land focus step 4: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_land_focus_wait_4, g_wmap_land_focus_step, g_wmap_land_focus_timer)

/**
 * @brief Land focus step 5: wait four frames.
 */
void wmap_land_focus_hold(void)
{
    g_wmap_land_focus_timer = 4;
    g_wmap_land_focus_step++;
}

/**
 * @brief Land focus step 6: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_land_focus_wait_6, g_wmap_land_focus_step, g_wmap_land_focus_timer)

/**
 * @brief Land focus step 7: set the transition flags and wait eight frames.
 */
void wmap_land_focus_lock(void)
{
    g_wmap_land_display_limit = 0;
    g_wmap_event_active = 1;
    g_wmap_carousel_mode = 1;
    g_wmap_land_focus_timer = 8;
    g_wmap_land_focus_step++;
}

/**
 * @brief Land focus step 8: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_land_focus_wait_8, g_wmap_land_focus_step, g_wmap_land_focus_timer)

/**
 * @brief Update every running counted sequence; a sequence that returns 0 is freed and uncounted.
 */
void wmap_update_sequences(void)
{
    s32 i;
    s32 result;

    for (i = 0; i < WMAP_SEQUENCE_SLOTS; i++)
    {
        if (g_wmap_sequence_active[i] != 0)
        {
            result = g_wmap_sequences[i](0);
            g_wmap_sequence_active[i] = result;

            if (result == 0)
            {
                g_wmap_sequence_count--;
            }
        }
    }
}

/**
 * @brief Start a counted sequence in the first free slot (g_wmap_sequence_count blocks map input).
 * @param callback Callback, called with 1 to initialize and 0 to update.
 */
void wmap_start_sequence(WmapSequenceCallback callback)
{
    wmap_start_sequence_slot(callback);
}

/**
 * @brief Update every installed callback and keep the ones that return nonzero.
 */
void wmap_update_callbacks(void)
{
    s32 i;

    for (i = 0; i < WMAP_CALLBACK_SLOTS; i++)
    {
        if (g_wmap_callback_active[i] != 0)
        {
            g_wmap_callback_active[i] = g_wmap_callbacks[i](0);
        }
    }
}

/**
 * @brief Install an uncounted callback in the first free slot and run its initialization.
 * @param callback Callback, called with 1 to initialize and 0 to update.
 */
void wmap_install_callback(WmapSequenceCallback callback)
{
    s32 i;

    for (i = 0; i < WMAP_CALLBACK_SLOTS; i++)
    {
        if (g_wmap_callback_active[i] == 0)
        {
            g_wmap_callbacks[i] = callback;
            g_wmap_callback_active[i] = 1;
            g_wmap_callback_active[i] = g_wmap_callbacks[i](1);
            return;
        }
    }
}

/**
 * @brief Advance the selected animation, wrapping when its end marker is reached.
 * @param actor_data Actor animation state and frame pointers.
 * @param resource_data Animation block containing sequence and frame offsets.
 * @return New frame index, or -1 when the current frame is still active.
 */
s32 wmap_step_actor_animation(void* actor_data, void* resource_data)
{
    WmapSpriteActor* actor = actor_data;
    WmapAnimationSlot* resource = resource_data;
    u8* data;
    s16* offsets;
    u8* cursor;
    s32 result;
    s32 frame;

    result = -1;
    offsets = (s16*)resource->data;
    data = (u8*)offsets;

    if (actor->previous_sequence != actor->sequence)
    {
        actor->previous_sequence = actor->sequence;
        actor->sequence_start = (u8*)offsets + offsets[actor->sequence];
        actor->cursor = actor->sequence_start;
        actor->remaining = 1;
    }

    if (actor->remaining != WMAP_ANIMATION_HOLD)
    {
        actor->remaining--;
    }

    if (actor->remaining == 0)
    {
        cursor = actor->cursor;
        frame = cursor[0];
        actor->remaining = cursor[1];

        if (frame == WMAP_ANIMATION_LOOP)
        {
            cursor = actor->sequence_start;
            actor->cursor = cursor;
            frame = cursor[0];
            actor->remaining = cursor[1];
        }

        actor->cursor += 4;
        actor->frame_data = data + offsets[WMAP_ANIMATION_SEQUENCES + frame];
        result = frame;
    }

    return result;
}

/**
 * @brief Give every sprite actor and animation slot its index and clear all sequence and callback slots.
 */
void wmap_init_sequences(void)
{
    s32 i;

    for (i = 0; i < WMAP_ACTOR_COUNT; i++)
    {
        g_wmap_sprite_actors[i].unknown_00 = i;
        g_wmap_actor_animations[i].unknown_00 = i;
        g_wmap_sprite_actors[i].resource_index = -1;
    }

    for (i = WMAP_SEQUENCE_SLOTS - 1; i >= 0; i--)
    {
        g_wmap_sequence_active[i] = 0;
    }

    for (i = WMAP_CALLBACK_SLOTS - 1; i >= 0; i--)
    {
        g_wmap_callback_active[i] = 0;
    }
}

/**
 * @brief Draw a model with no screen offset and the default depth divisor.
 */
void wmap_draw_model_default(u8* resource_table, s32 resource_index, s32 ot_index, s32 tpage, s32 clut, s32 blend_mode, s32 color_scale)
{
    wmap_draw_model(resource_table, resource_index, ot_index, tpage, clut, blend_mode, color_scale, 0, 0, -1);
}

/**
 * @brief Project the focused land cell, relative to the map view, through the current GTE matrix.
 */
void wmap_project_focus_cell(void)
{
    SVECTOR position;

    position.vz = 0;
    position.vx = (((g_wmap_focus_cell_x - 1) * WMAP_CELL_SIZE - g_wmap_view.x * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) /
                  g_wmap_view.projection_scale;
    position.vy = (((g_wmap_focus_cell_y - 1) * WMAP_CELL_SIZE - g_wmap_view.y * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) /
                  g_wmap_view.projection_scale;
    gte_ldv0(&position);
    gte_rtps();
}

/**
 * @brief Scale packed RGB channels, preserving the high byte.
 * @param color Packed RGB value and high byte.
 * @param scale Scale in units of 1/128, or -1 to preserve the color.
 * @return Packed scaled color; channel results wrap to eight bits.
 */
s32 wmap_scale_color(CVECTOR color, s32 scale)
{
    /* The packet color word is passed by value and returned in packed form. */
    CVECTOR* channels = &color;
    s32 red;
    s32 green;
    s32 blue;

    if (scale == -1)
    {
        return *(s32*)&color;
    }
    else
    {
        red = channels->r * scale;
        green = channels->g * scale;
        blue = channels->b * scale;
        channels->r = red >> 7;
        channels->g = green >> 7;
        channels->b = blue >> 7;
        return *(s32*)&color;
    }
}

/**
 * @brief Install the model transform through the helper selected by g_wmap_models_use_effect_camera.
 */
void wmap_set_model_transform(VECTOR* translation, SVECTOR* rotation)
{
    if (g_wmap_models_use_effect_camera != 0)
    {
        wmap_set_effect_model_transform(translation, rotation);
        return;
    }

    wmap_set_camera_model_transform(translation, rotation);
}

/**
 * @brief Run wmap_update_particles_from_settings in motion mode 0.
 * @param actors Sprite actor records of the particles.
 * @param animations Animation resource slots, one per actor record.
 * @param count Number of particles.
 * @param peak_shade Target shade of a new particle.
 * @param initial_shade Shade of a new particle.
 * @param shade_step Shade step of a new particle.
 */
void wmap_update_default_particles_from_settings(void* actors, WmapAnimationSlot* animations, s32 count, s32 peak_shade, s32 initial_shade, s32 shade_step)
{
    wmap_update_particles_from_settings(actors, animations, count, peak_shade, initial_shade, shade_step, 0);
}

/**
 * @brief Load the particle settings into the shared effect parameters and run the particle system.
 * @param actors Sprite actor records of the particles.
 * @param animations Animation resource slots, one per actor record.
 * @param count Number of particles.
 * @param peak_shade Target shade of a new particle.
 * @param initial_shade Shade of a new particle.
 * @param shade_step Shade step of a new particle.
 * @param mode Motion mode passed to wmap_update_particle_system.
 */
void wmap_update_particles_from_settings(void* actors, WmapAnimationSlot* animations, s32 count, s32 peak_shade, s32 initial_shade, s32 shade_step, s32 mode)
{
    WMAP_SET_PARTICLE_CONFIG(0, g_wmap_particle_velocity_min, g_wmap_particle_velocity_range, g_wmap_particle_lifetime_min, g_wmap_particle_lifetime_range,
                             g_wmap_particle_spawn_interval, g_wmap_particle_radial_step, g_wmap_particle_first_motion, g_wmap_particle_texture_index,
                             g_wmap_particle_sequence, g_wmap_particle_radius);
    wmap_update_particle_system(actors, animations, count, peak_shade, initial_shade, shade_step, mode, g_wmap_effect_params);
}

/**
 * @brief Find the map cell that holds a land.
 * @param value Land id to find.
 * @param row_out Receives the cell x (first index of g_wmap_cells).
 * @param column_out Receives the cell y.
 * @return One if found, otherwise zero; outputs are unchanged on failure.
 */
s32 wmap_find_land_cell(s32 value, s32* row_out, s32* column_out)
{
    s32 row;
    s32 column;

    for (column = 0; column < 6; column++)
    {
        for (row = 0; row < 6; row++)
        {
            if (g_wmap_cells[row][column].land_id == value)
            {
                *row_out = row;
                *column_out = column;
                return 1;
            }
        }
    }

    return 0;
}

/**
 * @brief Install @p rotation as the GTE rotation, with zero translation.
 * @param rotation Rotation to install.
 */
void wmap_set_map_rotation(SVECTOR* rotation)
{
    MATRIX matrix;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, &g_wmap_zero_translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
}

/**
 * @brief Put the land focus back at its default screen position.
 */
void wmap_reset_focus_screen_position(void)
{
    g_wmap_focus_screen_position.point.x = WMAP_FOCUS_DEFAULT_X;
    g_wmap_focus_screen_position.point.y = WMAP_FOCUS_DEFAULT_Y;
}

/**
 * @brief Compose the camera rotation with a model rotation and install the result.
 */
void wmap_set_camera_model_transform(VECTOR* translation, SVECTOR* rotation)
{
    MATRIX matrices[2];

    RotMatrix(&g_wmap_camera_rotation, &matrices[0]);
    TransMatrix(&matrices[0], translation);
    SetRotMatrix(&matrices[0]);
    SetTransMatrix(&matrices[0]);
    RotMatrix(rotation, &matrices[1]);
    TransMatrix(&matrices[1], &g_wmap_zero_translation);
    CompMatrix(&matrices[0], &matrices[1], &matrices[1]);
    SetRotMatrix(&matrices[1]);
    SetTransMatrix(&matrices[1]);
}

/**
 * @brief Run the current step of the land entry sequence (land focus, then wait for the land effect).
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
WMAP_STEP_RUNNER(wmap_run_land_entry, g_wmap_land_entry_steps, WMAP_LAND_ENTRY_STEPS, g_wmap_land_entry_step, g_wmap_land_entry_timer)

/**
 * @brief Land entry step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_land_entry_reset, g_wmap_land_entry_step, g_wmap_land_entry_timer)

/**
 * @brief Land entry step 2: wait until the land effect clears g_wmap_sequence_busy.
 */
WMAP_STEP_WAIT_IDLE(wmap_land_entry_wait, g_wmap_land_entry_step, wmap_land_entry_finish)

/**
 * @brief Land entry step 3: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_land_entry_finish, g_wmap_land_entry_step)

/**
 * @brief Run the current step of the land return sequence (scroll back to the vehicle cell).
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_run_land_return, g_wmap_land_return_steps, WMAP_LAND_RETURN_STEPS, g_wmap_land_return_step, g_wmap_land_return_timer)

    /**
     * @brief Land return step 0: restart the sequence.
     */
    WMAP_STEP_RESET(wmap_land_return_reset, g_wmap_land_return_step, g_wmap_land_return_timer)

    /**
     * @brief Land return step 1: start the return.
     */
    void wmap_land_return_start(void)
{
    g_wmap_land_return_step++;
    wmap_land_return_wait_sequences();
}

/**
 * @brief Land return step 2: wait until no counted sequence is running.
 */
void wmap_land_return_wait_sequences(void)
{
    if (g_wmap_sequence_count == 0)
    {
        g_wmap_land_return_step++;
        wmap_land_return_scroll_back();
    }
}

/**
 * @brief Land return step 3: scroll the map back to the vehicle cell.
 */
void wmap_land_return_scroll_back(void)
{
    g_wmap_view_scroll_mode = WMAP_SCROLL_SCRIPTED;
    g_wmap_scroll_remaining_x = g_wmap_view.x - ((g_wmap_vehicle_cell_x - 1) * WMAP_CELL_SPACING);
    g_wmap_scroll_remaining_y = g_wmap_view.y - ((g_wmap_vehicle_cell_y - 1) * WMAP_CELL_SPACING);
    g_wmap_land_return_step++;
    wmap_land_return_wait_scroll();
}

/**
 * @brief Land return step 4: wait for the map scroll.
 */
WMAP_STEP_WAIT_SCROLL(wmap_land_return_wait_scroll, g_wmap_land_return_step, wmap_land_return_finish)

/**
 * @brief Land return step 5: clear the transition flag and restore the map state.
 */
void wmap_land_return_finish(void)
{
    g_wmap_event_active = 0;
    wmap_reset_after_transition();
    g_wmap_land_return_step++;
}
