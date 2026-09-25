/**
 * @file field_actor_state_updates.c
 * @brief Actor command handlers (action chains, movement, landing, defeat,
 *        idle) and the object sequence interpreter with its animation actors
 *        and tint flashing.
 */
#include "common.h"
#include "field_calls.h"
#include "vector.h"
#include "field_types.h"
#include "field_actor.h"
#include "sdk/inline_c.h"
/* Apply the matching GTE instruction encodings after the SDK macros. */
#include "sdk/gte_dmpsx_compat.h"
#include "sdk/memory.h"
#include "field_actor_runtime.h"
#include "field_actor_sequence_runtime.h"
#include "field_contact_geometry.h"

/** @brief Scratchpad vector that receives an actor displacement. */
#define FIELD_SCRATCH_DISPLACEMENT ((Vec3i*)0x1F800000)

/**
 * @brief Object-runtime bytes that FieldObjectRuntime does not name yet.
 * @note Each reads the matching padding byte of the shared record.
 */
#define FIELD_OBJECT_IDLE_FLAGS(state) ((state)->pad_0x60)
#define FIELD_OBJECT_IDLE_ANIMATION(state) ((state)->pad_0x168[4])
#define FIELD_OBJECT_ACTION_MODE(state) ((state)->pad_0x16f[0])
#define FIELD_OBJECT_LINKED_OBJECT(state) ((state)->pad_0x16f[1])

/** @brief Per-player metadata; the kind byte also selects the bank of sequence rows. */
typedef struct
{
    u8 flags;
    u8 kind;
    u8 pad2[0x266];
} FieldPlayerRecord;

/** @brief Actor template followed by the remaining per-player record data. */
typedef struct
{
    FieldActorState actor;
    u8 tail[0x24];
} FieldSequenceTemplate;

/** @brief Action slot command as this file reads it (FieldActionSlot::command). */
typedef struct
{
    u16 command;
    u8 pad2[6];
} FieldSequenceActionCommand;

/** @brief Action slot whose flag halfword is also written as a whole (FieldActionSlot). */
typedef struct
{
    u16 command;
    /** @brief FieldActionFlags; the low byte is the target filter. */
    union
    {
        u16 word;
        struct
        {
            u8 target_filter;
            u8 high;
        } bytes;
    } flags;
    u16 animation;
    u16 parameter;
} FieldSequenceAction;

/** @brief Resource action row (FieldActionRow) as the sequence code uses it. */
typedef struct
{
    FieldSequenceActionCommand slots[2];
    u8 pad10[0x58 - 0x10];
    /** @brief Slot that action chains program before performing it. */
    FieldSequenceAction chain_slot;
    u8 pad60[0x190 - 0x60];
} FieldResourceAction;

/** @brief Resource entry mode, bound-animation flags, and behavior flags. */
typedef struct
{
    u8 pad0[8];
    u8 mode;
    u8 pad9[0xE - 9];
    u16 bound_animation_flags;
    u32 flags;
} FieldResourceEntry;

extern FieldActorPartDef g_field_object_parts[];
extern FieldPlayerRecord g_field_player_records[];
extern FieldSequenceTemplate g_field_actor_templates[];
extern FieldActorState g_field_shared_actor_template;
extern u8 g_field_actor_sequence_data[];
extern FieldActorState g_field_actor_slots[];
extern FieldResourceAction g_field_resource_actions[];
extern FieldResourceEntry g_field_resource_entries[];
extern s32 g_field_active_group;
extern s32 g_frame_counter;

void field_restart_actor_animation(FieldActor* actor);
s32 field_resolve_object_hit(s32 source_index, s32 target_index, s32 action);
s32 field_roll_object_evasion(s32 source_index, s32 target_index);
void field_release_object_link(FieldActor* actor);
void field_prepare_actor_action(FieldActor* actor);
/* Called with one or two arguments (the second is ignored), so it stays unprototyped. */
void field_resolve_collected_hits();
void field_stop_actor_animations_for_object(FieldActor* actor, s32 force);
void field_restart_actor_animation_reverse(FieldActor* actor);
/* Defined as returning u8; the original caller tests the unmasked int result. */
s32 field_get_next_animation_frame_count(FieldActor* actor);
void field_update_actor_movement_animation(FieldActor* actor, s32 delta_x, s32 delta_z);

/** @brief Action slot that action chains program (FIELD_ACTION_COMMAND(FIELD_CHAIN_ACTION_SLOT) performs it). */
#define FIELD_CHAIN_ACTION_SLOT 11
/** @brief FieldActionFlags bits cleared when a chain action is programmed: instrument, then target group. */
#define FIELD_ACTION_FLAG_INSTRUMENT 0x400
#define FIELD_ACTION_FLAG_TARGET_GROUP 0x300
/** @brief Target filter of an action without targets. */
#define FIELD_ACTION_TARGET_NONE 0xFF

/**
 * @brief Program the chain action slot of @p actor's resource action row.
 * @note A macro: as an inline function the constants are loaded early and the code changes.
 */
#define FIELD_SET_CHAIN_ACTION(actor, action_command, action_animation, action_parameter)                                                                      \
    g_field_resource_actions[(actor)->object_index].chain_slot.flags.word &= ~FIELD_ACTION_FLAG_INSTRUMENT;                                                    \
    g_field_resource_actions[(actor)->object_index].chain_slot.command = action_command;                                                                        \
    g_field_resource_actions[(actor)->object_index].chain_slot.flags.bytes.target_filter = FIELD_ACTION_TARGET_NONE;                                           \
    g_field_resource_actions[(actor)->object_index].chain_slot.animation = action_animation;                                                                    \
    g_field_resource_actions[(actor)->object_index].chain_slot.parameter = action_parameter;                                                                    \
    g_field_resource_actions[(actor)->object_index].chain_slot.flags.word &= ~FIELD_ACTION_FLAG_TARGET_GROUP;

/** @brief Height interpolated from height to next_height across the current animation frame, in 1/256 units. */
#define FIELD_STEP_OFFSET(actor)                                                                                                                              \
    (((actor)->height + ((s8)(actor)->next_height - (actor)->height) * (actor)->frame_ticks / (actor)->frame_length) << 8)

#define FIELD_SEQUENCE_DISPLACEMENT_SCRATCH 0x1F800000
#define FIELD_SEQUENCE_BINDING_COUNT 3
#define FIELD_SEQUENCE_SHARED_BINDING (FIELD_SEQUENCE_BINDING_COUNT - 1)
#define FIELD_SEQUENCE_ACTOR_LIMIT 80
#define FIELD_SEQUENCE_TARGET_LIMIT 14
#define FIELD_SEQUENCE_FRAME_WAIT 1
#define FIELD_SEQUENCE_NO_ACTOR 0xFF
#define FIELD_SEQUENCE_RESTORE_TEMPLATE 2
#define FIELD_SEQUENCE_MOTION_SCALE_SHIFT 6
#define FIELD_SEQUENCE_ROW_SIZE 32
#define FIELD_SEQUENCE_BANK_SIZE (24 * FIELD_SEQUENCE_ROW_SIZE)
#define FIELD_SEQUENCE_COMMAND_NONE 0xFFFF
#define FIELD_SEQUENCE_ANIMATION_OVERRIDE 0x4000
#define FIELD_SEQUENCE_TRANSIENT_ACTOR 0x8000
#define FIELD_SEQUENCE_ANIMATION_MASK 0x3FF
#define FIELD_SEQUENCE_FACING 0x80
#define FIELD_TINT_BLINK_BIT 4
#define FIELD_TINT_DIM_NUMERATOR 100
#define FIELD_TINT_DIM_DENOMINATOR 128

/**
 * @brief Runtime state of object @p index in @p states, addressed as index-first byte arithmetic.
 * @note The original adds the scaled index before the table base.
 */
#define FIELD_OBJECT_STATE_AT(states, index) ((FieldObjectRuntime*)((index) * sizeof(FieldObjectRuntime) + (u32)(states)))

/** @brief Control bytes following the frame values in a 32-byte sequence row. */
typedef enum
{
    FIELD_SEQUENCE_START_TARGETS_0 = 0xEB,
    FIELD_SEQUENCE_START_TARGETS_1 = 0xEC,
    FIELD_SEQUENCE_START_TARGETS_2 = 0xED,
    FIELD_SEQUENCE_START_CURRENT_TARGETS = 0xEE,
    FIELD_SEQUENCE_WAIT_REPEAT = 0xEF,
    FIELD_SEQUENCE_DELAY = 0xF0,
    FIELD_SEQUENCE_WAIT_ANIMATION = 0xF1,
    FIELD_SEQUENCE_TOGGLE_CONTROL_14 = 0xF2,
    FIELD_SEQUENCE_TOGGLE_CONTROL_15 = 0xF3,
    FIELD_SEQUENCE_TOGGLE_FACING = 0xF4,
    FIELD_SEQUENCE_START_RESOURCE = 0xF5,
    FIELD_SEQUENCE_START_0 = 0xF6,
    FIELD_SEQUENCE_START_1 = 0xF7,
    FIELD_SEQUENCE_START_2 = 0xF8,
    FIELD_SEQUENCE_START_CURRENT = 0xF9,
    FIELD_SEQUENCE_SET_ANIMATION = 0xFA,
    FIELD_SEQUENCE_ALLOCATE_0 = 0xFB,
    FIELD_SEQUENCE_ALLOCATE_1 = 0xFC,
    FIELD_SEQUENCE_ALLOCATE_2 = 0xFD,
    FIELD_SEQUENCE_ALLOCATE_CURRENT = 0xFE,
    FIELD_SEQUENCE_END = 0xFF
} FieldSequenceOpcode;


/**
 * @brief Settle a landing actor to the ground and play the landing effect (animation 0x1A).
 * @param actor Landing actor.
 * @return Zero while the state is being advanced, or one when it is complete.
 * @note The high state bit selects horizontal displacement direction.
 */
s32 field_update_actor_landing(FieldActor* actor)
{
    s32 height;
    s32 rising_height;
    Vec3i* displacement;

    displacement = FIELD_SCRATCH_DISPLACEMENT;
    switch (actor->animation & 0x7F)
    {
    case 8:
        actor->animation = (actor->animation & 0x80) | 9;
        field_restart_actor_animation(actor);
        return 0;
    case 61:
        height = actor->y;
        if (height < -0xC00)
        {
            actor->y = height + 0xC00;
            return 0;
        }
        else
        {
            actor->y = 0;
            actor->animation = (actor->animation & 0x80) | 9;
            field_restart_actor_animation(actor);
            return 0;
        }
    case 72:
    case 73:
        height = actor->y;
        if (height < -0xC00)
        {
            if (actor->animation & 0x80)
            {
                displacement->x = 0x200;
            }
            else
            {
                displacement->x = -0x200;
            }
            displacement->z = 0;
            displacement->y = 0;
            rising_height = actor->y;
            field_resolve_actor_movement(actor, &displacement->x, 1);
            rising_height += 0xC00;
            actor->y = rising_height;
            return 0;
        }
        else if (height == 0)
        {
            break;
        }
        else if (height < -0xA)
        {
            actor->y = -0xA;
            field_play_object_animation(actor, 0x1A);
        }
        else
        {
            actor->y = height + 1;
        }
        return 0;
    case 9:
    case 58:
    case 59:
    case 60:
    case 70:
    case 71:
    case 78:
    case 79:
        field_play_object_animation(actor, 0x1A);
        return 1;
    default:
        break;
    }
    return 1;
}

/**
 * @brief Play a built-in animation on a free actor slot owned by the actor.
 * @param actor Owner of the animation.
 * @param animation_id Animation resource passed to field_start_builtin_animation.
 */
void field_play_object_animation(FieldActor* actor, s32 animation_id)
{
    s32 actor_index = field_find_free_actor_slot(actor->object_index, 0);

    if (actor_index != -1)
    {
        if (field_start_builtin_animation(actor->object_index, actor_index, animation_id))
        {
            field_start_actor_animation(actor_index, 0, 0);
        }
    }
}

/**
 * @brief Chain the actor's held action buttons into a follow-up action.
 * @param actor Player actor; its animation selects which actions can follow.
 * @return Unspecified; callers ignore it.
 */
s32 field_update_actor_action_chain(FieldActor* actor)
{
    s32 targets;
    s32 tmp;
    s32 anim;
    s32 anim_id;
    s32 index;

    if (actor->control.word & 0x1FF)
    {
        return;
    }
    if (actor->command == FIELD_ACTOR_COMMAND_RECOVER)
    {
        tmp = actor->animation & 0x7F;
        if (tmp == 0x3D)
        {
            /* Called as returning int: the original uses the u16 result unmasked. */
            anim = ((s32 (*)(FieldActor*, s32))field_resolve_action_command)(actor, actor->object_index);
            if (g_field_resource_actions[actor->object_index].slots[1].command == tmp && anim == FIELD_ACTION_COMMAND(1))
            {
                actor->command = anim;
                actor->y -= FIELD_STEP_OFFSET(actor);
                field_prepare_actor_action(actor);
                field_command_history_clear(actor->object_index);
                actor->command = FIELD_ACTOR_COMMAND_9B;
                return;
            }
            else if (g_field_resource_actions[actor->object_index].slots[0].command == 0x3D && anim == FIELD_ACTION_COMMAND(0))
            {
                actor->command = anim;
                actor->y -= FIELD_STEP_OFFSET(actor);
                field_prepare_actor_action(actor);
                field_command_history_clear(actor->object_index);
                actor->command = FIELD_ACTOR_COMMAND_9B;
                return;
            }
        }
    }
    if (field_get_held_action_buttons(actor->object_index, 3, actor) != 0)
    {
        switch (actor->animation & 0x7F)
        {
        case 0x2F:
        case 0x44:
            actor->command = FIELD_ACTION_COMMAND(8);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case 0x3E:
            actor->command = FIELD_ACTION_COMMAND(10);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case 0x38:
            actor->command = FIELD_ACTION_COMMAND(10);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case 0x3A:
            FIELD_SET_CHAIN_ACTION(actor, 0x4F, 0x25, 0);
            actor->command = FIELD_ACTION_COMMAND(FIELD_CHAIN_ACTION_SLOT);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case 0x39:
            FIELD_SET_CHAIN_ACTION(actor, 0x4F, 0x25, 0);
            actor->command = FIELD_ACTION_COMMAND(FIELD_CHAIN_ACTION_SLOT);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case 0x34:
            FIELD_SET_CHAIN_ACTION(actor, 0x51, 0x27, 0);
            actor->command = FIELD_ACTION_COMMAND(FIELD_CHAIN_ACTION_SLOT);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case 0x8:
        case 0x3B:
        case 0x3C:
        case 0x3D:
            if (actor->animation_frame < 3)
            {
                return;
            }
            actor->animation = (actor->animation & 0x80) | 0x49;
            actor->y -= FIELD_STEP_OFFSET(actor);
            field_restart_actor_animation(actor);
            field_command_history_clear(actor->object_index);
            actor->command = FIELD_ACTOR_COMMAND_96;
            actor->frame_timer = 1;
            actor->frame_ticks = 1;
            actor->frame_length = 1;
            return;
        case 0x35:
            if ((g_field_object_states[actor->object_index].contact.flags >> 1) & 1)
            {
                g_field_object_states[FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index])].object_flags &= ~0x2000;
                actor->command = 0;
                field_restart_sequence_animation(actor);
                tmp = field_find_free_actor_slot(actor->object_index, 0);
                if (tmp != -1)
                {
                    if (field_roll_object_evasion(actor->object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index])) != 0)
                    {
                        if (g_field_player_records[actor->object_index].kind == 8)
                        {
                            field_resolve_object_hit(actor->object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index]), 0xD);
                        }
                        else
                        {
                            field_resolve_object_hit(actor->object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index]), 0xC);
                        }
                        index = actor->object_index;
                        anim_id = 0x64;
                        if (g_field_player_records[index].kind == 8)
                        {
                            anim_id = 0x61;
                        }
                    }
                    else
                    {
                        if (g_field_player_records[actor->object_index].kind == 8)
                        {
                            field_resolve_object_hit(FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index]), actor->object_index, 0x18);
                        }
                        else
                        {
                            field_resolve_object_hit(FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index]), actor->object_index, 0x17);
                        }
                        index = actor->object_index;
                        anim_id = 0x65;
                        if (g_field_player_records[index].kind == 8)
                        {
                            anim_id = 0x63;
                        }
                    }
                    if (field_start_builtin_animation(index, tmp, anim_id) != 0)
                    {
                        targets = FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index]);
                        field_start_actor_animation(tmp, 1, (u8*)&targets);
                    }
                    field_command_history_clear(actor->object_index);
                }
                field_release_object_link(actor);
            }
            return;
        default:
            return;
        }
    }
    else if (field_get_held_action_buttons(actor->object_index, 1, actor) != 0 || field_get_held_action_buttons(actor->object_index, 0, actor) != 0)
    {
        tmp = field_get_held_action_buttons(actor->object_index, 1, actor) != 0;
        switch (actor->animation & 0x7F)
        {
        case 0x25:
            if (g_field_resource_actions[actor->object_index].slots[tmp].command == 8 ||
                g_field_resource_actions[actor->object_index].slots[tmp].command == 0x3C)
            {
                actor->command = FIELD_ACTION_COMMAND(9);
                field_prepare_actor_action(actor);
                field_command_history_clear(actor->object_index);
            }
            break;
        case 0x31:
            if (g_field_resource_actions[actor->object_index].slots[tmp].command == 8)
            {
                FIELD_SET_CHAIN_ACTION(actor, 0x3C, 0, 1);
                actor->command = FIELD_ACTION_COMMAND(FIELD_CHAIN_ACTION_SLOT);
                field_prepare_actor_action(actor);
                field_command_history_clear(actor->object_index);
            }
            break;
        }
    }
    else if (field_get_held_action_buttons(actor->object_index, 2, actor) != 0)
    {
        if ((actor->animation & ~0x80) == 0x34)
        {
            FIELD_SET_CHAIN_ACTION(actor, 0x50, 0x26, 0);
            actor->command = FIELD_ACTION_COMMAND(FIELD_CHAIN_ACTION_SLOT);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
        }
        if ((actor->animation & ~0x80) == 0x35)
        {
            if ((g_field_object_states[actor->object_index].contact.flags >> 1) & 1)
            {
                g_field_object_states[FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index])].object_flags &= ~0x2000;
                actor->command = 0;
                field_restart_sequence_animation(actor);
                tmp = field_find_free_actor_slot(actor->object_index, 0);
                if (tmp != -1)
                {
                    if (field_roll_object_evasion(actor->object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index])) != 0)
                    {
                        if (g_field_player_records[actor->object_index].kind == 8)
                        {
                            field_resolve_object_hit(actor->object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index]), 0xD);
                        }
                        else
                        {
                            field_resolve_object_hit(actor->object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index]), 0xC);
                        }
                        index = actor->object_index;
                        anim_id = 0x64;
                        if (g_field_player_records[index].kind == 8)
                        {
                            anim_id = 0x61;
                        }
                    }
                    else
                    {
                        if (g_field_player_records[actor->object_index].kind == 8)
                        {
                            field_resolve_object_hit(FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index]), actor->object_index, 0x18);
                        }
                        else
                        {
                            field_resolve_object_hit(FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index]), actor->object_index, 0x17);
                        }
                        index = actor->object_index;
                        anim_id = 0x65;
                        if (g_field_player_records[index].kind == 8)
                        {
                            anim_id = 0x63;
                        }
                    }
                    if (field_start_builtin_animation(index, tmp, anim_id) != 0)
                    {
                        targets = FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[actor->object_index]);
                        field_start_actor_animation(tmp, 1, (u8*)&targets);
                    }
                }
            }
            field_command_history_clear(actor->object_index);
        }
    }
}

/* field_actor_action_runtime: Validate pending actions, clear completed state, and advance actor sequences. */

/** @brief Object mode that cancels a pending action unless an action selection holds it. */
#define FIELD_PENDING_MODE_CANCEL 3


/**
 * @brief Update pending actor actions and clear stale action state.
 * @param actor Actor whose runtime state and pending-action counter are checked.
 * @return One when the actor enters state 0x95; zero otherwise.
 * @note Selection 2 retries the pending action up to five frames (three for kind 0xA players).
 */
s32 field_update_pending_action(FieldActor* actor)
{
    s32 selection;
    s32 flags;
    u16 count;
    u8 object_index;
    s32 mode;
    FieldObjectRuntime* states;
    FieldObjectRuntime* state;

    if (actor->control.word & 0x1FF)
    {
        g_field_object_states[actor->object_index].object_flags &= 0xFFFF7FFF;
        actor->variant = actor->variant + 1;
        return 0;
    }
    selection = field_command_history_match(actor->object_index, (actor->animation >> 7) ^ 1, 1);
    states = g_field_object_states;
    state = &states[actor->object_index];
    if ((FIELD_OBJECT_ACTION_MODE(state) == 2 || actor->variant != 0) && actor->y == 0)
    {
        if (!(actor->control.word & 0x1FF))
        {
            flags = state->object_flags;
            if (!(flags & 0x400))
            {
                state->object_flags = flags | 0x8000;
            }
        }
        if (selection == 3)
        {
            g_field_object_states[actor->object_index].retry_count = 0;
            actor->variant = 0;
        }
        else if (selection < 4)
        {
            if (selection == 2)
            {
                count = actor->variant;
                if (count < 5U)
                {
                    actor->variant = count + 1;
                }
                if (field_get_next_animation_frame_count(actor) == 0 || actor->variant >= 5U ||
                    (actor->object_index < 2U && g_field_player_records[actor->object_index].kind == 0xA &&
                     actor->variant >= 3U))
                {
                    field_command_history_clear(actor->object_index);
                    actor->variant = 0;
                    g_field_object_states[actor->object_index].retry_count = 0;
                    g_field_object_states[actor->object_index].object_flags &= 0xFFFF7FFF;
                    field_restart_actor_animation_reverse(actor);
                    actor->command = FIELD_ACTOR_COMMAND_SEQUENCE_WAIT;
                    actor->command_param = 0x14;
                    return 1;
                }
            }
            else
            {
                g_field_object_states[actor->object_index].retry_count = 0;
                actor->variant = 0;
            }
        }
        else
        {
            g_field_object_states[actor->object_index].retry_count = 0;
            actor->variant = 0;
        }
    }
    object_index = actor->object_index;
    mode = FIELD_OBJECT_ACTION_MODE(&g_field_object_states[object_index]);
    switch (mode)
    {
    case FIELD_PENDING_MODE_CANCEL:
        if (selection != 4 && selection != 6 && selection != 5 && selection != 7 && selection != 8 && selection != 9 && selection != 10)
        {
            field_command_history_clear(object_index);
            g_field_object_states[actor->object_index].retry_count = 0;
            actor->variant = 0;
            g_field_object_states[actor->object_index].object_flags &= 0xFFFF7FFF;
            field_restart_actor_animation_reverse(actor);
            actor->command = FIELD_ACTOR_COMMAND_SEQUENCE_WAIT;
            actor->command_param = 0x14;
            return 1;
        }
        break;
    case 8:
    case 9:
    case 10:
        if (selection != 4 && selection != 6 && selection != 5 && selection != 7)
        {
            field_command_history_clear(object_index);
            g_field_object_states[actor->object_index].retry_count = 0;
            actor->variant = 0;
            g_field_object_states[actor->object_index].object_flags &= 0xFFFF7FFF;
        }
        break;
    }
    return 0;
}

/**
 * @brief Finish a sequence-driven action once its bound animation actor is released.
 * @param actor Actor whose movement flags and sequence state are cleared.
 */
void field_update_instrument_command(FieldActor* actor)
{
    FieldActorState* anim_slot;
    FieldObjectRuntime* state;
    FieldObjectRuntime* states = g_field_object_states;
    u8 object_index;
    u8 actor_index;
    s32 value;

    object_index = actor->object_index;
    state = &states[object_index];
    actor_index = state->contact.bytes.animation_actor_index;
    if (actor_index == FIELD_SEQUENCE_NO_ACTOR)
    {
        if (actor->animation_state != 0)
        {
            return;
        }
        state->movement.word &= ~0x1800;
        actor->command = 0;
        actor->control.word &= ~0x800;
        states[actor->object_index].effect_intensity = 0;
        if (actor->object_index < 2)
        {
            field_command_history_clear(actor->object_index);
        }
    }
    else
    {
        value = actor->animation_state;
        anim_slot = &g_field_actor_slots[actor_index];
        if (value != 0 || (anim_slot->active_track_mask != 0 && anim_slot->owner_object_index == object_index))
        {
            return;
        }
        /* The index is passed twice; field_resolve_collected_hits reads only the first. */
        value = object_index;
        field_resolve_collected_hits(value, object_index);
        actor->command = 0;
        states[actor->object_index].movement.word &= ~0x1800;
        if (anim_slot->owner_object_index == actor->object_index)
        {
            field_update_sequence_actor_binding(actor, 1);
        }
        actor->control.word &= ~0x800;
        states[actor->object_index].effect_intensity = 0;
        if (actor->object_index < 2)
        {
            field_command_history_clear(actor->object_index);
        }
    }
}

/**
 * @brief Read one byte of a party member's technique sequence.
 * @param sequence_index Script row within the member's bank.
 * @param object_index Party member whose bank (player record kind) is used.
 * @param cursor Byte offset within the row.
 * @return The sequence byte.
 */
static inline u8 field_sequence_byte(s32 sequence_index, s32 object_index, s32 cursor)
{
    u8* scripts = g_field_actor_sequence_data;
    FieldPlayerRecord* players = g_field_player_records;
    s32 row;

    row = (sequence_index << 5) + players[object_index].kind * FIELD_SEQUENCE_BANK_SIZE;
    return *(u8*)(row + (s32)scripts + cursor);
}

/**
 * @brief Advance an actor sequence and apply its pending horizontal motion.
 * @param actor Actor whose sequence cursor and motion remainder are advanced.
 * @param sequence_index Script row within the actor's selected bank.
 * @note Command 0xF1 advances the cursor; 0xEF handles sequence completion.
 * @note The loop-wrapped row computations are scheduling levers.
 */
void field_update_technique_command(FieldActor* actor, s32 sequence_index)
{
    Vec3i* scratch = (Vec3i*)FIELD_SEQUENCE_DISPLACEMENT_SCRATCH;
    FieldObjectRuntime* states;
    FieldObjectRuntime* state;
    FieldObjectRuntime* delay_state;
    FieldObjectRuntime* reset_state;
    FieldObjectRuntime* final_states;
    s32 active_cursor;
    s32 first_cursor;
    s32 bank;
    FieldPlayerRecord* players;
    FieldActorPartDef* part;
    u8* scripts;
    s32 object_index;
    s32 cursor;
    s32 row_offset;
    s32 row_address;
    s32 row_sum;
    s32 amount;
    u8 delay;
    u8 command;

    if (actor->animation_state == 0)
    {
        states = g_field_object_states;
        delay_state = &states[actor->object_index];
        delay = delay_state->sequence_delay;
        if (delay != 0)
        {
            delay_state->sequence_delay = delay - 1;
        }
        if (delay == 0 || states[actor->object_index].sequence_delay == 0)
        {
            /* Unless the bound actor still plays the current frame, run the sequence. */
            if ((field_object_has_active_actor_tracks(actor->object_index) == 0) ||
                (((active_cursor = states[actor->object_index].sequence_cursor) != 1) &&
                 ((command = field_sequence_byte(sequence_index, actor->object_index, active_cursor)) != FIELD_SEQUENCE_END) &&
                 (command != FIELD_SEQUENCE_WAIT_ANIMATION)))
            {
                reset_state = &states[actor->object_index];
                if (reset_state->sequence_cursor == 1)
                {
                    reset_state->movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;
                    states[actor->object_index].contact.bytes.target_count = 0;
                }
                scripts = g_field_actor_sequence_data;
                players = g_field_player_records;
                object_index = actor->object_index;

                do
                {
                    bank = players[object_index].kind;
                    row_offset = sequence_index << 5;
                    row_address = bank << 1;
                    row_address += bank;
                    row_address <<= 8;
                    row_sum = row_offset + row_address;
                    row_address = row_sum;
                } while (0);
                state = &states[object_index];
                first_cursor = state->sequence_cursor;
                row_address += (s32)scripts;
                row_address += first_cursor;
                if (*(u8*)row_address == FIELD_SEQUENCE_WAIT_ANIMATION)
                {
                    state->sequence_cursor = first_cursor + 1;
                }
                object_index = actor->object_index;
                do
                {
                    bank = players[object_index].kind;
                    row_address = bank << 1;
                    row_address += bank;
                    row_address <<= 8;
                    row_sum = row_offset + row_address;
                    row_address = row_sum;
                } while (0);
                state = &states[object_index];
                cursor = state->sequence_cursor;
                row_address += (s32)scripts;
                row_address += cursor;
                if (*(u8*)row_address == FIELD_SEQUENCE_WAIT_REPEAT)
                {
                    if (state->contact.bytes.target_count == 0)
                    {
                        field_resolve_collected_hits(object_index);
                        field_stop_actor_animations_for_object(actor, 1);
                        actor->command = 0;
                        field_update_sequence_actor_binding(actor, 1);
                        actor->control.word &= ~0x800;
                        if (actor->object_index < 2U)
                        {
                            field_command_history_clear(actor->object_index);
                        }
                        states[actor->object_index].object_flags &= ~0x4000;
                        states[actor->object_index].object_flags &= 0xFFFF7FFF;
                        return;
                    }
                    state->sequence_cursor = cursor + 1;
                    return;
                }
                if (field_execute_actor_sequence(actor, sequence_index) != 0)
                {
                    field_resolve_collected_hits(actor->object_index);
                    actor->command = 0;
                    field_update_sequence_actor_binding(actor, 1);
                    actor->control.word &= ~0x800;
                    if (actor->object_index < 2U)
                    {
                        field_command_history_clear(actor->object_index);
                    }
                    states[actor->object_index].object_flags &= ~0x4000;
                    states[actor->object_index].object_flags &= 0xFFFF7FFF;
                    return;
                }
                field_restart_actor_animation(actor);
                actor->control.word |= 0x800;
            }
        }
    }
    amount = actor->speed_accumulator / actor->frame_timer;
    part = &g_field_object_parts[actor->object_index];
    actor->speed_accumulator = actor->speed_accumulator - amount;
    if (actor->animation & FIELD_SEQUENCE_FACING)
    {
        scratch->x = ((amount << 8) * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    }
    else
    {
        scratch->x = (-(amount << 8) * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    }
    scratch->z = 0;
    scratch->y = 0;
    field_resolve_actor_movement(actor, scratch, 1);
    if (g_field_resource_entries[actor->resource_index].mode == 0)
    {
        final_states = g_field_object_states;
        final_states[actor->object_index].movement.word &= ~0x4000;
    }
}

/* field_actor_displacement: Apply actor displacement, follow leader history, and refresh collision contact. */

/**
 * @brief Apply a scaled movement step and clear selected states when the move is blocked.
 * @param actor Moving actor; frame_timer splits speed_accumulator into steps.
 * @param direction_x Horizontal direction scale.
 * @param vertical_step Vertical displacement before part scaling.
 * @param direction_z Depth direction scale.
 * @return Unspecified; callers ignore it.
 */
s32 field_move_actor_step(FieldActor* actor, s32 direction_x, s32 vertical_step, s32 direction_z)
{
    s32 step;
    FieldActorPartDef* part;
    s32* out;
    s16 state;

    out = (s32*)FIELD_SEQUENCE_DISPLACEMENT_SCRATCH;
    if (actor->animation_state == 0)
    {
        actor->command = 0;
    }
    else
    {
        field_update_actor_movement_animation(actor, direction_x, direction_z);
        step = actor->speed_accumulator / actor->frame_timer;
        actor->speed_accumulator = actor->speed_accumulator - step;
        part = &g_field_object_parts[actor->object_index];
        out[0] = (step * direction_x * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
        out[1] = (vertical_step * part->footprint_scale_y) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
        out[2] = (step * direction_z * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
        if (field_resolve_actor_movement(actor, out, 0) == 0)
        {
            state = actor->command;
            if (state == FIELD_ACTOR_COMMAND_WALK_TO_TARGET || state == FIELD_ACTOR_COMMAND_RUN_TO_TARGET || state == FIELD_ACTOR_COMMAND_WALK_FROM_TARGET ||
                state == FIELD_ACTOR_COMMAND_WALK_PATH || state == FIELD_ACTOR_COMMAND_RUN_PATH)
            {
                actor->command = 0;
            }
        }
    }
}

/**
 * @brief Move an actor by a speed-scaled X/Z displacement.
 * @param actor Actor supplying the speed byte; a blocked move clears its state.
 * @param x X displacement before scaling.
 * @param z Z displacement before scaling.
 */
void field_update_timed_slide(FieldActor* actor, s32 x, s32 z)
{
    Vec3i* vector;
    s32 scaled;

    scaled = x * actor->command_param;
    vector = (Vec3i*)FIELD_SEQUENCE_DISPLACEMENT_SCRATCH;
    vector->y = 0;
    vector->x = scaled;
    vector->z = z * actor->command_param;
    if (field_resolve_actor_movement(actor, (s32*)vector, 0) == 0)
    {
        actor->command = 0;
    }
}

/**
 * @brief History point @p point of object @p object_index, indexed through one flat point array.
 * @note The object and point indices are folded into one four-byte stride, as the original does.
 */
#define FIELD_HISTORY_POINT(states, object_index, point)                                                                                                       \
    (*(FieldObjectHistoryPoint*)((u8*)(states) +                                                                                                               \
                                 ((object_index) * (sizeof(FieldObjectRuntime) / sizeof(FieldObjectHistoryPoint)) + (point)) *                                 \
                                     sizeof(FieldObjectHistoryPoint) +                                                                                         \
                                 0x6C /* position_history */))

/**
 * @brief Follow the leader's stored positions or restore the actor's idle behavior.
 * @param actor Follower whose movement and history index are updated.
 */
void field_follow_leader(FieldActor* actor)
{
    VECTOR* delta = (VECTOR*)0x1F800010;
    VECTOR* squares = (VECTOR*)0x1F800000;
    FieldObjectRuntime* states;
    FieldObjectRuntime* state;
    s32 distance;
    u8 index;
    u8 next;
    s32 facing;

    delta->vy = 0;
    delta->vx = (g_field_actors[0].x - actor->x) / 256;
    delta->vz = (g_field_actors[0].z - actor->z) / 256;
    gte_ldlvl(delta);
    gte_sqr0();
    gte_stlvnl(squares);
    index = actor->object_index;
    distance = squares->vx + squares->vz;
    if (((index + 1) * 2000 < distance) && (states = g_field_object_states, state = &states[index], next = state->history_index, next < 47))
    {
        state->history_index = next + 1;
        delta->vx = (FIELD_HISTORY_POINT(states, g_field_actors[0].object_index, states[actor->object_index].history_index).x << 8) - actor->x;
        delta->vz = (FIELD_HISTORY_POINT(states, g_field_actors[0].object_index, states[actor->object_index].history_index).z << 8) - actor->z;
        gte_ldlvl(delta);
        gte_sqr12();
        gte_stlvnl(squares);
        if (squares->vx + squares->vz > 384 && !(g_field_resource_entries[actor->resource_index].flags & 1))
        {
            actor->running = 1;
        }
        else
        {
            actor->running = 0;
        }
    }
    else
    {
        if (g_field_resource_entries[actor->resource_index].flags & 1)
        {
            actor->animation &= 0x80;
        }
        else
        {
            facing = actor->animation & 0x80;
            facing += 2;
            actor->animation = facing;
        }
        actor->animation_state = 1;
        field_restart_sequence_animation(actor);
        actor->running = 0;
        actor->command = 0;
        return;
    }
    field_update_actor_movement_animation(actor, delta->vx, delta->vz);
    if (field_resolve_actor_movement(actor, (s32*)delta, 0) == 0)
    {
        actor->running = 0;
        actor->command = 0;
    }
}

/**
 * @brief Drop a bound action once its actor stops, or move the actor by a speed-scaled vector.
 * @param actor Actor whose binding or movement is updated.
 * @param x X displacement before scaling.
 * @param y Y displacement before scaling.
 * @param z Z displacement before scaling.
 * @note Each binding lookup clamps objects 2 and up to the shared third binding.
 */
void field_update_actor_jump(FieldActor* actor, s32 x, s32 y, s32 z)
{
    s32 offset;
    s32 owner;
    s32 object_index;
    u8* owner_bindings;
    u8* actor_bindings;
    u8* track_bindings;
    FieldActorState* anim_slots;
    FieldActorState* anim_slot;
    s32* scratch;

    scratch = (s32*)FIELD_SEQUENCE_DISPLACEMENT_SCRATCH;
    if (actor->animation_state == 0)
    {
        owner_bindings = (u8*)g_field_actor_bindings;
        if (actor->object_index < 2U)
        {
            offset = actor->object_index * sizeof(FieldSequenceBinding);
        }
        else
        {
            offset = FIELD_SEQUENCE_SHARED_BINDING * sizeof(FieldSequenceBinding);
        }
        object_index = actor->object_index;
        owner = ((FieldSequenceBinding*)(owner_bindings + offset))->owner_object_index;
        if (owner == object_index)
        {
            anim_slots = g_field_actor_slots;
            actor_bindings = (u8*)g_field_actor_bindings;
            if ((u8)owner < 2U)
            {
                offset = owner * sizeof(FieldSequenceBinding);
            }
            else
            {
                offset = FIELD_SEQUENCE_SHARED_BINDING * sizeof(FieldSequenceBinding);
            }
            anim_slot = anim_slots + ((FieldSequenceBinding*)(actor_bindings + offset))->actor_index;
            if (anim_slot->is_active != 0)
            {
                anim_slots = g_field_actor_slots;
                track_bindings = (u8*)g_field_actor_bindings;
                if (actor->object_index < 2U)
                {
                    offset = actor->object_index * sizeof(FieldSequenceBinding);
                }
                else
                {
                    offset = FIELD_SEQUENCE_SHARED_BINDING * sizeof(FieldSequenceBinding);
                }
                anim_slot = anim_slots + ((FieldSequenceBinding*)(track_bindings + offset))->actor_index;
                if (anim_slot->active_track_mask == 0)
                {
                    actor->command = 0;
                }
            }
            else
            {
                actor->command = 0;
            }
        }
        else
        {
            actor->command = 0;
        }
    }
    else
    {
        scratch[0] = x * actor->command_param;
        scratch[1] = y * actor->command_param;
        scratch[2] = z * actor->command_param;
        field_resolve_actor_movement(actor, scratch, 0);
    }
}

/**
 * @brief Count down an actor's step timer and slide its height by its speed.
 * @param actor Actor whose timer, height and state are updated.
 * @param rising Nonzero lowers y by the speed; zero raises it and stops at ground level.
 */
void field_update_actor_lift(FieldActor* actor, s32 rising)
{
    s8 timer;

    timer = actor->unk26 - 1;
    actor->unk26 = timer;
    if (timer == 0)
    {
        actor->command = 0;
    }

    if (rising != 0)
    {
        actor->y -= actor->command_param << 8;
    }
    else
    {
        s32 height = actor->y + (actor->command_param << 8);
        actor->y = height;
        if (height >= 0)
        {
            actor->y = 0;
            actor->command = 0;
        }
    }
}

/**
 * @brief Move an actor on the ground plane by a speed-scaled X/Z step.
 * @param actor Actor whose position and speed are used.
 * @param dx X step before scaling.
 * @param dz Z step before scaling.
 */
void field_slide_actor(FieldActor* actor, s32 dx, s32 dz)
{
    u8 speed;

    speed = actor->command_param;
    actor->x = actor->x + (dx * speed);
    actor->z = actor->z + (dz * speed);
}

/** @brief Scratchpad collision request and resolver output. */
typedef struct
{
    s32 x, y, z, dx, dy, dz, height, contact, surface;
    s16 radius, depth;
    union
    {
        s32 flags;
        struct
        {
            unsigned step : 16;
            unsigned bit16 : 1;
            unsigned bit17 : 1;
            unsigned high : 14;
        } bits;
    } mode;
} FieldActorCollisionMover;
/** @brief Map dimensions used to validate fixed-point actor coordinates. */
typedef struct
{
    s16 width;
    u16 height;
} FieldActorCollisionBounds;

/**
 * @brief Move an actor by its speed and refresh its collision contact and height.
 * @param actor Actor whose position, speed and runtime state are updated.
 * @param dx Horizontal direction or displacement multiplier.
 * @param dz Depth direction or displacement multiplier.
 * @note Positions outside the map bounds at 0x801ED400 clear the contact instead.
 */
void field_update_timed_walk(FieldActor* actor, s32 dx, s32 dz)
{
    FieldActorCollisionBounds* bounds = (FieldActorCollisionBounds*)0x801ED400;
    FieldActorCollisionMover* mover = (FieldActorCollisionMover*)0x1F800000;
    s32 x, z;
    u8 speed;

    speed = actor->command_param;
    actor->x += dx * speed;
    actor->z += dz * speed;
    x = actor->x;
    z = actor->z;
    if (x >= 0 && x < (bounds->width << 8) && z >= 0 && z < ((s32)(bounds->height << 16) >> 7))
    {
        mover->x = x;
        mover->y = actor->y;
        mover->z = actor->z;
        mover->dx = 0;
        mover->dy = 0;
        mover->dz = 0;
        if (g_field_object_parts[actor->object_index].appearance.fields.footprint_scale_x == 0x40)
        {
            mover->radius = 12;
            mover->mode.bits.step = 8;
        }
        else
        {
            mover->radius = 9;
            mover->mode.bits.step = 6;
        }
        mover->depth = 16;
        /* Two separate bitfield clears, as in the original. */
        mover->mode.bits.bit17 = 0;
        mover->mode.bits.bit16 = 0;
        mover->contact = g_field_object_states[actor->object_index].contact_index;
        mover->surface = g_field_object_states[actor->object_index].surface;
        field_collision_move_mover((struct FieldCollisionMover*)mover);
        g_field_object_states[actor->object_index].contact_index = mover->contact;
        g_field_object_states[actor->object_index].surface = mover->surface;
        g_field_object_states[actor->object_index].movement.half.height = mover->height / 256;
    }
    else
    {
        g_field_object_states[actor->object_index].contact_index = -1;
        g_field_object_states[actor->object_index].surface = 0;
        g_field_object_states[actor->object_index].movement.half.height = 0;
    }
}

/* field_actor_resource_states: Advance actor states after their resource requests complete. */

/**
 * @brief Start the actor's bound defeat animation and switch to FIELD_ACTOR_COMMAND_KNOCKED_DOWN.
 * @param actor Actor whose resource entry supplies the animation flags.
 * @return The busy flag or start result, or 0x8E after a successful start.
 */
s32 field_start_defeat_bound_animation(FieldActor* actor)
{
    s32 result;

    result = g_field_object_states[actor->object_index].contact.bytes.flags_low & 1;
    if (result == 0)
    {
        result = field_start_bound_action_animation(actor->object_index, 0, 0, g_field_resource_entries[actor->resource_index].bound_animation_flags);
        if (result != 0)
        {
            result = FIELD_ACTOR_COMMAND_KNOCKED_DOWN;
            actor->command = result;
        }
    }
    return result;
}

/**
 * @brief Start the actor's bound defeat animation and switch to FIELD_ACTOR_COMMAND_DEFEAT_END.
 * @param actor Actor whose resource entry supplies the animation flags.
 * @return The busy flag or start result, or 0x94 after a successful start.
 */
s32 field_start_defeat_wait_animation(FieldActor* actor)
{
    s32 result;

    result = g_field_object_states[actor->object_index].contact.bytes.flags_low & 1;
    if (result == 0)
    {
        result = field_start_bound_action_animation(actor->object_index, 0, 0, g_field_resource_entries[actor->resource_index].bound_animation_flags);
        if (result != 0)
        {
            result = FIELD_ACTOR_COMMAND_DEFEAT_END;
            actor->command = result;
        }
    }
    return result;
}

/* field_actor_animation_resume: Detect released animation slots and resume the actor record animation. */

/**
 * @brief Resume the actor's idle animation once its bound actor slot is free.
 * @param actor Actor whose binding selects the actor slot.
 * @return Unspecified; callers ignore it.
 */
s32 field_update_defeat_end(FieldActor* actor)
{
    FieldActorState* anim_slots;
    u8* bindings;
    s32 offset;
    s32 actor_index;
    FieldActorState* anim_slot;

    anim_slots = g_field_actor_slots;
    bindings = (u8*)g_field_actor_bindings;
    if (actor->object_index < 2)
    {
        offset = actor->object_index * sizeof(FieldSequenceBinding);
    }
    else
    {
        offset = FIELD_SEQUENCE_SHARED_BINDING * sizeof(FieldSequenceBinding);
    }
    actor_index = ((FieldSequenceBinding*)(bindings + offset))->actor_index;
    anim_slot = anim_slots + actor_index;
    if (anim_slot->is_active == 0)
    {
        actor->command = 0;
        actor->presence = FIELD_ACTOR_UNUSED;
        field_restart_idle_animation(actor);
    }
}

/**
 * @brief Restart the actor's idle animation, or its first idle track for idle kind 0x1F.
 * @param actor Actor whose runtime state holds the idle animation.
 */
void field_restart_idle_animation(FieldActor* actor)
{
    s32 i;
    s32 actor_index;
    s32 animation_id;

    if (FIELD_OBJECT_IDLE_ANIMATION(&g_field_object_states[actor->object_index]) == 0xFF)
    {
        return;
    }
    if (FIELD_OBJECT_IDLE_ANIMATION(&g_field_object_states[actor->object_index]) == 0x1F)
    {
        for (i = 0; i < 4; i++)
        {
            if (FIELD_OBJECT_IDLE_FLAGS(&g_field_object_states[actor->object_index])[i] != 0)
            {
                animation_id = FIELD_OBJECT_IDLE_ANIMATION(&g_field_object_states[actor->object_index]);
                actor_index = field_find_free_actor_slot(actor->object_index, 0);
                if (actor_index != -1)
                {
                    if (field_start_builtin_animation(actor->object_index, actor_index, animation_id))
                    {
                        field_start_actor_animation(actor_index, 0, 0);
                    }
                }
                return;
            }
        }
        return;
    }
    animation_id = FIELD_OBJECT_IDLE_ANIMATION(&g_field_object_states[actor->object_index]);
    actor_index = field_find_free_actor_slot(actor->object_index, 0);
    if (actor_index != -1)
    {
        if (field_start_builtin_animation(actor->object_index, actor_index, animation_id))
        {
            field_start_actor_animation(actor_index, 0, 0);
        }
    }
}

/**
 * @brief Resume the actor's idle animation once its reserved actor slot is free.
 * @param actor Actor whose index selects the reserved slot at g_field_actor_slots[64 + index].
 * @return Unspecified; the return register is left live but no caller reads it.
 */
s32 field_update_defeated(FieldActor* actor)
{
    if (g_field_actor_slots[actor->object_index + 0x40].is_active == 0)
    {
        actor->command = 0;
        actor->presence = FIELD_ACTOR_UNUSED;
        field_restart_idle_animation(actor);
    }
}

/* field_actor_sequence_runtime: Execute object sequences, manage their animation actors, and update tint flashing. */

/**
 * @brief Consume a signed displacement remainder and apply a scaled movement step.
 * @param actor Moving actor; frame_timer splits speed_accumulator into steps.
 * @param direction_x Horizontal direction scale.
 * @param vertical_step Vertical displacement before part scaling.
 * @param direction_z Depth direction scale.
 */
void field_apply_sequence_displacement(FieldActor* actor, s32 direction_x, s32 vertical_step, s32 direction_z)
{
    s32 step;
    FieldActorPartDef* part;
    s32* out;

    out = (s32*)FIELD_SEQUENCE_DISPLACEMENT_SCRATCH;
    if (actor->animation_state == 0)
    {
        actor->command = 0;
        return;
    }
    step = actor->speed_accumulator / actor->frame_timer;
    actor->speed_accumulator = actor->speed_accumulator - step;
    part = &g_field_object_parts[actor->object_index];
    out[0] = (step * direction_x * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    out[1] = (vertical_step * part->footprint_scale_y) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    out[2] = (step * direction_z * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    field_resolve_actor_movement(actor, out, 0);
}

/**
 * @brief Mark an owned animation as running, or release its completed binding.
 * @param actor Actor selecting the binding, with indices above one sharing the third entry.
 * @param release_actor Clear the completed actor's active flag before releasing its binding.
 */
void field_update_sequence_actor_binding(FieldActor* actor, s32 release_actor)
{
    FieldSequenceBinding* base;
    s32 object_index;
    s32 active_owner;
    s32 finished_owner;
    s32 binding_offset;
    s32 animation_binding_offset;
    s32 owner_binding_offset;
    s32 active_binding_offset;
    s32 finished_binding_offset;
    s32 release_binding_offset;

    base = g_field_actor_bindings;
    if (actor->object_index < 2U)
    {
        binding_offset = (actor->object_index) * sizeof(*base);
    }
    else
    {
        binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
    }
    if (((FieldSequenceBinding*)((u8*)base + binding_offset))->state != 0)
    {
        base = g_field_actor_bindings;
        if (actor->object_index < 2U)
        {
            animation_binding_offset = (actor->object_index) * sizeof(*base);
        }
        else
        {
            animation_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
        }
        if (field_is_actor_animation_active(((FieldSequenceBinding*)((u8*)base + animation_binding_offset))->actor_index) != 0)
        {
            base = g_field_actor_bindings;
            if (actor->object_index < 2U)
            {
                owner_binding_offset = (actor->object_index) * sizeof(*base);
            }
            else
            {
                owner_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
            }
            if (((FieldSequenceBinding*)((u8*)base + owner_binding_offset))->owner_object_index == actor->object_index)
            {
                FieldActorState* anim_slots;
                FieldSequenceBinding* lookup;

                active_owner = ((FieldSequenceBinding*)((u8*)base + owner_binding_offset))->owner_object_index;
                anim_slots = g_field_actor_slots;
                lookup = g_field_actor_bindings;
                if ((u32)(active_owner & 0xFF) < 2U)
                {
                    active_binding_offset = (active_owner) * sizeof(*base);
                }
                else
                {
                    active_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
                }
                {
                    u32 actor_address = (u32)anim_slots;
                    actor_address += ((FieldSequenceBinding*)((u8*)lookup + active_binding_offset))->actor_index * sizeof(*anim_slots);
                    ((FieldActorState*)actor_address)->sequence_active = 1;
                }
            }
        }
        else
        {
            base = g_field_actor_bindings;
            if (actor->object_index < 2U)
            {
                finished_binding_offset = (actor->object_index) * sizeof(*base);
            }
            else
            {
                finished_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
            }
            object_index = actor->object_index;
            finished_owner = ((FieldSequenceBinding*)((u8*)base + finished_binding_offset))->owner_object_index;
            if (finished_owner == object_index)
            {
                if (release_actor != 0)
                {
                    FieldActorState* anim_slots;
                    FieldSequenceBinding* lookup;

                    anim_slots = g_field_actor_slots;
                    lookup = g_field_actor_bindings;
                    if ((u32)(finished_owner & 0xFF) < 2U)
                    {
                        release_binding_offset = (finished_owner) * sizeof(*base);
                    }
                    else
                    {
                        release_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
                    }
                    {
                        u32 actor_address = (u32)anim_slots;
                        actor_address += ((FieldSequenceBinding*)((u8*)lookup + release_binding_offset))->actor_index * sizeof(*anim_slots);
                        ((FieldActorState*)actor_address)->is_active = 0;
                    }
                }
                field_release_actor_binding(actor->object_index);
            }
        }
    }
}

/**
 * @brief Consume sequence commands until a frame, wait, delay, or terminator is reached.
 * @param actor Actor whose runtime state holds the cursor and animation binding.
 * @param script_index Row within the player's selected sequence bank.
 * @return One if the initial cursor already points at the terminator; zero otherwise.
 * @note Animation targets are expanded to four-byte entries for the animation API.
 * @note Command cases retain independent movement-flag updates and target-copy cursors.
 */
s32 field_execute_actor_sequence(FieldActor* actor, s32 script_index)
{
    FieldObjectRuntime* slots;
    FieldPlayerRecord* players;
    u8* programs;
    u8* initial_program;
    u8* initial_program_base;
    FieldSequenceBinding* bindings;
    FieldSequenceBinding* binding_test;
    FieldActorState* anim_slots;
    s32 parameters[FIELD_SEQUENCE_TARGET_LIMIT];
    FieldActorState* copy_source;
    FieldActorState* template_actor;
    s32 actor_index;
    s32 script_offset;
    s32 bank_offset;
    s32 pending_command;
    s32 target_index;
    s32 current_target_index;
    s32 animation_command;
    u32 delay_operand;
    u32 resource_operand;
    u32 animation_operand;
    s32 clear_slot;
    s32 movement_mask;
    s32 cursor;
    s32 result;
    s32 initial_binding_offset;
    s32 restore_binding_offset;
    s32 release_binding_offset;
    s32 updated_flags;
    u8* opcode_ptr;
    s32 target_owner;
    s32 current_target_owner;
    u8 delay_owner;
    u8 resource_owner;
    u8 animation_owner;
    s32 allocation_owner;
    u8 command;
    u8 pending_owner;
    u8 initial_owner;
    u8 command_owner;
    u8 opcode;
    s32 command_slot;
    FieldObjectRuntime* target_state;
    FieldObjectRuntime* allocation_state;
    FieldObjectRuntime* animation_state;
    FieldObjectRuntime* pending_state;
    FieldObjectRuntime* current_target_state;
    FieldObjectRuntime* current_animation_state;
    FieldObjectRuntime* current_allocation_state;
    FieldActorState* pending_actor;
    FieldActorState* pending_actors;
    FieldObjectRuntime* flag_state;

    g_field_object_states[actor->object_index].sequence_delay = 0;
    pending_owner = actor->object_index;
    pending_state = &g_field_object_states[pending_owner];
    pending_command = pending_state->sequence_command;
    if (pending_command != FIELD_SEQUENCE_COMMAND_NONE)
    {
        if (pending_command & FIELD_SEQUENCE_TRANSIENT_ACTOR)
        {
            if (pending_state->contact.bytes.animation_actor_index < FIELD_SEQUENCE_ACTOR_LIMIT)
            {
                pending_actors = g_field_actor_slots;
                pending_actor = &pending_actors[pending_state->contact.bytes.animation_actor_index];
                if ((pending_actor->is_active != 0) && (pending_actor->owner_object_index == pending_owner))
                {
                    pending_actor->is_active = 0U;
                }
            }
            g_field_object_states[actor->object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
        }
    }
    initial_program_base = g_field_actor_sequence_data;
    initial_owner = actor->object_index;
    cursor = g_field_object_states[initial_owner].sequence_cursor;
    initial_program =
        (script_index * FIELD_SEQUENCE_ROW_SIZE) + (g_field_player_records[initial_owner].kind * FIELD_SEQUENCE_BANK_SIZE) + initial_program_base + cursor;
    if (*initial_program == FIELD_SEQUENCE_END)
    {
        return 1;
    }
    if (cursor == 1)
    {
        binding_test = g_field_actor_bindings;
        if (initial_owner < 2U)
        {
            initial_binding_offset = (initial_owner) * sizeof(*bindings);
        }
        else
        {
            initial_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*bindings);
        }
        result = 0;
        if (((FieldSequenceBinding*)((u8*)binding_test + initial_binding_offset))->state != FIELD_SEQUENCE_RESTORE_TEMPLATE)
        {
            return 0;
        }
        {
            FieldActorState* restore_actors;
            FieldSequenceBinding* restore_bindings;
            restore_actors = g_field_actor_slots;
            restore_actors[g_field_actor_bindings[actor->object_index].actor_index].sequence_active = 0;
            restore_actors[g_field_actor_bindings[actor->object_index].actor_index].track_count = 0;
            restore_bindings = g_field_actor_bindings;
            if (actor->object_index < 2U)
            {
                restore_binding_offset = (actor->object_index) * sizeof(*restore_bindings);
            }
            else
            {
                restore_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*restore_bindings);
            }
            copy_source = &g_field_actor_slots[((FieldSequenceBinding*)((u8*)restore_bindings + restore_binding_offset))->actor_index];
            if (actor->object_index < 2U)
            {
                template_actor = &g_field_actor_templates[actor->object_index].actor;
            }
            else
            {
                template_actor = &g_field_shared_actor_template;
            }
            /* Restore the bound runtime actor into its player template. */
            bcopy((const u8*)copy_source, (u8*)template_actor, sizeof(*copy_source));
            anim_slots = g_field_actor_slots;
            bindings = g_field_actor_bindings;
            if (actor->object_index < 2U)
            {
                release_binding_offset = actor->object_index * sizeof(*bindings);
            }
            else
            {
                release_binding_offset = FIELD_SEQUENCE_SHARED_BINDING * sizeof(*bindings);
            }
        }
        {
            s32 actor_offset = ((FieldSequenceBinding*)((u8*)bindings + release_binding_offset))->actor_index * sizeof(*anim_slots);
            ((FieldActorState*)((s32)anim_slots + actor_offset))->is_active = 0;
        }
    }
    programs = g_field_actor_sequence_data;
    players = g_field_player_records;
    slots = g_field_object_states;
    script_offset = script_index * FIELD_SEQUENCE_ROW_SIZE;
    command_slot = actor->object_index;
    opcode_ptr = script_offset + players[command_slot].kind * FIELD_SEQUENCE_BANK_SIZE + programs + cursor;
    opcode = *opcode_ptr;
    result = 0;
    /* Frame bytes stop dispatch; command bytes may consume additional operands. */
    for (; opcode >= FIELD_SEQUENCE_START_TARGETS_0; command_slot = actor->object_index,
                                                     bank_offset = script_offset + players[command_slot].kind * FIELD_SEQUENCE_BANK_SIZE,
                                                     opcode_ptr = (u8*)(bank_offset + (s32)programs + cursor), opcode = *opcode_ptr)
    {
        switch (opcode)
        {
        case FIELD_SEQUENCE_END:
            g_field_object_states[command_slot].sequence_cursor = cursor;
            actor->animation_state = FIELD_SEQUENCE_FRAME_WAIT;
            actor->animation_frame = 0;
            actor->animation_active = 1;
            actor->animation = (u8)(actor->animation & FIELD_SEQUENCE_FACING);
            return 0;
        default:
            command = *opcode_ptr;
            switch (command)
            {
            case FIELD_SEQUENCE_START_TARGETS_0:
            case FIELD_SEQUENCE_START_TARGETS_1:
            case FIELD_SEQUENCE_START_TARGETS_2:
            {
                FieldObjectRuntime* source_state;
                s32 kind_flags;
                s32 sequence_command;
                source_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                kind_flags = ((command - FIELD_SEQUENCE_START_TARGETS_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                sequence_command = source_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK;
                target_state = source_state;
                sequence_command |= kind_flags;
                sequence_command |= FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                target_state->sequence_command = sequence_command;
            }
                target_owner = actor->object_index;
                actor_index = field_allocate_sequence_actor(target_owner, slots[target_owner].sequence_command);
                {
                    s32 copy_owner;
                    s32* target_output;
                    copy_owner = actor->object_index;
                    target_index = 0;
                    if (slots[copy_owner].contact.bytes.target_count != 0)
                    {
                        target_output = parameters;
                        do
                        {
                            *target_output = slots[copy_owner].targets[target_index];
                            target_index++;
                            target_output++;
                            copy_owner = actor->object_index;
                        } while (target_index < slots[copy_owner].contact.bytes.target_count);
                    }
                }
                field_start_actor_animation(actor_index, slots[actor->object_index].contact.bytes.target_count, (u8*)parameters);
                cursor += 1;
                slots[actor->object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                continue;
            case FIELD_SEQUENCE_START_CURRENT_TARGETS:
                current_target_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                current_target_state->sequence_command = current_target_state->current_sequence_animation;
                current_target_owner = actor->object_index;
                actor_index = field_allocate_sequence_actor(current_target_owner, slots[current_target_owner].current_sequence_animation);
                slots[actor->object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                {
                    s32 copy_owner;
                    s32* target_output;
                    copy_owner = actor->object_index;
                    current_target_index = 0;
                    if (slots[copy_owner].contact.bytes.target_count != 0)
                    {
                        target_output = parameters;
                        do
                        {
                            *target_output = slots[copy_owner].targets[current_target_index];
                            current_target_index++;
                            target_output++;
                            copy_owner = actor->object_index;
                        } while (current_target_index < slots[copy_owner].contact.bytes.target_count);
                    }
                }
                field_start_actor_animation(actor_index, slots[actor->object_index].contact.bytes.target_count, (u8*)parameters);
                cursor += 1;
                continue;
            case FIELD_SEQUENCE_DELAY:
                result = 0;
                delay_owner = actor->object_index;
                pending_state = (FieldObjectRuntime*)(delay_owner * sizeof(*slots));
                delay_operand = script_offset + players[delay_owner].kind * FIELD_SEQUENCE_BANK_SIZE;
                delay_operand += (u32)programs;
                delay_operand += cursor;
                pending_state = (FieldObjectRuntime*)((u8*)pending_state + (s32)slots);
                pending_state->sequence_delay = ((u8*)delay_operand)[1];
                cursor += 2;
                slots[actor->object_index].sequence_cursor = cursor;
                return result;
            case FIELD_SEQUENCE_WAIT_REPEAT:
            case FIELD_SEQUENCE_WAIT_ANIMATION:
                slots[actor->object_index].sequence_cursor = cursor;
                return 0;
            case FIELD_SEQUENCE_TOGGLE_CONTROL_14:
                flag_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                updated_flags = flag_state->object_flags ^ 0x4000;
                flag_state->object_flags = updated_flags;
                cursor += 1;
                field_update_object_effects(actor->object_index);
                continue;
            case FIELD_SEQUENCE_TOGGLE_CONTROL_15:
                flag_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                updated_flags = flag_state->object_flags ^ 0x8000;
                flag_state->object_flags = updated_flags;
                cursor += 1;
                field_update_object_effects(actor->object_index);
                continue;
            case FIELD_SEQUENCE_TOGGLE_FACING:
                cursor += 1;
                actor->animation = (u8)(actor->animation ^ FIELD_SEQUENCE_FACING);
                continue;
            case FIELD_SEQUENCE_START_RESOURCE:
                actor_index = field_find_free_actor_slot(actor->object_index, 0);
                if (actor_index != -1)
                {
                    resource_owner = actor->object_index;
                    resource_operand = script_offset + players[resource_owner].kind * FIELD_SEQUENCE_BANK_SIZE;
                    resource_operand += (u32)programs;
                    resource_operand += cursor;
                    field_start_builtin_animation(resource_owner, actor_index, ((u8*)resource_operand)[1]);
                    field_start_actor_animation(actor_index, 0U, NULL);
                }
                cursor += 2;
                clear_slot = actor->object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_START_0:
            case FIELD_SEQUENCE_START_1:
            case FIELD_SEQUENCE_START_2:
                animation_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                {
                    s32 kind_flags;
                    s32 sequence_command;
                    kind_flags = ((command - FIELD_SEQUENCE_START_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                    sequence_command = animation_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK;
                    sequence_command |= kind_flags;
                    sequence_command |= FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                    animation_state->sequence_command = sequence_command;
                }
                animation_owner = actor->object_index;
                field_start_actor_animation(field_allocate_sequence_actor(animation_owner, slots[animation_owner].sequence_command), 0U, NULL);
                slots[actor->object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                clear_slot = actor->object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    cursor += 1;
                    movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_START_CURRENT:
                current_animation_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                current_animation_state->sequence_command = current_animation_state->current_sequence_animation;
                allocation_owner = actor->object_index;
                actor_index = field_allocate_sequence_actor(allocation_owner, slots[allocation_owner].current_sequence_animation);
                slots[actor->object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                field_start_actor_animation(actor_index, 0U, NULL);
                clear_slot = actor->object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    cursor += 1;
                    movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_SET_ANIMATION:
                command_owner = actor->object_index;
                target_state = (FieldObjectRuntime*)(command_owner * sizeof(*slots));
                animation_operand = script_offset + players[command_owner].kind * FIELD_SEQUENCE_BANK_SIZE;
                animation_operand += (u32)programs;
                animation_operand += cursor;
                target_state = (FieldObjectRuntime*)((s32)target_state + (s32)slots);
                target_state->sequence_command = ((u8*)animation_operand)[1];
                cursor += 2;
                clear_slot = actor->object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_ALLOCATE_0:
            case FIELD_SEQUENCE_ALLOCATE_1:
            case FIELD_SEQUENCE_ALLOCATE_2:
                allocation_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                {
                    s32 kind_flags;
                    s32 sequence_command;
                    kind_flags = ((command - FIELD_SEQUENCE_ALLOCATE_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                    sequence_command = allocation_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK;
                    sequence_command |= kind_flags;
                    sequence_command |= FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                    allocation_state->sequence_command = sequence_command;
                }
                allocation_owner = actor->object_index;
                animation_command = slots[allocation_owner].sequence_command;
                field_allocate_sequence_actor(allocation_owner, animation_command);
                clear_slot = actor->object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    cursor += 1;
                    movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_ALLOCATE_CURRENT:
                current_allocation_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                current_allocation_state->sequence_command = current_allocation_state->current_sequence_animation;
                allocation_owner = actor->object_index;
                animation_command = slots[allocation_owner].current_sequence_animation;
                field_allocate_sequence_actor(allocation_owner, animation_command);
                clear_slot = actor->object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    cursor += 1;
                    movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            default:
                continue;
            }
        }
    }
    {
        u8* frame_programs;
        FieldPlayerRecord* frame_players;
        FieldObjectRuntime* frame_slots;
        u32 frame_address;
        s32 frame_offset;
        frame_programs = g_field_actor_sequence_data;
        frame_players = g_field_player_records;
        frame_offset = (script_index * FIELD_SEQUENCE_ROW_SIZE) + frame_players[actor->object_index].kind * FIELD_SEQUENCE_BANK_SIZE;
        frame_address = frame_offset;
        frame_address += (u32)frame_programs;
        frame_address += cursor;
        cursor++;
        frame_slots = g_field_object_states;
        actor->animation = *(u8*)frame_address + (actor->animation & FIELD_SEQUENCE_FACING);
        frame_slots[actor->object_index].sequence_cursor = cursor;
        actor->animation_state = FIELD_SEQUENCE_FRAME_WAIT;
        actor->animation_frame = 0;
        actor->animation_active = 1;
        return 0;
    }
}

/**
 * @brief Copy an object template into a free actor slot and select its animation.
 * @param index Object/template index, clamped to two only for the binding table.
 * @param flags Animation override flag and two-bit animation index.
 * @return Assigned actor slot, or -1 when allocation fails.
 */
s32 field_allocate_sequence_actor(s32 index, s32 flags)
{
    s32 slot, binding;
    FieldActorState *actor, *updated, *slots;
    FieldSequenceBinding* bindings;
    slot = field_find_free_actor_slot(index, 0);
    if (slot != -1)
    {
        actor = &g_field_actor_slots[slot];
        bcopy((const u8*)&g_field_actor_templates[index], (u8*)actor, sizeof(*actor));
        actor->is_active = 1;
        actor->actor_index = slot;
        if (flags & FIELD_SEQUENCE_ANIMATION_OVERRIDE)
        {
            actor->animation_index = (flags >> 12) & 3;
            actor->unknown_0x222 = actor->animations[actor->animation_index].unknown_0x12;
        }
        else
        {
            actor->animation_index = 0;
            actor->unknown_0x222 = actor->animations->unknown_0x12;
        }
        slots = g_field_actor_slots;
        updated = &slots[slot];
        binding = index;
        updated->animation_mode = updated->animations[updated->animation_index].animation_mode;
        updated->animation = &updated->animations[updated->animation_index];
        g_field_object_states[binding].contact.bytes.animation_actor_index = slot;
        bindings = g_field_actor_bindings;
        if (binding >= FIELD_SEQUENCE_BINDING_COUNT)
        {
            binding = FIELD_SEQUENCE_SHARED_BINDING;
        }
        bindings[binding].actor_index = slot;
    }
    else
    {
        g_field_object_states[index].contact.bytes.animation_actor_index = FIELD_SEQUENCE_NO_ACTOR;
    }
    return slot;
}

/**
 * @brief Reset sequence frame progress and restart the actor's animation.
 * @param actor Actor whose movement flags and animation state are reset.
 */
void field_restart_sequence_animation(FieldActor* actor)
{
    actor->animation_state = FIELD_SEQUENCE_FRAME_WAIT;
    actor->animation_frame = 0;
    actor->animation_active = 1;

    g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;

    field_restart_actor_animation(actor);
}

/**
 * @brief Update object tint colors for timed flashes and active selection blinking.
 * @note Flash phases dim each base color to 100/128 of its value.
 */
void field_update_object_tints(void)
{
    s32 i;
    FieldActorPartDef* visual = g_field_object_parts;
    FieldObjectRuntime* slot = g_field_object_states;
    u32 flags;
    u32 options;
    u8 timer;

    for (i = 0; i < FIELD_OBJECT_COUNT; i++)
    {
        slot = &g_field_object_states[i];
        visual = &g_field_object_parts[i];
        if (g_field_actors[i].presence != FIELD_ACTOR_UNUSED)
        {
            timer = slot->tint_flash_timer;
            if (timer != 0)
            {
                if (timer & FIELD_TINT_BLINK_BIT)
                {
                    visual->red_or_track = (slot->tint_red * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                    visual->green_or_track = (slot->tint_green * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                    visual->blue_or_track = (slot->tint_blue * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                }
                else
                {
                    visual->red_or_track = slot->tint_red;
                    visual->green_or_track = slot->tint_green;
                    visual->blue_or_track = slot->tint_blue;
                }
                timer = slot->tint_flash_timer - 1;
                slot->tint_flash_timer = timer;
                if (timer == 0)
                {
                    slot->movement.word &= ~FIELD_MOVEMENT_TINT_FLASH;
                }
            }
            else
            {
                if (slot->current_hp != 0 && !(slot->contact.bytes.flags_low & 1))
                {
                    flags = slot->contact.flags;
                    if (!((flags >> 5) & 1) && !((flags >> 7) & 1))
                    {
                        if (((flags >> 6) & 1) || g_field_actor_bindings[FIELD_BINDING_INDEX(i)].owner_object_index != i ||
                            g_field_actor_bindings[FIELD_BINDING_INDEX(i)].state == 0)
                        {
                            slot->movement.word &= ~FIELD_MOVEMENT_TINT_FLASH;
                            visual->red_or_track = slot->tint_red;
                            visual->green_or_track = slot->tint_green;
                            visual->blue_or_track = slot->tint_blue;
                            continue;
                        }
                    }
                }
                options = slot->movement.word & ~FIELD_MOVEMENT_TINT_FLASH;
                slot->movement.word = options;
                if (g_field_actors[i].presence != FIELD_ACTOR_UNUSED && slot->current_hp != 0 && !(slot->contact.bytes.flags_low & 1))
                {
                    flags = slot->contact.flags;
                    if (!((flags >> 5) & 1) && !((flags >> 7) & 1) && g_field_active_group != 0)
                    {
                        slot->movement.word = options | FIELD_MOVEMENT_TINT_FLASH;
                        if (g_frame_counter & FIELD_TINT_BLINK_BIT)
                        {
                            g_field_object_parts[i].red_or_track = (slot->tint_red * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                            g_field_object_parts[i].green_or_track = (slot->tint_green * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                            g_field_object_parts[i].blue_or_track = (slot->tint_blue * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                        }
                        else
                        {
                            visual->red_or_track = slot->tint_red;
                            visual->green_or_track = slot->tint_green;
                            visual->blue_or_track = slot->tint_blue;
                        }
                    }
                }
            }
        }
    }
}
