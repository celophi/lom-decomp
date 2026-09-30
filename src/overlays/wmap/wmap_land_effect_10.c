#include "wmap_frame_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_10.h"
#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"

/** @brief Spark emitter tuning block passed to func_8008ECF8. */
typedef struct
{
    s32 period;
    s32 ot_index;
    s32 spread_base;
    s32 spread_range;
    s32 start_z;
    s32 trail_length;
    s32 velocity_x;
    s32 velocity_y;
    s32 velocity_z;
    s32 fade_frames;
    s32 sprite_id;
    s32 sprite_field_0E;
} WmapSparkConfig;

/** @brief Spark particle: 4.12-ish fixed position and per-frame velocity. */
typedef struct
{
    SVECTOR position;
    SVECTOR velocity;
} WmapSparkParticle;

/** @brief Per-spark state slot (0 = free, 1 = flying, 2 = fading sprite). */
typedef struct
{
    s16 state;
    u8 pad_02[0xA];
    s16 timer;
    u8 pad_0E[2];
    s32 screen_xy;
} WmapSparkSlot;

/** @brief World-map actor configuration. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapSparkActor;

void wmap_land_effect_10_sequence_4_step_02(void);
void wmap_land_effect_10_sequence_5_step_02(void);
void wmap_land_effect_10_sequence_10_step_02(void);
void wmap_land_effect_10_sequence_11_step_02(void);
void wmap_land_effect_10_wait_idle_02(void);
void wmap_land_effect_10_step_03(void);
s32 wmap_land_effect_10_run_timeline(s32 arg0);
void wmap_land_effect_10_wait_idle_04(void);
void wmap_land_effect_10_end(void);
s32 wmap_land_effect_10_run_sequence_2(s32 arg0);
s32 wmap_land_effect_10_run_sequence_5(s32 arg0);
s32 wmap_land_effect_10_run_sequence_4(s32 arg0);
s32 wmap_land_effect_10_run_sequence_7(s32 arg0);
s32 wmap_land_effect_10_run_sequence_1(s32 arg0);
s32 wmap_land_effect_10_run_sequence_11(s32 arg0);
s32 wmap_land_effect_10_run_sequence_9(s32 arg0);
s32 wmap_land_effect_10_run_sequence_10(s32 arg0);
s32 wmap_land_effect_10_run_sequence_3(s32 arg0);
s32 wmap_land_effect_10_run_sequence_8(s32 arg0);
s32 wmap_land_effect_10_run_sequence_6(s32 arg0);
void wmap_land_effect_10_sequence_1_step_02(void);
void wmap_land_effect_10_sequence_2_step_02(void);
void wmap_land_effect_10_sequence_2_step_04(void);
void wmap_land_effect_10_sequence_3_step_02(void);
void wmap_land_effect_10_sequence_3_step_04(void);
void wmap_land_effect_10_sequence_4_step_04(void);
void wmap_land_effect_10_sequence_5_step_04(void);
void wmap_land_effect_10_sequence_6_step_02(void);
void func_8008ECF8(s32 first, s32 end, WmapSparkParticle *particles, WmapSparkConfig *config);

/** @brief World-map 8-byte slot: only the +4 pointer field is written here. */
typedef struct
{
    s32 field_00;
    void *field_04;
} WmapSlot8;

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

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

extern s32 D_800D9230;
extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 g_wmap_land_effect_10_sequence_4_timer;
extern s32 g_wmap_land_effect_10_sequence_5_timer;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_10_sequence_8_timer;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_10_sequence_7_timer;
extern s8 D_80051B4C[];
extern void *D_8011CF24;
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern s32 D_801B24B4;
extern s32 g_wmap_land_effect_10_sequence_9_timer;
extern u8 D_80121538[];
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 g_wmap_land_effect_10_sequence_10_timer;
extern s32 rand(void);
extern s32 g_wmap_land_effect_10_sequence_11_timer;
extern s32 g_wmap_land_effect_10_timer;
extern void (*D_800D5EE0[])(void);
extern void wmap_land_effect_10_step_03(void);
extern void wmap_land_effect_10_end(void);
extern s32 g_wmap_land_effect_10_timeline_timer;
extern void (*D_800D5EF8[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_10_sequence_1_timer;
extern void (*D_800D5F50[])(void);
extern u8* D_801399AC;
extern void wmap_land_effect_10_sequence_1_step_02(void);
extern s32 g_wmap_land_effect_10_sequence_2_timer;
extern void (*D_800D5F60[])(void);
extern void *D_801399CC;
extern void wmap_land_effect_10_sequence_2_step_04(void);
extern s32 g_wmap_land_effect_10_sequence_3_timer;
extern void (*D_800D5F78[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern void wmap_land_effect_10_sequence_3_step_02(void);
extern void wmap_land_effect_10_sequence_3_step_04(void);
extern void (*D_800D5F90[])(void);
extern u8 D_800D99F8[];
extern u8 D_80139AE8[];
extern void wmap_land_effect_10_sequence_4_step_04(void);
extern void (*D_800D5FA8[])(void);
extern WmapAnimationSlot D_80139A28[];
extern void wmap_land_effect_10_sequence_5_step_04(void);
extern s32 g_wmap_land_effect_10_sequence_6_timer;
extern void (*D_800D5FC0[])(void);
extern u8* D_801399BC;
extern void wmap_land_effect_10_sequence_6_step_02(void);
extern void (*D_800D5FD0[])(void);
extern void (*D_800D5FE0[])(void);
extern void (*D_800D5FF0[])(void);
extern void (*D_800D6008[])(void);
extern void (*D_800D6018[])(void);
extern WmapSparkParticle* D_8011CF28;

/** @brief World-map actor configuration. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

extern u8 D_8011D538[];
extern u32 g_wmap_land_effect_10_sequence_4_step;
extern WmapConfigA D_800D95D8[];
extern u32 g_wmap_land_effect_10_sequence_5_step;
extern u32 g_wmap_land_effect_10_sequence_8_step;
extern u32 g_wmap_land_effect_10_sequence_7_step;

extern u32 g_wmap_land_effect_10_sequence_9_step;
extern u32 g_wmap_land_effect_10_sequence_10_step;
extern u32 g_wmap_land_effect_10_sequence_11_step;
extern u32 g_wmap_land_effect_10_step;
extern u32 g_wmap_land_effect_10_timeline_step;
extern u32 g_wmap_land_effect_10_sequence_1_step;
extern u32 g_wmap_land_effect_10_sequence_2_step;
extern u32 g_wmap_land_effect_10_sequence_3_step;
extern u32 g_wmap_land_effect_10_sequence_6_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_801B2498;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D93C8;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399C8;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801AFD60[];
extern WmapMotion D_801B0080[];

/**
 * @brief Spawn, move, and draw a batch of falling sparks, then fade them out as sprites.
 * @param first First particle index to process.
 * @param end One past the last particle index to process.
 * @param particles Particle position/velocity array indexed by particle.
 * @param config Emitter tuning block.
 */
void func_8008ECF8(s32 first, s32 end, WmapSparkParticle *particles, WmapSparkConfig *config)
{
    SVECTOR position;
    s32 head_xy;
    s32 tail_xy;
    LINE_G2 *line;
    WmapSparkActor *actor;
    WmapSparkSlot *slot;
    SVECTOR *velocity;
    s32 spawn;
    s32 i;

    spawn = 3;
    if (D_8011CF74 % config->period == 0)
    {
        for (i = first; i < end; i++)
        {
            slot = &D_801AFBD0[i];
            if (slot->state == 0)
            {
                slot->state = 1;
                particles[i].position.vx = config->spread_base + ((rand() * config->spread_range) >> 15);
                particles[i].position.vy = config->spread_base + ((rand() * config->spread_range) >> 15);
                particles[i].position.vz = config->start_z;
                particles[i].velocity.vx = config->velocity_x;
                velocity = &particles[i].velocity;
                velocity->vy = config->velocity_y;
                velocity->vz = config->velocity_z;
                if (--spawn == 0)
                {
                    break;
                }
            }
        }
    }
    D_800D9230 = 0;
    for (i = first; i < end; i++)
    {
        slot = &D_801AFBD0[i];
        switch (slot->state)
        {
        case 1:
            position.vx = particles[i].position.vx / 16;
            position.vy = particles[i].position.vy / 16;
            position.vz = particles[i].position.vz / 16;
            gte_ldv0(&position);
            gte_rtps();
            position.vx = (particles[i].position.vx - particles[i].velocity.vx * config->trail_length) / 16;
            position.vy = (particles[i].position.vy - particles[i].velocity.vy * config->trail_length) / 16;
            position.vz = (particles[i].position.vz - particles[i].velocity.vz * config->trail_length) / 16;
            gte_stsxy(&head_xy);
            gte_ldv0(&position);
            gte_rtps();
            gte_stsxy(&tail_xy);
            line = (LINE_G2 *)g_wmap_current_frame->packet_cursor;
            *(s32 *)&line->r0 = 0x808080;
            *(s32 *)&line->r1 = 0;
            *(s32 *)&line->x0 = head_xy;
            *(s32 *)&line->x1 = tail_xy;
            setlen(line, 4);
            setcode(line, 0x52);
            addPrim(&g_wmap_current_frame->ordering_table[config->ot_index], line);
            if (g_wmap_packet_bytes < 0x7D00)
            {
                g_wmap_packet_bytes += 0x14;
                g_wmap_current_frame->packet_cursor += 0x14;
            }
            particles[i].position.vx += particles[i].velocity.vx;
            particles[i].position.vy += particles[i].velocity.vy;
            particles[i].position.vz += particles[i].velocity.vz;
            D_800D9230++;
            if (particles[i].position.vz < 0)
            {
                actor = &D_800D9268[i];
                slot->screen_xy = head_xy;
                slot->timer = config->fade_frames;
                slot->state = 2;
                actor->field_02 = 0;
                actor->field_06 = 0xF;
                actor->field_0E = config->sprite_field_0E;
                actor->field_10 = -1;
                actor->field_26 = 0x80 / config->fade_frames;
                actor->field_22 = 1;
                actor->field_24 = 0x81;
            }
            break;
        case 2:
            if (config->sprite_id != -1)
            {
                actor = &D_800D9268[i];
                wmap_step_actor_animation(actor, &D_80139988[i]);
                wmap_draw_actor_sprite(actor, *(s32 *)((u8 *)&D_801AFBD0[i] + 0x10), config->sprite_id, config->ot_index, 0);
                /* Reuse the exhausted spawn counter for the fade test. */
                spawn = --slot->timer >= 0;
                if (spawn)
                {
                    break;
                }
            }
            slot->state = 0;
            break;
        }
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_10_sequence_4_step_01(void)
{
    s32 i;

    D_801B0FD0 = 10;
    D_80139280[0xB] = 1;
    D_80139280[0xC] = 4;
    D_80139280[0xD] = 144;
    D_80139280[0xE] = 1;
    D_80139280[0xF] = 2;
    D_80139280[0x10] = 6;
    D_80139280[0x11] = 40;
    D_80139280[0x12] = 29;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 12000;
    for (i = 0; i < 10; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].state = 0;
        D_80139988[i + 44].data = D_8011D538;
    }
    g_wmap_land_effect_10_sequence_4_timer = 40;
    g_wmap_land_effect_10_sequence_4_step++;
    wmap_land_effect_10_sequence_4_step_02();
}

/** @brief Initialize the effect actors, resources, and evenly spaced angles. */
void wmap_land_effect_10_sequence_5_step_01(void)
{
    s32 i;

    i = 0;
    D_801B0FD0 = 20;
    D_80139280[0x15] = 0;
    D_80139280[0x16] = 0;
    D_80139280[0x17] = 128;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = 2;
    D_80139280[0x1A] = 1000;
    D_80139280[0x1B] = 20;
    D_80139280[0x1C] = 29;
    D_80139280[0x1D] = 3;
    D_80139280[0x1E] = 13000;
    do
    {
        D_801AFD60[i].state = 0;
        D_801AFD60[i].angle = i * 204;
        D_801AFD60[i].scale = 128;
        D_801AFD60[i].z = 13000;
        D_801AFD60[i].x = 0;
        D_801AFD60[i].field_0E = 0;
        D_80139988[i + 20].data = D_8011D538;
        D_800D95D8[i].field_06 = 15;
        D_800D95D8[i].field_0E = 3;
        D_800D95D8[i].field_10 = -1;
        D_800D95D8[i].field_26 = 1;
        D_800D95D8[i].field_02 = 0;
        D_800D95D8[i].field_22 = 0;
        D_800D95D8[i].field_24 = 127;
        i++;
    } while (i < 20);
    g_wmap_land_effect_10_sequence_5_timer = 40;
    g_wmap_land_effect_10_sequence_5_step++;
    wmap_land_effect_10_sequence_5_step_02();
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_10_sequence_7_step_02(void)
{
    MATRIX m;
    s32 x;

    x = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = x;
    if (x < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A0, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DE8 != 0)
    {
        wmap_draw_model_default((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x2;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_10_sequence_7_timer == 0)
    {
        g_wmap_land_effect_10_sequence_7_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_10_sequence_8_step_02(void)
{
    MATRIX m;
    s32 x;

    x = D_801B2478.vz - 0xDAC;
    D_801B2478.vz = x;
    if (x < 0x2710)
    {
        D_801B2478.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A8, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DEC != 0)
    {
        wmap_draw_model_default(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DEC);
        D_80182DEC -= 0x2;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_10_sequence_8_timer == 0)
    {
        g_wmap_land_effect_10_sequence_8_step += 1;
    }
}

/** @brief Draw two oscillating effect layers and update their intensity. */
void wmap_land_effect_10_sequence_9_step_02(void)
{
    s32 first_frame;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF24, first_frame, 8, 0x35, 0x7800, 0, D_801B2468);
    first_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    D_801B2490.vz = (u16) (D_801B2490.vz - 0x14);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF24, first_frame, 8, 0x35, 0x7800, 0, D_801B2468);
    D_801B2498.vz = (u16) (D_801B2498.vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 6) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 3) & 0xFF;
    intensity = D_801B2468 + 1;
    D_801B2468 = intensity;
    if (intensity >= 0x81)
    {
        D_801B2468 = 0x80;
    }
    remaining = g_wmap_land_effect_10_sequence_9_timer - 1;
    g_wmap_land_effect_10_sequence_9_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_10_sequence_9_step += 1;
    }
}

/** @brief Draw two oscillating effect layers and reduce their shared intensity. */
void wmap_land_effect_10_sequence_9_step_04(void)
{
    s32 first_frame;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF24, first_frame, 8, 0x35, 0x7800, 0, D_801B2468);
    first_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    D_801B2490.vz = (u16) (D_801B2490.vz - 0x14);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF24, first_frame, 8, 0x35, 0x7800, 0, D_801B2468);
    D_801B2498.vz = (u16) (D_801B2498.vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 6) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 3) & 0xFF;
    intensity = D_801B2468 - 2;
    D_801B2468 = intensity;
    if (intensity < 0)
    {
        D_801B2468 = 0;
    }
    remaining = g_wmap_land_effect_10_sequence_9_timer - 1;
    g_wmap_land_effect_10_sequence_9_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_10_sequence_9_step += 1;
    }
}

/** @brief Initialize twenty-four actors and advance to their update step. */
void wmap_land_effect_10_sequence_10_step_01(void)
{
    s32 i;
    WmapConfigA *actor;
    WmapSlot8 *resource;
    WmapSlot14 *slot;

    D_80139234 = 0;
    D_8013923C = 0xFFFF;
    for (i = 60; i < 84; i++)
    {
        actor = &D_800D9268[i];
        resource = &D_80139988[i];
        slot = &D_801AFBD0[i];
        resource->field_04 = D_80121538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_26 = 4;
        actor->field_22 = 1;
        actor->field_02 = 0;
        actor->field_0E = 0;
        actor->field_24 = 0x81;
        slot->field_00 = 0;
    }
    g_wmap_land_effect_10_sequence_10_timer = D_8013923C;
    g_wmap_land_effect_10_sequence_10_step++;
    wmap_land_effect_10_sequence_10_step_02();
}

/** @brief Spawn and draw spiraling particles until the effect finishes. */
void wmap_land_effect_10_sequence_10_step_02(void)
{
    SVECTOR position;
    s32 screen_position;
    s32 i;
    s32 finished;
    s32 value;
    WmapMotion *motion;
    WmapConfigA *actor;

    if (D_80139234 < 24)
    {
        motion = &D_801B0080[D_80139234];
        motion->state = 1;
        motion->z = 150000;
        motion->angle = rand() & 0xFFF;
        motion->field_0E = 0;
        D_80139234++;
    }
    finished = 1;
    for (i = 60; i < 84; i++)
    {
        motion = &D_801AFBD0[i];
        actor = &D_800D9268[i];
        if (motion->state != 0)
        {
            finished = 0;
            position.vx = ((motion->z >> 3) * (ccos(motion->angle) >> 6)) >> 12;
            position.vy = ((motion->z >> 3) * (csin(motion->angle) >> 6)) >> 12;
            position.vz = motion->field_0E;
            gte_ldv0(&position);
            gte_rtps();
            wmap_step_actor_animation(actor, &D_80139988[i]);
            value = motion->z - 3500;
            motion->z = value;
            if (value < 5000)
            {
                motion->state = 0;
            }
            motion->angle += 96;
            gte_stsxy(&screen_position);
            wmap_draw_actor_sprite(actor, screen_position, 19, 10, 0);
            if (actor->field_24 < 4)
            {
                motion->state = 0;
            }
        }
    }
    if (finished != 0)
    {
        g_wmap_land_effect_10_sequence_10_timer = 1;
    }
    value = g_wmap_land_effect_10_sequence_10_timer - 1;
    g_wmap_land_effect_10_sequence_10_timer = value;
    if (value == 0)
    {
        g_wmap_land_effect_10_sequence_10_step++;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_10_sequence_11_step_01(void)
{
    s32 i;

    for (i = 100; i < 150; i++)
    {
        D_801AFBD0[i].state = 0;
        D_80139988[i].data = D_80121538;
    }
    g_wmap_land_effect_10_sequence_11_timer = 200;
    D_80139280[0x1E] = 1;
    D_80139280[0x1F] = 2;
    D_80139280[0x20] = -1000;
    D_80139280[0x21] = 6000;
    D_80139280[0x22] = 5000;
    D_80139280[0x23] = 5;
    D_80139280[0x24] = -100;
    D_80139280[0x25] = 0;
    D_80139280[0x26] = -300;
    D_80139280[0x27] = 8;
    D_80139280[0x28] = 19;
    D_80139280[0x29] = 3;
    g_wmap_land_effect_10_sequence_11_step++;
    wmap_land_effect_10_sequence_11_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_10_run, D_800D5EE0, 0x6, g_wmap_land_effect_10_step, g_wmap_land_effect_10_timer)

WMAP_STEP_RESET(wmap_land_effect_10_reset, g_wmap_land_effect_10_step, g_wmap_land_effect_10_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_10_step_01, g_wmap_land_effect_10_step, wmap_run_land_focus, wmap_land_effect_10_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_10_wait_idle_02, g_wmap_land_effect_10_step, wmap_land_effect_10_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_10_step_03, g_wmap_land_effect_10_step, wmap_land_effect_10_run_timeline, wmap_land_effect_10_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_10_wait_idle_04, g_wmap_land_effect_10_step, wmap_land_effect_10_end)

WMAP_STEP_ADVANCE(wmap_land_effect_10_end, g_wmap_land_effect_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_timeline, D_800D5EF8, 0x16, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_10_timeline_reset, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void wmap_land_effect_10_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_play_sound(0x26, 0x80);
    wmap_start_sequence(wmap_land_effect_10_run_sequence_2);
    g_wmap_land_effect_10_timeline_timer = 0x20;
    g_wmap_land_effect_10_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_10_timeline_wait_02, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_10_timeline_step_03, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer,
                         wmap_land_effect_10_run_sequence_5, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_10_timeline_wait_04, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_10_timeline_step_05, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer,
                         wmap_land_effect_10_run_sequence_4, 0x28)

WMAP_STEP_WAIT(wmap_land_effect_10_timeline_wait_06, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

/** @brief Register world-map callbacks, seed a mode value, and advance step counters. */
void wmap_land_effect_10_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_10_run_sequence_7);
    wmap_start_map_tint(0x602030);
    g_wmap_backdrop_target_level = 4;
    wmap_start_sequence(wmap_land_effect_10_run_sequence_1);
    g_wmap_land_effect_10_timeline_timer = 8;
    g_wmap_land_effect_10_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_10_timeline_wait_08, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void wmap_land_effect_10_timeline_step_09(void)
{
    D_801ADAE0 = 1;
    wmap_start_sequence(wmap_land_effect_10_run_sequence_11);
    g_wmap_land_effect_10_timeline_timer = 0x3C;
    g_wmap_land_effect_10_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_10_timeline_wait_10, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_10_timeline_step_11, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer,
                         wmap_land_effect_10_run_sequence_9, 0x32)

WMAP_STEP_WAIT(wmap_land_effect_10_timeline_wait_12, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_10_timeline_step_13, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer,
                             wmap_land_effect_10_run_sequence_10, wmap_land_effect_10_run_sequence_3, 0x28)

WMAP_STEP_WAIT(wmap_land_effect_10_timeline_wait_14, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_10_timeline_step_15, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer,
                         wmap_land_effect_10_run_sequence_8, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_10_timeline_wait_16, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_10_timeline_step_17, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer,
                         wmap_land_effect_10_run_sequence_6, 0xA0)

WMAP_STEP_WAIT(wmap_land_effect_10_timeline_wait_18, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

/** @brief Set the drawing color and world-map value, then start a 26-tick delay. */
void wmap_land_effect_10_timeline_step_19(void)
{
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0xF;
    g_wmap_land_effect_10_timeline_timer = 0x1A;
    g_wmap_land_effect_10_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_10_timeline_wait_20, g_wmap_land_effect_10_timeline_step, g_wmap_land_effect_10_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_10_timeline_finish, g_wmap_land_effect_10_timeline_step, D_80139290, D_8011D510, D_8011D530, D_8011D4FC)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_sequence_1, D_800D5F50, 0x4, g_wmap_land_effect_10_sequence_1_step, g_wmap_land_effect_10_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_10_sequence_1_reset, g_wmap_land_effect_10_sequence_1_step, g_wmap_land_effect_10_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_10_sequence_1_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.target_shade = 0x81;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade = 1;
    g_wmap_land_effect_10_sequence_1_timer = 0x28;
    g_wmap_land_effect_10_sequence_1_step += 1;
    wmap_land_effect_10_sequence_1_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_10_sequence_1_step_02, g_wmap_land_effect_10_sequence_1_step, g_wmap_land_effect_10_sequence_1_timer, D_800D9318,
                              D_801399A8, g_wmap_focus_screen_position, 0x13, 0x1E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_10_sequence_1_end, g_wmap_land_effect_10_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_sequence_2, D_800D5F60, 0x6, g_wmap_land_effect_10_sequence_2_step, g_wmap_land_effect_10_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_10_sequence_2_reset, g_wmap_land_effect_10_sequence_2_step, g_wmap_land_effect_10_sequence_2_timer)

/**
 * @brief Arm the world-map sprite actor, set its wait, advance the step, and run the draw handler.
 */
void wmap_land_effect_10_sequence_2_step_01(void)
{
    D_801399CC = &D_8011D538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.sequence = 1;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 2;
    D_800D93C8.resource_index = 0;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.shade = 1;
    g_wmap_land_effect_10_sequence_2_timer = 0x60;
    g_wmap_land_effect_10_sequence_2_step += 1;
    wmap_land_effect_10_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_10_sequence_2_step_02, g_wmap_land_effect_10_sequence_2_step, g_wmap_land_effect_10_sequence_2_timer, D_800D93C8,
                              D_801399C8, g_wmap_focus_screen_position, 0x13, 0x2, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_10_sequence_2_step_03(void)
{
    D_800D93C8.shade_step = 4;
    D_800D93C8.target_shade = 0;
    g_wmap_land_effect_10_sequence_2_timer = 0x20;
    g_wmap_land_effect_10_sequence_2_step += 1;
    wmap_land_effect_10_sequence_2_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_10_sequence_2_step_04, g_wmap_land_effect_10_sequence_2_step, g_wmap_land_effect_10_sequence_2_timer, D_800D93C8,
                              D_801399C8, g_wmap_focus_screen_position, 0x13, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_10_sequence_2_end, g_wmap_land_effect_10_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_sequence_3, D_800D5F78, 0x6, g_wmap_land_effect_10_sequence_3_step, g_wmap_land_effect_10_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_10_sequence_3_reset, g_wmap_land_effect_10_sequence_3_step, g_wmap_land_effect_10_sequence_3_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_10_sequence_3_step_01(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 0x10;
    D_800D9344.target_shade = 0x81;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.shade = 1;
    g_wmap_land_effect_10_sequence_3_timer = 0xA0;
    g_wmap_land_effect_10_sequence_3_step += 1;
    wmap_land_effect_10_sequence_3_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_10_sequence_3_step_02, g_wmap_land_effect_10_sequence_3_step, g_wmap_land_effect_10_sequence_3_timer, D_800D9344,
                              D_801399B0, g_wmap_focus_screen_position, 0x17, 0xA, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_10_sequence_3_step_03(void)
{
    D_800D9344.shade_step = 8;
    D_800D9344.target_shade = 0;
    g_wmap_land_effect_10_sequence_3_timer = 0x10;
    g_wmap_land_effect_10_sequence_3_step += 1;
    wmap_land_effect_10_sequence_3_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_10_sequence_3_step_04, g_wmap_land_effect_10_sequence_3_step, g_wmap_land_effect_10_sequence_3_timer, D_800D9344,
                              D_801399B0, g_wmap_focus_screen_position, 0x17, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_10_sequence_3_end, g_wmap_land_effect_10_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_sequence_4, D_800D5F90, 0x6, g_wmap_land_effect_10_sequence_4_step, g_wmap_land_effect_10_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_10_sequence_4_reset, g_wmap_land_effect_10_sequence_4_step, g_wmap_land_effect_10_sequence_4_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_10_sequence_4_step_02(void)
{
    func_8006A2FC(D_800D99F8, D_80139AE8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_10_sequence_4_timer == 0)
    {
        g_wmap_land_effect_10_sequence_4_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_10_sequence_4_step_03(void)
{
    g_wmap_land_effect_10_sequence_4_timer = 0x20;
    D_80139280[15] = -1;
    g_wmap_land_effect_10_sequence_4_step += 1;
    wmap_land_effect_10_sequence_4_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_10_sequence_4_step_04(void)
{
    func_8006A2FC(D_800D99F8, D_80139AE8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_10_sequence_4_timer == 0)
    {
        g_wmap_land_effect_10_sequence_4_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_10_sequence_4_end, g_wmap_land_effect_10_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_sequence_5, D_800D5FA8, 0x6, g_wmap_land_effect_10_sequence_5_step, g_wmap_land_effect_10_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_10_sequence_5_reset, g_wmap_land_effect_10_sequence_5_step, g_wmap_land_effect_10_sequence_5_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_10_sequence_5_step_02(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x14, 0, 0x7F, 0x1, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--g_wmap_land_effect_10_sequence_5_timer == 0)
    {
        g_wmap_land_effect_10_sequence_5_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_10_sequence_5_step_03(void)
{
    g_wmap_land_effect_10_sequence_5_timer = 0x20;
    D_80139280[25] = -1;
    g_wmap_land_effect_10_sequence_5_step += 1;
    wmap_land_effect_10_sequence_5_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_10_sequence_5_step_04(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x14, 0, 0x7F, 0x1, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--g_wmap_land_effect_10_sequence_5_timer == 0)
    {
        g_wmap_land_effect_10_sequence_5_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_10_sequence_5_end, g_wmap_land_effect_10_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_sequence_6, D_800D5FC0, 0x4, g_wmap_land_effect_10_sequence_6_step, g_wmap_land_effect_10_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_10_sequence_6_reset, g_wmap_land_effect_10_sequence_6_step, g_wmap_land_effect_10_sequence_6_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_10_sequence_6_step_01(void)
{
    D_801399BC = D_8011F538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 1;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 8;
    D_800D9370.resource_index = 0;
    D_800D9370.target_shade = 0x80;
    D_800D9370.shade = 0;
    g_wmap_land_effect_10_sequence_6_timer = 0xBC;
    g_wmap_land_effect_10_sequence_6_step += 1;
    wmap_land_effect_10_sequence_6_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_10_sequence_6_step_02, g_wmap_land_effect_10_sequence_6_step, g_wmap_land_effect_10_sequence_6_timer, D_800D9370,
                              D_801399B8, g_wmap_focus_screen_position, 0x17, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_10_sequence_6_end, g_wmap_land_effect_10_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_sequence_7, D_800D5FD0, 0x4, g_wmap_land_effect_10_sequence_7_step, g_wmap_land_effect_10_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_10_sequence_7_reset, g_wmap_land_effect_10_sequence_7_step, g_wmap_land_effect_10_sequence_7_timer)

WMAP_STEP_DROP_START(wmap_land_effect_10_sequence_7_step_01, g_wmap_land_effect_10_sequence_7_step, g_wmap_land_effect_10_sequence_7_timer, D_801B24A0,
                     D_80139258, D_801B2650, D_80182DE8, 0x80, 0xAFC8, 0x40, wmap_land_effect_10_sequence_7_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_10_sequence_7_end, g_wmap_land_effect_10_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_sequence_8, D_800D5FE0, 0x4, g_wmap_land_effect_10_sequence_8_step, g_wmap_land_effect_10_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_10_sequence_8_reset, g_wmap_land_effect_10_sequence_8_step, g_wmap_land_effect_10_sequence_8_timer)

WMAP_STEP_DROP_START(wmap_land_effect_10_sequence_8_step_01, g_wmap_land_effect_10_sequence_8_step, g_wmap_land_effect_10_sequence_8_timer, D_801B24A8,
                     D_80139258, D_801B2478, D_80182DEC, 0x80, 0xAFC8, 0x80, wmap_land_effect_10_sequence_8_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_10_sequence_8_end, g_wmap_land_effect_10_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_sequence_9, D_800D5FF0, 0x6, g_wmap_land_effect_10_sequence_9_step, g_wmap_land_effect_10_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_10_sequence_9_reset, g_wmap_land_effect_10_sequence_9_step, g_wmap_land_effect_10_sequence_9_timer)

/** @brief World-map step: reset the sprite counters and advance to the next handler. */
void wmap_land_effect_10_sequence_9_step_01(void)
{
    D_801B2468 = 1;
    D_801B24B4 = 0;
    D_80182DE4 = 0;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    g_wmap_land_effect_10_sequence_9_timer = 0x80;
    g_wmap_land_effect_10_sequence_9_step += 1;
    wmap_land_effect_10_sequence_9_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_10_sequence_9_step_03, g_wmap_land_effect_10_sequence_9_step, g_wmap_land_effect_10_sequence_9_timer, 0x40,
                    wmap_land_effect_10_sequence_9_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_10_sequence_9_end, g_wmap_land_effect_10_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_sequence_10, D_800D6008, 0x4, g_wmap_land_effect_10_sequence_10_step, g_wmap_land_effect_10_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_10_sequence_10_reset, g_wmap_land_effect_10_sequence_10_step, g_wmap_land_effect_10_sequence_10_timer)

WMAP_STEP_ADVANCE(wmap_land_effect_10_sequence_10_end, g_wmap_land_effect_10_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_10_run_sequence_11, D_800D6018, 0x4, g_wmap_land_effect_10_sequence_11_step, g_wmap_land_effect_10_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_10_sequence_11_reset, g_wmap_land_effect_10_sequence_11_step, g_wmap_land_effect_10_sequence_11_timer)

/** @brief Draw the effect, select texture page 37, and update the sequence countdown. */
void wmap_land_effect_10_sequence_11_step_02(void)
{
    s32 value;

    /* The spark config lives 30 words into the shared effect parameter block. */
    func_8008ECF8(100, 150, D_8011CF28, (WmapSparkConfig*)&D_80139280[30]);
    func_8006534C(0x25, 2);
    value = g_wmap_land_effect_10_sequence_11_timer - 1;
    g_wmap_land_effect_10_sequence_11_timer = value;
    if (value == 0)
    {
        g_wmap_land_effect_10_sequence_11_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_10_sequence_11_end, g_wmap_land_effect_10_sequence_11_step)
