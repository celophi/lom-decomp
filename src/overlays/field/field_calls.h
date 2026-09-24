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

struct FieldActorResourceSlot;
struct FieldActorState;
struct FieldCdBuffer;
struct FieldCharacterRecord;
struct FieldCollisionMover;
struct FieldCollisionQuery;
struct FieldItemKey;
struct FieldItemRecord;
struct FieldMotionRecord;
struct FieldNode;
struct FieldRenderHalf;
struct FieldStatusRecord;
struct FieldStatusState;

/* field_action_modifiers.c */
void func_800B61C4(s32 record_id);
s32 func_800B6334(struct FieldStatusRecord *record);
void func_800B65CC(s32 result);
void func_800B66F0(s32 record_id);
s32 func_800B6808(void);

/* field_active_record_ops.c */
s32 func_800C2D08(void);
s32 func_800C2DC0(void);
s32 func_800C318C(void);
void func_800C31BC(s32 companion);

/* field_actor_action_defaults.c */
void func_80091518(s32 map_index);

/* field_actor_camera.c */
void func_80091BC8(void);
void func_80092124(void);
void func_80092394(void);

/* field_actor_effects.c */
void func_8009D4D8(struct FieldMotionRecord *actor, u32 mode);

/* field_actor_hud_effects.c */
void field_draw_actor_hud(u8 *render_context);
void func_80086494(s32 index);
void func_80086F20(void);
void func_80086F48(const void *src, s16 value);
void func_80086FB8(u8 *buffer);

/* field_actor_input_actions.c */
s32 func_80091728(s32 player, s32 action, struct FieldMotionRecord *actor);
u16 func_80091914(struct FieldMotionRecord *actor, s32 map_index);

/* field_actor_input_map_init.c */
void func_80091410(void);
void func_80091438(s32 player_index);

/* field_actor_key_ops.c */
void func_80087614(s32 key, s32 group);
s32 func_80087770(s32 first_key, s32 second_key);
s32 func_800878B4(s32 key);
s32 func_80087A9C(s32 key, s32 resource_entry_index, s32 resource_slot_id, u8 *resource_base, s32 group, s32 x, s32 y, s32 z, s32 animation, s32 resource_flag);

/* field_actor_lifecycle.c */
void func_800B4390(void);
void func_800B4410(s32 group);
void func_800B4584(void);

/* field_actor_motion.c */
s32 func_80092988(Vec3i *position, Vec3i *delta);

/* field_actor_reactions.c */
s32 func_8008BD88(s32 key);
s32 func_8008C2EC(s32 first_key, s32 second_key);

/* field_actor_record_ops.c */
void func_800C2640(s32 actor_id, s32 argument);
s32 func_800C2724(void);
void func_800C2848(s32 actor_id, s32 flags);
void func_800C28B8(void);

/* field_actor_resource_unpack.c */
void func_8009A384(void);
s32 func_8009A390(void);
void func_8009A3E8(void);
void func_8009A4A0(s32 group);
void func_8009A4CC(s32 group, struct FieldActorState *actor);

/* field_actor_runtime.c */
s32 field_get_actor_resource_id(s32 unused_slot_index, struct FieldActorResourceSlot *entry, s32 alternate_set);
void field_initialize_actor_part(s32 part_index, s32 timer_mode);
void field_initialize_actor_record(s32 actor_index, s32 resource_entry_index);
void field_load_resource_entry(s32 resource_slot_id, u8 *resource_base, s32 entry_index);
void field_release_resource_entry(s32 entry_index);
void field_render_actor_objects(FieldRenderContext *render_context);
void field_set_global_color_scale(s16 red, s16 green, s16 blue);
void field_unpack_resource_package(struct FieldCdBuffer *buf, s32 size, s32 slot_index, s32 palette_row);
void func_8006AA7C(s32 actor_slot);

/* field_actor_slot_resources.c */
s32 field_object_has_active_actor_tracks(s32 object_index);
void func_80083948(void);
void func_8008396C(void);
s32 func_800839F8(s32 binding_index, s32 require_idle);
s32 func_80083EEC(s32 object_index, s32 slot_index, s32 resource_index);
s32 func_8008404C(s32 owner, s32 resource_id);
void func_80084240(void);
void func_800842E0(void);
void func_80084424(s32 owner);
void func_80084524(void);
void func_80084630(void);

/* field_actor_state_updates.c */
s32 func_80092AD8(struct FieldMotionRecord *object);
s32 func_80092C98(struct FieldMotionRecord *object);
s32 func_80093AB8(struct FieldMotionRecord *object);

/* field_actor_templates.c */
s32 func_800B3DF4(s32 group);
void func_800B3F1C(s32 actor_id, struct FieldStatusRecord *record, struct FieldStatusState *state);

/* field_actor_transition_reset.c */
void func_800966F0(s32 mode, void *actor_data);
void func_80096B54(void);
void func_80096E60(void);

/* field_audio_runtime.c */
void field_stop_song(void);
s32 func_800A35F4(s32 resource_base);
void func_800A3654(void);
void func_800A368C(s32 music_index, s32 destination_index);
void func_800A3728(void);
void func_800A37E4(void);
void func_800A380C(void);
void func_800A38D4(void);
void func_800A3904(s32 slot, s32 count, s32 value);
void func_800A3938(s32 sound_id, s32 pan);
void func_800A3988(s32 sfx_index, s32 pan, s32 unused);
void func_800A39A8(s32 sfx_index, s32 pan, s32 arg2, s32 channel_group);
void func_800A3B78(s32 idx);
void func_800A3BE8(s32 bank_id);
void func_800A3D44(s32 slot, s32 bank_id);
void func_800A3EBC(void);
void func_800A3FB0(void);
void func_800A43C0(void);

/* field_block_allocator.c */
void func_8009CA08(u32 *pool, u32 size);

/* field_card_clock.c */
void func_800B0094(void);

/* field_choice_labels.c */
void func_800AED20(void *ot, void *prim, s32 x_offset, s32 y_offset);

/* field_collision.c */
s16 func_8005B368(struct FieldCollisionQuery *query);
s32 func_8005B6AC(struct FieldCollisionMover *mover);
void func_8005F5BC(s32 unused, struct FieldNode *clip);

/* field_command_history.c */
void func_800A255C(void);
void func_800A2594(s32 player, s32 age_sequence);
s32 func_800A29F8(s32 player, s32 unused, s32 peek);
void func_800A2DD8(s32 player);
void func_800A2DFC(void);
u8 *func_800A2E34(void);

/* field_coordinate_icon.c */
s32 func_800AE8A8(void *ot, s32 prim, s32 x_offset, s32 y_offset);
void func_800AE9E0(void);

/* field_dialog_screens.c */
void func_800A5670(s32 index);
void func_800A6204(void);
s32 func_800A6490(void);
void func_800A6EEC(void);
void func_800A710C(void);
void func_800A7384(void);
void func_800A7434(void);
void func_800A74B8(void);
void func_800A74E8(void);

/* field_draw_state.c */
void func_80067AA4(void);

/* field_event_dispatch.c */
s32 func_800B28E0(s32 owner_id, s32 event_id, s32 mode);

/* field_fade.c */
void field_reset_fade_state(void);
void field_restore_fade_target(void);
void field_restore_fade_target_with_duration(s16 duration);
void field_set_fade_target(s16 red, s16 green, s16 blue, s16 duration);
void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration);
void field_update_and_render_fade(struct FieldRenderHalf *ctx);

/* field_generated_record_ops.c */
void func_800BFA34(void);
void func_800BFF90(struct FieldItemRecord *record);
void func_800C015C(struct FieldItemRecord *record);

/* field_group_derived_stats.c */
void func_800C4364(s32 arg0);

/* field_group_layout_ops.c */
void func_800C3A00(s32 command);
void func_800C3B50(s32 type);
void func_800C3BB0(void);

/* field_group_stat_transfer.c */
void func_800C3F18(s32 group_index, void *destination);

/* field_interaction_start.c */
void func_800B177C(void);
void func_800B19FC(void);
void func_800B2654(s32 *actor_id, s32 *plane, s32 *effect, s32 *selector);
void func_800B2844(s32 slot, u8 *text, u8 character_limit);

/* field_item_selection.c */
void func_800AEE28(void);
void func_800AF824(s32 actor_index);

/* field_layout_slot_state.c */
void func_800CA1A0(s32 arg0);
void func_800CA1E0(void);

/* field_menu_ops.c */
void func_800C5704(s32 op);

/* field_menu_windows.c */
void func_800ADE2C(void);
void func_800ADEB0(void);
s32 func_800ADEEC(void);
void func_800ADF34(void);
s32 func_800AE864(u8 *str);

/* field_modal_runtime.c */
void field_bind_saved_game_context(void);
void field_compact_inventory(void);
void field_copy_inventory_record(u8 *destination, u8 *source);
void field_open_gosub_screen_sequence(void *screen_sequence);
void field_open_shop_mode_0(s32 shop_options);
void field_open_shop_mode_1(s32 entry_count, s32 entries, s32 list_options, s32 shop_options);
void field_run_name_entry(s32 initial_name, s32 active_name, s32 source_mode, s32 history_index, s32 custom_name);
void field_run_zukan(s32 context);

/* field_modal_stream_start.c */
void func_800AD030(s32 mode);
void func_800AD194(s32 mode);

/* field_pair_indicators.c */
void func_800A2E40(u8 *buffer);

/* field_progression_ops.c */
void func_800B60DC(s32 level);

/* field_record_effect_ops.c */
void func_800C0260(s32 group_index, s32 slot_index);
void func_800C0490(s32 group_index);
void func_800C06E8(void);

/* field_record_growth_ops.c */
void func_800C0E18(s32 recipient, s32 amount);
void func_800C0E54(s32 record_index, s32 amount);
void func_800C11F0(s32 index, s32 notify);
void func_800C1230(s32 slot);
s32 func_800C14A4(s32 index, s32 notify);
s32 func_800C19D0(s32 value, s32 increase, s32 flags);

/* field_record_lookup_ops.c */
void field_restore_actor_capacity_fraction(s32 record_id, s32 fraction_256);
void func_800C1A18(void *unused, s32 owner_id);

/* field_record_position_queries.c */
s32 func_800C1FFC(s32 actor_id, s32 half_width, s32 half_depth);

/* field_record_setup_ops.c */
void func_800BE710(s32 kind);
void func_800BE888(struct FieldItemRecord *record, s32 category, s32 item_type, s32 item_subtype);
void func_800BEC44(struct FieldItemRecord *record, s32 command_index);

/* field_record_stat_ops.c */
void func_800B7C58(s32 index);
s32 func_800B7EE8(struct FieldCharacterRecord *character, u32 stat_index);

/* field_record_table_ops.c */
void func_800C35AC(s32 land_index);
s32 func_800C35E4(s32 land_index);
s32 func_800C3688(s32 land_index);
void func_800C37A8(u32 seed, struct FieldItemKey *out);
s32 func_800C3860(s32 amount);
s32 func_800C3894(u32 amount);
s32 func_800C38C8(struct FieldItemRecord *item);
void func_800C396C(void);

/* field_resource_load.c */
void func_800B0244(void);
s32 func_800B0710(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 func_800B0850(void);
void func_800B0A08(s32 arg0);

/* field_ring_selection.c */
void func_800A43E8(s32 position_mode, s32 resource_index, u16 excluded_mask, s32 cancel_index);
s32 func_800A4744(void);
u8 func_800A4778(void);
s32 func_800A4798(u8 *render_context);
void func_800A5174(s32 bank, s32 queue_id);
void func_800A54D0(void);

/* field_saved_slot_ops.c */
s32 func_800C2264(s32 template_index);
s32 func_800C23F4(void);
s32 func_800C24BC(s32 index);
void func_800C25A0(s32 index);

/* field_scene_build.c */
void field_draw_scene_objects(u8* *cursor, u_long *ot, s32 update_mode);
void field_size_work_buffer(void);

/* field_scene_control.c */
void field_apply_pixel_lookup(u16 *pixels, s32 pixel_count, s32 table_index, void *unused);
void field_control_animation(s32 list_kind, s32 index, s32 keyframe, s32 op);
void field_update_scene_fade(void);
void func_8005A67C(s32 index, s32 op);
s32 func_8005A84C(s32 list_kind, s32 index);
void func_8005B1EC(void);
s32 func_8005B218(void);
void func_8005B228(s32 index, s32 enabled);
void func_8005B288(s32 selector);

/* field_scene_load.c */
void field_load_map(s32 map_id);

/* field_script_arith_ops.c */
s32 func_800BE5C8(s32 idx, s32 arg1, s32 arg2);

/* field_script_commands.c */
void func_800BD6F4(s32 value, u8 *params);

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
