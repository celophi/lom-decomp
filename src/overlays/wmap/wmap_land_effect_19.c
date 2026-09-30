#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_19.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

void wmap_land_effect_19_sequence_7_step_02(void);
void wmap_land_effect_19_sequence_8_step_02(void);
void wmap_land_effect_19_sequence_9_step_02(void);
void wmap_land_effect_19_sequence_10_step_02(void);
void wmap_land_effect_19_sequence_11_step_02(void);
void wmap_land_effect_19_wait_idle_02(void);
void wmap_land_effect_19_step_03(void);
s32 wmap_land_effect_19_run_timeline(s32 arg0);
void wmap_land_effect_19_wait_idle_04(void);
void wmap_land_effect_19_end(void);
s32 wmap_land_effect_19_run_sequence_7(s32 arg0);
s32 wmap_land_effect_19_run_sequence_1(s32 arg0);
s32 wmap_land_effect_19_run_sequence_3(s32 arg0);
s32 wmap_land_effect_19_run_sequence_5(s32 arg0);
s32 wmap_land_effect_19_run_sequence_6(s32 arg0);
s32 wmap_land_effect_19_run_sequence_10(s32 arg0);
s32 wmap_land_effect_19_run_sequence_11(s32 arg0);
s32 wmap_land_effect_19_run_sequence_4(s32 arg0);
s32 wmap_land_effect_19_run_sequence_9(s32 arg0);
s32 wmap_land_effect_19_run_sequence_8(s32 arg0);
s32 wmap_land_effect_19_run_sequence_2(s32 arg0);
s32 wmap_land_effect_19_run_sequence_12(s32 arg0);
void wmap_land_effect_19_sequence_1_step_02(void);
void wmap_land_effect_19_sequence_2_step_02(void);
void wmap_land_effect_19_sequence_7_step_04(void);
void wmap_land_effect_19_sequence_7_step_06(void);
void wmap_land_effect_19_sequence_8_step_04(void);
void wmap_land_effect_19_sequence_9_step_04(void);
void wmap_land_effect_19_sequence_9_step_06(void);
void wmap_land_effect_19_sequence_10_step_04(void);
void wmap_land_effect_19_sequence_10_step_06(void);
void wmap_land_effect_19_sequence_11_step_04(void);
void wmap_land_effect_19_sequence_11_step_06(void);
void wmap_land_effect_19_sequence_12_step_02(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

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

extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_19_sequence_3_timer;
extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_19_sequence_4_timer;
extern s32* g_wmap_effect_model_pack_2;
extern s32 g_wmap_land_effect_19_sequence_5_timer;
extern s32 g_wmap_land_effect_19_sequence_6_timer;
extern s32 D_80182DE4;
extern s32 D_800DCEA8;
extern s32 D_800D9150;
extern s32 g_wmap_land_effect_19_sequence_7_timer;
extern s32 g_wmap_land_effect_19_sequence_8_timer;
extern s32 D_801B25D8;
extern s32 D_800DCEAC;
extern s32 D_800D9154;
extern s32 g_wmap_land_effect_19_sequence_9_timer;
extern s32 rand(void);
extern u8 g_wmap_animation_bank_2[];
extern s32 D_801B25DC;
extern s32 D_800DCEB0;
extern s32 D_800D9158;
extern s32 g_wmap_land_effect_19_sequence_10_timer;
extern s32 D_800D915C;
extern s32 D_800DCEB4;
extern s32 g_wmap_land_effect_19_sequence_11_timer;
extern s32 g_wmap_land_effect_19_timer;
extern void (*D_800D6A84[])(void);
extern void wmap_land_effect_19_step_03(void);
extern void wmap_land_effect_19_end(void);
extern s32 g_wmap_land_effect_19_timeline_timer;
extern void (*D_800D6A9C[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_19_sequence_1_timer;
extern void (*D_800D6AFC[])(void);
extern s32 g_wmap_land_effect_19_sequence_2_timer;
extern void (*D_800D6B0C[])(void);
extern u8 g_wmap_animation_bank_0[];
extern void wmap_land_effect_19_sequence_2_step_02(void);
extern void (*D_800D6B1C[])(void);
extern void (*D_800D6B2C[])(void);
extern void (*D_800D6B3C[])(void);
extern void (*D_800D6B54[])(void);
extern void (*D_800D6B6C[])(void);
extern void (*D_800D6B8C[])(void);
extern void (*D_800D6BA4[])(void);
extern void (*D_800D6BC4[])(void);
extern void (*D_800D6BE4[])(void);
extern s32 g_wmap_land_effect_19_sequence_12_timer;
extern void (*D_800D6C04[])(void);
extern u8 g_wmap_animation_bank_4[];

extern u32 g_wmap_land_effect_19_sequence_3_step;
extern u32 g_wmap_land_effect_19_sequence_4_step;
extern u32 g_wmap_land_effect_19_sequence_5_step;
extern u32 g_wmap_land_effect_19_sequence_6_step;
extern u8 g_wmap_animation_bank_1[];
extern u32 g_wmap_land_effect_19_sequence_7_step;
extern u8 g_wmap_animation_bank_3[];
extern u32 g_wmap_land_effect_19_sequence_8_step;
extern u32 g_wmap_land_effect_19_sequence_9_step;
extern u32 g_wmap_land_effect_19_sequence_10_step;
extern u32 g_wmap_land_effect_19_sequence_11_step;
extern u32 g_wmap_land_effect_19_step;
extern u32 g_wmap_land_effect_19_timeline_step;
extern u32 g_wmap_land_effect_19_sequence_1_step;
extern u32 g_wmap_land_effect_19_sequence_2_step;
extern u32 g_wmap_land_effect_19_sequence_12_step;

extern VECTOR g_wmap_camera_translation;



extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapSlot14 g_wmap_actor_motions[];

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_19_sequence_3_step_02(void)
{
    MATRIX m;
    s32 x;

    x = g_wmap_effect_model_a_position.vz - 0xDAC;
    g_wmap_effect_model_a_position.vz = x;
    if (x < 0x2710)
    {
        g_wmap_effect_model_a_position.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&g_wmap_effect_model_a_rotation, &m);
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
    if (--g_wmap_land_effect_19_sequence_3_timer == 0)
    {
        g_wmap_land_effect_19_sequence_3_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_19_sequence_4_step_02(void)
{
    MATRIX m;
    s32 x;

    x = g_wmap_effect_model_b_position.vz - 0xDAC;
    g_wmap_effect_model_b_position.vz = x;
    if (x < 0x2710)
    {
        g_wmap_effect_model_b_position.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&g_wmap_effect_model_b_rotation, &m);
    TransMatrix(&m, &g_wmap_zero_translation);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (g_wmap_effect_fade_b != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 0x4, 0x35, 0x7800, 0x1, g_wmap_effect_fade_b);
        g_wmap_effect_fade_b -= 1;
        if (g_wmap_effect_fade_b < 0)
        {
            g_wmap_effect_fade_b = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_19_sequence_4_timer == 0)
    {
        g_wmap_land_effect_19_sequence_4_step += 1;
    }
}

/**
 * @brief World-map step handler: draw the animated actor, ramp its size up to a
 *        cap, scroll the sprite field, then advance when the frame counter expires.
 */
void wmap_land_effect_19_sequence_5_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x36, 0x7880, 0x1001, g_wmap_effect_fade_c, 0, 0xA, -1);
    value = g_wmap_effect_fade_c + 4;
    g_wmap_effect_fade_c = value;
    if (value >= 0x82)
    {
        g_wmap_effect_fade_c = 0x81;
    }
    timer = g_wmap_land_effect_19_sequence_5_timer;
    g_wmap_effect_model_c_rotation.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_5_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_5_step++;
    }
}

/**
 * @brief World-map step handler: render the actor, ramp its size down to a floor,
 *        scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_effect_19_sequence_5_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x36, 0x7880, 0x1001, g_wmap_effect_fade_c, 0, 0xA, -1);
    value = g_wmap_effect_fade_c - 2;
    g_wmap_effect_fade_c = value;
    if (value < 0)
    {
        g_wmap_effect_fade_c = 0;
    }
    timer = g_wmap_land_effect_19_sequence_5_timer;
    g_wmap_effect_model_c_rotation.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_5_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_5_step++;
    }
}

/**
 * @brief World-map step handler: draw the animated actor, ramp its size up to a
 *        cap, scroll the sprite field, then advance when the frame counter expires.
 */
void wmap_land_effect_19_sequence_6_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x36, 0x78C0, 0x1001, g_wmap_effect_fade_d, 0, 0xA, -1);
    value = g_wmap_effect_fade_d + 8;
    g_wmap_effect_fade_d = value;
    if (value >= 0x82)
    {
        g_wmap_effect_fade_d = 0x81;
    }
    timer = g_wmap_land_effect_19_sequence_6_timer;
    g_wmap_effect_model_d_rotation.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_6_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_6_step++;
    }
}

/**
 * @brief World-map step handler: render the actor, ramp its size down to a floor,
 *        scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_effect_19_sequence_6_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x36, 0x78C0, 0x1001, g_wmap_effect_fade_d, 0, 0xA, -1);
    value = g_wmap_effect_fade_d - 2;
    g_wmap_effect_fade_d = value;
    if (value < 0)
    {
        g_wmap_effect_fade_d = 0;
    }
    timer = g_wmap_land_effect_19_sequence_6_timer;
    g_wmap_effect_model_d_rotation.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_6_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_6_step++;
    }
}

/**
 * @brief Reset a range of world-map actor slots and kick the next handler.
 * @note Clears three parallel slot arrays for indices [0xA, 0x14), seeds each
 *       actor's default fields, then advances the shared step counters.
 */
void wmap_land_effect_19_sequence_7_step_01(void)
{
    s32 i;
    u8* pa;
    u8* pb;

    D_80182DE4 = 1;
    D_800DCEA8 = 1;
    for (i = 0xA; i < 0x14; i++)
    {
        *(s16*)((u8*)g_wmap_actor_motions + i * 0x14) = 0;
        pb = (u8*)g_wmap_actor_animations + i * 0x8;
        *(s32*)(pb + 0x4) = (s32)&g_wmap_animation_bank_1;
        pa = (u8*)g_wmap_sprite_actors + i * 0x2C;
        *(s16*)(pa + 0x2) = 0;
        *(s8*)(pa + 0x6) = 0xF;
        *(s16*)(pa + 0xE) = 0;
        *(s16*)(pa + 0x10) = -1;
    }
    D_800D9150 = 6;
    g_wmap_land_effect_19_sequence_7_timer = 0x10;
    g_wmap_land_effect_19_sequence_7_step += 1;
    wmap_land_effect_19_sequence_7_step_02();
}

/** @brief Configure the effect block, reset its twelve resource slots, and advance the step. */
void wmap_land_effect_19_sequence_8_step_01(void)
{
    s32 i;

    g_wmap_effect_params[1] = 0;
    g_wmap_effect_params[2] = 0;
    g_wmap_effect_params[3] = 0x20;
    g_wmap_effect_params[4] = 0;
    g_wmap_effect_params[5] = 2;
    g_wmap_effect_params[6] = 0x384;
    g_wmap_effect_params[7] = 0x14;
    g_wmap_effect_params[8] = 8;
    g_wmap_effect_params[9] = 1;
    g_wmap_effect_params[10] = 0x32C8;

    for (i = 0; i < 0xC; i++)
    {
        g_wmap_actor_motions[i + 20].field_00 = 0;
        g_wmap_actor_animations[i + 20].data = g_wmap_animation_bank_3;
    }

    g_wmap_land_effect_19_sequence_8_timer = 0x18;
    g_wmap_land_effect_19_sequence_8_step += 1;
    wmap_land_effect_19_sequence_8_step_02();
}

/** @brief Reset the effect actors with randomized animation variants. */
void wmap_land_effect_19_sequence_9_step_01(void)
{
    s32 i;

    D_801B25D8 = 1;
    D_800DCEAC = 1;
    for (i = 80; i < 140; i++)
    {
        g_wmap_actor_motions[i].field_00 = 0;
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_1;
        g_wmap_sprite_actors[i].resource_index = 0;
        g_wmap_sprite_actors[i].scale_index = 15;
        g_wmap_sprite_actors[i].sequence = rand() % 3 + 2;
        g_wmap_sprite_actors[i].previous_sequence = -1;
    }
    D_800D9154 = 2;
    g_wmap_land_effect_19_sequence_9_timer = 16;
    g_wmap_land_effect_19_sequence_9_step++;
    wmap_land_effect_19_sequence_9_step_02();
}

/** @brief Initialize sixty alternating actors and begin their timed effect. */
void wmap_land_effect_19_sequence_10_step_01(void)
{
    s32 i;

    D_801B25DC = 1;
    D_800DCEB0 = 12;
    for (i = 150; i < 210; i++)
    {
        g_wmap_actor_motions[i].field_00 = 0;
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_2;
        g_wmap_sprite_actors[i].sequence = i & 1;
        g_wmap_sprite_actors[i].resource_index = 0;
        g_wmap_sprite_actors[i].scale_index = 15;
        g_wmap_sprite_actors[i].previous_sequence = -1;
    }
    D_800D9158 = 1;
    g_wmap_land_effect_19_sequence_10_timer = 16;
    g_wmap_land_effect_19_sequence_10_step++;
    wmap_land_effect_19_sequence_10_step_02();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_19_sequence_11_step_01(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 0xD2;
    D_801B25D8 = 1;
    D_800DCEB4 = 8;

    do
    {
        g_wmap_actor_motions[i].field_00 = 0;
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_3;
        entry = &g_wmap_sprite_actors[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 0;
        entry->unk10 = -1;
        i++;
    } while (i < 0xE6);

    D_800D915C = 2;
    g_wmap_land_effect_19_sequence_11_timer = 0x10;
    g_wmap_land_effect_19_sequence_11_step += 1;
    wmap_land_effect_19_sequence_11_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_19_run, D_800D6A84, 0x6, g_wmap_land_effect_19_step, g_wmap_land_effect_19_timer)

WMAP_STEP_RESET(wmap_land_effect_19_reset, g_wmap_land_effect_19_step, g_wmap_land_effect_19_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_19_step_01, g_wmap_land_effect_19_step, wmap_run_land_focus, wmap_land_effect_19_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_19_wait_idle_02, g_wmap_land_effect_19_step, wmap_land_effect_19_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_19_step_03, g_wmap_land_effect_19_step, wmap_land_effect_19_run_timeline, wmap_land_effect_19_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_19_wait_idle_04, g_wmap_land_effect_19_step, wmap_land_effect_19_end)

WMAP_STEP_ADVANCE(wmap_land_effect_19_end, g_wmap_land_effect_19_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_timeline, D_800D6A9C, 0x18, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_19_timeline_reset, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void wmap_land_effect_19_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x702540);
    g_wmap_backdrop_target_level = 8;
    wmap_play_sound(0x2B, 0x80);
    wmap_start_sequence(wmap_land_effect_19_run_sequence_7);
    g_wmap_land_effect_19_timeline_timer = 0x20;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_02, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/** @brief Register callbacks, set effect flags and color, and begin a ten-tick delay. */
void wmap_land_effect_19_timeline_step_03(void)
{
    wmap_start_sequence(&wmap_land_effect_19_run_sequence_3);
    g_wmap_transition_mesh_hidden = 1;
    g_wmap_placement_overlay_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_19_run_sequence_1);
    g_wmap_backdrop_target_level = 1;
    wmap_start_map_tint(0x201010);
    g_wmap_land_effect_19_timeline_timer = 0xA;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_04, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_05, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_5, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_06, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_07, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_6, 0xC)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_08, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_19_timeline_step_09, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                             wmap_land_effect_19_run_sequence_10, wmap_land_effect_19_run_sequence_11, 0x44)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_10, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_11, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_4, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_12, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/** @brief World-map step: kick a sub-request, set the next state, and arm the timer. */
void wmap_land_effect_19_timeline_step_13(void)
{
    g_wmap_transition_mesh_hidden = 0;
    wmap_start_map_tint(0x504060);
    g_wmap_backdrop_target_level = 8;
    g_wmap_land_effect_19_timeline_timer = 0x1E;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_14, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_15, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_9, 0x10)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_16, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_19_timeline_step_17, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                             wmap_land_effect_19_run_sequence_8, wmap_land_effect_19_run_sequence_2, 0x64)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_18, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_19, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_12, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_20, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_21, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_8, 0x7C)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_22, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_19_timeline_finish, g_wmap_land_effect_19_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_1, D_800D6AFC, 0x4, g_wmap_land_effect_19_sequence_1_step, g_wmap_land_effect_19_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_1_reset, g_wmap_land_effect_19_sequence_1_step, g_wmap_land_effect_19_sequence_1_timer)

/**
 * @brief Arm the world-map sprite actor, set its wait, advance the step, and run the draw handler.
 */
void wmap_land_effect_19_sequence_1_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[4];

    g_wmap_actor_animations[4].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->resource_index = 0;
    actor->target_shade = 0x81;
    actor->shade = 1;
    g_wmap_land_effect_19_sequence_1_timer = 0x80;
    g_wmap_land_effect_19_sequence_1_step += 1;
    wmap_land_effect_19_sequence_1_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_19_sequence_1_step_02, g_wmap_land_effect_19_sequence_1_step, g_wmap_land_effect_19_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x19, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_1_end, g_wmap_land_effect_19_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_2, D_800D6B0C, 0x4, g_wmap_land_effect_19_sequence_2_step, g_wmap_land_effect_19_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_2_reset, g_wmap_land_effect_19_sequence_2_step, g_wmap_land_effect_19_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_19_sequence_2_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 4;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    g_wmap_land_effect_19_sequence_2_timer = 0x88;
    g_wmap_land_effect_19_sequence_2_step += 1;
    wmap_land_effect_19_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_19_sequence_2_step_02, g_wmap_land_effect_19_sequence_2_step, g_wmap_land_effect_19_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x1F, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_2_end, g_wmap_land_effect_19_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_3, D_800D6B1C, 0x4, g_wmap_land_effect_19_sequence_3_step, g_wmap_land_effect_19_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_3_reset, g_wmap_land_effect_19_sequence_3_step, g_wmap_land_effect_19_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_19_sequence_3_step_01, g_wmap_land_effect_19_sequence_3_step, g_wmap_land_effect_19_sequence_3_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_19_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_3_end, g_wmap_land_effect_19_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_4, D_800D6B2C, 0x4, g_wmap_land_effect_19_sequence_4_step, g_wmap_land_effect_19_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_4_reset, g_wmap_land_effect_19_sequence_4_step, g_wmap_land_effect_19_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_19_sequence_4_step_01, g_wmap_land_effect_19_sequence_4_step, g_wmap_land_effect_19_sequence_4_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x80, wmap_land_effect_19_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_4_end, g_wmap_land_effect_19_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_5, D_800D6B3C, 0x6, g_wmap_land_effect_19_sequence_5_step, g_wmap_land_effect_19_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_5_reset, g_wmap_land_effect_19_sequence_5_step, g_wmap_land_effect_19_sequence_5_timer)

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_19_sequence_5_step_01(void)
{
    g_wmap_effect_fade_c = 1;
    g_wmap_effect_model_c_rotation = g_wmap_zero_rotation;
    g_wmap_land_effect_19_sequence_5_timer = 0x40;
    g_wmap_land_effect_19_sequence_5_step += 1;
    wmap_land_effect_19_sequence_5_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_5_step_03, g_wmap_land_effect_19_sequence_5_step, g_wmap_land_effect_19_sequence_5_timer, 0x40,
                    wmap_land_effect_19_sequence_5_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_5_end, g_wmap_land_effect_19_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_6, D_800D6B54, 0x6, g_wmap_land_effect_19_sequence_6_step, g_wmap_land_effect_19_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_6_reset, g_wmap_land_effect_19_sequence_6_step, g_wmap_land_effect_19_sequence_6_timer)

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_19_sequence_6_step_01(void)
{
    g_wmap_effect_fade_d = 1;
    g_wmap_effect_model_d_rotation = g_wmap_zero_rotation;
    g_wmap_land_effect_19_sequence_6_timer = 0x14;
    g_wmap_land_effect_19_sequence_6_step += 1;
    wmap_land_effect_19_sequence_6_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_6_step_03, g_wmap_land_effect_19_sequence_6_step, g_wmap_land_effect_19_sequence_6_timer, 0x40,
                    wmap_land_effect_19_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_6_end, g_wmap_land_effect_19_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_7, D_800D6B6C, 0x8, g_wmap_land_effect_19_sequence_7_step, g_wmap_land_effect_19_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_7_reset, g_wmap_land_effect_19_sequence_7_step, g_wmap_land_effect_19_sequence_7_timer)

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_19_sequence_7_step_02(void)
{
    s32 remaining;

    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32, 0x64, 1, 0x7F, 0x7F, 0, 0);
    D_80182DE4 += 8;
    remaining = g_wmap_land_effect_19_sequence_7_timer - 1;
    g_wmap_land_effect_19_sequence_7_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_19_sequence_7_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_7_step_03, g_wmap_land_effect_19_sequence_7_step, g_wmap_land_effect_19_sequence_7_timer, 0x3C,
                    wmap_land_effect_19_sequence_7_step_04)

/** @brief World-map step: spawn an effect object then count down a timer. */
void wmap_land_effect_19_sequence_7_step_04(void)
{
    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32,
                  0x64, 1, 0x7F, 0x7F, 0, 0);
    if (--g_wmap_land_effect_19_sequence_7_timer == 0)
    {
        g_wmap_land_effect_19_sequence_7_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_7_step_05(void)
{
    D_800DCEA8 = 0;
    g_wmap_land_effect_19_sequence_7_timer = 0x14;
    g_wmap_land_effect_19_sequence_7_step += 1;
    wmap_land_effect_19_sequence_7_step_06();
}

/** @brief World-map step: spawn an effect object then count down a timer. */
void wmap_land_effect_19_sequence_7_step_06(void)
{
    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32,
                  0x64, 1, 0x7F, 0x7F, 0, 0);
    if (--g_wmap_land_effect_19_sequence_7_timer == 0)
    {
        g_wmap_land_effect_19_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_7_end, g_wmap_land_effect_19_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_8, D_800D6B8C, 0x6, g_wmap_land_effect_19_sequence_8_step, g_wmap_land_effect_19_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_8_reset, g_wmap_land_effect_19_sequence_8_step, g_wmap_land_effect_19_sequence_8_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_19_sequence_8_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0xC, 0xFF, 0x1, 0x8, 0, (s32)g_wmap_effect_params);
    if (--g_wmap_land_effect_19_sequence_8_timer == 0)
    {
        g_wmap_land_effect_19_sequence_8_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_19_sequence_8_step_03(void)
{
    g_wmap_land_effect_19_sequence_8_timer = 0x20;
    g_wmap_effect_params[5] = -1;
    g_wmap_land_effect_19_sequence_8_step += 1;
    wmap_land_effect_19_sequence_8_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_19_sequence_8_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0xC, 0xFF, 0x1, 0x8, 0, (s32)g_wmap_effect_params);
    if (--g_wmap_land_effect_19_sequence_8_timer == 0)
    {
        g_wmap_land_effect_19_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_8_end, g_wmap_land_effect_19_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_9, D_800D6BA4, 0x8, g_wmap_land_effect_19_sequence_9_step, g_wmap_land_effect_19_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_9_reset, g_wmap_land_effect_19_sequence_9_step, g_wmap_land_effect_19_sequence_9_timer)

/** @brief World-map effect spawn: submit a request and tick a refcount. */
void wmap_land_effect_19_sequence_9_step_02(void)
{
    s32 c;

    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32,
                  0x64, 0xB4, 0x81, 0x81, 8, 1);
    D_801B25D8 += 8;
    c = g_wmap_land_effect_19_sequence_9_timer - 1;
    g_wmap_land_effect_19_sequence_9_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_19_sequence_9_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_9_step_03, g_wmap_land_effect_19_sequence_9_step, g_wmap_land_effect_19_sequence_9_timer, 0x18,
                    wmap_land_effect_19_sequence_9_step_04)

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_9_step_04(void)
{
    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 1);
    if (--g_wmap_land_effect_19_sequence_9_timer == 0)
    {
        g_wmap_land_effect_19_sequence_9_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_9_step_05(void)
{
    D_800DCEAC = 0;
    g_wmap_land_effect_19_sequence_9_timer = 0x18;
    g_wmap_land_effect_19_sequence_9_step += 1;
    wmap_land_effect_19_sequence_9_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_9_step_06(void)
{
    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 1);
    if (--g_wmap_land_effect_19_sequence_9_timer == 0)
    {
        g_wmap_land_effect_19_sequence_9_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_9_end, g_wmap_land_effect_19_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_10, D_800D6BC4, 0x8, g_wmap_land_effect_19_sequence_10_step, g_wmap_land_effect_19_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_10_reset, g_wmap_land_effect_19_sequence_10_step, g_wmap_land_effect_19_sequence_10_timer)

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_19_sequence_10_step_02(void)
{
    s32 remaining;

    func_8006B328(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2);
    D_801B25DC += 8;
    remaining = g_wmap_land_effect_19_sequence_10_timer - 1;
    g_wmap_land_effect_19_sequence_10_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_19_sequence_10_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_10_step_03, g_wmap_land_effect_19_sequence_10_step, g_wmap_land_effect_19_sequence_10_timer, 0x30,
                    wmap_land_effect_19_sequence_10_step_04)

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_10_step_04(void)
{
    func_8006B328(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2);
    if (--g_wmap_land_effect_19_sequence_10_timer == 0)
    {
        g_wmap_land_effect_19_sequence_10_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_10_step_05(void)
{
    D_800DCEB0 = 0;
    g_wmap_land_effect_19_sequence_10_timer = 0xA0;
    g_wmap_land_effect_19_sequence_10_step += 1;
    wmap_land_effect_19_sequence_10_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_10_step_06(void)
{
    func_8006B328(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2);
    if (--g_wmap_land_effect_19_sequence_10_timer == 0)
    {
        g_wmap_land_effect_19_sequence_10_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_10_end, g_wmap_land_effect_19_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_11, D_800D6BE4, 0x8, g_wmap_land_effect_19_sequence_11_step, g_wmap_land_effect_19_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_11_reset, g_wmap_land_effect_19_sequence_11_step, g_wmap_land_effect_19_sequence_11_timer)

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_19_sequence_11_step_02(void)
{
    s32 remaining;

    func_8006B328(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3);
    D_801B25D8 += 8;
    remaining = g_wmap_land_effect_19_sequence_11_timer - 1;
    g_wmap_land_effect_19_sequence_11_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_19_sequence_11_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_11_step_03, g_wmap_land_effect_19_sequence_11_step, g_wmap_land_effect_19_sequence_11_timer, 0x28,
                    wmap_land_effect_19_sequence_11_step_04)

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_11_step_04(void)
{
    func_8006B328(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3);
    if (--g_wmap_land_effect_19_sequence_11_timer == 0)
    {
        g_wmap_land_effect_19_sequence_11_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_11_step_05(void)
{
    D_800DCEB4 = 0;
    g_wmap_land_effect_19_sequence_11_timer = 0xA0;
    g_wmap_land_effect_19_sequence_11_step += 1;
    wmap_land_effect_19_sequence_11_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_11_step_06(void)
{
    func_8006B328(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3);
    if (--g_wmap_land_effect_19_sequence_11_timer == 0)
    {
        g_wmap_land_effect_19_sequence_11_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_11_end, g_wmap_land_effect_19_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_12, D_800D6C04, 0x4, g_wmap_land_effect_19_sequence_12_step, g_wmap_land_effect_19_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_12_reset, g_wmap_land_effect_19_sequence_12_step, g_wmap_land_effect_19_sequence_12_timer)

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void wmap_land_effect_19_sequence_12_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = g_wmap_animation_bank_4;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade_step = 0;
    actor->target_shade = 0x80;
    actor->shade = 0x80;
    g_wmap_land_effect_19_sequence_12_timer = 0x82;
    g_wmap_land_effect_19_sequence_12_step += 1;
    wmap_land_effect_19_sequence_12_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_19_sequence_12_step_02, g_wmap_land_effect_19_sequence_12_step, g_wmap_land_effect_19_sequence_12_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x1F, 0x7, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_12_end, g_wmap_land_effect_19_sequence_12_step)
