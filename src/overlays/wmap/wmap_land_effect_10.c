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

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapSparkResource;

void func_80090B84(void);
void func_80090D7C(void);
void func_8008FA40(void);
void func_800915C8(void);
void func_8008FDF0(void);
void func_8008FE2C(void);
s32 func_8008FEC4(s32 arg0);
void func_8008FE70(void);
void func_8008FEAC(void);
s32 func_8009061C(s32 arg0);
s32 func_80090CEC(s32 arg0);
s32 func_80090AF4(s32 arg0);
s32 func_80091088(s32 arg0);
s32 func_80090478(s32 arg0);
s32 func_80091538(s32 arg0);
s32 func_80091334(s32 arg0);
s32 func_80091490(s32 arg0);
s32 func_80090888(s32 arg0);
s32 func_800911E0(s32 arg0);
s32 func_80090EE4(s32 arg0);
void func_80090588(void);
void func_8009072C(void);
void func_800907F4(void);
void func_80090998(void);
void func_80090A60(void);
void func_80090C50(void);
void func_80090E48(void);
void func_80090FF4(void);
void func_8008ECF8(s32 first, s32 end, WmapSparkParticle *particles, WmapSparkConfig *config);

/**
 * @brief Spawn, move, and draw a batch of falling sparks, then fade them out as sprites.
 * @param first First particle index to process.
 * @param end One past the last particle index to process.
 * @param particles Particle position/velocity array indexed by particle.
 * @param config Emitter tuning block.
 */
void func_8008ECF8(s32 first, s32 end, WmapSparkParticle *particles, WmapSparkConfig *config)
{

extern s32 D_800D9230;
extern WmapSparkActor D_800D9268[];
extern s32 D_8011CF74;

extern WmapSparkResource D_80139988[];
extern WmapSparkSlot D_801AFBD0[];

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
void func_8008F218(void)
{
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

extern s32 *D_80139280;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern s32 D_801B0FD0;
extern u8 D_8011D538[];
extern s32 D_801B2A90;
extern s32 D_801B2A94;

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
        D_801AFBD0[i + D_80139280[0x11]].field_00 = 0;
        D_80139988[i + 44].field_04 = D_8011D538;
    }
    D_801B2A94 = 40;
    D_801B2A90++;
    func_80090B84();
}

/** @brief Initialize the effect actors, resources, and evenly spaced angles. */
void func_8008F304(void)
{
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

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D95D8[];
extern WmapMotion D_801AFD60[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern s32 D_801B0FD0;
extern u8 D_8011D538[];
extern s32 D_801B2A98;
extern s32 D_801B2A9C;

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
        D_80139988[i + 20].resource = D_8011D538;
        D_800D95D8[i].field_06 = 15;
        D_800D95D8[i].field_0E = 3;
        D_800D95D8[i].field_10 = -1;
        D_800D95D8[i].field_26 = 1;
        D_800D95D8[i].field_02 = 0;
        D_800D95D8[i].field_22 = 0;
        D_800D95D8[i].field_24 = 127;
        i++;
    } while (i < 20);
    D_801B2A9C = 40;
    D_801B2A98++;
    func_80090D7C();
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8008F430(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2AB4;
extern s32 D_801B2AB0;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2AAC;
extern s32 D_801B2AA8;

    MATRIX m;
    s32 x;

    x = D_801B2650[2] - 0xDAC;
    D_801B2650[2] = x;
    if (x < 0x2710)
    {
        D_801B2650[2] = 0x2710;
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
    if (--D_801B2AAC == 0)
    {
        D_801B2AA8 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8008F530(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2AB4;
extern s32 D_801B2AB0;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2AAC;
extern s32 D_801B2AA8;

    MATRIX m;
    s32 x;

    x = D_801B2478[2] - 0xDAC;
    D_801B2478[2] = x;
    if (x < 0x2710)
    {
        D_801B2478[2] = 0x2710;
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
    if (--D_801B2AB4 == 0)
    {
        D_801B2AB0 += 1;
    }
}

/** @brief Draw two oscillating effect layers and update their intensity. */
void func_8008F630(void)
{
extern s8 D_80051B4C[];
extern void *D_8011CF24;
extern VECTOR g_wmap_camera_translation;
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B24B4;
extern s32 D_801B2AB8;
extern s32 D_801B2ABC;

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
    remaining = D_801B2ABC - 1;
    D_801B2ABC = remaining;
    if (remaining == 0)
    {
        D_801B2AB8 += 1;
    }
}

/** @brief Draw two oscillating effect layers and reduce their shared intensity. */
void func_8008F7D4(void)
{
extern s8 D_80051B4C[];
extern void *D_8011CF24;
extern VECTOR g_wmap_camera_translation;
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B24B4;
extern s32 D_801B2AB8;
extern s32 D_801B2ABC;

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
    remaining = D_801B2ABC - 1;
    D_801B2ABC = remaining;
    if (remaining == 0)
    {
        D_801B2AB8 += 1;
    }
}

/** @brief Initialize twenty-four actors and advance to their update step. */
void func_8008F970(void)
{
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

extern WmapConfigA D_800D9268[];
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_80121538[];
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_801B2AC0;
extern s32 D_801B2AC4;

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
    D_801B2AC4 = D_8013923C;
    D_801B2AC0++;
    func_8008FA40();
}

/** @brief Spawn and draw spiraling particles until the effect finishes. */
void func_8008FA40(void)
{
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

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D9268[];
extern WmapResource D_80139988[];
extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801B0080[];
extern s32 D_80139234;
extern s32 D_801B2AC0;
extern s32 D_801B2AC4;
extern s32 rand(void);

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
        D_801B2AC4 = 1;
    }
    value = D_801B2AC4 - 1;
    D_801B2AC4 = value;
    if (value == 0)
    {
        D_801B2AC0++;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8008FC40(void)
{
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

extern s32 *D_80139280;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern s32 D_801B0FD0;
extern s32 D_801B2AC8;
extern s32 D_801B2ACC;
extern u8 D_80121538[];

    s32 i;

    for (i = 100; i < 150; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].field_04 = D_80121538;
    }
    D_801B2ACC = 200;
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
    D_801B2AC8++;
    func_800915C8();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008FD1C(s32 arg0)
{
extern u32 D_801B2A68;
extern s32 D_801B2A6C;
extern void (*D_800D5EE0[])(void);
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A68 = 1;
        D_801B2A6C = 1;
        return 1;
    }

    if (D_801B2A68 < 0x6)
    {
        D_800D5EE0[D_801B2A68]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008FD94(void)
{
extern u32 D_801B2A68;
extern s32 D_801B2A6C;
extern void (*D_800D5EE0[])(void);
extern s32 D_8013B20C;

    D_801B2A68 = 1;
    D_801B2A6C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008FDAC(void)
{
extern u32 D_801B2A68;
extern s32 D_801B2A6C;
extern void (*D_800D5EE0[])(void);
extern s32 D_8013B20C;

    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2A68 += 1;
    func_8008FDF0();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008FDF0(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2A68;

    if (D_8013B20C == 0)
    {
        D_801B2A68 += 1;
        func_8008FE2C();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008FE2C(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2A68;
extern void func_8008FE2C(void);

    wmap_start_sequence(func_8008FEC4);
    D_8013B20C = 1;
    D_801B2A68 += 1;
    func_8008FE70();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008FE70(void)
{
extern s32 D_801B2A68;
extern s32 D_8013B20C;

    if (D_8013B20C == 0)
    {
        D_801B2A68 += 1;
        func_8008FEAC();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008FEAC(void)
{
extern s32 D_801B2A68;
extern s32 D_8013B20C;
extern void func_8008FEAC(void);

    D_801B2A68 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008FEC4(s32 arg0)
{
extern u32 D_801B2A70;
extern s32 D_801B2A74;
extern void (*D_800D5EF8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A70 = 1;
        D_801B2A74 = 1;
        return 1;
    }

    if (D_801B2A70 < 0x16)
    {
        D_800D5EF8[D_801B2A70]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008FF3C(void)
{
extern u32 D_801B2A70;
extern s32 D_801B2A74;
extern void (*D_800D5EF8[])(void);

    D_801B2A70 = 1;
    D_801B2A74 = 1;
}

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void func_8008FF54(void)
{
extern s32 D_8013B208;
extern s32 D_801B2A70;
extern s32 D_801B2A74;

    D_8013B208 = 1;
    wmap_play_sound(0x26, 0x80);
    wmap_start_sequence(func_8009061C);
    D_801B2A74 = 0x20;
    D_801B2A70 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008FFA8(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008FFDC(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    wmap_start_sequence(func_80090CEC);
    D_801B2A74 = 0x2;
    D_801B2A70 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80090018(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009004C(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    wmap_start_sequence(func_80090AF4);
    D_801B2A74 = 0x28;
    D_801B2A70 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80090088(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/** @brief Register world-map callbacks, seed a mode value, and advance step counters. */
void func_800900BC(void)
{
extern s32 D_801B2A70;
extern s32 D_801B2A74;

    wmap_start_sequence(func_80091088);
    wmap_start_map_tint(0x602030);
    g_wmap_backdrop_target_level = 4;
    wmap_start_sequence(func_80090478);
    D_801B2A74 = 8;
    D_801B2A70 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009011C(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_80090150(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    D_801ADAE0 = 1;
    wmap_start_sequence(func_80091538);
    D_801B2A74 = 0x3C;
    D_801B2A70 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80090198(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800901CC(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    wmap_start_sequence(func_80091334);
    D_801B2A74 = 0x32;
    D_801B2A70 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80090208(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009023C(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    wmap_start_sequence(func_80091490);
    wmap_start_sequence(func_80090888);
    D_801B2A74 = 0x28;
    D_801B2A70 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80090284(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800902B8(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    wmap_start_sequence(func_800911E0);
    D_801B2A74 = 0x2;
    D_801B2A70 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800902F4(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80090328(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    wmap_start_sequence(func_80090EE4);
    D_801B2A74 = 0xA0;
    D_801B2A70 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80090364(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/** @brief Set the drawing color and world-map value, then start a 26-tick delay. */
void func_80090398(void)
{
extern s32 D_801B2A70;
extern s32 D_801B2A74;

    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0xF;
    D_801B2A74 = 0x1A;
    D_801B2A70 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800903E0(void)
{
extern s32 D_801B2A74;
extern s32 D_801B2A70;

    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

void func_80090414(void)
{
/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern s32 D_8013B20C;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2A70;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2A70 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80090478(s32 arg0)
{
extern u32 D_801B2A78;
extern s32 D_801B2A7C;
extern void (*D_800D5F50[])(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A78 = 1;
        D_801B2A7C = 1;
        return 1;
    }

    if (D_801B2A78 < 0x4)
    {
        D_800D5F50[D_801B2A78]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800904F0(void)
{
extern u32 D_801B2A78;
extern s32 D_801B2A7C;
extern void (*D_800D5F50[])(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    D_801B2A78 = 1;
    D_801B2A7C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80090508(void)
{
extern u32 D_801B2A78;
extern s32 D_801B2A7C;
extern void (*D_800D5F50[])(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x24] = 1;
    D_801B2A7C = 0x28;
    D_801B2A78 += 1;
    func_80090588();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80090588(void)
{
extern u32 D_801B2A78;
extern s32 D_801B2A7C;
extern void (*D_800D5F50[])(void);
extern void func_80090588(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9318, D_801399A8);
    wmap_draw_actor_sprite(D_800D9318, g_wmap_focus_screen_position, 0x13, 0x1E, 0);
    if (--D_801B2A7C == 0)
    {
        D_801B2A78 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80090604(void)
{
extern s32 D_801B2A78;

    D_801B2A78 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009061C(s32 arg0)
{
extern u32 D_801B2A80;
extern s32 D_801B2A84;
extern void (*D_800D5F60[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A80 = 1;
        D_801B2A84 = 1;
        return 1;
    }

    if (D_801B2A80 < 0x6)
    {
        D_800D5F60[D_801B2A80]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80090694(void)
{
extern u32 D_801B2A80;
extern s32 D_801B2A84;
extern void (*D_800D5F60[])(void);

    D_801B2A80 = 1;
    D_801B2A84 = 1;
}

/**
 * @brief Arm the world-map sprite actor, set its wait, advance the step, and run the draw handler.
 */
void func_800906AC(void)
{
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
} __attribute__((aligned(4))) WmapConfigA;

extern WmapConfigA D_800D93C8;
extern u8 D_8011D538;
extern void *D_801399CC;
extern s32 D_801B2A80;
extern s32 D_801B2A84;

    D_801399CC = &D_8011D538;
    D_800D93C8.field_06 = 0xF;
    D_800D93C8.field_0E = 1;
    D_800D93C8.field_10 = -1;
    D_800D93C8.field_26 = 2;
    D_800D93C8.field_02 = 0;
    D_800D93C8.field_22 = 0x81;
    D_800D93C8.field_24 = 1;
    D_801B2A84 = 0x60;
    D_801B2A80 += 1;
    func_8009072C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009072C(void)
{
extern s16 D_800D93C8[];
extern s32 D_801B2A84;
extern s32 D_801B2A80;
extern u8 D_801399C8[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D93C8, D_801399C8);
    wmap_draw_actor_sprite(D_800D93C8, g_wmap_focus_screen_position, 0x13, 0x2, 0);
    if (--D_801B2A84 == 0)
    {
        D_801B2A80 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800907A8(void)
{
extern s16 D_800D93C8[];
extern s32 D_801B2A84;
extern s32 D_801B2A80;
extern u8 D_801399C8[];
extern s32 g_wmap_focus_screen_position;

    D_800D93C8[19] = 4;
    D_800D93C8[17] = 0;
    D_801B2A84 = 0x20;
    D_801B2A80 += 1;
    func_800907F4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800907F4(void)
{
extern void func_800907F4(void);
extern s16 D_800D93C8[];
extern s32 D_801B2A84;
extern s32 D_801B2A80;
extern u8 D_801399C8[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D93C8, D_801399C8);
    wmap_draw_actor_sprite(D_800D93C8, g_wmap_focus_screen_position, 0x13, 0x2, 0);
    if (--D_801B2A84 == 0)
    {
        D_801B2A80 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80090870(void)
{
extern s32 D_801B2A80;

    D_801B2A80 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80090888(s32 arg0)
{
extern u32 D_801B2A88;
extern s32 D_801B2A8C;
extern void (*D_800D5F78[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A88 = 1;
        D_801B2A8C = 1;
        return 1;
    }

    if (D_801B2A88 < 0x6)
    {
        D_800D5F78[D_801B2A88]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80090900(void)
{
extern u32 D_801B2A88;
extern s32 D_801B2A8C;
extern void (*D_800D5F78[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    D_801B2A88 = 1;
    D_801B2A8C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80090918(void)
{
extern u32 D_801B2A88;
extern s32 D_801B2A8C;
extern void (*D_800D5F78[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 0x10;
    *(s16*)&D_800D9344[0x22] = 0x81;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x24] = 1;
    D_801B2A8C = 0xA0;
    D_801B2A88 += 1;
    func_80090998();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80090998(void)
{
extern u32 D_801B2A88;
extern s32 D_801B2A8C;
extern void (*D_800D5F78[])(void);
extern void func_80090998(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, g_wmap_focus_screen_position, 0x17, 0xA, 0);
    if (--D_801B2A8C == 0)
    {
        D_801B2A88 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80090A14(void)
{
extern s16 D_800D9344[];
extern s32 D_801B2A8C;
extern s32 D_801B2A88;
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    D_800D9344[19] = 8;
    D_800D9344[17] = 0;
    D_801B2A8C = 0x10;
    D_801B2A88 += 1;
    func_80090A60();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80090A60(void)
{
extern void func_80090A60(void);
extern s16 D_800D9344[];
extern s32 D_801B2A8C;
extern s32 D_801B2A88;
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, g_wmap_focus_screen_position, 0x17, 0xA, 0);
    if (--D_801B2A8C == 0)
    {
        D_801B2A88 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80090ADC(void)
{
extern s32 D_801B2A88;

    D_801B2A88 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80090AF4(s32 arg0)
{
extern u32 D_801B2A90;
extern s32 D_801B2A94;
extern void (*D_800D5F90[])(void);
extern u8 D_800D99F8[];
extern u8 D_80139AE8[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A90 = 1;
        D_801B2A94 = 1;
        return 1;
    }

    if (D_801B2A90 < 0x6)
    {
        D_800D5F90[D_801B2A90]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80090B6C(void)
{
extern u32 D_801B2A90;
extern s32 D_801B2A94;
extern void (*D_800D5F90[])(void);
extern u8 D_800D99F8[];
extern u8 D_80139AE8[];
extern s32 D_80139280;

    D_801B2A90 = 1;
    D_801B2A94 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80090B84(void)
{
extern u32 D_801B2A90;
extern s32 D_801B2A94;
extern void (*D_800D5F90[])(void);
extern u8 D_800D99F8[];
extern u8 D_80139AE8[];
extern s32 D_80139280;

    func_8006A2FC(D_800D99F8, D_80139AE8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2A94 == 0)
    {
        D_801B2A90 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80090C08(void)
{
extern s32* D_80139280;
extern s32 D_801B2A94;
extern s32 D_801B2A90;
extern u8 D_800D99F8[];
extern u8 D_80139AE8[];

    D_801B2A94 = 0x20;
    D_80139280[15] = -1;
    D_801B2A90 += 1;
    func_80090C50();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80090C50(void)
{
extern void func_80090C50(void);
extern s32* D_80139280;
extern s32 D_801B2A94;
extern s32 D_801B2A90;
extern u8 D_800D99F8[];
extern u8 D_80139AE8[];

    func_8006A2FC(D_800D99F8, D_80139AE8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2A94 == 0)
    {
        D_801B2A90 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80090CD4(void)
{
extern s32 D_801B2A90;

    D_801B2A90 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80090CEC(s32 arg0)
{
extern u32 D_801B2A98;
extern s32 D_801B2A9C;
extern void (*D_800D5FA8[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A98 = 1;
        D_801B2A9C = 1;
        return 1;
    }

    if (D_801B2A98 < 0x6)
    {
        D_800D5FA8[D_801B2A98]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80090D64(void)
{
extern u32 D_801B2A98;
extern s32 D_801B2A9C;
extern void (*D_800D5FA8[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    D_801B2A98 = 1;
    D_801B2A9C = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80090D7C(void)
{
extern u32 D_801B2A98;
extern s32 D_801B2A9C;
extern void (*D_800D5FA8[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    func_8006A2FC(D_800D95D8, D_80139A28, 0x14, 0, 0x7F, 0x1, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2A9C == 0)
    {
        D_801B2A98 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80090E00(void)
{
extern s32* D_80139280;
extern s32 D_801B2A9C;
extern s32 D_801B2A98;
extern u8 D_800D95D8[];
extern u8 D_80139A28[];

    D_801B2A9C = 0x20;
    D_80139280[25] = -1;
    D_801B2A98 += 1;
    func_80090E48();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80090E48(void)
{
extern void func_80090E48(void);
extern s32* D_80139280;
extern s32 D_801B2A9C;
extern s32 D_801B2A98;
extern u8 D_800D95D8[];
extern u8 D_80139A28[];

    func_8006A2FC(D_800D95D8, D_80139A28, 0x14, 0, 0x7F, 0x1, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2A9C == 0)
    {
        D_801B2A98 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80090ECC(void)
{
extern s32 D_801B2A98;

    D_801B2A98 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80090EE4(s32 arg0)
{
extern u32 D_801B2AA0;
extern s32 D_801B2AA4;
extern void (*D_800D5FC0[])(void);
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AA0 = 1;
        D_801B2AA4 = 1;
        return 1;
    }

    if (D_801B2AA0 < 0x4)
    {
        D_800D5FC0[D_801B2AA0]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80090F5C(void)
{
extern u32 D_801B2AA0;
extern s32 D_801B2AA4;
extern void (*D_800D5FC0[])(void);
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;

    D_801B2AA0 = 1;
    D_801B2AA4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80090F74(void)
{
extern u32 D_801B2AA0;
extern s32 D_801B2AA4;
extern void (*D_800D5FC0[])(void);
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;

    D_801399BC = D_8011F538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0xE] = 1;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 8;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0x22] = 0x80;
    *(s16*)&D_800D9370[0x24] = 0;
    D_801B2AA4 = 0xBC;
    D_801B2AA0 += 1;
    func_80090FF4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80090FF4(void)
{
extern u32 D_801B2AA0;
extern s32 D_801B2AA4;
extern void (*D_800D5FC0[])(void);
extern void func_80090FF4(void);
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, g_wmap_focus_screen_position, 0x17, 0xA, 0);
    if (--D_801B2AA4 == 0)
    {
        D_801B2AA0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80091070(void)
{
extern s32 D_801B2AA0;

    D_801B2AA0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80091088(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2AA8;
extern s32 D_801B2AAC;
extern void (*D_800D5FD0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AA8 = 1;
        D_801B2AAC = 1;
        return 1;
    }

    if (D_801B2AA8 < 0x4)
    {
        D_800D5FD0[D_801B2AA8]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80091100(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2AA8;
extern s32 D_801B2AAC;
extern void (*D_800D5FD0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    D_801B2AA8 = 1;
    D_801B2AAC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80091118(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2AA8;
extern s32 D_801B2AAC;
extern void (*D_800D5FD0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2AAC = 0x40;
    D_801B2AA8 += 1;
    func_8008F430();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800911C8(void)
{
extern s32 D_801B2AA8;

    D_801B2AA8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800911E0(s32 arg0)
{
extern u32 D_801B2AB0;
extern s32 D_801B2AB4;
extern void (*D_800D5FE0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AB0 = 1;
        D_801B2AB4 = 1;
        return 1;
    }

    if (D_801B2AB0 < 0x4)
    {
        D_800D5FE0[D_801B2AB0]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80091258(void)
{
extern u32 D_801B2AB0;
extern s32 D_801B2AB4;
extern void (*D_800D5FE0[])(void);

    D_801B2AB0 = 1;
    D_801B2AB4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80091270(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern s32 D_801B2AB4;
extern u32 D_801B2AB0;

    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2AB4 = 0x80;
    D_801B2AB0 += 1;
    func_8008F530();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009131C(void)
{
extern s32 D_801B2AB0;

    D_801B2AB0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80091334(s32 arg0)
{
extern u32 D_801B2AB8;
extern s32 D_801B2ABC;
extern void (*D_800D5FF0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AB8 = 1;
        D_801B2ABC = 1;
        return 1;
    }

    if (D_801B2AB8 < 0x6)
    {
        D_800D5FF0[D_801B2AB8]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800913AC(void)
{
extern u32 D_801B2AB8;
extern s32 D_801B2ABC;
extern void (*D_800D5FF0[])(void);

    D_801B2AB8 = 1;
    D_801B2ABC = 1;
}

/** @brief World-map step: reset the sprite counters and advance to the next handler. */
void func_800913C4(void)
{
extern s32 D_801B2468;
extern s32 D_801B24B4;
extern s32 D_80182DE4;
extern s16 D_801B2490[];
extern s16 D_801B2498[];
extern s32 D_801B2ABC;
extern s32 D_801B2AB8;

    D_801B2468 = 1;
    D_801B24B4 = 0;
    D_80182DE4 = 0;
    D_801B2490[0] = 0;
    D_801B2490[1] = 0;
    D_801B2490[2] = 0;
    D_801B2498[0] = 0;
    D_801B2498[1] = 0;
    D_801B2498[2] = 0;
    D_801B2ABC = 0x80;
    D_801B2AB8 += 1;
    func_8008F630();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80091440(void)
{
extern s32 D_801B2AB8;
extern s32 D_801B2ABC;

    D_801B2ABC = 0x40;
    D_801B2AB8 += 1;
    func_8008F7D4();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80091478(void)
{
extern s32 D_801B2AB8;
extern s32 D_801B2ABC;

    D_801B2AB8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80091490(s32 arg0)
{
extern u32 D_801B2AC0;
extern s32 D_801B2AC4;
extern void (*D_800D6008[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AC0 = 1;
        D_801B2AC4 = 1;
        return 1;
    }

    if (D_801B2AC0 < 0x4)
    {
        D_800D6008[D_801B2AC0]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80091508(void)
{
extern u32 D_801B2AC0;
extern s32 D_801B2AC4;
extern void (*D_800D6008[])(void);

    D_801B2AC0 = 1;
    D_801B2AC4 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80091520(void)
{
extern u32 D_801B2AC0;
extern s32 D_801B2AC4;
extern void (*D_800D6008[])(void);

    D_801B2AC0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80091538(s32 arg0)
{
extern u32 D_801B2AC8;
extern s32 D_801B2ACC;
extern void (*D_800D6018[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AC8 = 1;
        D_801B2ACC = 1;
        return 1;
    }

    if (D_801B2AC8 < 0x4)
    {
        D_800D6018[D_801B2AC8]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800915B0(void)
{
extern u32 D_801B2AC8;
extern s32 D_801B2ACC;
extern void (*D_800D6018[])(void);

    D_801B2AC8 = 1;
    D_801B2ACC = 1;
}

/** @brief Draw the effect, select texture page 37, and update the sequence countdown. */
void func_800915C8(void)
{
extern WmapSparkParticle* D_8011CF28;
extern s32* D_80139280;
extern s32 D_801B2AC8;
extern s32 D_801B2ACC;

    s32 value;

    /* The spark config lives 30 words into the shared effect parameter block. */
    func_8008ECF8(100, 150, D_8011CF28, (WmapSparkConfig*)&D_80139280[30]);
    func_8006534C(0x25, 2);
    value = D_801B2ACC - 1;
    D_801B2ACC = value;
    if (value == 0)
    {
        D_801B2AC8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80091638(void)
{
extern s32 D_801B2AC8;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern s32 D_8011CF1C;
extern s32 D_801B2AFC;
extern s32 D_801B2AF8;

    D_801B2AC8 += 1;
}
