/**
 * @file field_actor_state_updates.c
 * @brief Actor movement/trigger states, pending actions, displacement and follow
 *        movement, resource-state waits, animation resume, and the object
 *        sequence interpreter with its animation actors and tint flashing.
 *
 * One translation unit: the jump tables of func_80092C98 (0x80050F14) and
 * field_execute_actor_sequence (0x8005100C) share one object's .rodata; the
 * zero word at 0x80051008 between them is the compiler's 8-byte alignment
 * before the second table.
 */
#include "common.h"
#include "vector.h"
#include "field_types.h"
#include "sdk/inline_c.h"
/* Apply the matching GTE instruction encodings after the SDK macros. */
#include "sdk/gte_dmpsx_compat.h"
#include "sdk/memory.h"
#include "field_actor_runtime.h"
#include "field_actor_sequence_runtime.h"
#include "field_contact_geometry.h"

/** @brief Scratchpad vector that receives an actor displacement. */
#define FIELD_SCRATCH_DISPLACEMENT ((Vec3i*)0x1F800000)

/** @brief Binding index of an object: objects 0 and 1 own theirs, all others share the third. */
#define FIELD_BINDING_INDEX(object_index) ((object_index) < 3 ? (object_index) : 2)

/**
 * @brief Object-runtime bytes that FieldObjectRuntime does not name yet.
 * @note Each reads the matching padding byte of the shared record.
 */
#define FIELD_OBJECT_IDLE_FLAGS(state) ((state)->pad_0x60)
#define FIELD_OBJECT_IDLE_ANIMATION(state) ((state)->pad_0x168[4])
#define FIELD_OBJECT_ACTION_MODE(state) ((state)->pad_0x16f[0])
#define FIELD_OBJECT_LINKED_OBJECT(state) ((state)->pad_0x16f[1])
#define FIELD_OBJECT_PENDING_STEP(state) ((state)->targets[13])

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

/** @brief Command track of a resource action record. */
typedef struct
{
    u16 command;
    u8 pad2[6];
} FieldResourceActionTrack;

/** @brief Resource action record with its command tracks and animation request. */
typedef struct
{
    FieldResourceActionTrack tracks[2];
    u8 pad10[0x58 - 0x10];
    u16 animation;
    /** @brief Request flags; the low byte is also written on its own. */
    union
    {
        u16 word;
        struct
        {
            u8 low;
            u8 high;
        } bytes;
    } request;
    u16 animation_arg;
    u16 animation_mode;
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
extern FieldMotionRecord g_field_actors[];
extern FieldResourceAction g_field_resource_actions[];
extern FieldResourceEntry g_field_resource_entries[];
extern s32 g_field_active_group;
extern s32 g_frame_counter;

s32 func_800839F8(s32 owner_index, s32 require_idle_binding);
s32 func_80083EEC(s32 owner_index, s32 actor_index, s32 resource_index);
void func_80084424(s32 owner_index);
void func_80086494(s32 object_index);
void field_restart_actor_animation(FieldMotionRecord* object);
void func_8008A9D8(s32 arg0, s32 arg1, s32 arg2);
s32 func_8008AABC(s32 a, s32 b);
void func_8008BC5C(FieldMotionRecord* object);
void field_prepare_actor_action(FieldMotionRecord* object);
s32 func_80091728(u8 index, s32 kind, FieldMotionRecord* object);
s32 func_80091914(FieldMotionRecord* object, u8 index);
void func_800A2DD8();
void func_8008A678();
s32 func_800A29F8(s32 object_index, s32 facing, s32 mode);
s32 field_object_has_active_actor_tracks(u8 object_index);
void field_stop_actor_animations_for_object(FieldMotionRecord* object, s32 force);
void field_restart_actor_animation_reverse(FieldMotionRecord* object);
s32 field_get_next_animation_frame_count(FieldMotionRecord* object);
void field_update_actor_movement_animation();
void func_80095074(FieldMotionRecord* object);

/** @brief Program the resource-action animation request of object @p index. */
#define FIELD_SET_ACTION_ANIMATION(index, animation_id, arg, mode)                                                                                             \
    g_field_resource_actions[index].request.word &= 0xFBFF;                                                                                                    \
    g_field_resource_actions[index].animation = animation_id;                                                                                                  \
    g_field_resource_actions[index].request.bytes.low = 0xFF;                                                                                                  \
    g_field_resource_actions[index].animation_arg = arg;                                                                                                       \
    g_field_resource_actions[index].animation_mode = mode;                                                                                                     \
    g_field_resource_actions[index].request.word &= 0xFCFF;

/** @brief Interpolated step height between the start and end offsets, in 1/256 units. */
#define FIELD_STEP_OFFSET(object)                                                                                                                              \
    (((s8)(object)->vertical_offset + ((s8)(object)->unknown_0x38 - (s8)(object)->vertical_offset) * (object)->unknown_0x34 / (object)->unknown_0x35) << 8)

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
#define FIELD_SEQUENCE_MOVEMENT_MASK 0x1800
#define FIELD_SEQUENCE_FACING 0x80
#define FIELD_OBJECT_TINT_FLASH 0x8000
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

void func_80092C24(FieldMotionRecord* object, s32 animation_id);

/**
 * @brief Advance selected FIELD movement states and request animation 0x1A.
 * @param object Object whose height and state byte are advanced.
 * @return Zero while the state is being advanced, or one when it is complete.
 * @note The high state bit selects horizontal displacement direction.
 * @see decomp.me WIP
 */
s32 func_80092AD8(FieldMotionRecord* object)
{
    s32 height;
    s32 rising_height;
    Vec3i* displacement;

    displacement = FIELD_SCRATCH_DISPLACEMENT;
    switch (object->facing_or_reward_kind & 0x7F)
    {
    case 8:
        object->facing_or_reward_kind = (object->facing_or_reward_kind & 0x80) | 9;
        field_restart_actor_animation(object);
        return 0;
    case 61:
        height = object->y;
        if (height < -0xC00)
        {
            object->y = height + 0xC00;
            return 0;
        }
        else
        {
            object->y = 0;
            object->facing_or_reward_kind = (object->facing_or_reward_kind & 0x80) | 9;
            field_restart_actor_animation(object);
            return 0;
        }
    case 72:
    case 73:
        height = object->y;
        if (height < -0xC00)
        {
            if (object->facing_or_reward_kind & 0x80)
            {
                displacement->x = 0x200;
            }
            else
            {
                displacement->x = -0x200;
            }
            displacement->z = 0;
            displacement->y = 0;
            rising_height = object->y;
            field_resolve_actor_movement(object, &displacement->x, 1);
            rising_height += 0xC00;
            object->y = rising_height;
            return 0;
        }
        else if (height == 0)
        {
            break;
        }
        else if (height < -0xA)
        {
            object->y = -0xA;
            func_80092C24(object, 0x1A);
        }
        else
        {
            object->y = height + 1;
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
        func_80092C24(object, 0x1A);
        return 1;
    default:
        break;
    }
    return 1;
}

/**
 * @brief Start an animation on the object's actor slot when the slot resolves and accepts it.
 * @param object Object whose index selects the actor slot.
 * @param animation_id Animation resource passed to func_80083EEC.
 */
void func_80092C24(FieldMotionRecord* object, s32 animation_id)
{
    s32 actor_index = func_800839F8(object->source_object_index, 0);

    if (actor_index != -1)
    {
        if (func_80083EEC(object->source_object_index, actor_index, animation_id))
        {
            field_start_actor_animation(actor_index, 0, 0);
        }
    }
}

/**
 * @brief Per-frame state handler for a field actor's opcode 0x86 / trigger-kind states.
 *
 * With no pending flags, first resolves the 0x3D transition when the current
 * animation matches, then dispatches on the state byte by trigger kind
 * (func_80091728 kinds 3, 1/0, 2), programming the resource-action animation
 * request and queueing the follow-up state via field_prepare_actor_action.
 *
 * @param object Field actor record.
 * @return Never set; callers ignore it.
 * @see decomp.me (100%) TODO
 */
s32 func_80092C98(FieldMotionRecord* object)
{
    s32 targets;
    s32 tmp;
    s32 anim;
    s32 anim_id;
    s32 index;

    if (object->flags & 0x1FF)
    {
        return;
    }
    if (object->motion_parameter == 0x86)
    {
        tmp = object->facing_or_reward_kind & 0x7F;
        if (tmp == 0x3D)
        {
            anim = func_80091914(object, object->source_object_index);
            if (g_field_resource_actions[object->source_object_index].tracks[1].command == tmp && anim == 0x185)
            {
                object->motion_parameter = anim;
                object->y -= FIELD_STEP_OFFSET(object);
                field_prepare_actor_action(object);
                func_800A2DD8(object->source_object_index);
                object->motion_parameter = 0x9B;
                return;
            }
            else if (g_field_resource_actions[object->source_object_index].tracks[0].command == 0x3D && anim == 0x85)
            {
                object->motion_parameter = anim;
                object->y -= FIELD_STEP_OFFSET(object);
                field_prepare_actor_action(object);
                func_800A2DD8(object->source_object_index);
                object->motion_parameter = 0x9B;
                return;
            }
        }
    }
    if (func_80091728(object->source_object_index, 3, object) != 0)
    {
        switch (object->facing_or_reward_kind & 0x7F)
        {
        case 0x2F:
        case 0x44:
            object->motion_parameter = 0x885;
            field_prepare_actor_action(object);
            func_800A2DD8(object->source_object_index);
            break;
        case 0x3E:
            object->motion_parameter = 0xA85;
            field_prepare_actor_action(object);
            func_800A2DD8(object->source_object_index);
            break;
        case 0x38:
            object->motion_parameter = 0xA85;
            field_prepare_actor_action(object);
            func_800A2DD8(object->source_object_index);
            break;
        case 0x3A:
            FIELD_SET_ACTION_ANIMATION(object->source_object_index, 0x4F, 0x25, 0);
            object->motion_parameter = 0xB85;
            field_prepare_actor_action(object);
            func_800A2DD8(object->source_object_index);
            break;
        case 0x39:
            FIELD_SET_ACTION_ANIMATION(object->source_object_index, 0x4F, 0x25, 0);
            object->motion_parameter = 0xB85;
            field_prepare_actor_action(object);
            func_800A2DD8(object->source_object_index);
            break;
        case 0x34:
            FIELD_SET_ACTION_ANIMATION(object->source_object_index, 0x51, 0x27, 0);
            object->motion_parameter = 0xB85;
            field_prepare_actor_action(object);
            func_800A2DD8(object->source_object_index);
            break;
        case 0x8:
        case 0x3B:
        case 0x3C:
        case 0x3D:
            if (object->saved_state < 3)
            {
                return;
            }
            object->facing_or_reward_kind = (object->facing_or_reward_kind & 0x80) | 0x49;
            object->y -= FIELD_STEP_OFFSET(object);
            field_restart_actor_animation(object);
            func_800A2DD8(object->source_object_index);
            object->motion_parameter = 0x96;
            object->motion_divisor = 1;
            object->unknown_0x34 = 1;
            object->unknown_0x35 = 1;
            return;
        case 0x35:
            if ((g_field_object_states[object->source_object_index].contact.flags >> 1) & 1)
            {
                g_field_object_states[FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index])].object_flags &= ~0x2000;
                object->motion_parameter = 0;
                field_restart_sequence_animation(object);
                tmp = func_800839F8(object->source_object_index, 0);
                if (tmp != -1)
                {
                    if (func_8008AABC(object->source_object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index])) != 0)
                    {
                        if (g_field_player_records[object->source_object_index].kind == 8)
                        {
                            func_8008A9D8(object->source_object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index]), 0xD);
                        }
                        else
                        {
                            func_8008A9D8(object->source_object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index]), 0xC);
                        }
                        index = object->source_object_index;
                        anim_id = 0x64;
                        if (g_field_player_records[index].kind == 8)
                        {
                            anim_id = 0x61;
                        }
                    }
                    else
                    {
                        if (g_field_player_records[object->source_object_index].kind == 8)
                        {
                            func_8008A9D8(FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index]), object->source_object_index, 0x18);
                        }
                        else
                        {
                            func_8008A9D8(FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index]), object->source_object_index, 0x17);
                        }
                        index = object->source_object_index;
                        anim_id = 0x65;
                        if (g_field_player_records[index].kind == 8)
                        {
                            anim_id = 0x63;
                        }
                    }
                    if (func_80083EEC(index, tmp, anim_id) != 0)
                    {
                        targets = FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index]);
                        field_start_actor_animation(tmp, 1, (u8*)&targets);
                    }
                    func_800A2DD8(object->source_object_index);
                }
                func_8008BC5C(object);
            }
            return;
        default:
            return;
        }
    }
    else if (func_80091728(object->source_object_index, 1, object) != 0 || func_80091728(object->source_object_index, 0, object) != 0)
    {
        tmp = func_80091728(object->source_object_index, 1, object) != 0;
        switch (object->facing_or_reward_kind & 0x7F)
        {
        case 0x25:
            if (g_field_resource_actions[object->source_object_index].tracks[tmp].command == 8 ||
                g_field_resource_actions[object->source_object_index].tracks[tmp].command == 0x3C)
            {
                object->motion_parameter = 0x985;
                field_prepare_actor_action(object);
                func_800A2DD8(object->source_object_index);
            }
            break;
        case 0x31:
            if (g_field_resource_actions[object->source_object_index].tracks[tmp].command == 8)
            {
                FIELD_SET_ACTION_ANIMATION(object->source_object_index, 0x3C, 0, 1);
                object->motion_parameter = 0xB85;
                field_prepare_actor_action(object);
                func_800A2DD8(object->source_object_index);
            }
            break;
        }
    }
    else if (func_80091728(object->source_object_index, 2, object) != 0)
    {
        if ((object->facing_or_reward_kind & ~0x80) == 0x34)
        {
            FIELD_SET_ACTION_ANIMATION(object->source_object_index, 0x50, 0x26, 0);
            object->motion_parameter = 0xB85;
            field_prepare_actor_action(object);
            func_800A2DD8(object->source_object_index);
        }
        if ((object->facing_or_reward_kind & ~0x80) == 0x35)
        {
            if ((g_field_object_states[object->source_object_index].contact.flags >> 1) & 1)
            {
                g_field_object_states[FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index])].object_flags &= ~0x2000;
                object->motion_parameter = 0;
                field_restart_sequence_animation(object);
                tmp = func_800839F8(object->source_object_index, 0);
                if (tmp != -1)
                {
                    if (func_8008AABC(object->source_object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index])) != 0)
                    {
                        if (g_field_player_records[object->source_object_index].kind == 8)
                        {
                            func_8008A9D8(object->source_object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index]), 0xD);
                        }
                        else
                        {
                            func_8008A9D8(object->source_object_index, FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index]), 0xC);
                        }
                        index = object->source_object_index;
                        anim_id = 0x64;
                        if (g_field_player_records[index].kind == 8)
                        {
                            anim_id = 0x61;
                        }
                    }
                    else
                    {
                        if (g_field_player_records[object->source_object_index].kind == 8)
                        {
                            func_8008A9D8(FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index]), object->source_object_index, 0x18);
                        }
                        else
                        {
                            func_8008A9D8(FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index]), object->source_object_index, 0x17);
                        }
                        index = object->source_object_index;
                        anim_id = 0x65;
                        if (g_field_player_records[index].kind == 8)
                        {
                            anim_id = 0x63;
                        }
                    }
                    if (func_80083EEC(index, tmp, anim_id) != 0)
                    {
                        targets = FIELD_OBJECT_LINKED_OBJECT(&g_field_object_states[object->source_object_index]);
                        field_start_actor_animation(tmp, 1, (u8*)&targets);
                    }
                }
            }
            func_800A2DD8(object->source_object_index);
        }
    }
}

/* field_actor_action_runtime: Validate pending actions, clear completed state, and advance actor sequences. */

/** @brief Object mode that cancels a pending action unless an action selection holds it. */
#define FIELD_PENDING_MODE_CANCEL 3

/** @brief Pending-action retry counter; world objects keep it in the unsigned view of reference_index. */
#define FIELD_PENDING_RETRIES(object) (*(u16*)&(object)->reference_index)

/**
 * @brief Update pending actor actions and clear stale action state.
 * @param object Object whose runtime state and pending-action counter are checked.
 * @return One when the object enters state 0x95; zero otherwise.
 * @note Selection 2 retries the pending action up to five frames (three for kind 0xA players).
 */
s32 func_80093AB8(FieldMotionRecord* object)
{
    s32 selection;
    s32 flags;
    u16 count;
    u8 object_index;
    s32 mode;
    FieldObjectRuntime* states;
    FieldObjectRuntime* state;

    if (object->flags & 0x1FF)
    {
        g_field_object_states[object->source_object_index].object_flags &= 0xFFFF7FFF;
        FIELD_PENDING_RETRIES(object) = FIELD_PENDING_RETRIES(object) + 1;
        return 0;
    }
    selection = func_800A29F8(object->source_object_index, (object->facing_or_reward_kind >> 7) ^ 1, 1);
    states = g_field_object_states;
    state = &states[object->source_object_index];
    if ((FIELD_OBJECT_ACTION_MODE(state) == 2 || FIELD_PENDING_RETRIES(object) != 0) && object->y == 0)
    {
        if (!(object->flags & 0x1FF))
        {
            flags = state->object_flags;
            if (!(flags & 0x400))
            {
                state->object_flags = flags | 0x8000;
            }
        }
        if (selection == 3)
        {
            FIELD_OBJECT_PENDING_STEP(&g_field_object_states[object->source_object_index]) = 0;
            FIELD_PENDING_RETRIES(object) = 0;
        }
        else if (selection < 4)
        {
            if (selection == 2)
            {
                count = FIELD_PENDING_RETRIES(object);
                if (count < 5U)
                {
                    FIELD_PENDING_RETRIES(object) = count + 1;
                }
                if (field_get_next_animation_frame_count(object) == 0 || FIELD_PENDING_RETRIES(object) >= 5U ||
                    (object->source_object_index < 2U && g_field_player_records[object->source_object_index].kind == 0xA &&
                     FIELD_PENDING_RETRIES(object) >= 3U))
                {
                    func_800A2DD8(object->source_object_index);
                    FIELD_PENDING_RETRIES(object) = 0;
                    FIELD_OBJECT_PENDING_STEP(&g_field_object_states[object->source_object_index]) = 0;
                    g_field_object_states[object->source_object_index].object_flags &= 0xFFFF7FFF;
                    field_restart_actor_animation_reverse(object);
                    object->motion_parameter = 0x95;
                    object->position_data.path_time = 0x14;
                    return 1;
                }
            }
            else
            {
                FIELD_OBJECT_PENDING_STEP(&g_field_object_states[object->source_object_index]) = 0;
                FIELD_PENDING_RETRIES(object) = 0;
            }
        }
        else
        {
            FIELD_OBJECT_PENDING_STEP(&g_field_object_states[object->source_object_index]) = 0;
            FIELD_PENDING_RETRIES(object) = 0;
        }
    }
    object_index = object->source_object_index;
    mode = FIELD_OBJECT_ACTION_MODE(&g_field_object_states[object_index]);
    switch (mode)
    {
    case FIELD_PENDING_MODE_CANCEL:
        if (selection != 4 && selection != 6 && selection != 5 && selection != 7 && selection != 8 && selection != 9 && selection != 10)
        {
            func_800A2DD8(object_index);
            FIELD_OBJECT_PENDING_STEP(&g_field_object_states[object->source_object_index]) = 0;
            FIELD_PENDING_RETRIES(object) = 0;
            g_field_object_states[object->source_object_index].object_flags &= 0xFFFF7FFF;
            field_restart_actor_animation_reverse(object);
            object->motion_parameter = 0x95;
            object->position_data.path_time = 0x14;
            return 1;
        }
        break;
    case 8:
    case 9:
    case 10:
        if (selection != 4 && selection != 6 && selection != 5 && selection != 7)
        {
            func_800A2DD8(object_index);
            FIELD_OBJECT_PENDING_STEP(&g_field_object_states[object->source_object_index]) = 0;
            FIELD_PENDING_RETRIES(object) = 0;
            g_field_object_states[object->source_object_index].object_flags &= 0xFFFF7FFF;
        }
        break;
    }
    return 0;
}

/**
 * @brief Finish a sequence-driven action once its bound animation actor is released.
 * @param object Object whose movement flags and sequence state are cleared.
 */
void func_80093EB4(FieldMotionRecord* object)
{
    FieldActorState* actor;
    FieldObjectRuntime* state;
    FieldObjectRuntime* states = g_field_object_states;
    u8 object_index;
    u8 actor_index;
    s32 value;

    object_index = object->source_object_index;
    state = &states[object_index];
    actor_index = state->contact.bytes.animation_actor_index;
    if (actor_index == FIELD_SEQUENCE_NO_ACTOR)
    {
        if (object->motion_scale != 0)
        {
            return;
        }
        state->movement.word &= ~0x1800;
        object->motion_parameter = 0;
        object->flags &= ~0x800;
        states[object->source_object_index].effect_intensity = 0;
        if (object->source_object_index < 2)
        {
            func_800A2DD8(object->source_object_index);
        }
    }
    else
    {
        value = object->motion_scale;
        actor = &g_field_actor_slots[actor_index];
        if (value != 0 || (actor->active_track_mask != 0 && actor->owner_object_index == object_index))
        {
            return;
        }
        /* The index is passed twice; func_8008A678 reads only the first. */
        value = object_index;
        func_8008A678(value, object_index);
        object->motion_parameter = 0;
        states[object->source_object_index].movement.word &= ~0x1800;
        if (actor->owner_object_index == object->source_object_index)
        {
            field_update_sequence_actor_binding(object, 1);
        }
        object->flags &= ~0x800;
        states[object->source_object_index].effect_intensity = 0;
        if (object->source_object_index < 2)
        {
            func_800A2DD8(object->source_object_index);
        }
    }
}

/**
 * @brief Advance an actor sequence and apply its pending horizontal motion.
 * @param object Object whose sequence cursor and motion remainder are advanced.
 * @param sequence_index Script row within the object's selected bank.
 * @note Command 0xF1 advances the cursor; 0xEF handles sequence completion.
 * @note The loop-wrapped row computations are scheduling levers.
 */
void func_8009403C(FieldMotionRecord* object, s32 sequence_index)
{
    Vec3i* scratch = (Vec3i*)FIELD_SEQUENCE_DISPLACEMENT_SCRATCH;
    FieldObjectRuntime* states;
    FieldObjectRuntime* state;
    FieldObjectRuntime* delay_state;
    FieldObjectRuntime* reset_state;
    FieldObjectRuntime* final_states;
    u8* active_scripts;
    FieldPlayerRecord* active_players;
    s32 active_address;
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

    if (object->motion_scale == 0)
    {
        states = g_field_object_states;
        delay_state = &states[object->source_object_index];
        delay = delay_state->sequence_delay;
        if (delay != 0)
        {
            delay_state->sequence_delay = delay - 1;
        }
        if (delay == 0 || states[object->source_object_index].sequence_delay == 0)
        {
            if (field_object_has_active_actor_tracks(object->source_object_index) != 0)
            {
                /* Wait while the bound actor still plays the current frame. */
                object_index = object->source_object_index;
                active_cursor = states[object_index].sequence_cursor;
                if (active_cursor == 1)
                {
                    goto apply_motion;
                }
                do
                {
                    active_scripts = g_field_actor_sequence_data;
                } while (0);
                active_players = g_field_player_records;
                active_address = (sequence_index << 5) + active_players[object_index].kind * FIELD_SEQUENCE_BANK_SIZE;
                active_address += (s32)active_scripts;
                command = *(u8*)(active_address + active_cursor);
                if (command == FIELD_SEQUENCE_END || command == FIELD_SEQUENCE_WAIT_ANIMATION)
                {
                    goto apply_motion;
                }
            }
            reset_state = &states[object->source_object_index];
            if (reset_state->sequence_cursor == 1)
            {
                reset_state->movement.word &= ~FIELD_SEQUENCE_MOVEMENT_MASK;
                states[object->source_object_index].contact.bytes.target_count = 0;
            }
            scripts = g_field_actor_sequence_data;
            players = g_field_player_records;
            object_index = object->source_object_index;

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
            object_index = object->source_object_index;
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
                    func_8008A678(object_index);
                    field_stop_actor_animations_for_object(object, 1);
                    object->motion_parameter = 0;
                    field_update_sequence_actor_binding(object, 1);
                    object->flags &= ~0x800;
                    if (object->source_object_index < 2U)
                    {
                        func_800A2DD8(object->source_object_index);
                    }
                    states[object->source_object_index].object_flags &= ~0x4000;
                    states[object->source_object_index].object_flags &= 0xFFFF7FFF;
                    return;
                }
                state->sequence_cursor = cursor + 1;
                return;
            }
            if (field_execute_actor_sequence(object, sequence_index) != 0)
            {
                func_8008A678(object->source_object_index);
                object->motion_parameter = 0;
                field_update_sequence_actor_binding(object, 1);
                object->flags &= ~0x800;
                if (object->source_object_index < 2U)
                {
                    func_800A2DD8(object->source_object_index);
                }
                states[object->source_object_index].object_flags &= ~0x4000;
                states[object->source_object_index].object_flags &= 0xFFFF7FFF;
                return;
            }
            field_restart_actor_animation(object);
            object->flags |= 0x800;
        }
    }
apply_motion:
    amount = (s8)object->motion_remainder / object->motion_divisor;
    part = &g_field_object_parts[object->source_object_index];
    object->motion_remainder = object->motion_remainder - amount;
    if (object->facing_or_reward_kind & FIELD_SEQUENCE_FACING)
    {
        scratch->x = ((amount << 8) * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    }
    else
    {
        scratch->x = (-(amount << 8) * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    }
    scratch->z = 0;
    scratch->y = 0;
    field_resolve_actor_movement(object, scratch, 1);
    if (g_field_resource_entries[object->resource_index].mode == 0)
    {
        final_states = g_field_object_states;
        final_states[object->source_object_index].movement.word &= ~0x4000;
    }
}

/* field_actor_displacement: Apply actor displacement, follow leader history, and refresh collision contact. */

/**
 * @brief Apply a scaled movement step and clear selected states when the move is blocked.
 * @param object Moving object; motion_divisor splits motion_remainder into steps.
 * @param direction_x Horizontal direction scale.
 * @param vertical_step Vertical displacement before part scaling.
 * @param direction_z Depth direction scale.
 * @return Unspecified; callers ignore it.
 */
s32 func_80094508(FieldMotionRecord* object, s32 direction_x, s32 vertical_step, s32 direction_z)
{
    s32 step;
    FieldActorPartDef* part;
    s32* out;
    s16 state;

    out = (s32*)FIELD_SEQUENCE_DISPLACEMENT_SCRATCH;
    if (object->motion_scale == 0)
    {
        object->motion_parameter = 0;
    }
    else
    {
        field_update_actor_movement_animation(object, direction_x, direction_z);
        step = (s8)object->motion_remainder / object->motion_divisor;
        object->motion_remainder = object->motion_remainder - step;
        part = &g_field_object_parts[object->source_object_index];
        out[0] = (step * direction_x * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
        out[1] = (vertical_step * part->footprint_scale_y) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
        out[2] = (step * direction_z * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
        if (field_resolve_actor_movement(object, out, 0) == 0)
        {
            state = object->motion_parameter;
            if (state == 0x8B || state == 0xAC || state == 0x8C || state == 0xB0 || state == 0xB1)
            {
                object->motion_parameter = 0;
            }
        }
    }
}

/**
 * @brief Move an object by a speed-scaled X/Z displacement.
 * @param object Object supplying the speed byte; a blocked move clears its state.
 * @param x X displacement before scaling.
 * @param z Z displacement before scaling.
 */
void func_80094690(FieldMotionRecord* object, s32 x, s32 z)
{
    Vec3i* vector;
    s32 scaled;

    scaled = x * object->position_data.path_time;
    vector = (Vec3i*)FIELD_SEQUENCE_DISPLACEMENT_SCRATCH;
    vector->y = 0;
    vector->x = scaled;
    vector->z = z * object->position_data.path_time;
    if (field_resolve_actor_movement(object, (s32*)vector, 0) == 0)
    {
        object->motion_parameter = 0;
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
 * @brief Follow the leader's stored positions or restore the object's idle behavior.
 * @param object Follower whose movement and history index are updated.
 */
void func_800946FC(FieldMotionRecord* object)
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
    delta->vx = (g_field_actors[0].x - object->x) / 256;
    delta->vz = (g_field_actors[0].z - object->z) / 256;
    gte_ldlvl(delta);
    gte_sqr0();
    gte_stlvnl(squares);
    index = object->source_object_index;
    distance = squares->vx + squares->vz;
    if (((index + 1) * 2000 < distance) && (states = g_field_object_states, state = &states[index], next = state->history_index, next < 47))
    {
        state->history_index = next + 1;
        delta->vx = (FIELD_HISTORY_POINT(states, g_field_actors[0].source_object_index, states[object->source_object_index].history_index).x << 8) - object->x;
        delta->vz = (FIELD_HISTORY_POINT(states, g_field_actors[0].source_object_index, states[object->source_object_index].history_index).z << 8) - object->z;
        gte_ldlvl(delta);
        gte_sqr12();
        gte_stlvnl(squares);
        if (squares->vx + squares->vz > 384 && !(g_field_resource_entries[object->resource_index].flags & 1))
        {
            object->rotation_y_16 = 1;
        }
        else
        {
            object->rotation_y_16 = 0;
        }
    }
    else
    {
        if (g_field_resource_entries[object->resource_index].flags & 1)
        {
            object->facing_or_reward_kind &= 0x80;
        }
        else
        {
            facing = object->facing_or_reward_kind & 0x80;
            facing += 2;
            object->facing_or_reward_kind = facing;
        }
        object->motion_scale = 1;
        field_restart_sequence_animation(object);
        object->rotation_y_16 = 0;
        object->motion_parameter = 0;
        return;
    }
    field_update_actor_movement_animation(object, delta->vx, delta->vz);
    if (field_resolve_actor_movement(object, (s32*)delta, 0) == 0)
    {
        object->rotation_y_16 = 0;
        object->motion_parameter = 0;
    }
}

/**
 * @brief Drop a bound action once its actor stops, or move the object by a speed-scaled vector.
 * @param object Object whose binding or movement is updated.
 * @param x X displacement before scaling.
 * @param y Y displacement before scaling.
 * @param z Z displacement before scaling.
 * @note Each binding lookup clamps objects 2 and up to the shared third binding.
 */
void func_800949CC(FieldMotionRecord* object, s32 x, s32 y, s32 z)
{
    s32 offset;
    s32 owner;
    s32 object_index;
    u8* owner_bindings;
    u8* actor_bindings;
    u8* track_bindings;
    FieldActorState* actors;
    FieldActorState* actor;
    s32* scratch;

    scratch = (s32*)FIELD_SEQUENCE_DISPLACEMENT_SCRATCH;
    if (object->motion_scale == 0)
    {
        owner_bindings = (u8*)g_field_actor_bindings;
        if (object->source_object_index < 2U)
        {
            offset = object->source_object_index * sizeof(FieldSequenceBinding);
        }
        else
        {
            offset = FIELD_SEQUENCE_SHARED_BINDING * sizeof(FieldSequenceBinding);
        }
        object_index = object->source_object_index;
        owner = ((FieldSequenceBinding*)(owner_bindings + offset))->owner_object_index;
        if (owner == object_index)
        {
            actors = g_field_actor_slots;
            actor_bindings = (u8*)g_field_actor_bindings;
            if ((u8)owner < 2U)
            {
                offset = owner * sizeof(FieldSequenceBinding);
            }
            else
            {
                offset = FIELD_SEQUENCE_SHARED_BINDING * sizeof(FieldSequenceBinding);
            }
            actor = actors + ((FieldSequenceBinding*)(actor_bindings + offset))->actor_index;
            if (actor->is_active != 0)
            {
                actors = g_field_actor_slots;
                track_bindings = (u8*)g_field_actor_bindings;
                if (object->source_object_index < 2U)
                {
                    offset = object->source_object_index * sizeof(FieldSequenceBinding);
                }
                else
                {
                    offset = FIELD_SEQUENCE_SHARED_BINDING * sizeof(FieldSequenceBinding);
                }
                actor = actors + ((FieldSequenceBinding*)(track_bindings + offset))->actor_index;
                if (actor->active_track_mask == 0)
                {
                    object->motion_parameter = 0;
                }
            }
            else
            {
                object->motion_parameter = 0;
            }
        }
        else
        {
            object->motion_parameter = 0;
        }
    }
    else
    {
        scratch[0] = x * object->position_data.path_time;
        scratch[1] = y * object->position_data.path_time;
        scratch[2] = z * object->position_data.path_time;
        field_resolve_actor_movement(object, scratch, 0);
    }
}

/**
 * @brief Count down an object's step timer and slide its height by its speed.
 * @param object Object whose timer, height and state are updated.
 * @param rising Nonzero lowers y by the speed; zero raises it and stops at ground level.
 */
void func_80094B5C(FieldMotionRecord* object, s32 rising)
{
    s8 timer;

    timer = object->height_or_retired_state - 1;
    object->height_or_retired_state = timer;
    if (timer == 0)
    {
        object->motion_parameter = 0;
    }

    if (rising != 0)
    {
        object->y -= object->position_data.path_time << 8;
    }
    else
    {
        s32 height = object->y + (object->position_data.path_time << 8);
        object->y = height;
        if (height >= 0)
        {
            object->y = 0;
            object->motion_parameter = 0;
        }
    }
}

/**
 * @brief Move an object on the ground plane by a speed-scaled X/Z step.
 * @param object Object whose position and speed are used.
 * @param dx X step before scaling.
 * @param dz Z step before scaling.
 */
void func_80094BC4(FieldMotionRecord* object, s32 dx, s32 dz)
{
    u8 speed;

    speed = object->position_data.path_time;
    object->x = object->x + (dx * speed);
    object->z = object->z + (dz * speed);
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

s32 func_8005B6AC(FieldActorCollisionMover* mover);

/**
 * @brief Move an object by its speed and refresh its collision contact and height.
 * @param object Object whose position, speed and runtime state are updated.
 * @param dx Horizontal direction or displacement multiplier.
 * @param dz Depth direction or displacement multiplier.
 * @note Positions outside the map bounds at 0x801ED400 clear the contact instead.
 */
void func_80094C00(FieldMotionRecord* object, s32 dx, s32 dz)
{
    FieldActorCollisionBounds* bounds = (FieldActorCollisionBounds*)0x801ED400;
    FieldActorCollisionMover* mover = (FieldActorCollisionMover*)0x1F800000;
    s32 x, z;
    u8 speed;

    speed = object->position_data.path_time;
    object->x += dx * speed;
    object->z += dz * speed;
    x = object->x;
    z = object->z;
    if (x >= 0 && x < (bounds->width << 8) && z >= 0 && z < ((s32)(bounds->height << 16) >> 7))
    {
        mover->x = x;
        mover->y = object->y;
        mover->z = object->z;
        mover->dx = 0;
        mover->dy = 0;
        mover->dz = 0;
        if (g_field_object_parts[object->source_object_index].appearance.fields.footprint_scale_x == 0x40)
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
        mover->contact = g_field_object_states[object->source_object_index].contact_index;
        mover->surface = g_field_object_states[object->source_object_index].surface;
        func_8005B6AC(mover);
        g_field_object_states[object->source_object_index].contact_index = mover->contact;
        g_field_object_states[object->source_object_index].surface = mover->surface;
        g_field_object_states[object->source_object_index].movement.half.height = mover->height / 256;
    }
    else
    {
        g_field_object_states[object->source_object_index].contact_index = -1;
        g_field_object_states[object->source_object_index].surface = 0;
        g_field_object_states[object->source_object_index].movement.half.height = 0;
    }
}

/* field_actor_resource_states: Advance actor states after their resource requests complete. */

/**
 * @brief Start the object's bound resource animation and enter state 0x8E when it starts.
 * @param object Object whose resource entry supplies the animation flags.
 * @return The busy flag or start result, or 0x8E after a successful start.
 */
s32 func_80094EA4(FieldMotionRecord* object)
{
    s32 result;

    result = g_field_object_states[object->source_object_index].contact.bytes.flags_low & 1;
    if (result == 0)
    {
        result = field_start_bound_action_animation(object->source_object_index, 0, 0, g_field_resource_entries[object->resource_index].bound_animation_flags);
        if (result != 0)
        {
            result = 0x8E;
            object->motion_parameter = result;
        }
    }
    return result;
}

/**
 * @brief Start the object's bound resource animation and enter state 0x94 when it starts.
 * @param object Object whose resource entry supplies the animation flags.
 * @return The busy flag or start result, or 0x94 after a successful start.
 */
s32 func_80094F40(FieldMotionRecord* object)
{
    s32 result;

    result = g_field_object_states[object->source_object_index].contact.bytes.flags_low & 1;
    if (result == 0)
    {
        result = field_start_bound_action_animation(object->source_object_index, 0, 0, g_field_resource_entries[object->resource_index].bound_animation_flags);
        if (result != 0)
        {
            result = 0x94;
            object->motion_parameter = result;
        }
    }
    return result;
}

/* field_actor_animation_resume: Detect released animation slots and resume the actor record animation. */

/**
 * @brief Resume the object's idle animation once its bound actor slot is free.
 * @param object Object whose binding selects the actor slot.
 * @return Unspecified; callers ignore it.
 */
s32 func_80094FDC(FieldMotionRecord* object)
{
    FieldActorState* actors;
    u8* bindings;
    s32 offset;
    s32 actor_index;
    FieldActorState* actor;

    actors = g_field_actor_slots;
    bindings = (u8*)g_field_actor_bindings;
    if (object->source_object_index < 2)
    {
        offset = object->source_object_index * sizeof(FieldSequenceBinding);
    }
    else
    {
        offset = FIELD_SEQUENCE_SHARED_BINDING * sizeof(FieldSequenceBinding);
    }
    actor_index = ((FieldSequenceBinding*)(bindings + offset))->actor_index;
    actor = actors + actor_index;
    if (actor->is_active == 0)
    {
        object->motion_parameter = 0;
        object->state = 0xFF;
        func_80095074(object);
    }
}

/**
 * @brief Restart the object's idle animation, or its first idle track for idle kind 0x1F.
 * @param object Object whose runtime state holds the idle animation.
 * @see decomp.me (100%) TODO
 */
void func_80095074(FieldMotionRecord* object)
{
    s32 i;
    s32 actor_index;
    s32 animation_id;

    if (FIELD_OBJECT_IDLE_ANIMATION(&g_field_object_states[object->source_object_index]) == 0xFF)
    {
        return;
    }
    if (FIELD_OBJECT_IDLE_ANIMATION(&g_field_object_states[object->source_object_index]) == 0x1F)
    {
        for (i = 0; i < 4; i++)
        {
            if (FIELD_OBJECT_IDLE_FLAGS(&g_field_object_states[object->source_object_index])[i] != 0)
            {
                animation_id = FIELD_OBJECT_IDLE_ANIMATION(&g_field_object_states[object->source_object_index]);
                actor_index = func_800839F8(object->source_object_index, 0);
                if (actor_index != -1)
                {
                    if (func_80083EEC(object->source_object_index, actor_index, animation_id))
                    {
                        field_start_actor_animation(actor_index, 0, 0);
                    }
                }
                return;
            }
        }
        return;
    }
    animation_id = FIELD_OBJECT_IDLE_ANIMATION(&g_field_object_states[object->source_object_index]);
    actor_index = func_800839F8(object->source_object_index, 0);
    if (actor_index != -1)
    {
        if (func_80083EEC(object->source_object_index, actor_index, animation_id))
        {
            field_start_actor_animation(actor_index, 0, 0);
        }
    }
}

/**
 * @brief Resume the object's idle animation once its reserved actor slot is free.
 * @param object Object whose index selects the reserved slot at g_field_actor_slots[64 + index].
 * @return Unspecified; the return register is left live but no caller reads it.
 */
s32 func_80095168(FieldMotionRecord* object)
{
    if (g_field_actor_slots[object->source_object_index + 0x40].is_active == 0)
    {
        object->motion_parameter = 0;
        object->state = 0xFF;
        func_80095074(object);
    }
}

/* field_actor_sequence_runtime: Execute object sequences, manage their animation actors, and update tint flashing. */

/**
 * @brief Consume a signed displacement remainder and apply a scaled movement step.
 * @param object Moving object; motion_divisor is the divisor and motion_remainder the remainder.
 * @param direction_x Horizontal direction scale.
 * @param vertical_step Vertical displacement before part scaling.
 * @param direction_z Depth direction scale.
 */
void field_apply_sequence_displacement(FieldMotionRecord* object, s32 direction_x, s32 vertical_step, s32 direction_z)
{
    s32 step;
    FieldActorPartDef* part;
    s32* out;

    out = (s32*)FIELD_SEQUENCE_DISPLACEMENT_SCRATCH;
    if (object->motion_scale == 0)
    {
        object->motion_parameter = 0;
        return;
    }
    step = (s8)object->motion_remainder / object->motion_divisor;
    object->motion_remainder = object->motion_remainder - step;
    part = &g_field_object_parts[object->source_object_index];
    out[0] = (step * direction_x * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    out[1] = (vertical_step * part->footprint_scale_y) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    out[2] = (step * direction_z * part->appearance.fields.footprint_scale_x) >> FIELD_SEQUENCE_MOTION_SCALE_SHIFT;
    field_resolve_actor_movement(object, out, 0);
}

/**
 * @brief Mark an owned animation as running, or release its completed binding.
 * @param object Object selecting the binding, with indices above one sharing the third entry.
 * @param release_actor Clear the completed actor's active flag before releasing its binding.
 */
void field_update_sequence_actor_binding(FieldMotionRecord* object, s32 release_actor)
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
    if (object->source_object_index < 2U)
    {
        binding_offset = (object->source_object_index) * sizeof(*base);
    }
    else
    {
        binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
    }
    if (((FieldSequenceBinding*)((u8*)base + binding_offset))->state != 0)
    {
        base = g_field_actor_bindings;
        if (object->source_object_index < 2U)
        {
            animation_binding_offset = (object->source_object_index) * sizeof(*base);
        }
        else
        {
            animation_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
        }
        if (field_is_actor_animation_active(((FieldSequenceBinding*)((u8*)base + animation_binding_offset))->actor_index) != 0)
        {
            base = g_field_actor_bindings;
            if (object->source_object_index < 2U)
            {
                owner_binding_offset = (object->source_object_index) * sizeof(*base);
            }
            else
            {
                owner_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
            }
            if (((FieldSequenceBinding*)((u8*)base + owner_binding_offset))->owner_object_index == object->source_object_index)
            {
                FieldActorState* actors;
                FieldSequenceBinding* lookup;

                active_owner = ((FieldSequenceBinding*)((u8*)base + owner_binding_offset))->owner_object_index;
                actors = g_field_actor_slots;
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
                    u32 actor_address = (u32)actors;
                    actor_address += ((FieldSequenceBinding*)((u8*)lookup + active_binding_offset))->actor_index * sizeof(*actors);
                    ((FieldActorState*)actor_address)->sequence_active = 1;
                }
            }
        }
        else
        {
            base = g_field_actor_bindings;
            if (object->source_object_index < 2U)
            {
                finished_binding_offset = (object->source_object_index) * sizeof(*base);
            }
            else
            {
                finished_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*base);
            }
            object_index = object->source_object_index;
            finished_owner = ((FieldSequenceBinding*)((u8*)base + finished_binding_offset))->owner_object_index;
            if (finished_owner == object_index)
            {
                if (release_actor != 0)
                {
                    FieldActorState* actors;
                    FieldSequenceBinding* lookup;

                    actors = g_field_actor_slots;
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
                        u32 actor_address = (u32)actors;
                        actor_address += ((FieldSequenceBinding*)((u8*)lookup + release_binding_offset))->actor_index * sizeof(*actors);
                        ((FieldActorState*)actor_address)->is_active = 0;
                    }
                }
                func_80084424(object->source_object_index);
            }
        }
    }
}

/**
 * @brief Consume sequence commands until a frame, wait, delay, or terminator is reached.
 * @param object Object whose runtime state holds the cursor and animation binding.
 * @param script_index Row within the player's selected sequence bank.
 * @return One if the initial cursor already points at the terminator; zero otherwise.
 * @note Animation targets are expanded to four-byte entries for the animation API.
 * @note Command cases retain independent movement-flag updates and target-copy cursors.
 */
s32 field_execute_actor_sequence(FieldMotionRecord* object, s32 script_index)
{
    FieldObjectRuntime* slots;
    FieldPlayerRecord* players;
    u8* programs;
    u8* initial_program;
    u8* initial_program_base;
    FieldSequenceBinding* bindings;
    FieldSequenceBinding* binding_test;
    FieldActorState* actors;
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

    g_field_object_states[object->source_object_index].sequence_delay = 0;
    pending_owner = object->source_object_index;
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
            g_field_object_states[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
        }
    }
    initial_program_base = g_field_actor_sequence_data;
    initial_owner = object->source_object_index;
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
            restore_actors[g_field_actor_bindings[object->source_object_index].actor_index].sequence_active = 0;
            restore_actors[g_field_actor_bindings[object->source_object_index].actor_index].track_count = 0;
            restore_bindings = g_field_actor_bindings;
            if (object->source_object_index < 2U)
            {
                restore_binding_offset = (object->source_object_index) * sizeof(*restore_bindings);
            }
            else
            {
                restore_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*restore_bindings);
            }
            copy_source = &g_field_actor_slots[((FieldSequenceBinding*)((u8*)restore_bindings + restore_binding_offset))->actor_index];
            if (object->source_object_index < 2U)
            {
                template_actor = &g_field_actor_templates[object->source_object_index].actor;
            }
            else
            {
                template_actor = &g_field_shared_actor_template;
            }
            /* Restore the bound runtime actor into its player template. */
            bcopy((const u8*)copy_source, (u8*)template_actor, sizeof(*copy_source));
            actors = g_field_actor_slots;
            bindings = g_field_actor_bindings;
            if (object->source_object_index < 2U)
            {
                release_binding_offset = object->source_object_index * sizeof(*bindings);
            }
            else
            {
                release_binding_offset = FIELD_SEQUENCE_SHARED_BINDING * sizeof(*bindings);
            }
        }
        {
            s32 actor_offset = ((FieldSequenceBinding*)((u8*)bindings + release_binding_offset))->actor_index * sizeof(*actors);
            ((FieldActorState*)((s32)actors + actor_offset))->is_active = 0;
        }
    }
    programs = g_field_actor_sequence_data;
    players = g_field_player_records;
    slots = g_field_object_states;
    script_offset = script_index * FIELD_SEQUENCE_ROW_SIZE;
    command_slot = object->source_object_index;
    opcode_ptr = script_offset + players[command_slot].kind * FIELD_SEQUENCE_BANK_SIZE + programs + cursor;
    opcode = *opcode_ptr;
    result = 0;
    /* Frame bytes stop dispatch; command bytes may consume additional operands. */
    for (; opcode >= FIELD_SEQUENCE_START_TARGETS_0; command_slot = object->source_object_index,
                                                     bank_offset = script_offset + players[command_slot].kind * FIELD_SEQUENCE_BANK_SIZE,
                                                     opcode_ptr = (u8*)(bank_offset + (s32)programs + cursor), opcode = *opcode_ptr)
    {
        switch (opcode)
        {
        case FIELD_SEQUENCE_END:
            g_field_object_states[command_slot].sequence_cursor = cursor;
            object->motion_scale = FIELD_SEQUENCE_FRAME_WAIT;
            object->saved_state = 0;
            object->animation_active = 1;
            object->facing_or_reward_kind = (u8)(object->facing_or_reward_kind & FIELD_SEQUENCE_FACING);
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
                source_state = FIELD_OBJECT_STATE_AT(slots, object->source_object_index);
                kind_flags = ((command - FIELD_SEQUENCE_START_TARGETS_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                sequence_command = source_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK;
                target_state = source_state;
                sequence_command |= kind_flags;
                sequence_command |= FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                target_state->sequence_command = sequence_command;
            }
                target_owner = object->source_object_index;
                actor_index = field_allocate_sequence_actor(target_owner, slots[target_owner].sequence_command);
                {
                    s32 copy_owner;
                    s32* target_output;
                    copy_owner = object->source_object_index;
                    target_index = 0;
                    if (slots[copy_owner].contact.bytes.target_count != 0)
                    {
                        target_output = parameters;
                        do
                        {
                            *target_output = slots[copy_owner].targets[target_index];
                            target_index++;
                            target_output++;
                            copy_owner = object->source_object_index;
                        } while (target_index < slots[copy_owner].contact.bytes.target_count);
                    }
                }
                field_start_actor_animation(actor_index, slots[object->source_object_index].contact.bytes.target_count, (u8*)parameters);
                cursor += 1;
                slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                continue;
            case FIELD_SEQUENCE_START_CURRENT_TARGETS:
                current_target_state = FIELD_OBJECT_STATE_AT(slots, object->source_object_index);
                current_target_state->sequence_command = current_target_state->current_sequence_animation;
                current_target_owner = object->source_object_index;
                actor_index = field_allocate_sequence_actor(current_target_owner, slots[current_target_owner].current_sequence_animation);
                slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                {
                    s32 copy_owner;
                    s32* target_output;
                    copy_owner = object->source_object_index;
                    current_target_index = 0;
                    if (slots[copy_owner].contact.bytes.target_count != 0)
                    {
                        target_output = parameters;
                        do
                        {
                            *target_output = slots[copy_owner].targets[current_target_index];
                            current_target_index++;
                            target_output++;
                            copy_owner = object->source_object_index;
                        } while (current_target_index < slots[copy_owner].contact.bytes.target_count);
                    }
                }
                field_start_actor_animation(actor_index, slots[object->source_object_index].contact.bytes.target_count, (u8*)parameters);
                cursor += 1;
                continue;
            case FIELD_SEQUENCE_DELAY:
                result = 0;
                delay_owner = object->source_object_index;
                pending_state = (FieldObjectRuntime*)(delay_owner * sizeof(*slots));
                delay_operand = script_offset + players[delay_owner].kind * FIELD_SEQUENCE_BANK_SIZE;
                delay_operand += (u32)programs;
                delay_operand += cursor;
                pending_state = (FieldObjectRuntime*)((u8*)pending_state + (s32)slots);
                pending_state->sequence_delay = ((u8*)delay_operand)[1];
                cursor += 2;
                slots[object->source_object_index].sequence_cursor = cursor;
                return result;
            case FIELD_SEQUENCE_WAIT_REPEAT:
            case FIELD_SEQUENCE_WAIT_ANIMATION:
                slots[object->source_object_index].sequence_cursor = cursor;
                return 0;
            case FIELD_SEQUENCE_TOGGLE_CONTROL_14:
                flag_state = FIELD_OBJECT_STATE_AT(slots, object->source_object_index);
                updated_flags = flag_state->object_flags ^ 0x4000;
                flag_state->object_flags = updated_flags;
                cursor += 1;
                func_80086494(object->source_object_index);
                continue;
            case FIELD_SEQUENCE_TOGGLE_CONTROL_15:
                flag_state = FIELD_OBJECT_STATE_AT(slots, object->source_object_index);
                updated_flags = flag_state->object_flags ^ 0x8000;
                flag_state->object_flags = updated_flags;
                cursor += 1;
                func_80086494(object->source_object_index);
                continue;
            case FIELD_SEQUENCE_TOGGLE_FACING:
                cursor += 1;
                object->facing_or_reward_kind = (u8)(object->facing_or_reward_kind ^ FIELD_SEQUENCE_FACING);
                continue;
            case FIELD_SEQUENCE_START_RESOURCE:
                actor_index = func_800839F8(object->source_object_index, 0);
                if (actor_index != -1)
                {
                    resource_owner = object->source_object_index;
                    resource_operand = script_offset + players[resource_owner].kind * FIELD_SEQUENCE_BANK_SIZE;
                    resource_operand += (u32)programs;
                    resource_operand += cursor;
                    func_80083EEC(resource_owner, actor_index, ((u8*)resource_operand)[1]);
                    field_start_actor_animation(actor_index, 0U, NULL);
                }
                cursor += 2;
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_START_0:
            case FIELD_SEQUENCE_START_1:
            case FIELD_SEQUENCE_START_2:
                animation_state = FIELD_OBJECT_STATE_AT(slots, object->source_object_index);
                {
                    s32 kind_flags;
                    s32 sequence_command;
                    kind_flags = ((command - FIELD_SEQUENCE_START_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                    sequence_command = animation_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK;
                    sequence_command |= kind_flags;
                    sequence_command |= FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                    animation_state->sequence_command = sequence_command;
                }
                animation_owner = object->source_object_index;
                field_start_actor_animation(field_allocate_sequence_actor(animation_owner, slots[animation_owner].sequence_command), 0U, NULL);
                slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    cursor += 1;
                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_START_CURRENT:
                current_animation_state = FIELD_OBJECT_STATE_AT(slots, object->source_object_index);
                current_animation_state->sequence_command = current_animation_state->current_sequence_animation;
                allocation_owner = object->source_object_index;
                actor_index = field_allocate_sequence_actor(allocation_owner, slots[allocation_owner].current_sequence_animation);
                slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                field_start_actor_animation(actor_index, 0U, NULL);
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    cursor += 1;
                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_SET_ANIMATION:
                command_owner = object->source_object_index;
                target_state = (FieldObjectRuntime*)(command_owner * sizeof(*slots));
                animation_operand = script_offset + players[command_owner].kind * FIELD_SEQUENCE_BANK_SIZE;
                animation_operand += (u32)programs;
                animation_operand += cursor;
                target_state = (FieldObjectRuntime*)((s32)target_state + (s32)slots);
                target_state->sequence_command = ((u8*)animation_operand)[1];
                cursor += 2;
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_ALLOCATE_0:
            case FIELD_SEQUENCE_ALLOCATE_1:
            case FIELD_SEQUENCE_ALLOCATE_2:
                allocation_state = FIELD_OBJECT_STATE_AT(slots, object->source_object_index);
                {
                    s32 kind_flags;
                    s32 sequence_command;
                    kind_flags = ((command - FIELD_SEQUENCE_ALLOCATE_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                    sequence_command = allocation_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK;
                    sequence_command |= kind_flags;
                    sequence_command |= FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                    allocation_state->sequence_command = sequence_command;
                }
                allocation_owner = object->source_object_index;
                animation_command = slots[allocation_owner].sequence_command;
                field_allocate_sequence_actor(allocation_owner, animation_command);
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    cursor += 1;
                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
                    cleared_state->movement.word &= movement_mask;
                }
                break;
            case FIELD_SEQUENCE_ALLOCATE_CURRENT:
                current_allocation_state = FIELD_OBJECT_STATE_AT(slots, object->source_object_index);
                current_allocation_state->sequence_command = current_allocation_state->current_sequence_animation;
                allocation_owner = object->source_object_index;
                animation_command = slots[allocation_owner].current_sequence_animation;
                field_allocate_sequence_actor(allocation_owner, animation_command);
                clear_slot = object->source_object_index;
                {
                    FieldObjectRuntime* cleared_state = FIELD_OBJECT_STATE_AT(slots, clear_slot);

                    cursor += 1;
                    movement_mask = ~FIELD_SEQUENCE_MOVEMENT_MASK;
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
        frame_offset = (script_index * FIELD_SEQUENCE_ROW_SIZE) + frame_players[object->source_object_index].kind * FIELD_SEQUENCE_BANK_SIZE;
        frame_address = frame_offset;
        frame_address += (u32)frame_programs;
        frame_address += cursor;
        cursor++;
        frame_slots = g_field_object_states;
        object->facing_or_reward_kind = *(u8*)frame_address + (object->facing_or_reward_kind & FIELD_SEQUENCE_FACING);
        frame_slots[object->source_object_index].sequence_cursor = cursor;
        object->motion_scale = FIELD_SEQUENCE_FRAME_WAIT;
        object->saved_state = 0;
        object->animation_active = 1;
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
    slot = func_800839F8(index, 0);
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
 * @brief Reset sequence frame progress and restart the object's animation.
 * @param object Object whose movement flags and animation state are reset.
 */
void field_restart_sequence_animation(FieldMotionRecord* object)
{
    object->motion_scale = FIELD_SEQUENCE_FRAME_WAIT;
    object->saved_state = 0;
    object->animation_active = 1;

    g_field_object_states[object->source_object_index].movement.word &= ~FIELD_SEQUENCE_MOVEMENT_MASK;

    field_restart_actor_animation(object);
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
        if (g_field_actors[i].state != 0xFF)
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
                    slot->movement.word &= ~FIELD_OBJECT_TINT_FLASH;
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
                            slot->movement.word &= ~FIELD_OBJECT_TINT_FLASH;
                            visual->red_or_track = slot->tint_red;
                            visual->green_or_track = slot->tint_green;
                            visual->blue_or_track = slot->tint_blue;
                            continue;
                        }
                    }
                }
                options = slot->movement.word & ~FIELD_OBJECT_TINT_FLASH;
                slot->movement.word = options;
                if (g_field_actors[i].state != 0xFF && slot->current_hp != 0 && !(slot->contact.bytes.flags_low & 1))
                {
                    flags = slot->contact.flags;
                    if (!((flags >> 5) & 1) && !((flags >> 7) & 1) && g_field_active_group != 0)
                    {
                        slot->movement.word = options | FIELD_OBJECT_TINT_FLASH;
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
