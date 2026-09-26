#ifndef FIELD_CALLS_H
#define FIELD_CALLS_H

/**
 * @file field_calls.h
 * @brief Prototypes of FIELD functions that other FIELD translation units call.
 *
 * Each prototype is the signature of the definition, grouped by the file
 * that defines it. Functions with a public header of their own
 * (include/field_*.h) are declared there instead. Pointer parameters of
 * tagged record types use the struct tag, so callers need not include the
 * header that defines the record.
 *
 * A caller whose original code treats a call differently (an extra argument
 * left in a register, or a narrow result used unmasked) calls through a cast
 * to the function type it used, with a comment at the call. GCC 2.7.2
 * rejects a cast that changes the argument count, so a function that such a
 * file calls with extra or missing arguments stays out of this header and
 * keeps a commented local declaration in its callers.
 */

#include "common.h"
#include "field_effect_dispatch.h"
#include "vector.h"

struct FieldActionDescriptor;
struct FieldActor;
struct FieldActorSlot;
struct FieldPlayerRecord;
struct FieldActorState;
struct FieldCardClock;
struct FieldCdBuffer;
struct FieldCharacterRecord;
struct FieldCollisionMover;
struct FieldItemKey;
struct FieldItemRecord;
struct FieldMotionRecord;
struct FieldNode;
struct FieldRenderHalf;
struct FieldStatusRecord;
struct FieldStatusState;

/**
 * @brief Collision probe: world position (24.8 fixed point), footprint and
 *        vertical tolerance, passed to field_collision_hit_markers and the
 *        path functions.
 * @note width and depth are read signed by the collision code.
 */
typedef struct FieldCollisionQuery
{
    s32 x;
    s32 y;
    s32 z;
    /** Footprint width along x, in cells. */
    u16 width;
    /** How far below y the probe still reaches a marker band. */
    s16 height_tolerance;
    /** Footprint depth along z, in cells. */
    u16 depth;
} FieldCollisionQuery;

/** @brief Position of a scene object or part in whole pixels (field_get_object_position). */
typedef struct FieldPos
{
    s16 x;
    s16 y;
    s16 z;
} FieldPos;

/* field_action_modifiers.c */
void field_battle_set_watched_record(s32 record_id);
s32 field_battle_handle_defeat(struct FieldStatusRecord *record);
void field_battle_finish(s32 result);
void field_battle_defeat_record(s32 record_id);
s32 field_run_action_handler(void);
void field_select_coordinate_labels(void);

/* field_active_record_ops.c */
s32 field_join_companion(void);
s32 field_join_golem(void);
s32 field_join_guest(s32 guest_id);
void field_leave_party(s32 companion);
s32 field_rejoin_companion(void);

/* field_actor_action_defaults.c */
void field_reset_action_command_map(s32 map_index);

/* field_actor_camera.c */
void field_camera_track_party(void);
void field_camera_update(void);
void field_camera_select_scroll_limits(void);
void field_camera_reset(void);

/* field_actor_effects.c */
void field_start_object_ground_effect(struct FieldMotionRecord *actor, u32 kind);

/* field_actor_hud_effects.c */
void field_draw_actor_hud(struct FieldRenderHalf *render_half);
void field_update_object_effects(s32 index);
void field_clear_fade_prims(void);
void field_add_fade_prim(const void *prim, s16 depth);
s32 field_load_vram_resource(s32 id, RECT *rect, s32 mode);
void field_draw_fade_prims(struct FieldRenderHalf *render_half);

/* field_actor_input_actions.c */
s32 field_get_held_action_buttons(s32 player, s32 action, struct FieldActor *actor);
void field_poll_leader_interaction(void);
u16 field_resolve_action_command(struct FieldActor *actor, s32 player);
void field_update_actor_run_button(struct FieldActor *actor, s32 player);

/* field_actor_input_map_init.c */
void field_reset_action_command_maps(void);
void field_apply_weapon_action_params(s32 player_index);

/* field_actor_key_ops.c */
void field_set_actor_group(s32 key, s32 group);
s32 field_actor_faces_actor(s32 first_key, s32 second_key);
s32 field_get_actor_binding_state(s32 key);
s32 field_reload_actor(s32 key, s32 resource_entry_index, s32 resource_slot_id, u8 *resource_base, s32 group, s32 x, s32 y, s32 z, s32 animation, s32 resource_flag);

/* field_actor_lifecycle.c */
struct FieldActorTemplate;
struct FieldActorTemplateTable;
void field_battle_scan_actor_objects(void);
void field_battle_start(s32 group);
void field_battle_suspend(void);
void field_battle_end(void);
struct FieldActorTemplate* field_find_actor_template(struct FieldActorTemplateTable* table, s32 id);

/* field_actor_motion.c */
s32 field_update_actor_action(struct FieldActor *actor, s32 update_action);
s32 field_move_leaves_screen(struct FieldActor *actor, Vec3i *delta);

/* field_actor_reactions.c */
s32 field_stop_actor(s32 key);
s32 field_test_actor_depth_overlap(s32 first_key, s32 second_key);

/* field_actor_record_ops.c */
void field_spawn_item_record(s32 key, s32 item);
s32 field_find_nearest_faced_item(s32 unused);
void field_set_actor_record_script_only(s32 key, s32 flags);
void field_clear_actor_record_script_only(s32 key);

/* field_actor_resource_unpack.c */
s32 field_request_resource_read(s32 resource_id);
void field_clear_resource_queue(void);
s32 field_get_loading_resource(void);
void field_issue_next_resource_read(void);
void field_free_owner_resources(s32 tag);
void field_unpack_actor_resource(s32 owner, struct FieldActorState *actor);

/* field_actor_runtime.c */
s32 field_get_actor_resource_id(s32 unused_slot_index, struct FieldPlayerRecord* player, s32 weapon_set);
void field_initialize_actor_part(s32 part_index, s32 timer_mode);
void field_initialize_actor_record(s32 actor_index, s32 resource_entry_index);
void field_load_resource_entry(s32 resource_slot_id, u8 *resource_base, s32 entry_index);
void field_release_resource_entry(s32 entry_index);
void field_render_actor_objects(FieldRenderContext *render_context);
void field_set_global_color_scale(s16 red, s16 green, s16 blue);
void field_unpack_resource_package(struct FieldCdBuffer *buf, s32 size, s32 slot_index, s32 palette_row);
void field_finish_party_slot_reload(s32 actor_slot);

/* field_actor_slot_resources.c */
s32 field_object_has_active_actor_tracks(s32 object_index);
void field_bind_builtin_animations(void);
void field_upload_common_texture(void);
s32 field_find_free_actor_slot(s32 binding_index, s32 require_idle);
s32 field_start_builtin_animation(s32 object_index, s32 slot_index, s32 animation_id);
void field_stop_actor_slot(struct FieldActor* actor, struct FieldActorSlot* slot, s32 force);
s32 field_start_streamed_animation(s32 owner, s32 resource_id);
void field_reset_actor_resources(void);
void field_poll_streamed_animations(void);
void field_release_actor_binding(s32 owner);
void field_reset_object_states(void);
void field_reset_object_tints(void);

/* field_actor_state_updates.c */
s32 field_update_actor_landing(struct FieldActor* actor);
void field_play_object_animation(struct FieldActor* actor, s32 animation_id);
s32 field_update_actor_action_chain(struct FieldActor* actor);
s32 field_update_pending_action(struct FieldActor* actor);
void field_update_instrument_command(struct FieldActor* actor);
void field_update_technique_command(struct FieldActor* actor, s32 sequence_index);
s32 field_move_actor_step(struct FieldActor* actor, s32 direction_x, s32 vertical_step, s32 direction_z);
void field_update_timed_slide(struct FieldActor* actor, s32 x, s32 z);
void field_follow_leader(struct FieldActor* actor);
void field_update_actor_jump(struct FieldActor* actor, s32 x, s32 y, s32 z);
void field_update_actor_lift(struct FieldActor* actor, s32 rising);
void field_slide_actor(struct FieldActor* actor, s32 dx, s32 dz);
void field_update_timed_walk(struct FieldActor* actor, s32 dx, s32 dz);
s32 field_start_defeat_bound_animation(struct FieldActor* actor);
s32 field_start_defeat_wait_animation(struct FieldActor* actor);
s32 field_update_defeat_end(struct FieldActor* actor);
void field_restart_idle_animation(struct FieldActor* actor);
s32 field_update_defeated(struct FieldActor* actor);

/* field_actor_templates.c */
s32 field_build_group_monster_records(s32 group);
void field_init_monster_record(s32 actor_id, struct FieldStatusRecord *record, struct FieldStatusState *state);

/* field_actor_transition_reset.c */
void field_set_battle_group(s32 mode, void *actor_data);
void field_cancel_animation_bindings(void);
void field_update_battle_end(void);

/* field_audio_runtime.c */
s32 field_load_instrument_bank(s32 bank_index);
void field_upload_resource_22_bank(void);
void field_load_song(s32 music_index, s32 second_song);
void field_load_fixed_song(void);
void field_stop_song(void);
void field_stop_field_song(void);
void field_stop_second_song(void);
void field_play_song(void);
void field_play_second_song(void);
void field_fade_song(s32 song, s32 frames, s32 volume);
void field_play_sound(s32 sound_id, s32 pan);
void field_play_sound_half_pan(s32 sound_id, s32 pan);
void field_play_set_sfx_group0(s32 sfx_index, s32 pan, s32 unused);
void field_play_set_sfx(s32 sfx_index, s32 pan, s32 unused, s32 channel_group);
void field_play_weapon_sfx(s32 sfx_index, s32 pan, s32 table_index);
void field_release_sfx_group(s32 channel_group);
void field_load_sfx_tables(s32 set_id);
void field_load_weapon_sfx_table(s32 slot, s32 weapon_type);
s32 field_play_sfx_buffer(s32 buffer, s32 pan, s32 channel_group);
void field_reset_music_stream(void);
void field_start_music_stream(s32 music_index);
void field_update_music_stream(void);
void field_reset_ring_selections(void);

/* field_block_allocator.c */
void field_block_pool_init(void* pool, u32 size);
void* field_block_alloc(void* pool, s32 size, s32 tag);
void field_block_free_tag(void* pool, s32 tag);

/* field_card_clock.c */
/** @brief Non-zero once the PocketStation clock was read. */
extern s32 g_field_card_clock_valid;
s32 field_get_card_clock(struct FieldCardClock *clock);
void field_capture_card_clock(void);

/* field_character_name_flags.c */
void field_flag_known_save(char *file_name);

/* field_choice_labels.c */
/** @brief Selected return-to-title choice: 0 continues (restores the saved state), 1 returns to the title. */
extern s32 g_field_return_to_title_choice;
void field_draw_return_to_title_choices(s32 *ot, void *prim, s32 scroll_x, s32 scroll_y);

/* field_collision.c */
s16 field_collision_hit_markers(struct FieldCollisionQuery *query);
s32 field_collision_move_mover(struct FieldCollisionMover *mover);
void field_collision_rasterize_groups(s32 unused, struct FieldNode *clip);

/* field_command_history.c */
void field_command_history_reset(void);
void field_command_history_record(s32 player, s32 age_sequence);
s32 field_command_history_match(s32 player, s32 unused, s32 peek);
void field_command_history_clear(s32 player);
void field_pair_indicators_reset(void);
u8 *field_pair_indicators_get_list(void);

/* field_coordinate_icon.c */
s32 field_draw_coordinate_panel(void *ot, s32 prim, s32 x_offset, s32 y_offset);
void field_upload_player_icons(void);

/* field_dialog_screens.c */
void field_start_timed_panel(s32 index);
void field_clear_actor_texts(void);
s32 field_actor_text_pending(void);
void field_save_retry_snapshot(void);
void field_update_battle_results_input(void);
void field_close_battle_results(void);
void field_open_battle_results(void);
void field_open_duel_results(void);
void field_setup_return_to_title_prompt(void);

/* field_draw_state.c */
void field_reset_draw_state(void);

/* field_event_dispatch.c */
s32 field_run_actor_event(s32 owner_id, s32 event_id, s32 mode);

/* field_fade.c */
void field_reset_fade_state(void);
void field_restore_fade_target(void);
void field_restore_fade_target_with_duration(s16 duration);
void field_set_fade_target(s16 red, s16 green, s16 blue, s16 duration);
void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration);
void field_update_and_render_fade(struct FieldRenderHalf *ctx);

/* field_generated_record_ops.c */
void field_write_staged_item(void);
void field_derive_weapon_values(struct FieldItemRecord *record);
void field_derive_armor_values(struct FieldItemRecord *record);

/* field_group_derived_stats.c */
void field_golem_build_group_record(s32 group);

/* field_group_layout_ops.c */
void field_golem_commit_group_edit(s32 command);
void field_golem_select_logic_type(s32 type);
void field_golem_rebuild_current_grid(void);

/* field_group_stat_transfer.c */
void field_golem_build_companion(s32 group, struct FieldCharacterRecord *record);

/* field_interaction_start.c */
void field_end_party_script_control(void);
void field_runtime_update(void);
void field_resolve_talk_window(s32 *actor_id, s32 *plane, s32 *effect, s32 *selector);
void field_set_text_macro(s32 slot, u8 *text, u8 character_limit);

/* field_item_selection.c */
void field_open_item_drop_menu(void);
void field_pick_up_item_actor(s32 actor_index);

/* field_layout_slot_state.c */
void field_place_land(s32 land_index);
void field_reset_lands(void);

/* field_menu_ops.c */
void field_run_menu_op(s32 op);

/* field_modal_runtime.c */
void field_bind_saved_game_context(void);
void field_compact_inventory(void);
void field_copy_inventory_record(u8 *destination, u8 *source);
void field_open_gosub_screen_sequence(void *screen_sequence);
void field_open_shop_mode_0(s32 shop_options);
void field_open_shop_mode_1(s32 entry_count, s32 entries, s32 list_options, s32 shop_options);
void field_run_name_entry(u8 *initial_name, u8 *active_name, s32 source_mode, s32 history_index, s32 custom_name);
void field_run_zukan(s32 context);

/* field_modal_stream_start.c */
void field_open_carda(s32 mode);
void field_run_golem(void);
void field_modal_frame_stub(s32 render_half);
void field_open_niki(s32 mode);
void field_open_addhero(s32 mode);

/* field_pair_indicators.c */
void field_update_pair_indicators(struct FieldRenderHalf *render_half);

/* field_progression_ops.c */
void field_reset_party_to_level(s32 level);

/* field_record_effect_ops.c */
void field_roll_menu_slot_effect(s32 group_index, s32 slot_index);
void field_classify_menu_slots(s32 group_index);
void field_apply_pending_region_effects(void);

/* field_record_growth_ops.c */
void field_add_money(s32 recipient, s32 amount);
void field_award_experience(s32 record_index, s32 amount);
void field_apply_character_level_ups(s32 index, s32 notify);
void field_apply_region_level_ups(s32 slot);
s32 field_try_character_level_up(s32 index, s32 notify);
s32 field_add_stat_increase(s32 value, s32 increase, s32 flags);

/* field_record_lookup_ops.c */
void field_restore_actor_capacity_fraction(s32 record_id, s32 fraction_256);
void field_grant_actor_pickup(void *unused, s32 owner_id);

/* field_record_position_queries.c */
s32 field_is_actor_near_stored_position(s32 actor_id, s32 half_width, s32 half_depth);

/* field_record_setup_ops.c */
void field_create_item_from_gosub(s32 kind);
void field_create_equipment_item(struct FieldItemRecord *record, s32 category, s32 item_type, s32 item_subtype);
void field_temper_item(struct FieldItemRecord *record, s32 command_index);

/* field_record_stat_ops.c */
void field_refresh_party_member(s32 index);
s32 field_get_equipped_stat(struct FieldCharacterRecord *character, u32 stat_index);

/* field_record_table_ops.c */
void field_make_land_available(s32 land_index);
s32 field_get_land_state(s32 land_index);
s32 field_get_land_distance(s32 land_index);
void field_generate_item_key(u32 seed, struct FieldItemKey *out);
s32 field_receive_money(s32 amount);
s32 field_spend_money(u32 amount);
s32 field_get_item_value(struct FieldItemRecord *item);
void field_cache_inventory_values(void);

/* field_resource_load.c */
void field_reset_battle_entry(void);
s32 field_begin_battle_entry(void);
void field_update_battle_entry(void);
s32 field_queue_battle_entry_change(s32 actor_key, s32 animation, s32 builtin_animation, s32 sound);
s32 field_party_reload_pending(void);
void field_install_party_reload(s32 alternate_layout, s32 slot);
void field_request_party_reload(s32 weapon_set);

/* field_resource_table_ops.c */
u8* field_get_party_event_script(s32 page, u16 entry);
u8* field_get_party_private_script(s32 page, u16 entry);
struct FieldActionDescriptor* field_get_party_action(s32 page, u16 index);
void field_unlock_encyclopedia_entry(u32 entry);
s32 field_add_template_item(s32 index);
void field_discard_item(s32 index);

/* field_reward_command_ops.c */
s32 field_roll_defeat_drop(struct FieldStatusRecord* record);
void field_grant_reward(s32 recipient, s32 owner, u32 kind);

/* field_ring_selection.c */
void field_open_ring_menu(s32 position_mode, s32 menu_id, u16 excluded_mask, s32 cancel_index);
s32 field_get_ring_result(void);
u8 field_get_ring_cursor_entry(void);
s32 field_update_ring_menu(struct FieldRenderHalf *render_half);
void field_load_party_script_page(s32 party_slot, s32 resource_id);
void field_upload_golem_palettes(void);
void field_copy_golem_portrait_palette(u8 *destination, s32 palette);

/* field_saved_slot_ops.c */
s32 field_add_stored_companion(s32 template_index);
s32 field_release_stored_companion(void);
s32 field_get_stored_companion_status(s32 index);
void field_rename_stored_companion(s32 index);

/* field_scene_build.c */
void field_draw_scene_objects(u8* *cursor, u_long *ot, s32 update_mode);
void field_size_work_buffer(void);

/* field_scene_control.c */
void field_apply_pixel_lookup(u16 *pixels, s32 pixel_count, s32 table_index, void *unused);
void field_control_animation(s32 list_kind, s32 index, s32 keyframe, s32 op);
void field_update_scene_fade(void);
void field_set_object_visible(s32 obj_index, s32 part_index, s32 visible);
void field_get_object_position(s32 obj_index, s32 part_index, FieldPos *out);
void field_play_effect_animation(s32 index, s32 forward);
void field_control_sequence(s32 index, s32 op);
s32 field_get_animation_state(s32 list_kind, s32 index);
void field_begin_scene_fade_out(void);
s32 field_is_scene_fading(void);
void field_set_node_enabled(s32 index, s32 enabled);
void field_set_pixel_lookup(s32 selector);

/* field_scene_load.c */
void field_load_map(u16 map_id);
void field_init_ctx(struct FieldRenderHalf* buffers, u16 object_index);

/* field_script_arith_ops.c */
s32 field_script_calc(s32 op, s32 left, s32 right);

/* field_script_commands.c */
void field_script_command(s32 command, void *params);

/* field_select_distance_bucket.c */
s32 func_800C9ED4(s32 actor_id);

/* field_slot_pool_ops.c */
void func_800BF2F0(s32 offset);
void func_800BF3D8(void);
s32 func_800BF68C(s32 cost, s32 value, s32 replacement);
void func_800BF700(void);
void func_800BF800(void);
void func_800BF880(s32 index);
void func_800BF9A0(s32 index);
s32 func_800BF9F0(s32 cost);

/* field_stat_counter_ops.c */
s32 func_800C2094(s32 bit_index);
u8 func_800C20D8(s32 index);
void func_800C2138(s32 index);
void func_800C21C0(s32 index);

/* field_status_ticks.c */
void func_800B4934(struct FieldStatusRecord *record);
void func_800B49C0(void);
u32 func_800B4CE4(struct FieldStatusRecord *record, s32 status);

/* field_text_session.c */
void func_800AF8C4(void);
void func_800AF8E8(s32 context);

/* field_text_window_api.c */
void func_8009C620(s32 window_slot, s32 layout_index, s32 unused, s32 portrait_selector);
void func_8009C77C(s32 window_slot, s32 string_index, s32 options);
void func_8009C974(s32 string_index);

#endif
