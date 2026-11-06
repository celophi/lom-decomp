#include "common.h"
#include <libgte.h>

#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_effect_resources.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_map_labels.h"
#include "../internal/wmap_model_render.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_special_effect_34.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"
#include "main/cdrom.h"

extern WmapSequenceStep g_wmap_special_effect_34_steps[];
extern WmapSequenceStep g_wmap_special_effect_34_timeline_steps[];
extern s32 g_wmap_load_buffer[];
extern s32 g_wmap_special_effect_34_saved_cursor_column;
extern s32 g_wmap_special_effect_34_saved_cursor_row;
extern u32 g_wmap_special_effect_34_step;
extern s32 g_wmap_special_effect_34_timer;
extern u32 g_wmap_special_effect_34_timeline_step;
extern s32 g_wmap_special_effect_34_timeline_timer;
extern u32 g_wmap_special_effect_34_sequence_1_step;
extern s32 g_wmap_special_effect_34_sequence_1_timer;
extern u32 g_wmap_special_effect_34_sequence_2_step;
extern u32 g_wmap_special_effect_34_sequence_3_step;
extern s32 g_wmap_special_effect_34_sequence_3_timer;
extern WmapSequenceStep g_wmap_special_effect_34_sequence_1_steps[];
extern WmapSequenceStep g_wmap_special_effect_34_sequence_2_steps[];
extern WmapSequenceStep g_wmap_special_effect_34_sequence_3_steps[];
extern WmapSequenceStep g_wmap_special_effect_34_sequence_4_steps[];
extern s32 g_wmap_special_effect_34_sequence_2_timer;
extern u32 g_wmap_special_effect_34_sequence_4_step;
extern s32 g_wmap_special_effect_34_sequence_4_timer;
/** @brief Land ids whose cells sequence 2 scrolls to, indexed by g_wmap_special_land_event. */
extern const s32 g_wmap_special_effect_34_lands[5];

/** @brief Lower model a and fade it in, then draw it until the timer ends. */
void wmap_special_effect_34_sequence_1_step_02(void)
{
    MATRIX matrix;
    s32 value;

    value = g_wmap_effect_model_a_position.vz + 0x7D0;
    g_wmap_effect_model_a_position.vz = value;

    if (value > 0xBB80)
    {
        g_wmap_effect_model_a_position.vz = 0xBB80;
    }

    PushMatrix();
    RotMatrix(&g_wmap_effect_model_a_rotation, &matrix);
    TransMatrix(&matrix, &g_wmap_effect_model_a_position);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    wmap_draw_model(g_wmap_load_buffer, 0, 4, 0xB6, 0x7880, 1, g_wmap_effect_fade_a, 0, 0, -1);
    value = g_wmap_effect_fade_a + 2;
    g_wmap_effect_fade_a = value;

    if (value >= 0x41)
    {
        g_wmap_effect_fade_a = 0x10080;
    }

    PopMatrix();

    if (--g_wmap_special_effect_34_sequence_1_timer == 0)
    {
        g_wmap_special_effect_34_sequence_1_step++;
    }
}

/** @brief Spin model a faster while it starts to rise, until the timer ends. */
void wmap_special_effect_34_sequence_1_step_04(void)
{
    MATRIX matrix;
    SVECTOR* rotation = &g_wmap_effect_model_a_rotation;
    VECTOR* translation;

    rotation->vy += g_wmap_particle_lifetime_min;
    g_wmap_particle_lifetime_min += 4;

    if (g_wmap_particle_lifetime_min >= 0x101)
    {
        g_wmap_particle_lifetime_min = 0x100;
    }

    g_wmap_particle_velocity_range -= 2;
    PushMatrix();
    translation = &g_wmap_effect_model_a_position;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    wmap_draw_model(g_wmap_load_buffer, 0, 4, 0xB6, 0x7880, 1, 0x10080, 0, g_wmap_particle_velocity_range >> 4, -1);
    PopMatrix();

    if (--g_wmap_special_effect_34_sequence_1_timer == 0)
    {
        g_wmap_special_effect_34_sequence_1_step++;
    }
}

/** @brief Spin model a while it rises quickly off the screen, until the timer ends. */
void wmap_special_effect_34_sequence_1_step_06(void)
{
    MATRIX matrix;
    SVECTOR* rotation = &g_wmap_effect_model_a_rotation;
    VECTOR* translation;

    rotation->vy += g_wmap_particle_lifetime_min;
    g_wmap_particle_lifetime_min++;

    if (g_wmap_particle_lifetime_min >= 0x81)
    {
        g_wmap_particle_lifetime_min = 0x80;
    }

    g_wmap_particle_velocity_range -= 0x12C;
    PushMatrix();
    translation = &g_wmap_effect_model_a_position;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    wmap_draw_model(g_wmap_load_buffer, 0, 4, 0xB6, 0x7880, 1, 0x10080, 0, g_wmap_particle_velocity_range >> 4, -1);
    PopMatrix();

    if (--g_wmap_special_effect_34_sequence_1_timer == 0)
    {
        g_wmap_special_effect_34_sequence_1_step++;
    }
}

/**
 * @brief Start scrolling to the cell of an entry of g_wmap_special_effect_34_lands, if the land is on the map.
 * @param event_offset Entries to count back from the current special land event, wrapping at five.
 */
static inline void wmap_special_effect_34_scroll_to_land(s32 event_offset)
{
    g_wmap_view_scroll_mode = WMAP_SCROLL_NONE;

    if (wmap_find_land_cell(g_wmap_special_effect_34_lands[(g_wmap_special_land_event - event_offset) % 5], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y) !=
        0)
    {
        WMAP_START_SCROLL_TO_CELL(g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y);
    }
}

/** @brief Scroll to the land three entries back, then run step 2. */
void wmap_special_effect_34_sequence_2_step_01(void)
{
    wmap_special_effect_34_scroll_to_land(3);
    g_wmap_special_effect_34_sequence_2_step++;
    wmap_special_effect_34_sequence_2_step_02();
}

/** @brief Scroll to the land two entries back, then run step 4. */
void wmap_special_effect_34_sequence_2_step_03(void)
{
    wmap_special_effect_34_scroll_to_land(2);
    g_wmap_special_effect_34_sequence_2_step++;
    wmap_special_effect_34_sequence_2_step_04();
}

/** @brief Scroll to the land one entry back, then run step 6. */
void wmap_special_effect_34_sequence_2_step_05(void)
{
    wmap_special_effect_34_scroll_to_land(1);
    g_wmap_special_effect_34_sequence_2_step++;
    wmap_special_effect_34_sequence_2_step_06();
}

/** @brief Scroll to the current special land event's land, then run step 8. */
void wmap_special_effect_34_sequence_2_step_07(void)
{
    wmap_special_effect_34_scroll_to_land(0);
    g_wmap_special_effect_34_sequence_2_step++;
    wmap_special_effect_34_sequence_2_step_08();
}

/** @brief Move sprite actor 5 down and left across the screen, animating and drawing it until the timer ends. */
void wmap_special_effect_34_sequence_3_step_02(void)
{
    s32 x;
    s32 y;
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    if (g_wmap_effect_sprite_position_a.point.y < 0x78)
    {
        g_wmap_effect_sprite_position_b.point.x -= 0x80;
        g_wmap_effect_sprite_position_b.point.y += 0x168;
    }

    x = g_wmap_effect_sprite_position_b.point.x;

    if (x < 0)
    {
        x += 0xF;
    }

    y = g_wmap_effect_sprite_position_b.point.y;
    g_wmap_effect_sprite_position_a.point.x = (x >> 4) + 0xE6;

    if (y < 0)
    {
        y += 0xF;
    }

    g_wmap_effect_sprite_position_a.point.y = y >> 4;
    wmap_step_actor_animation(actor, &g_wmap_actor_animations[5]);
    wmap_draw_actor_sprite(actor, g_wmap_effect_sprite_position_a.packed, 8, 8, 0);

    if (--g_wmap_special_effect_34_sequence_3_timer == 0)
    {
        g_wmap_special_effect_34_sequence_3_step++;
    }
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run, g_wmap_special_effect_34_steps, 8, g_wmap_special_effect_34_step, g_wmap_special_effect_34_timer)

WMAP_STEP_RESET(wmap_special_effect_34_step_00, g_wmap_special_effect_34_step, g_wmap_special_effect_34_timer)

/**
 * @brief Effect 34 step: save the view and cursor, move the focus, load
 *        effect resource 0x22 and advance the step.
 * @note JP keeps the cursor and calls wmap_set_map_label(-1) instead.
 */
void wmap_special_effect_34_step_01(void)
{
#if !defined(VERSION_JP)
    s32 saved_column;
    s32 saved_row;
#endif

    g_wmap_focus_screen_position.point.x = WMAP_FOCUS_DEFAULT_X;
    g_wmap_focus_screen_position.point.y = WMAP_FOCUS_DEFAULT_Y;
    g_wmap_saved_view = g_wmap_view;
#if defined(VERSION_JP)
    wmap_set_map_label(-1);
#else
    saved_column = g_wmap_cursor_column;
    saved_row = g_wmap_cursor_row;
    g_wmap_cursor_row = 1;
    g_wmap_cursor_column = 1;
    g_wmap_special_effect_34_saved_cursor_column = saved_column;
    g_wmap_special_effect_34_saved_cursor_row = saved_row;
#endif
    wmap_queue_effect_resources(0x22);
    g_wmap_special_effect_34_timer = 0x1E;
    g_wmap_special_effect_34_step++;
}

WMAP_STEP_WAIT(wmap_special_effect_34_step_02, g_wmap_special_effect_34_step, g_wmap_special_effect_34_timer)

/** @brief Wait for the CD queue, upload both effect textures and start the timeline. */
void wmap_special_effect_34_step_03(void)
{
    cdrom_wait_queue_empty();
    wmap_upload_tim(g_wmap_effect_texture_buffer_0);
    wmap_upload_tim(g_wmap_effect_texture_buffer_1);
    wmap_start_sequence(wmap_special_effect_34_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_special_effect_34_step++;
    wmap_special_effect_34_step_04();
}

WMAP_STEP_WAIT_IDLE(wmap_special_effect_34_step_04, g_wmap_special_effect_34_step, wmap_special_effect_34_step_05)

/** @brief Start scrolling back to the view saved in step 1. */
void wmap_special_effect_34_step_05(void)
{
    g_wmap_view_scroll_mode = WMAP_SCROLL_SCRIPTED;
    g_wmap_scroll_remaining_x = g_wmap_saved_view.x - g_wmap_view.x;
    g_wmap_scroll_remaining_y = g_wmap_saved_view.y - g_wmap_view.y;
    g_wmap_special_effect_34_step++;
    wmap_special_effect_34_step_06();
}

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(wmap_special_effect_34_step_06, g_wmap_special_effect_34_step, wmap_special_effect_34_step_07)

/**
 * @brief Effect 34 step: reset after the transition, restore the saved
 *        cursor and advance the step.
 * @note JP neither sets g_wmap_exit_frame nor restores the cursor.
 */
void wmap_special_effect_34_step_07(void)
{
    g_wmap_event_active = 0;
    wmap_reset_after_transition();
#if !defined(VERSION_JP)
    g_wmap_exit_frame = 1;
    g_wmap_cursor_column = g_wmap_special_effect_34_saved_cursor_column;
    g_wmap_cursor_row = g_wmap_special_effect_34_saved_cursor_row;
#endif
    g_wmap_special_effect_34_step++;
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run_timeline, g_wmap_special_effect_34_timeline_steps, 0xE, g_wmap_special_effect_34_timeline_step,
                 g_wmap_special_effect_34_timeline_timer)

WMAP_STEP_RESET(wmap_special_effect_34_timeline_step_00, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer)

/** @brief Take over the map, play the effect sound, tint the map and start sequences 1 and 2. */
void wmap_special_effect_34_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_play_sound(0x3B, 0x80);
    wmap_start_map_tint(0x703040);
    g_wmap_backdrop_target_level = 0xA;
    g_wmap_particle_velocity_min = 1;
    g_wmap_particle_lifetime_range = 1;
    wmap_start_sequence(wmap_special_effect_34_run_sequence_1);
    wmap_start_sequence(wmap_special_effect_34_run_sequence_2);
    g_wmap_special_effect_34_timeline_step++;
    wmap_special_effect_34_timeline_step_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_special_effect_34_timeline_step_02(void)
{
    if (g_wmap_particle_velocity_min == 0)
    {
        g_wmap_special_effect_34_timeline_step++;
        wmap_special_effect_34_timeline_step_03();
    }
}

/**
 * @brief Prepare sequence coordinates for the selected world-map entry and advance the step.
 */
void wmap_special_effect_34_timeline_step_03(void)
{
    wmap_find_land_cell(g_wmap_special_effect_34_lands[g_wmap_special_land_event - 4], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    WMAP_START_SCROLL_TO_CELL(g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y);
    g_wmap_special_effect_34_timeline_step++;
    wmap_special_effect_34_timeline_step_04();
}

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(wmap_special_effect_34_timeline_step_04, g_wmap_special_effect_34_timeline_step, wmap_special_effect_34_timeline_step_05)

/** @brief World-map step handler: bump the step counter and run the next step. */
void wmap_special_effect_34_timeline_step_05(void)
{
    g_wmap_special_effect_34_timeline_step++;
    wmap_special_effect_34_timeline_step_06();
}

/** @brief Wait for sequence 1 to clear its running flag. */
void wmap_special_effect_34_timeline_step_06(void)
{
    if (g_wmap_particle_lifetime_range == 0)
    {
        g_wmap_special_effect_34_timeline_step++;
        wmap_special_effect_34_timeline_step_07();
    }
}

WMAP_STEP_START_AND_WAIT(wmap_special_effect_34_timeline_step_07, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer,
                         wmap_special_effect_34_run_sequence_3, 0xC)

WMAP_STEP_WAIT(wmap_special_effect_34_timeline_step_08, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_special_effect_34_timeline_step_09, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer,
                         wmap_special_effect_34_run_sequence_4, 0x5A)

WMAP_STEP_WAIT(wmap_special_effect_34_timeline_step_10, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer)

/** @brief Restore the map tint and backdrop level, then wait 60 frames. */
void wmap_special_effect_34_timeline_step_11(void)
{
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    g_wmap_special_effect_34_timeline_timer = 0x3C;
    g_wmap_special_effect_34_timeline_step++;
}

WMAP_STEP_WAIT(wmap_special_effect_34_timeline_step_12, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer)

/**
 * @brief Clear the blocking flag and finish the timeline.
 */
WMAP_STEP_FINISH_BLOCKING(wmap_special_effect_34_timeline_finish, g_wmap_special_effect_34_timeline_step)

WMAP_STEP_RUNNER(wmap_special_effect_34_run_sequence_1, g_wmap_special_effect_34_sequence_1_steps, 8, g_wmap_special_effect_34_sequence_1_step,
                 g_wmap_special_effect_34_sequence_1_timer)

WMAP_STEP_RESET(wmap_special_effect_34_sequence_1_reset, g_wmap_special_effect_34_sequence_1_step, g_wmap_special_effect_34_sequence_1_timer)

/** @brief Reset model a's rotation, position and fade, then run step 2. */
void wmap_special_effect_34_sequence_1_step_01(void)
{
    g_wmap_effect_model_a_rotation = g_wmap_zero_rotation;
    g_wmap_effect_model_a_position = g_wmap_zero_translation;
    g_wmap_effect_fade_a = 0;
    g_wmap_effect_model_a_position.vz = 0x1388;
    g_wmap_particle_lifetime_min = 0;
    g_wmap_particle_velocity_range = 0;
    g_wmap_special_effect_34_sequence_1_timer = 0x3C;
    g_wmap_special_effect_34_sequence_1_step++;
    wmap_special_effect_34_sequence_1_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_special_effect_34_sequence_1_step_03, g_wmap_special_effect_34_sequence_1_step, g_wmap_special_effect_34_sequence_1_timer, 0xB4,
                    wmap_special_effect_34_sequence_1_step_04)

WMAP_STEP_ARM_TIMER(wmap_special_effect_34_sequence_1_step_05, g_wmap_special_effect_34_sequence_1_step, g_wmap_special_effect_34_sequence_1_timer, 0x1E,
                    wmap_special_effect_34_sequence_1_step_06)

/** @brief Clear the running flag that timeline step 6 waits on. */
void wmap_special_effect_34_sequence_1_step_07(void)
{
    g_wmap_particle_lifetime_range = 0;
    g_wmap_special_effect_34_sequence_1_step++;
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run_sequence_2, g_wmap_special_effect_34_sequence_2_steps, 0xA, g_wmap_special_effect_34_sequence_2_step,
                 g_wmap_special_effect_34_sequence_2_timer)

WMAP_STEP_RESET(wmap_special_effect_34_sequence_2_reset, g_wmap_special_effect_34_sequence_2_step, g_wmap_special_effect_34_sequence_2_timer)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(wmap_special_effect_34_sequence_2_step_02, g_wmap_special_effect_34_sequence_2_step, wmap_special_effect_34_sequence_2_step_03)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(wmap_special_effect_34_sequence_2_step_04, g_wmap_special_effect_34_sequence_2_step, wmap_special_effect_34_sequence_2_step_05)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(wmap_special_effect_34_sequence_2_step_06, g_wmap_special_effect_34_sequence_2_step, wmap_special_effect_34_sequence_2_step_07)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(wmap_special_effect_34_sequence_2_step_08, g_wmap_special_effect_34_sequence_2_step, wmap_special_effect_34_sequence_2_step_09)

/** @brief Clear sequence 2's running flag. */
void wmap_special_effect_34_sequence_2_step_09(void)
{
    g_wmap_particle_velocity_min = 0;
    g_wmap_special_effect_34_sequence_2_step++;
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run_sequence_3, g_wmap_special_effect_34_sequence_3_steps, 6, g_wmap_special_effect_34_sequence_3_step,
                 g_wmap_special_effect_34_sequence_3_timer)

WMAP_STEP_RESET(wmap_special_effect_34_sequence_3_reset, g_wmap_special_effect_34_sequence_3_step, g_wmap_special_effect_34_sequence_3_timer)

/** @brief Set up sprite actor 5 with animation bank 1 at the start of its path, then run step 2. */
void wmap_special_effect_34_sequence_3_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_1;
    actor->scale_index = WMAP_SPRITE_SCALE_FULL;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->resource_index = 0;
    actor->shade_step = 0;
    actor->target_shade = WMAP_SPRITE_SHADE_NEUTRAL;
    actor->shade = WMAP_SPRITE_SHADE_NEUTRAL;
    g_wmap_effect_sprite_position_b.point.x = 0;
    g_wmap_effect_sprite_position_b.point.y = 0;
    g_wmap_special_effect_34_sequence_3_timer = 0x64;
    g_wmap_special_effect_34_sequence_3_step++;
    wmap_special_effect_34_sequence_3_step_02();
}

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_special_effect_34_sequence_3_step_03, g_wmap_special_effect_34_sequence_3_step, g_wmap_special_effect_34_sequence_3_timer,
                     g_wmap_sprite_actors[5], 4, 0x20, wmap_special_effect_34_sequence_3_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_special_effect_34_sequence_3_step_04, g_wmap_special_effect_34_sequence_3_step, g_wmap_special_effect_34_sequence_3_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_effect_sprite_position_a, 8, 8, 0)

WMAP_STEP_ADVANCE(wmap_special_effect_34_sequence_3_end, g_wmap_special_effect_34_sequence_3_step)

WMAP_STEP_RUNNER(wmap_special_effect_34_run_sequence_4, g_wmap_special_effect_34_sequence_4_steps, 6, g_wmap_special_effect_34_sequence_4_step,
                 g_wmap_special_effect_34_sequence_4_timer)

WMAP_STEP_RESET(wmap_special_effect_34_sequence_4_reset, g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_special_effect_34_sequence_4_step_01, g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer, 6,
                      g_wmap_animation_bank_1, 0, 0, 0x80, 0x10, 0x3E, wmap_special_effect_34_sequence_4_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_special_effect_34_sequence_4_step_02, g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 8, 8, 0)

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_special_effect_34_sequence_4_step_03, g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer,
                     g_wmap_sprite_actors[6], 2, 0x40, wmap_special_effect_34_sequence_4_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_special_effect_34_sequence_4_step_04, g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 8, 8, 0)

WMAP_STEP_ADVANCE(wmap_special_effect_34_sequence_4_end, g_wmap_special_effect_34_sequence_4_step)

/**
 * @brief Install a model transform built from a rotation and a translation.
 * @param translation Model translation.
 * @param rotation Model rotation.
 */
void wmap_special_effect_34_set_model_transform(VECTOR* translation, SVECTOR* rotation)
{
    MATRIX matrix;

    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
}
