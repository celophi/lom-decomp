#ifndef FIELD_ACTOR_RUNTIME_H
#define FIELD_ACTOR_RUNTIME_H

#include "common.h"

void field_update_dialog_runtime(s32 update_mode);
void func_80068028(void);
void func_8006809C(void);
void func_800681C0(s32 command_value);
void field_begin_gover_transition(s32 image_resource_index, s32 music_resource_index, s32 audio_clip_index);
void field_update_gover_load(void);
void field_update_return_to_title_prompt(s32 render_context);
void field_open_return_to_title_prompt(void);
void field_begin_return_to_title_prompt_close(void);
void field_update_audio_timer(void);
s32 field_get_track_counter_modulo(s32 animation_data, s32 divisor);
void field_initialize_actor_slots(void);
void field_clear_actor_slots(void);
void field_start_actor_animation(s32 slot_index, int target_count, u8* targets);
s32 field_is_actor_animation_active(s32 slot_index);
s32 field_find_active_special_attack_actor(void);
void field_update_actor_animations(void);
void field_prepare_actor_render_commands(s32 render_context, s32 unused);
void field_reset_global_color_scale(void);
void field_reset_actor_resource_slots(void);
void field_initialize_actor_system(void);
void field_initialize_actor_parts(s32 timer_mode);
void field_release_actor_resource_slot(s32 slot_index_minus_one);
s32 field_activate_actor_resource_slot(s32 source_selector, s32 resource_variant, s32 slot_index_minus_one);
void field_find_or_load_resource_entry(s32 resource_slot_id, s32 resource_base);
void field_set_all_actor_render_state(s32 red, s32 green, s32 blue, s32 color_flag, s32 render_mode);
s32 field_set_actor_render_state(s32 red, s32 green, s32 blue, s32 color_flag, s32 render_mode, s32 actor_selector);
void field_update_actor_objects(void);
s32 field_get_actor_sound_pan(s32 actor_index);
void field_reset_effect_pool(void);

#endif
