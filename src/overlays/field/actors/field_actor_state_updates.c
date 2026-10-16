/**
 * @file field_actor_state_updates.c
 * @brief Actor command handlers (action chains, movement, landing, defeat,
 *        idle) and the object sequence interpreter with its animation actors
 *        and tint flashing.
 */
#include "common.h"
#include "common/saved_game.h"
#include "main/main.h"
#include "../internal/field_calls.h"
#include "../internal/field_held_action_buttons.h"
#include "common/vector.h"
#include "overlays/field/field_types.h"
#include "../internal/field_actor.h"
#include "../internal/field_actor_tables.h"
#include <inline_c.h>
#include "sdk/gte_dmpsx_compat.h"
#include <memory.h>
#include "../internal/field_actor_runtime.h"
#include "../internal/field_actor_sequence_runtime.h"
#include "../internal/field_player_records.h"
#include "../internal/field_resource_actions.h"
#include "../internal/field_contact_geometry.h"
#include "overlays/field/field_actor_records.h"
#include "../internal/field_actor_behavior.h"

/** @brief Actor template followed by the remaining per-player record data. */
typedef struct
{
    FieldActorSlot actor;
    u8 tail[0x24];
} FieldSequenceTemplate;

extern FieldSequenceTemplate g_field_actor_templates[];
extern FieldActorSlot g_field_shared_actor_template;
void field_restart_actor_animation(FieldActor* actor);
void field_resolve_collected_hits();

/**
 * @brief Program the chain action slot of @p actor's resource action row.
 */
#define FIELD_SET_CHAIN_ACTION(actor, action_command, action_animation, action_parameter)                                                                      \
    g_field_resource_actions[(actor)->object_index].slots[FIELD_RESOURCE_ACTION_CHAIN].flags.instrument = 0;                                                   \
    g_field_resource_actions[(actor)->object_index].slots[FIELD_RESOURCE_ACTION_CHAIN].command = action_command;                                               \
    g_field_resource_actions[(actor)->object_index].slots[FIELD_RESOURCE_ACTION_CHAIN].flags.target_filter = FIELD_ACTION_TARGET_NONE;                         \
    g_field_resource_actions[(actor)->object_index].slots[FIELD_RESOURCE_ACTION_CHAIN].animation = action_animation;                                           \
    g_field_resource_actions[(actor)->object_index].slots[FIELD_RESOURCE_ACTION_CHAIN].parameter = action_parameter;                                           \
    g_field_resource_actions[(actor)->object_index].slots[FIELD_RESOURCE_ACTION_CHAIN].flags.target_group = 0;

/** @brief Height interpolated from height to next_height across the current animation frame, in 1/256 units. */
#define FIELD_STEP_OFFSET(actor)                                                                                                                               \
    (((actor)->height + ((s8)(actor)->next_height - (actor)->height) * (actor)->frame_ticks / (actor)->frame_length) << FIELD_POSITION_SHIFT)

/** @brief Most target parameters a sequence command collects. */
#define FIELD_SEQUENCE_TARGET_LIMIT 14
/** @brief FieldActor::animation_state value while a sequence frame is playing. */
#define FIELD_SEQUENCE_FRAME_WAIT 1
/** @brief Frame counter bit that blinks a tinted object between full and dimmed colour. */
#define FIELD_TINT_BLINK_BIT 4
/** @brief A dimmed tint is the colour scaled by 100/128. */
#define FIELD_TINT_DIM_NUMERATOR 100
#define FIELD_TINT_DIM_DENOMINATOR 128

/**
 * @brief Runtime state of object @p index in @p states, addressed as index-first byte arithmetic.
 */
#define FIELD_OBJECT_STATE_AT(states, index) ((FieldObjectState*)((index) * sizeof(FieldObjectState) + (uintptr_t)(states)))

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
    FIELD_SEQUENCE_TOGGLE_DASH = 0xF2,
    FIELD_SEQUENCE_TOGGLE_CHAINING = 0xF3,
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

/** @brief Built-in animation of the dust a landing actor kicks up. */
#define FIELD_LANDING_EFFECT 0x1A
/** @brief Height an actor falls per frame while landing from a Double Jump or an air attack. */
#define FIELD_LANDING_FALL_STEP 0xC00
/** @brief Forward drift per frame of a falling air attack. */
#define FIELD_AIR_ATTACK_DRIFT 0x200
/** @brief Height above ground at which a falling air attack plays the landing effect and settles. */
#define FIELD_LANDING_SETTLE_HEIGHT 0xA
/** @brief First jump frame from which the Power Attack button starts an air Power Attack. */
#define FIELD_AIR_POWER_ATTACK_FIRST_FRAME 3
/** @brief Built-in animations started with a chained follow-up: Flip-Kick, and the two Spin follow-ups. */
#define FIELD_FLIP_KICK_EFFECT 0x25
#define FIELD_SPIN_FOLLOW_UP_50_EFFECT 0x26
#define FIELD_SPIN_FOLLOW_UP_51_EFFECT 0x27
/** @brief Battle actions of a grapple throw, and of the reversal when the grabbed object resists; gloves have their own. */
#define FIELD_GRAPPLE_THROW_ACTION 0xC
#define FIELD_GRAPPLE_GLOVE_THROW_ACTION 0xD
#define FIELD_GRAPPLE_REVERSAL_ACTION 0x17
#define FIELD_GRAPPLE_GLOVE_REVERSAL_ACTION 0x18
/** @brief Built-in animations of a grapple throw and of its reversal. */
#define FIELD_GRAPPLE_THROW_ANIMATION 0x64
#define FIELD_GRAPPLE_GLOVE_THROW_ANIMATION 0x61
#define FIELD_GRAPPLE_REVERSAL_ANIMATION 0x65
#define FIELD_GRAPPLE_GLOVE_REVERSAL_ANIMATION 0x63
/** @brief Squared distance to the leader per follower index beyond which a follower walks after it. */
#define FIELD_FOLLOW_START_DISTANCE 2000
/** @brief Squared distance to the next route point beyond which a follower runs. */
#define FIELD_FOLLOW_RUN_DISTANCE 384

/**
 * @brief History point @p point of object @p object_index, indexed through one flat point array.
 * @note The object and point indices are folded into one four-byte stride.
 */
#define FIELD_HISTORY_POINT(states, object_index, point)                                                                                                       \
    (*(FieldRoutePoint*)((u8*)(states) + ((object_index) * (sizeof(FieldObjectState) / sizeof(FieldRoutePoint)) + (point)) * sizeof(FieldRoutePoint) +         \
                         OFFSETOF(FieldObjectState, route_history)))

/**
 * @brief Settle a landing actor to the ground and play the landing effect.
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

    switch (actor->animation & FIELD_ANIMATION_INDEX_MASK)
    {
    case FIELD_ABILITY_JUMP:
        actor->animation = (actor->animation & FIELD_ANIMATION_FACING) | FIELD_ANIMATION_JUMP_LANDING;
        field_restart_actor_animation(actor);
        return 0;
    case FIELD_ABILITY_DOUBLE_JUMP:
        height = actor->y;

        if (height < -FIELD_LANDING_FALL_STEP)
        {
            actor->y = height + FIELD_LANDING_FALL_STEP;
            return 0;
        }
        else
        {
            actor->y = 0;
            actor->animation = (actor->animation & FIELD_ANIMATION_FACING) | FIELD_ANIMATION_JUMP_LANDING;
            field_restart_actor_animation(actor);
            return 0;
        }
    case FIELD_ANIMATION_AIR_ATTACK:
    case FIELD_ANIMATION_AIR_POWER_ATTACK:
        height = actor->y;

        if (height < -FIELD_LANDING_FALL_STEP)
        {
            if (actor->animation & FIELD_ANIMATION_FACING)
            {
                displacement->x = FIELD_AIR_ATTACK_DRIFT;
            }
            else
            {
                displacement->x = -FIELD_AIR_ATTACK_DRIFT;
            }

            displacement->z = 0;
            displacement->y = 0;
            rising_height = actor->y;
            field_resolve_actor_movement(actor, &displacement->x, 1);
            rising_height += FIELD_LANDING_FALL_STEP;
            actor->y = rising_height;
            return 0;
        }
        else if (height == 0)
        {
            break;
        }
        else if (height < -FIELD_LANDING_SETTLE_HEIGHT)
        {
            actor->y = -FIELD_LANDING_SETTLE_HEIGHT;
            field_play_object_animation(actor, FIELD_LANDING_EFFECT);
        }
        else
        {
            actor->y = height + 1;
        }

        return 0;
    case FIELD_ANIMATION_JUMP_LANDING:
    case FIELD_ABILITY_BACK_FLIP:
    case FIELD_ABILITY_MOONSAULT:
    case FIELD_ABILITY_HIGH_JUMP:
    case FIELD_ANIMATION_AERIAL_46:
    case FIELD_ANIMATION_AERIAL_47:
    case FIELD_ABILITY_FOLLOW_UP_4E:
    case FIELD_ABILITY_FLIP_KICK:
        field_play_object_animation(actor, FIELD_LANDING_EFFECT);
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

    if (actor_index != FIELD_ACTOR_SLOT_NONE)
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
 * @note JP expands field_get_held_action_buttons inline, and a recovering actor
 *       resolves its action command only once the resource slot matches.
 */
s32 field_update_actor_action_chain(FieldActor* actor)
{
#if defined(VERSION_JP)
    s32 animation_index;
#endif
    s32 targets;
    s32 tmp;
    s32 anim;
    s32 anim_id;
    s32 index;

    if (actor->control.word & FIELD_CONTROL_MODE_MASK)
    {
        return;
    }

    if (actor->command == FIELD_ACTOR_COMMAND_RECOVER)
    {
#if defined(VERSION_JP)
        animation_index = actor->animation & FIELD_ANIMATION_INDEX_MASK;

        if (animation_index == FIELD_ABILITY_DOUBLE_JUMP)
        {
            if (g_field_resource_actions[actor->object_index].slots[FIELD_RESOURCE_ACTION_ABILITY_1].command == animation_index &&
                (anim = field_resolve_action_command(actor, actor->object_index)) == FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_ABILITY_1))
#else
        tmp = actor->animation & FIELD_ANIMATION_INDEX_MASK;

        if (tmp == FIELD_ABILITY_DOUBLE_JUMP)
        {
            anim = field_resolve_action_command(actor, actor->object_index);

            if (g_field_resource_actions[actor->object_index].slots[FIELD_RESOURCE_ACTION_ABILITY_1].command == tmp &&
                anim == FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_ABILITY_1))
#endif
            {
                actor->command = anim;
                actor->y -= FIELD_STEP_OFFSET(actor);
                field_prepare_actor_action(actor);
                field_command_history_clear(actor->object_index);
                actor->command = FIELD_ACTOR_COMMAND_SECOND_JUMP;
                return;
            }
#if defined(VERSION_JP)
            else if ((animation_index = actor->animation & FIELD_ANIMATION_INDEX_MASK) == FIELD_ABILITY_DOUBLE_JUMP &&
                     g_field_resource_actions[actor->object_index].slots[FIELD_RESOURCE_ACTION_ABILITY_0].command == animation_index &&
                     (anim = field_resolve_action_command(actor, actor->object_index)) == FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_ABILITY_0))
#else
            else if (g_field_resource_actions[actor->object_index].slots[FIELD_RESOURCE_ACTION_ABILITY_0].command == FIELD_ABILITY_DOUBLE_JUMP &&
                     anim == FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_ABILITY_0))
#endif
            {
                actor->command = anim;
                actor->y -= FIELD_STEP_OFFSET(actor);
                field_prepare_actor_action(actor);
                field_command_history_clear(actor->object_index);
                actor->command = FIELD_ACTOR_COMMAND_SECOND_JUMP;
                return;
            }
        }
    }

    if (field_get_held_action_buttons(actor->object_index, FIELD_RESOURCE_ACTION_POWER_ATTACK, actor) != 0)
    {
        switch (actor->animation & FIELD_ANIMATION_INDEX_MASK)
        {
        case FIELD_ABILITY_LUNGE:
        case FIELD_ABILITY_DEFENSIVE_LUNGE:
            actor->command = FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_DASH_POWER_ATTACK);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case FIELD_ABILITY_TACKLE:
            actor->command = FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_TACKLE_POWER_ATTACK);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case FIELD_ABILITY_SOMERSAULT:
            actor->command = FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_TACKLE_POWER_ATTACK);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case FIELD_ABILITY_BACK_FLIP:
            FIELD_SET_CHAIN_ACTION(actor, FIELD_ABILITY_FLIP_KICK, FIELD_FLIP_KICK_EFFECT, 0);
            actor->command = FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_CHAIN);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case FIELD_ABILITY_BACK_ROLL:
            FIELD_SET_CHAIN_ACTION(actor, FIELD_ABILITY_FLIP_KICK, FIELD_FLIP_KICK_EFFECT, 0);
            actor->command = FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_CHAIN);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case FIELD_ABILITY_SPIN:
            FIELD_SET_CHAIN_ACTION(actor, FIELD_ABILITY_FOLLOW_UP_51, FIELD_SPIN_FOLLOW_UP_51_EFFECT, 0);
            actor->command = FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_CHAIN);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
            break;
        case FIELD_ABILITY_JUMP:
        case FIELD_ABILITY_MOONSAULT:
        case FIELD_ABILITY_HIGH_JUMP:
        case FIELD_ABILITY_DOUBLE_JUMP:
            if (actor->animation_frame < FIELD_AIR_POWER_ATTACK_FIRST_FRAME)
            {
                return;
            }

            actor->animation = (actor->animation & FIELD_ANIMATION_FACING) | FIELD_ANIMATION_AIR_POWER_ATTACK;
            actor->y -= FIELD_STEP_OFFSET(actor);
            field_restart_actor_animation(actor);
            field_command_history_clear(actor->object_index);
            actor->command = FIELD_ACTOR_COMMAND_AIR_FOLLOW_UP;
            actor->frame_timer = 1;
            actor->frame_ticks = 1;
            actor->frame_length = 1;
            return;
        case FIELD_ABILITY_GRAPPLE:
            if ((g_field_object_states[actor->object_index].contact.word >> FIELD_CONTACT_LINKED_BIT) & 1)
            {
                g_field_object_states[g_field_object_states[actor->object_index].linked_object_index].flags &= ~FIELD_OBJECT_FLAG_LINK_TARGET;
                actor->command = 0;
                field_restart_sequence_animation(actor);
                tmp = field_find_free_actor_slot(actor->object_index, 0);

                if (tmp != FIELD_ACTOR_SLOT_NONE)
                {
                    if (field_roll_object_evasion(actor->object_index, g_field_object_states[actor->object_index].linked_object_index) != 0)
                    {
                        if (g_field_player_records[actor->object_index].head.bytes.weapon_type == FIELD_WEAPON_GLOVE)
                        {
                            field_resolve_object_hit(actor->object_index, g_field_object_states[actor->object_index].linked_object_index,
                                                     FIELD_GRAPPLE_GLOVE_THROW_ACTION);
                        }
                        else
                        {
                            field_resolve_object_hit(actor->object_index, g_field_object_states[actor->object_index].linked_object_index,
                                                     FIELD_GRAPPLE_THROW_ACTION);
                        }

                        index = actor->object_index;
                        anim_id = FIELD_GRAPPLE_THROW_ANIMATION;

                        if (g_field_player_records[index].head.bytes.weapon_type == FIELD_WEAPON_GLOVE)
                        {
                            anim_id = FIELD_GRAPPLE_GLOVE_THROW_ANIMATION;
                        }
                    }
                    else
                    {
                        if (g_field_player_records[actor->object_index].head.bytes.weapon_type == FIELD_WEAPON_GLOVE)
                        {
                            field_resolve_object_hit(g_field_object_states[actor->object_index].linked_object_index, actor->object_index,
                                                     FIELD_GRAPPLE_GLOVE_REVERSAL_ACTION);
                        }
                        else
                        {
                            field_resolve_object_hit(g_field_object_states[actor->object_index].linked_object_index, actor->object_index,
                                                     FIELD_GRAPPLE_REVERSAL_ACTION);
                        }

                        index = actor->object_index;
                        anim_id = FIELD_GRAPPLE_REVERSAL_ANIMATION;

                        if (g_field_player_records[index].head.bytes.weapon_type == FIELD_WEAPON_GLOVE)
                        {
                            anim_id = FIELD_GRAPPLE_GLOVE_REVERSAL_ANIMATION;
                        }
                    }

                    if (field_start_builtin_animation(index, tmp, anim_id) != 0)
                    {
                        targets = g_field_object_states[actor->object_index].linked_object_index;
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
    else if (field_get_held_action_buttons(actor->object_index, FIELD_RESOURCE_ACTION_ABILITY_1, actor) != 0 ||
             field_get_held_action_buttons(actor->object_index, FIELD_RESOURCE_ACTION_ABILITY_0, actor) != 0)
    {
        tmp = field_get_held_action_buttons(actor->object_index, FIELD_RESOURCE_ACTION_ABILITY_1, actor) != 0;

        switch (actor->animation & FIELD_ANIMATION_INDEX_MASK)
        {
        case FIELD_ANIMATION_POWER_ATTACK:
            if (g_field_resource_actions[actor->object_index].slots[tmp].command == FIELD_ABILITY_JUMP ||
                g_field_resource_actions[actor->object_index].slots[tmp].command == FIELD_ABILITY_HIGH_JUMP)
            {
                actor->command = FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_JUMP_POWER_ATTACK);
                field_prepare_actor_action(actor);
                field_command_history_clear(actor->object_index);
            }

            break;
        case FIELD_ABILITY_CROUCH:
            if (g_field_resource_actions[actor->object_index].slots[tmp].command == FIELD_ABILITY_JUMP)
            {
                FIELD_SET_CHAIN_ACTION(actor, FIELD_ABILITY_HIGH_JUMP, 0, 1);
                actor->command = FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_CHAIN);
                field_prepare_actor_action(actor);
                field_command_history_clear(actor->object_index);
            }

            break;
        }
    }
    else if (field_get_held_action_buttons(actor->object_index, FIELD_RESOURCE_ACTION_COMBO, actor) != 0)
    {
        if ((actor->animation & ~FIELD_ANIMATION_FACING) == FIELD_ABILITY_SPIN)
        {
            FIELD_SET_CHAIN_ACTION(actor, FIELD_ABILITY_FOLLOW_UP_50, FIELD_SPIN_FOLLOW_UP_50_EFFECT, 0);
            actor->command = FIELD_ACTION_COMMAND(FIELD_RESOURCE_ACTION_CHAIN);
            field_prepare_actor_action(actor);
            field_command_history_clear(actor->object_index);
        }

        if ((actor->animation & ~FIELD_ANIMATION_FACING) == FIELD_ABILITY_GRAPPLE)
        {
            if ((g_field_object_states[actor->object_index].contact.word >> FIELD_CONTACT_LINKED_BIT) & 1)
            {
                g_field_object_states[g_field_object_states[actor->object_index].linked_object_index].flags &= ~FIELD_OBJECT_FLAG_LINK_TARGET;
                actor->command = 0;
                field_restart_sequence_animation(actor);
                tmp = field_find_free_actor_slot(actor->object_index, 0);

                if (tmp != FIELD_ACTOR_SLOT_NONE)
                {
                    if (field_roll_object_evasion(actor->object_index, g_field_object_states[actor->object_index].linked_object_index) != 0)
                    {
                        if (g_field_player_records[actor->object_index].head.bytes.weapon_type == FIELD_WEAPON_GLOVE)
                        {
                            field_resolve_object_hit(actor->object_index, g_field_object_states[actor->object_index].linked_object_index,
                                                     FIELD_GRAPPLE_GLOVE_THROW_ACTION);
                        }
                        else
                        {
                            field_resolve_object_hit(actor->object_index, g_field_object_states[actor->object_index].linked_object_index,
                                                     FIELD_GRAPPLE_THROW_ACTION);
                        }

                        index = actor->object_index;
                        anim_id = FIELD_GRAPPLE_THROW_ANIMATION;

                        if (g_field_player_records[index].head.bytes.weapon_type == FIELD_WEAPON_GLOVE)
                        {
                            anim_id = FIELD_GRAPPLE_GLOVE_THROW_ANIMATION;
                        }
                    }
                    else
                    {
                        if (g_field_player_records[actor->object_index].head.bytes.weapon_type == FIELD_WEAPON_GLOVE)
                        {
                            field_resolve_object_hit(g_field_object_states[actor->object_index].linked_object_index, actor->object_index,
                                                     FIELD_GRAPPLE_GLOVE_REVERSAL_ACTION);
                        }
                        else
                        {
                            field_resolve_object_hit(g_field_object_states[actor->object_index].linked_object_index, actor->object_index,
                                                     FIELD_GRAPPLE_REVERSAL_ACTION);
                        }

                        index = actor->object_index;
                        anim_id = FIELD_GRAPPLE_REVERSAL_ANIMATION;

                        if (g_field_player_records[index].head.bytes.weapon_type == FIELD_WEAPON_GLOVE)
                        {
                            anim_id = FIELD_GRAPPLE_GLOVE_REVERSAL_ANIMATION;
                        }
                    }

                    if (field_start_builtin_animation(index, tmp, anim_id) != 0)
                    {
                        targets = g_field_object_states[actor->object_index].linked_object_index;
                        field_start_actor_animation(tmp, 1, (u8*)&targets);
                    }
                }
            }

            field_command_history_clear(actor->object_index);
        }
    }
}

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
    FieldObjectState* states;
    FieldObjectState* state;

    if (actor->control.word & FIELD_CONTROL_MODE_MASK)
    {
        g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_CHAINING;
        actor->variant = actor->variant + 1;
        return 0;
    }

    selection = field_command_history_match(actor->object_index, (actor->animation >> FIELD_ANIMATION_FACING_SHIFT) ^ 1, 1);
    states = g_field_object_states;
    state = &states[actor->object_index];

    if ((state->action == FIELD_RESOURCE_ACTION_COMBO || actor->variant != 0) && actor->y == 0)
    {
        if (!(actor->control.word & FIELD_CONTROL_MODE_MASK))
        {
            flags = state->flags;

            if (!(flags & FIELD_OBJECT_FLAG_MINI))
            {
                state->flags = flags | FIELD_OBJECT_CHAINING;
            }
        }

        if (selection == FIELD_RESOURCE_ACTION_POWER_ATTACK)
        {
            g_field_object_states[actor->object_index].retry_count = 0;
            actor->variant = 0;
        }
        else if (selection <= FIELD_RESOURCE_ACTION_POWER_ATTACK)
        {
            if (selection == FIELD_RESOURCE_ACTION_COMBO)
            {
                count = actor->variant;

                if (count < FIELD_COMBO_VARIANTS)
                {
                    actor->variant = count + 1;
                }

                if (field_get_next_animation_frame_count(actor) == 0 || actor->variant >= FIELD_COMBO_VARIANTS ||
                    (actor->object_index < FIELD_PLAYER_COUNT && g_field_player_records[actor->object_index].head.bytes.weapon_type == FIELD_WEAPON_BOW &&
                     actor->variant >= FIELD_BOW_COMBO_VARIANTS))
                {
                    field_command_history_clear(actor->object_index);
                    actor->variant = 0;
                    g_field_object_states[actor->object_index].retry_count = 0;
                    g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_CHAINING;
                    field_restart_actor_animation_reverse(actor);
                    actor->command = FIELD_ACTOR_COMMAND_SEQUENCE_WAIT;
                    actor->command_param = FIELD_COMBO_END_WAIT;
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
    mode = g_field_object_states[object_index].action;

    switch (mode)
    {
    case FIELD_RESOURCE_ACTION_POWER_ATTACK:
        if (selection != 4 && selection != 6 && selection != 5 && selection != 7 && selection != FIELD_RESOURCE_ACTION_DASH_POWER_ATTACK &&
            selection != FIELD_RESOURCE_ACTION_JUMP_POWER_ATTACK && selection != FIELD_RESOURCE_ACTION_TACKLE_POWER_ATTACK)
        {
            field_command_history_clear(object_index);
            g_field_object_states[actor->object_index].retry_count = 0;
            actor->variant = 0;
            g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_CHAINING;
            field_restart_actor_animation_reverse(actor);
            actor->command = FIELD_ACTOR_COMMAND_SEQUENCE_WAIT;
            actor->command_param = FIELD_COMBO_END_WAIT;
            return 1;
        }

        break;
    case FIELD_RESOURCE_ACTION_DASH_POWER_ATTACK:
    case FIELD_RESOURCE_ACTION_JUMP_POWER_ATTACK:
    case FIELD_RESOURCE_ACTION_TACKLE_POWER_ATTACK:
        if (selection != 4 && selection != 6 && selection != 5 && selection != 7)
        {
            field_command_history_clear(object_index);
            g_field_object_states[actor->object_index].retry_count = 0;
            actor->variant = 0;
            g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_CHAINING;
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
    FieldActorSlot* anim_slot;
    FieldObjectState* state;
    FieldObjectState* states = g_field_object_states;
    u8 object_index;
    u8 actor_index;
    s32 value;

    object_index = actor->object_index;
    state = &states[object_index];
    actor_index = state->contact.bytes.animation_actor_index;

    if (actor_index == FIELD_ANIMATION_ACTOR_NONE)
    {
        if (actor->animation_state != 0)
        {
            return;
        }

        state->movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;
        actor->command = 0;
        actor->control.word &= ~FIELD_CONTROL_PLAY_ONCE;
        states[actor->object_index].action_charge = 0;

        if (actor->object_index < FIELD_PLAYER_COUNT)
        {
            field_command_history_clear(actor->object_index);
        }
    }
    else
    {
        value = actor->animation_state;
        anim_slot = &g_field_actor_slots[actor_index];

        if (value != 0 || (anim_slot->track_mask != 0 && anim_slot->owner_object_index == object_index))
        {
            return;
        }

        value = object_index;
        field_resolve_collected_hits(value, object_index);
        actor->command = 0;
        states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;

        if (anim_slot->owner_object_index == actor->object_index)
        {
            field_update_sequence_actor_binding(actor, 1);
        }

        actor->control.word &= ~FIELD_CONTROL_PLAY_ONCE;
        states[actor->object_index].action_charge = 0;

        if (actor->object_index < FIELD_PLAYER_COUNT)
        {
            field_command_history_clear(actor->object_index);
        }
    }
}

/**
 * @brief Read one byte of a party member's technique sequence.
 * @param sequence_index Script row within the member's bank.
 * @param object_index Party member whose weapon selects the sequence bank.
 * @param cursor Byte offset within the row.
 * @return The sequence byte.
 */
static inline u8 field_sequence_byte(s32 sequence_index, s32 object_index, s32 cursor)
{
    u8* scripts = g_field_actor_sequence_data;
    FieldPlayerRecord* players = g_field_player_records;
    s32 row_offset;

    row_offset = sequence_index * FIELD_SEQUENCE_ROW_SIZE + players[object_index].head.bytes.weapon_type * FIELD_SEQUENCE_BANK_SIZE;
    return *(row_offset + scripts + cursor);
}

/**
 * @brief Locate the current byte of an actor's technique sequence.
 * @param scripts Sequence resource containing the weapon banks.
 * @param players Player records whose weapons select the sequence banks.
 * @param states Object states containing the sequence cursors.
 * @param object_index Actor object whose weapon and cursor select the byte.
 * @param sequence_index Row within the selected weapon bank.
 * @return Address of the current sequence byte.
 */
static inline u8* field_current_actor_sequence_cursor(u8* scripts, FieldPlayerRecord* players, FieldObjectState* states,
                                                      s32 object_index, s32 sequence_index)
{
    s32 bank;
    s32 row_offset;
    u32 bank_offset;
    u32 offset;
    FieldObjectState* state;

    bank = players[object_index].head.bytes.weapon_type;
    row_offset = sequence_index * FIELD_SEQUENCE_ROW_SIZE;
    bank_offset = bank * FIELD_SEQUENCE_BANK_SIZE;
    offset = row_offset + bank_offset;
    state = &states[object_index];
    return scripts + offset + state->sequence_position;
}

/**
 * @brief Advance an actor sequence and apply its pending horizontal motion.
 * @param actor Actor whose sequence cursor and motion remainder are advanced.
 * @param sequence_index Script row within the actor's selected bank.
 * @note Command 0xF1 advances the cursor; 0xEF handles sequence completion.
 */
void field_update_technique_command(FieldActor* actor, s32 sequence_index)
{
    Vec3i* scratch = FIELD_SCRATCH_DISPLACEMENT;
    FieldObjectState* states;
    s32 active_cursor;
    FieldObjectPart* part;
    u8* scripts;
    FieldPlayerRecord* players;
    s32 object_index;
    s32 amount;
    u8 delay;
    u8 command;

    if (actor->animation_state == 0)
    {
        states = g_field_object_states;
        delay = states[actor->object_index].command_timer;

        if (delay != 0)
        {
            states[actor->object_index].command_timer = delay - 1;
        }

        if (delay == 0 || states[actor->object_index].command_timer == 0)
        {
            /* Unless the bound actor still plays the current frame, run the sequence. */
            if ((field_object_has_active_actor_tracks(actor->object_index) == 0) ||
                (((active_cursor = states[actor->object_index].sequence_position) != 1) &&
                 ((command = field_sequence_byte(sequence_index, actor->object_index, active_cursor)) != FIELD_SEQUENCE_END) &&
                 (command != FIELD_SEQUENCE_WAIT_ANIMATION)))
            {
                if (states[actor->object_index].sequence_position == 1)
                {
                    states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;
                    states[actor->object_index].contact.bytes.target_count = 0;
                }

                scripts = g_field_actor_sequence_data;
                players = g_field_player_records;
                object_index = actor->object_index;

                if (*field_current_actor_sequence_cursor(scripts, players, states, object_index, sequence_index) ==
                    FIELD_SEQUENCE_WAIT_ANIMATION)
                {
                    states[object_index].sequence_position++;
                }

                object_index = actor->object_index;

                if (*field_current_actor_sequence_cursor(scripts, players, states, object_index, sequence_index) ==
                    FIELD_SEQUENCE_WAIT_REPEAT)
                {
                    if (states[object_index].contact.bytes.target_count == 0)
                    {
                        field_resolve_collected_hits(object_index);
                        field_stop_actor_animations_for_object(actor, 1);
                        actor->command = 0;
                        field_update_sequence_actor_binding(actor, 1);
                        actor->control.word &= ~FIELD_CONTROL_PLAY_ONCE;

                        if (actor->object_index < FIELD_PLAYER_COUNT)
                        {
                            field_command_history_clear(actor->object_index);
                        }

                        states[actor->object_index].flags &= ~FIELD_OBJECT_FLAG_DASH;
                        states[actor->object_index].flags &= ~FIELD_OBJECT_CHAINING;
                        return;
                    }

                    states[object_index].sequence_position++;
                    return;
                }

                if (field_execute_actor_sequence(actor, sequence_index) != 0)
                {
                    field_resolve_collected_hits(actor->object_index);
                    actor->command = 0;
                    field_update_sequence_actor_binding(actor, 1);
                    actor->control.word &= ~FIELD_CONTROL_PLAY_ONCE;

                    if (actor->object_index < FIELD_PLAYER_COUNT)
                    {
                        field_command_history_clear(actor->object_index);
                    }

                    states[actor->object_index].flags &= ~FIELD_OBJECT_FLAG_DASH;
                    states[actor->object_index].flags &= ~FIELD_OBJECT_CHAINING;
                    return;
                }

                field_restart_actor_animation(actor);
                actor->control.word |= FIELD_CONTROL_PLAY_ONCE;
            }
        }
    }

    amount = actor->speed_accumulator / actor->frame_timer;
    part = &g_field_object_parts[actor->object_index];
    actor->speed_accumulator = actor->speed_accumulator - amount;

    if (actor->animation & FIELD_ANIMATION_FACING)
    {
        scratch->x = ((amount << FIELD_POSITION_SHIFT) * part->appearance.fields.scale_xz) >> FIELD_PART_SCALE_SHIFT;
    }
    else
    {
        scratch->x = (-(amount << FIELD_POSITION_SHIFT) * part->appearance.fields.scale_xz) >> FIELD_PART_SCALE_SHIFT;
    }

    scratch->z = 0;
    scratch->y = 0;
    field_resolve_actor_movement(actor, &scratch->x, 1);

    if (g_field_resource_entries[actor->resource_index].multi_image == 0)
    {
        g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_OVERLAPPING;
    }
}

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
    FieldObjectPart* part;
    Vec3i* out;
    s16 state;

    out = FIELD_SCRATCH_DISPLACEMENT;

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
        out->x = (step * direction_x * part->appearance.fields.scale_xz) >> FIELD_PART_SCALE_SHIFT;
        out->y = (vertical_step * part->scale_y) >> FIELD_PART_SCALE_SHIFT;
        out->z = (step * direction_z * part->appearance.fields.scale_xz) >> FIELD_PART_SCALE_SHIFT;

        if (field_resolve_actor_movement(actor, &out->x, 0) == 0)
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
    vector = FIELD_SCRATCH_DISPLACEMENT;
    vector->y = 0;
    vector->x = scaled;
    vector->z = z * actor->command_param;

    if (field_resolve_actor_movement(actor, &vector->x, 0) == 0)
    {
        actor->command = 0;
    }
}

/**
 * @brief Follow the leader's stored positions or restore the actor's idle behavior.
 * @param actor Follower whose movement and history index are updated.
 */
void field_follow_leader(FieldActor* actor)
{
    VECTOR* delta = (VECTOR*)SCRATCHPAD_AT(0x10);
    VECTOR* squares = (VECTOR*)SCRATCHPAD_ADDRESS;
    FieldObjectState* states;
    FieldObjectState* state;
    s32 distance;
    u8 index;
    u8 next;
    s32 facing;
    s32 advance_route;

    delta->vy = 0;
    delta->vx = (g_field_actors[0].x - actor->x) / 256;
    delta->vz = (g_field_actors[0].z - actor->z) / 256;
    gte_ldlvl(delta);
    gte_sqr0();
    gte_stlvnl(squares);
    index = actor->object_index;
    distance = squares->vx + squares->vz;

    advance_route = (index + 1) * FIELD_FOLLOW_START_DISTANCE < distance;
    if (advance_route)
    {
        states = g_field_object_states;
        state = &states[index];
        next = state->route_index;
        advance_route = next < FIELD_ROUTE_HISTORY_LENGTH - 1;
    }
    if (advance_route)
    {
        state->route_index = next + 1;
        delta->vx = (FIELD_HISTORY_POINT(states, g_field_actors[0].object_index, states[actor->object_index].route_index).x << FIELD_POSITION_SHIFT) - actor->x;
        delta->vz = (FIELD_HISTORY_POINT(states, g_field_actors[0].object_index, states[actor->object_index].route_index).z << FIELD_POSITION_SHIFT) - actor->z;
        gte_ldlvl(delta);
        gte_sqr12();
        gte_stlvnl(squares);

        if (squares->vx + squares->vz > FIELD_FOLLOW_RUN_DISTANCE && !(g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS))
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
        if (g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS)
        {
            actor->animation &= FIELD_ANIMATION_FACING;
        }
        else
        {
            facing = actor->animation & FIELD_ANIMATION_FACING;
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
    s32 owner;
    s32 object_index;
    FieldActorSlot* anim_slot;
    Vec3i* scratch;
    FieldActorSlot* anim_slots;
    FieldActorBinding* binding;

    scratch = FIELD_SCRATCH_DISPLACEMENT;

    if (actor->animation_state == 0)
    {
        binding = field_actor_binding(actor);
        object_index = actor->object_index;
        owner = binding->owner;

        if (owner == object_index)
        {
            anim_slots = g_field_actor_slots;
            anim_slot = anim_slots + field_object_binding(owner)->slot;

            if (anim_slot->active != 0)
            {
                anim_slots = g_field_actor_slots;
                anim_slot = anim_slots + field_actor_binding(actor)->slot;

                if (anim_slot->track_mask == 0)
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
        scratch->x = x * actor->command_param;
        scratch->y = y * actor->command_param;
        scratch->z = z * actor->command_param;
        field_resolve_actor_movement(actor, &scratch->x, 0);
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

    timer = actor->lift_timer - 1;
    actor->lift_timer = timer;

    if (timer == 0)
    {
        actor->command = 0;
    }

    if (rising != 0)
    {
        actor->y -= actor->command_param << FIELD_POSITION_SHIFT;
    }
    else
    {
        s32 height = actor->y + (actor->command_param << FIELD_POSITION_SHIFT);
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

/**
 * @brief Move an actor by its speed and refresh its collision contact and height.
 * @param actor Actor whose position, speed and runtime state are updated.
 * @param dx Horizontal direction or displacement multiplier.
 * @param dz Depth direction or displacement multiplier.
 * @note Positions outside the map bounds at 0x801ED400 clear the contact instead.
 */
void field_update_timed_walk(FieldActor* actor, s32 dx, s32 dz)
{
    FieldMapBounds* bounds = FIELD_MAP_BOUNDS;
    FieldCollisionMover* mover = FIELD_COLLISION_MOVER;
    s32 x, z;
    u8 speed;

    speed = actor->command_param;
    actor->x += dx * speed;
    actor->z += dz * speed;
    x = actor->x;
    z = actor->z;

    if (x >= 0 && x < (bounds->width << 8) && z >= 0 && z < ((s32)(bounds->depth << 16) >> 7))
    {
        mover->x = x;
        mover->height = actor->y;
        mover->z = actor->z;
        mover->move_x = 0;
        mover->move_height = 0;
        mover->move_z = 0;

        if (g_field_object_parts[actor->object_index].appearance.fields.scale_xz == FIELD_PART_FULL_SCALE)
        {
            mover->footprint_width = FIELD_FOOTPRINT_LARGE_WIDTH;
            mover->mode.bits.footprint_depth = FIELD_FOOTPRINT_LARGE_DEPTH;
        }
        else
        {
            mover->footprint_width = FIELD_FOOTPRINT_WIDTH;
            mover->mode.bits.footprint_depth = FIELD_FOOTPRINT_DEPTH;
        }

        mover->height_bias = FIELD_FOOTPRINT_STEP;
        mover->mode.bits.airborne_high = 0;
        mover->mode.bits.airborne_low = 0;
        mover->collision_node = g_field_object_states[actor->object_index].collision_node;
        mover->flags = g_field_object_states[actor->object_index].collision_flags;
        field_collision_resolve_move(mover);
        g_field_object_states[actor->object_index].collision_node = mover->collision_node;
        g_field_object_states[actor->object_index].collision_flags = mover->flags;
        g_field_object_states[actor->object_index].movement.half.height = mover->resolved_height / 256;
    }
    else
    {
        g_field_object_states[actor->object_index].collision_node = FIELD_COLLISION_NODE_SEARCH;
        g_field_object_states[actor->object_index].collision_flags = 0;
        g_field_object_states[actor->object_index].movement.half.height = 0;
    }
}

/**
 * @brief Start the actor's bound defeat animation and switch to FIELD_ACTOR_COMMAND_KNOCKED_DOWN.
 * @param actor Actor whose resource entry supplies the animation flags.
 * @return The busy flag or start result, or 0x8E after a successful start.
 */
s32 field_start_defeat_bound_animation(FieldActor* actor)
{
    s32 result;

    result = g_field_object_states[actor->object_index].contact.bytes.flags & 1;

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

    result = g_field_object_states[actor->object_index].contact.bytes.flags & 1;

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

/**
 * @brief Resume the actor's idle animation once its bound actor slot is free.
 * @param actor Actor whose binding selects the actor slot.
 * @return Unspecified; callers ignore it.
 */
s32 field_update_defeat_end(FieldActor* actor)
{
    FieldActorSlot* anim_slots;
    FieldActorSlot* anim_slot;

    anim_slots = g_field_actor_slots;
    anim_slot = anim_slots + field_actor_binding(actor)->slot;

    if (anim_slot->active == 0)
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

    if (g_field_object_states[actor->object_index].idle_animation == FIELD_IDLE_ANIMATION_NONE)
    {
        return;
    }

    if (g_field_object_states[actor->object_index].idle_animation == FIELD_IDLE_ANIMATION_REWARD)
    {
        for (i = 0; i < FIELD_REWARD_KIND_COUNT; i++)
        {
            if (g_field_object_states[actor->object_index].reward_counters[i] != 0)
            {
                animation_id = g_field_object_states[actor->object_index].idle_animation;
                actor_index = field_find_free_actor_slot(actor->object_index, 0);

                if (actor_index != FIELD_ACTOR_SLOT_NONE)
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

    animation_id = g_field_object_states[actor->object_index].idle_animation;
    actor_index = field_find_free_actor_slot(actor->object_index, 0);

    if (actor_index != FIELD_ACTOR_SLOT_NONE)
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
 * @return Unspecified; no caller reads it.
 */
s32 field_update_defeated(FieldActor* actor)
{
    if (g_field_actor_slots[actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE].active == 0)
    {
        actor->command = 0;
        actor->presence = FIELD_ACTOR_UNUSED;
        field_restart_idle_animation(actor);
    }
}

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
    FieldObjectPart* part;
    Vec3i* out;

    out = FIELD_SCRATCH_DISPLACEMENT;

    if (actor->animation_state == 0)
    {
        actor->command = 0;
        return;
    }

    step = actor->speed_accumulator / actor->frame_timer;
    actor->speed_accumulator = actor->speed_accumulator - step;
    part = &g_field_object_parts[actor->object_index];
    out->x = (step * direction_x * part->appearance.fields.scale_xz) >> FIELD_PART_SCALE_SHIFT;
    out->y = (vertical_step * part->scale_y) >> FIELD_PART_SCALE_SHIFT;
    out->z = (step * direction_z * part->appearance.fields.scale_xz) >> FIELD_PART_SCALE_SHIFT;
    field_resolve_actor_movement(actor, &out->x, 0);
}

/**
 * @brief Mark an owned animation as running, or release its completed binding.
 * @param actor Actor selecting the binding, with indices above one sharing the third entry.
 * @param release_actor Clear the completed actor's active flag before releasing its binding.
 */
void field_update_sequence_actor_binding(FieldActor* actor, s32 release_actor)
{
    FieldActorBinding* base;
    s32 object_index;
    s32 active_owner;
    s32 finished_owner;
    s32 binding_offset;
    s32 animation_binding_offset;
    s32 owner_binding_offset;
    s32 active_binding_offset;
    s32 finished_binding_offset;
    s32 release_binding_offset;
    FieldActorSlot* active_slots;
    FieldActorBinding* active_lookup;
    u8* active_actor_address;
    FieldActorSlot* release_slots;
    FieldActorBinding* release_lookup;
    u8* release_actor_address;

    base = g_field_actor_bindings;

    if (actor->object_index < FIELD_PLAYER_COUNT)
    {
        binding_offset = (actor->object_index) * sizeof(*base);
    }
    else
    {
        binding_offset = (FIELD_SHARED_BINDING) * sizeof(*base);
    }

    if (((FieldActorBinding*)((u8*)base + binding_offset))->state != 0)
    {
        base = g_field_actor_bindings;

        if (actor->object_index < FIELD_PLAYER_COUNT)
        {
            animation_binding_offset = (actor->object_index) * sizeof(*base);
        }
        else
        {
            animation_binding_offset = (FIELD_SHARED_BINDING) * sizeof(*base);
        }

        if (field_is_actor_animation_active(((FieldActorBinding*)((u8*)base + animation_binding_offset))->slot) != 0)
        {
            base = g_field_actor_bindings;

            if (actor->object_index < FIELD_PLAYER_COUNT)
            {
                owner_binding_offset = (actor->object_index) * sizeof(*base);
            }
            else
            {
                owner_binding_offset = (FIELD_SHARED_BINDING) * sizeof(*base);
            }

            if (((FieldActorBinding*)((u8*)base + owner_binding_offset))->owner == actor->object_index)
            {
                active_owner = ((FieldActorBinding*)((u8*)base + owner_binding_offset))->owner;
                active_slots = g_field_actor_slots;
                active_lookup = g_field_actor_bindings;

                if ((u8)active_owner < FIELD_PLAYER_COUNT)
                {
                    active_binding_offset = (active_owner) * sizeof(*base);
                }
                else
                {
                    active_binding_offset = (FIELD_SHARED_BINDING) * sizeof(*base);
                }

                active_actor_address = (u8*)active_slots;
                active_actor_address += ((FieldActorBinding*)((u8*)active_lookup + active_binding_offset))->slot * sizeof(*active_slots);
                ((FieldActorSlot*)active_actor_address)->sequence_active = 1;
            }
        }
        else
        {
            base = g_field_actor_bindings;

            if (actor->object_index < FIELD_PLAYER_COUNT)
            {
                finished_binding_offset = (actor->object_index) * sizeof(*base);
            }
            else
            {
                finished_binding_offset = (FIELD_SHARED_BINDING) * sizeof(*base);
            }

            object_index = actor->object_index;
            finished_owner = ((FieldActorBinding*)((u8*)base + finished_binding_offset))->owner;

            if (finished_owner == object_index)
            {
                if (release_actor != 0)
                {
                    release_slots = g_field_actor_slots;
                    release_lookup = g_field_actor_bindings;

                    if ((u8)finished_owner < FIELD_PLAYER_COUNT)
                    {
                        release_binding_offset = (finished_owner) * sizeof(*base);
                    }
                    else
                    {
                        release_binding_offset = (FIELD_SHARED_BINDING) * sizeof(*base);
                    }

                    release_actor_address = (u8*)release_slots;
                    release_actor_address += ((FieldActorBinding*)((u8*)release_lookup + release_binding_offset))->slot * sizeof(*release_slots);
                    ((FieldActorSlot*)release_actor_address)->active = 0;
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
    FieldObjectState* slots;
    FieldPlayerRecord* players;
    u8* programs;
    u8* initial_program;
    u8* initial_program_base;
    FieldActorBinding* bindings;
    FieldActorBinding* binding;
    FieldActorSlot* anim_slots;
    s32 parameters[FIELD_SEQUENCE_TARGET_LIMIT];
    FieldActorSlot* copy_source;
    FieldActorSlot* template_actor;
    s32 actor_index;
    s32 script_offset;
    s32 bank_offset;
    s32 pending_command;
    s32 target_index;
    s32 current_target_index;
    s32 animation_command;
    uintptr_t delay_operand;
    uintptr_t resource_operand;
    uintptr_t animation_operand;
    s32 clear_slot;
    s32 movement_mask;
    s32 cursor;
    s32 result;
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
    FieldObjectState* target_state;
    FieldObjectState* allocation_state;
    FieldObjectState* animation_state;
    FieldObjectState* pending_state;
    FieldObjectState* current_target_state;
    FieldObjectState* current_animation_state;
    FieldObjectState* current_allocation_state;
    FieldActorSlot* pending_actor;
    FieldActorSlot* pending_actors;
    FieldObjectState* flag_state;
    FieldObjectState* source_state;
    s32 target_kind_flags;
    s32 target_command;
    s32 actor_offset;
    s32 current_copy_owner;
    s32* current_target_output;
    s32 kind_flags;
    s32 sequence_command;
    s32 allocation_kind_flags;
    s32 allocation_command;
    u8* frame_programs;
    FieldPlayerRecord* frame_players;
    FieldObjectState* frame_slots;
    u8* frame;

    g_field_object_states[actor->object_index].command_timer = 0;
    pending_owner = actor->object_index;
    pending_state = &g_field_object_states[pending_owner];
    pending_command = pending_state->action_parameter;

    if (pending_command != FIELD_ACTION_PARAMETER_NONE)
    {
        if (pending_command & FIELD_ACTION_PARAMETER_TRANSIENT_ACTOR)
        {
            if (pending_state->contact.bytes.animation_actor_index < FIELD_ACTOR_SLOT_TOTAL)
            {
                pending_actors = g_field_actor_slots;
                pending_actor = &pending_actors[pending_state->contact.bytes.animation_actor_index];

                if ((pending_actor->active != 0) && (pending_actor->owner_object_index == pending_owner))
                {
                    pending_actor->active = 0;
                }
            }

            g_field_object_states[actor->object_index].action_parameter = FIELD_ACTION_PARAMETER_NONE;
        }
    }

    initial_program_base = g_field_actor_sequence_data;
    initial_owner = actor->object_index;
    cursor = g_field_object_states[initial_owner].sequence_position;
    initial_program = (script_index * FIELD_SEQUENCE_ROW_SIZE) + (g_field_player_records[initial_owner].head.bytes.weapon_type * FIELD_SEQUENCE_BANK_SIZE) +
                      initial_program_base + cursor;
    if (*initial_program == FIELD_SEQUENCE_END)
    {
        return 1;
    }

    if (cursor == 1)
    {
        binding = field_object_binding(initial_owner);
        result = 0;

        if (binding->state != FIELD_BINDING_READY)
        {
            return 0;
        }

        g_field_actor_slots[g_field_actor_bindings[actor->object_index].slot].sequence_active = 0;
        g_field_actor_slots[g_field_actor_bindings[actor->object_index].slot].target_count = 0;
        copy_source = &g_field_actor_slots[field_actor_binding(actor)->slot];

        if (actor->object_index < FIELD_PLAYER_COUNT)
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

        if (actor->object_index < FIELD_PLAYER_COUNT)
        {
            release_binding_offset = actor->object_index * sizeof(*bindings);
        }
        else
        {
            release_binding_offset = FIELD_SHARED_BINDING * sizeof(*bindings);
        }

        actor_offset = ((FieldActorBinding*)((u8*)bindings + release_binding_offset))->slot * sizeof(*anim_slots);
        ((FieldActorSlot*)((u8*)anim_slots + actor_offset))->active = 0;
    }

    programs = g_field_actor_sequence_data;
    players = g_field_player_records;
    slots = g_field_object_states;
    script_offset = script_index * FIELD_SEQUENCE_ROW_SIZE;
    command_slot = actor->object_index;
    opcode_ptr = script_offset + players[command_slot].head.bytes.weapon_type * FIELD_SEQUENCE_BANK_SIZE + programs + cursor;
    opcode = *opcode_ptr;
    result = 0;

    /* Frame bytes stop dispatch; command bytes may consume additional operands. */
    for (; opcode >= FIELD_SEQUENCE_START_TARGETS_0; command_slot = actor->object_index,
                                                     bank_offset = script_offset + players[command_slot].head.bytes.weapon_type * FIELD_SEQUENCE_BANK_SIZE,
                                                     opcode_ptr = (u8*)(bank_offset + (uintptr_t)programs + cursor), opcode = *opcode_ptr)
    {
        switch (opcode)
        {
        case FIELD_SEQUENCE_END:
            g_field_object_states[command_slot].sequence_position = cursor;
            actor->animation_state = FIELD_SEQUENCE_FRAME_WAIT;
            actor->animation_frame = 0;
            actor->animation_active = 1;
            actor->animation &= FIELD_ANIMATION_FACING;
            return 0;
        default:
            command = *opcode_ptr;

            switch (command)
            {
            case FIELD_SEQUENCE_START_TARGETS_0:
            case FIELD_SEQUENCE_START_TARGETS_1:
            case FIELD_SEQUENCE_START_TARGETS_2:
                source_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                target_kind_flags =
                    ((command - FIELD_SEQUENCE_START_TARGETS_0) << FIELD_ACTION_PARAMETER_ANIMATION_SHIFT) | FIELD_ACTION_PARAMETER_TRANSIENT_ACTOR;
                target_command = source_state->sequence_id & FIELD_ACTION_PARAMETER_SEQUENCE_MASK;
                target_state = source_state;
                target_command |= target_kind_flags;
                target_command |= FIELD_ACTION_PARAMETER_SELECT_ANIMATION;
                target_state->action_parameter = target_command;
                target_owner = actor->object_index;
                actor_index = field_allocate_sequence_actor(target_owner, slots[target_owner].action_parameter);

                for (target_index = 0; target_index < slots[actor->object_index].contact.bytes.target_count; target_index++)
                {
                    parameters[target_index] = slots[actor->object_index].targets[target_index];
                }

                field_start_actor_animation(actor_index, slots[actor->object_index].contact.bytes.target_count, (u8*)parameters);
                cursor += 1;
                slots[actor->object_index].action_parameter = FIELD_ACTION_PARAMETER_NONE;
                continue;
            case FIELD_SEQUENCE_START_CURRENT_TARGETS:
                current_target_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                current_target_state->action_parameter = current_target_state->sequence_id;
                current_target_owner = actor->object_index;
                actor_index = field_allocate_sequence_actor(current_target_owner, slots[current_target_owner].sequence_id);
                slots[actor->object_index].action_parameter = FIELD_ACTION_PARAMETER_NONE;
                current_copy_owner = actor->object_index;
                current_target_index = 0;

                if (slots[current_copy_owner].contact.bytes.target_count != 0)
                {
                    current_target_output = parameters;

                    do
                    {
                        *current_target_output = slots[current_copy_owner].targets[current_target_index];
                        current_target_index++;
                        current_target_output++;
                        current_copy_owner = actor->object_index;
                    } while (current_target_index < slots[current_copy_owner].contact.bytes.target_count);
                }

                field_start_actor_animation(actor_index, slots[actor->object_index].contact.bytes.target_count, (u8*)parameters);
                cursor += 1;
                continue;
            case FIELD_SEQUENCE_DELAY:
                result = 0;
                delay_owner = actor->object_index;
                pending_state = (FieldObjectState*)(delay_owner * sizeof(*slots));
                delay_operand = script_offset + players[delay_owner].head.bytes.weapon_type * FIELD_SEQUENCE_BANK_SIZE;
                delay_operand += (uintptr_t)programs;
                delay_operand += cursor;
                pending_state = (FieldObjectState*)((uintptr_t)pending_state + (uintptr_t)slots);
                pending_state->command_timer = ((u8*)delay_operand)[1];
                cursor += 2;
                slots[actor->object_index].sequence_position = cursor;
                return result;
            case FIELD_SEQUENCE_WAIT_REPEAT:
            case FIELD_SEQUENCE_WAIT_ANIMATION:
                slots[actor->object_index].sequence_position = cursor;
                return 0;
            case FIELD_SEQUENCE_TOGGLE_DASH:
                flag_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                updated_flags = flag_state->flags ^ FIELD_OBJECT_FLAG_DASH;
                flag_state->flags = updated_flags;
                cursor += 1;
                field_update_object_effects(actor->object_index);
                continue;
            case FIELD_SEQUENCE_TOGGLE_CHAINING:
                flag_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                updated_flags = flag_state->flags ^ FIELD_OBJECT_CHAINING;
                flag_state->flags = updated_flags;
                cursor += 1;
                field_update_object_effects(actor->object_index);
                continue;
            case FIELD_SEQUENCE_TOGGLE_FACING:
                cursor += 1;
                actor->animation ^= FIELD_ANIMATION_FACING;
                continue;
            case FIELD_SEQUENCE_START_RESOURCE:
                actor_index = field_find_free_actor_slot(actor->object_index, 0);

                if (actor_index != FIELD_ACTOR_SLOT_NONE)
                {
                    resource_owner = actor->object_index;
                    resource_operand = script_offset + players[resource_owner].head.bytes.weapon_type * FIELD_SEQUENCE_BANK_SIZE;
                    resource_operand += (uintptr_t)programs;
                    resource_operand += cursor;
                    field_start_builtin_animation(resource_owner, actor_index, ((u8*)resource_operand)[1]);
                    field_start_actor_animation(actor_index, 0, NULL);
                }

                cursor += 2;
                clear_slot = actor->object_index;
                movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                slots[clear_slot].movement.word &= movement_mask;

                break;
            case FIELD_SEQUENCE_START_0:
            case FIELD_SEQUENCE_START_1:
            case FIELD_SEQUENCE_START_2:
                animation_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                kind_flags = ((command - FIELD_SEQUENCE_START_0) << FIELD_ACTION_PARAMETER_ANIMATION_SHIFT) | FIELD_ACTION_PARAMETER_TRANSIENT_ACTOR;
                sequence_command = animation_state->sequence_id & FIELD_ACTION_PARAMETER_SEQUENCE_MASK;
                sequence_command |= kind_flags;
                sequence_command |= FIELD_ACTION_PARAMETER_SELECT_ANIMATION;
                animation_state->action_parameter = sequence_command;

                animation_owner = actor->object_index;
                field_start_actor_animation(field_allocate_sequence_actor(animation_owner, slots[animation_owner].action_parameter), 0, NULL);
                slots[actor->object_index].action_parameter = FIELD_ACTION_PARAMETER_NONE;
                clear_slot = actor->object_index;
                cursor += 1;
                movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                slots[clear_slot].movement.word &= movement_mask;

                break;
            case FIELD_SEQUENCE_START_CURRENT:
                current_animation_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                current_animation_state->action_parameter = current_animation_state->sequence_id;
                allocation_owner = actor->object_index;
                actor_index = field_allocate_sequence_actor(allocation_owner, slots[allocation_owner].sequence_id);
                slots[actor->object_index].action_parameter = FIELD_ACTION_PARAMETER_NONE;
                field_start_actor_animation(actor_index, 0, NULL);
                clear_slot = actor->object_index;
                cursor += 1;
                movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                slots[clear_slot].movement.word &= movement_mask;

                break;
            case FIELD_SEQUENCE_SET_ANIMATION:
                command_owner = actor->object_index;
                target_state = (FieldObjectState*)(command_owner * sizeof(*slots));
                animation_operand = script_offset + players[command_owner].head.bytes.weapon_type * FIELD_SEQUENCE_BANK_SIZE;
                animation_operand += (uintptr_t)programs;
                animation_operand += cursor;
                target_state = (FieldObjectState*)((uintptr_t)target_state + (uintptr_t)slots);
                target_state->action_parameter = ((u8*)animation_operand)[1];
                cursor += 2;
                clear_slot = actor->object_index;
                movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                slots[clear_slot].movement.word &= movement_mask;

                break;
            case FIELD_SEQUENCE_ALLOCATE_0:
            case FIELD_SEQUENCE_ALLOCATE_1:
            case FIELD_SEQUENCE_ALLOCATE_2:
                allocation_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                allocation_kind_flags =
                    ((command - FIELD_SEQUENCE_ALLOCATE_0) << FIELD_ACTION_PARAMETER_ANIMATION_SHIFT) | FIELD_ACTION_PARAMETER_TRANSIENT_ACTOR;
                allocation_command = allocation_state->sequence_id & FIELD_ACTION_PARAMETER_SEQUENCE_MASK;
                allocation_command |= allocation_kind_flags;
                allocation_command |= FIELD_ACTION_PARAMETER_SELECT_ANIMATION;
                allocation_state->action_parameter = allocation_command;

                allocation_owner = actor->object_index;
                animation_command = slots[allocation_owner].action_parameter;
                field_allocate_sequence_actor(allocation_owner, animation_command);
                clear_slot = actor->object_index;
                cursor += 1;
                movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                slots[clear_slot].movement.word &= movement_mask;

                break;
            case FIELD_SEQUENCE_ALLOCATE_CURRENT:
                current_allocation_state = FIELD_OBJECT_STATE_AT(slots, actor->object_index);
                current_allocation_state->action_parameter = current_allocation_state->sequence_id;
                allocation_owner = actor->object_index;
                animation_command = slots[allocation_owner].sequence_id;
                field_allocate_sequence_actor(allocation_owner, animation_command);
                clear_slot = actor->object_index;
                cursor += 1;
                movement_mask = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                slots[clear_slot].movement.word &= movement_mask;

                break;
            default:
                continue;
            }
        }
    }

    frame_programs = g_field_actor_sequence_data;
    frame_players = g_field_player_records;
    frame = (script_index * FIELD_SEQUENCE_ROW_SIZE) + frame_players[actor->object_index].head.bytes.weapon_type * FIELD_SEQUENCE_BANK_SIZE + frame_programs +
            cursor;
    cursor++;
    frame_slots = g_field_object_states;
    actor->animation = *frame + (actor->animation & FIELD_ANIMATION_FACING);
    frame_slots[actor->object_index].sequence_position = cursor;
    actor->animation_state = FIELD_SEQUENCE_FRAME_WAIT;
    actor->animation_frame = 0;
    actor->animation_active = 1;
    return 0;
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
    FieldActorSlot *actor, *updated, *slots;
    FieldActorBinding* bindings;
    slot = field_find_free_actor_slot(index, 0);

    if (slot != FIELD_ACTOR_SLOT_NONE)
    {
        actor = &g_field_actor_slots[slot];
        bcopy((const u8*)&g_field_actor_templates[index], (u8*)actor, sizeof(*actor));
        actor->active = 1;
        actor->slot_index = slot;

        if (flags & FIELD_ACTION_PARAMETER_SELECT_ANIMATION)
        {
            actor->animation_index = (flags >> FIELD_ACTION_PARAMETER_ANIMATION_SHIFT) & FIELD_ACTION_PARAMETER_ANIMATION_MASK;
            actor->duration = actor->animations[actor->animation_index].duration;
        }
        else
        {
            actor->animation_index = 0;
            actor->duration = actor->animations->duration;
        }

        slots = g_field_actor_slots;
        updated = &slots[slot];
        updated->track_interval = updated->animations[updated->animation_index].track_interval;
        updated->animation = &updated->animations[updated->animation_index];
        g_field_object_states[index].contact.bytes.animation_actor_index = slot;
        bindings = g_field_actor_bindings;
        binding = index;

        if (binding >= FIELD_ACTOR_BINDING_COUNT)
        {
            binding = FIELD_SHARED_BINDING;
        }

        bindings[binding].slot = slot;
    }
    else
    {
        g_field_object_states[index].contact.bytes.animation_actor_index = FIELD_ANIMATION_ACTOR_NONE;
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
    FieldObjectPart* visual;
    FieldObjectState* slot;
    u32 flags;
    u32 options;
    u8 timer;

    for (i = 0; i < FIELD_OBJECT_COUNT; i++)
    {
        slot = &g_field_object_states[i];
        visual = &g_field_object_parts[i];

        if (g_field_actors[i].presence != FIELD_ACTOR_UNUSED)
        {
            timer = slot->tint_timer;

            if (timer != 0)
            {
                if (timer & FIELD_TINT_BLINK_BIT)
                {
                    visual->tint_red = (slot->tint_red * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                    visual->tint_green = (slot->tint_green * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                    visual->tint_blue = (slot->tint_blue * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                }
                else
                {
                    visual->tint_red = slot->tint_red;
                    visual->tint_green = slot->tint_green;
                    visual->tint_blue = slot->tint_blue;
                }

                timer = slot->tint_timer - 1;
                slot->tint_timer = timer;

                if (timer == 0)
                {
                    slot->movement.word &= ~FIELD_MOVEMENT_INVULNERABLE;
                }
            }
            else
            {
                if (slot->current_hp.word != 0 && !(slot->contact.bytes.flags & FIELD_CONTACT_ANIMATION_HIDDEN))
                {
                    flags = slot->contact.word;

                    if (!((flags >> FIELD_CONTACT_NO_HIT_TEST_BIT) & 1) && !((flags >> FIELD_CONTACT_TARGETED_BIT) & 1))
                    {
                        if (((flags >> FIELD_CONTACT_ACTION_PENDING_BIT) & 1) || g_field_actor_bindings[FIELD_BINDING_INDEX(i)].owner != i ||
                            g_field_actor_bindings[FIELD_BINDING_INDEX(i)].state == FIELD_BINDING_IDLE)
                        {
                            slot->movement.word &= ~FIELD_MOVEMENT_INVULNERABLE;
                            visual->tint_red = slot->tint_red;
                            visual->tint_green = slot->tint_green;
                            visual->tint_blue = slot->tint_blue;
                            continue;
                        }
                    }
                }

                options = slot->movement.word & ~FIELD_MOVEMENT_INVULNERABLE;
                slot->movement.word = options;

                if (g_field_actors[i].presence != FIELD_ACTOR_UNUSED && slot->current_hp.word != 0 &&
                    !(slot->contact.bytes.flags & FIELD_CONTACT_ANIMATION_HIDDEN))
                {
                    flags = slot->contact.word;

                    if (!((flags >> FIELD_CONTACT_NO_HIT_TEST_BIT) & 1) && !((flags >> FIELD_CONTACT_TARGETED_BIT) & 1) && g_field_active_group != 0)
                    {
                        slot->movement.word = options | FIELD_MOVEMENT_INVULNERABLE;

                        if (g_frame_counter & FIELD_TINT_BLINK_BIT)
                        {
                            g_field_object_parts[i].tint_red = (slot->tint_red * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                            g_field_object_parts[i].tint_green = (slot->tint_green * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                            g_field_object_parts[i].tint_blue = (slot->tint_blue * FIELD_TINT_DIM_NUMERATOR) / FIELD_TINT_DIM_DENOMINATOR;
                        }
                        else
                        {
                            visual->tint_red = slot->tint_red;
                            visual->tint_green = slot->tint_green;
                            visual->tint_blue = slot->tint_blue;
                        }
                    }
                }
            }
        }
    }
}
