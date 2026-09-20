/** @file field_actor_sequence_runtime.c
 * @brief Execute object sequences, manage their animation actors, and update tint flashing.
 */
#include "field_actor_sequence_runtime.h"
#include "field_actor_runtime.h"
#include "field_contact_geometry.h"
#include "sdk/memory.h"

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

/** @brief Binding of an object owner to a temporary animation actor. */
typedef struct
{
    s32 state;
    u8 pad4[8];
    s32 owner_object_index;
    u8 pad10[8];
    s32 actor_index;
} FieldSequenceBinding;

/** @brief Player metadata selecting the bank of actor sequence rows. */
typedef struct
{
    u8 flags;
    u8 sequence_bank;
    u8 pad2[0x266];
} FieldSequencePlayer;

/** @brief Actor template followed by the remaining per-player record data. */
typedef struct
{
    FieldActorState actor;
    u8 tail[0x24];
} FieldSequenceTemplate;

extern FieldActorPartDef g_field_object_parts[];
extern FieldSequenceBinding g_field_actor_bindings[];
extern FieldSequencePlayer g_field_player_records[];
extern FieldSequenceTemplate g_field_actor_templates[];
extern FieldActorState g_field_shared_actor_template;
extern u8 g_field_actor_sequence_data[];
extern FieldActorState g_field_actor_slots[];
extern FieldMotionRecord g_field_actors[];
extern s32 g_field_active_group;
extern s32 g_frame_counter;

s32 func_800839F8(s32 owner_index, s32 require_idle_binding);
s32 func_80083EEC(s32 owner_index, s32 actor_index, s32 resource_index);
void func_80084424(s32 owner_index);
void func_80086494(s32 object_index);
void field_restart_actor_animation(FieldMotionRecord* object);

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
    func_80097FA0(object, out, 0);
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
 * @note Address-building and dispatch scopes retain compiler-sensitive forms;
 * see docs/decompilation/field-actor-sequence-runtime-analysis.md for measured alternatives.
 */
s32 field_execute_actor_sequence(FieldMotionRecord* object, s32 script_index)
{
    FieldObjectRuntime* slots;
    FieldSequencePlayer* players;
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
    FieldObjectRuntime* operand_slot;
    s32 clear_slot;
    s32 cursor;
    s32 result;
    s32 initial_binding_offset;
    s32 restore_binding_offset;
    s32 release_binding_offset;
    s32 updated_flags;
    u8* opcode_ptr;
    u8 target_owner;
    u8 current_target_owner;
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
    FieldObjectRuntime* cleared_state;
    FieldObjectRuntime* current_animation_state;
    FieldObjectRuntime* current_allocation_state;
    FieldActorState* pending_actor;
    u8* actor_base;
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
                actor_base = (u8*)g_field_actor_slots;
                pending_actor = (FieldActorState*)(pending_state->contact.bytes.animation_actor_index * sizeof(*pending_actor) + actor_base);
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
    initial_program = (script_index * FIELD_SEQUENCE_ROW_SIZE) + (g_field_player_records[initial_owner].sequence_bank * FIELD_SEQUENCE_BANK_SIZE) +
                      initial_program_base + cursor;
    result = 1;
    if (*initial_program != FIELD_SEQUENCE_END)
    {
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
            if (((FieldSequenceBinding*)((u8*)binding_test + initial_binding_offset))->state == FIELD_SEQUENCE_RESTORE_TEMPLATE)
            {
                actors = g_field_actor_slots;
                bindings = g_field_actor_bindings;
                ((FieldActorState*)(bindings[object->source_object_index].actor_index * sizeof(*actors) + (u8*)actors))->sequence_active = 0;
                ((FieldActorState*)(bindings[object->source_object_index].actor_index * sizeof(*actors) + (u8*)actors))->track_count = 0;
                if (object->source_object_index < 2U)
                {
                    restore_binding_offset = (object->source_object_index) * sizeof(*bindings);
                }
                else
                {
                    restore_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*bindings);
                }
                copy_source =
                    (FieldActorState*)(((FieldSequenceBinding*)((u8*)bindings + restore_binding_offset))->actor_index * sizeof(*actors) + (u8*)actors);
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
                    release_binding_offset = (object->source_object_index) * sizeof(*bindings);
                    goto release_restored_actor;
                }
                goto fallback_binding;
            }
            return 0;
        }
        goto begin_commands;
    sequence_finished:

        g_field_object_states[command_slot].sequence_cursor = cursor;
        object->motion_scale = FIELD_SEQUENCE_FRAME_WAIT;
        object->saved_state = 0;
        object->animation_active = 1;
        object->facing_or_reward_kind = (u8)(object->facing_or_reward_kind & FIELD_SEQUENCE_FACING);
        return 0;

    fallback_binding:
        release_binding_offset = (FIELD_SEQUENCE_SHARED_BINDING) * sizeof(*bindings);
    release_restored_actor:
        ((FieldActorState*)((u8*)actors + ((FieldSequenceBinding*)((u8*)bindings + release_binding_offset))->actor_index * sizeof(*actors)))->is_active = 0;
    begin_commands:
        programs = g_field_actor_sequence_data;
        players = g_field_player_records;
        slots = g_field_object_states;
        script_offset = script_index * FIELD_SEQUENCE_ROW_SIZE;
        command_slot = object->source_object_index;
        opcode_ptr = script_offset + players[command_slot].sequence_bank * FIELD_SEQUENCE_BANK_SIZE + programs + cursor;
        opcode = *opcode_ptr;
        result = 0;
        if (opcode >= FIELD_SEQUENCE_START_TARGETS_0)
        {
        dispatch_command:
            if (opcode != FIELD_SEQUENCE_END)
            {
                command = *opcode_ptr;
                /* Frame bytes are below FIELD_SEQUENCE_START_TARGETS_0; FIELD_SEQUENCE_END ends the sequence. */
                switch (command)
                {
                case FIELD_SEQUENCE_START_TARGETS_0:
                case FIELD_SEQUENCE_START_TARGETS_1:
                case FIELD_SEQUENCE_START_TARGETS_2:
                    target_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                    target_state = (FieldObjectRuntime*)((s32)target_state + (u8*)slots);
                    do
                    {
                        s32 kind;
                        kind = ((command - FIELD_SEQUENCE_START_TARGETS_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                        target_state->sequence_command =
                            kind | (target_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK) | FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                    } while (0);
                    target_owner = object->source_object_index;
                    actor_index = field_allocate_sequence_actor(target_owner, slots[target_owner].sequence_command);
                    for (target_index = 0; target_index < slots[object->source_object_index].contact.bytes.target_count; target_index++)
                    {
                        parameters[target_index] = slots[object->source_object_index].targets[target_index];
                    }
                    field_start_actor_animation(actor_index, slots[object->source_object_index].contact.bytes.target_count, (u8*)parameters);
                    cursor += 1;
                    slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                    break;
                case FIELD_SEQUENCE_START_CURRENT_TARGETS:
                    current_target_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                    current_target_state = (FieldObjectRuntime*)((s32)current_target_state + (u8*)slots);
                    current_target_state->sequence_command = current_target_state->current_sequence_animation;
                    current_target_owner = object->source_object_index;
                    actor_index = field_allocate_sequence_actor(current_target_owner, slots[current_target_owner].current_sequence_animation);
                    slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                    for (current_target_index = 0; current_target_index < slots[object->source_object_index].contact.bytes.target_count; current_target_index++)
                    {
                        parameters[current_target_index] = slots[object->source_object_index].targets[current_target_index];
                    }
                    cursor += 1;
                    field_start_actor_animation(actor_index, slots[object->source_object_index].contact.bytes.target_count, (u8*)parameters);
                    break;
                case FIELD_SEQUENCE_DELAY:
                    result = 0;
                    delay_owner = object->source_object_index;
                    operand_slot = (FieldObjectRuntime*)(delay_owner * sizeof(*slots));
                    delay_operand = script_offset + players[delay_owner].sequence_bank * FIELD_SEQUENCE_BANK_SIZE;
                    delay_operand += (u32)programs;
                    delay_operand += cursor;
                    operand_slot = (FieldObjectRuntime*)((u8*)operand_slot + (s32)slots);
                    operand_slot->sequence_delay = ((u8*)delay_operand)[1];
                    cursor += 2;
                    slots[object->source_object_index].sequence_cursor = cursor;
                    return result;
                case FIELD_SEQUENCE_WAIT_REPEAT:
                case FIELD_SEQUENCE_WAIT_ANIMATION:
                    slots[object->source_object_index].sequence_cursor = cursor;
                    return 0;
                case FIELD_SEQUENCE_TOGGLE_CONTROL_14:
                    flag_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                    flag_state = (FieldObjectRuntime*)((s32)flag_state + (u8*)slots);
                    updated_flags = flag_state->object_flags ^ 0x4000;
                    goto refresh_actor_flags;
                case FIELD_SEQUENCE_TOGGLE_CONTROL_15:
                    flag_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                    flag_state = (FieldObjectRuntime*)((s32)flag_state + (u8*)slots);
                    updated_flags = flag_state->object_flags ^ 0x8000;
                refresh_actor_flags:
                    flag_state->object_flags = updated_flags;
                    cursor += 1;
                    func_80086494(object->source_object_index);
                    break;
                case FIELD_SEQUENCE_TOGGLE_FACING:
                    cursor += 1;
                    object->facing_or_reward_kind = (u8)(object->facing_or_reward_kind ^ FIELD_SEQUENCE_FACING);
                    break;
                case FIELD_SEQUENCE_START_RESOURCE:
                    actor_index = func_800839F8(object->source_object_index, 0);
                    if (actor_index != -1)
                    {
                        resource_owner = object->source_object_index;
                        resource_operand = script_offset + players[resource_owner].sequence_bank * FIELD_SEQUENCE_BANK_SIZE;
                        resource_operand += (u32)programs;
                        resource_operand += cursor;
                        func_80083EEC(resource_owner, actor_index, ((u8*)resource_operand)[1]);
                        field_start_actor_animation(actor_index, 0U, NULL);
                    }
                    cursor += 2;
                    clear_slot = object->source_object_index;
                    goto clear_actor_state;
                case FIELD_SEQUENCE_START_0:
                case FIELD_SEQUENCE_START_1:
                case FIELD_SEQUENCE_START_2:
                    animation_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                    animation_state = (FieldObjectRuntime*)((s32)animation_state + (u8*)slots);
                    do
                    {
                        s32 kind;
                        kind = ((command - FIELD_SEQUENCE_START_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                        animation_state->sequence_command =
                            kind | (animation_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK) | FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                    } while (0);
                    animation_owner = object->source_object_index;
                    field_start_actor_animation(field_allocate_sequence_actor(animation_owner, slots[animation_owner].sequence_command), 0U, NULL);
                    slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                    goto advance_actor_command;
                case FIELD_SEQUENCE_START_CURRENT:
                    current_animation_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                    current_animation_state = (FieldObjectRuntime*)((s32)current_animation_state + (u8*)slots);
                    current_animation_state->sequence_command = current_animation_state->current_sequence_animation;
                    allocation_owner = object->source_object_index;
                    actor_index = field_allocate_sequence_actor(allocation_owner, slots[allocation_owner].current_sequence_animation);
                    slots[object->source_object_index].sequence_command = FIELD_SEQUENCE_COMMAND_NONE;
                    field_start_actor_animation(actor_index, 0U, NULL);
                    goto advance_actor_command;
                case FIELD_SEQUENCE_SET_ANIMATION:
                    command_owner = object->source_object_index;
                    operand_slot = &slots[command_owner];
                    animation_operand = script_offset + players[command_owner].sequence_bank * FIELD_SEQUENCE_BANK_SIZE;
                    animation_operand += (u32)programs;
                    animation_operand += cursor;
                    operand_slot->sequence_command = ((u8*)animation_operand)[1];
                    cursor += 2;
                    clear_slot = object->source_object_index;
                    goto clear_actor_state;
                case FIELD_SEQUENCE_ALLOCATE_0:
                case FIELD_SEQUENCE_ALLOCATE_1:
                case FIELD_SEQUENCE_ALLOCATE_2:
                    allocation_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                    allocation_state = (FieldObjectRuntime*)((s32)allocation_state + (u8*)slots);
                    do
                    {
                        s32 kind;
                        kind = ((command - FIELD_SEQUENCE_ALLOCATE_0) << 12) | FIELD_SEQUENCE_TRANSIENT_ACTOR;
                        allocation_state->sequence_command =
                            kind | (allocation_state->current_sequence_animation & FIELD_SEQUENCE_ANIMATION_MASK) | FIELD_SEQUENCE_ANIMATION_OVERRIDE;
                    } while (0);
                    allocation_owner = object->source_object_index;
                    animation_command = slots[allocation_owner].sequence_command;
                    goto allocate_actor;
                case FIELD_SEQUENCE_ALLOCATE_CURRENT:
                    current_allocation_state = (FieldObjectRuntime*)(object->source_object_index * sizeof(*slots));
                    current_allocation_state = (FieldObjectRuntime*)((s32)current_allocation_state + (u8*)slots);
                    current_allocation_state->sequence_command = current_allocation_state->current_sequence_animation;
                    allocation_owner = object->source_object_index;
                    animation_command = slots[allocation_owner].current_sequence_animation;
                allocate_actor:
                    field_allocate_sequence_actor(allocation_owner, animation_command);
                advance_actor_command:
                    clear_slot = object->source_object_index;
                    cursor += 1;
                clear_actor_state:
                    cleared_state = &slots[clear_slot];
                    cleared_state->movement.word = (s32)(cleared_state->movement.word & ~FIELD_SEQUENCE_MOVEMENT_MASK);
                }
                command_slot = object->source_object_index;
                bank_offset = script_offset + players[command_slot].sequence_bank * FIELD_SEQUENCE_BANK_SIZE;
                opcode_ptr = bank_offset + programs + cursor;
                opcode = *opcode_ptr;
                if (opcode < FIELD_SEQUENCE_START_TARGETS_0)
                {
                    result = 0;
                    goto apply_frame;
                }
                goto dispatch_command;
            }
            else
            {
                goto sequence_finished;
            }
        }
        else
        {
        apply_frame:
        {
            u8* frame_programs;
            FieldSequencePlayer* frame_players;
            FieldObjectRuntime* frame_slots;
            u32 frame_address;
            s32 frame_offset;
            frame_programs = g_field_actor_sequence_data;
            frame_players = g_field_player_records;
            frame_offset = (script_index * FIELD_SEQUENCE_ROW_SIZE) + frame_players[object->source_object_index].sequence_bank * FIELD_SEQUENCE_BANK_SIZE;
            frame_address = frame_offset;
            frame_address += (u32)frame_programs;
            frame_address += cursor;
            cursor++;
            frame_slots = g_field_object_states;
            object->facing_or_reward_kind = *(u8*)frame_address + (object->facing_or_reward_kind & FIELD_SEQUENCE_FACING);
            frame_slots[object->source_object_index].sequence_cursor = cursor;
        }
            object->motion_scale = FIELD_SEQUENCE_FRAME_WAIT;
            object->saved_state = 0;
            object->animation_active = 1;
            return 0;
        }
    }
    else
    {
        return 1;
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
    s32 selected;
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
                        if (!((flags >> 6) & 1))
                        {
                            selected = i;
                            if (i >= FIELD_SEQUENCE_BINDING_COUNT)
                            {
                                selected = FIELD_SEQUENCE_SHARED_BINDING;
                            }
                            if (g_field_actor_bindings[selected].owner_object_index == i)
                            {
                                selected = i;
                                if (i >= FIELD_SEQUENCE_BINDING_COUNT)
                                {
                                    selected = FIELD_SEQUENCE_SHARED_BINDING;
                                }
                                if (g_field_actor_bindings[selected].state != 0)
                                {
                                    goto check_blink;
                                }
                            }
                        }
                        slot->movement.word &= ~FIELD_OBJECT_TINT_FLASH;
                        visual->red_or_track = slot->tint_red;
                        goto restore_green;
                    }
                }
            check_blink:
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
                        restore_green:
                            visual->green_or_track = slot->tint_green;
                            visual->blue_or_track = slot->tint_blue;
                        }
                    }
                }
            }
        }
    }
}
