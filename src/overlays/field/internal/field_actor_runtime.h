#ifndef FIELD_ACTOR_RUNTIME_H
#define FIELD_ACTOR_RUNTIME_H

#include "common.h"

struct FieldActor;
struct FieldActorSlot;
struct FieldRenderHalf;
struct FieldRenderContext;

/** @brief Frame data address bit that selects the 8-bit frame renderer (field_render_effect_frame8). */
#define FIELD_FRAME_DATA_8BIT 0x80000000u

/** @brief Events an animation can attach a sound to (FieldAnimationDef::sound_events). */
#define FIELD_SOUND_EVENT_START 1
#define FIELD_SOUND_EVENT_PART_SPAWN 2
#define FIELD_SOUND_EVENT_EFFECT_RETIRED 3
#define FIELD_SOUND_EVENT_EFFECT_BOUNCE 4
#define FIELD_SOUND_EVENT_PART_SPAWN_ONCE 5

void field_update_dialog_runtime(struct FieldRenderHalf* render_half);
void field_show_battle_results(void);
void field_close_dialog_screen(void);
void field_request_return_to_title(s32 command_value);
void field_begin_gover_transition(s32 image_resource_index, s32 music_resource_index, s32 audio_clip_index);
void field_update_gover_load(void);
void field_update_return_to_title_prompt(struct FieldRenderHalf* render_half);
void field_open_return_to_title_prompt(void);
void field_begin_return_to_title_prompt_close(void);
void field_update_audio_timer(void);
s32 field_get_track_counter_modulo(struct FieldActorSlot* slot, s32 divisor);
void field_initialize_actor_slots(void);
void field_clear_actor_slots(void);
void field_start_actor_animation(s32 slot_index, int target_count, u8* targets);
s32 field_is_actor_animation_active(s32 slot_index);
s32 field_find_active_special_attack_actor(void);
void field_update_actor_animations(void);
void field_prepare_actor_render_commands(struct FieldRenderContext* render_context, s32 unused);
void field_reset_global_color_scale(void);
void field_reset_actor_resource_slots(void);
void field_initialize_actor_system(void);
void field_initialize_actor_parts(s32 timer_mode);
void field_release_actor_resource_slot(s32 slot_index_minus_one);
s32 field_activate_actor_resource_slot(s32 source_selector, s32 resource_variant, s32 slot_index_minus_one);
void field_find_or_load_resource_entry(s32 resource_slot_id, s32 package);
void field_set_all_actor_render_state(s32 red, s32 green, s32 blue, s32 color_flag, s32 render_mode);
s32 field_set_actor_render_state(s32 red, s32 green, s32 blue, s32 color_flag, s32 render_mode, s32 actor_selector);
void field_update_actor_objects(void);
s32 field_get_actor_sound_pan(s32 actor_index);
void field_restart_actor_animation_reverse(struct FieldActor* actor);
void field_reset_effect_pool(void);
s32 field_evaluate_parameter_track(struct FieldActorSlot* slot, s32 curve_index);

/** @brief (dx, dz) step of one facing in g_field_direction_offsets. */
typedef struct FieldDirectionOffset
{
    s16 dx;
    s16 dz;
} FieldDirectionOffset;

/** @brief Frames left until the game-over overlay loads; 0 when no transition runs. */
extern s32 g_field_gover_load_countdown;

/** @brief Non-zero while the return-to-title prompt is open. */
extern s32 g_field_return_to_title_prompt_state;

/** @brief Frames before the open return-to-title prompt starts taking input. */
extern s32 g_field_return_to_title_prompt_delay;

/** @brief Track (target) of the animation actor slot being evaluated. */
extern s32 g_field_track_index;

/** @brief Next free byte of the actor resource arena. */
extern void* g_field_resource_cursor;

/** @brief (dx, dz) step of each facing. */
extern FieldDirectionOffset g_field_direction_offsets[];

#endif
