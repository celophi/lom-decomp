/** @file field_subsystem_init.c
 * @brief Initialize the field subsystems and their persistent state.
 */

#include "common.h"
#include "field_modal_runtime.h"
#include "game_audio.h"

void field_clear_actor_slots(void);
void field_initialize_actor_slots(void);
void field_reset_fade_state(void);
void func_80067AA4(void);
void field_reset_actor_resource_slots(void);
void func_80083948(void);
void func_8008396C(void);
void func_80084240(void);
void func_80084524(void);
void func_80086F20(void);
void func_80091410(void);
void field_load_actor_sequence_data(void);
void func_800A255C(void);
void func_800A2DFC(void);
void func_800A3EBC(void);
void func_800A43C0(void);
void func_800A6204(void);
void field_bind_saved_game_context(void);
void field_reset_input_repeat(void);
void field_reset_text_session(void);
void field_rebuild_party_actions(s32);
void func_800ADE2C(void);
void func_800B0094(void);
void func_800B01FC(s32);

extern s32 D_800F229C;
extern s32 g_field_render_context;
extern s32 D_8010AE54;
extern s32 D_8010AE78;
extern s32 D_8010D034;
extern s32 D_8010D038;
extern s32 g_field_preserve_entry_music;
extern s32 g_field_scene_mode_bit;
extern s32 D_801178C8;
extern s32 D_8011F428;
extern s32 D_80122710;
extern s32 D_801227DC;
extern s32 D_801227E8;
extern s32 D_8011F3AC;
extern s32 g_field_gover_load_countdown;
extern s32 D_801227F0;
extern s32 g_field_return_to_title_prompt_delay;
extern s32 g_field_return_to_title_prompt_state;
extern s32 g_previous_game_state;
extern s32 g_field_audio_timer;
extern u16 g_music_track_index;
extern s32 D_800F2298;
extern s32 g_field_hide_actor_panels;
extern s32 g_field_modal_state;

/**
 * @brief Reset every field subsystem for a freshly entered scene.
 *
 * Also decides whether the entry music keeps playing: it does when the
 * previous game state was 1 and the current track is valid.
 *
 * @param render_context Render context to install as the active one.
 * @see decomp.me (100%) https://decomp.me/scratch/fKHnZ
 */
void field_initialize_subsystems(s32 render_context)
{
    s32 prev;
    s32 base;

    base = 0x80158000;
    D_8010D038 = 0x80140000;
    D_8010D034 = base;
    D_801227E8 = 0;
    func_800B0094();
    field_bind_saved_game_context();
    field_reset_actor_resource_slots();
    g_field_render_context = render_context;
    g_field_scene_mode_bit = 0;
    D_801178C8 = 0;
    func_80083948();
    func_8008396C();
    field_initialize_actor_slots();
    D_8010AE78 = 0;
    D_80122710 = 0;
    field_rebuild_party_actions(0);
    field_clear_actor_slots();
    func_80067AA4();
    func_80084240();
    func_80084524();
    func_800A255C();
    func_80091410();
    func_800A2DFC();
    field_reset_text_session();
    field_reset_fade_state();
    field_load_actor_sequence_data();
    D_801227DC = 0;
    func_800A6204();
    D_8011F3AC = 0;
    g_field_gover_load_countdown = 0;
    D_800F229C = 0;
    g_field_return_to_title_prompt_state = 0;
    g_field_return_to_title_prompt_delay = 0;
    g_field_hide_actor_panels = 0;
    D_8010AE54 = 0;
    g_field_modal_state = 0;
    D_800F2298 = 0;
    D_8011F428 = 0;
    D_801227F0 = 0;
    g_field_audio_timer = 0;
    prev = g_previous_game_state;
    if (prev == 1)
    {
        /* the unused table pointer is kept: it loads the table address before the index */
        u16* track_index = &g_music_track_index;
        u8* track_table = g_music_track_table;

        if (g_music_track_table[*track_index] != 0xFF)
        {
            g_field_preserve_entry_music = prev;
        }
        else
        {
            g_field_preserve_entry_music = 0;
        }
    }
    else
    {
        g_field_preserve_entry_music = 0;
    }
    func_800B01FC(prev);
    field_reset_input_repeat();
    func_80086F20();
    func_800A3EBC();
    func_800A43C0();
    func_800ADE2C();
}
