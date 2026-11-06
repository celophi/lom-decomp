#include "common.h"
#include <inline_c.h>
#include <libgte.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_cells.h"
#include "../internal/wmap_land_effect_01.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"
#include "sdk/gte_dmpsx_compat.h"

extern s32 rand(void);
extern s32 g_wmap_land_effect_01_sequence_3_timer;
extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_01_sequence_4_timer;
extern s32 g_wmap_land_effect_01_sequence_5_timer;
extern s32 g_wmap_land_effect_01_sequence_8_timer;
extern s32 g_wmap_land_effect_01_timeline_timer;
extern WmapSequenceStep g_wmap_land_effect_01_timeline_steps[];
extern s32 g_wmap_land_effect_01_sequence_2_timer;
extern WmapSequenceStep g_wmap_land_effect_01_sequence_2_steps[];
extern s32 g_wmap_land_effect_01_timer;
extern WmapSequenceStep g_wmap_land_effect_01_steps[];
extern s32 g_wmap_land_effect_01_sequence_1_timer;
extern WmapSequenceStep g_wmap_land_effect_01_sequence_1_steps[];
extern WmapSequenceStep g_wmap_land_effect_01_sequence_3_steps[];
extern WmapSequenceStep g_wmap_land_effect_01_sequence_4_steps[];
extern WmapSequenceStep g_wmap_land_effect_01_sequence_5_steps[];
extern s32 g_wmap_land_effect_01_sequence_6_timer;
extern WmapSequenceStep g_wmap_land_effect_01_sequence_6_steps[];
extern s32 g_wmap_land_effect_01_sequence_7_timer;
extern WmapSequenceStep g_wmap_land_effect_01_sequence_7_steps[];
extern WmapSequenceStep g_wmap_land_effect_01_sequence_8_steps[];

extern u32 g_wmap_land_effect_01_sequence_3_step;
extern u32 g_wmap_land_effect_01_sequence_4_step;
extern WmapMotion* g_wmap_land_effect_01_ring_motions;
extern s32 g_wmap_particle_shade;
extern u32 g_wmap_land_effect_01_sequence_5_step;
extern u32 g_wmap_land_effect_01_sequence_8_step;
extern u32 g_wmap_land_effect_01_timeline_step;
extern u32 g_wmap_land_effect_01_sequence_2_step;
extern u32 g_wmap_land_effect_01_step;
extern u32 g_wmap_land_effect_01_sequence_1_step;
extern u32 g_wmap_land_effect_01_sequence_6_step;
extern u32 g_wmap_land_effect_01_sequence_7_step;

extern WmapMotion g_wmap_actor_motions_at_10[];

/**
 * @brief Project active particles and initialize the first available slot.
 * @param actors Actor configurations for the particle slots.
 * @param resources Animation resources corresponding to the actor slots.
 * @param count Number of slots to process.
 */
void wmap_land_effect_01_update_particles(WmapSpriteActor* actors, WmapAnimationSlot* resources, s32 count)
{
    SVECTOR position;
    s32 screen_position;
    s32 active_count;
    s32 i;

    active_count = 0;

    for (i = 0; i < count; i++)
    {
        if (g_wmap_actor_motions[i].state != 0)
        {
            position.vx = ((g_wmap_actor_motions[i].radius >> 3) * (ccos(g_wmap_actor_motions[i].angle) >> 6)) >> 12;
            position.vy = ((g_wmap_actor_motions[i].radius >> 3) * (csin(g_wmap_actor_motions[i].angle) >> 6)) >> 12;
            position.vz = g_wmap_actor_motions[i].z;
            gte_ldv0(&position);
            gte_rtps();
            g_wmap_actor_motions[i].z += g_wmap_actor_motions[i].velocity;
            g_wmap_actor_motions[i].radius += 1000;
            gte_stsxy(&screen_position);
            wmap_step_actor_animation(&actors[i], &resources[i]);
            wmap_draw_actor_sprite(&actors[i], screen_position, 13, 10, 0);
            g_wmap_actor_motions[i].lifetime--;

            if (g_wmap_actor_motions[i].lifetime == 0)
            {
                g_wmap_actor_motions[i].state = 0;
            }

            active_count++;
        }
    }

    g_wmap_unused_tally = active_count;

    for (i = 0; i < count; i++)
    {
        if (g_wmap_actor_motions[i].state == 0)
        {
            if (g_wmap_particle_intensity >= active_count)
            {
                actors[i].scale_index = WMAP_SPRITE_SCALE_FULL;
                actors[i].previous_sequence = -1;
                actors[i].resource_index = 0;
                actors[i].sequence = 1;
                actors[i].target_shade = 129;
                actors[i].shade = 129;
                g_wmap_actor_motions[i].state = 1;
                g_wmap_actor_motions[i].angle = rand();
                g_wmap_actor_motions[i].radius = 10000;
                g_wmap_actor_motions[i].velocity = ((rand() * 5) >> 15) + 1;
                g_wmap_actor_motions[i].lifetime = ((rand() * 2) >> 15) + 24;
                g_wmap_actor_motions[i].z = 0;
            }

            break;
        }
    }
}

/** @brief Project and draw the map effect, advancing when its timer expires. */
void wmap_land_effect_01_sequence_3_step_02(void)
{
    SVECTOR position;
    s32 screen_position;
    s32 remaining;
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    position.vz = 0;
    position.vx = (((g_wmap_focus_cell_x - 1) * WMAP_CELL_SIZE - g_wmap_view.x * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) /
                  g_wmap_view.projection_scale;
    position.vy = (((g_wmap_focus_cell_y - 1) * WMAP_CELL_SIZE - g_wmap_view.y * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) /
                  g_wmap_view.projection_scale;
    gte_ldv0(&position);
    gte_rtps();
    wmap_step_actor_animation(actor, &g_wmap_actor_animations[5]);
    gte_stsxy(&screen_position);
    wmap_draw_actor_sprite(actor, screen_position, 12, 10, 0);
    remaining = g_wmap_land_effect_01_sequence_3_timer - 1;
    g_wmap_land_effect_01_sequence_3_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_01_sequence_3_step++;
    }
}

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void wmap_land_effect_01_sequence_4_step_02(void)
{
    MATRIX matrix;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = g_wmap_effect_model_b_position.vz - 2400;
    g_wmap_effect_model_b_position.vz = depth;

    if (depth < 10)
    {
        g_wmap_effect_model_b_position.vz = 10;
    }

    PushMatrix();
    RotMatrix(&g_wmap_effect_model_b_rotation, &matrix);
    TransMatrix(&matrix, &g_wmap_zero_translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);

    if (g_wmap_effect_flash_fade_a != 0)
    {
        wmap_draw_model_default(g_wmap_load_buffer, 0, 4, -1, -1, 1, g_wmap_effect_flash_fade_a);
    }

    PopMatrix();
    WMAP_MODEL_FADE_OUT(g_wmap_effect_flash_fade_a, 9, intensity);
    remaining = g_wmap_land_effect_01_sequence_4_timer - 1;
    g_wmap_land_effect_01_sequence_4_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_01_sequence_4_step++;
    }
}

/** @brief Initialize twenty effect actors with alternating motion parameters. */
void wmap_land_effect_01_sequence_5_step_01(void)
{
    s32 i = 0;
    WmapSpriteActor* actor;

    g_wmap_land_effect_01_ring_motions = g_wmap_actor_motions_at_10;
    g_wmap_effect_model_a_rotation = g_wmap_zero_rotation;
    g_wmap_effect_model_c_position = g_wmap_camera_translation;
    g_wmap_effect_flash_fade_a = 128;

    for (; i < 20; i++)
    {
        actor = &g_wmap_sprite_actors[6 + i];
        g_wmap_land_effect_01_ring_motions[i].state = 0;
        g_wmap_actor_animations[i + 6].data = g_wmap_animation_bank_1 + ((i % 3) << 13);
        actor->resource_index = 0;
        actor->scale_index = WMAP_SPRITE_SCALE_FULL;
        actor->sequence = rand() & 1;
        actor->previous_sequence = -1;
        actor->target_shade = WMAP_SPRITE_SHADE_NEUTRAL;
        actor->shade = WMAP_SPRITE_SHADE_NEUTRAL;
        g_wmap_land_effect_01_ring_motions[i].state = 1;
        g_wmap_land_effect_01_ring_motions[i].velocity = 30000;
        g_wmap_land_effect_01_ring_motions[i].angle = i * 204;
        g_wmap_land_effect_01_ring_motions[i].radius = 0;

        if (i & 1)
        {
            g_wmap_land_effect_01_ring_motions[i].z = 30;
        }
        else
        {
            g_wmap_land_effect_01_ring_motions[i].z = 0;
        }
    }

    g_wmap_particle_shade = 128;
    g_wmap_land_effect_01_sequence_5_timer = 36;
    g_wmap_land_effect_01_sequence_5_step++;
    wmap_land_effect_01_sequence_5_step_02();
}

/** @brief Compose the effect transform and project its twenty actors. */
void wmap_land_effect_01_draw_ring(void)
{
    MATRIX base_matrix;
    MATRIX effect_matrix;
    SVECTOR position;
    s32 screen_position;
    s32 i;
    s32 value;
    WmapMotion* motion;
    WmapSpriteActor* actor;
    WmapAnimationSlot* resource;

    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base_matrix);
    TransMatrix(&base_matrix, &g_wmap_effect_model_c_position);
    SetRotMatrix(&base_matrix);
    SetTransMatrix(&base_matrix);
    RotMatrix(&g_wmap_effect_model_a_rotation, &effect_matrix);
    TransMatrix(&effect_matrix, &g_wmap_zero_translation);
    CompMatrix(&base_matrix, &effect_matrix, &effect_matrix);
    SetRotMatrix(&effect_matrix);
    SetTransMatrix(&effect_matrix);
    g_wmap_effect_model_a_rotation.vz -= 80;

    for (i = 0; i < 20; i++)
    {
        actor = &g_wmap_sprite_actors[6 + i];
        position.vx = ((g_wmap_land_effect_01_ring_motions[i].radius >> 6) * (ccos(g_wmap_land_effect_01_ring_motions[i].angle) >> 6)) >> 12;
        position.vy = ((g_wmap_land_effect_01_ring_motions[i].radius >> 6) * (csin(g_wmap_land_effect_01_ring_motions[i].angle) >> 6)) >> 12;
        motion = &g_wmap_land_effect_01_ring_motions[i];
        resource = &g_wmap_actor_animations[6 + i];
        position.vz = motion->z;
        gte_ldv0(&position);
        gte_rtps();
        value = motion->radius + motion->velocity;
        motion->radius = value;

        if (value > 900000)
        {
            motion->radius = 900000;
        }

        actor->target_shade = g_wmap_particle_shade;
        actor->shade = g_wmap_particle_shade;
        gte_stsxy(&screen_position);
        wmap_step_actor_animation(actor, resource);
        wmap_draw_actor_sprite(actor, screen_position, 13, 31, 0);
    }

    PopMatrix();
}

/** @brief Configure effect parameters and reset its animation resource slots. */
void wmap_land_effect_01_sequence_8_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 24;
    g_wmap_effect_fade_b = 256;
    WMAP_SET_PARTICLE_SETTINGS(12, 10, 24, 2, 2, 1500, 128, 13, 1, 0);
    WMAP_RESET_PARTICLE_SLOTS(i, 24, g_wmap_actor_motions[i + g_wmap_particle_first_motion].state, 204, g_wmap_animation_bank_4);
    g_wmap_land_effect_01_sequence_8_timer = 32;
    g_wmap_land_effect_01_sequence_8_step++;
    wmap_land_effect_01_sequence_8_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_01_run_timeline, g_wmap_land_effect_01_timeline_steps, 0xE, g_wmap_land_effect_01_timeline_step,
                 g_wmap_land_effect_01_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_01_timeline_reset, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

/** @brief World-map step handler: set up the actor, register callbacks, advance step. */
void wmap_land_effect_01_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x404040);
    wmap_start_sequence(wmap_land_effect_01_run_sequence_2);
    wmap_play_sound(0xF, 0x80);
    g_wmap_land_effect_01_timeline_timer = 8;
    g_wmap_land_effect_01_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_02, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

/** @brief Set the sequence flag, register two callbacks, and begin a 48-tick delay. */
void wmap_land_effect_01_timeline_step_03(void)
{
    g_wmap_placement_overlay_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_01_run_sequence_1);
    wmap_start_sequence(&wmap_land_effect_01_run_sequence_3);
    g_wmap_land_effect_01_timeline_timer = 0x30;
    g_wmap_land_effect_01_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_04, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_01_timeline_step_05, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer,
                         wmap_land_effect_01_run_sequence_4, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_06, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_01_timeline_step_07, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer,
                         wmap_land_effect_01_run_sequence_5, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_08, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

/**
 * @brief Start three sequences and set the wait timer.
 */
WMAP_STEP_START_THREE_AND_WAIT(wmap_land_effect_01_timeline_step_09, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer,
                               wmap_land_effect_01_run_sequence_7, wmap_land_effect_01_run_sequence_8, wmap_land_effect_01_run_sequence_6, 0x60)

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_10, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

/** @brief Play sound 19, start a 54-tick delay, and advance the state. */
void wmap_land_effect_01_timeline_step_11(void)
{
    wmap_play_sound(0x13, 0x80);
    g_wmap_land_effect_01_timeline_timer = 54;
    g_wmap_land_effect_01_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_12, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_01_timeline_finish, g_wmap_land_effect_01_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y,
                           g_wmap_selected_artifact)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_2, g_wmap_land_effect_01_sequence_2_steps, 0x6, g_wmap_land_effect_01_sequence_2_step,
                               g_wmap_land_effect_01_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_2_reset, g_wmap_land_effect_01_sequence_2_step, g_wmap_land_effect_01_sequence_2_timer)

/** @brief Initialize the actor configuration and begin an eight-tick sequence step. */
void wmap_land_effect_01_sequence_2_step_01(void)
{
    g_wmap_actor_animations[4].data = g_wmap_animation_bank_0;
    g_wmap_sprite_actors[4].scale_index = WMAP_SPRITE_SCALE_FULL;
    g_wmap_sprite_actors[4].sequence = 1;
    g_wmap_sprite_actors[4].previous_sequence = -1;
    g_wmap_sprite_actors[4].shade = 1;
    g_wmap_sprite_actors[4].resource_index = 0;
    g_wmap_sprite_actors[4].target_shade = 0xF1;
    g_wmap_land_effect_01_sequence_2_timer = 8;
    g_wmap_land_effect_01_sequence_2_step++;
    wmap_land_effect_01_sequence_2_step_02();
}

/**
 * @brief World-map step handler: draw the sprite, bump its animation field, then
 *        countdown-advance the step.
 */
void wmap_land_effect_01_sequence_2_step_02(void)
{
    WmapSpriteActor* obj;
    WmapSpriteActor* actors;

    obj = &g_wmap_sprite_actors[4];
    wmap_step_actor_animation(obj, &g_wmap_actor_animations[4]);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 0xC, 0xA, 0);
    /* This sprite occupies the fifth actor slot. */
    actors = obj - 4;
    actors[4].shade += 2;

    if (--g_wmap_land_effect_01_sequence_2_timer == 0)
    {
        g_wmap_land_effect_01_sequence_2_step++;
    }
}

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void wmap_land_effect_01_sequence_2_step_03(void)
{
    g_wmap_sprite_actors[4].target_shade = 1;
    g_wmap_land_effect_01_sequence_2_timer = 0x10;
    g_wmap_land_effect_01_sequence_2_step++;
    wmap_land_effect_01_sequence_2_step_04();
}

/**
 * @brief World-map step handler: draw the sprite, bump its animation field, then
 *        countdown-advance the step.
 */
void wmap_land_effect_01_sequence_2_step_04(void)
{
    WmapSpriteActor* obj;
    WmapSpriteActor* actors;

    obj = &g_wmap_sprite_actors[4];
    wmap_step_actor_animation(obj, &g_wmap_actor_animations[4]);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 0xC, 0xA, 0);
    /* This sprite occupies the fifth actor slot. */
    actors = obj - 4;
    actors[4].shade += 2;

    if (--g_wmap_land_effect_01_sequence_2_timer == 0)
    {
        g_wmap_land_effect_01_sequence_2_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_2_end, g_wmap_land_effect_01_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_01_run, g_wmap_land_effect_01_steps, 0x6, g_wmap_land_effect_01_step, g_wmap_land_effect_01_timer)

WMAP_STEP_RESET(wmap_land_effect_01_reset, g_wmap_land_effect_01_step, g_wmap_land_effect_01_timer)

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void wmap_land_effect_01_step_01(void)
{
    g_wmap_input_locked = 1;
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_01_step++;
    wmap_land_effect_01_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_01_wait_idle_02, g_wmap_land_effect_01_step, wmap_land_effect_01_step_03)

/** @brief World-map step: register the next draw callback and advance to the next handler. */
void wmap_land_effect_01_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_01_run_timeline);
    g_wmap_land_effect_01_step++;
    wmap_land_effect_01_wait_idle_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_01_wait_idle_04, g_wmap_land_effect_01_step, wmap_land_effect_01_end)

WMAP_STEP_ADVANCE(wmap_land_effect_01_end, g_wmap_land_effect_01_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_1, g_wmap_land_effect_01_sequence_1_steps, 0x4, g_wmap_land_effect_01_sequence_1_step,
                               g_wmap_land_effect_01_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_1_reset, g_wmap_land_effect_01_sequence_1_step, g_wmap_land_effect_01_sequence_1_timer)

/** @brief World-map step: reset a run of slot tables then advance the sub-counter. */
void wmap_land_effect_01_sequence_1_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 0x18;
    WMAP_RESET_PARTICLE_SLOTS(i, 0x18, g_wmap_actor_motions[i].state, 0xCC, g_wmap_animation_bank_4);
    g_wmap_land_effect_01_sequence_1_timer = 0x30;
    g_wmap_land_effect_01_sequence_1_step++;
    wmap_land_effect_01_sequence_1_step_02();
}

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_01_sequence_1_step_02, g_wmap_land_effect_01_sequence_1_step, g_wmap_land_effect_01_sequence_1_timer,
                          wmap_land_effect_01_update_particles(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x18))

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_1_end, g_wmap_land_effect_01_sequence_1_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_3, g_wmap_land_effect_01_sequence_3_steps, 0x4, g_wmap_land_effect_01_sequence_3_step,
                               g_wmap_land_effect_01_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_3_reset, g_wmap_land_effect_01_sequence_3_step, g_wmap_land_effect_01_sequence_3_timer)

/** @brief World-map step handler: init substate block and advance. */
void wmap_land_effect_01_sequence_3_step_01(void)
{
    WmapSpriteActor* actors = g_wmap_sprite_actors;

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_0;
    actors[5].scale_index = WMAP_SPRITE_SCALE_FULL;
    actors[5].previous_sequence = -1;
    actors[5].resource_index = 0;
    actors[5].sequence = 0;
    actors[5].target_shade = WMAP_SPRITE_SHADE_NEUTRAL;
    actors[5].shade = WMAP_SPRITE_SHADE_NEUTRAL;
    g_wmap_land_effect_01_sequence_3_timer = 0x32;
    g_wmap_land_effect_01_sequence_3_step++;
    wmap_land_effect_01_sequence_3_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_3_end, g_wmap_land_effect_01_sequence_3_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_4, g_wmap_land_effect_01_sequence_4_steps, 0x4, g_wmap_land_effect_01_sequence_4_step,
                               g_wmap_land_effect_01_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_4_reset, g_wmap_land_effect_01_sequence_4_step, g_wmap_land_effect_01_sequence_4_timer)

/** @brief Restore the default transform and advance the sequence. */
void wmap_land_effect_01_sequence_4_step_01(void)
{
    g_wmap_effect_model_b_rotation = g_wmap_zero_rotation;
    g_wmap_effect_model_b_position = g_wmap_camera_translation;
    g_wmap_effect_flash_fade_a = 0x80;
    g_wmap_land_effect_01_sequence_4_timer = 0x20;
    g_wmap_land_effect_01_sequence_4_step++;
    wmap_land_effect_01_sequence_4_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_4_end, g_wmap_land_effect_01_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_01_run_sequence_5, g_wmap_land_effect_01_sequence_5_steps, 0x6, g_wmap_land_effect_01_sequence_5_step,
                 g_wmap_land_effect_01_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_5_reset, g_wmap_land_effect_01_sequence_5_step, g_wmap_land_effect_01_sequence_5_timer)

/** @brief Update the sequence effect and advance when its countdown reaches zero. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_01_sequence_5_step_02, g_wmap_land_effect_01_sequence_5_step, g_wmap_land_effect_01_sequence_5_timer,
                          wmap_land_effect_01_draw_ring())

WMAP_STEP_ARM_TIMER(wmap_land_effect_01_sequence_5_step_03, g_wmap_land_effect_01_sequence_5_step, g_wmap_land_effect_01_sequence_5_timer, 0x20,
                    wmap_land_effect_01_sequence_5_step_04)

/** @brief Reduce the effect value toward zero, update it, and advance when the countdown expires. */
void wmap_land_effect_01_sequence_5_step_04(void)
{
    s32 value;
    s32 remaining_ticks;

    value = g_wmap_particle_shade - 4;
    g_wmap_particle_shade = value;

    if (value < 0)
    {
        g_wmap_particle_shade = 0;
    }

    wmap_land_effect_01_draw_ring();
    remaining_ticks = g_wmap_land_effect_01_sequence_5_timer - 1;
    g_wmap_land_effect_01_sequence_5_timer = remaining_ticks;

    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_01_sequence_5_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_5_end, g_wmap_land_effect_01_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_01_run_sequence_6, g_wmap_land_effect_01_sequence_6_steps, 0x4, g_wmap_land_effect_01_sequence_6_step,
                 g_wmap_land_effect_01_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_6_reset, g_wmap_land_effect_01_sequence_6_step, g_wmap_land_effect_01_sequence_6_timer)

/** @brief World-map step handler: init substate block and advance. */
void wmap_land_effect_01_sequence_6_step_01(void)
{
    WmapSpriteActor* actors = g_wmap_sprite_actors;

    g_wmap_actor_animations[254].data = g_wmap_animation_bank_5;
    actors[254].scale_index = WMAP_SPRITE_SCALE_FULL;
    actors[254].previous_sequence = -1;
    actors[254].resource_index = 0;
    actors[254].sequence = 0;
    actors[254].target_shade = WMAP_SPRITE_SHADE_NEUTRAL;
    actors[254].shade = 0;
    g_wmap_land_effect_01_sequence_6_timer = 0x98;
    g_wmap_land_effect_01_sequence_6_step++;
    wmap_land_effect_01_sequence_6_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_01_sequence_6_step_02, g_wmap_land_effect_01_sequence_6_step, g_wmap_land_effect_01_sequence_6_timer,
                              g_wmap_sprite_actors[254], g_wmap_actor_animations[254], g_wmap_focus_screen_position, 0xE, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_6_end, g_wmap_land_effect_01_sequence_6_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_7, g_wmap_land_effect_01_sequence_7_steps, 0x6, g_wmap_land_effect_01_sequence_7_step,
                               g_wmap_land_effect_01_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_7_reset, g_wmap_land_effect_01_sequence_7_step, g_wmap_land_effect_01_sequence_7_timer)

/** @brief World-map step handler: init substate block and advance. */
void wmap_land_effect_01_sequence_7_step_01(void)
{
    WmapSpriteActor* actors = g_wmap_sprite_actors;

    g_wmap_vehicle_animation.data = g_wmap_animation_bank_4;
    actors[WMAP_VEHICLE_ACTOR].scale_index = WMAP_SPRITE_SCALE_FULL;
    actors[WMAP_VEHICLE_ACTOR].previous_sequence = -1;
    actors[WMAP_VEHICLE_ACTOR].resource_index = 0;
    actors[WMAP_VEHICLE_ACTOR].sequence = 0;
    actors[WMAP_VEHICLE_ACTOR].target_shade = 0x81;
    actors[WMAP_VEHICLE_ACTOR].shade = 0x81;
    g_wmap_land_effect_01_sequence_7_timer = 0x40;
    g_wmap_land_effect_01_sequence_7_step++;
    wmap_land_effect_01_sequence_7_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_01_sequence_7_step_02, g_wmap_land_effect_01_sequence_7_step, g_wmap_land_effect_01_sequence_7_timer,
                              g_wmap_vehicle_actor, g_wmap_vehicle_animation, g_wmap_focus_screen_position, 0xD, 0xA, 0)

/**
 * @brief Reset a world-map animation flag and schedule the next step.
 */
void wmap_land_effect_01_sequence_7_step_03(void)
{
    g_wmap_vehicle_actor.target_shade = 0;
    g_wmap_land_effect_01_sequence_7_timer = 0x10;
    g_wmap_land_effect_01_sequence_7_step++;
    wmap_land_effect_01_sequence_7_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_01_sequence_7_step_04, g_wmap_land_effect_01_sequence_7_step, g_wmap_land_effect_01_sequence_7_timer,
                              g_wmap_vehicle_actor, g_wmap_vehicle_animation, g_wmap_focus_screen_position, 0xD, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_7_end, g_wmap_land_effect_01_sequence_7_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_8, g_wmap_land_effect_01_sequence_8_steps, 0x6, g_wmap_land_effect_01_sequence_8_step,
                               g_wmap_land_effect_01_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_8_reset, g_wmap_land_effect_01_sequence_8_step, g_wmap_land_effect_01_sequence_8_timer)

/** @brief Draw the sequence effect and advance when the countdown expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_01_sequence_8_step_02, g_wmap_land_effect_01_sequence_8_step, g_wmap_land_effect_01_sequence_8_timer,
                          wmap_update_default_particles_from_settings(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x18, 0x81, 0x81, 0x10))

WMAP_STEP_ARM_TIMER(wmap_land_effect_01_sequence_8_step_03, g_wmap_land_effect_01_sequence_8_step, g_wmap_land_effect_01_sequence_8_timer, 0x20,
                    wmap_land_effect_01_sequence_8_step_04)

/** @brief Draw the effect, reduce its value toward zero, and update the countdown. */
void wmap_land_effect_01_sequence_8_step_04(void)
{
    s32 value;
    s32 remaining_ticks;

    wmap_update_default_particles_from_settings(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x18, 0x81, 0x81, 0x10);
    value = g_wmap_effect_fade_b - 0xA;
    g_wmap_effect_fade_b = value;

    if (value < 0)
    {
        g_wmap_effect_fade_b = 0;
    }

    remaining_ticks = g_wmap_land_effect_01_sequence_8_timer - 1;
    g_wmap_land_effect_01_sequence_8_timer = remaining_ticks;

    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_01_sequence_8_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_8_end, g_wmap_land_effect_01_sequence_8_step)
