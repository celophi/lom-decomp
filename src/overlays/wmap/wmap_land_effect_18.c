#include "wmap_main.h"
#include "wmap_land_effect_18.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "sdk/rand.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

/** @brief Spark state used by the radial world-map particle effect. */
typedef struct
{
    s16 active;
    s16 angle;
    s32 delta;
    s32 radius;
    s16 timer;
    s16 unk0E;
    s16 unk10;
    s16 unk12;
} WmapEffect18Spark;

/** @brief Draw record fields touched by the radial particle effect. */
typedef struct
{
    u8 pad00[2];
    s16 unk02;
    u8 pad04[2];
    s8 unk06;
    u8 pad07[7];
    s16 unk0E;
    s16 unk10;
    u8 pad12[0x10];
    s16 unk22;
    s16 unk24;
    u8 pad26[6];
} WmapEffect18Draw;

void wmap_land_effect_18_sequence_6_step_02(void);
void wmap_land_effect_18_sequence_9_step_02(void);
void wmap_land_effect_18_sequence_10_step_02(void);
void wmap_land_effect_18_sequence_5_step_02(void);
void wmap_land_effect_18_sequence_5_step_04(void);
void wmap_land_effect_18_sequence_6_step_04(void);
void wmap_land_effect_18_sequence_7_step_02(void);
s32 wmap_land_effect_18_run_sequence_11(s32 arg0);
s32 wmap_land_effect_18_run_sequence_10(s32 arg0);
s32 wmap_land_effect_18_run_sequence_8(s32 arg0);
s32 wmap_land_effect_18_run_sequence_9(s32 arg0);
void wmap_land_effect_18_sequence_8_step_02(void);
void wmap_land_effect_18_sequence_8_step_04(void);
void wmap_land_effect_18_wait_idle_02(void);
void wmap_land_effect_18_step_03(void);
void wmap_land_effect_18_wait_idle_04(void);
void wmap_land_effect_18_end(void);

extern SVECTOR g_wmap_camera_rotation;
extern s32 g_wmap_land_effect_18_sequence_1_timer;
extern s32 D_801B24B4;
extern void *g_wmap_effect_model_pack_1;
extern s32 D_801B2468;
extern s32 g_wmap_land_effect_18_sequence_2_timer;
extern s32 D_801B24B0;
extern s32 D_80139878;
extern s32 g_wmap_land_effect_18_sequence_3_timer;
extern s32 D_801B2488;
extern s32 D_801B246C;
extern s32 D_80182DC8;
extern s32 g_wmap_land_effect_18_sequence_4_timer;
extern s32 D_801B2470;
extern s32 D_801B2474;
extern s32 D_80139980;
extern s32 g_wmap_land_effect_18_sequence_6_timer;
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
extern s32 g_wmap_land_effect_18_sequence_9_timer;
extern u8 g_wmap_animation_bank_1[];
extern s32 g_wmap_land_effect_18_sequence_10_timer;
extern s32 D_80182DE4;
extern s32 g_wmap_land_effect_18_sequence_11_timer;
extern void (*D_800D4CC4[])(void);
extern void (*D_800D4CDC[])(void);
extern void (*D_800D4D04[])(void);
extern void (*D_800D4D24[])(void);
extern s32 g_wmap_land_effect_18_sequence_5_timer;
extern void (*D_800D4D3C[])(void);
extern void (*D_800D4D54[])(void);
extern void wmap_land_effect_18_sequence_6_step_04(void);
extern s32 g_wmap_land_effect_18_sequence_7_timer;
extern void (*D_800D4D6C[])(void);
extern s32 g_wmap_land_effect_18_timeline_timer;
extern void (*D_800D4C54[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_18_sequence_8_timer;
extern void (*D_800D4D7C[])(void);
extern SVECTOR D_801398C8;
extern s16 D_801398CC;
extern void (*D_800D4D94[])(void);
extern void (*D_800D4DA4[])(void);
extern void (*D_800D4DB4[])(void);
extern s32 g_wmap_land_effect_18_timer;
extern void (*D_800D4CAC[])(void);
extern void wmap_land_effect_18_end(void);
extern u8 *g_wmap_effect_model_pack_2;
extern u32 g_wmap_land_effect_18_sequence_1_step;
extern u32 g_wmap_land_effect_18_sequence_2_step;
extern u32 g_wmap_land_effect_18_sequence_3_step;
extern u32 g_wmap_land_effect_18_sequence_4_step;
extern u8 g_wmap_animation_bank_0[];
extern u32 g_wmap_land_effect_18_sequence_6_step;
extern u32 g_wmap_land_effect_18_sequence_9_step;
extern u32 g_wmap_land_effect_18_sequence_10_step;
extern u32 g_wmap_land_effect_18_sequence_11_step;
extern u32 g_wmap_land_effect_18_sequence_5_step;
extern u32 g_wmap_land_effect_18_sequence_7_step;
extern u32 g_wmap_land_effect_18_timeline_step;
extern u32 g_wmap_land_effect_18_sequence_8_step;
extern s32 D_801B248C;
extern u32 g_wmap_land_effect_18_step;

extern VECTOR g_wmap_camera_translation;

/** @brief Map scroll position (map units) and projection scale. */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 unknown_0c;
} WmapView;

extern WmapView g_wmap_view;

extern SVECTOR D_801B2498;
extern SVECTOR D_801B2490;


extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern WmapEffect18Spark g_wmap_actor_motions[];

/** @brief Compose the effect transform, draw it, and advance its rotation and countdown. */
void wmap_land_effect_18_sequence_1_step_02(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;

    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_camera_translation);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_a_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, 0, 36, 183, 0x7A40, 1, -1);
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_1_timer - 1;
    g_wmap_effect_model_a_rotation.vz = (u16)(g_wmap_effect_model_a_rotation.vz + 320);
    g_wmap_land_effect_18_sequence_1_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_1_step++;
    }
}

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void wmap_land_effect_18_sequence_1_step_04(void)
{
    MATRIX base;
    MATRIX effect;
    s32 intensity;
    s32 remaining;

    if (D_801B24B4 != 0)
    {
        PushMatrix();
        RotMatrix(&g_wmap_camera_rotation, &base);
        TransMatrix(&base, &g_wmap_camera_translation);
        SetRotMatrix(&base);
        SetTransMatrix(&base);
        RotMatrix(&g_wmap_effect_model_a_rotation, &effect);
        TransMatrix(&effect, &g_wmap_zero_translation);
        CompMatrix(&base, &effect, &effect);
        SetRotMatrix(&effect);
        SetTransMatrix(&effect);
        wmap_draw_model_default(g_wmap_effect_model_pack_2, 0, 0x24, 0xB7, 0x7A40, 1, D_801B24B4);
        intensity = D_801B24B4 - 2;
        D_801B24B4 = intensity;
        if (intensity < 0)
        {
            D_801B24B4 = 0;
        }
        PopMatrix();
        g_wmap_effect_model_a_rotation.vz = (u16) (g_wmap_effect_model_a_rotation.vz + 0x12C);
    }
    remaining = g_wmap_land_effect_18_sequence_1_timer - 1;
    g_wmap_land_effect_18_sequence_1_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_1_step += 1;
    }
}

/** @brief Draw and brighten two rotating layers, then advance their shared countdown. */
void wmap_land_effect_18_sequence_2_step_02(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 32);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz + 16);
    PopMatrix();
    intensity = D_801B2468 + 2;
    D_801B2468 = intensity;
    if (intensity >= 129)
    {
        D_801B2468 = 128;
    }
    remaining = g_wmap_land_effect_18_sequence_2_timer - 1;
    g_wmap_land_effect_18_sequence_2_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_2_step++;
    }
}

/** @brief Draw two rotating effect layers and advance their shared countdown. */
void wmap_land_effect_18_sequence_2_step_04(void)
{
    s32 remaining;
    s32 frame;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    frame = D_801B24B0 + 1;
    D_801B24B0 = frame;
    if (frame >= 8)
    {
        D_801B24B0 = 7;
    }
    if (D_801B2468 >= 5)
    {
        D_801B2468 -= 4;
    }
    wmap_draw_model_default(g_wmap_effect_model_pack_1, D_801B24B0, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 32);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, D_801B24B0, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz + 16);
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_2_timer - 1;
    g_wmap_land_effect_18_sequence_2_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_2_step++;
    }
}

/** @brief Draw two rotating effect layers and advance their shared countdown. */
void wmap_land_effect_18_sequence_2_step_06(void)
{
    s32 remaining;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 7, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 32);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 7, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz + 16);
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_2_timer - 1;
    g_wmap_land_effect_18_sequence_2_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_2_step++;
    }
}

/** @brief Draw and fade two rotating layers, then advance their shared countdown. */
void wmap_land_effect_18_sequence_2_step_08(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 7, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 32);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 7, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz + 16);
    PopMatrix();
    intensity = D_801B2468 - 2;
    D_801B2468 = intensity;
    if (intensity < 0)
    {
        D_801B2468 = 0;
    }
    remaining = g_wmap_land_effect_18_sequence_2_timer - 1;
    g_wmap_land_effect_18_sequence_2_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_2_step++;
    }
}

/** @brief Draw the effect at successive depths and advance its countdown. */
void wmap_land_effect_18_sequence_3_step_02(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;

    switch (D_801B2488)
   
  {
    case 0:
        D_80139878 = 8000;
        break;
    case 1:
        D_80139878 = 18000;
        break;
    case 2:
        D_80139878 = 10000;
        break;
    }
    D_801B2488 += 1;
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_c_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_c_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    wmap_draw_model_default(g_wmap_effect_model_pack_2 + 0x4000, 0, 4, -1, -1, 1, -1);
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_3_timer - 1;
    g_wmap_land_effect_18_sequence_3_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_3_step += 1;
    }
}

/** @brief Compose the effect transform, draw it at alternating depths, and advance its countdown. */
void wmap_land_effect_18_sequence_3_step_04(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;

    if (g_wmap_frame_count & 1)
    {
        D_80139878 = 28000;
    }
    else
    {
        D_80139878 = 18000;
    }
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_c_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_c_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    wmap_draw_model_default(g_wmap_effect_model_pack_2 + 0x4000, 0, 4, -1, -1, 1, -1);
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_3_timer - 1;
    g_wmap_land_effect_18_sequence_3_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_3_step++;
    }
}

/** @brief Compose the effect transform, draw it at alternating depths, and advance its countdown. */
void wmap_land_effect_18_sequence_3_step_06(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;

    if (g_wmap_frame_count & 1)
    {
        D_80139878 = 28000;
    }
    else
    {
        D_80139878 = 18000;
    }
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_c_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_c_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    wmap_draw_model_default(g_wmap_effect_model_pack_2 + 0x4000, 0, 4, -1, -1, 1, D_801B246C);
    D_801B246C--;
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_3_timer - 1;
    g_wmap_land_effect_18_sequence_3_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_3_step++;
    }
}

/** @brief Advance the effect depth, fade its intensity, and update the countdown. */
void wmap_land_effect_18_sequence_4_step_02(void)
{
    MATRIX base;
    MATRIX effect;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = g_wmap_effect_model_b_position.vz - 0x960;
    g_wmap_effect_model_b_position.vz = depth;
    if (depth < 0x3E8)
    {
        g_wmap_effect_model_b_position.vz = (s32) D_80182DC8;
        D_801B2474 = D_801B2470;
    }
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_b_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_b_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_801B2474 != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_2 + 0x5000, 0, 4, -1, -1, 1, D_801B2474);
    }
    PopMatrix();
    intensity = D_801B2474 - 9;
    D_801B2474 = intensity;
    if (intensity < 0)
    {
        D_801B2474 = 0;
    }
    remaining = g_wmap_land_effect_18_sequence_4_timer - 1;
    g_wmap_land_effect_18_sequence_4_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_4_step += 1;
    }
}

/** @brief Advance the effect depth, fade its intensity, and update the countdown. */
void wmap_land_effect_18_sequence_4_step_04(void)
{
    MATRIX base;
    MATRIX effect;
    s32 depth;
    s32 intensity;
    s32 remaining;
    s32 peak;

    depth = g_wmap_effect_model_b_position.vz - 0x960;
    g_wmap_effect_model_b_position.vz = depth;
    if (depth < 0x3E8)
    {
        g_wmap_effect_model_b_position.vz = (s32) D_80182DC8;
        D_801B2474 = D_801B2470;
    }
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_b_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_b_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_801B2474 != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_2 + 0x5000, 0, 4, -1, -1, 1, D_801B2474);
    }
    PopMatrix();
    peak = D_801B2470 - 1;
    intensity = D_801B2474 - 9;
    D_801B2470 = peak;
    D_801B2474 = intensity;
    if (intensity < 0)
    {
        D_801B2474 = 0;
    }
    if (peak < 0)
    {
        D_801B2470 = 0;
    }
    remaining = g_wmap_land_effect_18_sequence_4_timer - 1;
    g_wmap_land_effect_18_sequence_4_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_4_step += 1;
    }
}

/**
 * @brief Project and draw active world-map sparks, then respawn empty slots.
 */
void func_8006E6B8(void)
{
    SVECTOR position;
    s32 screen;
    WmapEffect18Spark* spark;
    WmapEffect18Draw* draw;
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < 30; i++)
    {
        spark = &g_wmap_actor_motions[i];
        if (spark->active != 0)
        {
            draw = &g_wmap_sprite_actors[6 + i];
            position.vx = ((spark->radius >> 6) * (ccos(spark->angle) >> 6)) >> 0xC;
            position.vy = ((spark->radius >> 6) * (csin(spark->angle) >> 6)) >> 0xC;
            position.vz = 0;
            gte_ldv0(&position);
            gte_rtps();
            spark->radius += spark->delta;
            draw->unk22 = *(u16*)&D_80139980;
            draw->unk24 = *(u16*)&D_80139980;
            gte_stsxy(&screen);
            wmap_step_actor_animation(draw, &g_wmap_actor_animations[6 + i]);
            wmap_draw_actor_sprite(draw, screen, 9, 0x1F, 0);
            if (--spark->timer == 0)
            {
                spark->active = 0;
            }
            count++;
        }
    }

    for (i = 0; i < 30; i++)
    {
        spark = &g_wmap_actor_motions[i];
        if (spark->active == 0)
        {
            if (g_wmap_particle_intensity < count++)
            {
                break;
            }
            draw = &g_wmap_sprite_actors[6 + i];
            draw->unk06 = 15;
            draw->unk10 = -1;
            draw->unk02 = 0;
            draw->unk0E = 0;
            spark->active = 1;
            spark->angle = (u32)rand() >> 3;
            spark->radius = 0;
            spark->delta = rand() / 2 + 0x1000;
            spark->timer = (rand() & 0x3C) + 0x4B;
        }
    }
}

/** @brief Initialize and project the map effect before its first draw. */
void wmap_land_effect_18_sequence_6_step_01(void)
{
    SVECTOR position;

    g_wmap_actor_animations[4].data = g_wmap_animation_bank_0;
    g_wmap_sprite_actors[4].scale_index = 15;
    g_wmap_sprite_actors[4].previous_sequence = -1;
    g_wmap_sprite_actors[4].resource_index = 0;
    g_wmap_sprite_actors[4].sequence = 0;
    g_wmap_sprite_actors[4].target_shade = 128;
    g_wmap_sprite_actors[4].shade = 128;
    position.vz = 0;
    position.vx = (((g_wmap_focus_cell_x - 1) * 160 -
                   g_wmap_view.x * 0x14000 / g_wmap_view.projection_scale) * 0x6000) /
                  g_wmap_view.projection_scale;
    position.vy = (((g_wmap_focus_cell_y - 1) * 160 -
                   g_wmap_view.y * 0x14000 / g_wmap_view.projection_scale) * 0x6000) /
                  g_wmap_view.projection_scale;
    gte_ldv0(&position);
    gte_rtps();
    gte_stsxy(&g_wmap_focus_screen_position);
    g_wmap_effect_fade_c = 128;
    g_wmap_land_effect_18_sequence_6_timer = 64;
    g_wmap_land_effect_18_sequence_6_step++;
    wmap_land_effect_18_sequence_6_step_02();
}

/** @brief Configure effect parameters and reset its animation resource slots. */
void wmap_land_effect_18_sequence_9_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 16;
    D_80139234 = 3;
    D_8013923C = 1;
    D_80139240 = 56;
    D_8013924C = 0;
    D_80139250 = 1;
    D_80139260 = 5000;
    D_80139264 = 60;
    D_80139268 = 17;
    D_8013926C = 1;
    D_80139284 = 0;
    g_wmap_effect_fade_b = 127;
    for (i = 0; i < 16; i++)
    {
        g_wmap_actor_motions[i + D_80139264].active = 0;
        g_wmap_actor_animations[i + 104].data = g_wmap_animation_bank_1;
    }
    g_wmap_land_effect_18_sequence_9_timer = 32;
    g_wmap_land_effect_18_sequence_9_step++;
    wmap_land_effect_18_sequence_9_step_02();
}

/** @brief Configure effect parameters and reset its animation resource slots. */
void wmap_land_effect_18_sequence_10_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 40;
    D_80139234 = 6;
    D_8013923C = 2;
    D_80139240 = 32;
    D_8013924C = 2;
    D_80139250 = 1;
    D_80139260 = 5000;
    D_80139264 = 120;
    D_80139268 = 17;
    D_8013926C = 2;
    D_80139284 = 0;
    g_wmap_effect_fade_d = 127;
    for (i = 0; i < 40; i++)
    {
        g_wmap_actor_motions[i + D_80139264].active = 0;
        g_wmap_actor_animations[i + 204].data = g_wmap_animation_bank_1;
    }
    g_wmap_land_effect_18_sequence_10_timer = 48;
    g_wmap_land_effect_18_sequence_10_step++;
    wmap_land_effect_18_sequence_10_step_02();
}

/** @brief Compose the effect transform, draw and brighten it, and advance its countdown. */
void wmap_land_effect_18_sequence_11_step_02(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;
    s32 intensity;

    (void)((volatile VECTOR*)&g_wmap_effect_model_d_position)->vz;
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_d_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_d_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_80182DE4 != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_2 + 0x6000, 0, 4, -1, -1, 1, D_80182DE4);
    }
    PopMatrix();
    intensity = D_80182DE4 + 10;
    D_80182DE4 = intensity;
    if (intensity >= 256)
    {
        D_80182DE4 = 255;
    }
    remaining = g_wmap_land_effect_18_sequence_11_timer - 1;
    g_wmap_land_effect_18_sequence_11_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_11_step++;
    }
}

/** @brief Compose the effect transform, draw and fade it, and advance its countdown. */
void wmap_land_effect_18_sequence_11_step_04(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;
    s32 intensity;

    (void)((volatile VECTOR*)&g_wmap_effect_model_d_position)->vz;
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_d_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_d_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_80182DE4 != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_2 + 0x6000, 0, 4, -1, -1, 1, D_80182DE4);
    }
    PopMatrix();
    intensity = D_80182DE4 - 10;
    D_80182DE4 = intensity;
    if (intensity < 0)
    {
        D_80182DE4 = 0;
    }
    remaining = g_wmap_land_effect_18_sequence_11_timer - 1;
    g_wmap_land_effect_18_sequence_11_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_11_step++;
    }
}

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_1, D_800D4CC4, 0x6, g_wmap_land_effect_18_sequence_1_step,
                               g_wmap_land_effect_18_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_1_reset, g_wmap_land_effect_18_sequence_1_step, g_wmap_land_effect_18_sequence_1_timer)

/** @brief Restore the effect state and begin a one-tick sequence step. */
void wmap_land_effect_18_sequence_1_step_01(void)
{
    D_801B24B4 = 128;
    g_wmap_effect_model_a_rotation = g_wmap_zero_rotation;
    g_wmap_land_effect_18_sequence_1_timer = 1;
    g_wmap_land_effect_18_sequence_1_step += 1;
    wmap_land_effect_18_sequence_1_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_1_step_03, g_wmap_land_effect_18_sequence_1_step, g_wmap_land_effect_18_sequence_1_timer, 0x40,
                    wmap_land_effect_18_sequence_1_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_1_end, g_wmap_land_effect_18_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_18_run_sequence_2, D_800D4CDC, 0xA, g_wmap_land_effect_18_sequence_2_step, g_wmap_land_effect_18_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_2_reset, g_wmap_land_effect_18_sequence_2_step, g_wmap_land_effect_18_sequence_2_timer)

/** @brief Clear effect state and two vectors, then begin a 60-tick sequence step. */
void wmap_land_effect_18_sequence_2_step_01(void)
{
    D_801B2468 = 0;
    D_801B24B0 = 0;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    g_wmap_land_effect_18_sequence_2_timer = 0x3C;
    g_wmap_land_effect_18_sequence_2_step += 1;
    wmap_land_effect_18_sequence_2_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_2_step_03, g_wmap_land_effect_18_sequence_2_step, g_wmap_land_effect_18_sequence_2_timer, 0x9,
                    wmap_land_effect_18_sequence_2_step_04)

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_2_step_05, g_wmap_land_effect_18_sequence_2_step, g_wmap_land_effect_18_sequence_2_timer, 0x30,
                    wmap_land_effect_18_sequence_2_step_06)

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_2_step_07, g_wmap_land_effect_18_sequence_2_step, g_wmap_land_effect_18_sequence_2_timer, 0x40,
                    wmap_land_effect_18_sequence_2_step_08)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_2_end, g_wmap_land_effect_18_sequence_2_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_3, D_800D4D04, 0x8, g_wmap_land_effect_18_sequence_3_step,
                               g_wmap_land_effect_18_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_3_reset, g_wmap_land_effect_18_sequence_3_step, g_wmap_land_effect_18_sequence_3_timer)

/** @brief Restore the default transform and advance the sequence. */
void wmap_land_effect_18_sequence_3_step_01(void)
{
    g_wmap_effect_model_c_rotation = g_wmap_zero_rotation;
    g_wmap_effect_model_c_position = g_wmap_camera_translation;
    D_801B2488 = 0;
    D_801B246C = 0x80;
    g_wmap_land_effect_18_sequence_3_timer = 4;
    g_wmap_land_effect_18_sequence_3_step++;
    wmap_land_effect_18_sequence_3_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_3_step_03, g_wmap_land_effect_18_sequence_3_step, g_wmap_land_effect_18_sequence_3_timer, 0x20,
                    wmap_land_effect_18_sequence_3_step_04)

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_3_step_05, g_wmap_land_effect_18_sequence_3_step, g_wmap_land_effect_18_sequence_3_timer, 0x80,
                    wmap_land_effect_18_sequence_3_step_06)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_3_end, g_wmap_land_effect_18_sequence_3_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_4, D_800D4D24, 0x6, g_wmap_land_effect_18_sequence_4_step,
                               g_wmap_land_effect_18_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_4_reset, g_wmap_land_effect_18_sequence_4_step, g_wmap_land_effect_18_sequence_4_timer)

/** @brief Restore the default transform and advance the sequence. */
void wmap_land_effect_18_sequence_4_step_01(void)
{
    g_wmap_effect_model_b_rotation = g_wmap_zero_rotation;
    g_wmap_effect_model_b_position = g_wmap_camera_translation;
    D_801B2470 = 0x80;
    g_wmap_land_effect_18_sequence_4_timer = 1;
    g_wmap_land_effect_18_sequence_4_step++;
    wmap_land_effect_18_sequence_4_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_4_step_03, g_wmap_land_effect_18_sequence_4_step, g_wmap_land_effect_18_sequence_4_timer, 0x20,
                    wmap_land_effect_18_sequence_4_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_4_end, g_wmap_land_effect_18_sequence_4_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_5, D_800D4D3C, 0x6, g_wmap_land_effect_18_sequence_5_step,
                               g_wmap_land_effect_18_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_5_reset, g_wmap_land_effect_18_sequence_5_step, g_wmap_land_effect_18_sequence_5_timer)

/**
 * @brief World-map step handler: seed a 30-entry table and advance the step.
 */
void wmap_land_effect_18_sequence_5_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 0;
    D_80139980 = 0x80;
    for (i = 0; i < 30; i++)
    {
        g_wmap_actor_motions[i].active = 0;
        g_wmap_actor_animations[i + 6].data = g_wmap_animation_bank_1;
    }
    g_wmap_land_effect_18_sequence_5_timer = 0xC;
    g_wmap_land_effect_18_sequence_5_step += 1;
    wmap_land_effect_18_sequence_5_step_02();
}

/** @brief World-map step tick: bump a counter, run the sub-step, and expire the timer. */
void wmap_land_effect_18_sequence_5_step_02(void)
{
    if (g_wmap_frame_count & 1)
    {
        g_wmap_particle_intensity += 1;
    }
    func_8006E6B8();
    if (--g_wmap_land_effect_18_sequence_5_timer == 0)
    {
        g_wmap_land_effect_18_sequence_5_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_5_step_03, g_wmap_land_effect_18_sequence_5_step, g_wmap_land_effect_18_sequence_5_timer, 0x40,
                    wmap_land_effect_18_sequence_5_step_04)

/**
 * @brief World-map step tick: age two timers with zero clamps, run the sub-step,
 *        and expire the step counter.
 */
void wmap_land_effect_18_sequence_5_step_04(void)
{
    D_80139980 -= 1;
    if (D_80139980 < 0)
    {
        D_80139980 = 0;
    }
    if ((g_wmap_frame_count & 3) == 0)
    {
        g_wmap_particle_intensity -= 1;
    }
    if (g_wmap_particle_intensity < 0)
    {
        g_wmap_particle_intensity = 0;
    }
    func_8006E6B8();
    if (--g_wmap_land_effect_18_sequence_5_timer == 0)
    {
        g_wmap_land_effect_18_sequence_5_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_5_end, g_wmap_land_effect_18_sequence_5_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_6, D_800D4D54, 0x6, g_wmap_land_effect_18_sequence_6_step,
                               g_wmap_land_effect_18_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_6_reset, g_wmap_land_effect_18_sequence_6_step, g_wmap_land_effect_18_sequence_6_timer)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_18_sequence_6_step_02, g_wmap_land_effect_18_sequence_6_step, g_wmap_land_effect_18_sequence_6_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 8, 0x2E, 0)

void wmap_land_effect_18_sequence_6_step_03(void)
{
    g_wmap_sprite_actors[4].target_shade = 0;
    g_wmap_sprite_actors[4].shade_step = 2;
    g_wmap_land_effect_18_sequence_6_timer = 0x5D;
    g_wmap_land_effect_18_sequence_6_step += 1;
    wmap_land_effect_18_sequence_6_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_18_sequence_6_step_04, g_wmap_land_effect_18_sequence_6_step, g_wmap_land_effect_18_sequence_6_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x8, 0x2E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_6_end, g_wmap_land_effect_18_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_18_run_sequence_7, D_800D4D6C, 0x4, g_wmap_land_effect_18_sequence_7_step, g_wmap_land_effect_18_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_7_reset, g_wmap_land_effect_18_sequence_7_step, g_wmap_land_effect_18_sequence_7_timer)

/** @brief World-map step handler: init a UI descriptor block, set the timer, advance the step. */
void wmap_land_effect_18_sequence_7_step_01(void)
{
    u8 *base = &g_wmap_sprite_actors;

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_0;
    *(u8 *)(base + 0xE2) = 0xF;
    *(s16 *)(base + 0xEA) = 1;
    *(s16 *)(base + 0xEC) = -1;
    *(s16 *)(base + 0xDE) = 0;
    *(s16 *)(base + 0xFE) = 0x80;
    *(s16 *)(base + 0x100) = 0x80;
    g_wmap_land_effect_18_sequence_7_timer = 0xC7;
    g_wmap_land_effect_18_sequence_7_step += 1;
    wmap_land_effect_18_sequence_7_step_02();
}

/** @brief Draw the projected effect and advance after its countdown. */
void wmap_land_effect_18_sequence_7_step_02(void)
{
    s32 screen_position;
    s32 remaining;
    u8 *actor = (u8*)&g_wmap_sprite_actors[5];

    wmap_project_focus_cell();
    gte_stsxy(&screen_position);
    wmap_step_actor_animation(actor, &g_wmap_actor_animations[5]);
    wmap_draw_actor_sprite(actor, screen_position, 8, 0x2E, 0);
    remaining = g_wmap_land_effect_18_sequence_7_timer - 1;
    g_wmap_land_effect_18_sequence_7_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_7_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_7_end, g_wmap_land_effect_18_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_18_run_timeline, D_800D4C54, 0x16, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_18_timeline_reset, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

/** @brief Set world-map flags and color, then begin an eight-tick delay. */
void wmap_land_effect_18_timeline_step_01(void)
{
    g_wmap_transition_mesh_hidden = 1;
    g_wmap_backdrop_target_level = 8;
    wmap_start_map_tint(0x262726);
    g_wmap_land_effect_18_timeline_timer = 8;
    g_wmap_land_effect_18_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_02, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

/** @brief Play sound 18, register two callbacks, and begin a seven-tick delay. */
void wmap_land_effect_18_timeline_step_03(void)
{
    wmap_play_sound(0x12, 0x80);
    wmap_start_sequence(&wmap_land_effect_18_run_sequence_11);
    g_wmap_placement_overlay_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_18_run_sequence_6);
    g_wmap_land_effect_18_timeline_timer = 7;
    g_wmap_land_effect_18_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_04, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_18_timeline_step_05, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer,
                         wmap_land_effect_18_run_sequence_5, 0x34)

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_06, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_18_timeline_step_07, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer,
                         wmap_land_effect_18_run_sequence_2, 0x3A)

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_08, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_18_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_18_run_sequence_3);
    wmap_start_sequence(wmap_land_effect_18_run_sequence_1);
    wmap_start_sequence(wmap_land_effect_18_run_sequence_10);
    g_wmap_land_effect_18_timeline_timer = 0x10;
    g_wmap_land_effect_18_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_10, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_18_timeline_step_11, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer,
                             wmap_land_effect_18_run_sequence_4, wmap_land_effect_18_run_sequence_8, 0x18)

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_12, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_18_timeline_step_13, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer,
                         wmap_land_effect_18_run_sequence_7, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_14, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

/** @brief World-map step: set fade colour then advance to the next handler. */
void wmap_land_effect_18_timeline_step_15(void)
{
    wmap_start_map_tint(0x808080);
    g_wmap_land_effect_18_timeline_timer = 0x54;
    g_wmap_land_effect_18_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_16, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

/** @brief World-map step handler: seed timers and advance the counter. */
void wmap_land_effect_18_timeline_step_17(void)
{
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_backdrop_target_level = 0x10;
    g_wmap_land_effect_18_timeline_timer = 0x28;
    g_wmap_land_effect_18_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_18, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_18_timeline_step_19, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer,
                         wmap_land_effect_18_run_sequence_9, 0x46)

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_20, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_18_timeline_finish, g_wmap_land_effect_18_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_8, D_800D4D7C, 0x6, g_wmap_land_effect_18_sequence_8_step,
                               g_wmap_land_effect_18_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_8_reset, g_wmap_land_effect_18_sequence_8_step, g_wmap_land_effect_18_sequence_8_timer)

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_18_sequence_8_step_01(void)
{
    D_801B248C = 0;
    g_wmap_land_effect_18_sequence_8_timer = 0x3D;
    g_wmap_land_effect_18_sequence_8_step += 1;
    wmap_land_effect_18_sequence_8_step_02();
}

/** @brief Adjust the Z angle and advance when the countdown reaches zero. */
void wmap_land_effect_18_sequence_8_step_02(void)
{
    s32 remaining_ticks;
    D_801398C8.vz = (u16)(D_801398C8.vz + D_801B248C);
    remaining_ticks = g_wmap_land_effect_18_sequence_8_timer - 1;
    g_wmap_land_effect_18_sequence_8_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_18_sequence_8_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_8_step_03, g_wmap_land_effect_18_sequence_8_step, g_wmap_land_effect_18_sequence_8_timer, 0x80,
                    wmap_land_effect_18_sequence_8_step_04)

/** @brief Reduce angular speed toward zero, update the Z angle, and count down the step. */
void wmap_land_effect_18_sequence_8_step_04(void)
{
    s32 speed;
    s32 remaining_ticks;
    speed = D_801B248C - 5;
    D_801B248C = speed;
    if (speed < 0)
    {
        D_801B248C = 0;
    }
    D_801398C8.vz = (u16)(D_801398C8.vz + (u16)D_801B248C);
    remaining_ticks = g_wmap_land_effect_18_sequence_8_timer - 1;
    g_wmap_land_effect_18_sequence_8_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_18_sequence_8_step += 1;
    }
}

/**
 * @brief Clear the sequence flag and advance the step counter.
 */
void wmap_land_effect_18_sequence_8_step_05(void)
{
    D_801398CC = 0;
    g_wmap_land_effect_18_sequence_8_step += 1;
}

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_9, D_800D4D94, 0x4, g_wmap_land_effect_18_sequence_9_step,
                               g_wmap_land_effect_18_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_9_reset, g_wmap_land_effect_18_sequence_9_step, g_wmap_land_effect_18_sequence_9_timer)

/** @brief World-map step: draw the active overlay while its timer runs, then tick down. */
void wmap_land_effect_18_sequence_9_step_02(void)
{
    s32 c;

    if (g_wmap_effect_fade_b > 0)
    {
        func_8006CFE4(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0x10, 0, g_wmap_effect_fade_b, 3);
    }
    g_wmap_effect_fade_b -= 4;
    c = g_wmap_land_effect_18_sequence_9_timer - 1;
    g_wmap_land_effect_18_sequence_9_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_18_sequence_9_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_9_end, g_wmap_land_effect_18_sequence_9_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_10, D_800D4DA4, 0x4, g_wmap_land_effect_18_sequence_10_step,
                               g_wmap_land_effect_18_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_10_reset, g_wmap_land_effect_18_sequence_10_step, g_wmap_land_effect_18_sequence_10_timer)

/** @brief World-map step: draw the active overlay while its timer runs, then tick down. */
void wmap_land_effect_18_sequence_10_step_02(void)
{
    s32 c;

    if (g_wmap_effect_fade_d > 0)
    {
        func_8006CFE4(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x28, 0, g_wmap_effect_fade_d, 3);
    }
    g_wmap_effect_fade_d -= 4;
    c = g_wmap_land_effect_18_sequence_10_timer - 1;
    g_wmap_land_effect_18_sequence_10_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_18_sequence_10_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_10_end, g_wmap_land_effect_18_sequence_10_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_11, D_800D4DB4, 0x6, g_wmap_land_effect_18_sequence_11_step,
                               g_wmap_land_effect_18_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_11_reset, g_wmap_land_effect_18_sequence_11_step, g_wmap_land_effect_18_sequence_11_timer)

WMAP_STEP_DROP_START(wmap_land_effect_18_sequence_11_step_01, g_wmap_land_effect_18_sequence_11_step, g_wmap_land_effect_18_sequence_11_timer, g_wmap_effect_model_d_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_d_position, D_80182DE4, 0, 0, 12, wmap_land_effect_18_sequence_11_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_11_step_03, g_wmap_land_effect_18_sequence_11_step, g_wmap_land_effect_18_sequence_11_timer, 0x28,
                    wmap_land_effect_18_sequence_11_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_11_step_05, g_wmap_land_effect_18_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_18_run, D_800D4CAC, 0x6, g_wmap_land_effect_18_step, g_wmap_land_effect_18_timer)

WMAP_STEP_RESET(wmap_land_effect_18_reset, g_wmap_land_effect_18_step, g_wmap_land_effect_18_timer)

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void wmap_land_effect_18_step_01(void)
{
    g_wmap_input_locked = 1;
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_18_step += 1;
    wmap_land_effect_18_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_18_wait_idle_02, g_wmap_land_effect_18_step, wmap_land_effect_18_step_03)

/** @brief World-map step: register the next draw callback and advance to the next handler. */
void wmap_land_effect_18_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_18_run_timeline);
    g_wmap_land_effect_18_step += 1;
    wmap_land_effect_18_wait_idle_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_18_wait_idle_04, g_wmap_land_effect_18_step, wmap_land_effect_18_end)

WMAP_STEP_ADVANCE(wmap_land_effect_18_end, g_wmap_land_effect_18_step)
