/** @file field_frame_commands.c
 * @brief Construct per-frame field command buffers.
 */

#include "common.h"
#include "field_actor_runtime.h"
#include "field_calls.h"
#include "field_modal_runtime.h"
#include "field_runtime.h"
#include "field_text.h"

void func_8008B73C(void);
/* Private render-context views: their types live in field_dialog_screens.c. */
void func_800A5794(s32);
void func_800A64D0(s32);
void field_update_input_repeat(void);
void field_process_input(s32);
/* Defined as (void); the call passes render_half, which the original loads into $a0. */
void func_800AD118(s32);
extern s32 g_field_action_context;
extern s32 D_800F2288;
extern s32 D_800F2298;
extern s32 g_field_gover_load_countdown;
extern s32 g_field_active_group;
extern s32 g_field_pickup_sound_played;
extern s32 g_field_hide_actor_panels;
extern s32 D_8011F3AC;
extern s32 g_field_modal_state;
extern s32 g_field_text_session_active;
extern s32 g_field_scene_request_pending;
extern s32 g_frame_counter;

/**
 * @brief Run one frame of field logic and build its draw commands.
 *
 * Reads input, updates the fade, HUD, actors, dialogs, modals and text
 * session, then emits their packets into @p render_half. Actor updates are
 * skipped while a modal, text session or game-over load is active, and the
 * frame stops early when a scene change was requested.
 *
 * @param render_half Render half being drawn.
 * @param alternate Non-zero when drawing the alternate half.
 */
void field_build_frame_commands(s32 render_half, s32 alternate)
{
    D_800F2288 = render_half;
    g_field_pickup_sound_played = 0;
    g_field_action_context &= 0xFF;
    field_update_input_repeat();
    field_process_input(render_half);
    field_update_and_render_fade((FieldRenderHalf*)render_half);
    func_800B0244();
    if (g_field_active_group != 0)
    {
        if (g_field_hide_actor_panels == 0)
        {
            field_draw_actor_hud((u8*)render_half);
        }
    }
    if ((D_800F2298 == 0) && (g_field_gover_load_countdown == 0) && (g_field_modal_state == 0) && (g_field_text_session_active == 0))
    {
        func_800B19FC();
        if (g_field_scene_request_pending != 0)
        {
            return;
        }
        if (D_8011F3AC == 0)
        {
            field_update_actor_objects();
        }
    }
    func_800A4798((u8*)render_half);
    func_80096B54();
    if ((D_800F2298 == 0) && (g_field_gover_load_countdown == 0) && (g_field_modal_state == 0) && (D_8011F3AC == 0) && (g_field_text_session_active == 0))
    {
        field_update_actor_animations();
    }
    field_prepare_actor_render_commands(render_half, alternate);
    field_render_actor_objects((FieldRenderContext*)render_half);
    func_80086FB8((u8*)render_half);
    func_800A2E40((u8*)render_half);
    func_800A2E34();
    func_800842E0();
    g_frame_counter += 1;
    func_8008B73C();
    field_update_dialog_runtime(render_half);
    field_update_return_to_title_prompt(render_half);
    func_80096E60();
    func_800A64D0(render_half);
    field_update_modal(render_half);
    func_800AD118(render_half);
    func_800A5794(render_half);
    func_800AF8E8(render_half);
    func_800A3FB0();
    field_update_audio_timer();
    field_update_gover_load();
}
