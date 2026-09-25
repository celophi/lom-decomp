/**
 * @file field_actor_transition_reset.c
 * @brief Field actor transition and reset logic (vram 0x800966F0..0x800970B0).
 *
 * Resets the party actors when a field group starts or ends, clears pending
 * animation actor bindings and finishes a deferred reset once the field has
 * been idle for fifteen consecutive checks.
 */

#include "common.h"
#include "field_actor_tables.h"
#include "field_modal_runtime.h"

/** @brief Number of party members (player records and party actors). */
#define FIELD_PARTY_COUNT 3

/** @brief Number of animation actor slots scanned by the binding cleanup. */
#define FIELD_CLEANUP_SLOT_COUNT 80

/** @brief Idle checks in a row after which the pending reset completes. */
#define FIELD_RESET_IDLE_CHECKS 0xF

/**
 * @brief View of the data page at 0x80100000 that holds g_field_object_states.
 * @note func_80096B54 reaches the object states through this page once; the
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
extern s32 D_8010AE5C;
extern s32 D_8010CFD0;
extern s32 g_field_duel_mode;
extern s32 D_8011F420;
extern s32 D_8012291C;
extern u32 D_801229A0[];
extern u8* g_pad_ctx;

void akao_cmd_f1(void);
void field_clear_actor_slots(void);
void field_clear_actor_effects(FieldActorSlot* slot);
s32 field_find_active_special_attack_actor(void);
void field_initialize_actor_slots(void);
void field_reset_global_color_scale(void);
void field_restart_actor_animation(FieldActor* actor);
void field_set_global_color_scale(s32 red, s32 green, s32 blue);
void func_8005A0D0(s32 object_index, s32 red, s32 green, s32 blue);
s32 func_8005B218(void);
void func_80067AA4(void);
void func_80068028(void);
void func_80083BC0(FieldActor* actor, FieldActorSlot* slot, s32 mode);
void func_80084240(void);
void field_update_object_effects(s32 object_index);
void field_route_actor_to_object(FieldActor* actor, s32 arg1, s32 arg2);
void field_camera_select_scroll_limits(void);
void field_command_history_clear(s32 object_index);
void func_800A3938(s32 arg0, s32 arg1);
void func_800A3B78(s32 object_index);
void func_800A6204(void);
void func_800B0234(void);
void func_800B34D0(s32 mode);
s32 func_80096A00(void);
s32 func_80096A90(void);
void func_80096B54(void);

/**
 * @brief Reset field actors or prepare their state for a field transition.
 * @param mode Zero resets the three party actors; nonzero is the new active group.
 * @param actor_data Actor data supplied by lifecycle callers; unused here.
 */
void func_800966F0(s32 mode, void* actor_data)
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
        func_80096B54();
        D_8010AE5C = 0;
        func_800B34D0(0);
        g_field_actions_limited = 1;
        for (index = 0; index < FIELD_PARTY_COUNT; index++)
        {
            state = &g_field_object_states[index];
            state->tint_timer = 0;
            state->flags = state->flags & 0x200;
            state->contact.bits.flag5 = 0;
            state->movement.bits.flag15 = 0;
            state->contact.bits.flag7 = 0;
            g_field_actor_slots[64 + index].status.bytes[1] = 0;
            func_80083BC0(&g_field_actors[index], &g_field_actor_slots[64 + index], 1);
            state->retry_count = 0;
            g_field_actors[index].variant = 0;
            field_command_history_clear(index);
            field_update_object_effects(index);
        }
        D_8010CFD0 = 0;
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
        D_801229A0[index] = buttons >> 8;
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
    g_field_actors[0].control.word = g_field_actors[0].control.word & ~0x800;
    animation = g_field_actors[0].animation & 0x7F;
    /* The original reads the animation byte twice. */
    animation_flags = *(volatile u8*)&g_field_actors[0].animation & 0x80;
    animation %= 5;
    animation_flags += animation;
    g_field_actors[0].animation = animation_flags;
    field_restart_actor_animation(&g_field_actors[0]);

    if (g_field_duel_mode == 0)
    {
        for (index = 1; index < FIELD_PARTY_COUNT; index++)
        {
            if (g_field_player_records[index].flags & FIELD_PLAYER_ACTIVE)
            {
                if (g_field_actors[index].control.half[0] & 0x1FF)
                {
                    g_field_actors[index].command = 0xAF;
                    g_field_actors[index].animation_state = 0xFFFF;
                    g_field_object_parts[index].flags = g_field_object_parts[index].flags | 0x800000;
                }
                else
                {
                    field_route_actor_to_object(&g_field_actors[index], 0, 1);
                    g_field_object_parts[index].flags = g_field_object_parts[index].flags | 0x800000;
                    if (g_field_actors[index].command == 0xB5)
                    {
                        g_field_actors[index].command = 0xB1;
                    }
                }
            }
        }
    }
}

/**
 * @brief Scan the field actors for one present and running a transition command.
 * @return The actor index plus 0x100 for the first matching actor, else 0.
 */
s32 func_80096A00(void)
{
    s32 i;

    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        if (g_field_actors[i].presence != FIELD_ACTOR_UNUSED)
        {
            if (g_field_actors[i].command == 0x90 || g_field_actors[i].command == 0x94 || g_field_actors[i].command == 0x93 ||
                g_field_actors[i].command == 0xAE || g_field_actors[i].command == 0x94 || g_field_actors[i].command == 0x92)
            {
                return i + 0x100;
            }
        }
    }
    return 0;
}

/**
 * @brief Report whether a transitioning party actor still has animation tracks.
 * @return 1 if a matching actor's reserved slot has a nonzero track mask, else 0.
 */
s32 func_80096A90(void)
{
    s32 i;

    if (g_field_duel_mode == 0)
    {
        return 0;
    }

    for (i = 0; i < 2; i++)
    {
        if (g_field_actors[i].presence != FIELD_ACTOR_UNUSED)
        {
            if (g_field_actors[i].command == 0x90 || g_field_actors[i].command == 0x94 || g_field_actors[i].command == 0x93 ||
                g_field_actors[i].command == 0xAE || g_field_actors[i].command == 0x94 || g_field_actors[i].command == 0x8E ||
                g_field_actors[i].command == 0x92)
            {
                if (g_field_actor_slots[i + 64].track_mask != 0)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/**
 * @brief Process pending actor binding cleanups and reset their rendering state.
 */
void func_80096B54(void)
{
    FieldRenderState* render = FIELD_RENDER_STATE;
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
                for (j = 0; j < FIELD_CLEANUP_SLOT_COUNT; j++)
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
                            if ((index != 0x90 && index != 0x94) || (g_field_object_states[owner].flags & 0x200))
                            {
                                g_field_actors[g_field_actor_slots[j].owner_object_index].presence = 0;
                            }
                            g_field_object_states[g_field_actor_slots[j].owner_object_index].contact.word &= ~1;
                            for (k = 0; k < g_field_actor_slots[j].target_count; k++)
                            {
                                if (g_field_actor_slots[j].targets[k] != 0xFF)
                                {
                                    g_field_actors[g_field_actor_slots[j].targets[k]].presence = 0;
                                    g_field_object_states[g_field_actor_slots[j].targets[k]].contact.word &= ~1;
                                }
                            }
                        }
                    }
                }
                g_field_camera_offset_z = 0;
                g_field_camera_offset_y = 0;
                g_field_camera_offset_x = 0;
                states = page->object_states;
                states[g_field_actor_bindings[i].owner].contact.word &= ~1;
                field_set_global_color_scale(0x100, 0x100, 0x100);
                render->unk13F = 0;
                render->unk91 = 0;
                render->unk140 = 0;
                render->unk92 = 0;
            }
        }
    }
}

/**
 * @brief Complete a pending field reset after fifteen consecutive idle checks.
 */
void func_80096E60(void)
{
    s32 i;

    if (g_field_actions_limited != 0)
    {
        if ((field_find_active_special_attack_actor() == 0) && (func_80096A00() == 0) && (g_field_text_session_active == 0) && (func_8005B218() == 0) &&
            (func_80096A90() == 0))
        {
            D_8010CFD0 += 1;
        }
        else
        {
            D_8010CFD0 = 0;
        }
        if (D_8010CFD0 == FIELD_RESET_IDLE_CHECKS)
        {
            field_initialize_actor_slots();
            field_clear_actor_slots();
            func_80067AA4();
            func_80084240();
            g_field_camera_offset_z = 0;
            g_field_camera_offset_y = 0;
            g_field_camera_offset_x = 0;
            g_field_active_group = D_8010AE5C;
            func_80068028();
            akao_cmd_f1();
            field_reset_global_color_scale();
            func_8005A0D0(-1, 0x100, 0x100, 0x100);
            func_800A6204();
            func_800A3938(0x24, 0x80);
            for (i = 0; i < FIELD_PARTY_COUNT; i++)
            {
                if (g_field_player_records[i].flags & FIELD_PLAYER_ACTIVE)
                {
                    g_field_object_states[i].flags = 0;
                    g_field_object_states[i].contact.word &= ~1;
                    g_field_object_states[i].contact.word &= ~2;
                    g_field_object_states[i].contact.word &= ~0x20;
                    g_field_object_states[i].contact.bytes.target_count = 0;
                    g_field_object_states[i].action_parameter = 0xFFFF;
                    g_field_actors[i].y = 0;
                    if ((g_field_duel_mode != 0) && (g_field_actors[i].command == 0x8E))
                    {
                        g_field_actors[i].animation = (g_field_actors[i].animation & 0x80) + 0x31;
                    }
                    else
                    {
                        g_field_actors[i].animation = (g_field_actors[i].animation & 0x80) + 0x13;
                    }
                    g_field_actors[i].animation_state = 1;
                    g_field_actors[i].animation_active = 1;
                    g_field_actors[i].command = 0;
                    g_field_actors[i].presence = 0;
                    g_field_actors[i].animation_frame = 0;
                    g_field_actors[i].control.word = g_field_actors[i].control.word & ~0x800;
                    g_field_object_states[i].movement.word = g_field_object_states[i].movement.word & ~0x1800;
                    field_restart_actor_animation(&g_field_actors[i]);
                }
            }
            g_field_actions_limited = 0;
        }
    }
}
