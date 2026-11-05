#include "common.h"
#include <libgte.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_effect_resources.h"
#include "../internal/wmap_land_effect_24.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_model_render.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"
#include "main/cdrom.h"

/** @brief Fixed-point height units per screen pixel during the sprite descent. */
#define WMAP_EFFECT24_HEIGHT_SCALE 16

extern WmapSequenceStep g_wmap_land_effect_24_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_timeline_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_1_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_2_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_3_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_4_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_6_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_7_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_8_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_9_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_10_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_15_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_14_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_13_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_12_steps[];
extern WmapSequenceStep g_wmap_land_effect_24_sequence_11_steps[];
extern s32 g_wmap_load_buffer[];
extern u32 g_wmap_land_effect_24_step;
extern s32 g_wmap_land_effect_24_timer;
extern u32 g_wmap_land_effect_24_timeline_step;
extern s32 g_wmap_land_effect_24_timeline_timer;
extern u32 g_wmap_land_effect_24_sequence_1_step;
extern s32 g_wmap_land_effect_24_sequence_1_timer;
extern u32 g_wmap_land_effect_24_sequence_2_step;
extern s32 g_wmap_land_effect_24_sequence_2_timer;
extern u32 g_wmap_land_effect_24_sequence_3_step;
extern s32 g_wmap_land_effect_24_sequence_3_timer;
extern u32 g_wmap_land_effect_24_sequence_4_step;
extern s32 g_wmap_land_effect_24_sequence_4_timer;
extern u32 g_wmap_land_effect_24_sequence_5_step;
extern s32 g_wmap_land_effect_24_sequence_5_timer;
extern u32 g_wmap_land_effect_24_sequence_6_step;
extern s32 g_wmap_land_effect_24_sequence_6_timer;
extern u32 g_wmap_land_effect_24_sequence_7_step;
extern s32 g_wmap_land_effect_24_sequence_7_timer;
extern u32 g_wmap_land_effect_24_sequence_8_step;
extern s32 g_wmap_land_effect_24_sequence_8_timer;
extern u32 g_wmap_land_effect_24_sequence_9_step;
extern s32 g_wmap_land_effect_24_sequence_9_timer;
extern u32 g_wmap_land_effect_24_sequence_10_step;
extern s32 g_wmap_land_effect_24_sequence_10_timer;
extern u32 g_wmap_land_effect_24_sequence_11_step;
extern s32 g_wmap_land_effect_24_sequence_11_timer;
extern u32 g_wmap_land_effect_24_sequence_12_step;
extern s32 g_wmap_land_effect_24_sequence_12_timer;
extern u32 g_wmap_land_effect_24_sequence_13_step;
extern s32 g_wmap_land_effect_24_sequence_13_timer;
extern u32 g_wmap_land_effect_24_sequence_14_step;
extern s32 g_wmap_land_effect_24_sequence_14_timer;
extern u32 g_wmap_land_effect_24_sequence_15_step;
extern s32 g_wmap_land_effect_24_sequence_15_timer;

extern s32* g_wmap_effect_params;

/** @brief Start sequences 7, 2, 1 and 3, hide the transition mesh, tint the map and fade the backdrop to dark red. */
void wmap_land_effect_24_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_24_run_sequence_7);
    g_wmap_transition_mesh_hidden = 1;
    wmap_start_sequence(wmap_land_effect_24_run_sequence_2);
    wmap_start_sequence(wmap_land_effect_24_run_sequence_1);
    wmap_start_sequence(wmap_land_effect_24_run_sequence_3);
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 0;
    WMAP_SET_BACKDROP_TARGET(0x38, 0, 0x18, 0x38, 0, 0x18);
    g_wmap_land_effect_24_timeline_timer = 8;
    g_wmap_land_effect_24_timeline_step++;
}

/** @brief Start sequence 12, install a camera move that tilts and raises the view, and fade the backdrop's bottom edge to black. */
void wmap_land_effect_24_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_24_run_sequence_12);
    wmap_install_callback(wmap_run_effect_camera_move);
    g_wmap_effect_camera_rotation_step.vx = 5;
    g_wmap_effect_camera_rotation_step.vy = 0;
    g_wmap_effect_camera_rotation_step.vz = 0;
    g_wmap_effect_camera_translation_step.vx = 0;
    g_wmap_effect_camera_translation_step.vy = 2;
    g_wmap_effect_camera_translation_step.vz = 0;
    g_wmap_effect_camera_rotation_target.vx = 0xFA;
    g_wmap_effect_camera_rotation_target.vy = 0;
    g_wmap_effect_camera_rotation_target.vz = 0;
    g_wmap_effect_camera_translation_target.vx = 0;
    g_wmap_effect_camera_translation_target.vy = 0x64;
    g_wmap_effect_camera_translation_target.vz = 0;
    g_wmap_backdrop_target_level = 0;
    WMAP_SET_BACKDROP_TARGET(0x38, 0, 0x18, 0, 0, 0);
    g_wmap_land_effect_24_timeline_timer = 0x70;
    g_wmap_land_effect_24_timeline_step++;
}

/**
 * @brief Fade and draw the live focus sprites, then every other frame copy actor 7 into the first retired slot.
 * @note A slot is live while its resource_index is 0; a faded-out sprite is retired with -1.
 */
void wmap_land_effect_24_update_focus_sprites(void)
{
    s32 index;
    uintptr_t config_offset;
    u16 value;
    uintptr_t config_base;
    WmapSpriteActor* actor;
    void* entry;

    for (index = 0xAA; index < 0xB9; index++)
    {
        config_offset = index * 0x2C + 0xB0;
        config_base = (uintptr_t)g_wmap_sprite_actors;
        entry = (void*)(config_offset + config_base);
        actor = &g_wmap_sprite_actors[index + 4];

        if (*(s16*)(entry + 2) == 0)
        {
            wmap_draw_actor_sprite((void*)actor, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
            value = actor->shade - 5;
            actor->shade = value;

            if ((s32)(value << 0x10) <= 0)
            {
                *(s16*)(entry + 2) = (value = -1);
            }
        }
    }

    index = 0xAA;

    if ((g_wmap_frame_count & 1) == 0)
    {
        void* entry;
        u8* copy_end;
        uintptr_t config_offset;
        s32 screen_offset;
        u8* scan_base;
        u8* screen_base;

        scan_base = (u8*)g_wmap_sprite_actors;
        copy_end = scan_base + 0x154;
        screen_base = (u8*)g_wmap_actor_animations;
        config_offset = 0x1DE8;
        screen_offset = 0x570;

        while (index < 0xB9)
        {
            entry = (void*)(config_offset + (uintptr_t)scan_base);

            if (((WmapSpriteActor*)entry)->resource_index != 0)
            {
                typedef struct
                {
                    s32 words[4];
                } Chunk;
                typedef struct
                {
                    s32 words[3];
                } Tail;
                u8* source;
                source = scan_base + 0x134;

                do
                {
                    *(Chunk*)entry = *(Chunk*)source;
                    source += 16;
                    entry += 16;
                } while (source != copy_end);
                *(Tail*)entry = *(Tail*)source;
                *(WmapAnimationSlot*)(screen_offset + (uintptr_t)screen_base) = *(WmapAnimationSlot*)(screen_base + 0x38);
                ((WmapSpriteActor*)(config_offset + (uintptr_t)scan_base))->target_shade = 0;
                return;
            }

            config_offset += 0x2C;
            index++;
            screen_offset += 8;
        }
    }
}

/**
 * @brief Move the model along Z, draw it, and fade it until the timer expires.
 */
WMAP_STEP_MAP_DROP_UPDATE(wmap_land_effect_24_sequence_7_step_02, g_wmap_land_effect_24_sequence_7_step, g_wmap_land_effect_24_sequence_7_timer,
                          g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, g_wmap_load_buffer, -0xDAC, 2)

/**
 * @brief Move the model along Z, draw it, and fade it until the timer expires.
 */
WMAP_STEP_MAP_DROP_UPDATE(wmap_land_effect_24_sequence_8_step_02, g_wmap_land_effect_24_sequence_8_step, g_wmap_land_effect_24_sequence_8_timer,
                          g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -0xDAC, 1)

/**
 * @brief Move the model along Z, draw it, and fade it until the timer expires.
 */
WMAP_STEP_MAP_DROP_UPDATE(wmap_land_effect_24_sequence_9_step_02, g_wmap_land_effect_24_sequence_9_step, g_wmap_land_effect_24_sequence_9_timer,
                          g_wmap_effect_model_c_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, g_wmap_effect_model_pack_2, -0xDAC, 8)

/**
 * @brief Initialize the particle slots and run their first update.
 */
void wmap_land_effect_24_sequence_10_step_01(void)
{
    s32 i;

    g_wmap_effect_params[1] = 0;
    g_wmap_effect_params[2] = 0;
    g_wmap_effect_params[3] = 0x80;
    g_wmap_effect_params[4] = 0;
    g_wmap_effect_params[5] = 3;
    g_wmap_effect_params[6] = 0x384;
    g_wmap_effect_params[7] = 0x14;
    g_wmap_effect_params[8] = 8;
    g_wmap_effect_params[9] = 1;
    g_wmap_effect_params[10] = 0x1F40;
    WMAP_RESET_PARTICLE_SLOTS(i, 60, g_wmap_actor_motions[i + 20].state, 20, g_wmap_animation_bank_2);
    g_wmap_land_effect_24_sequence_10_timer = 0xB4;
    g_wmap_land_effect_24_sequence_10_step++;
    wmap_land_effect_24_sequence_10_step_02();
}

/**
 * @brief Assign animation resources to the effect actors and start the next step.
 */
void wmap_land_effect_24_sequence_11_step_01(void)
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
    resource = g_wmap_animation_bank_2;
    screen_offset = 0x280;
    config_offset = 0x640;
    g_wmap_effect_params[11] = 1;
    g_wmap_effect_params[12] = 4;
    g_wmap_effect_params[13] = 0x40;
    g_wmap_effect_params[15] = 3;
    g_wmap_effect_params[16] = -0x1C2;
    g_wmap_effect_params[17] = 0x50;
    g_wmap_effect_params[18] = 8;
    g_wmap_effect_params[19] = 2;
    g_wmap_effect_params[14] = 0;
    g_wmap_effect_params[20] = 0x61A8;

    while (index < 0x1E)
    {
        screen_entry = (u8*)(screen_offset + screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        ((WmapAnimationSlot*)screen_entry)->data = resource;
    }

    g_wmap_land_effect_24_sequence_11_timer = 0x5A;
    g_wmap_land_effect_24_sequence_11_step++;
    wmap_land_effect_24_sequence_11_step_02();
}

/**
 * @brief Initialize the particle actors and run their first update.
 */
void wmap_land_effect_24_sequence_12_step_01(void)
{
    s32 i;

    g_wmap_aux_effect_fade_a = 1;
    g_wmap_falling_spawn_counts[2] = 12;

    for (i = 120; i < 156; i++)
    {
        g_wmap_actor_motions[i].state = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, g_wmap_animation_bank_0, (i & 1) + 2);
    }

    g_wmap_falling_spawn_timers[2] = 2;
    g_wmap_land_effect_24_sequence_12_timer = 0x10;
    g_wmap_land_effect_24_sequence_12_step++;
    wmap_land_effect_24_sequence_12_step_02();
}

/**
 * @brief Update and draw the model while fading its color in.
 */
void wmap_land_effect_24_sequence_13_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_effect_camera_transform();
    wmap_draw_model(g_wmap_effect_model_pack_3, (D_80139240 >> 4) & 7, 0xA, 0x35, 0x7800, 1, g_wmap_effect_fade_d, 0, 0, -1);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_d, 8, 0x81, value);
    D_80139240 += 0x10;
    timer = g_wmap_land_effect_24_sequence_13_timer - 1;
    g_wmap_land_effect_24_sequence_13_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_13_step++;
    }
}

/**
 * @brief Update and draw the model while fading its color out.
 */
void wmap_land_effect_24_sequence_13_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_effect_camera_transform();
    wmap_draw_model(g_wmap_effect_model_pack_3, (D_80139240 >> 4) & 7, 0xA, 0x35, 0x7800, 1, g_wmap_effect_fade_d, 0, 0, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_d, 4, value);
    D_80139240 += 0x10;
    timer = g_wmap_land_effect_24_sequence_13_timer - 1;
    g_wmap_land_effect_24_sequence_13_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_13_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_effect_24_sequence_14_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_effect_model_g_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_4, (D_8013924C >> 4) & 7, 0xB0, 0x35, 0x7800, 1, D_801B25D8, 4, -0x44, -1);
    WMAP_MODEL_FADE_IN(D_801B25D8, 2, 0x80, value);
    D_8013924C += 0x18;
    timer = g_wmap_land_effect_24_sequence_14_timer - 1;
    g_wmap_land_effect_24_sequence_14_timer = timer;
    g_wmap_effect_model_g_rotation.vz += 0x28;

    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_14_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_effect_24_sequence_14_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_effect_model_g_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_4, (D_8013924C >> 4) & 7, 0xB0, 0x35, 0x7800, 1, D_801B25D8, 4, -0x44, -1);
    WMAP_MODEL_FADE_OUT(D_801B25D8, 0x10, value);
    D_8013924C += 0x18;
    timer = g_wmap_land_effect_24_sequence_14_timer - 1;
    g_wmap_land_effect_24_sequence_14_timer = timer;
    g_wmap_effect_model_g_rotation.vz += 0x28;

    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_14_step++;
    }
}

WMAP_STEP_RUNNER(wmap_land_effect_24_run, g_wmap_land_effect_24_steps, 6, g_wmap_land_effect_24_step, g_wmap_land_effect_24_timer)

WMAP_STEP_RESET(wmap_land_effect_24_step_00, g_wmap_land_effect_24_step, g_wmap_land_effect_24_timer)

/** @brief Start the land focus sequence and mark the map busy, then run step 2. */
void wmap_land_effect_24_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_24_step++;
    wmap_land_effect_24_step_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_24_step_02, g_wmap_land_effect_24_step, wmap_land_effect_24_step_03)

/** @brief Start the timeline and mark the map busy, then run step 4. */
void wmap_land_effect_24_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_24_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_24_step++;
    wmap_land_effect_24_step_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_24_step_04, g_wmap_land_effect_24_step, wmap_land_effect_24_step_05)

WMAP_STEP_ADVANCE(wmap_land_effect_24_step_05, g_wmap_land_effect_24_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_timeline, g_wmap_land_effect_24_timeline_steps, 0x1A, g_wmap_land_effect_24_timeline_step,
                 g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_24_timeline_step_00, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

/** @brief Take over the map, play the effect sound and start the effect camera move. */
void wmap_land_effect_24_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_play_sound(0x35, 0x80);
    g_wmap_effect_camera_move_active = 1;
    g_wmap_land_effect_24_timeline_timer = 4;
    g_wmap_land_effect_24_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_02, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

/** @brief Start sequence 4, tint the map and raise the backdrop level to 10. */
void wmap_land_effect_24_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_24_run_sequence_4);
    wmap_start_map_tint(0x605060);
    g_wmap_backdrop_target_level = 0xA;
    g_wmap_land_effect_24_timeline_timer = 0x12;
    g_wmap_land_effect_24_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_04, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

/** @brief Hide the placement overlay and set the backdrop level to 5. */
void wmap_land_effect_24_timeline_step_05(void)
{
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_backdrop_target_level = 5;
    g_wmap_land_effect_24_timeline_timer = 0xE;
    g_wmap_land_effect_24_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_06, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_08, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_24_timeline_step_09, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer,
                         wmap_land_effect_24_run_sequence_10, 4)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_10, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_24_timeline_step_11, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer,
                         wmap_land_effect_24_run_sequence_13, 0x5C)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_12, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_24_timeline_step_13, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer,
                         wmap_land_effect_24_run_sequence_15, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_14, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_16, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_24_timeline_step_17, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer,
                         wmap_land_effect_24_run_sequence_14, 0x5A)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_18, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

/** @brief Start sequences 8, 5 and 6, show the transition mesh, tint the map and stop the camera move. */
void wmap_land_effect_24_timeline_step_19(void)
{
    wmap_start_sequence(wmap_land_effect_24_run_sequence_8);
    g_wmap_backdrop_target_level = 7;
    g_wmap_transition_mesh_hidden = 0;
    wmap_start_map_tint(0x703080);
    g_wmap_effect_camera_move_active = 0;
    wmap_start_sequence(wmap_land_effect_24_run_sequence_5);
    wmap_start_sequence(wmap_land_effect_24_run_sequence_6);
    g_wmap_land_effect_24_timeline_timer = 4;
    g_wmap_land_effect_24_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_20, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_24_timeline_step_21, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer,
                         wmap_land_effect_24_run_sequence_11, 0xB7)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_22, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

/** @brief Start sequence 9, restore the map tint and backdrop, and place the selected artifact's land in the focused cell. */
void wmap_land_effect_24_timeline_step_23(void)
{
    wmap_start_sequence(wmap_land_effect_24_run_sequence_9);
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    g_wmap_land_effect_24_timeline_timer = 0x3C;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = g_wmap_selected_artifact | WMAP_LAND_FULL_SIZE;
    g_wmap_land_effect_24_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_24, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

/**
 * @brief Clear the blocking flag and finish the timeline.
 */
WMAP_STEP_FINISH_BLOCKING(wmap_land_effect_24_timeline_finish, g_wmap_land_effect_24_timeline_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_1, g_wmap_land_effect_24_sequence_1_steps, 6, g_wmap_land_effect_24_sequence_1_step,
                 g_wmap_land_effect_24_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_1_step_00, g_wmap_land_effect_24_sequence_1_step, g_wmap_land_effect_24_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_24_sequence_1_step_01, g_wmap_land_effect_24_sequence_1_step, g_wmap_land_effect_24_sequence_1_timer, 4,
                      g_wmap_animation_bank_1, 0, 1, 0x81, 4, 0x88, wmap_land_effect_24_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_24_sequence_1_step_02, g_wmap_land_effect_24_sequence_1_step, g_wmap_land_effect_24_sequence_1_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xB, 2, 0)

/** @brief Start fading out sprite actor 4, then run step 4. */
void wmap_land_effect_24_sequence_1_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[4];

    actor->target_shade = 0;
    actor->shade_step = 8;
    g_wmap_land_effect_24_sequence_1_timer = 0x10;
    g_wmap_land_effect_24_sequence_1_step++;
    wmap_land_effect_24_sequence_1_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_24_sequence_1_step_04, g_wmap_land_effect_24_sequence_1_step, g_wmap_land_effect_24_sequence_1_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xB, 2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_1_step_05, g_wmap_land_effect_24_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_2, g_wmap_land_effect_24_sequence_2_steps, 6, g_wmap_land_effect_24_sequence_2_step,
                 g_wmap_land_effect_24_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_2_step_00, g_wmap_land_effect_24_sequence_2_step, g_wmap_land_effect_24_sequence_2_timer)

/** @brief Set up sprite actor 5 with animation bank 1 and fade it in, then run step 2. */
void wmap_land_effect_24_sequence_2_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->shade = 1;
    actor->resource_index = 0;
    actor->target_shade = 0x81;
    g_wmap_land_effect_24_sequence_2_timer = 0x88;
    g_wmap_land_effect_24_sequence_2_step++;
    wmap_land_effect_24_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_24_sequence_2_step_02, g_wmap_land_effect_24_sequence_2_step, g_wmap_land_effect_24_sequence_2_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0xB, 2, 0)

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_24_sequence_2_step_03, g_wmap_land_effect_24_sequence_2_step, g_wmap_land_effect_24_sequence_2_timer,
                     g_wmap_sprite_actors[5], 8, 0x10, wmap_land_effect_24_sequence_2_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_24_sequence_2_step_04, g_wmap_land_effect_24_sequence_2_step, g_wmap_land_effect_24_sequence_2_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0xB, 2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_2_step_05, g_wmap_land_effect_24_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_3, g_wmap_land_effect_24_sequence_3_steps, 6, g_wmap_land_effect_24_sequence_3_step,
                 g_wmap_land_effect_24_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_3_step_00, g_wmap_land_effect_24_sequence_3_step, g_wmap_land_effect_24_sequence_3_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_24_sequence_3_step_01, g_wmap_land_effect_24_sequence_3_step, g_wmap_land_effect_24_sequence_3_timer, 6,
                      g_wmap_animation_bank_1, 2, 0x81, 0x81, 8, 0x88, wmap_land_effect_24_sequence_3_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_24_sequence_3_step_02, g_wmap_land_effect_24_sequence_3_step, g_wmap_land_effect_24_sequence_3_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0xB, 2, 0)

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_24_sequence_3_step_03, g_wmap_land_effect_24_sequence_3_step, g_wmap_land_effect_24_sequence_3_timer,
                     g_wmap_sprite_actors[6], 2, 0x40, wmap_land_effect_24_sequence_3_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_24_sequence_3_step_04, g_wmap_land_effect_24_sequence_3_step, g_wmap_land_effect_24_sequence_3_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0xB, 2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_3_step_05, g_wmap_land_effect_24_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_4, g_wmap_land_effect_24_sequence_4_steps, 6, g_wmap_land_effect_24_sequence_4_step,
                 g_wmap_land_effect_24_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_4_step_00, g_wmap_land_effect_24_sequence_4_step, g_wmap_land_effect_24_sequence_4_timer)

/** @brief Set up sprite actor 7 with animation bank 1, retire focus sprites 174-188 and set the spawn countdown, then run step 2. */
void wmap_land_effect_24_sequence_4_step_01(void)
{
    s32 i;
    WmapSpriteActor* actor;

    actor = &g_wmap_sprite_actors[7];
    g_wmap_actor_animations[7].data = g_wmap_animation_bank_1;
    D_80139268 = 0x1E;
    actor->scale_index = 0xF;
    actor->sequence = 3;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->target_shade = 0x81;
    actor->resource_index = 0;
    actor->shade = 1;

    for (i = 0xAA; i < 0xB9; i++)
    {
        g_wmap_sprite_actors[i + 4].resource_index = -1;
    }

    g_wmap_land_effect_24_sequence_4_timer = 0xAA;
    g_wmap_land_effect_24_sequence_4_step++;
    wmap_land_effect_24_sequence_4_step_02();
}

/** @brief Animate and draw sprite actor 7 at the focus and update the focus sprites while the spawn countdown runs, until the timer ends. */
void wmap_land_effect_24_sequence_4_step_02(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[7], &g_wmap_actor_animations[7]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[7], g_wmap_focus_screen_position.packed, 0xB, 2, 0);

    if (D_80139268 > 0)
    {
        wmap_land_effect_24_update_focus_sprites();
    }

    timer = g_wmap_land_effect_24_sequence_4_timer - 1;
    D_80139268--;
    g_wmap_land_effect_24_sequence_4_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_4_step++;
    }
}

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_24_sequence_4_step_03, g_wmap_land_effect_24_sequence_4_step, g_wmap_land_effect_24_sequence_4_timer,
                     g_wmap_sprite_actors[7], 8, 0x10, wmap_land_effect_24_sequence_4_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_24_sequence_4_step_04, g_wmap_land_effect_24_sequence_4_step, g_wmap_land_effect_24_sequence_4_timer,
                              g_wmap_sprite_actors[7], g_wmap_actor_animations[7], g_wmap_focus_screen_position, 0xB, 2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_4_step_05, g_wmap_land_effect_24_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_5, g_wmap_land_effect_24_sequence_5_steps, 4, g_wmap_land_effect_24_sequence_5_step,
                 g_wmap_land_effect_24_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_5_step_00, g_wmap_land_effect_24_sequence_5_step, g_wmap_land_effect_24_sequence_5_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_24_sequence_5_step_01, g_wmap_land_effect_24_sequence_5_step, g_wmap_land_effect_24_sequence_5_timer, 5,
                      g_wmap_animation_bank_0, 0, 0, 0x80, 8, 0xDC, wmap_land_effect_24_sequence_5_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_24_sequence_5_step_02, g_wmap_land_effect_24_sequence_5_step, g_wmap_land_effect_24_sequence_5_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x1F, 0xC, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_5_step_03, g_wmap_land_effect_24_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_6, g_wmap_land_effect_24_sequence_6_steps, 4, g_wmap_land_effect_24_sequence_6_step,
                 g_wmap_land_effect_24_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_6_step_00, g_wmap_land_effect_24_sequence_6_step, g_wmap_land_effect_24_sequence_6_timer)

/** @brief Set up sprite actor 9 with animation bank 0 and fade it in, then run step 2. */
void wmap_land_effect_24_sequence_6_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[9];

    g_wmap_actor_animations[9].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->shade = 1;
    actor->resource_index = 0;
    actor->target_shade = 0x81;
    g_wmap_land_effect_24_sequence_6_timer = 0xBD;
    g_wmap_land_effect_24_sequence_6_step++;
    wmap_land_effect_24_sequence_6_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_24_sequence_6_step_02, g_wmap_land_effect_24_sequence_6_step, g_wmap_land_effect_24_sequence_6_timer,
                              g_wmap_sprite_actors[9], g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x2D, 0x1E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_6_step_03, g_wmap_land_effect_24_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_7, g_wmap_land_effect_24_sequence_7_steps, 4, g_wmap_land_effect_24_sequence_7_step,
                 g_wmap_land_effect_24_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_7_step_00, g_wmap_land_effect_24_sequence_7_step, g_wmap_land_effect_24_sequence_7_timer)

WMAP_STEP_DROP_START(wmap_land_effect_24_sequence_7_step_01, g_wmap_land_effect_24_sequence_7_step, g_wmap_land_effect_24_sequence_7_timer,
                     g_wmap_effect_model_a_rotation, g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40,
                     wmap_land_effect_24_sequence_7_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_7_step_03, g_wmap_land_effect_24_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_8, g_wmap_land_effect_24_sequence_8_steps, 4, g_wmap_land_effect_24_sequence_8_step,
                 g_wmap_land_effect_24_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_8_step_00, g_wmap_land_effect_24_sequence_8_step, g_wmap_land_effect_24_sequence_8_timer)

WMAP_STEP_DROP_START(wmap_land_effect_24_sequence_8_step_01, g_wmap_land_effect_24_sequence_8_step, g_wmap_land_effect_24_sequence_8_timer,
                     g_wmap_effect_model_b_rotation, g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x80,
                     wmap_land_effect_24_sequence_8_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_8_step_03, g_wmap_land_effect_24_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_9, g_wmap_land_effect_24_sequence_9_steps, 4, g_wmap_land_effect_24_sequence_9_step,
                 g_wmap_land_effect_24_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_9_step_00, g_wmap_land_effect_24_sequence_9_step, g_wmap_land_effect_24_sequence_9_timer)

WMAP_STEP_DROP_START(wmap_land_effect_24_sequence_9_step_01, g_wmap_land_effect_24_sequence_9_step, g_wmap_land_effect_24_sequence_9_timer,
                     g_wmap_effect_model_c_rotation, g_wmap_zero_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, 0x80, 0xAFC8, 0x10,
                     wmap_land_effect_24_sequence_9_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_9_step_03, g_wmap_land_effect_24_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_10, g_wmap_land_effect_24_sequence_10_steps, 6, g_wmap_land_effect_24_sequence_10_step,
                 g_wmap_land_effect_24_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_10_step_00, g_wmap_land_effect_24_sequence_10_step, g_wmap_land_effect_24_sequence_10_timer)

/**
 * @brief Run the effect update until the timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_24_sequence_10_step_02, g_wmap_land_effect_24_sequence_10_step, g_wmap_land_effect_24_sequence_10_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0x3C, 0xFF, 1, 2, 0, g_wmap_effect_params))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_24_sequence_10_step_03, g_wmap_land_effect_24_sequence_10_step, g_wmap_land_effect_24_sequence_10_timer,
                       g_wmap_effect_params[5], 0x80, wmap_land_effect_24_sequence_10_step_04)

/**
 * @brief Run the effect update until the timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_24_sequence_10_step_04, g_wmap_land_effect_24_sequence_10_step, g_wmap_land_effect_24_sequence_10_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0x3C, 0xFF, 1, 2, 0, g_wmap_effect_params))

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_10_step_05, g_wmap_land_effect_24_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_11, g_wmap_land_effect_24_sequence_11_steps, 6, g_wmap_land_effect_24_sequence_11_step,
                 g_wmap_land_effect_24_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_11_step_00, g_wmap_land_effect_24_sequence_11_step, g_wmap_land_effect_24_sequence_11_timer)

/**
 * @brief Run the effect update until the timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_24_sequence_11_step_02, g_wmap_land_effect_24_sequence_11_step, g_wmap_land_effect_24_sequence_11_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[80], &g_wmap_actor_animations[80], 0x1E, 0xFF, 1, 4, 0,
                                                          (u8*)g_wmap_effect_params + 0x28))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_24_sequence_11_step_03, g_wmap_land_effect_24_sequence_11_step, g_wmap_land_effect_24_sequence_11_timer,
                       g_wmap_effect_params[15], 0x40, wmap_land_effect_24_sequence_11_step_04)

/**
 * @brief Run the effect update until the timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_24_sequence_11_step_04, g_wmap_land_effect_24_sequence_11_step, g_wmap_land_effect_24_sequence_11_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[80], &g_wmap_actor_animations[80], 0x1E, 0xFF, 1, 4, 0,
                                                          (u8*)g_wmap_effect_params + 0x28))

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_11_step_05, g_wmap_land_effect_24_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_12, g_wmap_land_effect_24_sequence_12_steps, 8, g_wmap_land_effect_24_sequence_12_step,
                 g_wmap_land_effect_24_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_12_step_00, g_wmap_land_effect_24_sequence_12_step, g_wmap_land_effect_24_sequence_12_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_24_sequence_12_step_02, g_wmap_land_effect_24_sequence_12_step, g_wmap_land_effect_24_sequence_12_timer,
                          g_wmap_aux_effect_fade_a, 8,
                          wmap_update_falling_particles(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2))

WMAP_STEP_ARM_TIMER(wmap_land_effect_24_sequence_12_step_03, g_wmap_land_effect_24_sequence_12_step, g_wmap_land_effect_24_sequence_12_timer, 0x12,
                    wmap_land_effect_24_sequence_12_step_04)

/**
 * @brief Update the particles until the timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_24_sequence_12_step_04, g_wmap_land_effect_24_sequence_12_step, g_wmap_land_effect_24_sequence_12_timer,
                          wmap_update_falling_particles(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_24_sequence_12_step_05, g_wmap_land_effect_24_sequence_12_step, g_wmap_land_effect_24_sequence_12_timer,
                               g_wmap_falling_spawn_counts[2], 0x64, wmap_land_effect_24_sequence_12_step_06)

/**
 * @brief Update the particles until the timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_24_sequence_12_step_06, g_wmap_land_effect_24_sequence_12_step, g_wmap_land_effect_24_sequence_12_timer,
                          wmap_update_falling_particles(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2))

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_12_step_07, g_wmap_land_effect_24_sequence_12_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_13, g_wmap_land_effect_24_sequence_13_steps, 6, g_wmap_land_effect_24_sequence_13_step,
                 g_wmap_land_effect_24_sequence_13_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_13_step_00, g_wmap_land_effect_24_sequence_13_step, g_wmap_land_effect_24_sequence_13_timer)

/** @brief Reset the model fade levels, then run step 2. */
void wmap_land_effect_24_sequence_13_step_01(void)
{
    D_80139240 = 0;
    g_wmap_effect_fade_d = 1;
    g_wmap_land_effect_24_sequence_13_timer = 0x5A;
    g_wmap_land_effect_24_sequence_13_step++;
    wmap_land_effect_24_sequence_13_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_24_sequence_13_step_03, g_wmap_land_effect_24_sequence_13_step, g_wmap_land_effect_24_sequence_13_timer, 0x20,
                    wmap_land_effect_24_sequence_13_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_13_step_05, g_wmap_land_effect_24_sequence_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_14, g_wmap_land_effect_24_sequence_14_steps, 6, g_wmap_land_effect_24_sequence_14_step,
                 g_wmap_land_effect_24_sequence_14_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_14_step_00, g_wmap_land_effect_24_sequence_14_step, g_wmap_land_effect_24_sequence_14_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
void wmap_land_effect_24_sequence_14_step_01(void)
{
    D_8013924C = 0;
    g_wmap_effect_model_g_rotation = g_wmap_zero_rotation;
    D_801B25D8 = 0;
    g_wmap_land_effect_24_sequence_14_timer = 0x58;
    g_wmap_land_effect_24_sequence_14_step++;
    wmap_land_effect_24_sequence_14_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_24_sequence_14_step_03, g_wmap_land_effect_24_sequence_14_step, g_wmap_land_effect_24_sequence_14_timer, 8,
                    wmap_land_effect_24_sequence_14_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_14_step_05, g_wmap_land_effect_24_sequence_14_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_15, g_wmap_land_effect_24_sequence_15_steps, 6, g_wmap_land_effect_24_sequence_15_step,
                 g_wmap_land_effect_24_sequence_15_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_15_step_00, g_wmap_land_effect_24_sequence_15_step, g_wmap_land_effect_24_sequence_15_timer)

/** @brief Set up sprite actor 10 with animation bank 2 high above the focus, then run step 2. */
void wmap_land_effect_24_sequence_15_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[10];

    g_wmap_actor_animations[10].data = g_wmap_animation_bank_2;
    D_80139250 = 0x780;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 4;
    actor->target_shade = 0x81;
    actor->shade = 1;
    actor->resource_index = 0;
    actor->sequence = 0;
    g_wmap_land_effect_24_sequence_15_timer = 0xE1;
    D_80182D58.packed = g_wmap_focus_screen_position.packed;
    g_wmap_land_effect_24_sequence_15_step++;
    wmap_land_effect_24_sequence_15_step_02();
}

/**
 * @brief Draw the descending sprite and advance when its timer expires.
 * @note D_80139250 stores height in sixteenths of a pixel.
 */
void wmap_land_effect_24_sequence_15_step_02(void)
{
    s32 height;
    s32 remaining_frames;

    wmap_step_actor_animation(&g_wmap_sprite_actors[10], &g_wmap_actor_animations[10]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[10], D_80182D58.packed, 8, 0x14, 0);

    height = D_80139250;
    D_80182D58.point.y = height / WMAP_EFFECT24_HEIGHT_SCALE;

    if (height >= 0x321)
    {
        D_80139250 = height - WMAP_EFFECT24_HEIGHT_SCALE;
    }

    remaining_frames = g_wmap_land_effect_24_sequence_15_timer - 1;
    g_wmap_land_effect_24_sequence_15_timer = remaining_frames;

    if (remaining_frames == 0)
    {
        g_wmap_land_effect_24_sequence_15_step++;
    }
}

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_24_sequence_15_step_03, g_wmap_land_effect_24_sequence_15_step, g_wmap_land_effect_24_sequence_15_timer,
                     g_wmap_sprite_actors[10], 8, 0x10, wmap_land_effect_24_sequence_15_step_04)

/**
 * @brief Draw the descending sprite and advance when its timer expires.
 * @note D_80139250 stores height in sixteenths of a pixel.
 */
void wmap_land_effect_24_sequence_15_step_04(void)
{
    s32 height;
    s32 remaining_frames;

    wmap_step_actor_animation(&g_wmap_sprite_actors[10], &g_wmap_actor_animations[10]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[10], D_80182D58.packed, 8, 0x14, 0);

    height = D_80139250;
    D_80182D58.point.y = height / WMAP_EFFECT24_HEIGHT_SCALE;

    if (height >= 0x321)
    {
        D_80139250 = height - WMAP_EFFECT24_HEIGHT_SCALE;
    }

    remaining_frames = g_wmap_land_effect_24_sequence_15_timer - 1;
    g_wmap_land_effect_24_sequence_15_timer = remaining_frames;

    if (remaining_frames == 0)
    {
        g_wmap_land_effect_24_sequence_15_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_15_step_05, g_wmap_land_effect_24_sequence_15_step)
