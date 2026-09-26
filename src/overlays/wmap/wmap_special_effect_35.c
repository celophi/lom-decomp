#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_special_effect_35.h"
#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"

/** @brief Step counts of the effect's sequences. */
#define WMAP_EFFECT35_MAIN_STEPS 4
#define WMAP_EFFECT35_TIMELINE_STEPS 38
#define WMAP_EFFECT35_DROP_STEPS 4
#define WMAP_EFFECT35_SPRITE_STEPS 6
#define WMAP_EFFECT35_SPIN_STEPS 6
#define WMAP_EFFECT35_ORBITER_STEPS 4
#define WMAP_EFFECT35_EMITTER_STEPS 6

/** @brief Land the map scrolls to, the effect sound and its resource file. */
#define WMAP_EFFECT35_LAND 31
#define WMAP_EFFECT35_SOUND 58
#define WMAP_EFFECT35_RESOURCE 35
#define WMAP_EFFECT35_TINT 0x801530

/** @brief Particle slots of the two emitters (emitter B pre-spawns its first ten). */
#define WMAP_EFFECT35_EMITTER_A_FIRST 20
#define WMAP_EFFECT35_EMITTER_A_END 50
#define WMAP_EFFECT35_EMITTER_B_FIRST 120
#define WMAP_EFFECT35_EMITTER_B_END 220
#define WMAP_EFFECT35_EMITTER_B_PRESPAWN_END 130

/** @brief Particles stop moving inward at this radius and are dropped once faded to this shade. */
#define WMAP_EFFECT35_INNER_RADIUS 0x579
#define WMAP_EFFECT35_FADED_SHADE 5

/** @brief Sprite textures and ordering-table depth of the particles. */
#define WMAP_EFFECT35_TEXTURE_SPARK 22
#define WMAP_EFFECT35_TEXTURE_GLOW 25
#define WMAP_EFFECT35_PARTICLE_OT 8

/** @brief Dropping models: start height above the camera, fall per frame, lowest height. */
#define WMAP_EFFECT35_DROP_START_Z 45000
#define WMAP_EFFECT35_DROP_SPEED 3500
#define WMAP_EFFECT35_DROP_MIN_Z 10000

/** @brief Size of one effect animation bank in D_8011D538. */
#define WMAP_EFFECT_BANK_SIZE 0x2000

/** @brief Animation resource slot of a sprite actor. */
typedef struct
{
    s32 unknown_00;
    u8* data;
} WmapAnimationSlot;

/** @brief Orbit particle motion: angle and radius around the screen center, inward speed. */
typedef struct
{
    s16 active;
    s16 angle;
    s32 speed;
    s32 radius;
    s16 delay;
    s16 height;
    u8 unknown_10[4];
} WmapEffectMotion;

/** @brief Particle emitter: a range of particle slots and the parameters of new particles. */
typedef struct
{
    s32 first;
    s32 end;
    s32 shade_step;
    s32 target_shade;
    s32 shade;
    s32 radius;
    s32 speed_min;
    s32 speed_range;
    s32 delay;
    s32 spawning;
} WmapEffectEmitter;


/** @brief Map scroll position and projection scale (see wmap_view_effects.c). */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 unknown_0c;
} WmapView;

/** @brief Backdrop color: three channels. */
typedef struct
{
    u8 r;
    u8 g;
    u8 b;
} WmapColor3;

/** @brief Sequence step handler. */
typedef void (*WmapHandler)(void);

extern WmapSpriteActor D_800D9268[];
extern WmapAnimationSlot D_80139988[];
extern WmapEffectMotion D_801AFBD0[];
extern s32 D_800DBE70;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_800DCF18[];
extern WmapHandler g_wmap_effect35_drop_a_steps[WMAP_EFFECT35_DROP_STEPS];
extern WmapHandler g_wmap_effect35_drop_b_steps[WMAP_EFFECT35_DROP_STEPS];
extern WmapHandler g_wmap_effect35_sprite_a_steps[WMAP_EFFECT35_SPRITE_STEPS];
extern WmapHandler g_wmap_effect35_sprite_b_steps[WMAP_EFFECT35_SPRITE_STEPS];
extern WmapHandler g_wmap_effect35_sprite_c_steps[WMAP_EFFECT35_SPRITE_STEPS];
extern WmapHandler g_wmap_effect35_spin_a_steps[WMAP_EFFECT35_SPIN_STEPS];
extern WmapHandler g_wmap_effect35_spin_b_steps[WMAP_EFFECT35_SPIN_STEPS];
extern WmapHandler g_wmap_effect35_orbiter_1_steps[WMAP_EFFECT35_ORBITER_STEPS];
extern WmapHandler g_wmap_effect35_orbiter_2_steps[WMAP_EFFECT35_ORBITER_STEPS];
extern WmapHandler g_wmap_effect35_orbiter_3_steps[WMAP_EFFECT35_ORBITER_STEPS];
extern WmapHandler g_wmap_effect35_orbiter_4_steps[WMAP_EFFECT35_ORBITER_STEPS];
extern WmapHandler g_wmap_effect35_orbiter_5_steps[WMAP_EFFECT35_ORBITER_STEPS];
extern WmapHandler g_wmap_effect35_orbiter_6_steps[WMAP_EFFECT35_ORBITER_STEPS];
extern WmapHandler g_wmap_effect35_orbiter_7_steps[WMAP_EFFECT35_ORBITER_STEPS];
extern WmapHandler g_wmap_effect35_emitter_a_steps[WMAP_EFFECT35_EMITTER_STEPS];
extern WmapHandler g_wmap_effect35_emitter_b_steps[WMAP_EFFECT35_EMITTER_STEPS];
extern WmapHandler g_wmap_effect35_timeline_steps[WMAP_EFFECT35_TIMELINE_STEPS];
extern WmapHandler g_wmap_effect35_steps[WMAP_EFFECT35_MAIN_STEPS];
extern s32 g_wmap_focus_screen_position;
extern s32 D_8011D500;
extern u8* D_8011CF1C;
extern u8* D_8011CF24;
extern u8* D_8011CF28;
extern u8 D_8011D538[];
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern SVECTOR D_80139258;
extern s32 D_80139260;
extern WmapEffectEmitter* D_80139280;
extern s32 g_wmap_view_mode;
extern s32 g_wmap_view_scroll_mode;
extern WmapView g_wmap_view;
extern s32 D_80139978;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern SVECTOR D_8013B238;
extern SVECTOR D_8013B240;
extern s32 D_801ADAE0;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR g_wmap_camera_translation;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern u32 g_wmap_effect35_step;
extern s32 g_wmap_effect35_timer;
extern u32 g_wmap_effect35_timeline_step;
extern s32 g_wmap_effect35_timeline_timer;
extern u32 g_wmap_effect35_drop_a_step;
extern s32 g_wmap_effect35_drop_a_timer;
extern u32 g_wmap_effect35_drop_b_step;
extern s32 g_wmap_effect35_drop_b_timer;
extern u32 g_wmap_effect35_sprite_a_step;
extern s32 g_wmap_effect35_sprite_a_timer;
extern u32 g_wmap_effect35_sprite_b_step;
extern s32 g_wmap_effect35_sprite_b_timer;
extern u32 g_wmap_effect35_sprite_c_step;
extern s32 g_wmap_effect35_sprite_c_timer;
extern u32 g_wmap_effect35_spin_a_step;
extern s32 g_wmap_effect35_spin_a_timer;
extern u32 g_wmap_effect35_spin_b_step;
extern s32 g_wmap_effect35_spin_b_timer;
extern u32 g_wmap_effect35_orbiter_1_step;
extern s32 g_wmap_effect35_orbiter_1_timer;
extern u32 g_wmap_effect35_orbiter_2_step;
extern s32 g_wmap_effect35_orbiter_2_timer;
extern u32 g_wmap_effect35_orbiter_3_step;
extern s32 g_wmap_effect35_orbiter_3_timer;
extern u32 g_wmap_effect35_orbiter_4_step;
extern s32 g_wmap_effect35_orbiter_4_timer;
extern u32 g_wmap_effect35_orbiter_5_step;
extern s32 g_wmap_effect35_orbiter_5_timer;
extern u32 g_wmap_effect35_orbiter_6_step;
extern s32 g_wmap_effect35_orbiter_6_timer;
extern u32 g_wmap_effect35_orbiter_7_step;
extern s32 g_wmap_effect35_orbiter_7_timer;
extern u32 g_wmap_effect35_emitter_a_step;
extern s32 g_wmap_effect35_emitter_a_timer;
extern u32 g_wmap_effect35_emitter_b_step;
extern s32 g_wmap_effect35_emitter_b_timer;

static void wmap_effect35_update_emitter(WmapEffectEmitter* emitter);
void wmap_effect35_timeline_flash_backdrop(void);
void wmap_effect35_timeline_start_finale(void);
void wmap_effect35_drop_a_update(void);
void wmap_effect35_drop_b_update(void);
void wmap_effect35_spin_a_fade_in(void);
void wmap_effect35_spin_a_fade_out(void);
void wmap_effect35_spin_b_fade_in(void);
void wmap_effect35_spin_b_fade_out(void);
void wmap_effect35_orbiter_1_update(void);
void wmap_effect35_orbiter_2_update(void);
void wmap_effect35_orbiter_3_update(void);
void wmap_effect35_orbiter_4_update(void);
void wmap_effect35_orbiter_5_update(void);
void wmap_effect35_orbiter_6_update(void);
void wmap_effect35_orbiter_7_update(void);
void wmap_effect35_emitter_a_start(void);
void wmap_effect35_emitter_b_start(void);
s32 wmap_effect35_run(s32 reset);
void wmap_effect35_reset(void);
void wmap_effect35_load(void);
void wmap_effect35_wait_timeline(void);
void wmap_effect35_finish(void);
static s32 wmap_effect35_run_timeline(s32 reset);
void wmap_effect35_timeline_reset(void);
void wmap_effect35_timeline_scroll_to_land(void);
void wmap_effect35_timeline_wait_scroll(void);
void wmap_effect35_timeline_start_sprite_a(void);
void wmap_effect35_timeline_wait_04(void);
void wmap_effect35_timeline_step_05(void);
void wmap_effect35_timeline_wait_06(void);
void wmap_effect35_timeline_start_drop_a(void);
void wmap_effect35_timeline_wait_08(void);
void wmap_effect35_timeline_start_orbiter_1(void);
void wmap_effect35_timeline_wait_10(void);
void wmap_effect35_timeline_start_orbiter_2(void);
void wmap_effect35_timeline_wait_12(void);
void wmap_effect35_timeline_start_orbiter_3(void);
void wmap_effect35_timeline_wait_14(void);
void wmap_effect35_timeline_start_orbiter_4(void);
void wmap_effect35_timeline_wait_16(void);
void wmap_effect35_timeline_start_orbiter_5(void);
void wmap_effect35_timeline_wait_18(void);
void wmap_effect35_timeline_start_orbiter_6(void);
void wmap_effect35_timeline_wait_20(void);
void wmap_effect35_timeline_start_orbiter_7(void);
void wmap_effect35_timeline_wait_22(void);
void wmap_effect35_timeline_start_emitter_a(void);
void wmap_effect35_timeline_wait_24(void);
void wmap_effect35_timeline_wait_26(void);
void wmap_effect35_timeline_start_drop_b(void);
void wmap_effect35_timeline_wait_28(void);
void wmap_effect35_timeline_start_emitter_b(void);
void wmap_effect35_timeline_wait_30(void);
void wmap_effect35_timeline_wait_32(void);
void wmap_effect35_timeline_start_spin_b(void);
void wmap_effect35_timeline_wait_34(void);
void wmap_effect35_timeline_step_35(void);
void wmap_effect35_timeline_wait_36(void);
void wmap_effect35_timeline_finish(void);
static s32 wmap_effect35_run_drop_a(s32 reset);
void wmap_effect35_drop_a_reset(void);
void wmap_effect35_drop_a_start(void);
void wmap_effect35_drop_a_end(void);
static s32 wmap_effect35_run_drop_b(s32 reset);
void wmap_effect35_drop_b_reset(void);
void wmap_effect35_drop_b_start(void);
void wmap_effect35_drop_b_end(void);
static s32 wmap_effect35_run_sprite_a(s32 reset);
void wmap_effect35_sprite_a_reset(void);
void wmap_effect35_sprite_a_start(void);
void wmap_effect35_sprite_a_draw(void);
void wmap_effect35_sprite_a_fade(void);
void wmap_effect35_sprite_a_draw_fading(void);
void wmap_effect35_sprite_a_end(void);
static s32 wmap_effect35_run_sprite_b(s32 reset);
void wmap_effect35_sprite_b_reset(void);
void wmap_effect35_sprite_b_start(void);
void wmap_effect35_sprite_b_draw(void);
void wmap_effect35_sprite_b_fade(void);
void wmap_effect35_sprite_b_draw_fading(void);
void wmap_effect35_sprite_b_end(void);
static s32 wmap_effect35_run_sprite_c(s32 reset);
void wmap_effect35_sprite_c_reset(void);
void wmap_effect35_sprite_c_start(void);
void wmap_effect35_sprite_c_draw(void);
void wmap_effect35_sprite_c_fade(void);
void wmap_effect35_sprite_c_draw_fading(void);
void wmap_effect35_sprite_c_end(void);
static s32 wmap_effect35_run_spin_a(s32 reset);
void wmap_effect35_spin_a_reset(void);
void wmap_effect35_spin_a_start(void);
void wmap_effect35_spin_a_start_fade_out(void);
void wmap_effect35_spin_a_end(void);
static s32 wmap_effect35_run_spin_b(s32 reset);
void wmap_effect35_spin_b_reset(void);
void wmap_effect35_spin_b_start(void);
void wmap_effect35_spin_b_start_fade_out(void);
void wmap_effect35_spin_b_end(void);
static s32 wmap_effect35_run_orbiter_1(s32 reset);
void wmap_effect35_orbiter_1_reset(void);
void wmap_effect35_orbiter_1_start(void);
void wmap_effect35_orbiter_1_end(void);
static s32 wmap_effect35_run_orbiter_2(s32 reset);
void wmap_effect35_orbiter_2_reset(void);
void wmap_effect35_orbiter_2_start(void);
void wmap_effect35_orbiter_2_end(void);
static s32 wmap_effect35_run_orbiter_3(s32 reset);
void wmap_effect35_orbiter_3_reset(void);
void wmap_effect35_orbiter_3_start(void);
void wmap_effect35_orbiter_3_end(void);
static s32 wmap_effect35_run_orbiter_4(s32 reset);
void wmap_effect35_orbiter_4_reset(void);
void wmap_effect35_orbiter_4_start(void);
void wmap_effect35_orbiter_4_end(void);
static s32 wmap_effect35_run_orbiter_5(s32 reset);
void wmap_effect35_orbiter_5_reset(void);
void wmap_effect35_orbiter_5_start(void);
void wmap_effect35_orbiter_5_end(void);
static s32 wmap_effect35_run_orbiter_6(s32 reset);
void wmap_effect35_orbiter_6_reset(void);
void wmap_effect35_orbiter_6_start(void);
void wmap_effect35_orbiter_6_end(void);
static s32 wmap_effect35_run_orbiter_7(s32 reset);
void wmap_effect35_orbiter_7_reset(void);
void wmap_effect35_orbiter_7_start(void);
void wmap_effect35_orbiter_7_end(void);
static s32 wmap_effect35_run_emitter_a(s32 reset);
void wmap_effect35_emitter_a_reset(void);
void wmap_effect35_emitter_a_update(void);
void wmap_effect35_emitter_a_stop(void);
void wmap_effect35_emitter_a_update_stopping(void);
void wmap_effect35_emitter_a_end(void);
static s32 wmap_effect35_run_emitter_b(s32 reset);
void wmap_effect35_emitter_b_reset(void);
void wmap_effect35_emitter_b_update(void);
void wmap_effect35_emitter_b_stop(void);
void wmap_effect35_emitter_b_update_stopping(void);
void wmap_effect35_emitter_b_end(void);

/**
 * @brief Move, animate and draw the live particles of an emitter, then spawn one new particle.
 * @param emitter Emitter whose slot range is updated; spawns only while @c spawning is set.
 */
static void wmap_effect35_update_emitter(WmapEffectEmitter* emitter)
{
    SVECTOR position;
    s32 screen_position;
    s32 index = emitter->first;
    WmapEffectMotion* motion;
    WmapEffectMotion* walker;
    WmapSpriteActor* actor;
    s32 speed;
    s32 speed_min;

    if (index < emitter->end)
    {
        u8* motion_base;

        motion_base = (u8*)D_801AFBD0;
        walker = (WmapEffectMotion*)((index * sizeof(WmapEffectMotion)) + (s32)motion_base);
        for (; index < emitter->end; index++)
        {
            /* Net-zero index pair and dead data_base store below: without them the reads stop going
               through a per-iteration copy of the walker and the registers change. */
            motion = (WmapEffectMotion*)((u32)walker + index);
            motion = (WmapEffectMotion*)((u32)motion - index);
            if (motion->active != 0)
            {
                u8* data_base;
                u8* resource_base;
                s32 resource_offset;
                data_base = (u8*)D_800D9268;
                actor = (WmapSpriteActor*)(index * sizeof(WmapSpriteActor) + (u32)data_base);
                position.vx = (motion->radius * (ccos(motion->angle) >> 5)) >> 0xC;
                position.vy = (motion->radius * (csin(motion->angle) >> 5)) >> 0xC;
                position.vz = motion->height;
                gte_ldv0(&position);
                gte_rtps();
                resource_offset = index * sizeof(WmapAnimationSlot);
                data_base = (u8*)D_80139988;
                resource_base = data_base;
                data_base = 0;
                wmap_step_actor_animation(actor, (void*)(resource_offset + (u32)resource_base));
                gte_stsxy(&screen_position);
                if (index < WMAP_EFFECT35_EMITTER_B_FIRST)
                {
                    wmap_draw_actor_sprite(actor, screen_position, WMAP_EFFECT35_TEXTURE_SPARK, WMAP_EFFECT35_PARTICLE_OT, 0);
                }
                else
                {
                    wmap_draw_actor_sprite(actor, screen_position, WMAP_EFFECT35_TEXTURE_GLOW, WMAP_EFFECT35_PARTICLE_OT, 0);
                }

                if (motion->delay != 0)
                {
                    motion->delay--;
                }
                else if (motion->radius >= WMAP_EFFECT35_INNER_RADIUS)
                {
                    motion->radius -= motion->speed;
                }
                else if (actor->shade >= WMAP_EFFECT35_FADED_SHADE)
                {
                    actor->target_shade = 0;
                }
                else
                {
                    walker->active = 0;
                }
            }
            walker++;
        }
    }
    if (emitter->spawning != 0)
    {
        index = emitter->first;
        if (index < emitter->end)
        {
            do
            {
                if (D_801AFBD0[index].active == 0)
                {
                    D_801AFBD0[index].active = 1;
                    D_800D9268[index].scale_index = 0xF;
                    D_800D9268[index].sequence = 2;
                    D_800D9268[index].unknown_02 = 0;
                    D_800D9268[index].previous_sequence = -1;
                    D_800D9268[index].shade_step = emitter->shade_step;
                    D_800D9268[index].target_shade = emitter->target_shade;
                    D_800D9268[index].shade = emitter->shade;
                    D_801AFBD0[index].radius = emitter->radius;
                    D_801AFBD0[index].angle = rand() & 0xFFF;
                    speed = rand() * emitter->speed_range;
                    speed_min = emitter->speed_min;
                    D_801AFBD0[index].height = 0;
                    D_801AFBD0[index].speed = (speed >> 0xF) + speed_min;
                    D_801AFBD0[index].delay = emitter->delay;
                    return;
                }
                index++;
            } while (index < emitter->end);
        }
    }
}

/**
 * @brief Timeline step 25: turn the backdrop colors bright.
 */
void wmap_effect35_timeline_flash_backdrop(void)
{
    g_wmap_backdrop_target_level = 0;
    D_80182D74.r = 0xC8;
    D_80182D74.g = 0xC8;
    D_80182D74.b = 0xC8;
    D_80182D80.r = 0xC8;
    D_80182D80.g = 0xC8;
    D_80182D80.b = 0xC8;
    D_80182D8C.r = 0xC8;
    D_80182D8C.g = 0xC8;
    D_80182D8C.b = 0xC8;
    D_80182D94.r = 0xC8;
    D_80182D94.g = 0xC8;
    D_80182D94.b = 0xC8;
    g_wmap_effect35_timeline_timer = 8;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 31: start sprites B and C and spinning model A, darken the backdrop.
 */
void wmap_effect35_timeline_start_finale(void)
{
    wmap_start_sequence(wmap_effect35_run_sprite_b);
    wmap_start_sequence(wmap_effect35_run_spin_a);
    wmap_start_sequence(wmap_effect35_run_sprite_c);
    g_wmap_backdrop_target_level = 0;
    D_80182D74.r = 0x40;
    D_80182D74.g = 0x10;
    D_80182D74.b = 0x20;
    D_80182D80.r = 0x40;
    D_80182D80.g = 0x10;
    D_80182D80.b = 0x20;
    D_80182D8C.r = 0x40;
    D_80182D8C.g = 0x10;
    D_80182D8C.b = 0x20;
    D_80182D94.r = 0x40;
    D_80182D94.g = 0x10;
    D_80182D94.b = 0x20;
    g_wmap_effect35_timeline_timer = 24;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Model drop A: update and draw until the step timer runs out.
 */
void wmap_effect35_drop_a_update(void)
{
    s32 value = D_801B2650.vz - WMAP_EFFECT35_DROP_SPEED;

    D_801B2650.vz = value;
    if (value < WMAP_EFFECT35_DROP_MIN_Z)
    {
        D_801B2650.vz = WMAP_EFFECT35_DROP_MIN_Z;
    }
    PushMatrix();
    wmap_set_map_rotation(&D_80139258);
    if (D_80182DE8 != 0)
    {
        wmap_draw_model(D_8011CF1C, 0, 4, 0x35, 0x7800, 1,
                      D_80182DE8, 0, 0, -1);
        value = D_80182DE8 - 4;
        D_80182DE8 = value;
        if (value < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    if (--g_wmap_effect35_drop_a_timer == 0)
    {
        g_wmap_effect35_drop_a_step++;
    }
}

/**
 * @brief Model drop B: update and draw until the step timer runs out.
 */
void wmap_effect35_drop_b_update(void)
{
    s32 value = D_801B2478.vz - WMAP_EFFECT35_DROP_SPEED;

    D_801B2478.vz = value;
    if (value < WMAP_EFFECT35_DROP_MIN_Z)
    {
        D_801B2478.vz = WMAP_EFFECT35_DROP_MIN_Z;
    }
    PushMatrix();
    wmap_set_map_rotation(&D_80139258);
    if (D_80182DEC != 0)
    {
        wmap_draw_model(D_800DCF18, 0, 4, 0x35, 0x7800, 1,
                      D_80182DEC, 0, 0, -1);
        value = D_80182DEC - 1;
        D_80182DEC = value;
        if (value < 0)
        {
            D_80182DEC = 0;
        }
    }
    PopMatrix();
    if (--g_wmap_effect35_drop_b_timer == 0)
    {
        g_wmap_effect35_drop_b_step++;
    }
}

/**
 * @brief Spinning model A: spin and draw the model while fading it in.
 */
void wmap_effect35_spin_a_fade_in(void)
{
    s32 value;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF24, (D_8013923C >> 4) & 3, 0xA, 0x35,
                  0x7800, 0x1001, D_80182DF0, 0, 0, -1);
    value = D_80182DF0 + 2;
    D_80182DF0 = value;
    if (value >= 0x82)
    {
        D_80182DF0 = 0x81;
    }
    D_8013923C += 8;
    D_8013B238.vz += 0xA;
    PopMatrix();
    if (--g_wmap_effect35_spin_a_timer == 0)
    {
        g_wmap_effect35_spin_a_step++;
    }
}

/**
 * @brief Spinning model A: spin and draw the model while fading it out.
 */
void wmap_effect35_spin_a_fade_out(void)
{
    s32 value;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF24, (D_8013923C >> 4) & 3, 0xA, 0x35,
                  0x7800, 0x1001, D_80182DF0, 0, 0, -1);
    value = D_80182DF0 - 4;
    D_80182DF0 = value;
    if (value < 0)
    {
        D_80182DF0 = 0;
    }
    D_8013923C += 8;
    D_8013B238.vz += 0xA;
    PopMatrix();
    if (--g_wmap_effect35_spin_a_timer == 0)
    {
        g_wmap_effect35_spin_a_step++;
    }
}

/**
 * @brief Spinning model B: spin and draw the model while fading it in.
 */
void wmap_effect35_spin_b_fade_in(void)
{
    s32 value;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
    wmap_draw_model(D_8011CF28, (D_80139260 >> 4) & 7, 0xA, 0x36,
                  0x7880, 0x1001, D_80182DF4, 0, 0, -1);
    value = D_80182DF4 + 2;
    D_80182DF4 = value;
    if (value >= 0x62)
    {
        D_80182DF4 = 0x61;
    }
    D_80139260 -= 0x10;
    D_8013B240.vz += 4;
    PopMatrix();
    if (--g_wmap_effect35_spin_b_timer == 0)
    {
        g_wmap_effect35_spin_b_step++;
    }
}

/**
 * @brief Spinning model B: spin and draw the model while fading it out.
 */
void wmap_effect35_spin_b_fade_out(void)
{
    s32 value;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
    wmap_draw_model(D_8011CF28, (D_80139260 >> 4) & 7, 0xA, 0x36,
                  0x7880, 0x1001, D_80182DF4, 0, 0, -1);
    value = D_80182DF4 - 8;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    D_80139260 -= 0x10;
    D_8013B240.vz += 4;
    PopMatrix();
    if (--g_wmap_effect35_spin_b_timer == 0)
    {
        g_wmap_effect35_spin_b_step++;
    }
}

/**
 * @brief Orbit particle 1: update and draw until the step timer runs out.
 */
void wmap_effect35_orbiter_1_update(void)
{
    SVECTOR position;
    s32 screen_position;
    WmapSpriteActor* actor = &D_800D9268[8];
    WmapEffectMotion* motion = &D_801AFBD0[8];
    WmapAnimationSlot* slots;

    position.vx = (motion->radius * (ccos(motion->angle) >> 5)) >> 0xC;
    position.vy = (motion->radius * (csin(motion->angle) >> 5)) >> 0xC;
    position.vz = motion->height;
    gte_ldv0(&position);
    gte_rtps();
    slots = D_80139988;
    wmap_step_actor_animation(actor, &slots[8]);
    gte_stsxy(&screen_position);
    wmap_draw_actor_sprite(actor, screen_position, WMAP_EFFECT35_TEXTURE_SPARK, WMAP_EFFECT35_PARTICLE_OT, 0);
    if (motion->delay != 0)
    {
        motion->delay--;
    }
    else if (motion->radius >= WMAP_EFFECT35_INNER_RADIUS)
    {
        motion->radius -= motion->speed;
    }
    else if (actor->shade >= WMAP_EFFECT35_FADED_SHADE)
    {
        actor->target_shade = 0;
    }
    else
    {
        motion->active = 0;
    }
    if (--g_wmap_effect35_orbiter_1_timer == 0)
    {
        g_wmap_effect35_orbiter_1_step++;
    }
}

/**
 * @brief Orbit particle 2: update and draw until the step timer runs out.
 */
void wmap_effect35_orbiter_2_update(void)
{
    SVECTOR position;
    s32 screen_position;
    WmapSpriteActor* actor = &D_800D9268[9];
    WmapEffectMotion* motion = &D_801AFBD0[9];
    WmapAnimationSlot* slots;

    position.vx = (motion->radius * (ccos(motion->angle) >> 5)) >> 0xC;
    position.vy = (motion->radius * (csin(motion->angle) >> 5)) >> 0xC;
    position.vz = motion->height;
    gte_ldv0(&position);
    gte_rtps();
    slots = D_80139988;
    wmap_step_actor_animation(actor, &slots[9]);
    gte_stsxy(&screen_position);
    wmap_draw_actor_sprite(actor, screen_position, WMAP_EFFECT35_TEXTURE_SPARK, WMAP_EFFECT35_PARTICLE_OT, 0);
    if (motion->delay != 0)
    {
        motion->delay--;
    }
    else if (motion->radius >= WMAP_EFFECT35_INNER_RADIUS)
    {
        motion->radius -= motion->speed;
    }
    else if (actor->shade >= WMAP_EFFECT35_FADED_SHADE)
    {
        actor->target_shade = 0;
    }
    else
    {
        motion->active = 0;
    }
    if (--g_wmap_effect35_orbiter_2_timer == 0)
    {
        g_wmap_effect35_orbiter_2_step++;
    }
}

/**
 * @brief Orbit particle 3: update and draw until the step timer runs out.
 */
void wmap_effect35_orbiter_3_update(void)
{
    SVECTOR position;
    s32 screen_position;
    WmapSpriteActor* actor = &D_800D9268[10];
    WmapEffectMotion* motion = &D_801AFBD0[10];
    WmapAnimationSlot* slots;

    position.vx = (motion->radius * (ccos(motion->angle) >> 5)) >> 0xC;
    position.vy = (motion->radius * (csin(motion->angle) >> 5)) >> 0xC;
    position.vz = motion->height;
    gte_ldv0(&position);
    gte_rtps();
    slots = D_80139988;
    wmap_step_actor_animation(actor, &slots[10]);
    gte_stsxy(&screen_position);
    wmap_draw_actor_sprite(actor, screen_position, WMAP_EFFECT35_TEXTURE_SPARK, WMAP_EFFECT35_PARTICLE_OT, 0);
    if (motion->delay != 0)
    {
        motion->delay--;
    }
    else if (motion->radius >= WMAP_EFFECT35_INNER_RADIUS)
    {
        motion->radius -= motion->speed;
    }
    else if (actor->shade >= WMAP_EFFECT35_FADED_SHADE)
    {
        actor->target_shade = 0;
    }
    else
    {
        motion->active = 0;
    }
    if (--g_wmap_effect35_orbiter_3_timer == 0)
    {
        g_wmap_effect35_orbiter_3_step++;
    }
}

/**
 * @brief Orbit particle 4: update and draw until the step timer runs out.
 */
void wmap_effect35_orbiter_4_update(void)
{
    SVECTOR position;
    s32 screen_position;
    WmapSpriteActor* actor = &D_800D9268[14];
    WmapEffectMotion* motion = &D_801AFBD0[14];
    WmapAnimationSlot* slots;

    position.vx = (motion->radius * (ccos(motion->angle) >> 5)) >> 0xC;
    position.vy = (motion->radius * (csin(motion->angle) >> 5)) >> 0xC;
    position.vz = motion->height;
    gte_ldv0(&position);
    gte_rtps();
    slots = D_80139988;
    wmap_step_actor_animation(actor, &slots[14]);
    gte_stsxy(&screen_position);
    wmap_draw_actor_sprite(actor, screen_position, WMAP_EFFECT35_TEXTURE_SPARK, WMAP_EFFECT35_PARTICLE_OT, 0);
    if (motion->delay != 0)
    {
        motion->delay--;
    }
    else if (motion->radius >= WMAP_EFFECT35_INNER_RADIUS)
    {
        motion->radius -= motion->speed;
    }
    else if (actor->shade >= WMAP_EFFECT35_FADED_SHADE)
    {
        actor->target_shade = 0;
    }
    else
    {
        motion->active = 0;
    }
    if (--g_wmap_effect35_orbiter_4_timer == 0)
    {
        g_wmap_effect35_orbiter_4_step++;
    }
}

/**
 * @brief Orbit particle 5: update and draw until the step timer runs out.
 */
void wmap_effect35_orbiter_5_update(void)
{
    SVECTOR position;
    s32 screen_position;
    WmapSpriteActor* actor = &D_800D9268[15];
    WmapEffectMotion* motion = &D_801AFBD0[15];
    WmapAnimationSlot* slots;

    position.vx = (motion->radius * (ccos(motion->angle) >> 5)) >> 0xC;
    position.vy = (motion->radius * (csin(motion->angle) >> 5)) >> 0xC;
    position.vz = motion->height;
    gte_ldv0(&position);
    gte_rtps();
    slots = D_80139988;
    wmap_step_actor_animation(actor, &slots[15]);
    gte_stsxy(&screen_position);
    wmap_draw_actor_sprite(actor, screen_position, WMAP_EFFECT35_TEXTURE_SPARK, WMAP_EFFECT35_PARTICLE_OT, 0);
    if (motion->delay != 0)
    {
        motion->delay--;
    }
    else if (motion->radius >= WMAP_EFFECT35_INNER_RADIUS)
    {
        motion->radius -= motion->speed;
    }
    else if (actor->shade >= WMAP_EFFECT35_FADED_SHADE)
    {
        actor->target_shade = 0;
    }
    else
    {
        motion->active = 0;
    }
    if (--g_wmap_effect35_orbiter_5_timer == 0)
    {
        g_wmap_effect35_orbiter_5_step++;
    }
}

/**
 * @brief Orbit particle 6: update and draw until the step timer runs out.
 */
void wmap_effect35_orbiter_6_update(void)
{
    SVECTOR position;
    s32 screen_position;
    WmapSpriteActor* actor = &D_800D9268[16];
    WmapEffectMotion* motion = &D_801AFBD0[16];
    WmapAnimationSlot* slots;

    position.vx = (motion->radius * (ccos(motion->angle) >> 5)) >> 0xC;
    position.vy = (motion->radius * (csin(motion->angle) >> 5)) >> 0xC;
    position.vz = motion->height;
    gte_ldv0(&position);
    gte_rtps();
    slots = D_80139988;
    wmap_step_actor_animation(actor, &slots[16]);
    gte_stsxy(&screen_position);
    wmap_draw_actor_sprite(actor, screen_position, WMAP_EFFECT35_TEXTURE_SPARK, WMAP_EFFECT35_PARTICLE_OT, 0);
    if (motion->delay != 0)
    {
        motion->delay--;
    }
    else if (motion->radius >= WMAP_EFFECT35_INNER_RADIUS)
    {
        motion->radius -= motion->speed;
    }
    else if (actor->shade >= WMAP_EFFECT35_FADED_SHADE)
    {
        actor->target_shade = 0;
    }
    else
    {
        motion->active = 0;
    }
    if (--g_wmap_effect35_orbiter_6_timer == 0)
    {
        g_wmap_effect35_orbiter_6_step++;
    }
}

/**
 * @brief Orbit particle 7: update and draw until the step timer runs out.
 */
void wmap_effect35_orbiter_7_update(void)
{
    SVECTOR position;
    s32 screen_position;
    WmapSpriteActor* actor = &D_800D9268[17];
    WmapEffectMotion* motion = &D_801AFBD0[17];
    WmapAnimationSlot* slots;

    position.vx = (motion->radius * (ccos(motion->angle) >> 5)) >> 0xC;
    position.vy = (motion->radius * (csin(motion->angle) >> 5)) >> 0xC;
    position.vz = motion->height;
    gte_ldv0(&position);
    gte_rtps();
    slots = D_80139988;
    wmap_step_actor_animation(actor, &slots[17]);
    gte_stsxy(&screen_position);
    wmap_draw_actor_sprite(actor, screen_position, WMAP_EFFECT35_TEXTURE_SPARK, WMAP_EFFECT35_PARTICLE_OT, 0);
    if (motion->delay != 0)
    {
        motion->delay--;
    }
    else if (motion->radius >= WMAP_EFFECT35_INNER_RADIUS)
    {
        motion->radius -= motion->speed;
    }
    else if (actor->shade >= WMAP_EFFECT35_FADED_SHADE)
    {
        actor->target_shade = 0;
    }
    else
    {
        motion->active = 0;
    }
    if (--g_wmap_effect35_orbiter_7_timer == 0)
    {
        g_wmap_effect35_orbiter_7_step++;
    }
}

/**
 * @brief Particle emitter A: set up and run the first update.
 */
void wmap_effect35_emitter_a_start(void)
{
    s32 i;
    WmapEffectEmitter* emitter;

    for (i = WMAP_EFFECT35_EMITTER_A_FIRST; i < WMAP_EFFECT35_EMITTER_A_END; i++)
    {
        D_80139988[i].data = D_8011D538;
        D_801AFBD0[i].active = 0;
    }
    /* A plain word store: it may alias D_80139280, so the pointer is reloaded below. */
    *(s32*)D_80139280 = WMAP_EFFECT35_EMITTER_A_FIRST;
    g_wmap_effect35_emitter_a_timer = 144;
    emitter = D_80139280;
    emitter->shade = 1;
    emitter->end = WMAP_EFFECT35_EMITTER_A_END;
    emitter->shade_step = 0x10;
    emitter->target_shade = 0x81;
    emitter->radius = 0x2EE0;
    emitter->speed_min = 100;
    emitter->speed_range = 100;
    emitter->delay = 2;
    emitter->spawning = 1;
    g_wmap_effect35_emitter_a_step++;
    wmap_effect35_emitter_a_update();
}

/**
 * @brief Particle emitter B: set up and run the first update.
 */
void wmap_effect35_emitter_b_start(void)
{
    s32 i;

    for (i = WMAP_EFFECT35_EMITTER_B_FIRST; i < WMAP_EFFECT35_EMITTER_B_END; i++)
    {
        D_80139988[i].data = &D_8011D538[WMAP_EFFECT_BANK_SIZE];
        D_801AFBD0[i].active = 0;
    }
    for (i = WMAP_EFFECT35_EMITTER_B_FIRST; i < WMAP_EFFECT35_EMITTER_B_PRESPAWN_END; i++)
    {
        D_800D9268[i].scale_index = 0xF;
        D_800D9268[i].sequence = 2;
        D_800D9268[i].previous_sequence = -1;
        D_800D9268[i].shade_step = 0x10;
        D_800D9268[i].target_shade = 0x81;
        D_800D9268[i].unknown_02 = 0;
        D_800D9268[i].shade = 1;
        D_801AFBD0[i].radius = 0x2710;
        D_801AFBD0[i].angle = rand() & 0xFFF;
        D_801AFBD0[i].speed = ((rand() * 0x32) >> 0xF) + 0x32;
        D_801AFBD0[i].height = 0;
        D_801AFBD0[i].delay = 0x26;
    }
    g_wmap_effect35_emitter_b_timer = 240;
    D_80139280[1].first = WMAP_EFFECT35_EMITTER_B_FIRST;
    D_80139280[1].end = WMAP_EFFECT35_EMITTER_B_END;
    D_80139280[1].shade_step = 0x10;
    D_80139280[1].target_shade = 0x81;
    D_80139280[1].speed_min = 0x32;
    D_80139280[1].speed_range = 0x32;
    D_80139280[1].shade = 1;
    D_80139280[1].radius = 0x2710;
    D_80139280[1].delay = 2;
    D_80139280[1].spawning = 1;
    g_wmap_effect35_emitter_b_step++;
    wmap_effect35_emitter_b_update();
}

/**
 * @brief Run the current step of special effect 35.
 * @param reset Nonzero restarts the effect instead of running a step.
 * @return 1 while the effect runs, 0 once it has finished.
 */
s32 wmap_effect35_run(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_step = 1;
        g_wmap_effect35_timer = 1;
        return 1;
    }
    if (g_wmap_effect35_step >= WMAP_EFFECT35_MAIN_STEPS)
    {
        return 0;
    }
    g_wmap_effect35_steps[g_wmap_effect35_step]();
    return 1;
}

/**
 * @brief Effect step 0: restart the effect.
 */
void wmap_effect35_reset(void)
{
    g_wmap_effect35_step = 1;
    g_wmap_effect35_timer = 1;
}

/**
 * @brief Effect step 1: load and upload the effect resources and start the timeline.
 */
void wmap_effect35_load(void)
{
    D_80139978 = 0x1F;
    func_800A89DC(WMAP_EFFECT35_RESOURCE);
    cdrom_wait_queue_empty();
    func_800651B4(&D_80182E40);
    func_800651B4(&D_8018B240);
    func_800651B4(&D_80193640);
    wmap_reset_focus_screen_position();
    wmap_start_sequence(wmap_effect35_run_timeline);
    D_8013B20C = 1;
    g_wmap_effect35_step++;
    wmap_effect35_wait_timeline();
}

/**
 * @brief Effect step 2: wait for the timeline to finish.
 */
void wmap_effect35_wait_timeline(void)
{
    if (D_8013B20C == 0)
    {
        g_wmap_effect35_step++;
        wmap_effect35_finish();
    }
}

/**
 * @brief Effect step 3: flag the arrival.
 */
void wmap_effect35_finish(void)
{
    D_8013B294 = 1;
    D_80139228 = 2;
    g_wmap_effect35_step++;
}

/**
 * @brief Run the current step of the timeline sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_timeline(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_timeline_step = 1;
        g_wmap_effect35_timeline_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_timeline_step >= WMAP_EFFECT35_TIMELINE_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_timeline_steps[g_wmap_effect35_timeline_step]();
    return 1;
}

/**
 * @brief Timeline: step 0: restart the sequence.
 */
void wmap_effect35_timeline_reset(void)
{
    g_wmap_effect35_timeline_step = 1;
    g_wmap_effect35_timeline_timer = 1;
}

/**
 * @brief Timeline step 1: scroll the map to the effect land.
 */
void wmap_effect35_timeline_scroll_to_land(void)
{
    D_8013B208 = 1;
    D_801ADAE0 = 1;
    wmap_find_land_cell(WMAP_EFFECT35_LAND, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 48) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 48) - g_wmap_view.y;
    g_wmap_effect35_timeline_step++;
    wmap_effect35_timeline_wait_scroll();
}

/**
 * @brief Timeline step 2: wait for the map scroll.
 */
void wmap_effect35_timeline_wait_scroll(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_effect35_timeline_step++;
        wmap_effect35_timeline_start_sprite_a();
    }
}

/**
 * @brief Timeline step 3: play the effect sound and start sprite A.
 */
void wmap_effect35_timeline_start_sprite_a(void)
{
    wmap_play_sound(WMAP_EFFECT35_SOUND, 0x80);
    wmap_start_sequence(wmap_effect35_run_sprite_a);
    g_wmap_effect35_timeline_timer = 20;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 4: wait for the step timer.
 */
void wmap_effect35_timeline_wait_04(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline step 5: set D_800DBE70.
 */
void wmap_effect35_timeline_step_05(void)
{
    D_800DBE70 = 1;
    g_wmap_effect35_timeline_timer = 45;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 6: wait for the step timer.
 */
void wmap_effect35_timeline_wait_06(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline step 7: start model drop A and tint the map.
 */
void wmap_effect35_timeline_start_drop_a(void)
{
    wmap_start_sequence(wmap_effect35_run_drop_a);
    D_800DBE70 = 0;
    g_wmap_backdrop_target_level = 3;
    wmap_start_map_tint(WMAP_EFFECT35_TINT);
    g_wmap_effect35_timeline_timer = 100;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 8: wait for the step timer.
 */
void wmap_effect35_timeline_wait_08(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline: start orbit particle 1.
 */
void wmap_effect35_timeline_start_orbiter_1(void)
{
    wmap_start_sequence(wmap_effect35_run_orbiter_1);
    g_wmap_effect35_timeline_timer = 100;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 10: wait for the step timer.
 */
void wmap_effect35_timeline_wait_10(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline: start orbit particle 2.
 */
void wmap_effect35_timeline_start_orbiter_2(void)
{
    wmap_start_sequence(wmap_effect35_run_orbiter_2);
    g_wmap_effect35_timeline_timer = 20;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 12: wait for the step timer.
 */
void wmap_effect35_timeline_wait_12(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline: start orbit particle 3.
 */
void wmap_effect35_timeline_start_orbiter_3(void)
{
    wmap_start_sequence(wmap_effect35_run_orbiter_3);
    g_wmap_effect35_timeline_timer = 15;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 14: wait for the step timer.
 */
void wmap_effect35_timeline_wait_14(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline: start orbit particle 4.
 */
void wmap_effect35_timeline_start_orbiter_4(void)
{
    wmap_start_sequence(wmap_effect35_run_orbiter_4);
    g_wmap_effect35_timeline_timer = 15;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 16: wait for the step timer.
 */
void wmap_effect35_timeline_wait_16(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline: start orbit particle 5.
 */
void wmap_effect35_timeline_start_orbiter_5(void)
{
    wmap_start_sequence(wmap_effect35_run_orbiter_5);
    g_wmap_effect35_timeline_timer = 10;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 18: wait for the step timer.
 */
void wmap_effect35_timeline_wait_18(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline: start orbit particle 6.
 */
void wmap_effect35_timeline_start_orbiter_6(void)
{
    wmap_start_sequence(wmap_effect35_run_orbiter_6);
    g_wmap_effect35_timeline_timer = 5;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 20: wait for the step timer.
 */
void wmap_effect35_timeline_wait_20(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline: start orbit particle 7.
 */
void wmap_effect35_timeline_start_orbiter_7(void)
{
    wmap_start_sequence(wmap_effect35_run_orbiter_7);
    g_wmap_backdrop_target_level = 0;
    g_wmap_effect35_timeline_timer = 8;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 22: wait for the step timer.
 */
void wmap_effect35_timeline_wait_22(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline: start particle emitter A.
 */
void wmap_effect35_timeline_start_emitter_a(void)
{
    wmap_start_sequence(wmap_effect35_run_emitter_a);
    g_wmap_effect35_timeline_timer = 144;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 24: wait for the step timer.
 */
void wmap_effect35_timeline_wait_24(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline step 26: wait for the step timer.
 */
void wmap_effect35_timeline_wait_26(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline step 27: start model drop B and hide the map view.
 */
void wmap_effect35_timeline_start_drop_b(void)
{
    wmap_start_sequence(wmap_effect35_run_drop_b);
    D_80139244 = 1;
    D_80139978 = -1;
    g_wmap_view_mode = -1;
    g_wmap_effect35_timeline_timer = 2;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 28: wait for the step timer.
 */
void wmap_effect35_timeline_wait_28(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline: start particle emitter B.
 */
void wmap_effect35_timeline_start_emitter_b(void)
{
    wmap_start_sequence(wmap_effect35_run_emitter_b);
    g_wmap_effect35_timeline_timer = 13;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 30: wait for the step timer.
 */
void wmap_effect35_timeline_wait_30(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline step 32: wait for the step timer.
 */
void wmap_effect35_timeline_wait_32(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline step 33: start spinning model B.
 */
void wmap_effect35_timeline_start_spin_b(void)
{
    wmap_start_sequence(wmap_effect35_run_spin_b);
    g_wmap_effect35_timeline_timer = 124;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 34: wait for the step timer.
 */
void wmap_effect35_timeline_wait_34(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline step 35: set D_8011D500.
 */
void wmap_effect35_timeline_step_35(void)
{
    D_8011D500 = 0xFE;
    g_wmap_effect35_timeline_timer = 128;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 36: wait for the step timer.
 */
void wmap_effect35_timeline_wait_36(void)
{
    if (--g_wmap_effect35_timeline_timer == 0)
    {
        g_wmap_effect35_timeline_step++;
    }
}

/**
 * @brief Timeline step 37: end the timeline.
 */
void wmap_effect35_timeline_finish(void)
{
    D_8013B20C = 0;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Run the current step of the model drop A sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_drop_a(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_drop_a_step = 1;
        g_wmap_effect35_drop_a_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_drop_a_step >= WMAP_EFFECT35_DROP_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_drop_a_steps[g_wmap_effect35_drop_a_step]();
    return 1;
}

/**
 * @brief Model drop A: step 0: restart the sequence.
 */
void wmap_effect35_drop_a_reset(void)
{
    g_wmap_effect35_drop_a_step = 1;
    g_wmap_effect35_drop_a_timer = 1;
}

/**
 * @brief Model drop A: set up and run the first update.
 */
void wmap_effect35_drop_a_start(void)
{
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = WMAP_EFFECT35_DROP_START_Z;
    g_wmap_effect35_drop_a_timer = 32;
    g_wmap_effect35_drop_a_step++;
    wmap_effect35_drop_a_update();
}

/**
 * @brief Model drop A: last step: nothing left to do.
 */
void wmap_effect35_drop_a_end(void)
{
    g_wmap_effect35_drop_a_step++;
}

/**
 * @brief Run the current step of the model drop B sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_drop_b(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_drop_b_step = 1;
        g_wmap_effect35_drop_b_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_drop_b_step >= WMAP_EFFECT35_DROP_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_drop_b_steps[g_wmap_effect35_drop_b_step]();
    return 1;
}

/**
 * @brief Model drop B: step 0: restart the sequence.
 */
void wmap_effect35_drop_b_reset(void)
{
    g_wmap_effect35_drop_b_step = 1;
    g_wmap_effect35_drop_b_timer = 1;
}

/**
 * @brief Model drop B: set up and run the first update.
 */
void wmap_effect35_drop_b_start(void)
{
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = WMAP_EFFECT35_DROP_START_Z;
    g_wmap_effect35_drop_b_timer = 128;
    g_wmap_effect35_drop_b_step++;
    wmap_effect35_drop_b_update();
}

/**
 * @brief Model drop B: last step: nothing left to do.
 */
void wmap_effect35_drop_b_end(void)
{
    g_wmap_effect35_drop_b_step++;
}

/**
 * @brief Run the current step of the sprite A sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_sprite_a(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_sprite_a_step = 1;
        g_wmap_effect35_sprite_a_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_sprite_a_step >= WMAP_EFFECT35_SPRITE_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_sprite_a_steps[g_wmap_effect35_sprite_a_step]();
    return 1;
}

/**
 * @brief Sprite A: step 0: restart the sequence.
 */
void wmap_effect35_sprite_a_reset(void)
{
    g_wmap_effect35_sprite_a_step = 1;
    g_wmap_effect35_sprite_a_timer = 1;
}

/**
 * @brief Sprite A: set up and run the first update.
 */
void wmap_effect35_sprite_a_start(void)
{
    WmapSpriteActor* actor = &D_800D9268[5];

    D_80139988[5].data = &D_8011D538[2 * WMAP_EFFECT_BANK_SIZE];
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 2;
    actor->target_shade = 0x81;
    actor->unknown_02 = 0;
    actor->sequence = 0;
    actor->shade = 1;
    g_wmap_effect35_sprite_a_timer = 502;
    g_wmap_effect35_sprite_a_step++;
    wmap_effect35_sprite_a_draw();
}

/**
 * @brief Sprite A: animate and draw until the step timer runs out.
 */
void wmap_effect35_sprite_a_draw(void)
{
    wmap_step_actor_animation(&D_800D9268[5], &D_80139988[5]);
    wmap_draw_actor_sprite(&D_800D9268[5], g_wmap_focus_screen_position, 8, 2, 0);
    if (--g_wmap_effect35_sprite_a_timer == 0)
    {
        g_wmap_effect35_sprite_a_step++;
    }
}

/**
 * @brief Sprite A: start fading the sprite out.
 */
void wmap_effect35_sprite_a_fade(void)
{
    WmapSpriteActor* actor = &D_800D9268[5];

    actor->shade_step = 8;
    actor->target_shade = 0;
    g_wmap_effect35_sprite_a_timer = 16;
    g_wmap_effect35_sprite_a_step++;
    wmap_effect35_sprite_a_draw_fading();
}

/**
 * @brief Sprite A: animate and draw while fading out.
 */
void wmap_effect35_sprite_a_draw_fading(void)
{
    wmap_step_actor_animation(&D_800D9268[5], &D_80139988[5]);
    wmap_draw_actor_sprite(&D_800D9268[5], g_wmap_focus_screen_position, 8, 2, 0);
    if (--g_wmap_effect35_sprite_a_timer == 0)
    {
        g_wmap_effect35_sprite_a_step++;
    }
}

/**
 * @brief Sprite A: last step: nothing left to do.
 */
void wmap_effect35_sprite_a_end(void)
{
    g_wmap_effect35_sprite_a_step++;
}

/**
 * @brief Run the current step of the sprite B sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_sprite_b(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_sprite_b_step = 1;
        g_wmap_effect35_sprite_b_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_sprite_b_step >= WMAP_EFFECT35_SPRITE_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_sprite_b_steps[g_wmap_effect35_sprite_b_step]();
    return 1;
}

/**
 * @brief Sprite B: step 0: restart the sequence.
 */
void wmap_effect35_sprite_b_reset(void)
{
    g_wmap_effect35_sprite_b_step = 1;
    g_wmap_effect35_sprite_b_timer = 1;
}

/**
 * @brief Sprite B: set up and run the first update.
 */
void wmap_effect35_sprite_b_start(void)
{
    WmapSpriteActor* actor = &D_800D9268[6];
    s32 value;

    D_80139988[6].data = &D_8011D538[WMAP_EFFECT_BANK_SIZE];
    actor->scale_index = 0xF;
    actor->sequence = value = 1;
    actor->previous_sequence = -value;
    actor->shade_step = 0x80;
    actor->shade = value;
    actor->unknown_02 = 0;
    actor->target_shade = 0x81;
    g_wmap_effect35_sprite_b_timer = 280;
    g_wmap_effect35_sprite_b_step++;
    wmap_effect35_sprite_b_draw();
}

/**
 * @brief Sprite B: animate and draw until the step timer runs out.
 */
void wmap_effect35_sprite_b_draw(void)
{
    wmap_step_actor_animation(&D_800D9268[6], &D_80139988[6]);
    wmap_draw_actor_sprite(&D_800D9268[6], g_wmap_focus_screen_position, 0x19, 7, 0);
    if (--g_wmap_effect35_sprite_b_timer == 0)
    {
        g_wmap_effect35_sprite_b_step++;
    }
}

/**
 * @brief Sprite B: start fading the sprite out.
 */
void wmap_effect35_sprite_b_fade(void)
{
    WmapSpriteActor* actor = &D_800D9268[6];

    actor->shade_step = 0x80;
    actor->target_shade = 0;
    g_wmap_effect35_sprite_b_timer = 1;
    g_wmap_effect35_sprite_b_step++;
    wmap_effect35_sprite_b_draw_fading();
}

/**
 * @brief Sprite B: animate and draw while fading out.
 */
void wmap_effect35_sprite_b_draw_fading(void)
{
    wmap_step_actor_animation(&D_800D9268[6], &D_80139988[6]);
    wmap_draw_actor_sprite(&D_800D9268[6], g_wmap_focus_screen_position, 0x19, 7, 0);
    if (--g_wmap_effect35_sprite_b_timer == 0)
    {
        g_wmap_effect35_sprite_b_step++;
    }
}

/**
 * @brief Sprite B: last step: nothing left to do.
 */
void wmap_effect35_sprite_b_end(void)
{
    g_wmap_effect35_sprite_b_step++;
}

/**
 * @brief Run the current step of the sprite C sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_sprite_c(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_sprite_c_step = 1;
        g_wmap_effect35_sprite_c_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_sprite_c_step >= WMAP_EFFECT35_SPRITE_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_sprite_c_steps[g_wmap_effect35_sprite_c_step]();
    return 1;
}

/**
 * @brief Sprite C: step 0: restart the sequence.
 */
void wmap_effect35_sprite_c_reset(void)
{
    g_wmap_effect35_sprite_c_step = 1;
    g_wmap_effect35_sprite_c_timer = 1;
}

/**
 * @brief Sprite C: set up and run the first update.
 */
void wmap_effect35_sprite_c_start(void)
{
    WmapSpriteActor* actor = &D_800D9268[7];

    D_80139988[7].data = &D_8011D538[WMAP_EFFECT_BANK_SIZE];
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 2;
    actor->target_shade = 0x81;
    actor->unknown_02 = 0;
    actor->sequence = 0;
    actor->shade = 1;
    g_wmap_effect35_sprite_c_timer = 240;
    g_wmap_effect35_sprite_c_step++;
    wmap_effect35_sprite_c_draw();
}

/**
 * @brief Sprite C: animate and draw until the step timer runs out.
 */
void wmap_effect35_sprite_c_draw(void)
{
    wmap_step_actor_animation(&D_800D9268[7], &D_80139988[7]);
    wmap_draw_actor_sprite(&D_800D9268[7], g_wmap_focus_screen_position, 0x19, 8, 0);
    if (--g_wmap_effect35_sprite_c_timer == 0)
    {
        g_wmap_effect35_sprite_c_step++;
    }
}

/**
 * @brief Sprite C: start fading the sprite out.
 */
void wmap_effect35_sprite_c_fade(void)
{
    WmapSpriteActor* actor = &D_800D9268[7];

    actor->shade_step = 4;
    actor->target_shade = 0;
    g_wmap_effect35_sprite_c_timer = 32;
    g_wmap_effect35_sprite_c_step++;
    wmap_effect35_sprite_c_draw_fading();
}

/**
 * @brief Sprite C: animate and draw while fading out.
 */
void wmap_effect35_sprite_c_draw_fading(void)
{
    wmap_step_actor_animation(&D_800D9268[7], &D_80139988[7]);
    wmap_draw_actor_sprite(&D_800D9268[7], g_wmap_focus_screen_position, 0x19, 8, 0);
    if (--g_wmap_effect35_sprite_c_timer == 0)
    {
        g_wmap_effect35_sprite_c_step++;
    }
}

/**
 * @brief Sprite C: last step: nothing left to do.
 */
void wmap_effect35_sprite_c_end(void)
{
    g_wmap_effect35_sprite_c_step++;
}

/**
 * @brief Run the current step of the spinning model A sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_spin_a(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_spin_a_step = 1;
        g_wmap_effect35_spin_a_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_spin_a_step >= WMAP_EFFECT35_SPIN_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_spin_a_steps[g_wmap_effect35_spin_a_step]();
    return 1;
}

/**
 * @brief Spinning model A: step 0: restart the sequence.
 */
void wmap_effect35_spin_a_reset(void)
{
    g_wmap_effect35_spin_a_step = 1;
    g_wmap_effect35_spin_a_timer = 1;
}

/**
 * @brief Spinning model A: set up and run the first update.
 */
void wmap_effect35_spin_a_start(void)
{
    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    D_8013923C = 0;
    g_wmap_effect35_spin_a_timer = 240;
    g_wmap_effect35_spin_a_step++;
    wmap_effect35_spin_a_fade_in();
}

/**
 * @brief Spinning model A: start fading the model out.
 */
void wmap_effect35_spin_a_start_fade_out(void)
{
    g_wmap_effect35_spin_a_timer = 32;
    g_wmap_effect35_spin_a_step++;
    wmap_effect35_spin_a_fade_out();
}

/**
 * @brief Spinning model A: last step: nothing left to do.
 */
void wmap_effect35_spin_a_end(void)
{
    g_wmap_effect35_spin_a_step++;
}

/**
 * @brief Run the current step of the spinning model B sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_spin_b(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_spin_b_step = 1;
        g_wmap_effect35_spin_b_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_spin_b_step >= WMAP_EFFECT35_SPIN_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_spin_b_steps[g_wmap_effect35_spin_b_step]();
    return 1;
}

/**
 * @brief Spinning model B: step 0: restart the sequence.
 */
void wmap_effect35_spin_b_reset(void)
{
    g_wmap_effect35_spin_b_step = 1;
    g_wmap_effect35_spin_b_timer = 1;
}

/**
 * @brief Spinning model B: set up and run the first update.
 */
void wmap_effect35_spin_b_start(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139260 = 0;
    g_wmap_effect35_spin_b_timer = 208;
    g_wmap_effect35_spin_b_step++;
    wmap_effect35_spin_b_fade_in();
}

/**
 * @brief Spinning model B: start fading the model out.
 */
void wmap_effect35_spin_b_start_fade_out(void)
{
    g_wmap_effect35_spin_b_timer = 16;
    g_wmap_effect35_spin_b_step++;
    wmap_effect35_spin_b_fade_out();
}

/**
 * @brief Spinning model B: last step: nothing left to do.
 */
void wmap_effect35_spin_b_end(void)
{
    g_wmap_effect35_spin_b_step++;
}

/**
 * @brief Run the current step of the orbit particle 1 sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_orbiter_1(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_orbiter_1_step = 1;
        g_wmap_effect35_orbiter_1_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_orbiter_1_step >= WMAP_EFFECT35_ORBITER_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_orbiter_1_steps[g_wmap_effect35_orbiter_1_step]();
    return 1;
}

/**
 * @brief Orbit particle 1: step 0: restart the sequence.
 */
void wmap_effect35_orbiter_1_reset(void)
{
    g_wmap_effect35_orbiter_1_step = 1;
    g_wmap_effect35_orbiter_1_timer = 1;
}

/**
 * @brief Orbit particle 1: set up and run the first update.
 */
void wmap_effect35_orbiter_1_start(void)
{
    WmapSpriteActor* actor = &D_800D9268[8];
    WmapEffectMotion* motion = &D_801AFBD0[8];

    D_80139988[8].data = D_8011D538;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 0x10;
    actor->unknown_02 = 0;
    actor->sequence = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    motion->radius = 0x1964;
    motion->angle = 0xBB8;
    motion->speed = 0x64;
    motion->height = 0;
    motion->delay = 0x5A;
    g_wmap_effect35_orbiter_1_timer = 191;
    g_wmap_effect35_orbiter_1_step++;
    wmap_effect35_orbiter_1_update();
}

/**
 * @brief Orbit particle 1: last step: nothing left to do.
 */
void wmap_effect35_orbiter_1_end(void)
{
    g_wmap_effect35_orbiter_1_step++;
}

/**
 * @brief Run the current step of the orbit particle 2 sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_orbiter_2(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_orbiter_2_step = 1;
        g_wmap_effect35_orbiter_2_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_orbiter_2_step >= WMAP_EFFECT35_ORBITER_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_orbiter_2_steps[g_wmap_effect35_orbiter_2_step]();
    return 1;
}

/**
 * @brief Orbit particle 2: step 0: restart the sequence.
 */
void wmap_effect35_orbiter_2_reset(void)
{
    g_wmap_effect35_orbiter_2_step = 1;
    g_wmap_effect35_orbiter_2_timer = 1;
}

/**
 * @brief Orbit particle 2: set up and run the first update.
 */
void wmap_effect35_orbiter_2_start(void)
{
    WmapSpriteActor* actor = &D_800D9268[9];
    WmapEffectMotion* motion = &D_801AFBD0[9];

    D_80139988[9].data = D_8011D538;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 0x10;
    actor->unknown_02 = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    motion->radius = 0x2710;
    motion->angle = 0x200;
    motion->speed = 0x64;
    motion->height = 0;
    motion->delay = 0x3D;
    g_wmap_effect35_orbiter_2_timer = 191;
    g_wmap_effect35_orbiter_2_step++;
    wmap_effect35_orbiter_2_update();
}

/**
 * @brief Orbit particle 2: last step: nothing left to do.
 */
void wmap_effect35_orbiter_2_end(void)
{
    g_wmap_effect35_orbiter_2_step++;
}

/**
 * @brief Run the current step of the orbit particle 3 sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_orbiter_3(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_orbiter_3_step = 1;
        g_wmap_effect35_orbiter_3_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_orbiter_3_step >= WMAP_EFFECT35_ORBITER_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_orbiter_3_steps[g_wmap_effect35_orbiter_3_step]();
    return 1;
}

/**
 * @brief Orbit particle 3: step 0: restart the sequence.
 */
void wmap_effect35_orbiter_3_reset(void)
{
    g_wmap_effect35_orbiter_3_step = 1;
    g_wmap_effect35_orbiter_3_timer = 1;
}

/**
 * @brief Orbit particle 3: set up and run the first update.
 */
void wmap_effect35_orbiter_3_start(void)
{
    WmapSpriteActor* actor = &D_800D9268[10];
    WmapEffectMotion* motion = &D_801AFBD0[10];

    D_80139988[10].data = D_8011D538;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 0x10;
    actor->unknown_02 = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    motion->radius = 0x2710;
    motion->angle = 0x898;
    motion->speed = 0x64;
    motion->height = 0;
    motion->delay = 0x3D;
    g_wmap_effect35_orbiter_3_timer = 191;
    g_wmap_effect35_orbiter_3_step++;
    wmap_effect35_orbiter_3_update();
}

/**
 * @brief Orbit particle 3: last step: nothing left to do.
 */
void wmap_effect35_orbiter_3_end(void)
{
    g_wmap_effect35_orbiter_3_step++;
}

/**
 * @brief Run the current step of the orbit particle 4 sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_orbiter_4(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_orbiter_4_step = 1;
        g_wmap_effect35_orbiter_4_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_orbiter_4_step >= WMAP_EFFECT35_ORBITER_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_orbiter_4_steps[g_wmap_effect35_orbiter_4_step]();
    return 1;
}

/**
 * @brief Orbit particle 4: step 0: restart the sequence.
 */
void wmap_effect35_orbiter_4_reset(void)
{
    g_wmap_effect35_orbiter_4_step = 1;
    g_wmap_effect35_orbiter_4_timer = 1;
}

/**
 * @brief Orbit particle 4: set up and run the first update.
 */
void wmap_effect35_orbiter_4_start(void)
{
    WmapSpriteActor* actor = &D_800D9268[14];
    WmapEffectMotion* motion = &D_801AFBD0[14];

    D_80139988[14].data = D_8011D538;
    actor->scale_index = 0xF;
    actor->sequence = 2;
    actor->previous_sequence = -1;
    actor->shade_step = 0x10;
    actor->unknown_02 = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    motion->radius = 0x2710;
    motion->angle = 0x50;
    motion->speed = 0x64;
    motion->height = 0;
    motion->delay = 0x26;
    g_wmap_effect35_orbiter_4_timer = 191;
    g_wmap_effect35_orbiter_4_step++;
    wmap_effect35_orbiter_4_update();
}

/**
 * @brief Orbit particle 4: last step: nothing left to do.
 */
void wmap_effect35_orbiter_4_end(void)
{
    g_wmap_effect35_orbiter_4_step++;
}

/**
 * @brief Run the current step of the orbit particle 5 sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_orbiter_5(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_orbiter_5_step = 1;
        g_wmap_effect35_orbiter_5_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_orbiter_5_step >= WMAP_EFFECT35_ORBITER_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_orbiter_5_steps[g_wmap_effect35_orbiter_5_step]();
    return 1;
}

/**
 * @brief Orbit particle 5: step 0: restart the sequence.
 */
void wmap_effect35_orbiter_5_reset(void)
{
    g_wmap_effect35_orbiter_5_step = 1;
    g_wmap_effect35_orbiter_5_timer = 1;
}

/**
 * @brief Orbit particle 5: set up and run the first update.
 */
void wmap_effect35_orbiter_5_start(void)
{
    WmapSpriteActor* actor = &D_800D9268[15];
    WmapEffectMotion* motion = &D_801AFBD0[15];

    D_80139988[15].data = D_8011D538;
    actor->scale_index = 0xF;
    actor->sequence = 2;
    actor->previous_sequence = -1;
    actor->shade_step = 0x10;
    actor->unknown_02 = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    motion->radius = 0x1388;
    motion->angle = 0x6D6;
    motion->speed = 0x64;
    motion->height = 0;
    motion->delay = 0x26;
    g_wmap_effect35_orbiter_5_timer = 191;
    g_wmap_effect35_orbiter_5_step++;
    wmap_effect35_orbiter_5_update();
}

/**
 * @brief Orbit particle 5: last step: nothing left to do.
 */
void wmap_effect35_orbiter_5_end(void)
{
    g_wmap_effect35_orbiter_5_step++;
}

/**
 * @brief Run the current step of the orbit particle 6 sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_orbiter_6(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_orbiter_6_step = 1;
        g_wmap_effect35_orbiter_6_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_orbiter_6_step >= WMAP_EFFECT35_ORBITER_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_orbiter_6_steps[g_wmap_effect35_orbiter_6_step]();
    return 1;
}

/**
 * @brief Orbit particle 6: step 0: restart the sequence.
 */
void wmap_effect35_orbiter_6_reset(void)
{
    g_wmap_effect35_orbiter_6_step = 1;
    g_wmap_effect35_orbiter_6_timer = 1;
}

/**
 * @brief Orbit particle 6: set up and run the first update.
 */
void wmap_effect35_orbiter_6_start(void)
{
    WmapSpriteActor* actor = &D_800D9268[16];
    WmapEffectMotion* motion = &D_801AFBD0[16];

    D_80139988[16].data = D_8011D538;
    actor->scale_index = 0xF;
    actor->sequence = 2;
    actor->previous_sequence = -1;
    actor->shade_step = 0x10;
    actor->unknown_02 = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    motion->radius = 0x2710;
    motion->angle = 0xA8C;
    motion->speed = 0x64;
    motion->height = 0;
    motion->delay = 0x26;
    g_wmap_effect35_orbiter_6_timer = 191;
    g_wmap_effect35_orbiter_6_step++;
    wmap_effect35_orbiter_6_update();
}

/**
 * @brief Orbit particle 6: last step: nothing left to do.
 */
void wmap_effect35_orbiter_6_end(void)
{
    g_wmap_effect35_orbiter_6_step++;
}

/**
 * @brief Run the current step of the orbit particle 7 sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_orbiter_7(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_orbiter_7_step = 1;
        g_wmap_effect35_orbiter_7_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_orbiter_7_step >= WMAP_EFFECT35_ORBITER_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_orbiter_7_steps[g_wmap_effect35_orbiter_7_step]();
    return 1;
}

/**
 * @brief Orbit particle 7: step 0: restart the sequence.
 */
void wmap_effect35_orbiter_7_reset(void)
{
    g_wmap_effect35_orbiter_7_step = 1;
    g_wmap_effect35_orbiter_7_timer = 1;
}

/**
 * @brief Orbit particle 7: set up and run the first update.
 */
void wmap_effect35_orbiter_7_start(void)
{
    WmapSpriteActor* actor = &D_800D9268[17];
    WmapEffectMotion* motion = &D_801AFBD0[17];

    D_80139988[17].data = D_8011D538;
    actor->scale_index = 0xF;
    actor->sequence = 2;
    actor->previous_sequence = -1;
    actor->shade_step = 0x10;
    actor->unknown_02 = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    motion->radius = 0x2710;
    motion->angle = 0x320;
    motion->speed = 0x64;
    motion->height = 0;
    motion->delay = 0x26;
    g_wmap_effect35_orbiter_7_timer = 191;
    g_wmap_effect35_orbiter_7_step++;
    wmap_effect35_orbiter_7_update();
}

/**
 * @brief Orbit particle 7: last step: nothing left to do.
 */
void wmap_effect35_orbiter_7_end(void)
{
    g_wmap_effect35_orbiter_7_step++;
}

/**
 * @brief Run the current step of the particle emitter A sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_emitter_a(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_emitter_a_step = 1;
        g_wmap_effect35_emitter_a_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_emitter_a_step >= WMAP_EFFECT35_EMITTER_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_emitter_a_steps[g_wmap_effect35_emitter_a_step]();
    return 1;
}

/**
 * @brief Particle emitter A: step 0: restart the sequence.
 */
void wmap_effect35_emitter_a_reset(void)
{
    g_wmap_effect35_emitter_a_step = 1;
    g_wmap_effect35_emitter_a_timer = 1;
}

/**
 * @brief Particle emitter A: update and draw until the step timer runs out.
 */
void wmap_effect35_emitter_a_update(void)
{
    wmap_effect35_update_emitter(D_80139280);
    if (--g_wmap_effect35_emitter_a_timer == 0)
    {
        g_wmap_effect35_emitter_a_step++;
    }
}

/**
 * @brief Particle emitter A: stop spawning new particles.
 */
void wmap_effect35_emitter_a_stop(void)
{
    D_80139280->spawning = 0;
    g_wmap_effect35_emitter_a_timer = 16;
    g_wmap_effect35_emitter_a_step++;
    wmap_effect35_emitter_a_update_stopping();
}

/**
 * @brief Particle emitter A: update the remaining particles until the timer runs out.
 */
void wmap_effect35_emitter_a_update_stopping(void)
{
    wmap_effect35_update_emitter(D_80139280);
    if (--g_wmap_effect35_emitter_a_timer == 0)
    {
        g_wmap_effect35_emitter_a_step++;
    }
}

/**
 * @brief Particle emitter A: last step: nothing left to do.
 */
void wmap_effect35_emitter_a_end(void)
{
    g_wmap_effect35_emitter_a_step++;
}

/**
 * @brief Run the current step of the particle emitter B sequence.
 * @param reset Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static s32 wmap_effect35_run_emitter_b(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_effect35_emitter_b_step = 1;
        g_wmap_effect35_emitter_b_timer = 1;
        return 1;
    }

    if (g_wmap_effect35_emitter_b_step >= WMAP_EFFECT35_EMITTER_STEPS)
    {
        return 0;
    }

    g_wmap_effect35_emitter_b_steps[g_wmap_effect35_emitter_b_step]();
    return 1;
}

/**
 * @brief Particle emitter B: step 0: restart the sequence.
 */
void wmap_effect35_emitter_b_reset(void)
{
    g_wmap_effect35_emitter_b_step = 1;
    g_wmap_effect35_emitter_b_timer = 1;
}

/**
 * @brief Particle emitter B: update and draw until the step timer runs out.
 */
void wmap_effect35_emitter_b_update(void)
{
    wmap_effect35_update_emitter(&D_80139280[1]);
    if (--g_wmap_effect35_emitter_b_timer == 0)
    {
        g_wmap_effect35_emitter_b_step++;
    }
}

/**
 * @brief Particle emitter B: stop spawning new particles.
 */
void wmap_effect35_emitter_b_stop(void)
{
    D_80139280[1].spawning = 0;
    g_wmap_effect35_emitter_b_timer = 32;
    g_wmap_effect35_emitter_b_step++;
    wmap_effect35_emitter_b_update_stopping();
}

/**
 * @brief Particle emitter B: update the remaining particles until the timer runs out.
 */
void wmap_effect35_emitter_b_update_stopping(void)
{
    wmap_effect35_update_emitter(&D_80139280[1]);
    if (--g_wmap_effect35_emitter_b_timer == 0)
    {
        g_wmap_effect35_emitter_b_step++;
    }
}

/**
 * @brief Particle emitter B: last step: nothing left to do.
 */
void wmap_effect35_emitter_b_end(void)
{
    g_wmap_effect35_emitter_b_step++;
}
