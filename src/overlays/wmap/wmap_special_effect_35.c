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
#include "sdk/rand.h"
#include "wmap_step_sequence.h"

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

/** @brief Size of one effect animation bank in g_wmap_animation_bank_0. */
#define WMAP_EFFECT_BANK_SIZE 0x2000

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

extern WmapAnimationSlot g_wmap_actor_animations[];
extern WmapEffectMotion g_wmap_actor_motions[];
extern s32 D_800DBE70;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_load_buffer[];
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
extern u8* g_wmap_effect_model_pack_1;
extern u8* g_wmap_effect_model_pack_2;
extern u8* g_wmap_effect_model_pack_3;
extern u8 g_wmap_animation_bank_0[];
extern s32 D_80139228;
extern s32 D_8013923C;
extern s32 D_80139260;
extern WmapEffectEmitter* g_wmap_effect_params;
extern s32 g_wmap_view_mode;
extern s32 g_wmap_view_scroll_mode;
extern WmapView g_wmap_view;
extern s32 D_80139978;
extern s32 D_8013B294;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR g_wmap_camera_translation;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern u8 D_80182E40;
extern u8 D_8018B240;
extern u8 D_80193640;
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
        uintptr_t motion_base;

        motion_base = (uintptr_t)g_wmap_actor_motions;
        walker = (WmapEffectMotion*)((index * sizeof(WmapEffectMotion)) + motion_base);
        for (; index < emitter->end; index++)
        {
            motion = (WmapEffectMotion*)((uintptr_t)walker + index);
            motion = (WmapEffectMotion*)((uintptr_t)motion - index);
            if (motion->active != 0)
            {
                uintptr_t data_base;
                uintptr_t resource_base;
                s32 resource_offset;
                data_base = (uintptr_t)g_wmap_sprite_actors;
                actor = (WmapSpriteActor*)(index * sizeof(WmapSpriteActor) + data_base);
                position.vx = (motion->radius * (ccos(motion->angle) >> 5)) >> 0xC;
                position.vy = (motion->radius * (csin(motion->angle) >> 5)) >> 0xC;
                position.vz = motion->height;
                gte_ldv0(&position);
                gte_rtps();
                resource_offset = index * sizeof(WmapAnimationSlot);
                data_base = (uintptr_t)g_wmap_actor_animations;
                resource_base = data_base;
                data_base = 0;
                wmap_step_actor_animation(actor, (void*)(resource_offset + resource_base));
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
                if (g_wmap_actor_motions[index].active == 0)
                {
                    g_wmap_actor_motions[index].active = 1;
                    g_wmap_sprite_actors[index].scale_index = 0xF;
                    g_wmap_sprite_actors[index].sequence = 2;
                    g_wmap_sprite_actors[index].resource_index = 0;
                    g_wmap_sprite_actors[index].previous_sequence = -1;
                    g_wmap_sprite_actors[index].shade_step = emitter->shade_step;
                    g_wmap_sprite_actors[index].target_shade = emitter->target_shade;
                    g_wmap_sprite_actors[index].shade = emitter->shade;
                    g_wmap_actor_motions[index].radius = emitter->radius;
                    g_wmap_actor_motions[index].angle = rand() & 0xFFF;
                    speed = rand() * emitter->speed_range;
                    speed_min = emitter->speed_min;
                    g_wmap_actor_motions[index].height = 0;
                    g_wmap_actor_motions[index].speed = (speed >> 0xF) + speed_min;
                    g_wmap_actor_motions[index].delay = emitter->delay;
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
WMAP_STEP_MAP_DROP_UPDATE(wmap_effect35_drop_a_update,
    g_wmap_effect35_drop_a_step, g_wmap_effect35_drop_a_timer,
    g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a,
    g_wmap_effect_model_pack_1, -WMAP_EFFECT35_DROP_SPEED,
    4)

/**
 * @brief Model drop B: update and draw until the step timer runs out.
 */
WMAP_STEP_MAP_DROP_UPDATE(wmap_effect35_drop_b_update,
    g_wmap_effect35_drop_b_step, g_wmap_effect35_drop_b_timer,
    g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b,
    g_wmap_load_buffer, -WMAP_EFFECT35_DROP_SPEED,
    1)

/**
 * @brief Spinning model A: spin and draw the model while fading it in.
 */
void wmap_effect35_spin_a_fade_in(void)
{
    s32 value;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, (D_8013923C >> 4) & 3, 0xA, 0x35,
                  0x7800, 0x1001, g_wmap_effect_fade_c, 0, 0, -1);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_c, 2, 0x81, value);
    D_8013923C += 8;
    g_wmap_effect_model_c_rotation.vz += 0xA;
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
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, (D_8013923C >> 4) & 3, 0xA, 0x35,
                  0x7800, 0x1001, g_wmap_effect_fade_c, 0, 0, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_c, 4, value);
    D_8013923C += 8;
    g_wmap_effect_model_c_rotation.vz += 0xA;
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
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, (D_80139260 >> 4) & 7, 0xA, 0x36,
                  0x7880, 0x1001, g_wmap_effect_fade_d, 0, 0, -1);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_d, 2, 0x61, value);
    D_80139260 -= 0x10;
    g_wmap_effect_model_d_rotation.vz += 4;
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
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, (D_80139260 >> 4) & 7, 0xA, 0x36,
                  0x7880, 0x1001, g_wmap_effect_fade_d, 0, 0, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_d, 8, value);
    D_80139260 -= 0x10;
    g_wmap_effect_model_d_rotation.vz += 4;
    PopMatrix();
    if (--g_wmap_effect35_spin_b_timer == 0)
    {
        g_wmap_effect35_spin_b_step++;
    }
}

/** @brief Target shade, fade increment, inward speed, and duration of each orbiter. */
#define WMAP_EFFECT35_ORBITER_SHADE 128
#define WMAP_EFFECT35_ORBITER_SHADE_STEP 16
#define WMAP_EFFECT35_ORBITER_SPEED 100
#define WMAP_EFFECT35_ORBITER_FRAMES 191

/**
 * @brief Define an orbiter start: set its sprite and motion, then run the first update.
 * @param name Start function name.
 * @param step The orbiter sequence's step global.
 * @param timer Frames left before advancing the sequence.
 * @param slot Shared index in the actor, motion and animation pools.
 * @param sequence_id Constant animation sequence to start.
 * @param start_radius Initial radius stored in the motion record.
 * @param start_angle Initial angle stored in the motion record.
 * @param start_delay Initial delay counter.
 * @param next Update function, called immediately after advancing.
 * @note The constant conditions preserve the sprite's assignment order.
 */
#define WMAP_EFFECT35_DEFINE_ORBITER_START(name, step, timer, slot, sequence_id, start_radius, start_angle, start_delay, next) \
    void name(void)                                                                      \
    {                                                                                    \
        WmapSpriteActor* actor = &g_wmap_sprite_actors[(slot)];                          \
        WmapEffectMotion* motion = &g_wmap_actor_motions[(slot)];                        \
                                                                                         \
        g_wmap_actor_animations[(slot)].data = g_wmap_animation_bank_0;                  \
        actor->scale_index = WMAP_STEP_ACTOR_SCALE_INDEX;                                \
        if ((sequence_id) != 0)                                                          \
        {                                                                                \
            actor->sequence = (sequence_id);                                             \
        }                                                                                \
        actor->previous_sequence = -1;                                                   \
        actor->shade_step = WMAP_EFFECT35_ORBITER_SHADE_STEP;                            \
        actor->resource_index = 0;                                                       \
        if ((sequence_id) == 0)                                                          \
        {                                                                                \
            actor->sequence = 0;                                                         \
        }                                                                                \
        actor->target_shade = WMAP_EFFECT35_ORBITER_SHADE;                               \
        actor->shade = 0;                                                                \
        motion->radius = (start_radius);                                                 \
        motion->angle = (start_angle);                                                   \
        motion->speed = WMAP_EFFECT35_ORBITER_SPEED;                                     \
        motion->height = 0;                                                              \
        motion->delay = (start_delay);                                                   \
        (timer) = WMAP_EFFECT35_ORBITER_FRAMES;                                          \
        (step) += 1;                                                                     \
        next();                                                                          \
    }

/** @brief Fixed-point shifts used to project an orbiter's radius and angle. */
#define WMAP_EFFECT35_ORBITER_TRIG_SHIFT 5
#define WMAP_EFFECT35_ORBITER_PRODUCT_SHIFT 12

/**
 * @brief Define an orbiter update: draw, wait, move inward, then fade.
 * @param name Update function name.
 * @param slot_index Shared index in the actor, motion and animation pools.
 * @param step The orbiter sequence's step global.
 * @param timer Frames left before advancing the sequence.
 * @note Clearing the motion's active flag does not end this step. It keeps
 *       drawing until the timer expires.
 */
#define WMAP_EFFECT35_DEFINE_ORBITER_UPDATE(name, slot_index, step, timer)                       \
    void name(void)                                                                              \
    {                                                                                            \
        SVECTOR position;                                                                        \
        s32 screen_position;                                                                     \
        WmapSpriteActor* actor = &g_wmap_sprite_actors[(slot_index)];                            \
        WmapEffectMotion* motion = &g_wmap_actor_motions[(slot_index)];                          \
        WmapAnimationSlot* slots;                                                                \
                                                                                                 \
        position.vx = (motion->radius *                                                          \
            (ccos(motion->angle) >> WMAP_EFFECT35_ORBITER_TRIG_SHIFT)) >>                        \
            WMAP_EFFECT35_ORBITER_PRODUCT_SHIFT;                                                 \
        position.vy = (motion->radius *                                                          \
            (csin(motion->angle) >> WMAP_EFFECT35_ORBITER_TRIG_SHIFT)) >>                        \
            WMAP_EFFECT35_ORBITER_PRODUCT_SHIFT;                                                 \
        position.vz = motion->height;                                                            \
        gte_ldv0(&position);                                                                     \
        gte_rtps();                                                                              \
        slots = g_wmap_actor_animations;                                                         \
        wmap_step_actor_animation(actor, &slots[(slot_index)]);                                  \
        gte_stsxy(&screen_position);                                                             \
        wmap_draw_actor_sprite(actor, screen_position, WMAP_EFFECT35_TEXTURE_SPARK,              \
                               WMAP_EFFECT35_PARTICLE_OT, 0);                                    \
        if (motion->delay != 0)                                                                  \
        {                                                                                        \
            motion->delay--;                                                                     \
        }                                                                                        \
        else if (motion->radius >= WMAP_EFFECT35_INNER_RADIUS)                                   \
        {                                                                                        \
            motion->radius -= motion->speed;                                                     \
        }                                                                                        \
        else if (actor->shade >= WMAP_EFFECT35_FADED_SHADE)                                      \
        {                                                                                        \
            actor->target_shade = 0;                                                             \
        }                                                                                        \
        else                                                                                     \
        {                                                                                        \
            motion->active = 0;                                                                  \
        }                                                                                        \
        if (--(timer) == 0)                                                                      \
        {                                                                                        \
            (step)++;                                                                            \
        }                                                                                        \
    }

/** @brief Draw and update orbiter 1 until its timer expires. */
WMAP_EFFECT35_DEFINE_ORBITER_UPDATE(wmap_effect35_orbiter_1_update, 8,
    g_wmap_effect35_orbiter_1_step, g_wmap_effect35_orbiter_1_timer)

/** @brief Draw and update orbiter 2 until its timer expires. */
WMAP_EFFECT35_DEFINE_ORBITER_UPDATE(wmap_effect35_orbiter_2_update, 9,
    g_wmap_effect35_orbiter_2_step, g_wmap_effect35_orbiter_2_timer)

/** @brief Draw and update orbiter 3 until its timer expires. */
WMAP_EFFECT35_DEFINE_ORBITER_UPDATE(wmap_effect35_orbiter_3_update, 10,
    g_wmap_effect35_orbiter_3_step, g_wmap_effect35_orbiter_3_timer)

/** @brief Draw and update orbiter 4 until its timer expires. */
WMAP_EFFECT35_DEFINE_ORBITER_UPDATE(wmap_effect35_orbiter_4_update, 14,
    g_wmap_effect35_orbiter_4_step, g_wmap_effect35_orbiter_4_timer)

/** @brief Draw and update orbiter 5 until its timer expires. */
WMAP_EFFECT35_DEFINE_ORBITER_UPDATE(wmap_effect35_orbiter_5_update, 15,
    g_wmap_effect35_orbiter_5_step, g_wmap_effect35_orbiter_5_timer)

/** @brief Draw and update orbiter 6 until its timer expires. */
WMAP_EFFECT35_DEFINE_ORBITER_UPDATE(wmap_effect35_orbiter_6_update, 16,
    g_wmap_effect35_orbiter_6_step, g_wmap_effect35_orbiter_6_timer)

/** @brief Draw and update orbiter 7 until its timer expires. */
WMAP_EFFECT35_DEFINE_ORBITER_UPDATE(wmap_effect35_orbiter_7_update, 17,
    g_wmap_effect35_orbiter_7_step, g_wmap_effect35_orbiter_7_timer)

#undef WMAP_EFFECT35_DEFINE_ORBITER_UPDATE

/**
 * @brief Particle emitter A: set up and run the first update.
 */
void wmap_effect35_emitter_a_start(void)
{
    s32 i;
    WmapEffectEmitter* emitter;

    for (i = WMAP_EFFECT35_EMITTER_A_FIRST; i < WMAP_EFFECT35_EMITTER_A_END; i++)
    {
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_0;
        g_wmap_actor_motions[i].active = 0;
    }
    /* A plain word store: it may alias g_wmap_effect_params, so the pointer is reloaded below. */
    *(s32*)g_wmap_effect_params = WMAP_EFFECT35_EMITTER_A_FIRST;
    g_wmap_effect35_emitter_a_timer = 144;
    emitter = g_wmap_effect_params;
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
        g_wmap_actor_animations[i].data = &g_wmap_animation_bank_0[WMAP_EFFECT_BANK_SIZE];
        g_wmap_actor_motions[i].active = 0;
    }
    for (i = WMAP_EFFECT35_EMITTER_B_FIRST; i < WMAP_EFFECT35_EMITTER_B_PRESPAWN_END; i++)
    {
        g_wmap_sprite_actors[i].scale_index = 0xF;
        g_wmap_sprite_actors[i].sequence = 2;
        g_wmap_sprite_actors[i].previous_sequence = -1;
        g_wmap_sprite_actors[i].shade_step = 0x10;
        g_wmap_sprite_actors[i].target_shade = 0x81;
        g_wmap_sprite_actors[i].resource_index = 0;
        g_wmap_sprite_actors[i].shade = 1;
        g_wmap_actor_motions[i].radius = 0x2710;
        g_wmap_actor_motions[i].angle = rand() & 0xFFF;
        g_wmap_actor_motions[i].speed = ((rand() * 0x32) >> 0xF) + 0x32;
        g_wmap_actor_motions[i].height = 0;
        g_wmap_actor_motions[i].delay = 0x26;
    }
    g_wmap_effect35_emitter_b_timer = 240;
    g_wmap_effect_params[1].first = WMAP_EFFECT35_EMITTER_B_FIRST;
    g_wmap_effect_params[1].end = WMAP_EFFECT35_EMITTER_B_END;
    g_wmap_effect_params[1].shade_step = 0x10;
    g_wmap_effect_params[1].target_shade = 0x81;
    g_wmap_effect_params[1].speed_min = 0x32;
    g_wmap_effect_params[1].speed_range = 0x32;
    g_wmap_effect_params[1].shade = 1;
    g_wmap_effect_params[1].radius = 0x2710;
    g_wmap_effect_params[1].delay = 2;
    g_wmap_effect_params[1].spawning = 1;
    g_wmap_effect35_emitter_b_step++;
    wmap_effect35_emitter_b_update();
}

/**
 * @brief Run the current step of special effect 35.
 * @param arg0 Nonzero restarts the effect instead of running a step.
 * @return 1 while the effect runs, 0 once it has finished.
 */
WMAP_STEP_RUNNER(wmap_effect35_run,
    g_wmap_effect35_steps, WMAP_EFFECT35_MAIN_STEPS,
    g_wmap_effect35_step, g_wmap_effect35_timer)

/**
 * @brief Effect step 0: restart the effect.
 */
WMAP_STEP_RESET(wmap_effect35_reset, g_wmap_effect35_step, g_wmap_effect35_timer)

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
    g_wmap_sequence_busy = 1;
    g_wmap_effect35_step++;
    wmap_effect35_wait_timeline();
}

/**
 * @brief Effect step 2: wait for the timeline to finish.
 */
WMAP_STEP_WAIT_IDLE(wmap_effect35_wait_timeline,
    g_wmap_effect35_step, wmap_effect35_finish)

/**
 * @brief Start the world-map exit and advance the sequence.
 */
WMAP_STEP_BEGIN_EXIT(wmap_effect35_finish, g_wmap_effect35_step, 2)

/**
 * @brief Run the current step of the timeline sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_timeline,
    g_wmap_effect35_timeline_steps, WMAP_EFFECT35_TIMELINE_STEPS,
    g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_timeline_reset, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline step 1: scroll the map to the effect land.
 */
void wmap_effect35_timeline_scroll_to_land(void)
{
    g_wmap_event_active = 1;
    g_wmap_placement_overlay_hidden = 1;
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
WMAP_STEP_WAIT_SCROLL(wmap_effect35_timeline_wait_scroll,
    g_wmap_effect35_timeline_step, wmap_effect35_timeline_start_sprite_a)

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
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_04, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

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
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_06, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

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
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_08, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline: start orbit particle 1.
 */
WMAP_STEP_START_AND_WAIT(wmap_effect35_timeline_start_orbiter_1,
    g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer, wmap_effect35_run_orbiter_1, 100)

/**
 * @brief Timeline step 10: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_10, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline: start orbit particle 2.
 */
WMAP_STEP_START_AND_WAIT(wmap_effect35_timeline_start_orbiter_2,
    g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer, wmap_effect35_run_orbiter_2, 20)

/**
 * @brief Timeline step 12: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_12, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline: start orbit particle 3.
 */
WMAP_STEP_START_AND_WAIT(wmap_effect35_timeline_start_orbiter_3,
    g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer, wmap_effect35_run_orbiter_3, 15)

/**
 * @brief Timeline step 14: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_14, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline: start orbit particle 4.
 */
WMAP_STEP_START_AND_WAIT(wmap_effect35_timeline_start_orbiter_4,
    g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer, wmap_effect35_run_orbiter_4, 15)

/**
 * @brief Timeline step 16: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_16, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline: start orbit particle 5.
 */
WMAP_STEP_START_AND_WAIT(wmap_effect35_timeline_start_orbiter_5,
    g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer, wmap_effect35_run_orbiter_5, 10)

/**
 * @brief Timeline step 18: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_18, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline: start orbit particle 6.
 */
WMAP_STEP_START_AND_WAIT(wmap_effect35_timeline_start_orbiter_6,
    g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer, wmap_effect35_run_orbiter_6, 5)

/**
 * @brief Timeline step 20: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_20, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

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
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_22, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline: start particle emitter A.
 */
WMAP_STEP_START_AND_WAIT(wmap_effect35_timeline_start_emitter_a,
    g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer, wmap_effect35_run_emitter_a, 144)

/**
 * @brief Timeline step 24: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_24, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline step 26: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_26, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline step 27: start model drop B and hide the map view.
 */
void wmap_effect35_timeline_start_drop_b(void)
{
    wmap_start_sequence(wmap_effect35_run_drop_b);
    g_wmap_transition_mesh_hidden = 1;
    D_80139978 = -1;
    g_wmap_view_mode = -1;
    g_wmap_effect35_timeline_timer = 2;
    g_wmap_effect35_timeline_step++;
}

/**
 * @brief Timeline step 28: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_28, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline: start particle emitter B.
 */
WMAP_STEP_START_AND_WAIT(wmap_effect35_timeline_start_emitter_b,
    g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer, wmap_effect35_run_emitter_b, 13)

/**
 * @brief Timeline step 30: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_30, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline step 32: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_32, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline step 33: start spinning model B.
 */
WMAP_STEP_START_AND_WAIT(wmap_effect35_timeline_start_spin_b,
    g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer, wmap_effect35_run_spin_b, 124)

/**
 * @brief Timeline step 34: wait for the step timer.
 */
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_34, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

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
WMAP_STEP_WAIT(wmap_effect35_timeline_wait_36, g_wmap_effect35_timeline_step, g_wmap_effect35_timeline_timer)

/**
 * @brief Timeline step 37: end the timeline.
 */
WMAP_STEP_FINISH_BLOCKING(wmap_effect35_timeline_finish,
    g_wmap_effect35_timeline_step)

/**
 * @brief Run the current step of the model drop A sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_drop_a,
    g_wmap_effect35_drop_a_steps, WMAP_EFFECT35_DROP_STEPS,
    g_wmap_effect35_drop_a_step, g_wmap_effect35_drop_a_timer)

/**
 * @brief Model drop A: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_drop_a_reset, g_wmap_effect35_drop_a_step, g_wmap_effect35_drop_a_timer)

/**
 * @brief Model drop A: set up and run the first update.
 */
void wmap_effect35_drop_a_start(void)
{
    g_wmap_effect_model_a_position = g_wmap_camera_translation;
    g_wmap_effect_fade_a = 0x80;
    g_wmap_effect_model_a_position.vz = WMAP_EFFECT35_DROP_START_Z;
    g_wmap_effect35_drop_a_timer = 32;
    g_wmap_effect35_drop_a_step++;
    wmap_effect35_drop_a_update();
}

/**
 * @brief Model drop A: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_drop_a_end, g_wmap_effect35_drop_a_step)

/**
 * @brief Run the current step of the model drop B sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_drop_b,
    g_wmap_effect35_drop_b_steps, WMAP_EFFECT35_DROP_STEPS,
    g_wmap_effect35_drop_b_step, g_wmap_effect35_drop_b_timer)

/**
 * @brief Model drop B: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_drop_b_reset, g_wmap_effect35_drop_b_step, g_wmap_effect35_drop_b_timer)

/**
 * @brief Model drop B: set up and run the first update.
 */
void wmap_effect35_drop_b_start(void)
{
    g_wmap_effect_model_b_position = g_wmap_camera_translation;
    g_wmap_effect_fade_b = 0x80;
    g_wmap_effect_model_b_position.vz = WMAP_EFFECT35_DROP_START_Z;
    g_wmap_effect35_drop_b_timer = 128;
    g_wmap_effect35_drop_b_step++;
    wmap_effect35_drop_b_update();
}

/**
 * @brief Model drop B: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_drop_b_end, g_wmap_effect35_drop_b_step)

/**
 * @brief Run the current step of the sprite A sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_sprite_a,
    g_wmap_effect35_sprite_a_steps, WMAP_EFFECT35_SPRITE_STEPS,
    g_wmap_effect35_sprite_a_step, g_wmap_effect35_sprite_a_timer)

/**
 * @brief Sprite A: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_sprite_a_reset, g_wmap_effect35_sprite_a_step, g_wmap_effect35_sprite_a_timer)

/**
 * @brief Sprite A: set up and run the first update.
 */
WMAP_STEP_START_ACTOR(wmap_effect35_sprite_a_start,
    g_wmap_effect35_sprite_a_step, g_wmap_effect35_sprite_a_timer,
    5, &g_wmap_animation_bank_0[2 * WMAP_EFFECT_BANK_SIZE], 0,
    1, 0x81, 2,
    502, wmap_effect35_sprite_a_draw)

/**
 * @brief Sprite A: animate and draw until the step timer runs out.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_effect35_sprite_a_draw,
    g_wmap_effect35_sprite_a_step, g_wmap_effect35_sprite_a_timer,
    wmap_step_actor_animation(&g_wmap_sprite_actors[5], &g_wmap_actor_animations[5]),
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[5], g_wmap_focus_screen_position, 8, 2, 0))

/**
 * @brief Sprite A: start fading the sprite out.
 */
WMAP_STEP_FADE_ACTOR(wmap_effect35_sprite_a_fade,
    g_wmap_effect35_sprite_a_step, g_wmap_effect35_sprite_a_timer,
    g_wmap_sprite_actors[5], 8, 16, wmap_effect35_sprite_a_draw_fading)

/**
 * @brief Sprite A: animate and draw while fading out.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_effect35_sprite_a_draw_fading,
    g_wmap_effect35_sprite_a_step, g_wmap_effect35_sprite_a_timer,
    wmap_step_actor_animation(&g_wmap_sprite_actors[5], &g_wmap_actor_animations[5]),
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[5], g_wmap_focus_screen_position, 8, 2, 0))

/**
 * @brief Sprite A: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_sprite_a_end, g_wmap_effect35_sprite_a_step)

/**
 * @brief Run the current step of the sprite B sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_sprite_b,
    g_wmap_effect35_sprite_b_steps, WMAP_EFFECT35_SPRITE_STEPS,
    g_wmap_effect35_sprite_b_step, g_wmap_effect35_sprite_b_timer)

/**
 * @brief Sprite B: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_sprite_b_reset, g_wmap_effect35_sprite_b_step, g_wmap_effect35_sprite_b_timer)

/**
 * @brief Sprite B: set up and run the first update.
 */
void wmap_effect35_sprite_b_start(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];
    s32 value;

    g_wmap_actor_animations[6].data = &g_wmap_animation_bank_0[WMAP_EFFECT_BANK_SIZE];
    actor->scale_index = 0xF;
    actor->sequence = value = 1;
    actor->previous_sequence = -value;
    actor->shade_step = 0x80;
    actor->shade = value;
    actor->resource_index = 0;
    actor->target_shade = 0x81;
    g_wmap_effect35_sprite_b_timer = 280;
    g_wmap_effect35_sprite_b_step++;
    wmap_effect35_sprite_b_draw();
}

/**
 * @brief Sprite B: animate and draw until the step timer runs out.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_effect35_sprite_b_draw,
    g_wmap_effect35_sprite_b_step, g_wmap_effect35_sprite_b_timer,
    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]),
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], g_wmap_focus_screen_position, 0x19, 7, 0))

/**
 * @brief Sprite B: start fading the sprite out.
 */
WMAP_STEP_FADE_ACTOR(wmap_effect35_sprite_b_fade,
    g_wmap_effect35_sprite_b_step, g_wmap_effect35_sprite_b_timer,
    g_wmap_sprite_actors[6], 0x80, 1, wmap_effect35_sprite_b_draw_fading)

/**
 * @brief Sprite B: animate and draw while fading out.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_effect35_sprite_b_draw_fading,
    g_wmap_effect35_sprite_b_step, g_wmap_effect35_sprite_b_timer,
    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]),
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], g_wmap_focus_screen_position, 0x19, 7, 0))

/**
 * @brief Sprite B: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_sprite_b_end, g_wmap_effect35_sprite_b_step)

/**
 * @brief Run the current step of the sprite C sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_sprite_c,
    g_wmap_effect35_sprite_c_steps, WMAP_EFFECT35_SPRITE_STEPS,
    g_wmap_effect35_sprite_c_step, g_wmap_effect35_sprite_c_timer)

/**
 * @brief Sprite C: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_sprite_c_reset, g_wmap_effect35_sprite_c_step, g_wmap_effect35_sprite_c_timer)

/**
 * @brief Sprite C: set up and run the first update.
 */
WMAP_STEP_START_ACTOR(wmap_effect35_sprite_c_start,
    g_wmap_effect35_sprite_c_step, g_wmap_effect35_sprite_c_timer,
    7, &g_wmap_animation_bank_0[WMAP_EFFECT_BANK_SIZE], 0,
    1, 0x81, 2,
    240, wmap_effect35_sprite_c_draw)

/**
 * @brief Sprite C: animate and draw until the step timer runs out.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_effect35_sprite_c_draw,
    g_wmap_effect35_sprite_c_step, g_wmap_effect35_sprite_c_timer,
    wmap_step_actor_animation(&g_wmap_sprite_actors[7], &g_wmap_actor_animations[7]),
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[7], g_wmap_focus_screen_position, 0x19, 8, 0))

/**
 * @brief Sprite C: start fading the sprite out.
 */
WMAP_STEP_FADE_ACTOR(wmap_effect35_sprite_c_fade,
    g_wmap_effect35_sprite_c_step, g_wmap_effect35_sprite_c_timer,
    g_wmap_sprite_actors[7], 4, 32, wmap_effect35_sprite_c_draw_fading)

/**
 * @brief Sprite C: animate and draw while fading out.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_effect35_sprite_c_draw_fading,
    g_wmap_effect35_sprite_c_step, g_wmap_effect35_sprite_c_timer,
    wmap_step_actor_animation(&g_wmap_sprite_actors[7], &g_wmap_actor_animations[7]),
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[7], g_wmap_focus_screen_position, 0x19, 8, 0))

/**
 * @brief Sprite C: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_sprite_c_end, g_wmap_effect35_sprite_c_step)

/**
 * @brief Run the current step of the spinning model A sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_spin_a,
    g_wmap_effect35_spin_a_steps, WMAP_EFFECT35_SPIN_STEPS,
    g_wmap_effect35_spin_a_step, g_wmap_effect35_spin_a_timer)

/**
 * @brief Spinning model A: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_spin_a_reset, g_wmap_effect35_spin_a_step, g_wmap_effect35_spin_a_timer)

/**
 * @brief Spinning model A: set up and run the first update.
 */
void wmap_effect35_spin_a_start(void)
{
    g_wmap_effect_fade_c = 1;
    g_wmap_effect_model_c_rotation = g_wmap_zero_rotation;
    D_8013923C = 0;
    g_wmap_effect35_spin_a_timer = 240;
    g_wmap_effect35_spin_a_step++;
    wmap_effect35_spin_a_fade_in();
}

/**
 * @brief Spinning model A: start fading the model out.
 */
WMAP_STEP_ARM_TIMER(wmap_effect35_spin_a_start_fade_out,
    g_wmap_effect35_spin_a_step, g_wmap_effect35_spin_a_timer, 32, wmap_effect35_spin_a_fade_out)

/**
 * @brief Spinning model A: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_spin_a_end, g_wmap_effect35_spin_a_step)

/**
 * @brief Run the current step of the spinning model B sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_spin_b,
    g_wmap_effect35_spin_b_steps, WMAP_EFFECT35_SPIN_STEPS,
    g_wmap_effect35_spin_b_step, g_wmap_effect35_spin_b_timer)

/**
 * @brief Spinning model B: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_spin_b_reset, g_wmap_effect35_spin_b_step, g_wmap_effect35_spin_b_timer)

/**
 * @brief Spinning model B: set up and run the first update.
 */
void wmap_effect35_spin_b_start(void)
{
    g_wmap_effect_fade_d = 1;
    g_wmap_effect_model_d_rotation = g_wmap_zero_rotation;
    D_80139260 = 0;
    g_wmap_effect35_spin_b_timer = 208;
    g_wmap_effect35_spin_b_step++;
    wmap_effect35_spin_b_fade_in();
}

/**
 * @brief Spinning model B: start fading the model out.
 */
WMAP_STEP_ARM_TIMER(wmap_effect35_spin_b_start_fade_out,
    g_wmap_effect35_spin_b_step, g_wmap_effect35_spin_b_timer, 16, wmap_effect35_spin_b_fade_out)

/**
 * @brief Spinning model B: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_spin_b_end, g_wmap_effect35_spin_b_step)

/**
 * @brief Run the current step of the orbit particle 1 sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_orbiter_1,
    g_wmap_effect35_orbiter_1_steps, WMAP_EFFECT35_ORBITER_STEPS,
    g_wmap_effect35_orbiter_1_step, g_wmap_effect35_orbiter_1_timer)

/**
 * @brief Orbit particle 1: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_orbiter_1_reset, g_wmap_effect35_orbiter_1_step, g_wmap_effect35_orbiter_1_timer)

/**
 * @brief Orbit particle 1: set up and run the first update.
 */
WMAP_EFFECT35_DEFINE_ORBITER_START(wmap_effect35_orbiter_1_start,
    g_wmap_effect35_orbiter_1_step, g_wmap_effect35_orbiter_1_timer,
    8, 0, 0x1964, 0xBB8, 0x5A, wmap_effect35_orbiter_1_update)

/**
 * @brief Orbit particle 1: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_orbiter_1_end, g_wmap_effect35_orbiter_1_step)

/**
 * @brief Run the current step of the orbit particle 2 sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_orbiter_2,
    g_wmap_effect35_orbiter_2_steps, WMAP_EFFECT35_ORBITER_STEPS,
    g_wmap_effect35_orbiter_2_step, g_wmap_effect35_orbiter_2_timer)

/**
 * @brief Orbit particle 2: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_orbiter_2_reset, g_wmap_effect35_orbiter_2_step, g_wmap_effect35_orbiter_2_timer)

/**
 * @brief Orbit particle 2: set up and run the first update.
 */
WMAP_EFFECT35_DEFINE_ORBITER_START(wmap_effect35_orbiter_2_start,
    g_wmap_effect35_orbiter_2_step, g_wmap_effect35_orbiter_2_timer,
    9, 1, 0x2710, 0x200, 0x3D, wmap_effect35_orbiter_2_update)

/**
 * @brief Orbit particle 2: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_orbiter_2_end, g_wmap_effect35_orbiter_2_step)

/**
 * @brief Run the current step of the orbit particle 3 sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_orbiter_3,
    g_wmap_effect35_orbiter_3_steps, WMAP_EFFECT35_ORBITER_STEPS,
    g_wmap_effect35_orbiter_3_step, g_wmap_effect35_orbiter_3_timer)

/**
 * @brief Orbit particle 3: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_orbiter_3_reset, g_wmap_effect35_orbiter_3_step, g_wmap_effect35_orbiter_3_timer)

/**
 * @brief Orbit particle 3: set up and run the first update.
 */
WMAP_EFFECT35_DEFINE_ORBITER_START(wmap_effect35_orbiter_3_start,
    g_wmap_effect35_orbiter_3_step, g_wmap_effect35_orbiter_3_timer,
    10, 1, 0x2710, 0x898, 0x3D, wmap_effect35_orbiter_3_update)

/**
 * @brief Orbit particle 3: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_orbiter_3_end, g_wmap_effect35_orbiter_3_step)

/**
 * @brief Run the current step of the orbit particle 4 sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_orbiter_4,
    g_wmap_effect35_orbiter_4_steps, WMAP_EFFECT35_ORBITER_STEPS,
    g_wmap_effect35_orbiter_4_step, g_wmap_effect35_orbiter_4_timer)

/**
 * @brief Orbit particle 4: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_orbiter_4_reset, g_wmap_effect35_orbiter_4_step, g_wmap_effect35_orbiter_4_timer)

/**
 * @brief Orbit particle 4: set up and run the first update.
 */
WMAP_EFFECT35_DEFINE_ORBITER_START(wmap_effect35_orbiter_4_start,
    g_wmap_effect35_orbiter_4_step, g_wmap_effect35_orbiter_4_timer,
    14, 2, 0x2710, 0x50, 0x26, wmap_effect35_orbiter_4_update)

/**
 * @brief Orbit particle 4: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_orbiter_4_end, g_wmap_effect35_orbiter_4_step)

/**
 * @brief Run the current step of the orbit particle 5 sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_orbiter_5,
    g_wmap_effect35_orbiter_5_steps, WMAP_EFFECT35_ORBITER_STEPS,
    g_wmap_effect35_orbiter_5_step, g_wmap_effect35_orbiter_5_timer)

/**
 * @brief Orbit particle 5: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_orbiter_5_reset, g_wmap_effect35_orbiter_5_step, g_wmap_effect35_orbiter_5_timer)

/**
 * @brief Orbit particle 5: set up and run the first update.
 */
WMAP_EFFECT35_DEFINE_ORBITER_START(wmap_effect35_orbiter_5_start,
    g_wmap_effect35_orbiter_5_step, g_wmap_effect35_orbiter_5_timer,
    15, 2, 0x1388, 0x6D6, 0x26, wmap_effect35_orbiter_5_update)

/**
 * @brief Orbit particle 5: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_orbiter_5_end, g_wmap_effect35_orbiter_5_step)

/**
 * @brief Run the current step of the orbit particle 6 sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_orbiter_6,
    g_wmap_effect35_orbiter_6_steps, WMAP_EFFECT35_ORBITER_STEPS,
    g_wmap_effect35_orbiter_6_step, g_wmap_effect35_orbiter_6_timer)

/**
 * @brief Orbit particle 6: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_orbiter_6_reset, g_wmap_effect35_orbiter_6_step, g_wmap_effect35_orbiter_6_timer)

/**
 * @brief Orbit particle 6: set up and run the first update.
 */
WMAP_EFFECT35_DEFINE_ORBITER_START(wmap_effect35_orbiter_6_start,
    g_wmap_effect35_orbiter_6_step, g_wmap_effect35_orbiter_6_timer,
    16, 2, 0x2710, 0xA8C, 0x26, wmap_effect35_orbiter_6_update)

/**
 * @brief Orbit particle 6: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_orbiter_6_end, g_wmap_effect35_orbiter_6_step)

/**
 * @brief Run the current step of the orbit particle 7 sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_orbiter_7,
    g_wmap_effect35_orbiter_7_steps, WMAP_EFFECT35_ORBITER_STEPS,
    g_wmap_effect35_orbiter_7_step, g_wmap_effect35_orbiter_7_timer)

/**
 * @brief Orbit particle 7: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_orbiter_7_reset, g_wmap_effect35_orbiter_7_step, g_wmap_effect35_orbiter_7_timer)

/**
 * @brief Orbit particle 7: set up and run the first update.
 */
WMAP_EFFECT35_DEFINE_ORBITER_START(wmap_effect35_orbiter_7_start,
    g_wmap_effect35_orbiter_7_step, g_wmap_effect35_orbiter_7_timer,
    17, 2, 0x2710, 0x320, 0x26, wmap_effect35_orbiter_7_update)

#undef WMAP_EFFECT35_DEFINE_ORBITER_START

/**
 * @brief Orbit particle 7: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_orbiter_7_end, g_wmap_effect35_orbiter_7_step)

/**
 * @brief Run the current step of the particle emitter A sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_emitter_a,
    g_wmap_effect35_emitter_a_steps, WMAP_EFFECT35_EMITTER_STEPS,
    g_wmap_effect35_emitter_a_step, g_wmap_effect35_emitter_a_timer)

/**
 * @brief Particle emitter A: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_emitter_a_reset, g_wmap_effect35_emitter_a_step, g_wmap_effect35_emitter_a_timer)

/**
 * @brief Particle emitter A: update and draw until the step timer runs out.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_effect35_emitter_a_update,
    g_wmap_effect35_emitter_a_step, g_wmap_effect35_emitter_a_timer,
    wmap_effect35_update_emitter(g_wmap_effect_params))

/**
 * @brief Particle emitter A: stop spawning new particles.
 */
void wmap_effect35_emitter_a_stop(void)
{
    g_wmap_effect_params->spawning = 0;
    g_wmap_effect35_emitter_a_timer = 16;
    g_wmap_effect35_emitter_a_step++;
    wmap_effect35_emitter_a_update_stopping();
}

/**
 * @brief Particle emitter A: update the remaining particles until the timer runs out.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_effect35_emitter_a_update_stopping,
    g_wmap_effect35_emitter_a_step, g_wmap_effect35_emitter_a_timer,
    wmap_effect35_update_emitter(g_wmap_effect_params))

/**
 * @brief Particle emitter A: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_emitter_a_end, g_wmap_effect35_emitter_a_step)

/**
 * @brief Run the current step of the particle emitter B sequence.
 * @param arg0 Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once it has finished.
 */
static WMAP_STEP_RUNNER(wmap_effect35_run_emitter_b,
    g_wmap_effect35_emitter_b_steps, WMAP_EFFECT35_EMITTER_STEPS,
    g_wmap_effect35_emitter_b_step, g_wmap_effect35_emitter_b_timer)

/**
 * @brief Particle emitter B: step 0: restart the sequence.
 */
WMAP_STEP_RESET(wmap_effect35_emitter_b_reset, g_wmap_effect35_emitter_b_step, g_wmap_effect35_emitter_b_timer)

/**
 * @brief Particle emitter B: update and draw until the step timer runs out.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_effect35_emitter_b_update,
    g_wmap_effect35_emitter_b_step, g_wmap_effect35_emitter_b_timer,
    wmap_effect35_update_emitter(&g_wmap_effect_params[1]))

/**
 * @brief Particle emitter B: stop spawning new particles.
 */
void wmap_effect35_emitter_b_stop(void)
{
    g_wmap_effect_params[1].spawning = 0;
    g_wmap_effect35_emitter_b_timer = 32;
    g_wmap_effect35_emitter_b_step++;
    wmap_effect35_emitter_b_update_stopping();
}

/**
 * @brief Particle emitter B: update the remaining particles until the timer runs out.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_effect35_emitter_b_update_stopping,
    g_wmap_effect35_emitter_b_step, g_wmap_effect35_emitter_b_timer,
    wmap_effect35_update_emitter(&g_wmap_effect_params[1]))

/**
 * @brief Particle emitter B: last step: finish the sequence.
 */
WMAP_STEP_ADVANCE(wmap_effect35_emitter_b_end, g_wmap_effect35_emitter_b_step)
