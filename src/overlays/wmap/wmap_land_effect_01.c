#include "wmap_land_effect_01.h"
#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_step_sequence.h"

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

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

void wmap_land_effect_01_sequence_5_step_02(void);
void wmap_land_effect_01_sequence_8_step_02(void);
s32 wmap_land_effect_01_run_sequence_2(s32 arg0);
s32 wmap_land_effect_01_run_sequence_1(s32 arg0);
s32 wmap_land_effect_01_run_sequence_3(s32 arg0);
s32 wmap_land_effect_01_run_sequence_4(s32 arg0);
s32 wmap_land_effect_01_run_sequence_5(s32 arg0);
s32 wmap_land_effect_01_run_sequence_7(s32 arg0);
s32 wmap_land_effect_01_run_sequence_8(s32 arg0);
s32 wmap_land_effect_01_run_sequence_6(s32 arg0);
void wmap_land_effect_01_sequence_2_step_02(void);
void wmap_land_effect_01_sequence_2_step_04(void);
void wmap_land_effect_01_wait_idle_02(void);
void wmap_land_effect_01_step_03(void);
void wmap_land_effect_01_wait_idle_04(void);
void wmap_land_effect_01_end(void);
void wmap_land_effect_01_sequence_1_step_02(void);
void wmap_land_effect_01_sequence_5_step_04(void);
void wmap_land_effect_01_sequence_6_step_02(void);
void wmap_land_effect_01_sequence_7_step_02(void);
void wmap_land_effect_01_sequence_7_step_04(void);
void wmap_land_effect_01_sequence_8_step_04(void);

extern s32 D_800D922C;
extern s32 D_801B0FD0;
extern s32 rand(void);
extern s32 D_8011D510;
extern s32 D_8011D530;
extern s32 g_wmap_land_effect_01_sequence_3_timer;
extern u8 D_800DCF18[];
extern VECTOR D_8011CF60;
extern s32 g_wmap_land_effect_01_sequence_4_timer;
extern s32 D_801B2470;
extern u8 D_8011F538[];
extern s32 g_wmap_land_effect_01_sequence_5_timer;
extern SVECTOR g_wmap_camera_rotation;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_80182DEC;
extern s32 g_wmap_land_effect_01_sequence_8_timer;
extern s32 g_wmap_land_effect_01_timeline_timer;
extern void (*D_800D4ED8[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_01_sequence_2_timer;
extern void (*D_800D4F38[])(void);
extern u8 *D_801399AC;
extern s16 D_800D933A;
extern s32 g_wmap_land_effect_01_timer;
extern void (*D_800D4F10[])(void);
extern void wmap_land_effect_01_end(void);
extern s32 g_wmap_land_effect_01_sequence_1_timer;
extern void (*D_800D4F28[])(void);
extern void (*D_800D4F50[])(void);
extern void *D_801399B4;
extern void (*D_800D4F60[])(void);
extern void (*D_800D4F70[])(void);
extern s32 g_wmap_land_effect_01_sequence_6_timer;
extern void (*D_800D4F88[])(void);
extern void *D_8013A17C;
extern s32 D_80127538;
extern u8 D_800DBE10[];
extern u8 D_8013A178[];
extern s32 g_wmap_land_effect_01_sequence_7_timer;
extern void (*D_800D4F98[])(void);
extern void *D_8013A184;
extern WmapAnimationSlot g_wmap_vehicle_animation;
extern u16 D_800DBE5E;
extern void (*D_800D4FB0[])(void);

extern u32 g_wmap_land_effect_01_sequence_3_step;
extern u32 g_wmap_land_effect_01_sequence_4_step;
extern WmapMotion* D_801B2560;
extern s32 D_80139980;
extern u32 g_wmap_land_effect_01_sequence_5_step;
extern u32 g_wmap_land_effect_01_sequence_8_step;
extern u8 D_80125538[];
extern u32 g_wmap_land_effect_01_timeline_step;
extern u32 g_wmap_land_effect_01_sequence_2_step;
extern u8 D_8011D538[];
extern u32 g_wmap_land_effect_01_step;
extern u32 g_wmap_land_effect_01_sequence_1_step;
extern u8 D_800DB578[];
extern WmapAnimationSlot D_80139FE8[];
extern u32 g_wmap_land_effect_01_sequence_6_step;
extern u32 g_wmap_land_effect_01_sequence_7_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2478;
extern VECTOR D_80139870;

/** @brief Map scroll position (map units) and projection scale. */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 unknown_0c;
} WmapView;

extern WmapView g_wmap_view;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370[];
extern WmapSpriteActor g_wmap_vehicle_actor;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801AFC98[];

/**
 * @brief Project active particles and initialize the first available slot.
 * @param actors Actor configurations for the particle slots.
 * @param resources Animation resources corresponding to the actor slots.
 * @param count Number of slots to process.
 */
void func_80072644(WmapConfigA* actors, WmapResource* resources, s32 count)
{
    SVECTOR position;
    s32 screen_position;
    s32 active_count;
    s32 i;

    active_count = 0;
    for (i = 0; i < count; i++)
    {
        if (D_801AFBD0[i].state != 0)
        {
            position.vx = ((D_801AFBD0[i].z >> 3) * (ccos(D_801AFBD0[i].angle) >> 6)) >> 12;
            position.vy = ((D_801AFBD0[i].z >> 3) * (csin(D_801AFBD0[i].angle) >> 6)) >> 12;
            position.vz = D_801AFBD0[i].field_0E;
            gte_ldv0(&position);
            gte_rtps();
            D_801AFBD0[i].field_0E += D_801AFBD0[i].x;
            D_801AFBD0[i].z += 1000;
            gte_stsxy(&screen_position);
            wmap_step_actor_animation(&actors[i], &resources[i]);
            wmap_draw_actor_sprite(&actors[i], screen_position, 13, 10, 0);
            D_801AFBD0[i].scale--;
            if (D_801AFBD0[i].scale == 0)
            {
                D_801AFBD0[i].state = 0;
            }
            active_count++;
        }
    }
    D_800D922C = active_count;
    for (i = 0; i < count; i++)
    {
        if (D_801AFBD0[i].state == 0)
        {
            if (D_801B0FD0 >= active_count)
            {
                actors[i].field_06 = 15;
                actors[i].field_10 = -1;
                actors[i].field_02 = 0;
                actors[i].field_0E = 1;
                actors[i].field_22 = 129;
                actors[i].field_24 = 129;
                D_801AFBD0[i].state = 1;
                D_801AFBD0[i].angle = rand();
                D_801AFBD0[i].z = 10000;
                D_801AFBD0[i].x = ((rand() * 5) >> 15) + 1;
                D_801AFBD0[i].scale = ((rand() * 2) >> 15) + 24;
                D_801AFBD0[i].field_0E = 0;
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
    u8 *actor = (u8*)&D_800D9344;

    position.vz = 0;
    position.vx = (((D_8011D510 - 1) * 160 -
                   g_wmap_view.x * 0x14000 / g_wmap_view.projection_scale) * 0x6000) /
                  g_wmap_view.projection_scale;
    position.vy = (((D_8011D530 - 1) * 160 -
                   g_wmap_view.y * 0x14000 / g_wmap_view.projection_scale) * 0x6000) /
                  g_wmap_view.projection_scale;
    gte_ldv0(&position);
    gte_rtps();
    wmap_step_actor_animation(actor, &D_801399B0);
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

    depth = D_801B2478.vz - 2400;
    D_801B2478.vz = depth;
    if (depth < 10)
    {
        D_801B2478.vz = 10;
    }
    PushMatrix();
    RotMatrix(&D_801B24A8, &matrix);
    TransMatrix(&matrix, &D_8011CF60);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    if (D_801B2470 != 0)
    {
        wmap_draw_model_default(D_800DCF18, 0, 4, -1, -1, 1, D_801B2470);
    }
    PopMatrix();
    intensity = D_801B2470 - 9;
    D_801B2470 = intensity;
    if (intensity < 0)
    {
        D_801B2470 = 0;
    }
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
    WmapConfigA* actor;

    D_801B2560 = D_801AFC98;
    D_801B24A0 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_801B2470 = 128;
    for (; i < 20; i++)
    {
        actor = &D_800D9370[i];
        D_801B2560[i].state = 0;
        D_80139988[i + 6].data = D_8011F538 + ((i % 3) << 13);
        actor->field_02 = 0;
        actor->field_06 = 15;
        actor->field_0E = rand() & 1;
        actor->field_10 = -1;
        actor->field_22 = 128;
        actor->field_24 = 128;
        D_801B2560[i].state = 1;
        D_801B2560[i].x = 30000;
        D_801B2560[i].angle = i * 204;
        D_801B2560[i].z = 0;
        if (i & 1)
        {
            D_801B2560[i].field_0E = 30;
        }
        else
        {
            D_801B2560[i].field_0E = 0;
        }
    }
    D_80139980 = 128;
    g_wmap_land_effect_01_sequence_5_timer = 36;
    g_wmap_land_effect_01_sequence_5_step++;
    wmap_land_effect_01_sequence_5_step_02();
}

/** @brief Compose the effect transform and project its twenty actors. */
void func_80072D30(void)
{
    MATRIX base_matrix;
    MATRIX effect_matrix;
    SVECTOR position;
    s32 screen_position;
    s32 i;
    s32 value;
    WmapMotion *motion;
    WmapConfigA *actor;
    WmapResource *resource;

    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base_matrix);
    TransMatrix(&base_matrix, &D_80139870);
    SetRotMatrix(&base_matrix);
    SetTransMatrix(&base_matrix);
    RotMatrix(&D_801B24A0, &effect_matrix);
    TransMatrix(&effect_matrix, &D_8011CF60);
    CompMatrix(&base_matrix, &effect_matrix, &effect_matrix);
    SetRotMatrix(&effect_matrix);
    SetTransMatrix(&effect_matrix);
    D_801B24A0.vz -= 80;
    for (i = 0; i < 20; i++)
    {
        actor = &D_800D9370[i];
        position.vx = ((D_801B2560[i].z >> 6) * (ccos(D_801B2560[i].angle) >> 6)) >> 12;
        position.vy = ((D_801B2560[i].z >> 6) * (csin(D_801B2560[i].angle) >> 6)) >> 12;
        motion = &D_801B2560[i];
        resource = &D_801399B8[i];
        position.vz = motion->field_0E;
        gte_ldv0(&position);
        gte_rtps();
        value = motion->z + motion->x;
        motion->z = value;
        if (value > 900000)
        {
            motion->z = 900000;
        }
        actor->field_22 = D_80139980;
        actor->field_24 = D_80139980;
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

    D_801B0FD0 = 24;
    D_80182DEC = 256;
    D_80139234 = 12;
    D_8013923C = 10;
    D_80139240 = 24;
    D_8013924C = 2;
    D_80139250 = 2;
    D_80139260 = 1500;
    D_80139264 = 128;
    D_80139268 = 13;
    D_8013926C = 1;
    D_80139284 = 0;
    for (i = 0; i < 24; i++)
    {
        D_801AFBD0[i + D_80139264].state = 0;
        D_80139988[i + 204].data = D_80125538;
    }
    g_wmap_land_effect_01_sequence_8_timer = 32;
    g_wmap_land_effect_01_sequence_8_step++;
    wmap_land_effect_01_sequence_8_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_01_run_timeline, D_800D4ED8, 0xE, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_01_timeline_reset, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

/** @brief World-map step handler: set up the actor, register callbacks, advance step. */
void wmap_land_effect_01_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x404040);
    wmap_start_sequence(wmap_land_effect_01_run_sequence_2);
    wmap_play_sound(0xF, 0x80);
    g_wmap_land_effect_01_timeline_timer = 8;
    g_wmap_land_effect_01_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_02, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

/** @brief Set the sequence flag, register two callbacks, and begin a 48-tick delay. */
void wmap_land_effect_01_timeline_step_03(void)
{
    D_801ADAE0 = 1;
    wmap_start_sequence(&wmap_land_effect_01_run_sequence_1);
    wmap_start_sequence(&wmap_land_effect_01_run_sequence_3);
    g_wmap_land_effect_01_timeline_timer = 0x30;
    g_wmap_land_effect_01_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_04, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_01_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_01_run_sequence_4);
    g_wmap_land_effect_01_timeline_timer = 0x2;
    g_wmap_land_effect_01_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_06, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_01_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_01_run_sequence_5);
    g_wmap_land_effect_01_timeline_timer = 0x2;
    g_wmap_land_effect_01_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_08, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_01_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_01_run_sequence_7);
    wmap_start_sequence(wmap_land_effect_01_run_sequence_8);
    wmap_start_sequence(wmap_land_effect_01_run_sequence_6);
    g_wmap_land_effect_01_timeline_timer = 0x60;
    g_wmap_land_effect_01_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_10, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

/** @brief Play sound 19, start a 54-tick delay, and advance the state. */
void wmap_land_effect_01_timeline_step_11(void)
{
    wmap_play_sound(0x13, 0x80);
    g_wmap_land_effect_01_timeline_timer = 54;
    g_wmap_land_effect_01_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_01_timeline_wait_12, g_wmap_land_effect_01_timeline_step, g_wmap_land_effect_01_timeline_timer)

/** @brief Update the selected world-map cell value, clear the gate flag, and advance the sequence. */
void wmap_land_effect_01_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    g_wmap_land_effect_01_timeline_step += 1;
}

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_2, D_800D4F38, 0x6, g_wmap_land_effect_01_sequence_2_step,
                               g_wmap_land_effect_01_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_2_reset, g_wmap_land_effect_01_sequence_2_step, g_wmap_land_effect_01_sequence_2_timer)

/** @brief Initialize the actor configuration and begin an eight-tick sequence step. */
void wmap_land_effect_01_sequence_2_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9268[4].scale_index = 0xF;
    D_800D9268[4].sequence = 1;
    D_800D9268[4].previous_sequence = -1;
    D_800D9268[4].shade = 1;
    D_800D9268[4].resource_index = 0;
    D_800D9268[4].target_shade = 0xF1;
    g_wmap_land_effect_01_sequence_2_timer = 8;
    g_wmap_land_effect_01_sequence_2_step += 1;
    wmap_land_effect_01_sequence_2_step_02();
}

/**
 * @brief World-map step handler: draw the sprite, bump its animation field, then
 *        countdown-advance the step.
 */
void wmap_land_effect_01_sequence_2_step_02(void)
{
    u8* obj;
    WmapConfigA* actors;

    obj = (u8*)&D_800D9318;
    wmap_step_actor_animation(obj, &D_801399A8);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 0xC, 0xA, 0);
    /* This sprite occupies the fifth actor slot. */
    actors = (WmapConfigA*)obj - 4;
    actors[4].field_24 += 2;
    if (--g_wmap_land_effect_01_sequence_2_timer == 0)
    {
        g_wmap_land_effect_01_sequence_2_step += 1;
    }
}

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void wmap_land_effect_01_sequence_2_step_03(void)
{
    D_800D933A = 1;
    g_wmap_land_effect_01_sequence_2_timer = 0x10;
    g_wmap_land_effect_01_sequence_2_step += 1;
    wmap_land_effect_01_sequence_2_step_04();
}

/**
 * @brief World-map step handler: draw the sprite, bump its animation field, then
 *        countdown-advance the step.
 */
void wmap_land_effect_01_sequence_2_step_04(void)
{
    u8* obj;
    WmapConfigA* actors;

    obj = (u8*)&D_800D9318;
    wmap_step_actor_animation(obj, &D_801399A8);
    wmap_draw_actor_sprite(obj, g_wmap_focus_screen_position.packed, 0xC, 0xA, 0);
    /* This sprite occupies the fifth actor slot. */
    actors = (WmapConfigA*)obj - 4;
    actors[4].field_24 += 2;
    if (--g_wmap_land_effect_01_sequence_2_timer == 0)
    {
        g_wmap_land_effect_01_sequence_2_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_2_end, g_wmap_land_effect_01_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_01_run, D_800D4F10, 0x6, g_wmap_land_effect_01_step, g_wmap_land_effect_01_timer)

WMAP_STEP_RESET(wmap_land_effect_01_reset, g_wmap_land_effect_01_step, g_wmap_land_effect_01_timer)

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void wmap_land_effect_01_step_01(void)
{
    g_wmap_input_locked = 1;
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_01_step += 1;
    wmap_land_effect_01_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_01_wait_idle_02, g_wmap_land_effect_01_step, wmap_land_effect_01_step_03)

/** @brief World-map step: register the next draw callback and advance to the next handler. */
void wmap_land_effect_01_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_01_run_timeline);
    g_wmap_land_effect_01_step += 1;
    wmap_land_effect_01_wait_idle_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_01_wait_idle_04, g_wmap_land_effect_01_step, wmap_land_effect_01_end)

WMAP_STEP_ADVANCE(wmap_land_effect_01_end, g_wmap_land_effect_01_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_1, D_800D4F28, 0x4, g_wmap_land_effect_01_sequence_1_step,
                               g_wmap_land_effect_01_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_1_reset, g_wmap_land_effect_01_sequence_1_step, g_wmap_land_effect_01_sequence_1_timer)

/** @brief World-map step: reset a run of slot tables then advance the sub-counter. */
void wmap_land_effect_01_sequence_1_step_01(void)
{
    s32 i;

    D_801B0FD0 = 0x18;
    for (i = 0; i < 0x18; i++)
    {
        D_801AFBD0[i].state = 0;
        D_80139988[i + 0xCC].data = &D_80125538;
    }
    g_wmap_land_effect_01_sequence_1_timer = 0x30;
    g_wmap_land_effect_01_sequence_1_step += 1;
    wmap_land_effect_01_sequence_1_step_02();
}

/** @brief World-map step handler: run the sub-step, then advance after the timer. */
void wmap_land_effect_01_sequence_1_step_02(void)
{
    func_80072644(D_800DB578, D_80139FE8, 0x18);
    if (--g_wmap_land_effect_01_sequence_1_timer == 0)
    {
        g_wmap_land_effect_01_sequence_1_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_1_end, g_wmap_land_effect_01_sequence_1_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_3, D_800D4F50, 0x4, g_wmap_land_effect_01_sequence_3_step,
                               g_wmap_land_effect_01_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_3_reset, g_wmap_land_effect_01_sequence_3_step, g_wmap_land_effect_01_sequence_3_timer)

/** @brief World-map step handler: init substate block and advance. */
void wmap_land_effect_01_sequence_3_step_01(void)
{
    u8 *base = &D_800D9268;

    D_801399B4 = &D_8011D538;
    *(u8 *)(base + 0xE2) = 0xF;
    *(s16 *)(base + 0xEC) = -1;
    *(s16 *)(base + 0xDE) = 0;
    *(s16 *)(base + 0xEA) = 0;
    *(s16 *)(base + 0xFE) = 0x80;
    *(s16 *)(base + 0x100) = 0x80;
    g_wmap_land_effect_01_sequence_3_timer = 0x32;
    g_wmap_land_effect_01_sequence_3_step += 1;
    wmap_land_effect_01_sequence_3_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_3_end, g_wmap_land_effect_01_sequence_3_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_4, D_800D4F60, 0x4, g_wmap_land_effect_01_sequence_4_step,
                               g_wmap_land_effect_01_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_4_reset, g_wmap_land_effect_01_sequence_4_step, g_wmap_land_effect_01_sequence_4_timer)

/** @brief Restore the default transform and advance the sequence. */
void wmap_land_effect_01_sequence_4_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_801B2470 = 0x80;
    g_wmap_land_effect_01_sequence_4_timer = 0x20;
    g_wmap_land_effect_01_sequence_4_step++;
    wmap_land_effect_01_sequence_4_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_4_end, g_wmap_land_effect_01_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_01_run_sequence_5, D_800D4F70, 0x6, g_wmap_land_effect_01_sequence_5_step, g_wmap_land_effect_01_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_5_reset, g_wmap_land_effect_01_sequence_5_step, g_wmap_land_effect_01_sequence_5_timer)

/** @brief Update the sequence effect and advance when its countdown reaches zero. */
void wmap_land_effect_01_sequence_5_step_02(void)
{
    s32 remaining_ticks;

    func_80072D30();
    remaining_ticks = g_wmap_land_effect_01_sequence_5_timer - 1;
    g_wmap_land_effect_01_sequence_5_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_01_sequence_5_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_01_sequence_5_step_03(void)
{
    g_wmap_land_effect_01_sequence_5_timer = 0x20;
    g_wmap_land_effect_01_sequence_5_step += 1;
    wmap_land_effect_01_sequence_5_step_04();
}

/** @brief Reduce the effect value toward zero, update it, and advance when the countdown expires. */
void wmap_land_effect_01_sequence_5_step_04(void)
{
    s32 value;
    s32 remaining_ticks;

    value = D_80139980 - 4;
    D_80139980 = value;
    if (value < 0)
    {
        D_80139980 = 0;
    }
    func_80072D30();
    remaining_ticks = g_wmap_land_effect_01_sequence_5_timer - 1;
    g_wmap_land_effect_01_sequence_5_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_01_sequence_5_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_5_end, g_wmap_land_effect_01_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_01_run_sequence_6, D_800D4F88, 0x4, g_wmap_land_effect_01_sequence_6_step, g_wmap_land_effect_01_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_6_reset, g_wmap_land_effect_01_sequence_6_step, g_wmap_land_effect_01_sequence_6_timer)

/** @brief World-map step handler: init substate block and advance. */
void wmap_land_effect_01_sequence_6_step_01(void)
{
    u8 *base = &D_800D9268;

    D_8013A17C = &D_80127538;
    *(u8 *)(base + 0x2BAE) = 0xF;
    *(s16 *)(base + 0x2BB8) = -1;
    *(s16 *)(base + 0x2BAA) = 0;
    *(s16 *)(base + 0x2BB6) = 0;
    *(s16 *)(base + 0x2BCA) = 0x80;
    *(s16 *)(base + 0x2BCC) = 0;
    g_wmap_land_effect_01_sequence_6_timer = 0x98;
    g_wmap_land_effect_01_sequence_6_step += 1;
    wmap_land_effect_01_sequence_6_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_01_sequence_6_step_02(void)
{
    wmap_step_actor_animation(D_800DBE10, D_8013A178);
    wmap_draw_actor_sprite(D_800DBE10, g_wmap_focus_screen_position.packed, 0xE, 0xA, 0);
    if (--g_wmap_land_effect_01_sequence_6_timer == 0)
    {
        g_wmap_land_effect_01_sequence_6_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_6_end, g_wmap_land_effect_01_sequence_6_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_7, D_800D4F98, 0x6, g_wmap_land_effect_01_sequence_7_step,
                               g_wmap_land_effect_01_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_7_reset, g_wmap_land_effect_01_sequence_7_step, g_wmap_land_effect_01_sequence_7_timer)

/** @brief World-map step handler: init substate block and advance. */
void wmap_land_effect_01_sequence_7_step_01(void)
{
    u8 *base = &D_800D9268;

    D_8013A184 = &D_80125538;
    *(u8 *)(base + 0x2BDA) = 0xF;
    *(s16 *)(base + 0x2BE4) = -1;
    *(s16 *)(base + 0x2BD6) = 0;
    *(s16 *)(base + 0x2BE2) = 0;
    *(s16 *)(base + 0x2BF6) = 0x81;
    *(s16 *)(base + 0x2BF8) = 0x81;
    g_wmap_land_effect_01_sequence_7_timer = 0x40;
    g_wmap_land_effect_01_sequence_7_step += 1;
    wmap_land_effect_01_sequence_7_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_01_sequence_7_step_02(void)
{
    wmap_step_actor_animation(&g_wmap_vehicle_actor, &g_wmap_vehicle_animation);
    wmap_draw_actor_sprite(&g_wmap_vehicle_actor, g_wmap_focus_screen_position.packed, 0xD, 0xA, 0);
    if (--g_wmap_land_effect_01_sequence_7_timer == 0)
    {
        g_wmap_land_effect_01_sequence_7_step += 1;
    }
}

/**
 * @brief Reset a world-map animation flag and schedule the next step.
 */
void wmap_land_effect_01_sequence_7_step_03(void)
{
    D_800DBE5E = 0;
    g_wmap_land_effect_01_sequence_7_timer = 0x10;
    g_wmap_land_effect_01_sequence_7_step += 1;
    wmap_land_effect_01_sequence_7_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_01_sequence_7_step_04(void)
{
    wmap_step_actor_animation(&g_wmap_vehicle_actor, &g_wmap_vehicle_animation);
    wmap_draw_actor_sprite(&g_wmap_vehicle_actor, g_wmap_focus_screen_position.packed, 0xD, 0xA, 0);
    if (--g_wmap_land_effect_01_sequence_7_timer == 0)
    {
        g_wmap_land_effect_01_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_7_end, g_wmap_land_effect_01_sequence_7_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_01_run_sequence_8, D_800D4FB0, 0x6, g_wmap_land_effect_01_sequence_8_step,
                               g_wmap_land_effect_01_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_01_sequence_8_reset, g_wmap_land_effect_01_sequence_8_step, g_wmap_land_effect_01_sequence_8_timer)

/** @brief Draw the sequence effect and advance when the countdown expires. */
void wmap_land_effect_01_sequence_8_step_02(void)
{
    s32 value;

    func_8006CFE4((s32)D_800DB578, (s32)D_80139FE8, 0x18, 0x81, 0x81, 0x10);
    value = g_wmap_land_effect_01_sequence_8_timer - 1;
    g_wmap_land_effect_01_sequence_8_timer = value;
    if (value == 0)
    {
        g_wmap_land_effect_01_sequence_8_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_01_sequence_8_step_03(void)
{
    g_wmap_land_effect_01_sequence_8_timer = 0x20;
    g_wmap_land_effect_01_sequence_8_step += 1;
    wmap_land_effect_01_sequence_8_step_04();
}

/** @brief Draw the effect, reduce its value toward zero, and update the countdown. */
void wmap_land_effect_01_sequence_8_step_04(void)
{
    s32 value;
    s32 remaining_ticks;

    func_8006CFE4((s32)D_800DB578, (s32)D_80139FE8, 0x18, 0x81, 0x81, 0x10);
    value = D_80182DEC - 0xA;
    D_80182DEC = value;
    if (value < 0)
    {
        D_80182DEC = 0;
    }
    remaining_ticks = g_wmap_land_effect_01_sequence_8_timer - 1;
    g_wmap_land_effect_01_sequence_8_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_01_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_01_sequence_8_end, g_wmap_land_effect_01_sequence_8_step)
