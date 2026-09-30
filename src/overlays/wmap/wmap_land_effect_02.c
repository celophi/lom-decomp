#include "wmap_main.h"
#include "wmap_land_effect_02.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

void wmap_land_effect_02_sequence_3_step_02(void);
void wmap_land_effect_02_sequence_5_step_02(void);
void wmap_land_effect_02_sequence_6_step_02(void);
void wmap_land_effect_02_wait_idle_02(void);
void wmap_land_effect_02_step_03(void);
s32 wmap_land_effect_02_run_timeline(s32 arg0);
void wmap_land_effect_02_wait_idle_04(void);
void wmap_land_effect_02_end(void);
s32 wmap_land_effect_02_run_sequence_2(s32 arg0);
s32 wmap_land_effect_02_run_sequence_3(s32 arg0);
s32 wmap_land_effect_02_run_sequence_5(s32 arg0);
s32 wmap_land_effect_02_run_sequence_4(s32 arg0);
s32 wmap_land_effect_02_run_sequence_6(s32 arg0);
s32 wmap_land_effect_02_run_sequence_1(s32 arg0);
void wmap_land_effect_02_sequence_1_step_02(void);
void wmap_land_effect_02_sequence_2_step_02(void);
void wmap_land_effect_02_sequence_3_step_04(void);
void wmap_land_effect_02_sequence_3_step_06(void);
void wmap_land_effect_02_sequence_6_step_04(void);
void wmap_land_effect_02_sequence_6_step_06(void);

typedef struct
{
    s16 unk0;
    s16 unk2;
    u8 unk4[2];
    u8 unk6;
    u8 unk7[7];
    s16 unkE;
    s16 unk10;
    u8 unk12[0x10];
    s16 unk22;
    s16 unk24;
    s16 unk26;
    u8 unk28[4];
} WmapD94Entry;

typedef union
{
    struct
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
    } f;
    s32 words[11];
} WmapEffect02Config;

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

typedef struct
{
    s32 field_00;
    void *resource;
} WmapEffect02Resource;

extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_8013B280;
extern s32 g_wmap_land_effect_02_sequence_3_timer;
extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_02_sequence_4_timer;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 g_wmap_land_effect_02_sequence_5_timer;
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_801B25D8;
extern s32 g_wmap_land_effect_02_sequence_6_timer;
extern s32 g_wmap_land_effect_02_timer;
extern void (*D_800D54F0[])(void);
extern void wmap_land_effect_02_step_03(void);
extern void wmap_land_effect_02_end(void);
extern s32 g_wmap_land_effect_02_timeline_timer;
extern void (*D_800D5508[])(void);
extern s32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_02_sequence_1_timer;
extern void (*D_800D5538[])(void);
extern u8 g_wmap_animation_bank_1[];
extern s32 g_wmap_land_effect_02_sequence_2_timer;
extern void (*D_800D5548[])(void);
extern void wmap_land_effect_02_sequence_2_step_02(void);
extern void (*D_800D5558[])(void);
extern void (*D_800D5578[])(void);
extern void (*D_800D5588[])(void);
extern void (*D_800D5598[])(void);
extern u8 g_wmap_animation_bank_0[];
extern u32 g_wmap_land_effect_02_sequence_3_step;
extern u32 g_wmap_land_effect_02_sequence_4_step;
extern u32 g_wmap_land_effect_02_sequence_5_step;
extern u32 g_wmap_land_effect_02_sequence_6_step;
extern u32 g_wmap_land_effect_02_step;
extern u32 g_wmap_land_effect_02_timeline_step;
extern u32 g_wmap_land_effect_02_sequence_1_step;
extern u32 g_wmap_land_effect_02_sequence_2_step;

extern VECTOR g_wmap_camera_translation;

extern SVECTOR D_801B2490;


extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern WmapEffect02Motion g_wmap_actor_motions[];
extern WmapEffect02Motion D_801AFC98[];

/**
 * @brief Initialize world-map globals and four per-entry state records.
 */
void wmap_land_effect_02_sequence_3_step_01(void)
{
    s32 i;
    s16 shift;
    WmapD94Entry *entry;

    i = 0;
    g_wmap_particle_intensity = 4;
    g_wmap_effect_fade_b = 0x7F;
    D_80139240 = 0x50;
    D_8013924C = 2;
    D_80139250 = -1;
    D_80139260 = 0x7D0;
    D_80139264 = 0xA;
    D_80139268 = 8;
    D_8013926C = 1;
    D_80139284 = 0;
    D_8013B264 = 0x28;
    D_8013B270 = 0x50;
    D_8013B278 = 0x14;
    D_8013B280 = 1;

    do
    {
        entry = &g_wmap_sprite_actors[14 + i];
        g_wmap_actor_animations[i + 14].data = g_wmap_animation_bank_0;
        shift = i << 10;
        entry->unk6 = 0xF;
        entry->unk10 = -1;
        entry->unk26 = 4;
        entry->unk2 = 0;
        entry->unk22 = 0x81;
        entry->unk24 = 0x81;
        entry->unkE = D_8013926C;
        D_801AFC98[i].state = 1;
        D_801AFC98[i].z = D_80139284;
        D_801AFC98[i].scale = 0x50;
        D_801AFC98[i].angle = shift;
        D_801AFC98[i].x = 0;
        D_801AFC98[i].field_0E = 0;
        i++;
    } while (i < 4);

    g_wmap_land_effect_02_sequence_3_timer = 0x3C;
    g_wmap_land_effect_02_sequence_3_step++;
    wmap_land_effect_02_sequence_3_step_02();
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_02_sequence_4_step_02(void)
{
    MATRIX m;
    s32 x;

    x = g_wmap_effect_model_d_position.vz - 0xDAC;
    g_wmap_effect_model_d_position.vz = x;
    if (x < 0x2710)
    {
        g_wmap_effect_model_d_position.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&g_wmap_effect_model_d_rotation, &m);
    TransMatrix(&m, &g_wmap_zero_translation);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (g_wmap_effect_fade_a != 0)
    {
        wmap_draw_model_default((s32)g_wmap_load_buffer, 0, 0x4, 0x35, 0x7800, 0x1, g_wmap_effect_fade_a);
        g_wmap_effect_fade_a -= 0x2;
        if (g_wmap_effect_fade_a < 0)
        {
            g_wmap_effect_fade_a = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_02_sequence_4_timer == 0)
    {
        g_wmap_land_effect_02_sequence_4_step += 1;
    }
}

/** @brief Initialize four equally spaced actors and start the effect. */
void wmap_land_effect_02_sequence_5_step_01(void)
{
    s32 i;
    WmapSpriteActor *actor;

    D_801B2490 = g_wmap_zero_rotation;
    D_80139234 = 1;
    D_8013923C = 2;
    D_80139240 = 0;
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
        g_wmap_actor_motions[i].angle = D_80139240;
        D_80139240 += 1024;
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

    D_8013923C -= 1;
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
        g_wmap_actor_motions[i].screen.half.x = ((u16 *)&screen_position)[0];
        g_wmap_actor_motions[i].screen.half.y = ((u16 *)&screen_position)[1];

        if (D_8013923C == 0)
        {
            g_wmap_sprite_actors[i + D_80139234] = g_wmap_sprite_actors[i];
            g_wmap_actor_motions[i + D_80139234] = g_wmap_actor_motions[i];
            g_wmap_actor_animations[i + D_80139234] = g_wmap_actor_animations[i];
            g_wmap_sprite_actors[i + D_80139234].shade_step = 8;
            g_wmap_sprite_actors[i + D_80139234].target_shade = 1;
        }
    }

    if (D_8013923C == 0)
    {
        D_8013923C = 2;
        D_80139234 += 1;
        if (D_80139234 >= 15)
        {
            D_80139234 = 1;
            i = 20;
        }
    }

    i = 20;
    {
        WmapEffect02Motion *motion = &g_wmap_actor_motions[i];
        WmapEffect02Resource *resource = &g_wmap_actor_animations[i];
        WmapEffect02Config *actor = &g_wmap_sprite_actors[i];

        do
        {
            if (motion->state != 0)
            {
                wmap_step_actor_animation(actor, resource);
                wmap_draw_actor_sprite(actor, motion->screen.packed, 8, 8, 0);
                if (actor->f.field_24 < 5)
                {
                    motion->state = 0;
                }
            }
            motion++;
            resource++;
            i++;
            actor++;
        } while (i < 80);
    }

    value = g_wmap_land_effect_02_sequence_5_timer - 1;
    g_wmap_land_effect_02_sequence_5_timer = value;
    if (value == 0)
    {
        g_wmap_land_effect_02_sequence_5_step += 1;
    }
}
#undef M2C_FIELD
#undef M2C_UNALIGNED32
#undef M2C_BITWISE

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_02_sequence_6_step_01(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 100;
    D_801B25D8 = 1;
    D_800DCEA8 = 1;

    do
    {
        g_wmap_actor_motions[i].state = 0;
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_0;
        entry = &g_wmap_sprite_actors[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 2;
        entry->unk10 = -1;
        i++;
    } while (i < 200);

    D_800D9150 = 2;
    g_wmap_land_effect_02_sequence_6_timer = 0x10;
    g_wmap_land_effect_02_sequence_6_step += 1;
    wmap_land_effect_02_sequence_6_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_02_run, D_800D54F0, 0x6, g_wmap_land_effect_02_step, g_wmap_land_effect_02_timer)

WMAP_STEP_RESET(wmap_land_effect_02_reset, g_wmap_land_effect_02_step, g_wmap_land_effect_02_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_02_step_01, g_wmap_land_effect_02_step, wmap_run_land_focus, wmap_land_effect_02_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_02_wait_idle_02, g_wmap_land_effect_02_step, wmap_land_effect_02_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_02_step_03, g_wmap_land_effect_02_step, wmap_land_effect_02_run_timeline, wmap_land_effect_02_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_02_wait_idle_04, g_wmap_land_effect_02_step, wmap_land_effect_02_end)

WMAP_STEP_ADVANCE(wmap_land_effect_02_end, g_wmap_land_effect_02_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_timeline, D_800D5508, 0xC, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer)

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
    g_wmap_land_effect_02_timeline_step += 1;
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
    g_wmap_land_effect_02_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_02_timeline_wait_08, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_02_timeline_step_09, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer,
                             wmap_land_effect_02_run_sequence_6, wmap_land_effect_02_run_sequence_1, 0x7F)

WMAP_STEP_WAIT(wmap_land_effect_02_timeline_wait_10, g_wmap_land_effect_02_timeline_step, g_wmap_land_effect_02_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_02_timeline_finish, g_wmap_land_effect_02_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_1, D_800D5538, 0x4, g_wmap_land_effect_02_sequence_1_step, g_wmap_land_effect_02_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_1_reset, g_wmap_land_effect_02_sequence_1_step, g_wmap_land_effect_02_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void wmap_land_effect_02_sequence_1_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[4];

    g_wmap_actor_animations[4].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade_step = 0;
    actor->target_shade = 0x80;
    actor->shade = 0x80;
    g_wmap_land_effect_02_sequence_1_timer = 0x82;
    g_wmap_land_effect_02_sequence_1_step += 1;
    wmap_land_effect_02_sequence_1_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_02_sequence_1_step_02, g_wmap_land_effect_02_sequence_1_step, g_wmap_land_effect_02_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xF, 0x1E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_1_end, g_wmap_land_effect_02_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_2, D_800D5548, 0x4, g_wmap_land_effect_02_sequence_2_step, g_wmap_land_effect_02_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_2_reset, g_wmap_land_effect_02_sequence_2_step, g_wmap_land_effect_02_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_02_sequence_2_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 4;
    actor->target_shade = 0x81;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade = 1;
    g_wmap_land_effect_02_sequence_2_timer = 0x8E;
    g_wmap_land_effect_02_sequence_2_step += 1;
    wmap_land_effect_02_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_02_sequence_2_step_02, g_wmap_land_effect_02_sequence_2_step, g_wmap_land_effect_02_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x8, 0x1, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_2_end, g_wmap_land_effect_02_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_3, D_800D5558, 0x8, g_wmap_land_effect_02_sequence_3_step, g_wmap_land_effect_02_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_3_reset, g_wmap_land_effect_02_sequence_3_step, g_wmap_land_effect_02_sequence_3_timer)

/**
 * @brief Draw a world-map element, then advance the step when its wait expires.
 */
void wmap_land_effect_02_sequence_3_step_02(void)
{
    func_8006D014(&g_wmap_sprite_actors[14], &g_wmap_actor_animations[14], 4, 1, g_wmap_effect_fade_b, 8, 2);
    if (--g_wmap_land_effect_02_sequence_3_timer == 0)
    {
        g_wmap_land_effect_02_sequence_3_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_02_sequence_3_step_03, g_wmap_land_effect_02_sequence_3_step, g_wmap_land_effect_02_sequence_3_timer, 0x14,
                    wmap_land_effect_02_sequence_3_step_04)

/**
 * @brief Draw a world-map element, then advance the step when its wait expires.
 */
void wmap_land_effect_02_sequence_3_step_04(void)
{
    func_8006D014(&g_wmap_sprite_actors[14], &g_wmap_actor_animations[14], 4, 1, g_wmap_effect_fade_b, 8, 2);
    if (--g_wmap_land_effect_02_sequence_3_timer == 0)
    {
        g_wmap_land_effect_02_sequence_3_step += 1;
    }
}

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
    g_wmap_land_effect_02_sequence_3_step += 1;
    wmap_land_effect_02_sequence_3_step_06();
}

/**
 * @brief Draw a world-map element, then advance the step when its wait expires.
 */
void wmap_land_effect_02_sequence_3_step_06(void)
{
    func_8006D014(&g_wmap_sprite_actors[14], &g_wmap_actor_animations[14], 4, 1, g_wmap_effect_fade_b, 8, 2);
    if (--g_wmap_land_effect_02_sequence_3_timer == 0)
    {
        g_wmap_land_effect_02_sequence_3_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_3_end, g_wmap_land_effect_02_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_4, D_800D5578, 0x4, g_wmap_land_effect_02_sequence_4_step, g_wmap_land_effect_02_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_4_reset, g_wmap_land_effect_02_sequence_4_step, g_wmap_land_effect_02_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_02_sequence_4_step_01, g_wmap_land_effect_02_sequence_4_step, g_wmap_land_effect_02_sequence_4_timer, g_wmap_effect_model_d_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_d_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_02_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_4_end, g_wmap_land_effect_02_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_5, D_800D5588, 0x4, g_wmap_land_effect_02_sequence_5_step, g_wmap_land_effect_02_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_5_reset, g_wmap_land_effect_02_sequence_5_step, g_wmap_land_effect_02_sequence_5_timer)

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_5_end, g_wmap_land_effect_02_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_02_run_sequence_6, D_800D5598, 0x8, g_wmap_land_effect_02_sequence_6_step, g_wmap_land_effect_02_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_02_sequence_6_reset, g_wmap_land_effect_02_sequence_6_step, g_wmap_land_effect_02_sequence_6_timer)

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_02_sequence_6_step_02(void)
{
    s32 remaining;

    func_8006B328(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA, 0x1F4, 0x32, 0x81, 0x81, 8, 0);
    D_801B25D8 += 8;
    remaining = g_wmap_land_effect_02_sequence_6_timer - 1;
    g_wmap_land_effect_02_sequence_6_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_02_sequence_6_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_02_sequence_6_step_03, g_wmap_land_effect_02_sequence_6_step, g_wmap_land_effect_02_sequence_6_timer, 0x40,
                    wmap_land_effect_02_sequence_6_step_04)

/** @brief World-map step: spawn an effect object then count down a timer. */
void wmap_land_effect_02_sequence_6_step_04(void)
{
    func_8006B328(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA,
                  0x1F4, 0x32, 0x81, 0x81, 8, 0);
    if (--g_wmap_land_effect_02_sequence_6_timer == 0)
    {
        g_wmap_land_effect_02_sequence_6_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_02_sequence_6_step_05(void)
{
    D_800DCEA8 = 0;
    g_wmap_land_effect_02_sequence_6_timer = 0x40;
    g_wmap_land_effect_02_sequence_6_step += 1;
    wmap_land_effect_02_sequence_6_step_06();
}

/** @brief World-map step: spawn an effect object then count down a timer. */
void wmap_land_effect_02_sequence_6_step_06(void)
{
    func_8006B328(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA,
                  0x1F4, 0x32, 0x81, 0x81, 8, 0);
    if (--g_wmap_land_effect_02_sequence_6_timer == 0)
    {
        g_wmap_land_effect_02_sequence_6_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_02_sequence_6_end, g_wmap_land_effect_02_sequence_6_step)
