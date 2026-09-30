#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_map_display.h"
#include "wmap_land_event_17.h"
#include "wmap_sequence_runtime.h"
#include "wmap_view_effects.h"
#include "wmap_map_labels.h"
#include "cdrom.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "sdk/rand.h"
#include "wmap_step_sequence.h"

void wmap_land_event_17_step_02(void);
void wmap_land_event_17_sequence_1_step_02(void);
void wmap_land_event_17_sequence_2_step_02(void);
void wmap_land_event_17_sequence_3_step_02(void);
void wmap_land_event_17_sequence_4_step_02(void);
void wmap_land_event_17_sequence_5_step_02(void);
void wmap_land_event_17_sequence_6_step_02(void);
void wmap_land_event_17_sequence_7_step_02(void);
void wmap_land_event_17_sequence_8_step_02(void);
void wmap_land_event_17_sequence_9_step_02(void);
void wmap_land_event_17_sequence_13_step_02(void);
void wmap_land_event_17_step_03(void);
s32 wmap_land_event_17_run_timeline(s32 arg0);
void wmap_land_event_17_wait_idle(void);
void wmap_land_event_17_step_05(void);
s32 wmap_land_event_17_run_sequence_9(s32 arg0);
s32 wmap_land_event_17_run_sequence_10(s32 arg0);
s32 wmap_land_event_17_run_sequence_1(s32 arg0);
s32 wmap_land_event_17_run_sequence_6(s32 arg0);
s32 wmap_land_event_17_run_sequence_12(s32 arg0);
s32 wmap_land_event_17_run_sequence_13(s32 arg0);
s32 wmap_land_event_17_run_sequence_4(s32 arg0);
s32 wmap_land_event_17_run_sequence_2(s32 arg0);
s32 wmap_land_event_17_run_sequence_3(s32 arg0);
s32 wmap_land_event_17_run_sequence_8(s32 arg0);
s32 wmap_land_event_17_run_sequence_11(s32 arg0);
s32 wmap_land_event_17_run_sequence_5(s32 arg0);
s32 wmap_land_event_17_run_sequence_7(s32 arg0);
void wmap_land_event_17_sequence_13_step_04(void);
void wmap_land_event_17_sequence_13_step_06(void);

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

typedef struct
{
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
    s16 unkC;
    s16 unkE;
    s32 unk10;
} WmapAfcEntry;

extern u8 D_800DEF18[];
extern s32 D_8013B258;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern u8 g_wmap_load_buffer[];
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern u8 D_80193640[];
extern s32 g_wmap_land_event_17_timeline_timer;
extern s32 g_wmap_land_event_17_sequence_1_timer;
extern s32 g_wmap_land_event_17_sequence_2_timer;
extern s32 g_wmap_land_event_17_sequence_3_timer;
extern u8 g_wmap_animation_bank_1[];
extern s32 g_wmap_land_event_17_sequence_4_timer;
extern s32 g_wmap_land_event_17_sequence_5_timer;
extern s32 g_wmap_land_event_17_sequence_6_timer;
extern u8* D_801399DC;
extern u8 g_wmap_animation_bank_2[];
extern s32 g_wmap_land_event_17_sequence_7_timer;
extern u8* D_801399E4;
extern s32 g_wmap_land_event_17_sequence_8_timer;
extern u8* D_801399EC;
extern u8 D_800D9478[];
extern s32 g_wmap_land_event_17_sequence_9_timer;
extern s32 g_wmap_land_event_17_sequence_10_timer;
extern s32 g_wmap_land_event_17_sequence_11_timer;
extern s32 g_wmap_land_event_17_sequence_12_timer;
extern s32 D_800D9154;
extern s32 D_800DCEAC;
extern s32 D_801B25D8;
extern s32 g_wmap_land_event_17_sequence_13_timer;
extern s32 g_wmap_land_event_17_timer;
extern void (*D_800D6DEC[])(void);
extern void wmap_land_event_17_step_03(void);
extern s32 D_8013B294;
extern s32 D_80139228;
extern void (*D_800D6E04[])(void);
extern void (*D_800D6EFC[])(void);
extern void (*D_800D6F0C[])(void);
extern void (*D_800D6F1C[])(void);
extern void (*D_800D6F2C[])(void);
extern void (*D_800D6F3C[])(void);
extern void (*D_800D6F4C[])(void);
extern void (*D_800D6F5C[])(void);
extern void (*D_800D6F6C[])(void);
extern void (*D_800D6F7C[])(void);
extern void (*D_800D6F8C[])(void);
extern void (*D_800D6F9C[])(void);
extern void (*D_800D6FAC[])(void);
extern void (*D_800D6FBC[])(void);
extern u8 *g_wmap_effect_model_pack_1;
extern u8 *g_wmap_effect_model_pack_2;
extern u32 g_wmap_land_event_17_step;
extern u8 g_wmap_animation_bank_0[];
extern u32 g_wmap_land_event_17_timeline_step;
extern u32 g_wmap_land_event_17_sequence_1_step;
extern u32 g_wmap_land_event_17_sequence_2_step;
extern u32 g_wmap_land_event_17_sequence_3_step;
extern u32 g_wmap_land_event_17_sequence_4_step;
extern u32 g_wmap_land_event_17_sequence_5_step;
extern u32 g_wmap_land_event_17_sequence_6_step;
extern u32 g_wmap_land_event_17_sequence_7_step;
extern u32 g_wmap_land_event_17_sequence_8_step;
extern u32 g_wmap_land_event_17_sequence_9_step;
extern u32 g_wmap_land_event_17_sequence_10_step;
extern u32 g_wmap_land_event_17_sequence_11_step;
extern u32 g_wmap_land_event_17_sequence_12_step;
extern u32 g_wmap_land_event_17_sequence_13_step;

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


extern WmapSpriteActor D_800D939C;
extern WmapSpriteActor D_800D93C8;
extern WmapSpriteActor D_800D93F4;
extern WmapSpriteActor D_800D9420;
extern WmapSpriteActor D_800D944C;

extern WmapAnimationSlot g_wmap_actor_animations[];
extern WmapAnimationSlot D_801399C0;
extern WmapAnimationSlot D_801399C8;
extern WmapAnimationSlot D_801399D0;
extern WmapAnimationSlot D_801399D8;
extern WmapAnimationSlot D_801399E0;
extern WmapAnimationSlot D_801399E8;

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D58;
extern WmapScreenPosition D_80182D60;
extern WmapScreenPosition D_80182D64;
extern WmapScreenPosition D_80182D6C;
extern WmapScreenPosition D_80182D7C;
extern WmapScreenPosition D_80182D84;
extern WmapScreenPosition D_80182D90;
extern WmapScreenPosition D_80182D98;
extern WmapScreenPosition D_80182DB8;

extern WmapAfcEntry g_wmap_actor_motions[];

/** @brief Set the effect resources and map-relative position, then advance. */
void wmap_land_event_17_step_01(void)
{
    g_wmap_effect_model_pack_1 = D_800DEF18;
    g_wmap_effect_model_pack_2 = D_800DEF18 + 0x2000;
    g_wmap_focus_screen_position.point.x = 0x94;
    g_wmap_focus_screen_position.point.y = 0x31;
    D_8013B258 = 1;
    wmap_find_land_cell(0x11, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    g_wmap_land_event_17_step++;
    wmap_land_event_17_step_02();
}

/** @brief Set transition controls, queue resources, and start the loading countdown. */
void wmap_land_event_17_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x301020);
    g_wmap_backdrop_target_level = 4;
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_spirit_target_brightness = 0;
    func_8005FF88(-1);
    cdrom_wait_queue_empty();
    cdrom_queue_read(0x1205, D_80182E40);
    cdrom_queue_read(0x1206, D_8018B240);
    cdrom_queue_read(0x1207, D_80193640);
    cdrom_queue_read(0x1208, g_wmap_animation_bank_0);
    cdrom_queue_read(0x1209, g_wmap_animation_bank_0 + 0x2000);
    cdrom_queue_read(0x120A, g_wmap_animation_bank_0 + 0x4000);
    cdrom_queue_read(0x120B, g_wmap_load_buffer);
    cdrom_queue_read(0x120C, g_wmap_effect_model_pack_1);
    cdrom_queue_read(0x120D, g_wmap_effect_model_pack_2);
    g_wmap_land_event_17_timeline_timer = 0x1E;
    g_wmap_land_event_17_timeline_step += 1;
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void wmap_land_event_17_sequence_1_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[4];

    g_wmap_actor_animations[4].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 0x2;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->target_shade = 0x81;
    actor->shade = 0x81;
    D_80182D58.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D58.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    g_wmap_land_event_17_sequence_1_timer = 0x24;
    g_wmap_land_event_17_sequence_1_step += 1;
    wmap_land_event_17_sequence_1_step_02();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void wmap_land_event_17_sequence_2_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 0x2;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->target_shade = 0x81;
    actor->shade = 0x81;
    D_80182D60.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D60.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    g_wmap_land_event_17_sequence_2_timer = 0x24;
    g_wmap_land_event_17_sequence_2_step += 1;
    wmap_land_event_17_sequence_2_step_02();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void wmap_land_event_17_sequence_3_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 0x2;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->target_shade = 0x81;
    actor->shade = 0x81;
    D_80182D64.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D64.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    g_wmap_land_event_17_sequence_3_timer = 0x24;
    g_wmap_land_event_17_sequence_3_step += 1;
    wmap_land_event_17_sequence_3_step_02();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void wmap_land_event_17_sequence_4_step_01(void)
{
    g_wmap_actor_animations[9].data = g_wmap_animation_bank_1;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 0x2;
    D_800D93F4.resource_index = 0;
    D_800D93F4.sequence = 0;
    D_800D93F4.target_shade = 0x81;
    D_800D93F4.shade = 0x81;
    D_80182D6C.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D6C.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    g_wmap_land_event_17_sequence_4_timer = 0x24;
    g_wmap_land_event_17_sequence_4_step += 1;
    wmap_land_event_17_sequence_4_step_02();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void wmap_land_event_17_sequence_5_step_01(void)
{
    g_wmap_actor_animations[8].data = g_wmap_animation_bank_1;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 0x2;
    D_800D93C8.resource_index = 0;
    D_800D93C8.sequence = 0;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.shade = 0x81;
    D_80182D7C.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D7C.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    g_wmap_land_event_17_sequence_5_timer = 0x24;
    g_wmap_land_event_17_sequence_5_step += 1;
    wmap_land_event_17_sequence_5_step_02();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void wmap_land_event_17_sequence_6_step_01(void)
{
    g_wmap_actor_animations[7].data = g_wmap_animation_bank_1;
    D_800D939C.scale_index = 0xF;
    D_800D939C.previous_sequence = -1;
    D_800D939C.shade_step = 0x2;
    D_800D939C.resource_index = 0;
    D_800D939C.sequence = 0;
    D_800D939C.target_shade = 0x81;
    D_800D939C.shade = 0x81;
    D_80182D84.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D84.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    g_wmap_land_event_17_sequence_6_timer = 0x24;
    g_wmap_land_event_17_sequence_6_step += 1;
    wmap_land_event_17_sequence_6_step_02();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void wmap_land_event_17_sequence_7_step_01(void)
{
    D_801399DC = g_wmap_animation_bank_2;
    D_800D9420.scale_index = 0xF;
    D_800D9420.previous_sequence = -1;
    D_800D9420.shade_step = 0x2;
    D_800D9420.resource_index = 0;
    D_800D9420.sequence = 0;
    D_800D9420.target_shade = 0x81;
    D_800D9420.shade = 0x81;
    D_80182D90.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D90.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    g_wmap_land_event_17_sequence_7_timer = 0x24;
    g_wmap_land_event_17_sequence_7_step += 1;
    wmap_land_event_17_sequence_7_step_02();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void wmap_land_event_17_sequence_8_step_01(void)
{
    D_801399E4 = g_wmap_animation_bank_2;
    D_800D944C.scale_index = 0xF;
    D_800D944C.previous_sequence = -1;
    D_800D944C.shade_step = 0x2;
    D_800D944C.resource_index = 0;
    D_800D944C.sequence = 0;
    D_800D944C.target_shade = 0x81;
    D_800D944C.shade = 0x81;
    D_80182D98.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D98.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    g_wmap_land_event_17_sequence_8_timer = 0x24;
    g_wmap_land_event_17_sequence_8_step += 1;
    wmap_land_event_17_sequence_8_step_02();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void wmap_land_event_17_sequence_9_step_01(void)
{
    D_801399EC = g_wmap_animation_bank_2;
    D_800D9478[0x6] = 0xF;
    *(s16*)&D_800D9478[0x10] = -1;
    *(s16*)&D_800D9478[0x26] = 0x2;
    *(s16*)&D_800D9478[0x2] = 0;
    *(s16*)&D_800D9478[0xE] = 0;
    *(s16*)&D_800D9478[0x22] = 0x81;
    *(s16*)&D_800D9478[0x24] = 0x81;
    D_80182DB8.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182DB8.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    g_wmap_land_event_17_sequence_9_timer = 0x24;
    g_wmap_land_event_17_sequence_9_step += 1;
    wmap_land_event_17_sequence_9_step_02();
}

/** @brief Approach the effect depth, draw its fading layer, and advance the countdown. */
void wmap_land_event_17_sequence_10_step_02(void)
{
    MATRIX transform;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = g_wmap_effect_model_a_position.vz - 3500;
    g_wmap_effect_model_a_position.vz = depth;
    if (depth < 10000)
    {
        g_wmap_effect_model_a_position.vz = 10000;
    }
    PushMatrix();
    RotMatrix(&g_wmap_effect_model_a_rotation, &transform);
    TransMatrix(&transform, &g_wmap_zero_translation);
    SetRotMatrix(&transform);
    SetTransMatrix(&transform);
    if (g_wmap_effect_fade_a != 0)
    {
        wmap_draw_model(g_wmap_load_buffer, 0, 4, 53, 0x7800, 1, g_wmap_effect_fade_a, 50, -20, -1);
        intensity = g_wmap_effect_fade_a - 4;
        g_wmap_effect_fade_a = intensity;
        if (intensity < 0)
        {
            g_wmap_effect_fade_a = 0;
        }
    }
    PopMatrix();
    remaining = g_wmap_land_event_17_sequence_10_timer - 1;
    g_wmap_land_event_17_sequence_10_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_event_17_sequence_10_step++;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_event_17_sequence_11_step_02(void)
{
    MATRIX m;
    s32 x;
    s32 timer;

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
        wmap_draw_model(g_wmap_effect_model_pack_1, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_b, -0x19, -0x32, -1);
        g_wmap_effect_fade_b -= 8;
        if (g_wmap_effect_fade_b < 0)
        {
            g_wmap_effect_fade_b = 0;
        }
    }

    PopMatrix();
    timer = g_wmap_land_event_17_sequence_11_timer - 1;
    g_wmap_land_event_17_sequence_11_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_event_17_sequence_11_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_event_17_sequence_12_step_02(void)
{
    MATRIX m;
    s32 x;
    s32 timer;

    x = g_wmap_effect_model_c_position.vz - 0xDAC;
    g_wmap_effect_model_c_position.vz = x;
    if (x < 0x2710)
    {
        g_wmap_effect_model_c_position.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&g_wmap_effect_model_c_rotation, &m);
    TransMatrix(&m, &g_wmap_zero_translation);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (g_wmap_effect_fade_c != 0)
    {
        wmap_draw_model(g_wmap_effect_model_pack_2, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_c, 0xF, -0x37, -1);
        g_wmap_effect_fade_c -= 8;
        if (g_wmap_effect_fade_c < 0)
        {
            g_wmap_effect_fade_c = 0;
        }
    }

    PopMatrix();
    timer = g_wmap_land_event_17_sequence_12_timer - 1;
    g_wmap_land_event_17_sequence_12_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_event_17_sequence_12_step += 1;
    }
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_event_17_sequence_13_step_01(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 80;
    D_801B25D8 = 1;
    D_800DCEAC = 1;

    do
    {
        g_wmap_actor_motions[i].unk0 = 0;
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_0;
        entry = &g_wmap_sprite_actors[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 1;
        entry->unk10 = -1;
        i++;
    } while (i < 124);

    D_800D9154 = 2;
    g_wmap_land_event_17_sequence_13_timer = 0x10;
    g_wmap_land_event_17_sequence_13_step += 1;
    wmap_land_event_17_sequence_13_step_02();
}

WMAP_STEP_RUNNER(wmap_land_event_17_run, D_800D6DEC, 0x6, g_wmap_land_event_17_step, g_wmap_land_event_17_timer)

WMAP_STEP_RESET(wmap_land_event_17_reset, g_wmap_land_event_17_step, g_wmap_land_event_17_timer)

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void wmap_land_event_17_step_02(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_land_event_17_step += 1;
        wmap_land_event_17_step_03();
    }
}

WMAP_STEP_START_BLOCKING(wmap_land_event_17_step_03, g_wmap_land_event_17_step, wmap_land_event_17_run_timeline, wmap_land_event_17_wait_idle)

WMAP_STEP_WAIT_IDLE(wmap_land_event_17_wait_idle, g_wmap_land_event_17_step, wmap_land_event_17_step_05)

/** @brief World-map trigger: set two flags and bump a counter. */
void wmap_land_event_17_step_05(void)
{
    D_8013B294 = 1;
    D_80139228 = 1;
    g_wmap_land_event_17_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_event_17_run_timeline, D_800D6E04, 0x3E, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_RESET(wmap_land_event_17_timeline_reset, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_02, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

/** @brief Wait for queued CD work, initialize three buffers, and register the next callback. */
void wmap_land_event_17_timeline_step_03(void)
{
    cdrom_wait_queue_empty();
    wmap_play_sound(0x2D, 0x80);
    func_800651B4(&D_80182E40);
    func_800651B4(&D_8018B240);
    func_800651B4(&D_80193640);
    wmap_start_sequence(&wmap_land_event_17_run_sequence_9);
    g_wmap_land_event_17_timeline_timer = 0x12;
    g_wmap_land_event_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_04, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_event_17_timeline_step_05, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                         wmap_land_event_17_run_sequence_10, 0xC)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_06, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

/** @brief World-map step: register the four draw callbacks and tick the frame counter. */
void wmap_land_event_17_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_event_17_run_sequence_1);
    wmap_start_sequence(wmap_land_event_17_run_sequence_6);
    wmap_start_sequence(wmap_land_event_17_run_sequence_12);
    wmap_start_sequence(wmap_land_event_17_run_sequence_13);
    g_wmap_land_event_17_timeline_timer = 4;
    g_wmap_land_event_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_08, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_09, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_4, wmap_land_event_17_run_sequence_2, 0x14)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_10, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_11, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_3, wmap_land_event_17_run_sequence_8, 0x4)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_12, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_event_17_timeline_step_13, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                         wmap_land_event_17_run_sequence_9, 0x8)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_14, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_15, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_11, wmap_land_event_17_run_sequence_5, 0xE)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_16, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_17, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_7, wmap_land_event_17_run_sequence_6, 0x10)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_18, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_event_17_timeline_step_19(void)
{
    wmap_start_sequence(wmap_land_event_17_run_sequence_12);
    wmap_start_sequence(wmap_land_event_17_run_sequence_8);
    wmap_start_sequence(wmap_land_event_17_run_sequence_4);
    g_wmap_land_event_17_timeline_timer = 0x18;
    g_wmap_land_event_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_20, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_21, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_9, wmap_land_event_17_run_sequence_3, 0x8)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_22, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_23, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_5, wmap_land_event_17_run_sequence_2, 0xC)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_24, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_event_17_timeline_step_25(void)
{
    wmap_start_sequence(wmap_land_event_17_run_sequence_1);
    wmap_start_sequence(wmap_land_event_17_run_sequence_6);
    wmap_start_sequence(wmap_land_event_17_run_sequence_12);
    g_wmap_land_event_17_timeline_timer = 0x4;
    g_wmap_land_event_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_26, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

/** @brief Register a sequence callback, play sound 45, and start a 20-tick delay. */
void wmap_land_event_17_timeline_step_27(void)
{
    wmap_start_sequence(&wmap_land_event_17_run_sequence_4);
    wmap_play_sound(0x2D, 0x80);
    g_wmap_land_event_17_timeline_timer = 0x14;
    g_wmap_land_event_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_28, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_29, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_3, wmap_land_event_17_run_sequence_8, 0x4)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_30, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_event_17_timeline_step_31, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                         wmap_land_event_17_run_sequence_9, 0x8)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_32, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_33, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_11, wmap_land_event_17_run_sequence_5, 0xE)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_34, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_35, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_7, wmap_land_event_17_run_sequence_6, 0x10)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_36, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_event_17_timeline_step_37(void)
{
    wmap_start_sequence(wmap_land_event_17_run_sequence_12);
    wmap_start_sequence(wmap_land_event_17_run_sequence_8);
    wmap_start_sequence(wmap_land_event_17_run_sequence_4);
    g_wmap_land_event_17_timeline_timer = 0x18;
    g_wmap_land_event_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_38, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

/** @brief Play sound 45, register two callbacks, and begin an eight-tick delay. */
void wmap_land_event_17_timeline_step_39(void)
{
    wmap_play_sound(0x2D, 0x80);
    wmap_start_sequence(&wmap_land_event_17_run_sequence_9);
    wmap_start_sequence(&wmap_land_event_17_run_sequence_3);
    g_wmap_land_event_17_timeline_timer = 8;
    g_wmap_land_event_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_40, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_41, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_5, wmap_land_event_17_run_sequence_2, 0xC)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_42, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

/** @brief World-map step: register the four draw callbacks and tick the frame counter. */
void wmap_land_event_17_timeline_step_43(void)
{
    wmap_start_sequence(wmap_land_event_17_run_sequence_1);
    wmap_start_sequence(wmap_land_event_17_run_sequence_6);
    wmap_start_sequence(wmap_land_event_17_run_sequence_12);
    wmap_start_sequence(wmap_land_event_17_run_sequence_13);
    g_wmap_land_event_17_timeline_timer = 4;
    g_wmap_land_event_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_44, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_event_17_timeline_step_45, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                         wmap_land_event_17_run_sequence_4, 0x14)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_46, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_47, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_3, wmap_land_event_17_run_sequence_8, 0x4)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_48, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_event_17_timeline_step_49, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                         wmap_land_event_17_run_sequence_9, 0x8)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_50, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_51, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_11, wmap_land_event_17_run_sequence_5, 0xE)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_52, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_53, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_7, wmap_land_event_17_run_sequence_6, 0x10)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_54, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_event_17_timeline_step_55(void)
{
    wmap_start_sequence(wmap_land_event_17_run_sequence_12);
    wmap_start_sequence(wmap_land_event_17_run_sequence_8);
    wmap_start_sequence(wmap_land_event_17_run_sequence_4);
    g_wmap_land_event_17_timeline_timer = 0x18;
    g_wmap_land_event_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_56, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_event_17_timeline_step_57(void)
{
    wmap_start_sequence(wmap_land_event_17_run_sequence_9);
    wmap_start_sequence(wmap_land_event_17_run_sequence_10);
    wmap_start_sequence(wmap_land_event_17_run_sequence_3);
    g_wmap_land_event_17_timeline_timer = 0x8;
    g_wmap_land_event_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_58, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_17_timeline_step_59, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer,
                             wmap_land_event_17_run_sequence_5, wmap_land_event_17_run_sequence_2, 0x54)

WMAP_STEP_WAIT(wmap_land_event_17_timeline_wait_60, g_wmap_land_event_17_timeline_step, g_wmap_land_event_17_timeline_timer)

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void wmap_land_event_17_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_land_event_17_timeline_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_1, D_800D6EFC, 0x4, g_wmap_land_event_17_sequence_1_step, g_wmap_land_event_17_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_1_reset, g_wmap_land_event_17_sequence_1_step, g_wmap_land_event_17_sequence_1_timer)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_17_sequence_1_step_02, g_wmap_land_event_17_sequence_1_step, g_wmap_land_event_17_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], D_80182D58, 0x8, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_1_end, g_wmap_land_event_17_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_2, D_800D6F0C, 0x4, g_wmap_land_event_17_sequence_2_step, g_wmap_land_event_17_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_2_reset, g_wmap_land_event_17_sequence_2_step, g_wmap_land_event_17_sequence_2_timer)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_17_sequence_2_step_02, g_wmap_land_event_17_sequence_2_step, g_wmap_land_event_17_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], D_80182D60, 0x20, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_2_end, g_wmap_land_event_17_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_3, D_800D6F1C, 0x4, g_wmap_land_event_17_sequence_3_step, g_wmap_land_event_17_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_3_reset, g_wmap_land_event_17_sequence_3_step, g_wmap_land_event_17_sequence_3_timer)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_17_sequence_3_step_02, g_wmap_land_event_17_sequence_3_step, g_wmap_land_event_17_sequence_3_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], D_80182D64, 0x21, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_3_end, g_wmap_land_event_17_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_4, D_800D6F2C, 0x4, g_wmap_land_event_17_sequence_4_step, g_wmap_land_event_17_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_4_reset, g_wmap_land_event_17_sequence_4_step, g_wmap_land_event_17_sequence_4_timer)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_17_sequence_4_step_02, g_wmap_land_event_17_sequence_4_step, g_wmap_land_event_17_sequence_4_timer, D_800D93F4,
                              D_801399D0, D_80182D6C, 0x24, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_4_end, g_wmap_land_event_17_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_5, D_800D6F3C, 0x4, g_wmap_land_event_17_sequence_5_step, g_wmap_land_event_17_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_5_reset, g_wmap_land_event_17_sequence_5_step, g_wmap_land_event_17_sequence_5_timer)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_17_sequence_5_step_02, g_wmap_land_event_17_sequence_5_step, g_wmap_land_event_17_sequence_5_timer, D_800D93C8,
                              D_801399C8, D_80182D7C, 0x11, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_5_end, g_wmap_land_event_17_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_6, D_800D6F4C, 0x4, g_wmap_land_event_17_sequence_6_step, g_wmap_land_event_17_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_6_reset, g_wmap_land_event_17_sequence_6_step, g_wmap_land_event_17_sequence_6_timer)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_17_sequence_6_step_02, g_wmap_land_event_17_sequence_6_step, g_wmap_land_event_17_sequence_6_timer, D_800D939C,
                              D_801399C0, D_80182D84, 0x23, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_6_end, g_wmap_land_event_17_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_7, D_800D6F5C, 0x4, g_wmap_land_event_17_sequence_7_step, g_wmap_land_event_17_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_7_reset, g_wmap_land_event_17_sequence_7_step, g_wmap_land_event_17_sequence_7_timer)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_17_sequence_7_step_02, g_wmap_land_event_17_sequence_7_step, g_wmap_land_event_17_sequence_7_timer, D_800D9420,
                              D_801399D8, D_80182D90, 0x25, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_7_end, g_wmap_land_event_17_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_8, D_800D6F6C, 0x4, g_wmap_land_event_17_sequence_8_step, g_wmap_land_event_17_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_8_reset, g_wmap_land_event_17_sequence_8_step, g_wmap_land_event_17_sequence_8_timer)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_17_sequence_8_step_02, g_wmap_land_event_17_sequence_8_step, g_wmap_land_event_17_sequence_8_timer, D_800D944C,
                              D_801399E0, D_80182D98, 0x26, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_8_end, g_wmap_land_event_17_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_9, D_800D6F7C, 0x4, g_wmap_land_event_17_sequence_9_step, g_wmap_land_event_17_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_9_reset, g_wmap_land_event_17_sequence_9_step, g_wmap_land_event_17_sequence_9_timer)

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_event_17_sequence_9_step_02(void)
{
    wmap_step_actor_animation(D_800D9478, &D_801399E8);
    wmap_draw_actor_sprite(D_800D9478, D_80182DB8.packed, 0x27, 0x2, 0);
    if (--g_wmap_land_event_17_sequence_9_timer == 0)
    {
        g_wmap_land_event_17_sequence_9_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_9_end, g_wmap_land_event_17_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_10, D_800D6F8C, 0x4, g_wmap_land_event_17_sequence_10_step, g_wmap_land_event_17_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_10_reset, g_wmap_land_event_17_sequence_10_step, g_wmap_land_event_17_sequence_10_timer)

WMAP_STEP_DROP_START(wmap_land_event_17_sequence_10_step_01, g_wmap_land_event_17_sequence_10_step, g_wmap_land_event_17_sequence_10_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x20, wmap_land_event_17_sequence_10_step_02)

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_10_end, g_wmap_land_event_17_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_11, D_800D6F9C, 0x4, g_wmap_land_event_17_sequence_11_step, g_wmap_land_event_17_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_11_reset, g_wmap_land_event_17_sequence_11_step, g_wmap_land_event_17_sequence_11_timer)

WMAP_STEP_DROP_START(wmap_land_event_17_sequence_11_step_01, g_wmap_land_event_17_sequence_11_step, g_wmap_land_event_17_sequence_11_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x10, wmap_land_event_17_sequence_11_step_02)

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_11_end, g_wmap_land_event_17_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_12, D_800D6FAC, 0x4, g_wmap_land_event_17_sequence_12_step, g_wmap_land_event_17_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_12_reset, g_wmap_land_event_17_sequence_12_step, g_wmap_land_event_17_sequence_12_timer)

WMAP_STEP_DROP_START(wmap_land_event_17_sequence_12_step_01, g_wmap_land_event_17_sequence_12_step, g_wmap_land_event_17_sequence_12_timer, g_wmap_effect_model_c_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, 0x80, 0xAFC8, 0x10, wmap_land_event_17_sequence_12_step_02)

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_12_end, g_wmap_land_event_17_sequence_12_step)

WMAP_STEP_RUNNER(wmap_land_event_17_run_sequence_13, D_800D6FBC, 0x8, g_wmap_land_event_17_sequence_13_step, g_wmap_land_event_17_sequence_13_timer)

WMAP_STEP_RESET(wmap_land_event_17_sequence_13_reset, g_wmap_land_event_17_sequence_13_step, g_wmap_land_event_17_sequence_13_timer)

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_event_17_sequence_13_step_02(void)
{
    s32 remaining;

    func_8006B328(0x50, 0x7C, 2, -1, 1, 2, 0x78, 8, -0x32, 0x64, -0x28, 0x50, 0x64, 0, 0x81, 2, 1);
    D_801B25D8 += 8;
    remaining = g_wmap_land_event_17_sequence_13_timer - 1;
    g_wmap_land_event_17_sequence_13_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_event_17_sequence_13_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_event_17_sequence_13_step_03, g_wmap_land_event_17_sequence_13_step, g_wmap_land_event_17_sequence_13_timer, 0x58,
                    wmap_land_event_17_sequence_13_step_04)

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_event_17_sequence_13_step_04(void)
{
    func_8006B328(0x50, 0x7C, 2, -1, 1, 2, 0x78, 8, -0x32, 0x64, -0x28, 0x50, 0x64, 0, 0x81, 2, 1);
    if (--g_wmap_land_event_17_sequence_13_timer == 0)
    {
        g_wmap_land_event_17_sequence_13_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_event_17_sequence_13_step_05(void)
{
    D_800DCEAC = 0;
    g_wmap_land_event_17_sequence_13_timer = 0x64;
    g_wmap_land_event_17_sequence_13_step += 1;
    wmap_land_event_17_sequence_13_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_event_17_sequence_13_step_06(void)
{
    func_8006B328(0x50, 0x7C, 2, -1, 1, 2, 0x78, 8, -0x32, 0x64, -0x28, 0x50, 0x64, 0, 0x81, 2, 1);
    if (--g_wmap_land_event_17_sequence_13_timer == 0)
    {
        g_wmap_land_event_17_sequence_13_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_event_17_sequence_13_end, g_wmap_land_event_17_sequence_13_step)
