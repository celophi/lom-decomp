/**
 * @file field_actor_script_ops.c
 * @brief Field actor script interpreter: script entry, the actor command
 *        opcodes, and the key-addressed action and animation commands scripts
 *        call.
 */

#include "common.h"
#include "field_actor_tables.h"
#include "main.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"

/** @brief Collision probe: position, horizontal footprint and height tolerance. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u16 width;
    s16 height;
    u16 depth;
} FieldCollisionQuery;

/** @brief Seven-word interaction request consumed by func_800B5534. */
typedef struct
{
    s32 source_key;
    s32 action;
    s32 param;
    s32 target_key;
    s32 unk10;
    s32 unk14;
    s32 mode;
} FieldInteractionRequest;

/** @brief Eight-word two-object request consumed by func_800B5F60. */
typedef struct
{
    s32 source_key;
    s32 unk4;
    s32 unk8;
    s32 target_key;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
} FieldObjectPairRequest;

/** @brief Hit counters kept past the mapped part of PadContext. */
typedef struct
{
    u8 unk0[0x3154];
    s32 unk3154;
    s32 unk3158;
} FieldPadCounters;

/**
 * @brief Object state from its target position onwards (0x23C stride).
 * @note D_80105B30 is &g_field_object_states[0].target_x, an address splat named on its own.
 */
typedef struct
{
    s32 target_x;
    s32 target_y;
    s32 target_z;
    u8 unk5C[0x1AC - 0x5C];
    s32 path_x;
    u8 unk1B0[0x23C - 0x1B0];
    u8 unk23C[0x50];
} FieldObjectTarget;

/** @brief g_field_object_states addressed through D_80105B30. */
#define FIELD_OBJECT_STATES_AT_TARGET ((FieldObjectState*)((u8*)D_80105B30 - 0x50))

/**
 * @brief &states[index] with index * 8 passed in precomputed (the 0x23C multiply spelled out).
 * @param states Object state table.
 * @param index8 @p index multiplied by 8.
 * @param index Object index.
 */
#define FIELD_OBJECT_STATE_BY_INDEX8(states, index8, index) \
    ((FieldObjectState*)((((index8) + (index)) * 16 - (index)) * 4 + (u32)(states)))

/** @brief Fixed address of the field camera block. */
#define FIELD_CAMERA ((FieldCamera*)0x801ED480)

/** @brief Camera block at 0x801ED480 (the X and Z scroll words are mapped). */
typedef struct
{
    s32 unk0;
    s32 x;
    s32 y;
    s32 z;
} FieldCamera;

extern FieldObjectTarget D_80105B30[];
extern s32 g_field_direction_animation_modes[];
extern s32 g_field_actor_walk_animations[];
extern s32 D_8010A020[];
extern u16* g_field_actor_scripts;
extern u8 g_field_resource_actions[];
extern s32 D_8010AE54;
extern s32 D_8010AE58;
extern s32 g_field_boss_hud_shake_frame;

void field_start_actor_animation();
s32 func_80060F58();
void func_8006304C();
void field_initialize_actor_record();
void field_restart_actor_animation();
void field_restart_actor_animation_reverse();
s32 field_get_next_animation_frame_count();
s32 func_800839F8();
s32 field_object_has_active_actor_tracks();
s32 field_count_free_actor_slots();
s32 func_80083EEC();
s32 func_8008404C();
void func_8008A4D0();
void func_8008B870();
void func_8008BE38();
void func_8008BF88();
void func_8008C024();
void func_8008C4A8();
s32 field_start_bound_action_animation();
void func_8009D4D8();
void func_800A3938();
void func_800A39A8();
void func_800A3A90();
void func_800B48B8();
s32 func_800B5534();
void func_800B5F60();
void func_8008A0B0();

void func_80088198(FieldActor* actor);

/**
 * @brief Run the next script command of an actor that has a script and no active command.
 * @param actor Actor to step.
 */
void func_800880EC(FieldActor* actor)
{
    if (actor->script_index != FIELD_SCRIPT_NONE)
    {
        if (actor->command == 0)
        {
            func_80088198(actor);
        }
    }
}

/**
 * @brief Resolve the current command in a field object's active script.
 * @param actor Actor whose active script and script offset are resolved.
 * @return Pointer to the actor's current script command.
 */
u8* field_get_object_script_command(FieldActor* actor)
{
    u8* script_base;

    if (actor->script_index == FIELD_SCRIPT_OBJECT)
    {
        script_base = g_field_object_states[actor->object_index].script;
    }
    else
    {
        script_base = (u8*)g_field_actor_scripts + g_field_actor_scripts[actor->script_index];
    }
    return script_base + actor->script_offset;
}

/**
 * @brief Run the actor's current script command (opcodes 0x00, 0x81-0xBC and 0xFF).
 *
 * Reads the command byte at the actor's script offset and dispatches to its
 * handler: animation and motion setup, spawn of child actors (0xB9), path
 * queries (0xB0/0xB1), resource bindings, and one-shot state changes. Most
 * handlers advance the script offset past their operands. Opcode 0x00 only
 * skips itself; 0x01-0x7F and the unused codes above 0x80 do nothing.
 *
 * @param actor Actor whose current script command runs.
 */
void func_80088198(FieldActor* actor)
{
    RECT rect;
    FieldCollisionQuery query_from;
    FieldCollisionQuery query_to;
    FieldObjectPart* casea5_part;
    FieldObjectPart* casea5_base;
    FieldObjectPart* casea6_part;
    FieldObjectPart* casea6_base;
    s32 casea6_mask;
    FieldActorSlot* bc_slot;
    FieldActor* spawn_actor;
    FieldActor* b9_actor_base;
    FieldObjectState* direct_path_state;
    FieldObjectState* case83_base;
    FieldObjectState* case9e_states;
    u8* case9e_entries;
    FieldObjectState* casea9_states;
    FieldObjectState* casea9_state;
    FieldActorSlot* casea9_slots;
    u8* casea9_entries;
    u8* casea9_check_entries;
    s32 key_or_index; /* action key (0x9E/0xA1/0xA9), binding index (0x9F/0xBC), animation slot (0x83) */
    FieldObjectState* b9_state_base;
    FieldObjectState* direct_path_state_z;
    FieldObjectState* fallback_path_state;
    FieldObjectState* fallback_path_state_z;
    FieldObjectState* case83_state;
    FieldObjectState* retry_state;
    FieldObjectState* hold_state;
    FieldObjectState* contact_state;
    FieldObjectState* movement_state;
    FieldObjectState* unk4c_state;
    FieldObjectState* precheck_state;
    FieldObjectState* query_state;
    FieldObjectState* spawn_state;
    s16 next_offset; /* new script offset in 0x9F and 0xB2 */
    s16 case83_offset;
    s32 offset_8d;
    s32 direct_offset;
    s32 fallback_offset;
    s32 path_offset;
    s16 a7_offset;
    u16 offset_88;
    s32* a1_flag;
    s32 bc_state;
    s32 spawn_control;
    s32 query_x;
    s32 bounds_word; /* map bounds address, then the depth limit and the command byte in 0xB0/0xB1 */
    s32 width_limit;
    s32 spawn_slot;
    s32 track_result;
    s32 query_target_x;
    s32 query_target_z;
    s32 contact_or_z; /* contact word in 0xB9, actor z in 0xB0/0xB1 */
    s32 index_or_count; /* spawn slot (0xB9), track count (0x9F), path length (0xB0/0xB1) */
    s32 one; /* the constant 1 kept in a register (0xB9, 0xBC) */
    s32 case83_binding_offset;
    s32 case9e_binding_offset;
    s32 casea9_check_offset;
    s32 casea9_binding_offset;
    u16 bind_word;
    u16 unk3c_value;
    u16 slot_action_value;
    u8 retry_object;
    u8 party_object;
    s32 opcode; /* command byte; 0xA4 reuses it for the constant 1 */
    u8 direction;
    s32 case9e_object;
    u8 slot_object;
    s32 action_or_index; /* action byte (0xA9), track index (0x9F) */
    u8 old_presence;
    u8 animation_8d;
    u8 wait_frames;
    s32 a7_command;
    u8 casea9_object;
    u8 byte_arg; /* command operand byte, or the constant 1 in 0xB0/0xB1 */
    u16* row;
    void* bc_binding;
    u8* script;
    FieldActor* call_actor;
    FieldActorSlot* case83_slots;
    u8* case83_entries;
    u8* bc_entries;
    FieldObjectState* update_states;
    s32 spawn_command;
    s32 parent_index;
    FieldObjectState* query_base;
    FieldObjectState* precheck_states;
    FieldActorSlot* bc_slots;
    s32 resource_offset;
    u8* action_row;
    s32 action_mask;
    s32 decoded_a4;
    s32 a4_old_flags;
    s32 a4_command_flags;
    s32 decoded_b2;
    s32 track_owner;
    s32 slot_offset_83;
    s32 slot_offset_a9;
    FieldActorBinding* bindings;
    FieldActorBinding* binding;
    s32 binding_state;
    s32 binding_owner;
    FieldActorSlot* slots;
    FieldActorSlot* slot;

    bounds_word = 0x801ED400;

    if (actor->script_index == FIELD_SCRIPT_OBJECT)
    {
        script = g_field_object_states[actor->object_index].script;
    }
    else
    {
        script = (u8*)g_field_actor_scripts + g_field_actor_scripts[actor->script_index];
    }
    script += actor->script_offset;

    opcode = script[0];

    switch (opcode)
    {
    case 0xFF:
        actor->script_index = 0xFF;
        actor->unk10 = 0;
        actor->script_offset += 1;
        return;
    case 0xA2:
        actor->unk10 = 0;
        actor->script_offset++;
        return;
    case 0xA3:
        actor->unk10 = 1;
        actor->script_offset++;
        return;
    case 0xAA:
        func_800A3938(script[1], 0x80);
        actor->script_offset += 2;
        return;
    case 0xAB:
        if (actor->object_index < 3U)
        {
            func_800A3A90(script[1], 0x80, actor->object_index);
        }
        else if (actor->object_index < 6U)
        {
            func_800A39A8(script[1], 0x80, actor->object_index - 3, actor->object_index);
        }
        actor->script_offset += 2;
        return;
    case 0xB9:
        /* Spawn into the highest free actor slot from 12 down to 3. A for or while loop lets loop.c hoist the spawn constants (+4 insns, frame +8). */
        do
        {
            do
            {
                index_or_count = 0xC;
                one = 1;
                b9_state_base = g_field_object_states;
                spawn_state = b9_state_base + 12;
                b9_actor_base = g_field_actors;
                spawn_actor = b9_actor_base + 12;
            loop_15:
                old_presence = spawn_actor->presence;
                if (old_presence == 0xFF)
                {
                    field_initialize_actor_record(index_or_count, 3);
                    spawn_control = spawn_actor->control.word & ~0x1FF;
                    spawn_actor->x = actor->x;
                    spawn_actor->y = actor->y;
                    spawn_control |= 2;
                    spawn_actor->z = actor->z;
                    spawn_actor->presence = 0xFE;
                    spawn_command = script[1];
                    spawn_actor->unk27 = 0;
                    spawn_actor->unk24 = one;
                    spawn_actor->command = 0xB8;
                    spawn_actor->animation = spawn_command;
                    parent_index = actor->object_index;
                    spawn_actor->unk3D = 3;
                    spawn_actor->control.word = spawn_control;
                    spawn_actor->script_index = 0;
                    spawn_actor->unk10 = one;
                    spawn_actor->unk20 = parent_index;
                    spawn_state->group_flags = 0;
                    spawn_state->flags = 0;
                    spawn_state->key = index_or_count;
                    spawn_state->unk18 = 0;
                    contact_or_z = spawn_state->contact.word;
                    contact_or_z &= ~0x80;
                    contact_or_z &= ~1;
                    spawn_state->contact.word = contact_or_z;
                    field_restart_actor_animation(spawn_actor, spawn_control);
                    spawn_slot = func_800839F8(index_or_count, 0);
                    if (spawn_slot != -1)
                    {
                        if (func_80083EEC(index_or_count, spawn_slot, 0xB0U) != 0)
                        {
                            field_start_actor_animation(spawn_slot, 0, 0);
                        }
                    }
                    else
                    {
                        spawn_actor->presence = old_presence;
                    }
                }
                else
                {
                    spawn_state -= 1;
                    index_or_count -= 1;
                    spawn_actor -= 1;
                    if (index_or_count >= 3)
                    {
                        goto loop_15;
                    }
                }
            } while (0);
        } while (0);

        actor->script_offset = actor->script_offset + 2;
        return;
    case 0x81:
        actor->command = (s16)script[0];
        wait_frames = script[1];
        actor->unk2E = (u16)wait_frames;
        if (wait_frames == 0)
        {
            actor->unk2E = 1;
        }
        actor->unk24 = 1;
        actor->script_offset += 2;
        field_restart_actor_animation(actor);
        return;
    case 0x83:
    case 0x84:
    case 0x85:
        case83_base = g_field_object_states;
        case83_state = &case83_base[actor->object_index];
        case83_state->contact.word &= ~0x1C;
        actor->command = (s16)script[0];
        if ((opcode == 0x83) || (opcode == 0x84))
        {
            actor->command = 0x85;
            case83_base[actor->object_index].action = opcode + 0x7D;
            case83_offset = (u16)actor->script_offset + 1;
        }
        else
        {
            case83_base[actor->object_index].action = script[1];
            case83_offset = (u16)actor->script_offset + 2;
        }
        actor->script_offset = case83_offset;
        retry_object = actor->object_index;
        if ((g_field_object_states[retry_object].action == 2) && (actor->unk30 != 0) && (retry_object < 2U))
        {
            actor->animation = (actor->animation & 0x80) + (u16)((u8)actor->unk30 + 0x1F);
            if ((field_get_next_animation_frame_count(actor) == 0) || (actor->unk30 >= 5U))
            {
                g_field_object_states[actor->object_index].retry_count = 0;
                actor->unk30 = 0;
                retry_state = &g_field_object_states[actor->object_index];
                retry_state->flags &= 0xFFFF7FFF;
                actor->animation &= 0x80;
                field_restart_actor_animation_reverse(actor);
                actor->command = 0x95;
                actor->unk20 = 0x14;
                return;
            }
            actor->animation = (actor->animation & 0x80) + 0x1F;
            hold_state = &g_field_object_states[actor->object_index];
            hold_state->flags |= 0x8000;
        }
        else
        {
            g_field_object_states[actor->object_index].retry_count = 0;
            actor->unk30 = 0;
        }
        precheck_states = g_field_object_states;
        precheck_state = &precheck_states[actor->object_index];
        if (precheck_state->flags & 0x400)
        {
            actor->command = 0;
            return;
        }
        /* action 4-7; the plain range test does not fold on the u8 field (+2 insns) */
        if (D_8010AE54 != 0 && (u32)(precheck_state->action - 4) < 4U)
        {
            actor->command = 0;
            return;
        }
        resource_offset = actor->resource_index * 0x190;
        action_row = g_field_resource_actions + g_field_object_states[actor->object_index].action * 8;
        row = (u16*)(resource_offset + (s32)action_row);
        if (!(row[1] & 0x400) && (row[0] == 0) && (row[2] == 0))
        {
            actor->command = 0;
            return;
        }
        if ((row[1] & 0x400) &&
            ((field_object_has_active_actor_tracks(actor->object_index) != 0) || (field_count_free_actor_slots(actor->object_index) < 3)))
        {
            actor->command = 0;
            return;
        }
        if ((row[0] & 0x8000) && !(row[1] & 0x400))
        {
            if (!(((u8)actor->object_index < 3U) && (field_object_has_active_actor_tracks(actor->object_index) == 0) && (D_8010AE58 == 0) &&
                  (field_count_free_actor_slots(actor->object_index) >= 3)))
            {
                actor->command = 0;
                return;
            }
            party_object = actor->object_index;
            if (!((g_field_object_states[party_object].unk48 == 0xFF) &&
                  (func_8008404C(party_object, (row[0] & 0x7FFF) + (u16)((g_field_player_records[party_object].type * 0x18) + 0x88)) != 0)))
            {
                actor->command = 0;
                return;
            }
            case83_slots = g_field_actor_slots;
            case83_entries = (u8*)g_field_actor_bindings;
            if ((u8)actor->object_index < 2U)
            {
                case83_binding_offset = actor->object_index * 0x1C;
            }
            else
            {
                case83_binding_offset = 0x38;
            }
            ((FieldActorSlot*)((s32)case83_slots + (((FieldActorBinding*)&case83_entries[case83_binding_offset])->slot) * (s32)sizeof(FieldActorSlot)))->actor_type = g_field_object_states[actor->object_index].action;
        }
        else
        {
            bind_word = row[3];
            if (bind_word & 0x8000)
            {
                if (func_8008404C(actor->object_index, bind_word & 0x3FF) == 0)
                {
                    actor->command = 0;
                    return;
                }
                case83_slots = g_field_actor_slots;
                case83_entries = (u8*)g_field_actor_bindings;
                if ((u8)actor->object_index < 2U)
                {
                    case83_binding_offset = actor->object_index * 0x1C;
                }
                else
                {
                    case83_binding_offset = 0x38;
                }
                slot_offset_83 = (((FieldActorBinding*)&case83_entries[case83_binding_offset])->slot) * (s32)sizeof(FieldActorSlot);
                ((FieldActorSlot*)((s32)case83_slots + slot_offset_83))->actor_type = g_field_object_states[actor->object_index].action;
            }
        }
        if (row[1] & 0x400)
        {
            update_states = g_field_object_states;
            contact_state = &update_states[actor->object_index];
            contact_state->contact.word |= 0x40;
            movement_state = &g_field_object_states[actor->object_index];
            action_mask = ~0x400;
            movement_state->movement.word = movement_state->movement.word & action_mask;
            g_field_object_states[actor->object_index].unk4A = 0;
            unk4c_state = &g_field_object_states[actor->object_index];
            action_mask = ~1;
            unk4c_state->unk4C = unk4c_state->unk4C & action_mask;
            unk3c_value = row[2];
            if ((unk3c_value != 0xFFFF) && (unk3c_value != 0))
            {
                g_field_object_states[actor->object_index].unk3C = (s32)row[2];
            }
        }
        else if (!(row[0] & 0x8000))
        {
            slot_action_value = row[2];
            if ((slot_action_value != 0xFFFF) && (slot_action_value != 0))
            {
                key_or_index = func_800839F8((s32)actor->object_index, 0);
                if ((key_or_index != -1) && (func_80083EEC((s32)actor->object_index, key_or_index, row[2]) != 0))
                {
                    slot_object = actor->object_index;
                    g_field_actor_slots[key_or_index].actor_type = g_field_object_states[slot_object].action;
                    field_start_actor_animation(key_or_index, 0, 0);
                }
            }
        }
        func_8009D4D8(actor, (u8)row[1]);
        return;
    case 0xAD:
        actor->unk33 = 1;
        /* fallthrough */
    case 0x88:
        actor->command = (s16)script[0];
        direction = script[1];
        actor->unk1B = direction;
        if (g_field_resource_entries[actor->resource_index].flags & 1)
        {
            actor->animation = g_field_actor_walk_animations[direction >> 5];
        }
        else
        {
            actor->animation = g_field_direction_animation_modes[direction >> 5] + ((actor->unk33 & 1) * 5) + 5;
        }
        call_actor = actor;
        byte_arg = script[2];
        actor->unk24 = 1;
        offset_88 = actor->script_offset;
        offset_88 += 3;
        actor->unk2E = (u16)byte_arg;
        call_actor->script_offset = offset_88;
        field_restart_actor_animation(call_actor);
        return;
    case 0x8D:
        actor->command = (s16)script[0];
        animation_8d = script[1] | (actor->animation & 0x80);
        actor->animation = animation_8d;
        if (animation_8d & 0x80)
        {
            actor->unk1B = 0;
        }
        else
        {
            actor->unk1B = 0x80;
        }
        call_actor = actor;
        byte_arg = script[2];
        actor->unk24 = 1;
        offset_8d = actor->script_offset;
        offset_8d += 3;
        actor->unk2E = (u16)byte_arg;
        call_actor->script_offset = offset_8d;
        field_restart_actor_animation(call_actor);
        return;
    case 0x8F:
        func_8008BF88(actor, script[1], script[2], script[3]);
        actor->script_offset = (u16)actor->script_offset + 4;
        return;
    case 0xB0:
    case 0xB1:
        query_x = actor->x;
        if ((query_x < 0) || (width_limit = ((FieldMapBounds*)bounds_word)->width << 8, ((query_x < width_limit) == 0)) ||
            (contact_or_z = actor->z, (contact_or_z < 0)) || (bounds_word = (s32)(((FieldMapBounds*)bounds_word)->depth << 0x10) >> 7, ((contact_or_z < bounds_word) == 0)) ||
            (query_base = g_field_object_states, query_state = &query_base[actor->object_index], query_target_x = query_state->target_x, (query_target_x < 0)) ||
            (query_target_x >= width_limit) || (query_target_z = query_state->target_z, (query_target_z < 0)) || (query_target_z >= bounds_word))
        {
            call_actor = actor;
            g_field_object_states[actor->object_index].path_index = 0;
            direct_path_state = &g_field_object_states[actor->object_index];
            direct_path_state->path_x = direct_path_state->target_x;
            direct_path_state_z = &g_field_object_states[actor->object_index];
            direct_path_state_z->path_z = (s32)direct_path_state_z->target_z;
            byte_arg = 1;
            g_field_object_states[actor->object_index].path_length = 1;
            bounds_word = script[0];
            call_actor->unk2E = 0xFF;
            call_actor->unk24 = 1;
            direct_offset = (u16)call_actor->script_offset + 1;
            call_actor->command = bounds_word;
            call_actor->script_offset = direct_offset;
            field_restart_actor_animation(call_actor);
            return;
        }
        else
        {
            query_from.x = query_x;
            query_from.y = actor->y;
            query_from.z = actor->z;
            if (g_field_object_parts[actor->object_index].footprint == 0x40)
            {
                query_from.width = 0xC;
                query_from.depth = 8;
                query_to.width = 0xC;
                query_to.depth = 8;
            }
            else
            {
                query_from.width = 9;
                query_from.depth = 6;
                query_to.width = 9;
                query_to.depth = 6;
            }
            query_from.height = 0x10;
            query_to.height = 0x10;
            func_8006304C(&query_from);
            query_to.x = g_field_object_states[actor->object_index].target_x;
            query_to.y = g_field_object_states[actor->object_index].target_y;
            query_to.z = g_field_object_states[actor->object_index].target_z;
            index_or_count = func_80060F58(&query_from, &query_to, &g_field_object_states[actor->object_index].path_x, 0);
            call_actor = actor;
            if (index_or_count <= 0)
            {
                g_field_object_states[actor->object_index].path_index = 0;
                fallback_path_state = &g_field_object_states[actor->object_index];
                fallback_path_state->path_x = (s32)fallback_path_state->target_x;
                fallback_path_state_z = &g_field_object_states[actor->object_index];
                fallback_path_state_z->path_z = (s32)fallback_path_state_z->target_z;
                byte_arg = 1;
                g_field_object_states[actor->object_index].path_length = 1;
                bounds_word = script[0];
                call_actor->unk2E = 0xFF;
                call_actor->unk24 = 1;
                fallback_offset = call_actor->script_offset;
                fallback_offset += 1;
                call_actor->command = bounds_word;
                call_actor->script_offset = fallback_offset;
                field_restart_actor_animation(call_actor);
                return;
            }
            else
            {
                g_field_object_states[actor->object_index].path_length = index_or_count;
                g_field_object_states[actor->object_index].path_index = 0;
                byte_arg = script[0];
                bounds_word = script[0];
                call_actor->unk2E = 0xFF;
                call_actor->unk24 = 1;
                path_offset = (u16)call_actor->script_offset + 1;
                call_actor->command = (s16)byte_arg;
                call_actor->script_offset = path_offset;
                field_restart_actor_animation(call_actor);
                return;
            }
        }

    case 0x89:
    case 0x8A:
    case 0x8B:
    case 0x8C:
    case 0xAC:
        actor->command = (s16)script[0];
        actor->unk2E = (u16)script[1];
        actor->script_offset = (u16)actor->script_offset + 2;
        actor->unk24 = 1;
        field_restart_actor_animation(actor);
        return;
    case 0x9C:
    case 0x9D:
        actor->command = (s16)script[0];
        actor->unk2E = 0xFF;
        actor->animation = script[1] + (actor->animation & 0x80);
        actor->unk20 = script[2];
        actor->unk26 = script[3];
        actor->script_offset = (u16)actor->script_offset + 4;
        actor->unk24 = 1;
        field_restart_actor_animation(actor);
        return;
    case 0x9E:
        case9e_entries = (u8*)g_field_actor_bindings;
        if ((u8)actor->object_index < 2U)
        {
            case9e_binding_offset = actor->object_index * 0x1C;
        }
        else
        {
            case9e_binding_offset = 0x38;
        }
        if (((FieldActorBinding*)(case9e_entries + case9e_binding_offset))->state == 0)
        {
            key_or_index = script[1] + (script[2] << 8);
            case9e_object = actor->object_index;
            actor->script_offset = (u16)actor->script_offset + 3;
            case9e_states = g_field_object_states;
            if (case9e_states[case9e_object].flags & 0x400)
            {
                actor->script_index = 0xFF;
                actor->unk10 = 0;
                return;
            }
            if (D_8010AE54 != 0)
            {
                actor->script_index = 0xFF;
                actor->unk10 = 0;
                return;
            }
            if (func_8008404C(case9e_object, key_or_index) == 0)
            {
                actor->script_index = 0xFF;
                actor->unk10 = 0;
                return;
            }
            case9e_states[actor->object_index].movement.word |= 0x8000;
            return;
        }
        break;
    case 0xA9:
        casea9_check_entries = (u8*)g_field_actor_bindings;
        if ((u8)actor->object_index < 2U)
        {
            casea9_check_offset = actor->object_index * 0x1C;
        }
        else
        {
            casea9_check_offset = 0x38;
        }
        if (((FieldActorBinding*)(casea9_check_entries + casea9_check_offset))->state == 0)
        {
            action_or_index = script[3];
            key_or_index = script[1] + (script[2] << 8);
            actor->script_offset = (u16)actor->script_offset + 4;
            casea9_states = g_field_object_states;
            casea9_states[actor->object_index].unk3C = 0xFFFF;
            if (D_8010AE54 != 0)
            {
                actor->script_index = 0xFF;
                actor->unk10 = 0;
                return;
            }
            if (func_8008404C(actor->object_index, key_or_index) == 0)
            {
                actor->script_index = 0xFF;
                actor->unk10 = 0;
                return;
            }
            casea9_object = actor->object_index;
            casea9_state = &casea9_states[casea9_object];
            casea9_slots = g_field_actor_slots;
            casea9_entries = (u8*)g_field_actor_bindings;
            if (casea9_object < 2U)
            {
                casea9_binding_offset = casea9_object * 0x1C;
            }
            else
            {
                casea9_binding_offset = 0x38;
            }
            slot_offset_a9 = (((FieldActorBinding*)&casea9_entries[casea9_binding_offset])->slot) * (s32)sizeof(FieldActorSlot);
            ((FieldActorSlot*)((s32)casea9_slots + slot_offset_a9))->actor_type = action_or_index;
            casea9_state->action = action_or_index;
            g_field_object_states[actor->object_index].movement.word |= 0x8000;
            return;
        }
        break;
    case 0xBC:
        key_or_index = 2;
        if ((u8)actor->object_index < 2U)
        {
            key_or_index = actor->object_index;
        }
        bc_entries = (u8*)g_field_actor_bindings;
        bc_binding = (key_or_index * 0x1C) + bc_entries;
        bc_state = ((FieldActorBinding*)bc_binding)->state;
        if (((bc_state >= 1) && (bc_state <= 2)) && (((FieldActorBinding*)bc_binding)->owner == actor->object_index))
        {
            one = 1;
            if (bc_state != one)
            {
                bc_slots = g_field_actor_slots;
                bc_slot = &bc_slots[((FieldActorBinding*)bc_binding)->slot];
                if (bc_slot->track_mask == 0)
                {
                    bc_slot->animation = bc_slot->default_animation;
                    g_field_object_states[actor->object_index].contact.bytes.target_count = 0;
                    field_start_actor_animation(((FieldActorBinding*)bc_binding)->slot, 0, 0);
                    g_field_object_states[key_or_index].contact.bytes.animation_actor_index = (u8)((FieldActorBinding*)bc_binding)->slot;
                    g_field_actor_slots[((FieldActorBinding*)bc_binding)->slot].unk2A = one;
                    actor->command = 0xBC;
                }
                actor->script_offset++;
                return;
            }
        }
        else
        {
            actor->script_offset++;
            return;
        }
        break;
    case 0x9F:
        key_or_index = 2;
        if ((u8)actor->object_index < 2U)
        {
            key_or_index = actor->object_index;
        }
        bindings = g_field_actor_bindings;
        binding = &bindings[key_or_index];
        binding_state = binding->state;
        if (((binding_state >= 1) && (binding_state <= 2)) && (track_owner = actor->object_index, binding_owner = binding->owner, (binding_owner == track_owner)))
        {
            if (binding_state == 1)
            {
                return;
            }
            slots = g_field_actor_slots;
            slot = &slots[binding->slot];
            if (slot->track_mask == 0)
            {
                index_or_count = 0;
                if (slot->default_animation->flags & 0x800)
                {
                    for (action_or_index = 0; action_or_index < 3; action_or_index++)
                    {
                        if (slot->tracks[action_or_index] != 0)
                        {
                            index_or_count += 1;
                        }
                    }
                    track_result = field_start_bound_action_animation(actor->object_index, 0, 0, ((index_or_count - 1) << 0xC) | 0x4400);
                }
                else
                {
                    track_result = field_start_bound_action_animation(binding_owner, 0, 0, 0);
                }
                if (track_result != 0)
                {
                    actor->command = 0xBC;
                    g_field_object_states[actor->object_index].contact.bytes.target_count = 0;
                }
            }
        }
        next_offset = (u16)actor->script_offset + 2;
        actor->script_offset = next_offset;
        return;
    case 0xA0:
    {
        FieldActor* alias;
        u16 off;
        u8 value;
        alias = actor;
        value = script[0];
        alias->unk2E = 0xFA;
        off = alias->script_offset;
        alias->command = value;
        value = script[1];
        off += 2;
        alias->script_offset = off;
        alias->unk24 = 1;
        alias->unk20 = value;
        field_restart_actor_animation(alias);
        return;
    }
    case 0xA7:
    case 0xB6:
    {
        FieldActor* alias;
        FieldObjectState* states;
        u8 value;
        alias = actor;
        a7_command = script[0];
        alias->unk2E = 0xFA;
        value = a7_command;
        alias->command = value;
        value = script[1];
        alias->unk1B = value;
        states = g_field_object_states;
        value = script[2];
        alias->unk20 = value;
        states[alias->object_index].unk171 = script[3];
        byte_arg = script[4];
        a7_offset = alias->script_offset;
        alias->unk2E = 0xF0;
        alias->unk24 = 1;
        a7_offset += 5;
        alias->animation = byte_arg;
        call_actor = alias;
        call_actor->script_offset = a7_offset;
        field_restart_actor_animation(call_actor);
        return;
    }
    case 0x82:
        actor->script_offset = (u16)actor->script_offset + 1;
        func_8008B870(actor, 0);
        return;
    case 0x8E:
        actor->script_offset = (u16)actor->script_offset + 1;
        func_8008BE38(actor, 1);
        return;
    case 0x90:
        actor->script_offset = (u16)actor->script_offset + 1;
        func_8008C024(actor, -1);
        return;
    case 0x97:
        script += 1;
        actor->script_offset = (u16)actor->script_offset + 9;
        rect.x = script[0] + (script[1] << 8);
        rect.y = (s16)script[2];
        rect.w = (s16)script[3];
        rect.h = (s16)script[4];
        func_8008A4D0(actor, &rect, script[5] | (script[6] << 8), script[7]);
        return;
    case 0xA1:
        key_or_index = script[1] + (script[2] << 8);
        actor->script_offset = (u16)actor->script_offset + 3;
        if (D_8010AE54 == 0 && func_8008404C(actor->object_index, key_or_index) != 0)
        {
            a1_flag = D_8010A020;
            if ((u8)actor->object_index < 2U)
            {
                a1_flag += actor->object_index;
            }
            else
            {
                a1_flag += 2;
            }
            *a1_flag = 1;
            return;
        }
        actor->script_index = 0xFF;
        actor->unk10 = 0;
        return;
    case 0xA4:
        opcode = 1;
        if ((u8)actor->object_index < 2U)
        {
            decoded_a4 = script[0];
            actor->unk2E = opcode;
            actor->command = decoded_a4;
            a4_old_flags = actor->animation;
            a4_command_flags = script[1];
            actor->unk24 = opcode;
            actor->animation = a4_command_flags | (a4_old_flags & 0x80);
            field_restart_actor_animation(actor);
        }
        actor->script_offset = (u16)actor->script_offset + 2;
        return;
    case 0xA5:
        casea5_base = g_field_object_parts;
        casea5_part = &casea5_base[actor->object_index];
        casea5_part->flags |= 0x800000;
        actor->script_offset++;
        return;
    case 0xA6:
        casea6_mask = 0xFF7FFFFF;
        casea6_base = g_field_object_parts;
        casea6_part = &casea6_base[actor->object_index];
        casea6_part->flags &= casea6_mask;
        actor->script_offset++;
        return;
    case 0xA8:
    {
        FieldActor* alias;
        u16 off;
        u8 value;
        alias = actor;
        alias->command = script[0];
        value = script[2];
        alias->unk2E = value;
        off = alias->script_offset;
        value = script[1];
        off += 3;
        alias->script_offset = off;
        alias->unk27 = 0;
        alias->unk24 = 1;
        alias->animation = value;
        field_restart_actor_animation(alias);
        return;
    }
    case 0xB7:
    {
        FieldActor* alias;
        u16 off;
        u8 value;
        alias = actor;
        alias->command = script[0];
        value = script[2];
        alias->unk2E = value;
        value = alias->animation = script[1];
        off = alias->script_offset;
        value = script[3];
        off += 4;
        alias->script_offset = off;
        alias->unk27 = 0;
        alias->unk24 = 1;
        alias->unk20 = value;
        field_restart_actor_animation_reverse(alias);
        return;
    }
    case 0xB2:
        decoded_b2 = script[0];
        actor->unk20 = 0;
        next_offset = (u16)actor->script_offset + 1;
        actor->command = decoded_b2;
        actor->script_offset = next_offset;
        return;
    case 0xB4:
        if (actor->presence == 0xFE)
        {
            actor->presence = 0;
        }
        else
        {
            actor->presence = 0xFE;
        }
        actor->script_offset++;
        return;
    case 0x00:
        actor->script_offset++;
        return;
    default:
        return;
    }
}

/**
 * @brief Start command 0xB2 on the actor with @p key, deriving its wait from its facing.
 * @param key Object key to look up.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 func_80089980(s32 key)
{
    FieldActor* actor;
    u8 animation;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    actor->command = 0xB2;
    animation = actor->animation;
    if (animation < 0x8F)
    {
        switch (animation)
        {
        case 1:
        case 6:
        case 11:
            actor->unk20 = 6;
            break;
        case 2:
        case 7:
        case 12:
            actor->unk20 = 12;
            break;
        case 3:
        case 8:
        case 13:
            actor->unk20 = 18;
            break;
        case 4:
        case 9:
        case 14:
        case 132:
        case 137:
        case 142:
            actor->unk20 = 24;
            break;
        case 131:
        case 136:
        case 141:
            actor->unk20 = 30;
            break;
        case 130:
        case 135:
        case 140:
            actor->unk20 = 36;
            break;
        case 129:
        case 134:
        case 139:
            actor->unk20 = 42;
            break;
        case 0:
            actor->unk20 = 0;
            break;
        }
    }
    else
    {
        actor->unk20 = 0;
    }
    return 0;
}

/**
 * @brief Toggle the hidden state of the actor with @p key.
 * @param key Object key to look up.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 func_80089A68(s32 key)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (actor->presence == FIELD_ACTOR_HIDDEN)
    {
        actor->presence = 0;
    }
    else
    {
        actor->presence = FIELD_ACTOR_HIDDEN;
    }
    return 0;
}

/**
 * @brief Start command 0xBB on the actor with @p key, optionally with an animation actor.
 * @param key Object key to look up.
 * @param resource_index Resource of the animation actor to start, or -1 for none.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 func_80089AE4(s32 key, s32 resource_index)
{
    FieldActor* actor;
    s32 slot_index;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (resource_index != -1)
    {
        slot_index = func_800839F8(actor->object_index, 0);
        if ((slot_index != -1) && (func_80083EEC(actor->object_index, slot_index, resource_index) != 0))
        {
            field_start_actor_animation(slot_index, 0, 0);
        }
    }

    actor->command = 0xBB;
    actor->unk3D = 2;
    actor->unk10 = 1;
    actor->script_index = 0;
    actor->control.word = (actor->control.word & ~0x1FF) | 2;
    return 0;
}

/**
 * @brief Set four parameters and field 0x260 of the party record of the actor with @p key.
 * @param key Object key to look up.
 * @param value1 Value stored in field 0x262.
 * @param value2 Value stored in field 0x264.
 * @param value3 Value stored in field 0x266.
 * @param value0 Value stored in field 0x260.
 * @return 0 on success, or -1 when the actor is absent or not a party member.
 */
s32 func_80089BE8(s32 key, s32 value1, s32 value2, s32 value3, s32 value0)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (actor->object_index >= 3)
    {
        return -1;
    }
    g_field_player_records[actor->object_index].unk260 = value0;
    g_field_player_records[actor->object_index].unk25E = 0;
    g_field_player_records[actor->object_index].unk262 = value1;
    g_field_player_records[actor->object_index].unk264 = value2;
    g_field_player_records[actor->object_index].unk266 = value3;
    return 0;
}

/**
 * @brief Reset the actor with @p key to idle and pull a stray party member back into view.
 * @param key Object key to look up.
 * @param animation New animation, or -1 for animation 0.
 * @param resource_index Resource of an animation actor to start, or -1 for none.
 * @param sound Sound effect to play, or -1 for none.
 * @return 0 on success, or -1 when no actor has @p key.
 */
s32 func_80089D44(s32 key, s32 animation, s32 resource_index, s32 sound)
{
    FieldActor* actor;
    FieldActor* companion;
    FieldObjectState* companion_state;
    s32 slot;
    s32 unused_presence;
    s32 slot_index;
    FieldCamera* camera;
    FieldObjectState* contact_state;
    FieldObjectState* hp_state;
    FieldObjectState* movement_state;

    camera = FIELD_CAMERA;
    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    actor->presence = 0;
    actor->command = 0;
    actor->unk10 = 0;
    actor->unk2E = 1;
    if (animation == -1)
    {
        animation = 0;
    }
    actor->unk27 = 0;
    actor->unk24 = 1;
    actor->animation = (actor->animation & 0x80) | animation;
    field_restart_actor_animation(actor);
    if (resource_index != -1)
    {
        slot_index = func_800839F8(actor->object_index, 0);
        if ((slot_index != -1) && (func_80083EEC(actor->object_index, slot_index, resource_index) != 0))
        {
            field_start_actor_animation(slot_index, 0, 0);
            g_field_object_states[actor->object_index].contact.bytes.animation_actor_index = slot_index;
        }
    }
    if (sound != -1)
    {
        func_800A3938(sound, 0x80);
    }
    func_800B48B8(g_field_object_states[actor->object_index].key);
    g_field_object_states[actor->object_index].flags = 0;
    contact_state = &g_field_object_states[actor->object_index];
    contact_state->contact.bits.flag5 = 0;
    hp_state = &g_field_object_states[actor->object_index];
    hp_state->unk8.word = (hp_state->unk8.word & 0xFF000000) | (hp_state->unk0 & 0xFFFFFF);
    hp_state->unk4.word = hp_state->unk0 & 0xFFFFFF;
    g_field_object_states[actor->object_index].tint_timer = 60;
    movement_state = &g_field_object_states[actor->object_index];
    movement_state->movement.bits.flag15 = 1;
    if (actor->object_index < 3U &&
        (actor->x <= -camera->x + 0xA00 || actor->x >= -camera->x + 0x13600 || actor->z <= -camera->z + 0xA00 || actor->z >= -camera->z + 0x1B600))
    {
        slot = 0;
        unused_presence = FIELD_ACTOR_UNUSED;
        companion_state = g_field_object_states;
        companion = g_field_actors;
        for (; slot < 3; companion_state++, slot++, companion++)
        {
            if (companion->presence != unused_presence && companion_state->unk4.word != 0 && actor->object_index != slot)
            {
                break;
            }
        }
        if (slot != 3)
        {
            func_8008A0B0(actor, slot, 1);
        }
    }
    return 0;
}

/**
 * @brief Route an actor towards another object, falling back to a straight line.
 * @param actor Actor whose path is computed.
 * @param target_index Object whose position is the destination.
 * @param restart Nonzero starts command 0xB5 on the actor.
 */
void func_8008A0B0(FieldActor* actor, s32 target_index, s32 restart)
{
    FieldCollisionQuery start;
    FieldCollisionQuery goal;
    s32 path_length;
    FieldMapBounds* bounds;
    s32 x;
    s32 map_depth;
    s32 map_width;
    s32 state_x;
    s32 state_z;
    s32 z;
    FieldActor* target;

    bounds = FIELD_MAP_BOUNDS;
    x = actor->x;
    if (x < 0 || x >= (map_width = bounds->width << 8) || (z = actor->z) < 0 ||
        z >= (map_depth = (s32)(bounds->depth << 0x10) >> 7) ||
        (state_x = g_field_object_states[actor->object_index].target_x) < 0 || state_x >= map_width ||
        (state_z = g_field_object_states[actor->object_index].target_z) < 0 || state_z >= map_depth)
    {
        g_field_object_states[actor->object_index].path_index = 0;
        g_field_object_states[actor->object_index].path_x = g_field_actors[target_index].x;
        g_field_object_states[actor->object_index].path_z = g_field_actors[target_index].z;
        g_field_object_states[actor->object_index].path_length = 1;
    }
    else
    {
        start.x = x;
        start.y = actor->y;
        start.z = actor->z;
        if (g_field_object_parts[actor->object_index].footprint == 0x40)
        {
            start.width = 12;
            start.depth = 8;
            goal.width = 12;
            goal.depth = 8;
        }
        else
        {
            start.width = 9;
            start.depth = 6;
            goal.width = 9;
            goal.depth = 6;
        }
        start.height = 16;
        goal.height = 16;
        func_8006304C(&start);
        D_80105B30[actor->object_index].target_x = g_field_actors[target_index].x;
        target = &g_field_actors[target_index];
        D_80105B30[actor->object_index].target_y = target->y;
        D_80105B30[actor->object_index].target_z = target->z;
        goal.x = target->x;
        goal.y = target->y;
        goal.z = target->z;
        path_length = func_80060F58(&start, &goal, &D_80105B30[actor->object_index].path_x, 0);
        if (path_length <= 0)
        {
            FIELD_OBJECT_STATES_AT_TARGET[actor->object_index].path_index = 0;
            FIELD_OBJECT_STATES_AT_TARGET[actor->object_index].path_x = target->x;
            FIELD_OBJECT_STATES_AT_TARGET[actor->object_index].path_z = target->z;
            FIELD_OBJECT_STATES_AT_TARGET[actor->object_index].path_length = 1;
        }
        else
        {
            FIELD_OBJECT_STATES_AT_TARGET[actor->object_index].path_length = path_length;
            FIELD_OBJECT_STATES_AT_TARGET[actor->object_index].path_index = 0;
        }
    }
    if (restart != 0)
    {
        actor->command = 0xB5;
        actor->unk2E = 0xFF;
        actor->unk24 = 1;
        field_restart_actor_animation(actor);
    }
}

/**
 * @brief Copy a rectangle of the actor's texture page to its VRAM destination.
 * @param actor Actor whose field 0x0C selects the texture page.
 * @param rect Source rectangle in texture units; converted in place to the VRAM rectangle.
 * @param x Destination X in texture units.
 * @param y Destination Y.
 */
void func_8008A4D0(FieldActor* actor, RECT* rect, s32 x, s32 y)
{
    s32 x_offset;
    s32 y_offset;
    s32 page;

    page = actor->unkC;
    if (page >= 2)
    {
        y_offset = 0;
        if (page >= 9)
        {
            y_offset = 0x100;
            x_offset = 0x3C0 - ((page - 9) << 6);
        }
        else
        {
            x_offset = 0x340 - (page << 6);
        }
    }
    else
    {
        y_offset = 0;
        x_offset = 0x380 - (page << 7);
    }

    rect->x = rect->x >> 2;
    rect->x += x_offset;
    rect->w = rect->w >> 2;
    rect->y += y_offset;
    MoveImage2(rect, (x >> 2) + x_offset, y + y_offset);
}

/**
 * @brief Bind an animation resource to the actor with @p key and flag its pending restart.
 * @param key Object key to look up.
 * @param resource_id Resource to bind.
 * @return -1 when no actor has @p key, 1 when the binding failed, 0 on success.
 */
s32 func_8008A580(s32 key, s32 resource_id)
{
    FieldActor* actor;
    s32* flag;
    s32* flags;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (func_8008404C(actor->object_index, resource_id) != 0)
    {
        flags = D_8010A020;
        if (actor->object_index < 2)
        {
            flag = &flags[actor->object_index];
        }
        else
        {
            flag = flags + 2;
        }
        *flag = 1;
        g_field_object_states[actor->object_index].unk3C = 0xFFFF;
    }
    else
    {
        return 1;
    }
    return 0;
}

/**
 * @brief Send the interactions queued on an object to each of its collected targets.
 * @param object_index Object whose collected targets are processed and then cleared.
 */
void func_8008A678(s32 object_index)
{
    FieldInteractionRequest request;
    FieldObjectState* states;
    FieldObjectState* loop_states;
    FieldObjectState* source;
    FieldObjectState* saved_source;
    FieldObjectState* current;
    s32 i;
    s32 index8;

    states = g_field_object_states;
    index8 = object_index * 8;
    source = FIELD_OBJECT_STATE_BY_INDEX8(states, index8, object_index);
    i = 0;
    if (source->contact.bytes.target_count != 0)
    {
        /* Copies and a per-pass index8 keep the source address inside the loop, as in the original. */
        loop_states = states;
        saved_source = source;
        index8 = object_index * 8;
        do
        {
            current = FIELD_OBJECT_STATE_BY_INDEX8(loop_states, index8, object_index);
            loop_states[current->targets[i]].contact.bits.flag7 = 0;
            if (!loop_states[current->targets[i]].contact.bits.flag5 && loop_states[current->targets[i]].unk4.word != 0)
            {
                request.source_key = current->key;
                if (current->action < 0xB)
                {
                    request.action = current->action;
                }
                else
                {
                    request.action = 0xA;
                }
                request.target_key = loop_states[saved_source->targets[i]].key;
                request.param = 0;
                request.unk10 = 0;
                request.unk14 = 0;
                request.mode = 1;
                func_800B5534(&request);
            }
            index8 = object_index * 8;
            i++;
        } while (i < FIELD_OBJECT_STATE_BY_INDEX8(loop_states, index8, object_index)->contact.bytes.target_count);
    }
    g_field_object_states[object_index].contact.bytes.target_count = 0;
}

/**
 * @brief Send a contact interaction from one object to another.
 * @param source_index Object that makes contact.
 * @param target_index Object that is touched.
 * @return Result of func_800B5534, or 0 when the interaction cannot start.
 */
s32 func_8008A840(s32 source_index, s32 target_index)
{
    FieldInteractionRequest request;
    FieldObjectState* source;
    FieldObjectState* target;
    FieldObjectState* states;
    s32 mode;

    if ((g_field_actors[source_index].command == 0x91) || (g_field_actors[source_index].command == 0x87))
    {
        return 0;
    }
    states = g_field_object_states;
    target = &states[target_index];
    target->contact.bits.flag7 = 0;
    if (target->unk4.word == 0)
    {
        return 0;
    }
    source = &states[source_index];
    request.source_key = source->key;
    if (source->action < 0xB)
    {
        request.action = source->action;
    }
    else
    {
        request.action = 0;
    }
    request.target_key = g_field_object_states[target_index].key;
    if (source_index < 2)
    {
        request.param = g_field_actors[source_index].unk30;
    }
    else
    {
        request.param = 0;
    }
    request.unk10 = 0;
    request.unk14 = 0;
    mode = (g_field_object_states[source_index].contact.word >> 2) & 7;
    if (mode == 0)
    {
        mode = 1;
    }
    request.mode = mode;
    func_8008C4A8(source_index);
    return func_800B5534(&request);
}

/**
 * @brief Send an interaction with an explicit action from one object to another.
 * @param source_index Object that starts the interaction.
 * @param target_index Object that receives it.
 * @param action Interaction action.
 * @return Result of func_800B5534, or 0 when the interaction cannot start.
 */
s32 func_8008A9D8(s32 source_index, s32 target_index, s32 action)
{
    FieldObjectState* states;
    FieldObjectState* target;
    FieldInteractionRequest request;

    if ((g_field_actors[source_index].command != 0x91) && (g_field_actors[source_index].command != 0x87))
    {
        states = g_field_object_states;
        target = &states[target_index];
        target->contact.bits.flag7 = 0;
        if (target->unk4.word != 0)
        {
            request.source_key = states[source_index].key;
            request.action = action;
            request.target_key = target->key;
            request.param = 0;
            request.unk10 = 0;
            request.unk14 = 0;
            request.mode = 1;
            func_8008C4A8(source_index);
            return func_800B5534(&request);
        }
    }
    return 0;
}

/**
 * @brief Send a two-object request built from their keys to func_800B5F60.
 * @param source_index First object.
 * @param target_index Second object.
 */
void func_8008AABC(s32 source_index, s32 target_index)
{
    FieldObjectPairRequest request;

    request.source_key = g_field_object_states[source_index].key;
    request.target_key = g_field_object_states[target_index].key;
    request.unk18 = 1;
    func_800B5F60(&request);
}

/**
 * @brief Register a hit on the actor with @p key and start its hit reaction when allowed.
 * @param key Object key to look up.
 * @param alternate Hit reaction selector forwarded to func_8008B870.
 * @return 0 when the actor was found, -1 when no actor has @p key.
 */
s32 func_8008AB2C(s32 key, s32 alternate)
{
    FieldActor* actor;
    s16 command;
    s32 hit_count;
    FieldObjectState* states;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (actor->object_index == 0)
    {
        hit_count = ((FieldPadCounters*)g_pad_ctx)->unk3154;
        if (hit_count != -1)
        {
            ((FieldPadCounters*)g_pad_ctx)->unk3154 = hit_count + 1;
        }
    }
    states = g_field_object_states;
    states[actor->object_index].flags |= 0x10000000;
    if (actor->object_index < 3)
    {
        g_field_player_records[actor->object_index].hit_state = 5;
    }
    else if (states[actor->object_index].unk8.word < 0)
    {
        g_field_boss_hud_shake_frame = 5;
    }
    command = actor->command;
    if (command == 0x87)
    {
        return 0;
    }
    if (command >= 0x88)
    {
        if (command != 0x91)
        {
            func_8008B870(actor, alternate);
        }
        return 0;
    }
    if (command >= 0x85)
    {
        switch (g_field_object_states[actor->object_index].action)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 8:
        case 9:
        case 10:
            break;
        default:
            if (!g_field_object_states[actor->object_index].contact.bits.flag6)
            {
                return 0;
            }
            break;
        }
    }
    func_8008B870(actor, alternate);
    return 0;
}

/**
 * @brief Knock down the actor with @p key.
 * @param key Object key to look up.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 func_8008AD44(s32 key)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    func_8008BE38(actor, 1);
    return 0;
}

/**
 * @brief Read the animation of the actor with @p key.
 * @param key Object key to look up.
 * @return The animation without its mirror bit, or -1 when no actor has @p key.
 */
s32 func_8008ADB4(s32 key)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    return actor->animation & 0x7F;
}

/**
 * @brief Start command 0xAE on the actor with @p key, counting the hit for non-party objects.
 * @param key Object key to look up.
 * @param value Value forwarded to func_8008C024.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 func_8008AE14(s32 key, s32 value)
{
    FieldActor* actor;
    s32 hit_count;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (actor->object_index >= 3)
    {
        hit_count = ((FieldPadCounters*)g_pad_ctx)->unk3158;
        if (hit_count != -1)
        {
            ((FieldPadCounters*)g_pad_ctx)->unk3158 = hit_count + 1;
        }
    }
    func_8008C024(actor, value);
    return 0;
}

/**
 * @brief Test whether the actor with @p key is idle.
 * @param key Object key to look up.
 * @return 1 when idle, 0 when busy, -1 when no actor has @p key.
 */
s32 func_8008AEB0(s32 key)
{
    s32 result;
    s32 masked;
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if ((actor->control.half[0] & 0x1FF) < 2)
    {
        result = 0;
        if (actor->command == 0)
        {
            masked = actor->control.word & 0x600;
            result = masked == 0;
        }
    }
    else
    {
        result = 0;
        if ((actor->command == 0x81) || (actor->command == 0))
        {
            result = 1;
        }
    }
    return result;
}

/**
 * @brief Start command 0x8F on the actor with @p key.
 * @param key Object key to look up.
 * @param heading Heading forwarded to func_8008BF88.
 * @param animation Animation forwarded to func_8008BF88.
 * @param wait Wait count forwarded to func_8008BF88.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 func_8008AF68(s32 key, s32 heading, s32 animation, s32 wait)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    func_8008BF88(actor, heading, animation, wait);
    return 0;
}

/**
 * @brief Turn the actor with @p source_key to face the actor with @p target_key.
 * @param source_key Key of the actor that turns.
 * @param target_key Key of the actor to face.
 * @return 0 on success, or -1 when either actor is absent.
 */
s32 func_8008AFD8(s32 source_key, s32 target_key)
{
    FieldActor* source;
    FieldActor* target;
    FieldObjectState* states;
    FieldObjectState* state;
    s32 angle;

    source = field_find_actor(source_key);
    if (source == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    target = field_find_actor(target_key);
    if (target == FIELD_ACTOR_NONE)
    {
        return -1;
    }

    angle = ratan2(source->z - target->z, target->x - source->x);
    if (!(g_field_resource_entries[source->resource_index].flags & 1))
    {
        if (angle < -0x700)
        {
            source->animation = 2;
        }
        else if (angle < -0x500)
        {
            source->animation = 3;
        }
        else if (angle < -0x300)
        {
            source->animation = 4;
        }
        else if (angle < -0x100)
        {
            source->animation = 0x83;
        }
        else if (angle < 0x100)
        {
            source->animation = 0x82;
        }
        else if (angle < 0x300)
        {
            source->animation = 0x81;
        }
        else if (angle < 0x500)
        {
            source->animation = 0;
        }
        else if (angle < 0x700)
        {
            source->animation = 1;
        }
        else
        {
            source->animation = 2;
        }
    }
    else
    {
        if (angle > 0x400 && angle < 0xC00)
        {
            source->animation = 0;
        }
        else if (angle < -0x400)
        {
            if (angle < -0xBFF)
            {
                source->animation = 0x80;
            }
            else
            {
                source->animation = 0;
            }
        }
        else
        {
            source->animation = 0x80;
        }
    }

    states = g_field_object_states;
    source->unk2E = 1;
    source->unk27 = 0;
    source->unk24 = 1;
    state = &states[source->object_index];
    state->movement.word &= ~0x1800;
    field_restart_actor_animation(source);
    return 0;
}

/**
 * @brief Set the animation of the actor with @p key and restart it.
 * @param key Object key to look up.
 * @param animation New animation byte.
 * @return 0 on success, or -1 when no actor has @p key.
 */
s32 func_8008B1C8(s32 key, u8 animation)
{
    FieldActor* actor;
    FieldObjectState* states;
    FieldObjectState* state;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    states = g_field_object_states;
    actor->animation = animation;
    actor->unk2E = 1;
    actor->unk27 = 0;
    actor->unk24 = 1;
    state = &states[actor->object_index];
    state->movement.word &= ~0x1800;
    field_restart_actor_animation(actor);
    return 0;
}

/**
 * @brief Convert the facing of the actor with @p key to an angle byte.
 * @param key Object key to look up.
 * @return The facing angle (0x00-0xE0 in steps of 0x20), the mirror bit for two-direction actors, or -1 when absent.
 */
s32 func_8008B288(s32 key)
{
    FieldActor* actor;
    s32 result;
    s32 animation;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (!(g_field_resource_entries[actor->resource_index].flags & 1))
    {
        animation = actor->animation;
        if ((animation & 0x7F) < 15)
        {
            switch (animation)
            {
            case 1:
            case 6:
            case 11:
                result = 0x60;
                break;
            case 2:
            case 7:
            case 12:
                result = 0x80;
                break;
            case 3:
            case 8:
            case 13:
                result = 0xA0;
                break;
            case 4:
            case 9:
            case 14:
            case 132:
            case 137:
            case 142:
                result = 0xC0;
                break;
            case 129:
            case 134:
            case 139:
                result = 0x20;
                break;
            case 130:
            case 135:
            case 140:
                result = 0;
                break;
            case 131:
            case 136:
            case 141:
                result = 0xE0;
                break;
            case 0:
            case 5:
            case 10:
            case 128:
            case 133:
            case 138:
            default:
                result = 0x40;
                break;
            }
        }
        else
        {
            result = 0x40;
        }
    }
    else
    {
        result = actor->animation & 0x80;
    }
    return result;
}

/**
 * @brief Read the binding state of the actor with @p key.
 * @param key Object key to look up.
 * @return The state of the actor's animation binding, or -1 when absent.
 */
s32 func_8008B398(s32 key)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    return field_actor_binding(actor)->state;
}

/**
 * @brief Start an animation actor for the actor with @p key in one of its own slots.
 * @param key Object key to look up.
 * @param resource_index Resource of the animation actor.
 * @return -1 when absent, 0 when the animation started, 1 otherwise.
 */
s32 func_8008B42C(s32 key, s32 resource_index)
{
    FieldActor* actor;
    s32 slot_index;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    slot_index = func_800839F8(actor->object_index, 0);
    if ((slot_index != -1) && (func_80083EEC(actor->object_index, slot_index, resource_index) != 0))
    {
        field_start_actor_animation(slot_index, 0, 0);
        return 0;
    }
    return 1;
}

/**
 * @brief Start an animation actor for the actor with @p key in a free shared slot.
 * @param key Object key to look up.
 * @param resource_index Resource of the animation actor.
 * @return -1 when absent, 0 when the animation started, 1 otherwise.
 */
s32 func_8008B500(s32 key, s32 resource_index)
{
    FieldActor* actor;
    s32 slot_index;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    slot_index = func_800839F8(0, 0);
    if ((slot_index != -1) && (func_80083EEC(actor->object_index, slot_index, resource_index) != 0))
    {
        field_start_actor_animation(slot_index, 0, 0);
        return 0;
    }
    return 1;
}

/**
 * @brief Start an animation actor for the actor with @p key aimed at a list of targets.
 * @param key Object key to look up.
 * @param resource_index Resource of the animation actor.
 * @param target_keys Number of target lookups; each looks up @p key again.
 * @param unused Unused.
 * @return -1 when absent, 0 when the animation started, 1 otherwise.
 */
s32 func_8008B5D0(s32 key, s32 resource_index, s32 target_keys, s32* unused)
{
    FieldActor* actor;
    s32 i;
    s32 target_count;
    s32 slot_index;
    s32 targets[16];

    target_count = 0;
    for (i = 0; i < target_keys; i++)
    {
        actor = field_find_actor(key);
        if (actor != FIELD_ACTOR_NONE)
        {
            targets[target_count] = actor->object_index;
            target_count++;
        }
    }

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    slot_index = func_800839F8(actor->object_index, 0);
    if ((slot_index != -1) && (func_80083EEC(actor->object_index, slot_index, resource_index) != 0))
    {
        field_start_actor_animation(slot_index, target_count, (u8*)targets);
        return 0;
    }
    return 1;
}

/**
 * @brief Clear the pending animation restart flags of the three bindings.
 */
void func_8008B724(void)
{
    D_8010A020[2] = 0;
    D_8010A020[1] = 0;
    D_8010A020[0] = 0;
}

/**
 * @brief Restart the loaded animation actors whose restart is pending.
 */
void func_8008B73C(void)
{
    s32 i;
    s32 owner;

    for (i = 0; i < FIELD_ACTOR_BINDING_COUNT; i++)
    {
        if (D_8010A020[i] != 0 && g_field_actor_bindings[i].state == 2)
        {
            g_field_actor_slots[g_field_actor_bindings[i].slot].animation = g_field_actor_slots[g_field_actor_bindings[i].slot].default_animation;
            field_start_actor_animation(g_field_actor_bindings[i].slot, 0, 0);
            owner = g_field_actor_bindings[i].owner;
            if (owner >= 3)
            {
                owner = 2;
            }
            g_field_object_states[owner].contact.bytes.animation_actor_index = g_field_actor_bindings[i].slot;
            D_8010A020[i] = 0;
            g_field_actor_slots[g_field_actor_bindings[i].slot].unk2A = 1;
        }
    }
}
