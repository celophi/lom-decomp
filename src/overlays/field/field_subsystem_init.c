/** @file field_subsystem_init.c
 * @brief Initialize the field subsystems and their persistent state.
 */

#include "common.h"
#include "field_contact_geometry.h"
#include "field_actor_runtime.h"
#include "field_calls.h"
#include "field_menu_element.h"
#include "field_modal_runtime.h"
#include "game_audio.h"
#include "game_state.h"
#include "main.h"
#include "field_runtime.h"
#include "menu.h"

/** @brief Fixed work areas of the field overlay: CD read buffer and actor resource heap. */
#define FIELD_CD_BUFFER_AREA ((u8*)0x80140000)
#define FIELD_ACTOR_HEAP_AREA ((u8*)0x80158000)

/** @brief g_music_track_table entry of a track index that has no music. */
#define FIELD_MUSIC_TRACK_NONE 0xFF

extern s32 g_field_dialog_screen_mode;
extern FieldRenderHalf* g_field_render_context;
extern s32 g_field_actions_limited;
extern s32 g_field_interaction_active;
extern u8* g_field_actor_heap;
extern u8* g_field_cd_buffer;
extern s32 g_field_preserve_entry_music;
extern s32 g_field_scene_mode_bit;
extern s32 D_801178C8;
extern s32 D_8011F428;
extern s32 D_80122710;
extern s32 g_field_actor_text_count;
extern s32 g_field_ring_menu_state;
extern s32 g_field_gover_load_countdown;
extern s32 D_801227F0;
extern s32 g_field_return_to_title_prompt_delay;
extern s32 g_field_return_to_title_prompt_state;
extern s32 D_800F2298;
extern s32 g_field_hide_actor_panels;
extern s32 g_field_modal_state;

/**
 * @brief Reset every field subsystem for a freshly entered scene.
 *
 * Also decides whether the entry music keeps playing: it does when the
 * field is entered from the world map and the current track has music.
 *
 * @param render_context Render context to install as the active one.
 * @see decomp.me (100%) https://decomp.me/scratch/fKHnZ
 */
void field_initialize_subsystems(FieldRenderHalf* render_context)
{
    s32 previous_state;

    g_field_cd_buffer = FIELD_CD_BUFFER_AREA;
    g_field_actor_heap = FIELD_ACTOR_HEAP_AREA;
    g_field_card_clock_valid = 0;
    field_capture_card_clock();
    field_bind_saved_game_context();
    field_reset_actor_resource_slots();
    g_field_render_context = render_context;
    g_field_scene_mode_bit = 0;
    D_801178C8 = 0;
    field_bind_builtin_animations();
    field_upload_common_texture();
    field_initialize_actor_slots();
    g_field_interaction_active = 0;
    D_80122710 = 0;
    field_rebuild_party_actions(0);
    field_clear_actor_slots();
    field_reset_draw_state();
    field_reset_actor_resources();
    field_reset_object_states();
    field_command_history_reset();
    field_reset_action_command_maps();
    field_pair_indicators_reset();
    field_reset_text_session();
    field_reset_fade_state();
    field_load_actor_sequence_data();
    g_field_actor_text_count = 0;
    field_clear_actor_texts();
    g_field_ring_menu_state = 0;
    g_field_gover_load_countdown = 0;
    g_field_dialog_screen_mode = 0;
    g_field_return_to_title_prompt_state = 0;
    g_field_return_to_title_prompt_delay = 0;
    g_field_hide_actor_panels = 0;
    g_field_actions_limited = 0;
    g_field_modal_state = 0;
    D_800F2298 = 0;
    D_8011F428 = 0;
    D_801227F0 = 0;
    g_field_audio_timer = 0;
    previous_state = g_previous_game_state;
    if (previous_state == GAME_STATE_WORLD_MAP && g_music_track_table[g_music_track_index] != FIELD_MUSIC_TRACK_NONE)
    {
        g_field_preserve_entry_music = previous_state;
    }
    else
    {
        g_field_preserve_entry_music = 0;
    }
    field_reset_battle_entry();
    field_reset_input_repeat();
    field_clear_fade_prims();
    field_reset_music_stream();
    field_reset_ring_selections();
    field_load_menu_frame_image();
}
