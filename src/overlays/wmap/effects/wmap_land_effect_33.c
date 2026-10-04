#include "../internal/wmap_model_render.h"
#include "../internal/wmap_land_effect_33.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_view_effects.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_effect_resources.h"
#include "main/cdrom.h"
#include <libgte.h>
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_cells.h"

/** @brief Activity flag in a world-map motion slot. */
typedef struct
{
    s16 active;
    u8 unknown_02[0x12];
} WmapEffectMotionSlot;

void wmap_land_effect_33_sequence_6_step_02(void);
void wmap_land_effect_33_sequence_7_step_02(void);
s32 wmap_land_effect_33_run_timeline(s32 reset);
void wmap_land_effect_33_step_02(void);
void wmap_land_effect_33_step_03(void);
s32 wmap_land_effect_33_run_sequence_2(s32 reset);
s32 wmap_land_effect_33_run_sequence_3(s32 reset);
s32 wmap_land_effect_33_run_sequence_4(s32 reset);
s32 wmap_land_effect_33_run_sequence_5(s32 reset);
s32 wmap_land_effect_33_run_sequence_6(s32 reset);
s32 wmap_land_effect_33_run_sequence_7(s32 reset);
s32 wmap_land_effect_33_run_sequence_8(s32 reset);
s32 wmap_land_effect_33_run_sequence_9(s32 reset);
s32 wmap_land_effect_33_run_sequence_10(s32 reset);
s32 wmap_land_effect_33_run_sequence_1(s32 reset);
void wmap_land_effect_33_sequence_1_step_02(void);
void wmap_land_effect_33_sequence_1_step_04(void);
void wmap_land_effect_33_sequence_2_step_02(void);
void wmap_land_effect_33_sequence_2_step_04(void);
void wmap_land_effect_33_sequence_3_step_02(void);
void wmap_land_effect_33_sequence_3_step_04(void);
void wmap_land_effect_33_sequence_4_step_02(void);
void wmap_land_effect_33_sequence_4_step_04(void);
void wmap_land_effect_33_sequence_5_step_02(void);
void wmap_land_effect_33_sequence_5_step_04(void);
void wmap_land_effect_33_sequence_6_step_04(void);
void wmap_land_effect_33_sequence_7_step_04(void);
void wmap_land_effect_33_sequence_8_step_04(void);
void wmap_land_effect_33_sequence_9_step_02(void);
void wmap_land_effect_33_sequence_9_step_04(void);
void wmap_land_effect_33_sequence_8_step_02(void);

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
    s32 field_10;
    s32 field_14;
    s32 field_18;
    s32 field_1C;
    s32 field_20;
    s32 state_24;
    s32 field_28;
    s32 field_2C;
    s32 field_30;
    s32 field_34;
    s32 field_38;
    s32 field_3C;
    s32 field_40;
    s32 field_44;
    s32 field_48;
    s32 tail_state;
} WmapState;

typedef void (*WmapHandler)(void);

extern u32 g_wmap_land_effect_33_sequence_6_step;
extern s32 g_wmap_land_effect_33_sequence_6_timer;
extern u32 g_wmap_land_effect_33_sequence_7_step;
extern s32 g_wmap_land_effect_33_sequence_7_timer;
extern WmapHandler D_800D78BC[];
extern WmapHandler D_800D78CC[];
extern WmapHandler D_800D7924[];
extern WmapHandler D_800D793C[];
extern WmapHandler D_800D7954[];
extern WmapHandler D_800D796C[];
extern WmapHandler D_800D7984[];
extern WmapHandler D_800D799C[];
extern WmapHandler D_800D79B4[];
extern WmapHandler D_800D79CC[];
extern WmapHandler D_800D79E4[];
extern WmapHandler D_800D79FC[];
extern s32 g_wmap_load_buffer[];
extern u8 g_wmap_animation_bank_3;
extern u8 g_wmap_animation_bank_4;
extern u8 g_wmap_animation_bank_5;
extern s32 D_80139228;
extern s32 D_8013B294;
extern u32 g_wmap_land_effect_33_step;
extern s32 g_wmap_land_effect_33_timer;
extern u32 g_wmap_land_effect_33_timeline_step;
extern s32 g_wmap_land_effect_33_timeline_timer;
extern u32 g_wmap_land_effect_33_sequence_1_step;
extern s32 g_wmap_land_effect_33_sequence_1_timer;
extern u32 g_wmap_land_effect_33_sequence_2_step;
extern s32 g_wmap_land_effect_33_sequence_2_timer;
extern u32 g_wmap_land_effect_33_sequence_3_step;
extern s32 g_wmap_land_effect_33_sequence_3_timer;
extern u32 g_wmap_land_effect_33_sequence_4_step;
extern s32 g_wmap_land_effect_33_sequence_4_timer;
extern u32 g_wmap_land_effect_33_sequence_5_step;
extern s32 g_wmap_land_effect_33_sequence_5_timer;
extern u32 g_wmap_land_effect_33_sequence_8_step;
extern s32 g_wmap_land_effect_33_sequence_8_timer;
extern u32 g_wmap_land_effect_33_sequence_9_step;
extern s32 g_wmap_land_effect_33_sequence_9_timer;
extern u32 g_wmap_land_effect_33_sequence_10_step;
extern s32 g_wmap_land_effect_33_sequence_10_timer;
extern void akao_fade_all_sfx_volume(s32, s32);
extern void wmap_land_effect_33_sequence_10_step_02(void);
extern void wmap_land_effect_33_step_02(void);
extern void wmap_land_effect_33_step_03(void);
extern s32 wmap_land_effect_33_run_timeline(s32);
extern s32 wmap_land_effect_33_run_sequence_1(s32);
extern void wmap_land_effect_33_sequence_1_step_02(void);
extern void wmap_land_effect_33_sequence_1_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_2(s32);
extern void wmap_land_effect_33_sequence_2_step_02(void);
extern void wmap_land_effect_33_sequence_2_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_3(s32);
extern void wmap_land_effect_33_sequence_3_step_02(void);
extern void wmap_land_effect_33_sequence_3_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_4(s32);
extern void wmap_land_effect_33_sequence_4_step_02(void);
extern void wmap_land_effect_33_sequence_4_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_5(s32);
extern void wmap_land_effect_33_sequence_5_step_02(void);
extern void wmap_land_effect_33_sequence_5_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_6(s32);
extern void wmap_land_effect_33_sequence_6_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_7(s32);
extern void wmap_land_effect_33_sequence_7_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_8(s32);
extern void wmap_land_effect_33_sequence_8_step_02(void);
extern void wmap_land_effect_33_sequence_8_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_9(s32);
extern void wmap_land_effect_33_sequence_9_step_02(void);
extern void wmap_land_effect_33_sequence_9_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_10(s32);
extern u8 g_wmap_animation_bank_2[];

extern VECTOR g_wmap_camera_translation;



extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapEffectMotionSlot g_wmap_actor_motions[];

/** @brief Configure the first world-map effect state and reset its fifty resource slots. */
void wmap_land_effect_33_sequence_6_step_01(void)
{
    s32 i;

    g_wmap_effect_params[1] = 0;
    g_wmap_effect_params[2] = 3;
    g_wmap_effect_params[3] = 0x20;
    g_wmap_effect_params[4] = 0;
    g_wmap_effect_params[5] = 2;
    g_wmap_effect_params[6] = 0x190;
    g_wmap_effect_params[7] = 0x14;
    g_wmap_effect_params[8] = 0x2C;
    g_wmap_effect_params[9] = 3;
    g_wmap_effect_params[10] = 0x32C8;

    WMAP_RESET_PARTICLE_SLOTS(i, 50,
                              g_wmap_actor_motions[i + 20].active,
                              20, g_wmap_animation_bank_2);

    g_wmap_land_effect_33_sequence_6_timer = 100;
    g_wmap_land_effect_33_sequence_6_step++;
    wmap_land_effect_33_sequence_6_step_02();
}

/** @brief Configure the second world-map effect state and reset its forty resource slots. */
void wmap_land_effect_33_sequence_7_step_01(void)
{
    s32 i;

    g_wmap_effect_params[11] = 1;
    g_wmap_effect_params[12] = 2;
    g_wmap_effect_params[13] = 0x80;
    g_wmap_effect_params[14] = 0;
    g_wmap_effect_params[15] = 1;
    g_wmap_effect_params[16] = 0x190;
    g_wmap_effect_params[17] = 0x50;
    g_wmap_effect_params[18] = 0x2C;
    g_wmap_effect_params[19] = 4;
    g_wmap_effect_params[20] = 0x4650;

    WMAP_RESET_PARTICLE_SLOTS(i, 40,
                              g_wmap_actor_motions[i + 80].active,
                              80, g_wmap_animation_bank_2);

    g_wmap_land_effect_33_sequence_7_timer = 40;
    g_wmap_land_effect_33_sequence_7_step++;
    wmap_land_effect_33_sequence_7_step_02();
}

void wmap_land_effect_33_sequence_8_step_01(void)
{
    s32 index;
    s32 screen_offset;
    s32 config_offset;
    uintptr_t config_base;
    uintptr_t screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = (uintptr_t)g_wmap_actor_motions;
    screen_base = (uintptr_t)g_wmap_actor_animations;
    resource = &g_wmap_animation_bank_4;
    screen_offset = 0x460;
    config_offset = 0xAF0;
    g_wmap_effect_params[21] = 0;
    g_wmap_effect_params[22] = 0;
    g_wmap_effect_params[23] = 0x80;
    g_wmap_effect_params[24] = 0;
    g_wmap_effect_params[25] = 3;
    g_wmap_effect_params[26] = 0x3E8;
    g_wmap_effect_params[27] = 0x8C;
    g_wmap_effect_params[28] = 0x13;
    g_wmap_effect_params[29] = 0;
    g_wmap_effect_params[30] = 0x32C8;

    do
    {
        screen_entry = (u8*)(screen_offset + screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 34);

    g_wmap_land_effect_33_sequence_8_timer = 102;
    g_wmap_land_effect_33_sequence_8_step++;
    wmap_land_effect_33_sequence_8_step_02();
}

void wmap_land_effect_33_sequence_9_step_01(void)
{
    s32 index;
    s32 screen_offset;
    s32 config_offset;
    uintptr_t config_base;
    uintptr_t screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = (uintptr_t)g_wmap_actor_motions;
    screen_base = (uintptr_t)g_wmap_actor_animations;
    resource = &g_wmap_animation_bank_2;
    screen_offset = 0x5F0;
    config_offset = 0xED8;
    g_wmap_effect_params[41] = 1;
    g_wmap_effect_params[42] = 3;
    g_wmap_effect_params[43] = 0x40;
    g_wmap_effect_params[44] = 0;
    g_wmap_effect_params[45] = 3;
    g_wmap_effect_params[46] = -0x1C2;
    g_wmap_effect_params[47] = 0xBE;
    g_wmap_effect_params[48] = 0x2C;
    g_wmap_effect_params[49] = 5;
    g_wmap_effect_params[50] = 0x6D60;

    do
    {
        screen_entry = (u8*)(screen_offset + screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 40);

    g_wmap_land_effect_33_sequence_9_timer = 120;
    g_wmap_land_effect_33_sequence_9_step++;
    wmap_land_effect_33_sequence_9_step_02();
}

/**
 * @brief Move the model along Z, draw it, and fade it until the timer expires.
 */
WMAP_STEP_MAP_DROP_UPDATE(wmap_land_effect_33_sequence_10_step_02,
    g_wmap_land_effect_33_sequence_10_step, g_wmap_land_effect_33_sequence_10_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a,
    g_wmap_load_buffer, -0xDAC,
    2)

WMAP_STEP_RUNNER(wmap_land_effect_33_run, D_800D78BC, 4, g_wmap_land_effect_33_step, g_wmap_land_effect_33_timer)

WMAP_STEP_RESET(wmap_land_effect_33_step_00, g_wmap_land_effect_33_step, g_wmap_land_effect_33_timer)

void wmap_land_effect_33_step_01(void)
{
    g_wmap_land_display_limit = 0;
    wmap_reset_focus_screen_position();
    g_wmap_focus_screen_position.point.x = 0xA4;
    g_wmap_focus_screen_position.point.y = 0x69;
    wmap_start_sequence(wmap_land_effect_33_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_33_step++;
    wmap_land_effect_33_step_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_33_step_02, g_wmap_land_effect_33_step, wmap_land_effect_33_step_03)

/** @brief Start the world-map exit and advance the sequence. */
WMAP_STEP_BEGIN_EXIT(wmap_land_effect_33_step_03, g_wmap_land_effect_33_step, 1)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_timeline, D_800D78CC, 0x16, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_33_timeline_step_00, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

void wmap_land_effect_33_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    g_wmap_placement_overlay_hidden = 1;
    wmap_play_sound(0x36, 0x80);
    g_wmap_land_effect_33_timeline_timer = 0xF;
    g_wmap_land_effect_33_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_02, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_33_timeline_step_03, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer,
                         wmap_land_effect_33_run_sequence_9, 0x23)

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_04, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

void wmap_land_effect_33_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_33_run_sequence_10);
    wmap_start_map_tint(0x703040);
    g_wmap_backdrop_target_level = 0xA;
    g_wmap_land_effect_33_timeline_timer = 0x19;
    g_wmap_land_effect_33_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_06, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_33_timeline_step_07, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer,
                             wmap_land_effect_33_run_sequence_1, wmap_land_effect_33_run_sequence_7, 0x20)

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_08, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

void wmap_land_effect_33_timeline_step_09(void)
{
    g_wmap_forced_animated_land_id = -1;
    wmap_start_sequence(wmap_land_effect_33_run_sequence_3);
    g_wmap_land_effect_33_timeline_timer = 0x36;
    g_wmap_land_effect_33_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_10, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_33_timeline_step_11, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer,
                         wmap_land_effect_33_run_sequence_6, 0x45)

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_12, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_33_timeline_step_13, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer,
                         wmap_land_effect_33_run_sequence_2, 0x12)

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_14, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_33_timeline_step_15, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer,
                         wmap_land_effect_33_run_sequence_8, 0x2B)

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_16, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

/**
 * @brief Start three sequences and set the wait timer.
 */
WMAP_STEP_START_THREE_AND_WAIT(wmap_land_effect_33_timeline_step_17,
    g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer,
    wmap_land_effect_33_run_sequence_5, wmap_land_effect_33_run_sequence_4, wmap_land_effect_33_run_sequence_10, 0x64)

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_18, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

void wmap_land_effect_33_timeline_step_19(void)
{
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = 0x121;
    akao_fade_all_sfx_volume(0x3C, 0);
    g_wmap_land_effect_33_timeline_timer = 0x5A;
    g_wmap_land_effect_33_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_20, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

/**
 * @brief Clear the blocking flag and finish the timeline.
 */
WMAP_STEP_FINISH_BLOCKING(wmap_land_effect_33_timeline_finish,
    g_wmap_land_effect_33_timeline_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_1, D_800D7924, 6, g_wmap_land_effect_33_sequence_1_step, g_wmap_land_effect_33_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_1_step_00, g_wmap_land_effect_33_sequence_1_step, g_wmap_land_effect_33_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_33_sequence_1_step_01,
    g_wmap_land_effect_33_sequence_1_step, g_wmap_land_effect_33_sequence_1_timer,
    5, &g_wmap_animation_bank_5, 0,
    0, 0x80, 4,
    0xA0, wmap_land_effect_33_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_1_step_02, g_wmap_land_effect_33_sequence_1_step, g_wmap_land_effect_33_sequence_1_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x2C, 8, 0)

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_33_sequence_1_step_03,
    g_wmap_land_effect_33_sequence_1_step, g_wmap_land_effect_33_sequence_1_timer,
    g_wmap_sprite_actors[5], 0x80, 1, wmap_land_effect_33_sequence_1_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_1_step_04, g_wmap_land_effect_33_sequence_1_step, g_wmap_land_effect_33_sequence_1_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x2C, 8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_1_step_05, g_wmap_land_effect_33_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_2, D_800D793C, 6, g_wmap_land_effect_33_sequence_2_step, g_wmap_land_effect_33_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_2_step_00, g_wmap_land_effect_33_sequence_2_step, g_wmap_land_effect_33_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
void wmap_land_effect_33_sequence_2_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = &g_wmap_animation_bank_3;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade_step = 0x80;
    actor->target_shade = 0x80;
    actor->shade = 0;
    g_wmap_land_effect_33_sequence_2_timer = 0x3E;
    g_wmap_land_effect_33_sequence_2_step++;
    wmap_land_effect_33_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_2_step_02, g_wmap_land_effect_33_sequence_2_step, g_wmap_land_effect_33_sequence_2_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x2C, 8, 0)

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_33_sequence_2_step_03,
    g_wmap_land_effect_33_sequence_2_step, g_wmap_land_effect_33_sequence_2_timer,
    g_wmap_sprite_actors[6], 0x80, 1, wmap_land_effect_33_sequence_2_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_2_step_04, g_wmap_land_effect_33_sequence_2_step, g_wmap_land_effect_33_sequence_2_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x2C, 8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_2_step_05, g_wmap_land_effect_33_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_3, D_800D7954, 6, g_wmap_land_effect_33_sequence_3_step, g_wmap_land_effect_33_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_3_step_00, g_wmap_land_effect_33_sequence_3_step, g_wmap_land_effect_33_sequence_3_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_33_sequence_3_step_01,
    g_wmap_land_effect_33_sequence_3_step, g_wmap_land_effect_33_sequence_3_timer,
    7, g_wmap_animation_bank_2, 0,
    1, 0x81, 0x80,
    0xBC, wmap_land_effect_33_sequence_3_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_3_step_02, g_wmap_land_effect_33_sequence_3_step, g_wmap_land_effect_33_sequence_3_timer, g_wmap_sprite_actors[7],
                              g_wmap_actor_animations[7], g_wmap_focus_screen_position, 0x2E, 0xC, 0)

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_33_sequence_3_step_03,
    g_wmap_land_effect_33_sequence_3_step, g_wmap_land_effect_33_sequence_3_timer,
    g_wmap_sprite_actors[7], 0x80, 1, wmap_land_effect_33_sequence_3_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_3_step_04, g_wmap_land_effect_33_sequence_3_step, g_wmap_land_effect_33_sequence_3_timer, g_wmap_sprite_actors[7],
                              g_wmap_actor_animations[7], g_wmap_focus_screen_position, 0x2E, 0xC, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_3_step_05, g_wmap_land_effect_33_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_4, D_800D796C, 6, g_wmap_land_effect_33_sequence_4_step, g_wmap_land_effect_33_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_4_step_00, g_wmap_land_effect_33_sequence_4_step, g_wmap_land_effect_33_sequence_4_timer)

void wmap_land_effect_33_sequence_4_step_01(void)
{
    s32 value;
    WmapSpriteActor* actor = &g_wmap_sprite_actors[8];

    g_wmap_actor_animations[8].data = g_wmap_animation_bank_2;
    actor->scale_index = 0xF;
    actor->sequence = value = 1;
    actor->previous_sequence = -value;
    actor->shade_step = 8;
    actor->shade = value;
    actor->resource_index = 0;
    actor->target_shade = 0x81;
    g_wmap_land_effect_33_sequence_4_timer = 0x65;
    g_wmap_land_effect_33_sequence_4_step++;
    wmap_land_effect_33_sequence_4_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_4_step_02, g_wmap_land_effect_33_sequence_4_step, g_wmap_land_effect_33_sequence_4_timer, g_wmap_sprite_actors[8],
                              g_wmap_actor_animations[8], g_wmap_focus_screen_position, 0x2E, 0xC, 0)

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_33_sequence_4_step_03,
    g_wmap_land_effect_33_sequence_4_step, g_wmap_land_effect_33_sequence_4_timer,
    g_wmap_sprite_actors[8], 0x80, 1, wmap_land_effect_33_sequence_4_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_4_step_04, g_wmap_land_effect_33_sequence_4_step, g_wmap_land_effect_33_sequence_4_timer, g_wmap_sprite_actors[8],
                              g_wmap_actor_animations[8], g_wmap_focus_screen_position, 0x2E, 0xC, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_4_step_05, g_wmap_land_effect_33_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_5, D_800D7984, 6, g_wmap_land_effect_33_sequence_5_step, g_wmap_land_effect_33_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_5_step_00, g_wmap_land_effect_33_sequence_5_step, g_wmap_land_effect_33_sequence_5_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_33_sequence_5_step_01,
    g_wmap_land_effect_33_sequence_5_step, g_wmap_land_effect_33_sequence_5_timer,
    9, g_wmap_animation_bank_2, 2,
    0, 0x80, 4,
    0x66, wmap_land_effect_33_sequence_5_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_5_step_02, g_wmap_land_effect_33_sequence_5_step, g_wmap_land_effect_33_sequence_5_timer, g_wmap_sprite_actors[9],
                              g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x2C, 8, 0)

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_33_sequence_5_step_03,
    g_wmap_land_effect_33_sequence_5_step, g_wmap_land_effect_33_sequence_5_timer,
    g_wmap_sprite_actors[9], 2, 0x40, wmap_land_effect_33_sequence_5_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_5_step_04, g_wmap_land_effect_33_sequence_5_step, g_wmap_land_effect_33_sequence_5_timer, g_wmap_sprite_actors[9],
                              g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x2C, 8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_5_step_05, g_wmap_land_effect_33_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_6, D_800D799C, 6, g_wmap_land_effect_33_sequence_6_step, g_wmap_land_effect_33_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_6_step_00, g_wmap_land_effect_33_sequence_6_step, g_wmap_land_effect_33_sequence_6_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_33_sequence_6_step_02,
    g_wmap_land_effect_33_sequence_6_step, g_wmap_land_effect_33_sequence_6_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0x32, 0xFF, 1, 8, 0, (WmapState*)g_wmap_effect_params))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_33_sequence_6_step_03,
    g_wmap_land_effect_33_sequence_6_step, g_wmap_land_effect_33_sequence_6_timer,
    g_wmap_effect_params[5], 0x20, wmap_land_effect_33_sequence_6_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_33_sequence_6_step_04,
    g_wmap_land_effect_33_sequence_6_step, g_wmap_land_effect_33_sequence_6_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0x32, 0xFF, 1, 8, 0, (WmapState*)g_wmap_effect_params))

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_6_step_05, g_wmap_land_effect_33_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_7, D_800D79B4, 6, g_wmap_land_effect_33_sequence_7_step, g_wmap_land_effect_33_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_7_step_00, g_wmap_land_effect_33_sequence_7_step, g_wmap_land_effect_33_sequence_7_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_33_sequence_7_step_02,
    g_wmap_land_effect_33_sequence_7_step, g_wmap_land_effect_33_sequence_7_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[80], &g_wmap_actor_animations[80], 0x28, 0xFF, 1, 2, 0, &g_wmap_effect_params[10]))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_33_sequence_7_step_03,
    g_wmap_land_effect_33_sequence_7_step, g_wmap_land_effect_33_sequence_7_timer,
    g_wmap_effect_params[15], 0x80, wmap_land_effect_33_sequence_7_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_33_sequence_7_step_04,
    g_wmap_land_effect_33_sequence_7_step, g_wmap_land_effect_33_sequence_7_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[80], &g_wmap_actor_animations[80], 0x28, 0xFF, 1, 2, 0, &g_wmap_effect_params[10]))

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_7_step_05, g_wmap_land_effect_33_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_8, D_800D79CC, 6, g_wmap_land_effect_33_sequence_8_step, g_wmap_land_effect_33_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_8_step_00, g_wmap_land_effect_33_sequence_8_step, g_wmap_land_effect_33_sequence_8_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_33_sequence_8_step_02,
    g_wmap_land_effect_33_sequence_8_step, g_wmap_land_effect_33_sequence_8_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[140], &g_wmap_actor_animations[140], 0x22, 0xFF, 1, 2, 0, (WmapState*)g_wmap_effect_params + 1))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_33_sequence_8_step_03,
    g_wmap_land_effect_33_sequence_8_step, g_wmap_land_effect_33_sequence_8_timer,
    g_wmap_effect_params[25], 0x80, wmap_land_effect_33_sequence_8_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_33_sequence_8_step_04,
    g_wmap_land_effect_33_sequence_8_step, g_wmap_land_effect_33_sequence_8_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[140], &g_wmap_actor_animations[140], 0x22, 0xFF, 1, 2, 0, (WmapState*)g_wmap_effect_params + 1))

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_8_step_05, g_wmap_land_effect_33_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_9, D_800D79E4, 6, g_wmap_land_effect_33_sequence_9_step, g_wmap_land_effect_33_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_9_step_00, g_wmap_land_effect_33_sequence_9_step, g_wmap_land_effect_33_sequence_9_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_33_sequence_9_step_02,
    g_wmap_land_effect_33_sequence_9_step, g_wmap_land_effect_33_sequence_9_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[190], &g_wmap_actor_animations[190], 0x28, 0xFF, 1, 4, 0, (WmapState*)g_wmap_effect_params + 2))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_33_sequence_9_step_03,
    g_wmap_land_effect_33_sequence_9_step, g_wmap_land_effect_33_sequence_9_timer,
    g_wmap_effect_params[45], 0x40, wmap_land_effect_33_sequence_9_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_33_sequence_9_step_04,
    g_wmap_land_effect_33_sequence_9_step, g_wmap_land_effect_33_sequence_9_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[190], &g_wmap_actor_animations[190], 0x28, 0xFF, 1, 4, 0, (WmapState*)g_wmap_effect_params + 2))

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_9_step_05, g_wmap_land_effect_33_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_10, D_800D79FC, 4, g_wmap_land_effect_33_sequence_10_step, g_wmap_land_effect_33_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_10_step_00, g_wmap_land_effect_33_sequence_10_step, g_wmap_land_effect_33_sequence_10_timer)

WMAP_STEP_DROP_START(wmap_land_effect_33_sequence_10_step_01, g_wmap_land_effect_33_sequence_10_step, g_wmap_land_effect_33_sequence_10_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_33_sequence_10_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_10_step_03, g_wmap_land_effect_33_sequence_10_step)
