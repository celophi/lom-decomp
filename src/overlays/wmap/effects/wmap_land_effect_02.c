#include "common.h"
#include <inline_c.h>
#include <libgte.h>

#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_land_effect_02.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    union
    {
        s32 packed;
        struct
        {
            s16 x;
            s16 y;
        } half;
    } screen;
} WmapEffect02Motion;

extern s32 g_wmap_land_effect_02_sequence_3_timer;
extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_02_sequence_4_timer;
extern s32 g_wmap_land_effect_02_sequence_5_timer;
extern s32 g_wmap_land_effect_02_sequence_6_timer;
extern s32 g_wmap_land_effect_02_timer;
extern WmapSequenceStep g_wmap_land_effect_02_steps[];
extern s32 g_wmap_land_effect_02_timeline_timer;
extern WmapSequenceStep g_wmap_land_effect_02_timeline_steps[];
extern s32 g_wmap_land_effect_02_sequence_1_timer;
extern WmapSequenceStep g_wmap_land_effect_02_sequence_1_steps[];
extern s32 g_wmap_land_effect_02_sequence_2_timer;
extern WmapSequenceStep g_wmap_land_effect_02_sequence_2_steps[];
extern WmapSequenceStep g_wmap_land_effect_02_sequence_3_steps[];
extern WmapSequenceStep g_wmap_land_effect_02_sequence_4_steps[];
extern WmapSequenceStep g_wmap_land_effect_02_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_effect_02_sequence_6_steps[];
extern u32 g_wmap_land_effect_02_sequence_3_step;
extern u32 g_wmap_land_effect_02_sequence_4_step;
extern u32 g_wmap_land_effect_02_sequence_5_step;
extern u32 g_wmap_land_effect_02_sequence_6_step;
extern u32 g_wmap_land_effect_02_step;
extern u32 g_wmap_land_effect_02_timeline_step;
extern u32 g_wmap_land_effect_02_sequence_1_step;
extern u32 g_wmap_land_effect_02_sequence_2_step;

extern WmapEffect02Motion g_wmap_actor_motions[];
extern WmapEffect02Motion g_wmap_actor_motions_at_10[];

/**
 * @brief Initialize world-map globals and four per-entry state records.
 */
void wmap_land_effect_02_sequence_3_step_01(void)
{
    s32 i;
    s16 shift;
    WmapSpriteActor* entry;

    g_wmap_particle_intensity = 4;
    g_wmap_effect_fade_b = 0x7F;
    g_wmap_particle_lifetime_min = 0x50;
    g_wmap_particle_lifetime_range = 2;
    g_wmap_particle_spawn_interval = -1;
    g_wmap_particle_radial_step = 0x7D0;
    g_wmap_particle_first_motion = 0xA;
    g_wmap_particle_texture_index = 8;
    g_wmap_particle_sequence = 1;
    g_wmap_particle_radius = 0;
    g_wmap_particle_arc_apex_lifetime = 0x28;
    g_wmap_particle_arc_peak = 0x50;
    g_wmap_particle_arc_divisor = 0x14;
    g_wmap_particle_arc_base = 1;

    for (i = 0; i < 4; i++)
    {
        entry = &g_wmap_sprite_actors[14 + i];
        g_wmap_actor_animations[i + 14].data = g_wmap_animation_bank_0;
        shift = i << 10;
        entry->scale_index = 0xF;
        entry->previous_sequence = -1;
        entry->shade_step = 4;
        entry->resource_index = 0;
        entry->target_shade = 0x81;
        entry->shade = 0x81;
        entry->sequence = g_wmap_particle_sequence;
        g_wmap_actor_motions_at_10[i].state = 1;
        g_wmap_actor_motions_at_10[i].z = g_wmap_particle_radius;
        g_wmap_actor_motions_at_10[i].scale = 0x50;
        g_wmap_actor_motions_at_10[i].angle = shift;
        g_wmap_actor_motions_at_10[i].x = 0;
        g_wmap_actor_motions_at_10[i].field_0E = 0;
    }

    g_wmap_land_effect_02_sequence_3_timer = 0x3C;
    g_wmap_land_effect_02_sequence_3_step++;
    wmap_land_effect_02_sequence_3_step_02();
}

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_02_sequence_4_step_02, g_wmap_land_effect_02_sequence_4_step, g_wmap_land_effect_02_sequence_4_timer,
                      g_wmap_effect_model_d_rotation, g_wmap_effect_model_d_position, g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 2)

/** @brief Initialize four equally spaced actors and start the effect. */
void wmap_land_effect_02_sequence_5_step_01(void)
{
    s32 i;
    WmapSpriteActor* actor;

    g_wmap_effect_model_e_rotation = g_wmap_zero_rotation;
    g_wmap_particle_velocity_min = 1;
    g_wmap_particle_velocity_range = 2;
    g_wmap_particle_lifetime_min = 0;

    for (i = 20; i < 80; i++)
    {
        g_wmap_actor_motions[i].state = 0;
    }

    for (i = 20; i < 80; i += 15)
    {
        actor = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_0;
        actor->scale_index = 15;
        actor->previous_sequence = -1;
        actor->shade_step = 2;
        actor->resource_index = 0;
        actor->sequence = 0;
        actor->target_shade = 1;
        actor->shade = 129;
        g_wmap_actor_motions[i].state = 1;
        g_wmap_actor_motions[i].z = 150000;
        g_wmap_actor_motions[i].field_0E = 0;
        g_wmap_actor_motions[i].angle = g_wmap_particle_lifetime_min;
        g_wmap_particle_lifetime_min += 1024;
    }

    g_wmap_land_effect_02_sequence_5_timer = 32;
    g_wmap_land_effect_02_sequence_5_step++;
    wmap_land_effect_02_sequence_5_step_02();
}

/**
 * @brief Advance the orbiting effect records, duplicate active state, render active slots, and tick the sequence timer.
 */
void wmap_land_effect_02_sequence_5_step_02(void)
{
    SVECTOR position;
    u32 screen_position;
    s32 i;
    s32 value;

    g_wmap_particle_velocity_range--;

    for (i = 20; i < 80; i += 15)
    {
        position.vx = ((g_wmap_actor_motions[i].z >> 3) * (ccos(g_wmap_actor_motions[i].angle) >> 6)) >> 12;
        position.vy = ((g_wmap_actor_motions[i].z >> 3) * (csin(g_wmap_actor_motions[i].angle) >> 6)) >> 12;
        position.vz = g_wmap_actor_motions[i].field_0E;
        gte_ldv0(&position);
        gte_rtps();
        g_wmap_actor_motions[i].z -= 3000;
        g_wmap_actor_motions[i].angle += 96;
        gte_stsxy(&screen_position);
        g_wmap_actor_motions[i].screen.half.x = ((u16*)&screen_position)[0];
        g_wmap_actor_motions[i].screen.half.y = ((u16*)&screen_position)[1];

        if (g_wmap_particle_velocity_range == 0)
        {
            g_wmap_sprite_actors[i + g_wmap_particle_velocity_min] = g_wmap_sprite_actors[i];
            g_wmap_actor_motions[i + g_wmap_particle_velocity_min] = g_wmap_actor_motions[i];
            g_wmap_actor_animations[i + g_wmap_particle_velocity_min] = g_wmap_actor_animations[i];
            g_wmap_sprite_actors[i + g_wmap_particle_velocity_min].shade_step = 8;
            g_wmap_sprite_actors[i + g_wmap_particle_velocity_min].target_shade = 1;
        }
    }

    if (g_wmap_particle_velocity_range == 0)
    {
        g_wmap_particle_velocity_range = 2;
        g_wmap_particle_velocity_min++;

        if (g_wmap_particle_velocity_min >= 15)
        {
            g_wmap_particle_velocity_min = 1;
            i = 20;
        }
    }

    i = 20;
    {
        WmapEffect02Motion* motion = &g_wmap_actor_motions[i];
        WmapAnimationSlot* resource = &g_wmap_actor_animations[i];
        WmapSpriteActor* actor = &g_wmap_sprite_actors[i];

        for (; i < 80; i++)
        {
            if (motion->state != 0)
            {
                wmap_step_actor_animation(actor, resource);
                wmap_draw_actor_sprite(actor, motion->screen.packed, 8, 8, 0);

                if (actor->shade < 5)
                {
                    motion->state = 0;
                }
            }

            motion++;
            resource++;
            actor++;
        }
    }

    value = g_wmap_land_effect_02_sequence_5_timer - 1;
    g_wmap_land_effect_02_sequence_5_timer = value;

    if (value == 0)
    {
        g_wmap_land_effect_02_sequence_5_step++;
    }
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_02_sequence_6_step_01(void)
{
    s32 i;

    g_wmap_aux_effect_fade_c = 1;
    g_wmap_falling_spawn_counts[0] = 1;

    for (i = 100; i < 200; i++)
    {
        g_wmap_actor_motions[i].state = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, g_wmap_animation_bank_0, 2);
    }

    g_wmap_falling_spawn_timers[0] = 2;
    g_wmap_land_effect_02_sequence_6_timer = 0x10;
    g_wmap_land_effect_02_sequence_6_step++;
    wmap_land_effect_02_sequence_6_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_02_run, g_wmap_land_effect_02_steps, 0x6, g_wmap_land_effect_02_step, g_wmap_land_effect_02_timer)

WMAP_STEP_RESET(wmap_land_effect_02_reset, g_wmap_land_effect_02_step, g_wmap_land_effect_02_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_02_step_01, g_wmap_land_effect_02_step, wmap_run_land_focus, wmap_land_effect_02_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_02_wait_idle_02, g_wmap_land_effect_02_step, wmap_land_effect_02_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_02_step_03, g_wmap_land_effect_02_step, wmap_land_effect_02_run_timeline, wmap_land_effect_02_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_02_wait_idle_04, g_wmap_land_effect_02_step, wmap_land_effect_02_end)

WMAP_STEP_ADVANCE(wmap_land_effect_02_end, g_wmap_land_effect_02_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_timeline, g_wmap_land_effect_02_timeline_steps, 0xC, g_wmap_land_effect_02_timeline_step,
                 g_wmap_land_effect_02_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_02_timeline_reset, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer)

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void wmap_land_effect_02_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x102045);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x1C, 0x80);
    wmap_start_sequence(wmap_land_effect_02_run_sequence_2);
    g_wmap_land_effect_02_timeline_timer = 0x18;
    g_wmap_land_effect_02_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_02_timeline_wait_02, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_02_timeline_step_03, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer,
                         wmap_land_effect_02_run_sequence_3, 0x50)

WMAP_STEP_WAIT(wmap_land_effect_02_timeline_wait_04, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_02_timeline_step_05, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer,
                         wmap_land_effect_02_run_sequence_5, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_02_timeline_wait_06, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer)

/** @brief Register a callback, set world-map state and color, and begin a two-tick delay. */
void wmap_land_effect_02_timeline_step_07(void)
{
    wmap_start_sequence(&wmap_land_effect_02_run_sequence_4);
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_backdrop_target_level = 8;
    wmap_start_map_tint(0x203050);
    g_wmap_land_effect_02_timeline_timer = 2;
    g_wmap_land_effect_02_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_02_timeline_wait_08, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_02_timeline_step_09, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer,
                             wmap_land_effect_02_run_sequence_6, wmap_land_effect_02_run_sequence_1, 0x7F)

WMAP_STEP_WAIT(wmap_land_effect_02_timeline_wait_10, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_02_timeline_finish, g_wmap_land_effect_02_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y,
                           g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_1, g_wmap_land_effect_02_sequence_1_steps, 0x4, g_wmap_land_effect_02_sequence_1_step,
                 g_wmap_land_effect_02_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_1_reset, g_wmap_land_effect_02_sequence_1_step, g_wmap_land_effect_02_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_02_sequence_1_step_01, g_wmap_land_effect_02_sequence_1_step, g_wmap_land_effect_02_sequence_1_timer, 4,
                      g_wmap_animation_bank_1, 0, 0x80, 0x80, 0, 0x82, wmap_land_effect_02_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_02_sequence_1_step_02, g_wmap_land_effect_02_sequence_1_step, g_wmap_land_effect_02_sequence_1_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xF, 0x1E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_1_end, g_wmap_land_effect_02_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_2, g_wmap_land_effect_02_sequence_2_steps, 0x4, g_wmap_land_effect_02_sequence_2_step,
                 g_wmap_land_effect_02_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_2_reset, g_wmap_land_effect_02_sequence_2_step, g_wmap_land_effect_02_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_02_sequence_2_step_01, g_wmap_land_effect_02_sequence_2_step, g_wmap_land_effect_02_sequence_2_timer, 5,
                      g_wmap_animation_bank_0, 0, 1, 0x81, 4, 0x8E, wmap_land_effect_02_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_02_sequence_2_step_02, g_wmap_land_effect_02_sequence_2_step, g_wmap_land_effect_02_sequence_2_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x8, 0x1, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_2_end, g_wmap_land_effect_02_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_3, g_wmap_land_effect_02_sequence_3_steps, 0x8, g_wmap_land_effect_02_sequence_3_step,
                 g_wmap_land_effect_02_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_3_reset, g_wmap_land_effect_02_sequence_3_step, g_wmap_land_effect_02_sequence_3_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_02_sequence_3_step_02, g_wmap_land_effect_02_sequence_3_step, g_wmap_land_effect_02_sequence_3_timer,
                          wmap_update_particles_from_settings(&g_wmap_sprite_actors[14], &g_wmap_actor_animations[14], 4, 1, g_wmap_effect_fade_b, 8, 2))

WMAP_STEP_ARM_TIMER(wmap_land_effect_02_sequence_3_step_03, g_wmap_land_effect_02_sequence_3_step, g_wmap_land_effect_02_sequence_3_timer, 0x14,
                    wmap_land_effect_02_sequence_3_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_02_sequence_3_step_04, g_wmap_land_effect_02_sequence_3_step, g_wmap_land_effect_02_sequence_3_timer,
                          wmap_update_particles_from_settings(&g_wmap_sprite_actors[14], &g_wmap_actor_animations[14], 4, 1, g_wmap_effect_fade_b, 8, 2))

/** @brief World-map step handler: clear a small entry table, set the timer, advance the step. */
void wmap_land_effect_02_sequence_3_step_05(void)
{
    s32 i;
    WmapSpriteActor* actors = &g_wmap_sprite_actors[14];

    for (i = 0; i < 4; i++)
    {
        actors[i].target_shade = 0;
        actors[i].shade_step = 8;
    }

    g_wmap_land_effect_02_sequence_3_timer = 0x10;
    g_wmap_land_effect_02_sequence_3_step++;
    wmap_land_effect_02_sequence_3_step_06();
}

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_02_sequence_3_step_06, g_wmap_land_effect_02_sequence_3_step, g_wmap_land_effect_02_sequence_3_timer,
                          wmap_update_particles_from_settings(&g_wmap_sprite_actors[14], &g_wmap_actor_animations[14], 4, 1, g_wmap_effect_fade_b, 8, 2))

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_3_end, g_wmap_land_effect_02_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_4, g_wmap_land_effect_02_sequence_4_steps, 0x4, g_wmap_land_effect_02_sequence_4_step,
                 g_wmap_land_effect_02_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_4_reset, g_wmap_land_effect_02_sequence_4_step, g_wmap_land_effect_02_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_02_sequence_4_step_01, g_wmap_land_effect_02_sequence_4_step, g_wmap_land_effect_02_sequence_4_timer,
                     g_wmap_effect_model_d_rotation, g_wmap_zero_rotation, g_wmap_effect_model_d_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40,
                     wmap_land_effect_02_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_4_end, g_wmap_land_effect_02_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_5, g_wmap_land_effect_02_sequence_5_steps, 0x4, g_wmap_land_effect_02_sequence_5_step,
                 g_wmap_land_effect_02_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_5_reset, g_wmap_land_effect_02_sequence_5_step, g_wmap_land_effect_02_sequence_5_timer)

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_5_end, g_wmap_land_effect_02_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_6, g_wmap_land_effect_02_sequence_6_steps, 0x8, g_wmap_land_effect_02_sequence_6_step,
                 g_wmap_land_effect_02_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_6_reset, g_wmap_land_effect_02_sequence_6_step, g_wmap_land_effect_02_sequence_6_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_02_sequence_6_step_02, g_wmap_land_effect_02_sequence_6_step, g_wmap_land_effect_02_sequence_6_timer,
                          g_wmap_aux_effect_fade_c, 8,
                          wmap_update_falling_particles(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA, 0x1F4, 0x32, 0x81, 0x81, 8, 0))

WMAP_STEP_ARM_TIMER(wmap_land_effect_02_sequence_6_step_03, g_wmap_land_effect_02_sequence_6_step, g_wmap_land_effect_02_sequence_6_timer, 0x40,
                    wmap_land_effect_02_sequence_6_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_02_sequence_6_step_04, g_wmap_land_effect_02_sequence_6_step, g_wmap_land_effect_02_sequence_6_timer,
                          wmap_update_falling_particles(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA, 0x1F4, 0x32, 0x81, 0x81, 8, 0))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_02_sequence_6_step_05, g_wmap_land_effect_02_sequence_6_step, g_wmap_land_effect_02_sequence_6_timer,
                               g_wmap_falling_spawn_counts[0], 0x40, wmap_land_effect_02_sequence_6_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_02_sequence_6_step_06, g_wmap_land_effect_02_sequence_6_step, g_wmap_land_effect_02_sequence_6_timer,
                          wmap_update_falling_particles(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA, 0x1F4, 0x32, 0x81, 0x81, 8, 0))

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_6_end, g_wmap_land_effect_02_sequence_6_step)
