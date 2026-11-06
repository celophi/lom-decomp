#include "common.h"
#include <inline_c.h>
#include <libgte.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_cells.h"
#include "../internal/wmap_land_effect_13.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"
#include "common/vector.h"
#include "sdk/gte_dmpsx_compat.h"

static void wmap_land_effect_13_update_particles(WmapSpriteActor* actors, WmapAnimationSlot* resources, s32 count);

/**
 * @brief Animate and draw a sprite at the focus with its shade at a fade level, then raise the fade level.
 * @param slot Sprite actor and animation slot.
 * @param fade Fade level global; its low halfword is the sprite's shade.
 * @param texture_index Sprite texture.
 * @param amount Amount added to the fade level each frame.
 * @param maximum Highest fade level.
 */
#define WMAP_EFFECT13_DRAW_BRIGHTENING_SPRITE(slot, fade, texture_index, amount, maximum)                                                                      \
    {                                                                                                                                                          \
        WmapSpriteActor* actor = &g_wmap_sprite_actors[(slot)];                                                                                                \
        u16 shade = *(u16*)&(fade);                                                                                                                            \
                                                                                                                                                               \
        actor->shade = shade;                                                                                                                                  \
        actor->target_shade = shade;                                                                                                                           \
        wmap_step_actor_animation(actor, &g_wmap_actor_animations[(slot)]);                                                                                    \
        wmap_draw_actor_sprite(actor, g_wmap_focus_screen_position.packed, (texture_index), 0xB, 0);                                                           \
        (fade) += (amount);                                                                                                                                    \
                                                                                                                                                               \
        if ((fade) > (maximum))                                                                                                                                \
        {                                                                                                                                                      \
            (fade) = (maximum);                                                                                                                                \
        }                                                                                                                                                      \
    }

/**
 * @brief Define a step that starts sprite sequence 1 on a slot, clears a fade level and runs its first update.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param slot Sprite actor and animation slot.
 * @param animation Animation data assigned to the slot.
 * @param fade Fade level global the update step raises from zero.
 * @param frames Frames the update step runs.
 * @param next Update step, called immediately after advancing.
 */
#define WMAP_EFFECT13_START_SPRITE(name, step, timer, slot, animation, fade, frames, next)                                                                     \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        WmapSpriteActor* actor = &g_wmap_sprite_actors[(slot)];                                                                                                \
                                                                                                                                                               \
        g_wmap_actor_animations[(slot)].data = (animation);                                                                                                    \
        actor->scale_index = WMAP_SPRITE_SCALE_FULL;                                                                                                           \
        actor->sequence = 1;                                                                                                                                   \
        actor->previous_sequence = -1;                                                                                                                         \
        (fade) = 0;                                                                                                                                            \
        actor->resource_index = 0;                                                                                                                             \
        (timer) = (frames);                                                                                                                                    \
        (step)++;                                                                                                                                              \
        next();                                                                                                                                                \
    }

extern s32 rand(void);
extern s32 g_wmap_land_effect_13_sequence_6_timer;
extern s32 g_wmap_land_effect_13_sequence_7_timer;
extern s32 g_wmap_land_effect_13_sequence_8_timer;
/** @brief Sprite shade for land effect 13 sequence 9. */
extern s32 g_wmap_land_effect_13_sequence_9_shade;
extern s32 g_wmap_land_effect_13_sequence_9_timer;
extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_13_sequence_12_timer;
extern s32 g_wmap_land_effect_13_sequence_2_timer;
extern WmapSequenceStep g_wmap_land_effect_13_sequence_2_steps[];
extern s32 g_wmap_land_effect_13_timer;
extern WmapSequenceStep g_wmap_land_effect_13_steps[];
extern s32 g_wmap_land_effect_13_timeline_timer;
extern WmapSequenceStep g_wmap_land_effect_13_timeline_steps[];
extern s32 g_wmap_land_effect_13_sequence_1_timer;
extern WmapSequenceStep g_wmap_land_effect_13_sequence_1_steps[];
extern s32 g_wmap_land_effect_13_sequence_3_timer;
extern WmapSequenceStep g_wmap_land_effect_13_sequence_3_steps[];
extern s32 g_wmap_land_effect_13_sequence_4_timer;
extern WmapSequenceStep g_wmap_land_effect_13_sequence_4_steps[];
extern s32 g_wmap_land_effect_13_sequence_5_timer;
extern WmapSequenceStep g_wmap_land_effect_13_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_effect_13_sequence_6_steps[];
extern WmapSequenceStep g_wmap_land_effect_13_sequence_7_steps[];
extern WmapSequenceStep g_wmap_land_effect_13_sequence_8_steps[];
/** @brief Step callbacks for land effect 13 sequence 9. */
extern WmapSequenceStep g_wmap_land_effect_13_sequence_9_steps[];
extern s32 g_wmap_land_effect_13_sequence_10_timer;
extern WmapSequenceStep g_wmap_land_effect_13_sequence_10_steps[];
extern s32 g_wmap_land_effect_13_sequence_11_timer;
extern WmapSequenceStep g_wmap_land_effect_13_sequence_11_steps[];
extern WmapSequenceStep g_wmap_land_effect_13_sequence_12_steps[];
extern u32 g_wmap_land_effect_13_sequence_6_step;
extern u32 g_wmap_land_effect_13_sequence_7_step;
extern u32 g_wmap_land_effect_13_sequence_8_step;
extern u32 g_wmap_land_effect_13_sequence_9_step;
extern u32 g_wmap_land_effect_13_sequence_12_step;
extern u32 g_wmap_land_effect_13_sequence_2_step;
extern u32 g_wmap_land_effect_13_step;
extern u32 g_wmap_land_effect_13_timeline_step;
extern u32 g_wmap_land_effect_13_sequence_1_step;
extern u32 g_wmap_land_effect_13_sequence_3_step;
extern u32 g_wmap_land_effect_13_sequence_4_step;
extern u32 g_wmap_land_effect_13_sequence_5_step;
extern u32 g_wmap_land_effect_13_sequence_10_step;
extern u32 g_wmap_land_effect_13_sequence_11_step;

/**
 * @brief Update and draw active particles, then initialize the first available slot.
 * @param actors Actor configurations for the particle slots.
 * @param resources Animation resources corresponding to the actor slots.
 * @param count Number of particle slots to process.
 */
static void wmap_land_effect_13_update_particles(WmapSpriteActor* actors, WmapAnimationSlot* resources, s32 count)
{
    SVECTOR position;
    s32 screen_position;
    s32 active_count;
    s32 i;
    WmapMotion* motion;
    WmapSpriteActor* actor;

    active_count = 0;

    for (i = 0; i < count; i++)
    {
        motion = &g_wmap_actor_motions[i];

        if (motion->state != 0)
        {
            position.vx = ((motion->radius >> 3) * (ccos(motion->angle) >> 6)) >> 12;
            position.vy = ((motion->radius >> 3) * (csin(motion->angle) >> 6)) >> 12;
            position.vz = motion->z;
            gte_ldv0(&position);
            gte_rtps();
            motion->z += motion->velocity;
            motion->radius += 1000;
            gte_stsxy(&screen_position);
            wmap_step_actor_animation(&actors[i], &resources[i]);
            wmap_draw_actor_sprite(&actors[i], screen_position, 15, 7, 0);
            motion->lifetime--;

            if (motion->lifetime == 0)
            {
                motion->state = 0;
            }

            active_count++;
        }
    }

    g_wmap_unused_tally = active_count;

    for (i = 0; i < count; i++)
    {
        WmapMotion* slot;

        slot = &g_wmap_actor_motions[i];
        actor = &actors[i];

        if (slot->state == 0)
        {
            if (g_wmap_particle_intensity >= active_count)
            {
                actor->scale_index = WMAP_SPRITE_SCALE_FULL;
                actor->sequence = 3;
                actor->previous_sequence = -1;
                actor->target_shade = 129;
                actor->shade = 129;
                actor->resource_index = 0;
                slot->state = 1;
                slot->angle = rand();
                slot->radius = 10000;
                slot->velocity = ((rand() * 5) >> 15) + 1;
                slot->lifetime = ((rand() * 2) >> 15) + 24;
                slot->z = 0;
            }

            break;
        }
    }
}

/**
 * @brief Fade a world-map actor in at a cursor-relative offset, then advance when its timer expires.
 * @note Copies the fade level into two object fields before drawing and clamps it at 0x81.
 */
void wmap_land_effect_13_sequence_6_step_02(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[16];
    s32 next_shade;
    WmapScreenPosition position;

    position.point.x = g_wmap_focus_screen_position.point.x - 6;
    position.point.y = g_wmap_focus_screen_position.point.y + 12;
    actor->shade = g_wmap_effect_fade_b;
    actor->target_shade = g_wmap_effect_fade_b;
    wmap_step_actor_animation(&g_wmap_sprite_actors[16], &g_wmap_actor_animations[16]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[16], position.packed, 4, 11, 0);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_b, 8, 0x81, next_shade);

    if (--g_wmap_land_effect_13_sequence_6_timer == 0)
    {
        g_wmap_land_effect_13_sequence_6_step++;
    }
}

/**
 * @brief Draw a world-map actor at a cursor-relative offset, then fade it out and advance when its timer expires.
 * @note Copies the fade level into two object fields after drawing and clamps it at 0.
 */
void wmap_land_effect_13_sequence_6_step_04(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[16];
    s32 remaining_shade;
    WmapScreenPosition position;

    position.point.x = g_wmap_focus_screen_position.point.x - 6;
    position.point.y = g_wmap_focus_screen_position.point.y + 12;
    wmap_step_actor_animation(&g_wmap_sprite_actors[16], &g_wmap_actor_animations[16]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[16], position.packed, 4, 11, 0);
    actor->shade = g_wmap_effect_fade_b;
    actor->target_shade = g_wmap_effect_fade_b;
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_b, 8, remaining_shade);

    if (--g_wmap_land_effect_13_sequence_6_timer == 0)
    {
        g_wmap_land_effect_13_sequence_6_step++;
    }
}

/** @brief Draw the actor, increase its shade, and advance when its timer expires. */
void wmap_land_effect_13_sequence_7_step_02(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[17];
    s32 next_shade;
    WmapScreenPosition position;

    position.point.x = g_wmap_focus_screen_position.point.x + 16;
    position.point.y = g_wmap_focus_screen_position.point.y;
    actor->shade = g_wmap_aux_effect_fade_a;
    actor->target_shade = g_wmap_aux_effect_fade_a;
    wmap_step_actor_animation(&g_wmap_sprite_actors[17], &g_wmap_actor_animations[17]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[17], position.packed, 4, 11, 0);
    WMAP_MODEL_FADE_IN(g_wmap_aux_effect_fade_a, 8, 0x81, next_shade);

    if (--g_wmap_land_effect_13_sequence_7_timer == 0)
    {
        g_wmap_land_effect_13_sequence_7_step++;
    }
}

/** @brief Draw the actor, reduce its shade, and advance when its timer expires. */
void wmap_land_effect_13_sequence_7_step_04(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[17];
    s32 remaining_shade;
    WmapScreenPosition position;

    position.point.x = g_wmap_focus_screen_position.point.x + 16;
    position.point.y = g_wmap_focus_screen_position.point.y;
    wmap_step_actor_animation(&g_wmap_sprite_actors[17], &g_wmap_actor_animations[17]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[17], position.packed, 4, 11, 0);
    actor->shade = g_wmap_aux_effect_fade_a;
    actor->target_shade = g_wmap_aux_effect_fade_a;
    WMAP_MODEL_FADE_OUT(g_wmap_aux_effect_fade_a, 8, remaining_shade);

    if (--g_wmap_land_effect_13_sequence_7_timer == 0)
    {
        g_wmap_land_effect_13_sequence_7_step++;
    }
}

/** @brief Draw the actor, increase its shade, and advance when its timer expires. */
void wmap_land_effect_13_sequence_8_step_02(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[18];
    s32 next_shade;
    WmapScreenPosition position;

    actor->shade = g_wmap_aux_effect_fade_b;
    actor->target_shade = g_wmap_aux_effect_fade_b;
    position.point.x = g_wmap_focus_screen_position.point.x + 24;
    position.point.y = g_wmap_focus_screen_position.point.y + 4;
    wmap_step_actor_animation(&g_wmap_sprite_actors[18], &g_wmap_actor_animations[18]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[18], position.packed, 4, 11, 0);
    WMAP_MODEL_FADE_IN(g_wmap_aux_effect_fade_b, 8, 0x81, next_shade);

    if (--g_wmap_land_effect_13_sequence_8_timer == 0)
    {
        g_wmap_land_effect_13_sequence_8_step++;
    }
}

/**
 * @brief Draw a world-map actor at a cursor-relative offset, then fade it out and advance when its timer expires.
 * @note Copies the fade level into two object fields after drawing and clamps it at 0.
 */
void wmap_land_effect_13_sequence_8_step_04(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[18];
    s32 remaining_shade;
    WmapScreenPosition position;

    position.point.x = g_wmap_focus_screen_position.point.x + 24;
    position.point.y = g_wmap_focus_screen_position.point.y + 4;
    wmap_step_actor_animation(&g_wmap_sprite_actors[18], &g_wmap_actor_animations[18]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[18], position.packed, 4, 11, 0);
    actor->shade = g_wmap_aux_effect_fade_b;
    actor->target_shade = g_wmap_aux_effect_fade_b;
    WMAP_MODEL_FADE_OUT(g_wmap_aux_effect_fade_b, 8, remaining_shade);

    if (--g_wmap_land_effect_13_sequence_8_timer == 0)
    {
        g_wmap_land_effect_13_sequence_8_step++;
    }
}

/**
 * @brief Fade a world-map actor in at a cursor-relative offset, then advance when its timer expires.
 * @note Copies the current shade before drawing, then raises it toward 0x81.
 */
void wmap_land_effect_13_sequence_9_step_02(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[19];
    WmapScreenPosition position;

    position.point.x = g_wmap_focus_screen_position.point.x - 24;
    position.point.y = g_wmap_focus_screen_position.point.y - 10;
    actor->shade = g_wmap_land_effect_13_sequence_9_shade;
    actor->target_shade = g_wmap_land_effect_13_sequence_9_shade;
    wmap_step_actor_animation(&g_wmap_sprite_actors[19], &g_wmap_actor_animations[19]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[19], position.packed, 4, 11, 0);
    g_wmap_land_effect_13_sequence_9_shade += 8;

    if (g_wmap_land_effect_13_sequence_9_shade >= 0x82)
    {
        g_wmap_land_effect_13_sequence_9_shade = 0x81;
    }

    if (--g_wmap_land_effect_13_sequence_9_timer == 0)
    {
        g_wmap_land_effect_13_sequence_9_step++;
    }
}

/**
 * @brief Draw a world-map actor at a cursor-relative offset, then fade it out and advance when its timer expires.
 * @note Copies the current shade after drawing, then reduces it toward zero.
 */
void wmap_land_effect_13_sequence_9_step_04(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[19];
    WmapScreenPosition position;

    position.point.x = g_wmap_focus_screen_position.point.x - 24;
    position.point.y = g_wmap_focus_screen_position.point.y - 10;
    wmap_step_actor_animation(&g_wmap_sprite_actors[19], &g_wmap_actor_animations[19]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[19], position.packed, 4, 11, 0);
    actor->shade = g_wmap_land_effect_13_sequence_9_shade;
    actor->target_shade = g_wmap_land_effect_13_sequence_9_shade;
    g_wmap_land_effect_13_sequence_9_shade -= 8;

    if (g_wmap_land_effect_13_sequence_9_shade < 0)
    {
        g_wmap_land_effect_13_sequence_9_shade = 0;
    }

    if (--g_wmap_land_effect_13_sequence_9_timer == 0)
    {
        g_wmap_land_effect_13_sequence_9_step++;
    }
}

/** @brief Draw the rotating effect, raise its intensity, and advance its countdown. */
void wmap_land_effect_13_sequence_12_step_02(void)
{
    MATRIX matrix;
    s32 intensity;

    PushMatrix();
    RotMatrix(&g_wmap_effect_model_a_rotation, &matrix);
    TransMatrix(&matrix, &g_wmap_effect_model_d_position);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    wmap_draw_model_default(g_wmap_load_buffer, 0, 36, 183, 0x7A40, 1, -1);
    PopMatrix();
    intensity = g_wmap_aux_effect_fade_c + 15;
    g_wmap_aux_effect_fade_c = intensity;

    if (intensity >= 129)
    {
        g_wmap_aux_effect_fade_c = 128;
    }

    g_wmap_effect_model_a_rotation.vz = (g_wmap_effect_model_a_rotation.vz + 40);
    g_wmap_effect_model_d_position.vz -= 7000;

    if (--g_wmap_land_effect_13_sequence_12_timer == 0)
    {
        g_wmap_land_effect_13_sequence_12_step++;
    }
}

/** @brief Draw and fade the rotating effect, then advance its countdown. */
void wmap_land_effect_13_sequence_12_step_04(void)
{
    MATRIX matrix;
    s32 intensity;
    s32 remaining;

    if (g_wmap_aux_effect_fade_c != 0)
    {
        PushMatrix();
        RotMatrix(&g_wmap_effect_model_a_rotation, &matrix);
        TransMatrix(&matrix, &g_wmap_effect_model_d_position);
        SetRotMatrix(&matrix);
        SetTransMatrix(&matrix);
        wmap_draw_model_default(g_wmap_load_buffer, 0, 36, 183, 0x7A40, 1, g_wmap_aux_effect_fade_c);
        WMAP_MODEL_FADE_OUT(g_wmap_aux_effect_fade_c, 4, intensity);
        PopMatrix();
        g_wmap_effect_model_a_rotation.vz = (g_wmap_effect_model_a_rotation.vz + 40);
    }

    remaining = g_wmap_land_effect_13_sequence_12_timer - 1;
    g_wmap_land_effect_13_sequence_12_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_13_sequence_12_step++;
    }
}

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_2, g_wmap_land_effect_13_sequence_2_steps, 0x4, g_wmap_land_effect_13_sequence_2_step,
                               g_wmap_land_effect_13_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_2_reset, g_wmap_land_effect_13_sequence_2_step, g_wmap_land_effect_13_sequence_2_timer)

/** @brief World-map step handler: init a UI descriptor block, set the timer, advance the step. */
void wmap_land_effect_13_sequence_2_step_01(void)
{
    WmapSpriteActor* actors = g_wmap_sprite_actors;

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_0;
    actors[5].scale_index = WMAP_SPRITE_SCALE_FULL;
    actors[5].sequence = 1;
    actors[5].previous_sequence = -1;
    actors[5].resource_index = 0;
    actors[5].target_shade = WMAP_SPRITE_SHADE_NEUTRAL;
    actors[5].shade = WMAP_SPRITE_SHADE_NEUTRAL;
    g_wmap_land_effect_13_sequence_2_timer = 0x36;
    g_wmap_land_effect_13_sequence_2_step++;
    wmap_land_effect_13_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_13_sequence_2_step_02, g_wmap_land_effect_13_sequence_2_step, g_wmap_land_effect_13_sequence_2_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0xF, 0xB, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_2_end, g_wmap_land_effect_13_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_13_run, g_wmap_land_effect_13_steps, 0x6, g_wmap_land_effect_13_step, g_wmap_land_effect_13_timer)

WMAP_STEP_RESET(wmap_land_effect_13_reset, g_wmap_land_effect_13_step, g_wmap_land_effect_13_timer)

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void wmap_land_effect_13_step_01(void)
{
    g_wmap_input_locked = 1;
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_13_step++;
    wmap_land_effect_13_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_13_wait_idle_02, g_wmap_land_effect_13_step, wmap_land_effect_13_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_13_step_03, g_wmap_land_effect_13_step, wmap_land_effect_13_run_timeline, wmap_land_effect_13_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_13_wait_idle_04, g_wmap_land_effect_13_step, wmap_land_effect_13_end)

WMAP_STEP_ADVANCE(wmap_land_effect_13_end, g_wmap_land_effect_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_13_run_timeline, g_wmap_land_effect_13_timeline_steps, 0x16, g_wmap_land_effect_13_timeline_step,
                 g_wmap_land_effect_13_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_13_timeline_reset, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

/** @brief Enable the world-map flag, set the drawing color, and start a two-tick delay. */
void wmap_land_effect_13_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x404040);
    g_wmap_land_effect_13_timeline_timer = 2;
    g_wmap_land_effect_13_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_02, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

/** @brief World-map step handler: register a callback and advance the step counter. */
void wmap_land_effect_13_timeline_step_03(void)
{
    wmap_play_sound(0x10, 0x80);
    wmap_start_sequence(wmap_land_effect_13_run_sequence_1);
    g_wmap_land_effect_13_timeline_timer = 8;
    g_wmap_land_effect_13_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_04, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

/**
 * @brief Hide the placement overlay, start the sequence, and set the wait timer.
 */
WMAP_STEP_HIDE_AND_START(wmap_land_effect_13_timeline_step_05, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer,
                         g_wmap_placement_overlay_hidden, wmap_land_effect_13_run_sequence_2, 0x24)

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_06, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_13_timeline_step_07, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer,
                             wmap_land_effect_13_run_sequence_4, wmap_land_effect_13_run_sequence_3, 0x20)

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_08, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_13_timeline_step_09, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer,
                         wmap_land_effect_13_run_sequence_12, 0xE)

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_10, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_13_timeline_step_11, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer,
                         wmap_land_effect_13_run_sequence_10, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_12, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_13_timeline_step_13, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer,
                         wmap_land_effect_13_run_sequence_11, 0x26)

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_14, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_13_timeline_step_15, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer,
                         wmap_land_effect_13_run_sequence_5, 1)

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_16, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

/** @brief Register two sequence callbacks and advance to a one-tick delay. */
void wmap_land_effect_13_timeline_step_17(void)
{
    wmap_start_sequence(&wmap_land_effect_13_run_sequence_6);
    wmap_start_sequence(&wmap_land_effect_13_run_sequence_7);
    g_wmap_land_effect_13_timeline_timer = 1;
    g_wmap_land_effect_13_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_18, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_13_timeline_step_19, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer,
                             wmap_land_effect_13_run_sequence_8, wmap_land_effect_13_run_sequence_9, 0x3B)

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_20, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_13_timeline_finish, g_wmap_land_effect_13_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y,
                           g_wmap_selected_artifact)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_1, g_wmap_land_effect_13_sequence_1_steps, 0x6, g_wmap_land_effect_13_sequence_1_step,
                               g_wmap_land_effect_13_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_1_reset, g_wmap_land_effect_13_sequence_1_step, g_wmap_land_effect_13_sequence_1_timer)

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void wmap_land_effect_13_sequence_1_step_01(void)
{
    WmapSpriteActor* actors;

    g_wmap_actor_animations[4].data = g_wmap_animation_bank_0;
    actors = g_wmap_sprite_actors;
    actors[4].scale_index = WMAP_SPRITE_SCALE_FULL;
    actors[4].resource_index = 0;
    actors[4].sequence = 0;
    actors[4].previous_sequence = -1;
    g_wmap_effect_wave_phase = 0;
    g_wmap_land_effect_13_sequence_1_timer = 8;
    g_wmap_land_effect_13_sequence_1_step++;
    wmap_land_effect_13_sequence_1_step_02();
}

/** @brief Draw the actor, increase its scale, and advance when its timer expires. */
void wmap_land_effect_13_sequence_1_step_02(void)
{
    WmapSpriteActor* obj;
    WmapSpriteActor* actors;

    obj = &g_wmap_sprite_actors[4];
    actors = obj - 4;
    actors[4].shade = *&g_wmap_effect_wave_phase;
    actors[4].target_shade = *&g_wmap_effect_wave_phase;
    wmap_step_actor_animation(obj, &g_wmap_actor_animations[4]);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 15, 4, 0);
    g_wmap_effect_wave_phase += 8;

    if (g_wmap_effect_wave_phase >= 0x82)
    {
        g_wmap_effect_wave_phase = 0x81;
    }

    if (--g_wmap_land_effect_13_sequence_1_timer == 0)
    {
        g_wmap_land_effect_13_sequence_1_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_1_step_03, g_wmap_land_effect_13_sequence_1_step, g_wmap_land_effect_13_sequence_1_timer, 0x20,
                    wmap_land_effect_13_sequence_1_step_04)

/** @brief Draw the actor, reduce its scale, and advance when its timer expires. */
void wmap_land_effect_13_sequence_1_step_04(void)
{
    WmapSpriteActor* obj;
    WmapSpriteActor* actors;

    obj = &g_wmap_sprite_actors[4];
    actors = obj - 4;
    actors[4].shade = *&g_wmap_effect_wave_phase;
    actors[4].target_shade = *&g_wmap_effect_wave_phase;
    wmap_step_actor_animation(obj, &g_wmap_actor_animations[4]);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 15, 4, 0);
    g_wmap_effect_wave_phase -= 2;

    if (g_wmap_effect_wave_phase < 0)
    {
        g_wmap_effect_wave_phase = 0;
    }

    if (--g_wmap_land_effect_13_sequence_1_timer == 0)
    {
        g_wmap_land_effect_13_sequence_1_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_1_end, g_wmap_land_effect_13_sequence_1_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_3, g_wmap_land_effect_13_sequence_3_steps, 0x4, g_wmap_land_effect_13_sequence_3_step,
                               g_wmap_land_effect_13_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_3_reset, g_wmap_land_effect_13_sequence_3_step, g_wmap_land_effect_13_sequence_3_timer)

/** @brief World-map step: reset a run of slot tables then advance the sub-counter. */
void wmap_land_effect_13_sequence_3_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 0x18;
    WMAP_RESET_PARTICLE_SLOTS(i, 0x18, g_wmap_actor_motions[i].state, 0xCC, g_wmap_animation_bank_0);
    g_wmap_land_effect_13_sequence_3_timer = 0x30;
    g_wmap_land_effect_13_sequence_3_step++;
    wmap_land_effect_13_sequence_3_step_02();
}

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_13_sequence_3_step_02, g_wmap_land_effect_13_sequence_3_step, g_wmap_land_effect_13_sequence_3_timer,
                          wmap_land_effect_13_update_particles(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x18))

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_3_end, g_wmap_land_effect_13_sequence_3_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_4, g_wmap_land_effect_13_sequence_4_steps, 0x6, g_wmap_land_effect_13_sequence_4_step,
                               g_wmap_land_effect_13_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_4_reset, g_wmap_land_effect_13_sequence_4_step, g_wmap_land_effect_13_sequence_4_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_13_sequence_4_step_01(void)
{
    g_wmap_actor_animations[6].data = g_wmap_animation_bank_0;
    g_wmap_sprite_actors[6].scale_index = WMAP_SPRITE_SCALE_FULL;
    g_wmap_sprite_actors[6].sequence = 2;
    g_wmap_sprite_actors[6].previous_sequence = -1;
    g_wmap_effect_fade_e = 0;
    g_wmap_sprite_actors[6].resource_index = 0;
    g_wmap_land_effect_13_sequence_4_timer = 0x28;
    g_wmap_land_effect_13_sequence_4_step++;
    wmap_land_effect_13_sequence_4_step_02();
}

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void wmap_land_effect_13_sequence_4_step_02(void)
{
    WMAP_EFFECT13_DRAW_BRIGHTENING_SPRITE(6, g_wmap_effect_fade_e, 0xF, 4, 0x81);

    if (--g_wmap_land_effect_13_sequence_4_timer == 0)
    {
        g_wmap_land_effect_13_sequence_4_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_4_step_03, g_wmap_land_effect_13_sequence_4_step, g_wmap_land_effect_13_sequence_4_timer, 0x20,
                    wmap_land_effect_13_sequence_4_step_04)

/** @brief Draw the actor, reduce its scale, and advance when its timer expires. */
void wmap_land_effect_13_sequence_4_step_04(void)
{
    WmapSpriteActor* actors;

    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], g_wmap_focus_screen_position.packed, 15, 11, 0);
    actors = &g_wmap_sprite_actors[6] - 6;
    actors[6].shade = *&g_wmap_effect_fade_e;
    actors[6].target_shade = *&g_wmap_effect_fade_e;
    g_wmap_effect_fade_e -= 8;

    if (g_wmap_effect_fade_e < 0)
    {
        g_wmap_effect_fade_e = 0;
    }

    if (--g_wmap_land_effect_13_sequence_4_timer == 0)
    {
        g_wmap_land_effect_13_sequence_4_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_4_end, g_wmap_land_effect_13_sequence_4_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_5, g_wmap_land_effect_13_sequence_5_steps, 0x6, g_wmap_land_effect_13_sequence_5_step,
                               g_wmap_land_effect_13_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_5_reset, g_wmap_land_effect_13_sequence_5_step, g_wmap_land_effect_13_sequence_5_timer)

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void wmap_land_effect_13_sequence_5_step_01(void)
{
    WmapSpriteActor* actor;

    g_wmap_actor_animations[7].data = g_wmap_animation_bank_1;
    actor = &g_wmap_sprite_actors[7];
    actor->scale_index = WMAP_SPRITE_SCALE_FULL;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->previous_sequence = -1;
    g_wmap_effect_fade_a = 0;
    g_wmap_land_effect_13_sequence_5_timer = 0xC;
    g_wmap_land_effect_13_sequence_5_step++;
    wmap_land_effect_13_sequence_5_step_02();
}

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void wmap_land_effect_13_sequence_5_step_02(void)
{
    WMAP_EFFECT13_DRAW_BRIGHTENING_SPRITE(7, g_wmap_effect_fade_a, 0x4, 0x18, 0x81);

    if (--g_wmap_land_effect_13_sequence_5_timer == 0)
    {
        g_wmap_land_effect_13_sequence_5_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_5_step_03, g_wmap_land_effect_13_sequence_5_step, g_wmap_land_effect_13_sequence_5_timer, 0x24,
                    wmap_land_effect_13_sequence_5_step_04)

/** @brief World-map step: build a sprite, decrement a shared budget, expire the timer. */
void wmap_land_effect_13_sequence_5_step_04(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[7];

    wmap_step_actor_animation(actor, &g_wmap_actor_animations[7]);
    wmap_draw_actor_sprite(actor, g_wmap_focus_screen_position.packed, 4, 0xB, 0);
    actor->shade = g_wmap_effect_fade_a;
    actor->target_shade = g_wmap_effect_fade_a;
    g_wmap_effect_fade_a -= 4;

    if (g_wmap_effect_fade_a < 0)
    {
        g_wmap_effect_fade_a = 0;
    }

    if (--g_wmap_land_effect_13_sequence_5_timer == 0)
    {
        g_wmap_land_effect_13_sequence_5_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_5_end, g_wmap_land_effect_13_sequence_5_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_6, g_wmap_land_effect_13_sequence_6_steps, 0x6, g_wmap_land_effect_13_sequence_6_step,
                               g_wmap_land_effect_13_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_6_reset, g_wmap_land_effect_13_sequence_6_step, g_wmap_land_effect_13_sequence_6_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
WMAP_EFFECT13_START_SPRITE(wmap_land_effect_13_sequence_6_step_01, g_wmap_land_effect_13_sequence_6_step, g_wmap_land_effect_13_sequence_6_timer, 16,
                           g_wmap_animation_bank_1, g_wmap_effect_fade_b, 0x18, wmap_land_effect_13_sequence_6_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_6_step_03, g_wmap_land_effect_13_sequence_6_step, g_wmap_land_effect_13_sequence_6_timer, 0x28,
                    wmap_land_effect_13_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_6_end, g_wmap_land_effect_13_sequence_6_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_7, g_wmap_land_effect_13_sequence_7_steps, 0x6, g_wmap_land_effect_13_sequence_7_step,
                               g_wmap_land_effect_13_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_7_reset, g_wmap_land_effect_13_sequence_7_step, g_wmap_land_effect_13_sequence_7_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
WMAP_EFFECT13_START_SPRITE(wmap_land_effect_13_sequence_7_step_01, g_wmap_land_effect_13_sequence_7_step, g_wmap_land_effect_13_sequence_7_timer, 17,
                           g_wmap_animation_bank_1, g_wmap_aux_effect_fade_a, 0x18, wmap_land_effect_13_sequence_7_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_7_step_03, g_wmap_land_effect_13_sequence_7_step, g_wmap_land_effect_13_sequence_7_timer, 0x20,
                    wmap_land_effect_13_sequence_7_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_7_end, g_wmap_land_effect_13_sequence_7_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_8, g_wmap_land_effect_13_sequence_8_steps, 0x6, g_wmap_land_effect_13_sequence_8_step,
                               g_wmap_land_effect_13_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_8_reset, g_wmap_land_effect_13_sequence_8_step, g_wmap_land_effect_13_sequence_8_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
WMAP_EFFECT13_START_SPRITE(wmap_land_effect_13_sequence_8_step_01, g_wmap_land_effect_13_sequence_8_step, g_wmap_land_effect_13_sequence_8_timer, 18,
                           g_wmap_animation_bank_1, g_wmap_aux_effect_fade_b, 0x20, wmap_land_effect_13_sequence_8_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_8_step_03, g_wmap_land_effect_13_sequence_8_step, g_wmap_land_effect_13_sequence_8_timer, 0x20,
                    wmap_land_effect_13_sequence_8_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_8_end, g_wmap_land_effect_13_sequence_8_step)

/**
 * @brief Run the current step of sequence 9, restarting it when requested.
 * @param reset Nonzero to restart the sequence and run its first step immediately.
 * @return Nonzero while the sequence has a step to run.
 */
WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_9, g_wmap_land_effect_13_sequence_9_steps, 0x6, g_wmap_land_effect_13_sequence_9_step,
                               g_wmap_land_effect_13_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_9_reset, g_wmap_land_effect_13_sequence_9_step, g_wmap_land_effect_13_sequence_9_timer)

/** @brief Start the sequence 9 sprite animation with zero shade and its fade-in timer. */
WMAP_EFFECT13_START_SPRITE(wmap_land_effect_13_sequence_9_step_01, g_wmap_land_effect_13_sequence_9_step, g_wmap_land_effect_13_sequence_9_timer, 19,
                           g_wmap_animation_bank_1, g_wmap_land_effect_13_sequence_9_shade, 0xC, wmap_land_effect_13_sequence_9_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_9_step_03, g_wmap_land_effect_13_sequence_9_step, g_wmap_land_effect_13_sequence_9_timer, 0x28,
                    wmap_land_effect_13_sequence_9_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_9_end, g_wmap_land_effect_13_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_13_run_sequence_10, g_wmap_land_effect_13_sequence_10_steps, 0x6, g_wmap_land_effect_13_sequence_10_step,
                 g_wmap_land_effect_13_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_10_reset, g_wmap_land_effect_13_sequence_10_step, g_wmap_land_effect_13_sequence_10_timer)

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void wmap_land_effect_13_sequence_10_step_01(void)
{
    WmapSpriteActor* actor;

    g_wmap_actor_animations[9].data = g_wmap_animation_bank_2;
    actor = &g_wmap_sprite_actors[9];
    actor->scale_index = WMAP_SPRITE_SCALE_FULL;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->previous_sequence = -1;
    g_wmap_effect_fade_c = 0;
    g_wmap_land_effect_13_sequence_10_timer = 0x10;
    g_wmap_land_effect_13_sequence_10_step++;
    wmap_land_effect_13_sequence_10_step_02();
}

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void wmap_land_effect_13_sequence_10_step_02(void)
{
    WMAP_EFFECT13_DRAW_BRIGHTENING_SPRITE(9, g_wmap_effect_fade_c, 0x10, 0x20, 0x81);

    if (--g_wmap_land_effect_13_sequence_10_timer == 0)
    {
        g_wmap_land_effect_13_sequence_10_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_10_step_03, g_wmap_land_effect_13_sequence_10_step, g_wmap_land_effect_13_sequence_10_timer, 0x10,
                    wmap_land_effect_13_sequence_10_step_04)

/** @brief World-map step: build a sprite, decrement a shared budget, expire the timer. */
void wmap_land_effect_13_sequence_10_step_04(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[9];

    wmap_step_actor_animation(actor, &g_wmap_actor_animations[9]);
    wmap_draw_actor_sprite(actor, g_wmap_focus_screen_position.packed, 0x10, 0xB, 0);
    actor->shade = g_wmap_effect_fade_c;
    actor->target_shade = g_wmap_effect_fade_c;
    g_wmap_effect_fade_c -= 0x10;

    if (g_wmap_effect_fade_c < 0)
    {
        g_wmap_effect_fade_c = 0;
    }

    if (--g_wmap_land_effect_13_sequence_10_timer == 0)
    {
        g_wmap_land_effect_13_sequence_10_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_10_end, g_wmap_land_effect_13_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_13_run_sequence_11, g_wmap_land_effect_13_sequence_11_steps, 0x6, g_wmap_land_effect_13_sequence_11_step,
                 g_wmap_land_effect_13_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_11_reset, g_wmap_land_effect_13_sequence_11_step, g_wmap_land_effect_13_sequence_11_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
WMAP_EFFECT13_START_SPRITE(wmap_land_effect_13_sequence_11_step_01, g_wmap_land_effect_13_sequence_11_step, g_wmap_land_effect_13_sequence_11_timer, 10,
                           g_wmap_animation_bank_2, g_wmap_effect_fade_d, 0x59, wmap_land_effect_13_sequence_11_step_02)

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void wmap_land_effect_13_sequence_11_step_02(void)
{
    WMAP_EFFECT13_DRAW_BRIGHTENING_SPRITE(10, g_wmap_effect_fade_d, 0x10, 0x8, 0x80);

    if (--g_wmap_land_effect_13_sequence_11_timer == 0)
    {
        g_wmap_land_effect_13_sequence_11_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_11_step_03, g_wmap_land_effect_13_sequence_11_step, g_wmap_land_effect_13_sequence_11_timer, 0x10,
                    wmap_land_effect_13_sequence_11_step_04)

/** @brief Draw the actor, update two effect fields, and advance when the countdown expires. */
void wmap_land_effect_13_sequence_11_step_04(void)
{
    s32 remaining_ticks;
    WmapSpriteActor* actor = &g_wmap_sprite_actors[10];

    wmap_step_actor_animation(&g_wmap_sprite_actors[10], &g_wmap_actor_animations[10]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[10], g_wmap_focus_screen_position.packed, 0x10, 0xB, 0);
    actor->shade = g_wmap_effect_fade_d;
    actor->target_shade = g_wmap_effect_fade_d;

    if (g_wmap_effect_fade_d < 0)
    {
        g_wmap_effect_fade_d = 0;
    }

    remaining_ticks = g_wmap_land_effect_13_sequence_11_timer - 1;
    g_wmap_land_effect_13_sequence_11_timer = remaining_ticks;

    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_13_sequence_11_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_11_end, g_wmap_land_effect_13_sequence_11_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_12, g_wmap_land_effect_13_sequence_12_steps, 0x6, g_wmap_land_effect_13_sequence_12_step,
                               g_wmap_land_effect_13_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_12_reset, g_wmap_land_effect_13_sequence_12_step, g_wmap_land_effect_13_sequence_12_timer)

/** @brief Restore the default transform and advance the sequence. */
void wmap_land_effect_13_sequence_12_step_01(void)
{
    g_wmap_aux_effect_fade_c = 0;
    g_wmap_effect_model_a_rotation = g_wmap_zero_rotation;
    g_wmap_effect_model_d_position = g_wmap_camera_translation;
    g_wmap_effect_model_d_position.vz = 0xC350;
    g_wmap_land_effect_13_sequence_12_timer = 10;
    g_wmap_land_effect_13_sequence_12_step++;
    wmap_land_effect_13_sequence_12_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_12_step_03, g_wmap_land_effect_13_sequence_12_step, g_wmap_land_effect_13_sequence_12_timer, 0x60,
                    wmap_land_effect_13_sequence_12_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_12_end, g_wmap_land_effect_13_sequence_12_step)
