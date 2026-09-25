/**
 * @file field_actor_transition_reset.c
 * @brief Battle start and end: party resets, pending animation cleanup and the deferred reset.
 *
 * Resets the party actors when a field group starts or ends, clears pending
 * animation actor bindings and finishes a deferred reset once the field has
 * been idle for fifteen consecutive checks.
 */

#include "common.h"
#include "controller_internal.h"
#include "field_actor_tables.h"
#include "field_calls.h"
#include "field_modal_runtime.h"

/** @brief Idle checks in a row after which the pending reset completes. */
#define FIELD_RESET_IDLE_CHECKS 15

/** @brief Added to the actor index returned by field_find_defeating_actor, so 0 means none. */
#define FIELD_DEFEATING_ACTOR_FOUND 0x100

/**
 * @brief View of the data page at 0x80100000 that holds g_field_object_states.
 * @note field_cancel_animation_bindings reaches the object states through this page once; the
 *       page base in a register plus the member offset is the original codegen
 *       (the plain g_field_object_states symbol schedules its %hi late).
 */
typedef struct
{
    u8 unk0[0x5AE0];
    FieldObjectState object_states[FIELD_ACTOR_COUNT];
} FieldStatePage;

/** @brief Fixed address of the data page that holds g_field_object_states. */
#define FIELD_STATE_PAGE ((FieldStatePage*)0x80100000)

extern s32 g_field_active_group;
extern s32 g_field_text_session_active;
extern s32 g_field_camera_offset_x;
extern s32 g_field_camera_offset_y;
extern s32 g_field_camera_offset_z;
extern s32 g_field_actions_limited;
extern s32 g_field_restore_group;
extern s32 g_field_battle_idle_checks;
extern s32 g_field_duel_mode;
extern s32 D_8011F420;
extern s32 D_8012291C;
extern u32 g_field_experience_snapshot[];
extern u8* g_pad_ctx;

void akao_cmd_f1(void);
void field_clear_actor_slots(void);
void field_clear_actor_effects(FieldActorSlot* slot);
s32 field_find_active_special_attack_actor(void);
void field_initialize_actor_slots(void);
void field_reset_global_color_scale(void);
void field_restart_actor_animation(FieldActor* actor);
void func_8005A0D0(s32 object_index, s32 red, s32 green, s32 blue);
s32 func_8005B218(void);
void func_80067AA4(void);
void func_80068028(void);
void field_update_object_effects(s32 object_index);
void field_route_actor_to_object(FieldActor* actor, s32 arg1, s32 arg2);
void field_camera_select_scroll_limits(void);
void field_command_history_clear(s32 object_index);
void func_800A3938(s32 arg0, s32 arg1);
void func_800A3B78(s32 object_index);
void func_800A6204(void);
void func_800B0234(void);
void func_800B34D0(s32 mode);
s32 field_find_defeating_actor(void);
s32 field_defeat_animation_playing(void);
void field_cancel_animation_bindings(void);

/**
 * @brief Start a battle with a trigger group, or reset the party when a battle ends.
 * @param mode 0 resets the party actors; otherwise the trigger group that starts fighting.
 * @param actor_data Unused.
 */
void field_set_battle_group(s32 mode, void* actor_data)
{
    FieldObjectState* state;
    s32 animation;
    s32 index;
    u32 buttons;
    u8 animation_flags;
    u8* pad_record;
    u8* pad_context;

    func_800A6204();
    if (mode == 0)
    {
        field_cancel_animation_bindings();
        g_field_restore_group = 0;
        func_800B34D0(0);
        g_field_actions_limited = 1;
        for (index = 0; index < FIELD_PARTY_COUNT; index++)
        {
            state = &g_field_object_states[index];
            state->tint_timer = 0;
            state->flags = state->flags & FIELD_OBJECT_FLAG_KNOCKED_OUT;
            state->contact.bits.flag5 = 0;
            state->movement.bits.flag15 = 0;
            state->contact.bits.flag7 = 0;
            g_field_actor_slots[FIELD_OBJECT_EFFECT_SLOT_BASE + index].status.parts.hiding_objects = 0;
            field_stop_actor_slot(&g_field_actors[index], &g_field_actor_slots[FIELD_OBJECT_EFFECT_SLOT_BASE + index], 1);
            state->retry_count = 0;
            g_field_actors[index].variant = 0;
            field_command_history_clear(index);
            field_update_object_effects(index);
        }
        g_field_battle_idle_checks = 0;
        return;
    }

    if (g_field_duel_mode != 0)
    {
        field_begin_duel_intro();
    }
    g_field_active_group = mode;
    field_camera_select_scroll_limits();

    for (index = 0; index < FIELD_PARTY_COUNT; index++)
    {
        pad_record = g_pad_ctx + index * 0x250;
        buttons = *(u32*)(pad_record + 0x610);
        g_field_experience_snapshot[index] = buttons >> 8;
        g_field_player_records[index].unk25D = 0;
        g_field_player_records[index].unk25C = 0;
        g_field_player_records[index].unk25B = 0;
        g_field_player_records[index].unk25A = 0;
    }

    pad_context = g_pad_ctx;
    D_8012291C = 1;
    D_8011F420 = *(s32*)(pad_context + 0x2C);
    func_800B0234();

    g_field_actors[0].animation_active = g_field_actors[0].animation_state = 1;
    g_field_actors[0].animation_frame = 0;
    g_field_actors[0].control.word = g_field_actors[0].control.word & ~FIELD_CONTROL_PLAY_ONCE;
    animation = g_field_actors[0].animation & FIELD_ANIMATION_INDEX_MASK;
    /* The original reads the animation byte twice. */
    animation_flags = *(volatile u8*)&g_field_actors[0].animation & FIELD_ANIMATION_FACING;
    animation %= FIELD_ANIMATION_DIRECTIONS;
    animation_flags += animation;
    g_field_actors[0].animation = animation_flags;
    field_restart_actor_animation(&g_field_actors[0]);

    if (g_field_duel_mode == 0)
    {
        for (index = 1; index < FIELD_PARTY_COUNT; index++)
        {
            if (g_field_player_records[index].flags & FIELD_PLAYER_ACTIVE)
            {
                if (g_field_actors[index].control.half[0] & FIELD_CONTROL_MODE_MASK)
                {
                    g_field_actors[index].command = FIELD_ACTOR_COMMAND_FOLLOW_LEADER;
                    g_field_actors[index].animation_state = 0xFFFF;
                    g_field_object_parts[index].flags = g_field_object_parts[index].flags | FIELD_PART_IGNORE_MAP_COLLISION;
                }
                else
                {
                    field_route_actor_to_object(&g_field_actors[index], 0, 1);
                    g_field_object_parts[index].flags = g_field_object_parts[index].flags | FIELD_PART_IGNORE_MAP_COLLISION;
                    if (g_field_actors[index].command == FIELD_ACTOR_COMMAND_LEAVE_PATH)
                    {
                        g_field_actors[index].command = FIELD_ACTOR_COMMAND_RUN_PATH;
                    }
                }
            }
        }
    }
}

/**
 * @brief Find an actor that is still going through its defeat sequence.
 * @return The actor index plus FIELD_DEFEATING_ACTOR_FOUND, or 0 when none is.
 */
s32 field_find_defeating_actor(void)
{
    s32 i;

    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        if (g_field_actors[i].presence != FIELD_ACTOR_UNUSED)
        {
            if (g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEATED || g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEAT_END ||
                g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEAT_WAIT || g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEAT_DELAY ||
                g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEAT_END || g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEAT_BOUND)
            {
                return i + FIELD_DEFEATING_ACTOR_FOUND;
            }
        }
    }
    return 0;
}

/**
 * @brief In a duel, report whether a player's defeat animation is still playing.
 * @return 1 while a defeated or knocked-down player's reserved slot has tracks, otherwise 0.
 */
s32 field_defeat_animation_playing(void)
{
    s32 i;

    if (g_field_duel_mode == 0)
    {
        return 0;
    }

    for (i = 0; i < FIELD_PLAYER_COUNT; i++)
    {
        if (g_field_actors[i].presence != FIELD_ACTOR_UNUSED)
        {
            if (g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEATED || g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEAT_END ||
                g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEAT_WAIT || g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEAT_DELAY ||
                g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEAT_END || g_field_actors[i].command == FIELD_ACTOR_COMMAND_KNOCKED_DOWN ||
                g_field_actors[i].command == FIELD_ACTOR_COMMAND_DEFEAT_BOUND)
            {
                if (g_field_actor_slots[FIELD_OBJECT_EFFECT_SLOT_BASE + i].track_mask != 0)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/**
 * @brief While the scene fades, cancel every pending animation binding and undo what the animations changed.
 */
void field_cancel_animation_bindings(void)
{
    ControllerState* controller = CONTROLLER_STATE;
    FieldStatePage* page;
    FieldObjectState* states;
    s32 i;
    s32 j;
    s32 k;
    s32 index;
    s32 owner;

    if (func_8005B218() != 0)
    {
        page = FIELD_STATE_PAGE;
        for (i = 0; i < FIELD_ACTOR_BINDING_COUNT; i++)
        {
            if (g_field_actor_bindings[i].state != 0)
            {
                g_field_actor_bindings[i].state = 0;
                g_field_actors[g_field_actor_bindings[i].owner].presence = 0;
                g_field_actors[g_field_actor_bindings[i].owner].command = 0;
                for (j = 0; j < FIELD_ACTOR_SLOT_TOTAL; j++)
                {
                    if (g_field_actor_slots[j].active != 0)
                    {
                        index = g_field_actor_slots[j].owner_object_index;
                        if (index == g_field_actor_bindings[i].owner)
                        {
                            g_field_actor_slots[j].duration = 0;
                            g_field_actors[index].command = 0;
                            func_800A3B78(g_field_actor_slots[j].owner_object_index);
                            field_clear_actor_effects(&g_field_actor_slots[j]);
                            owner = g_field_actor_slots[j].owner_object_index;
                            g_field_actor_slots[j].active = 0;
                            g_field_actor_slots[j].track_mask = 0;
                            index = g_field_actors[owner].command;
                            if ((index != FIELD_ACTOR_COMMAND_DEFEATED && index != FIELD_ACTOR_COMMAND_DEFEAT_END) || (g_field_object_states[owner].flags & FIELD_OBJECT_FLAG_KNOCKED_OUT))
                            {
                                g_field_actors[g_field_actor_slots[j].owner_object_index].presence = 0;
                            }
                            g_field_object_states[g_field_actor_slots[j].owner_object_index].contact.word &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
                            for (k = 0; k < g_field_actor_slots[j].target_count; k++)
                            {
                                if (g_field_actor_slots[j].targets[k] != FIELD_TARGET_NONE)
                                {
                                    g_field_actors[g_field_actor_slots[j].targets[k]].presence = 0;
                                    g_field_object_states[g_field_actor_slots[j].targets[k]].contact.word &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
                                }
                            }
                        }
                    }
                }
                g_field_camera_offset_z = 0;
                g_field_camera_offset_y = 0;
                g_field_camera_offset_x = 0;
                states = page->object_states;
                states[g_field_actor_bindings[i].owner].contact.word &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
                field_set_global_color_scale(FIELD_COLOR_SCALE_NEUTRAL, FIELD_COLOR_SCALE_NEUTRAL, FIELD_COLOR_SCALE_NEUTRAL);
                controller->ports[1].small_motor_command = 0;
                controller->ports[0].small_motor_command = 0;
                controller->ports[1].actuator_control.fields.large_motor_command = 0;
                controller->ports[0].actuator_control.fields.large_motor_command = 0;
            }
        }
    }
}

/**
 * @brief After a battle, reset the field once nothing has been animating for FIELD_RESET_IDLE_CHECKS checks.
 */
void field_update_battle_end(void)
{
    s32 i;

    if (g_field_actions_limited != 0)
    {
        if ((field_find_active_special_attack_actor() == 0) && (field_find_defeating_actor() == 0) && (g_field_text_session_active == 0) && (func_8005B218() == 0) &&
            (field_defeat_animation_playing() == 0))
        {
            g_field_battle_idle_checks += 1;
        }
        else
        {
            g_field_battle_idle_checks = 0;
        }
        if (g_field_battle_idle_checks == FIELD_RESET_IDLE_CHECKS)
        {
            field_initialize_actor_slots();
            field_clear_actor_slots();
            func_80067AA4();
            field_reset_actor_resources();
            g_field_camera_offset_z = 0;
            g_field_camera_offset_y = 0;
            g_field_camera_offset_x = 0;
            g_field_active_group = g_field_restore_group;
            func_80068028();
            akao_cmd_f1();
            field_reset_global_color_scale();
            func_8005A0D0(-1, FIELD_COLOR_SCALE_NEUTRAL, FIELD_COLOR_SCALE_NEUTRAL, FIELD_COLOR_SCALE_NEUTRAL);
            func_800A6204();
            func_800A3938(0x24, 0x80);
            for (i = 0; i < FIELD_PARTY_COUNT; i++)
            {
                if (g_field_player_records[i].flags & FIELD_PLAYER_ACTIVE)
                {
                    g_field_object_states[i].flags = 0;
                    g_field_object_states[i].contact.word &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
                    g_field_object_states[i].contact.word &= ~FIELD_CONTACT_LINKED;
                    g_field_object_states[i].contact.word &= ~FIELD_CONTACT_NO_HIT_TEST;
                    g_field_object_states[i].contact.bytes.target_count = 0;
                    g_field_object_states[i].action_parameter = 0xFFFF;
                    g_field_actors[i].y = 0;
                    if ((g_field_duel_mode != 0) && (g_field_actors[i].command == FIELD_ACTOR_COMMAND_KNOCKED_DOWN))
                    {
                        g_field_actors[i].animation = (g_field_actors[i].animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_DEFENSELESS;
                    }
                    else
                    {
                        g_field_actors[i].animation = (g_field_actors[i].animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_13;
                    }
                    g_field_actors[i].animation_state = 1;
                    g_field_actors[i].animation_active = 1;
                    g_field_actors[i].command = 0;
                    g_field_actors[i].presence = 0;
                    g_field_actors[i].animation_frame = 0;
                    g_field_actors[i].control.word = g_field_actors[i].control.word & ~FIELD_CONTROL_PLAY_ONCE;
                    g_field_object_states[i].movement.word = g_field_object_states[i].movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK;
                    field_restart_actor_animation(&g_field_actors[i]);
                }
            }
            g_field_actions_limited = 0;
        }
    }
}
