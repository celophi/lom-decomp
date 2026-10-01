#include "wmap_main.h"
#include "wmap_land_effect_13.h"
#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "vector.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

void wmap_land_effect_13_sequence_2_step_02(void);
void wmap_land_effect_13_wait_idle_02(void);
void wmap_land_effect_13_step_03(void);
s32 wmap_land_effect_13_run_timeline(s32 arg0);
void wmap_land_effect_13_wait_idle_04(void);
void wmap_land_effect_13_end(void);
s32 wmap_land_effect_13_run_sequence_1(s32 arg0);
s32 wmap_land_effect_13_run_sequence_4(s32 arg0);
s32 wmap_land_effect_13_run_sequence_3(s32 arg0);
s32 wmap_land_effect_13_run_sequence_12(s32 arg0);
s32 wmap_land_effect_13_run_sequence_10(s32 arg0);
s32 wmap_land_effect_13_run_sequence_11(s32 arg0);
s32 wmap_land_effect_13_run_sequence_5(s32 arg0);
s32 wmap_land_effect_13_run_sequence_6(s32 arg0);
s32 wmap_land_effect_13_run_sequence_7(s32 arg0);
s32 wmap_land_effect_13_run_sequence_8(s32 arg0);
s32 wmap_land_effect_13_run_sequence_9(s32 arg0);
void wmap_land_effect_13_sequence_1_step_02(void);
void wmap_land_effect_13_sequence_1_step_04(void);
void wmap_land_effect_13_sequence_3_step_02(void);
void wmap_land_effect_13_sequence_4_step_02(void);
void wmap_land_effect_13_sequence_4_step_04(void);
void wmap_land_effect_13_sequence_5_step_02(void);
void wmap_land_effect_13_sequence_5_step_04(void);
void wmap_land_effect_13_sequence_10_step_02(void);
void wmap_land_effect_13_sequence_10_step_04(void);
void wmap_land_effect_13_sequence_11_step_02(void);
void wmap_land_effect_13_sequence_11_step_04(void);

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern s32 D_800D922C;
extern s32 rand(void);
extern s32 g_wmap_land_effect_13_sequence_6_timer;
extern s32 D_801B25DC;
extern s32 g_wmap_land_effect_13_sequence_7_timer;
extern s32 D_801B25E0;
extern s32 g_wmap_land_effect_13_sequence_8_timer;
extern s32 D_801B25E4;
extern s32 g_wmap_land_effect_13_sequence_9_timer;
extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_13_sequence_12_timer;
extern s32 D_801B25D8;
extern s32 g_wmap_land_effect_13_sequence_2_timer;
extern void (*D_800D5050[])(void);
extern s32 g_wmap_land_effect_13_timer;
extern void (*D_800D4FC8[])(void);
extern void wmap_land_effect_13_step_03(void);
extern void wmap_land_effect_13_end(void);
extern s32 g_wmap_land_effect_13_timeline_timer;
extern void (*D_800D4FE0[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_13_sequence_1_timer;
extern void (*D_800D5038[])(void);
extern s32 D_801B24B4;
extern s32 g_wmap_land_effect_13_sequence_3_timer;
extern void (*D_800D5060[])(void);
extern s32 g_wmap_land_effect_13_sequence_4_timer;
extern void (*D_800D5070[])(void);
extern s32 g_wmap_land_effect_13_sequence_5_timer;
extern void (*D_800D5088[])(void);
extern void (*D_800D50A0[])(void);
extern void (*D_800D50B8[])(void);
extern void (*D_800D50D0[])(void);
extern void (*D_800D50E8[])(void);
extern s32 g_wmap_land_effect_13_sequence_10_timer;
extern void (*D_800D5100[])(void);
extern s32 g_wmap_land_effect_13_sequence_11_timer;
extern void (*D_800D5118[])(void);
extern void (*D_800D5130[])(void);
extern u32 g_wmap_land_effect_13_sequence_6_step;
extern u32 g_wmap_land_effect_13_sequence_7_step;
extern u32 g_wmap_land_effect_13_sequence_8_step;
extern u32 g_wmap_land_effect_13_sequence_9_step;
extern u32 g_wmap_land_effect_13_sequence_12_step;
extern u32 g_wmap_land_effect_13_sequence_2_step;
extern u8 g_wmap_animation_bank_0[];
extern u32 g_wmap_land_effect_13_step;
extern u32 g_wmap_land_effect_13_timeline_step;
extern u32 g_wmap_land_effect_13_sequence_1_step;
extern u32 g_wmap_land_effect_13_sequence_3_step;
extern u32 g_wmap_land_effect_13_sequence_4_step;
extern u32 g_wmap_land_effect_13_sequence_5_step;
extern u8 g_wmap_animation_bank_1[];
extern u32 g_wmap_land_effect_13_sequence_10_step;
extern u8 g_wmap_animation_bank_2[];
extern u32 g_wmap_land_effect_13_sequence_11_step;

extern VECTOR g_wmap_camera_translation;



extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern WmapSlot14 g_wmap_actor_motions[];

extern s32 D_80182DE4;

/** @brief Project active particles and initialize the first available slot. */
void func_80074368(WmapSpriteActor *actors, WmapAnimationSlot *resources, s32 count)
{
    SVECTOR position;
    s32 screen_position;
    s32 active_count;
    s32 i;
    WmapMotion *motion;
    WmapSpriteActor *actor;

    active_count = 0;
    for (i = 0; i < count; i++)
    {
        motion = &g_wmap_actor_motions[i];
        if (motion->state != 0)
        {
            position.vx = ((motion->z >> 3) * (ccos(motion->angle) >> 6)) >> 12;
            position.vy = ((motion->z >> 3) * (csin(motion->angle) >> 6)) >> 12;
            position.vz = motion->field_0E;
            gte_ldv0(&position);
            gte_rtps();
            motion->field_0E += motion->x;
            motion->z += 1000;
            gte_stsxy(&screen_position);
            wmap_step_actor_animation(&actors[i], &resources[i]);
            wmap_draw_actor_sprite(&actors[i], screen_position, 15, 7, 0);
            motion->scale--;
            if (motion->scale == 0)
            {
                motion->state = 0;
            }
            active_count++;
        }
    }
    D_800D922C = active_count;
    for (i = 0; i < count; i++)
    {
        WmapMotion *slot;

        slot = &g_wmap_actor_motions[i];
        actor = &actors[i];
        if (slot->state == 0)
        {
            if (g_wmap_particle_intensity >= active_count)
            {
                actor->scale_index = 15;
                actor->sequence = 3;
                actor->previous_sequence = -1;
                actor->target_shade = 129;
                actor->shade = 129;
                actor->resource_index = 0;
                slot->state = 1;
                slot->angle = rand();
                slot->z = 10000;
                slot->x = ((rand() * 5) >> 15) + 1;
                slot->scale = ((rand() * 2) >> 15) + 24;
                slot->field_0E = 0;
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

    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = g_wmap_focus_screen_position.point.x - 6;
    position.half[1] = g_wmap_focus_screen_position.point.y + 12;
    actor->shade = *(u16 *)&g_wmap_effect_fade_b;
    actor->target_shade = *(u16 *)&g_wmap_effect_fade_b;
    wmap_step_actor_animation(&g_wmap_sprite_actors[16], &g_wmap_actor_animations[16]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[16], position.word, 4, 11, 0);
    g_wmap_effect_fade_b += 8;
    if (g_wmap_effect_fade_b >= 0x82)
    {
        g_wmap_effect_fade_b = 0x81;
    }
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

    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = g_wmap_focus_screen_position.point.x - 6;
    position.half[1] = g_wmap_focus_screen_position.point.y + 12;
    wmap_step_actor_animation(&g_wmap_sprite_actors[16], &g_wmap_actor_animations[16]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[16], position.word, 4, 11, 0);
    actor->shade = *(u16 *)&g_wmap_effect_fade_b;
    actor->target_shade = *(u16 *)&g_wmap_effect_fade_b;
    g_wmap_effect_fade_b -= 8;
    if (g_wmap_effect_fade_b < 0)
    {
        g_wmap_effect_fade_b = 0;
    }
    if (--g_wmap_land_effect_13_sequence_6_timer == 0)
    {
        g_wmap_land_effect_13_sequence_6_step++;
    }
}

/** @brief Draw the actor, increase its scale, and advance when its timer expires. */
void wmap_land_effect_13_sequence_7_step_02(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[17];

    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = g_wmap_focus_screen_position.point.x + 16;
    position.half[1] = g_wmap_focus_screen_position.point.y;
    actor->shade = *(u16 *)&D_801B25DC;
    actor->target_shade = *(u16 *)&D_801B25DC;
    wmap_step_actor_animation(&g_wmap_sprite_actors[17], &g_wmap_actor_animations[17]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[17], position.word, 4, 11, 0);
    D_801B25DC += 8;
    if (D_801B25DC >= 0x82)
    {
        D_801B25DC = 0x81;
    }
    if (--g_wmap_land_effect_13_sequence_7_timer == 0)
    {
        g_wmap_land_effect_13_sequence_7_step++;
    }
}

/** @brief Draw the actor, reduce its scale, and advance when its timer expires. */
void wmap_land_effect_13_sequence_7_step_04(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[17];

    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = g_wmap_focus_screen_position.point.x + 16;
    position.half[1] = g_wmap_focus_screen_position.point.y;
    wmap_step_actor_animation(&g_wmap_sprite_actors[17], &g_wmap_actor_animations[17]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[17], position.word, 4, 11, 0);
    actor->shade = *(u16 *)&D_801B25DC;
    actor->target_shade = *(u16 *)&D_801B25DC;
    D_801B25DC -= 8;
    if (D_801B25DC < 0)
    {
        D_801B25DC = 0;
    }
    if (--g_wmap_land_effect_13_sequence_7_timer == 0)
    {
        g_wmap_land_effect_13_sequence_7_step++;
    }
}

/** @brief Draw the actor, increase its scale, and advance when its timer expires. */
void wmap_land_effect_13_sequence_8_step_02(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[18];

    union
    {
        u16 half[2];
        s32 word;
    } position;

    actor->shade = *(u16 *)&D_801B25E0;
    actor->target_shade = *(u16 *)&D_801B25E0;
    position.half[0] = g_wmap_focus_screen_position.point.x + 24;
    position.half[1] = g_wmap_focus_screen_position.point.y + 4;
    wmap_step_actor_animation(&g_wmap_sprite_actors[18], &g_wmap_actor_animations[18]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[18], position.word, 4, 11, 0);
    D_801B25E0 += 8;
    if (D_801B25E0 >= 0x82)
    {
        D_801B25E0 = 0x81;
    }
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

    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = g_wmap_focus_screen_position.point.x + 24;
    position.half[1] = g_wmap_focus_screen_position.point.y + 4;
    wmap_step_actor_animation(&g_wmap_sprite_actors[18], &g_wmap_actor_animations[18]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[18], position.word, 4, 11, 0);
    actor->shade = *(u16 *)&D_801B25E0;
    actor->target_shade = *(u16 *)&D_801B25E0;
    D_801B25E0 -= 8;
    if (D_801B25E0 < 0)
    {
        D_801B25E0 = 0;
    }
    if (--g_wmap_land_effect_13_sequence_8_timer == 0)
    {
        g_wmap_land_effect_13_sequence_8_step++;
    }
}

/**
 * @brief Fade a world-map actor in at a cursor-relative offset, then advance when its timer expires.
 * @note Copies the fade level into two object fields before drawing and clamps it at 0x81.
 */
void wmap_land_effect_13_sequence_9_step_02(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[19];

    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = g_wmap_focus_screen_position.point.x - 24;
    position.half[1] = g_wmap_focus_screen_position.point.y - 10;
    actor->shade = *(u16 *)&D_801B25E4;
    actor->target_shade = *(u16 *)&D_801B25E4;
    wmap_step_actor_animation(&g_wmap_sprite_actors[19], &g_wmap_actor_animations[19]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[19], position.word, 4, 11, 0);
    D_801B25E4 += 8;
    if (D_801B25E4 >= 0x82)
    {
        D_801B25E4 = 0x81;
    }
    if (--g_wmap_land_effect_13_sequence_9_timer == 0)
    {
        g_wmap_land_effect_13_sequence_9_step++;
    }
}

/**
 * @brief Draw a world-map actor at a cursor-relative offset, then fade it out and advance when its timer expires.
 * @note Copies the fade level into two object fields after drawing and clamps it at 0.
 */
void wmap_land_effect_13_sequence_9_step_04(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[19];

    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = g_wmap_focus_screen_position.point.x - 24;
    position.half[1] = g_wmap_focus_screen_position.point.y - 10;
    wmap_step_actor_animation(&g_wmap_sprite_actors[19], &g_wmap_actor_animations[19]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[19], position.word, 4, 11, 0);
    actor->shade = *(u16 *)&D_801B25E4;
    actor->target_shade = *(u16 *)&D_801B25E4;
    D_801B25E4 -= 8;
    if (D_801B25E4 < 0)
    {
        D_801B25E4 = 0;
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
    intensity = D_801B25D8 + 15;
    D_801B25D8 = intensity;
    if (intensity >= 129)
    {
        D_801B25D8 = 128;
    }
    g_wmap_effect_model_a_rotation.vz = (u16)(g_wmap_effect_model_a_rotation.vz + 40);
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

    if (D_801B25D8 != 0)
    {
        PushMatrix();
        RotMatrix(&g_wmap_effect_model_a_rotation, &matrix);
        TransMatrix(&matrix, &g_wmap_effect_model_d_position);
        SetRotMatrix(&matrix);
        SetTransMatrix(&matrix);
        wmap_draw_model_default(g_wmap_load_buffer, 0, 36, 183, 0x7A40, 1, D_801B25D8);
        WMAP_MODEL_FADE_OUT(D_801B25D8, 4, intensity);
        PopMatrix();
        g_wmap_effect_model_a_rotation.vz = (u16)(g_wmap_effect_model_a_rotation.vz + 40);
    }
    remaining = g_wmap_land_effect_13_sequence_12_timer - 1;
    g_wmap_land_effect_13_sequence_12_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_13_sequence_12_step++;
    }
}

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_2, D_800D5050, 0x4, g_wmap_land_effect_13_sequence_2_step,
                               g_wmap_land_effect_13_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_2_reset, g_wmap_land_effect_13_sequence_2_step, g_wmap_land_effect_13_sequence_2_timer)

/** @brief World-map step handler: init a UI descriptor block, set the timer, advance the step. */
void wmap_land_effect_13_sequence_2_step_01(void)
{
    u8 *base = &g_wmap_sprite_actors;

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_0;
    *(u8 *)(base + 0xE2) = 0xF;
    *(s16 *)(base + 0xEA) = 1;
    *(s16 *)(base + 0xEC) = -1;
    *(s16 *)(base + 0xDE) = 0;
    *(s16 *)(base + 0xFE) = 0x80;
    *(s16 *)(base + 0x100) = 0x80;
    g_wmap_land_effect_13_sequence_2_timer = 0x36;
    g_wmap_land_effect_13_sequence_2_step += 1;
    wmap_land_effect_13_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_13_sequence_2_step_02, g_wmap_land_effect_13_sequence_2_step, g_wmap_land_effect_13_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0xF, 0xB, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_2_end, g_wmap_land_effect_13_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_13_run, D_800D4FC8, 0x6, g_wmap_land_effect_13_step, g_wmap_land_effect_13_timer)

WMAP_STEP_RESET(wmap_land_effect_13_reset, g_wmap_land_effect_13_step, g_wmap_land_effect_13_timer)

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void wmap_land_effect_13_step_01(void)
{
    g_wmap_input_locked = 1;
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_13_step += 1;
    wmap_land_effect_13_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_13_wait_idle_02, g_wmap_land_effect_13_step, wmap_land_effect_13_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_13_step_03, g_wmap_land_effect_13_step, wmap_land_effect_13_run_timeline, wmap_land_effect_13_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_13_wait_idle_04, g_wmap_land_effect_13_step, wmap_land_effect_13_end)

WMAP_STEP_ADVANCE(wmap_land_effect_13_end, g_wmap_land_effect_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_13_run_timeline, D_800D4FE0, 0x16, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_13_timeline_reset, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

/** @brief Enable the world-map flag, set the drawing color, and start a two-tick delay. */
void wmap_land_effect_13_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x404040);
    g_wmap_land_effect_13_timeline_timer = 2;
    g_wmap_land_effect_13_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_02, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

/** @brief World-map step handler: register a callback and advance the step counter. */
void wmap_land_effect_13_timeline_step_03(void)
{
    wmap_play_sound(0x10, 0x80);
    wmap_start_sequence(wmap_land_effect_13_run_sequence_1);
    g_wmap_land_effect_13_timeline_timer = 8;
    g_wmap_land_effect_13_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_04, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

/**
 * @brief Hide the placement overlay, start the sequence, and set the wait timer.
 */
WMAP_STEP_HIDE_AND_START(wmap_land_effect_13_timeline_step_05,
    g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer,
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
    g_wmap_land_effect_13_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_18, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_13_timeline_step_19, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer,
                             wmap_land_effect_13_run_sequence_8, wmap_land_effect_13_run_sequence_9, 0x3B)

WMAP_STEP_WAIT(wmap_land_effect_13_timeline_wait_20, g_wmap_land_effect_13_timeline_step, g_wmap_land_effect_13_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_13_timeline_finish, g_wmap_land_effect_13_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_1, D_800D5038, 0x6, g_wmap_land_effect_13_sequence_1_step,
                               g_wmap_land_effect_13_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_1_reset, g_wmap_land_effect_13_sequence_1_step, g_wmap_land_effect_13_sequence_1_timer)

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void wmap_land_effect_13_sequence_1_step_01(void)
{
    u8 *base;

    g_wmap_actor_animations[4].data = g_wmap_animation_bank_0;
    base = g_wmap_sprite_actors;
    *(u8 *)(base + 0xB6) = 0xF;
    *(s16 *)(base + 0xB2) = 0;
    *(s16 *)(base + 0xBE) = 0;
    *(s16 *)(base + 0xC0) = -1;
    D_801B24B4 = 0;
    g_wmap_land_effect_13_sequence_1_timer = 8;
    g_wmap_land_effect_13_sequence_1_step += 1;
    wmap_land_effect_13_sequence_1_step_02();
}

/** @brief Draw the actor, increase its scale, and advance when its timer expires. */
void wmap_land_effect_13_sequence_1_step_02(void)
{
    WmapSpriteActor *obj;
    WmapSpriteActor *actors;

    obj = &g_wmap_sprite_actors[4];
    actors = obj - 4;
    actors[4].shade = *(u16 *)&D_801B24B4;
    actors[4].target_shade = *(u16 *)&D_801B24B4;
    wmap_step_actor_animation(obj, &g_wmap_actor_animations[4]);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 15, 4, 0);
    D_801B24B4 += 8;
    if (D_801B24B4 >= 0x82)
    {
        D_801B24B4 = 0x81;
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
    WmapSpriteActor *obj;
    WmapSpriteActor *actors;

    obj = &g_wmap_sprite_actors[4];
    actors = obj - 4;
    actors[4].shade = *(u16 *)&D_801B24B4;
    actors[4].target_shade = *(u16 *)&D_801B24B4;
    wmap_step_actor_animation(obj, &g_wmap_actor_animations[4]);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 15, 4, 0);
    D_801B24B4 -= 2;
    if (D_801B24B4 < 0)
    {
        D_801B24B4 = 0;
    }
    if (--g_wmap_land_effect_13_sequence_1_timer == 0)
    {
        g_wmap_land_effect_13_sequence_1_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_1_end, g_wmap_land_effect_13_sequence_1_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_3, D_800D5060, 0x4, g_wmap_land_effect_13_sequence_3_step,
                               g_wmap_land_effect_13_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_3_reset, g_wmap_land_effect_13_sequence_3_step, g_wmap_land_effect_13_sequence_3_timer)

/** @brief World-map step: reset a run of slot tables then advance the sub-counter. */
void wmap_land_effect_13_sequence_3_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 0x18;
    WMAP_RESET_PARTICLE_SLOTS(i, 0x18,
                              g_wmap_actor_motions[i].field_00,
                              0xCC, g_wmap_animation_bank_0);
    g_wmap_land_effect_13_sequence_3_timer = 0x30;
    g_wmap_land_effect_13_sequence_3_step += 1;
    wmap_land_effect_13_sequence_3_step_02();
}

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_13_sequence_3_step_02,
    g_wmap_land_effect_13_sequence_3_step, g_wmap_land_effect_13_sequence_3_timer,
    func_80074368(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x18))

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_3_end, g_wmap_land_effect_13_sequence_3_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_4, D_800D5070, 0x6, g_wmap_land_effect_13_sequence_4_step,
                               g_wmap_land_effect_13_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_4_reset, g_wmap_land_effect_13_sequence_4_step, g_wmap_land_effect_13_sequence_4_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_13_sequence_4_step_01(void)
{
    g_wmap_actor_animations[6].data = g_wmap_animation_bank_0;
    g_wmap_sprite_actors[6].scale_index = 0xF;
    g_wmap_sprite_actors[6].sequence = 2;
    g_wmap_sprite_actors[6].previous_sequence = -1;
    D_80182DE4 = 0;
    g_wmap_sprite_actors[6].resource_index = 0;
    g_wmap_land_effect_13_sequence_4_timer = 0x28;
    g_wmap_land_effect_13_sequence_4_step += 1;
    wmap_land_effect_13_sequence_4_step_02();
}

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void wmap_land_effect_13_sequence_4_step_02(void)
{
    u8* obj = (u8*)&g_wmap_sprite_actors[6];
    u16 pos = *(u16*)&D_80182DE4;

    *(s16*)&obj[0x24] = pos;
    *(s16*)&obj[0x22] = pos;
    wmap_step_actor_animation(obj, &g_wmap_actor_animations[6]);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 0xF, 0xB, 0);
    D_80182DE4 += 4;
    if (D_80182DE4 >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    if (--g_wmap_land_effect_13_sequence_4_timer == 0)
    {
        g_wmap_land_effect_13_sequence_4_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_4_step_03, g_wmap_land_effect_13_sequence_4_step, g_wmap_land_effect_13_sequence_4_timer, 0x20,
                    wmap_land_effect_13_sequence_4_step_04)

/** @brief Draw the actor, reduce its scale, and advance when its timer expires. */
void wmap_land_effect_13_sequence_4_step_04(void)
{
    WmapSpriteActor *actors;

    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], g_wmap_focus_screen_position.packed, 15, 11, 0);
    actors = &g_wmap_sprite_actors[6] - 6;
    actors[6].shade = *(u16 *)&D_80182DE4;
    actors[6].target_shade = *(u16 *)&D_80182DE4;
    D_80182DE4 -= 8;
    if (D_80182DE4 < 0)
    {
        D_80182DE4 = 0;
    }
    if (--g_wmap_land_effect_13_sequence_4_timer == 0)
    {
        g_wmap_land_effect_13_sequence_4_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_4_end, g_wmap_land_effect_13_sequence_4_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_5, D_800D5088, 0x6, g_wmap_land_effect_13_sequence_5_step,
                               g_wmap_land_effect_13_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_5_reset, g_wmap_land_effect_13_sequence_5_step, g_wmap_land_effect_13_sequence_5_timer)

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void wmap_land_effect_13_sequence_5_step_01(void)
{
    u8 *base;

    g_wmap_actor_animations[7].data = g_wmap_animation_bank_1;
    base = (u8*)&g_wmap_sprite_actors[7];
    *(u8 *)(base + 0x6) = 0xF;
    *(s16 *)(base + 0x2) = 0;
    *(s16 *)(base + 0xE) = 0;
    *(s16 *)(base + 0x10) = -1;
    g_wmap_effect_fade_a = 0;
    g_wmap_land_effect_13_sequence_5_timer = 0xC;
    g_wmap_land_effect_13_sequence_5_step += 1;
    wmap_land_effect_13_sequence_5_step_02();
}

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void wmap_land_effect_13_sequence_5_step_02(void)
{
    u8* obj = (u8*)&g_wmap_sprite_actors[7];
    u16 pos = *(u16*)&g_wmap_effect_fade_a;

    *(s16*)&obj[0x24] = pos;
    *(s16*)&obj[0x22] = pos;
    wmap_step_actor_animation(obj, &g_wmap_actor_animations[7]);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 0x4, 0xB, 0);
    g_wmap_effect_fade_a += 0x18;
    if (g_wmap_effect_fade_a >= 0x82)
    {
        g_wmap_effect_fade_a = 0x81;
    }
    if (--g_wmap_land_effect_13_sequence_5_timer == 0)
    {
        g_wmap_land_effect_13_sequence_5_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_5_step_03, g_wmap_land_effect_13_sequence_5_step, g_wmap_land_effect_13_sequence_5_timer, 0x24,
                    wmap_land_effect_13_sequence_5_step_04)

/** @brief World-map step: build a sprite, decrement a shared budget, expire the timer. */
void wmap_land_effect_13_sequence_5_step_04(void)
{
    wmap_step_actor_animation(&g_wmap_sprite_actors[7], &g_wmap_actor_animations[7]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[7], g_wmap_focus_screen_position.packed, 4, 0xB, 0);
    *(s16 *)((u8*)&g_wmap_sprite_actors[7] + 0x24) = (u16)g_wmap_effect_fade_a;
    *(s16 *)((u8*)&g_wmap_sprite_actors[7] + 0x22) = (u16)g_wmap_effect_fade_a;
    g_wmap_effect_fade_a -= 4;
    if (g_wmap_effect_fade_a < 0)
    {
        g_wmap_effect_fade_a = 0;
    }
    if (--g_wmap_land_effect_13_sequence_5_timer == 0)
    {
        g_wmap_land_effect_13_sequence_5_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_5_end, g_wmap_land_effect_13_sequence_5_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_6, D_800D50A0, 0x6, g_wmap_land_effect_13_sequence_6_step,
                               g_wmap_land_effect_13_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_6_reset, g_wmap_land_effect_13_sequence_6_step, g_wmap_land_effect_13_sequence_6_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_13_sequence_6_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[16];

    g_wmap_actor_animations[16].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    g_wmap_effect_fade_b = 0;
    actor->resource_index = 0;
    g_wmap_land_effect_13_sequence_6_timer = 0x18;
    g_wmap_land_effect_13_sequence_6_step += 1;
    wmap_land_effect_13_sequence_6_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_6_step_03, g_wmap_land_effect_13_sequence_6_step, g_wmap_land_effect_13_sequence_6_timer, 0x28,
                    wmap_land_effect_13_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_6_end, g_wmap_land_effect_13_sequence_6_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_7, D_800D50B8, 0x6, g_wmap_land_effect_13_sequence_7_step,
                               g_wmap_land_effect_13_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_7_reset, g_wmap_land_effect_13_sequence_7_step, g_wmap_land_effect_13_sequence_7_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_13_sequence_7_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[17];

    g_wmap_actor_animations[17].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    D_801B25DC = 0;
    actor->resource_index = 0;
    g_wmap_land_effect_13_sequence_7_timer = 0x18;
    g_wmap_land_effect_13_sequence_7_step += 1;
    wmap_land_effect_13_sequence_7_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_7_step_03, g_wmap_land_effect_13_sequence_7_step, g_wmap_land_effect_13_sequence_7_timer, 0x20,
                    wmap_land_effect_13_sequence_7_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_7_end, g_wmap_land_effect_13_sequence_7_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_8, D_800D50D0, 0x6, g_wmap_land_effect_13_sequence_8_step,
                               g_wmap_land_effect_13_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_8_reset, g_wmap_land_effect_13_sequence_8_step, g_wmap_land_effect_13_sequence_8_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_13_sequence_8_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[18];

    g_wmap_actor_animations[18].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    D_801B25E0 = 0;
    actor->resource_index = 0;
    g_wmap_land_effect_13_sequence_8_timer = 0x20;
    g_wmap_land_effect_13_sequence_8_step += 1;
    wmap_land_effect_13_sequence_8_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_8_step_03, g_wmap_land_effect_13_sequence_8_step, g_wmap_land_effect_13_sequence_8_timer, 0x20,
                    wmap_land_effect_13_sequence_8_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_8_end, g_wmap_land_effect_13_sequence_8_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_9, D_800D50E8, 0x6, g_wmap_land_effect_13_sequence_9_step,
                               g_wmap_land_effect_13_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_9_reset, g_wmap_land_effect_13_sequence_9_step, g_wmap_land_effect_13_sequence_9_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_13_sequence_9_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[19];

    g_wmap_actor_animations[19].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    D_801B25E4 = 0;
    actor->resource_index = 0;
    g_wmap_land_effect_13_sequence_9_timer = 0xC;
    g_wmap_land_effect_13_sequence_9_step += 1;
    wmap_land_effect_13_sequence_9_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_9_step_03, g_wmap_land_effect_13_sequence_9_step, g_wmap_land_effect_13_sequence_9_timer, 0x28,
                    wmap_land_effect_13_sequence_9_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_9_end, g_wmap_land_effect_13_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_13_run_sequence_10, D_800D5100, 0x6, g_wmap_land_effect_13_sequence_10_step, g_wmap_land_effect_13_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_10_reset, g_wmap_land_effect_13_sequence_10_step, g_wmap_land_effect_13_sequence_10_timer)

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void wmap_land_effect_13_sequence_10_step_01(void)
{
    u8 *base;

    g_wmap_actor_animations[9].data = g_wmap_animation_bank_2;
    base = (u8*)&g_wmap_sprite_actors[9];
    *(u8 *)(base + 0x6) = 0xF;
    *(s16 *)(base + 0x2) = 0;
    *(s16 *)(base + 0xE) = 0;
    *(s16 *)(base + 0x10) = -1;
    g_wmap_effect_fade_c = 0;
    g_wmap_land_effect_13_sequence_10_timer = 0x10;
    g_wmap_land_effect_13_sequence_10_step += 1;
    wmap_land_effect_13_sequence_10_step_02();
}

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void wmap_land_effect_13_sequence_10_step_02(void)
{
    u8* obj = (u8*)&g_wmap_sprite_actors[9];
    u16 pos = *(u16*)&g_wmap_effect_fade_c;

    *(s16*)&obj[0x24] = pos;
    *(s16*)&obj[0x22] = pos;
    wmap_step_actor_animation(obj, &g_wmap_actor_animations[9]);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 0x10, 0xB, 0);
    g_wmap_effect_fade_c += 0x20;
    if (g_wmap_effect_fade_c >= 0x82)
    {
        g_wmap_effect_fade_c = 0x81;
    }
    if (--g_wmap_land_effect_13_sequence_10_timer == 0)
    {
        g_wmap_land_effect_13_sequence_10_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_13_sequence_10_step_03, g_wmap_land_effect_13_sequence_10_step, g_wmap_land_effect_13_sequence_10_timer, 0x10,
                    wmap_land_effect_13_sequence_10_step_04)

/** @brief World-map step: build a sprite, decrement a shared budget, expire the timer. */
void wmap_land_effect_13_sequence_10_step_04(void)
{
    wmap_step_actor_animation(&g_wmap_sprite_actors[9], &g_wmap_actor_animations[9]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[9], g_wmap_focus_screen_position.packed, 0x10, 0xB, 0);
    *(s16 *)((u8*)&g_wmap_sprite_actors[9] + 0x24) = (u16)g_wmap_effect_fade_c;
    *(s16 *)((u8*)&g_wmap_sprite_actors[9] + 0x22) = (u16)g_wmap_effect_fade_c;
    g_wmap_effect_fade_c -= 0x10;
    if (g_wmap_effect_fade_c < 0)
    {
        g_wmap_effect_fade_c = 0;
    }
    if (--g_wmap_land_effect_13_sequence_10_timer == 0)
    {
        g_wmap_land_effect_13_sequence_10_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_10_end, g_wmap_land_effect_13_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_13_run_sequence_11, D_800D5118, 0x6, g_wmap_land_effect_13_sequence_11_step, g_wmap_land_effect_13_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_11_reset, g_wmap_land_effect_13_sequence_11_step, g_wmap_land_effect_13_sequence_11_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_13_sequence_11_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[10];

    g_wmap_actor_animations[10].data = g_wmap_animation_bank_2;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    g_wmap_effect_fade_d = 0;
    actor->resource_index = 0;
    g_wmap_land_effect_13_sequence_11_timer = 0x59;
    g_wmap_land_effect_13_sequence_11_step += 1;
    wmap_land_effect_13_sequence_11_step_02();
}

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void wmap_land_effect_13_sequence_11_step_02(void)
{
    u8* obj = (u8*)&g_wmap_sprite_actors[10];
    u16 pos = *(u16*)&g_wmap_effect_fade_d;

    *(s16*)&obj[0x24] = pos;
    *(s16*)&obj[0x22] = pos;
    wmap_step_actor_animation(obj, &g_wmap_actor_animations[10]);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 0x10, 0xB, 0);
    g_wmap_effect_fade_d += 0x8;
    if (g_wmap_effect_fade_d >= 0x81)
    {
        g_wmap_effect_fade_d = 0x80;
    }
    if (--g_wmap_land_effect_13_sequence_11_timer == 0)
    {
        g_wmap_land_effect_13_sequence_11_step += 1;
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
    actor->shade = (u16) g_wmap_effect_fade_d;
    actor->target_shade = (u16) g_wmap_effect_fade_d;
    if ((s32) g_wmap_effect_fade_d < 0)
    {
        g_wmap_effect_fade_d = 0;
    }
    remaining_ticks = g_wmap_land_effect_13_sequence_11_timer - 1;
    g_wmap_land_effect_13_sequence_11_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_13_sequence_11_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_13_sequence_11_end, g_wmap_land_effect_13_sequence_11_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_13_run_sequence_12, D_800D5130, 0x6, g_wmap_land_effect_13_sequence_12_step,
                               g_wmap_land_effect_13_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_13_sequence_12_reset, g_wmap_land_effect_13_sequence_12_step, g_wmap_land_effect_13_sequence_12_timer)

/** @brief Restore the default transform and advance the sequence. */
void wmap_land_effect_13_sequence_12_step_01(void)
{
    D_801B25D8 = 0;
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
