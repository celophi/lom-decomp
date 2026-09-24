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
 * @see decomp.me (100%) TODO
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
 * @brief Field actor primary opcode dispatch (opcodes 0x00, 0x80..0xBC).
 *
 * Reads the current command byte from the actor's active command stream and
 * dispatches to the matching handler: animation/motion setup, spawn of child
 * actors (0xB9), path/target queries (0xB0/0xB1), resource loads, and the many
 * one-shot state transitions. Advances the actor's stream offset (unk2C) before
 * returning.
 *
 * @param arg0 Pointer to the field actor record (Struct_D800FDF58 layout).
 * @see decomp.me (100%)
 */
void func_80088198(FieldActor* actor)
{
    RECT sp10;
    FieldCollisionQuery sp18;
    FieldCollisionQuery sp30;
    FieldObjectPart* temp_v0_19;
    FieldObjectPart* casea5_base;
    FieldObjectPart* temp_v0_20;
    FieldObjectPart* casea6_base;
    s32 casea6_mask;
    FieldActorSlot* temp_v1_11;
    FieldActorSlot* temp_v1_9;
    FieldActor* var_s0;
    FieldActor* b9_actor_base;
    FieldObjectState* temp_v0_13;
    FieldObjectState* case83_base;
    FieldObjectState* case9e_states;
    u8* case9e_entries;
    FieldObjectState* casea9_states;
    FieldObjectState* casea9_state;
    FieldActorSlot* casea9_slots;
    FieldActorSlot* track_slots;
    u8* casea9_entries;
    u8* casea9_check_entries;
    s32 shared_s1;
    FieldObjectState* b9_state_base;
    FieldObjectState* temp_v0_14;
    FieldObjectState* temp_v0_16;
    FieldObjectState* temp_v0_17;
    FieldObjectState* temp_v0_3;
    FieldObjectState* temp_v0_4;
    FieldObjectState* temp_v0_5;
    FieldObjectState* temp_v0_6;
    FieldObjectState* temp_v0_7;
    FieldObjectState* temp_v0_8;
    FieldObjectState* temp_v1_2;
    FieldObjectState* temp_v1_6;
    FieldObjectState* var_s1;
    s16 var_v0;
    s16 var_v0_2;
    s32 offset_8d;
    s32 offset_query;
    s32 queryoff2;
    s32 queryoff3;
    s16 var_v1;
    u16 offset_88;
    s32* var_v1_4;
    s32 temp_a0_3;
    s32 temp_a0_4;
    s32 temp_a1_2;
    s32 temp_a1_5;
    s32 temp_a1_7;
    s32 temp_a2;
    s32 temp_a3_3;
    s32 temp_v0;
    s32 track_result;
    s32 temp_v0_11;
    s32 temp_v0_12;
    s32 temp_v1_5;
    s32 var_s2;
    s32 var_s6;
    s32 var_v0_3;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    u16* var_v1_3;
    u16 temp_a1_3;
    u16 temp_v1_3;
    u16 temp_v1_4;
    u8 temp_a0;
    u8 temp_a0_2;
    s32 temp_a1;
    u8 temp_a1_4;
    s32 temp_a1_6;
    s32 temp_a3;
    u8 temp_a3_2;
    s32 temp_s0_2;
    u8 temp_s5;
    u8 temp_v0_10;
    u8 temp_v0_2;
    s32 temp_v1;
    u8 temp_v1_8;
    u8 var_a1;
    u16* row;
    void* temp_s2;
    u8* script;
    void* temp_v1_10;
    FieldActor* call_actor;
    FieldActorSlot* case83_slots;
    u8* case83_entries;
    u8* bc_entries;
    u8* track_entries;
    FieldObjectState* update_states;
    s32 spawn_command;
    s32 parent_index;
    s32 advance_0;
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

    temp_a2 = 0x801ED400;

    if (actor->script_index == FIELD_SCRIPT_OBJECT)
    {
        script = g_field_object_states[actor->object_index].script;
    }
    else
    {
        script = (u8*)g_field_actor_scripts + g_field_actor_scripts[actor->script_index];
    }
    script += actor->script_offset;

    temp_a1 = script[0];

    switch (temp_a1)
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

        do
        {
            do
            {
                var_s2 = 0xC;
                var_s6 = 1;
                b9_state_base = g_field_object_states;
                var_s1 = b9_state_base + 12;
                b9_actor_base = g_field_actors;
                var_s0 = b9_actor_base + 12;
            loop_15:
                temp_s5 = var_s0->presence;
                if (temp_s5 == 0xFF)
                {
                    field_initialize_actor_record(var_s2, 3);
                    temp_a1_2 = var_s0->control.word & ~0x1FF;
                    var_s0->x = actor->x;
                    var_s0->y = actor->y;
                    temp_a1_2 |= 2;
                    var_s0->z = actor->z;
                    var_s0->presence = 0xFE;
                    spawn_command = script[1];
                    var_s0->unk27 = 0;
                    var_s0->unk24 = var_s6;
                    var_s0->command = 0xB8;
                    var_s0->animation = spawn_command;
                    parent_index = actor->object_index;
                    var_s0->unk3D = 3;
                    var_s0->control.word = temp_a1_2;
                    var_s0->script_index = 0;
                    var_s0->unk10 = var_s6;
                    var_s0->unk20 = parent_index;
                    var_s1->group_flags = 0;
                    var_s1->flags = 0;
                    var_s1->key = var_s2;
                    var_s1->unk18 = 0;
                    temp_v1_5 = var_s1->contact.word;
                    temp_v1_5 &= ~0x80;
                    temp_v1_5 &= ~1;
                    var_s1->contact.word = temp_v1_5;
                    field_restart_actor_animation(var_s0, temp_a1_2);
                    temp_v0 = func_800839F8(var_s2, 0);
                    if (temp_v0 != -1)
                    {
                        if (func_80083EEC(var_s2, temp_v0, 0xB0U) != 0)
                        {
                            field_start_actor_animation(temp_v0, 0, 0);
                        }
                    }
                    else
                    {
                        var_s0->presence = temp_s5;
                    }
                }
                else
                {
                    var_s1 -= 1;
                    var_s2 -= 1;
                    var_s0 -= 1;
                    if (var_s2 >= 3)
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
        temp_v0_2 = script[1];
        actor->unk2E = (u16)temp_v0_2;
        if (temp_v0_2 == 0)
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
        temp_v0_3 = &case83_base[actor->object_index];
        temp_v0_3->contact.word &= ~0x1C;
        actor->command = (s16)script[0];
        if ((u32)(temp_a1 - 0x83) < 2U)
        {
            actor->command = 0x85;
            case83_base[actor->object_index].action = temp_a1 + 0x7D;
            var_v0_2 = (u16)actor->script_offset + 1;
        }
        else
        {
            case83_base[actor->object_index].action = script[1];
            var_v0_2 = (u16)actor->script_offset + 2;
        }
        actor->script_offset = var_v0_2;
        temp_a0 = actor->object_index;
        if ((g_field_object_states[temp_a0].action == 2) && (actor->unk30 != 0) && (temp_a0 < 2U))
        {
            actor->animation = (actor->animation & 0x80) + (u16)((u8)actor->unk30 + 0x1F);
            if ((field_get_next_animation_frame_count(actor) == 0) || (actor->unk30 >= 5U))
            {
                g_field_object_states[actor->object_index].retry_count = 0;
                actor->unk30 = 0;
                temp_v0_4 = &g_field_object_states[actor->object_index];
                temp_v0_4->flags &= 0xFFFF7FFF;
                actor->animation &= 0x80;
                field_restart_actor_animation_reverse(actor);
                actor->command = 0x95;
                actor->unk20 = 0x14;
                return;
            }
            actor->animation = (actor->animation & 0x80) + 0x1F;
            temp_v0_5 = &g_field_object_states[actor->object_index];
            temp_v0_5->flags |= 0x8000;
            goto block_36;
        }
        g_field_object_states[actor->object_index].retry_count = 0;
        actor->unk30 = 0;
    block_36:
        precheck_states = g_field_object_states;
        temp_v1_2 = &precheck_states[actor->object_index];
        if (temp_v1_2->flags & 0x400)
        {
            actor->command = 0;
            return;
        }
        if (D_8010AE54 != 0 && (u32)(temp_v1_2->action - 4) < 4U)
        {
            actor->command = 0;
            return;
        }
        {

            do
            {
                resource_offset = actor->resource_index * 0x190;
                action_row = g_field_resource_actions + g_field_object_states[actor->object_index].action * 8;
                row = (u16*)(resource_offset + (s32)action_row);
                if (!(row[1] & 0x400))
                {
                    if ((row[0] != 0) || (row[2] != 0))
                    {
                        if (*(volatile u16*)&row[1] & 0x400)
                        {
                            goto block_43;
                        }
                        goto block_45;
                    }
                    goto block_57;
                }
            block_43:
                if ((field_object_has_active_actor_tracks(actor->object_index) == 0) && (field_count_free_actor_slots(actor->object_index) >= 3))
                {
                block_45:
                    if ((row[0] & 0x8000) && !(row[1] & 0x400))
                    {
                        if (((u8)actor->object_index < 3U) && (field_object_has_active_actor_tracks(actor->object_index) == 0) && (D_8010AE58 == 0) &&
                            (field_count_free_actor_slots(actor->object_index) >= 3))
                        {
                            temp_a0_2 = actor->object_index;
                            if ((g_field_object_states[temp_a0_2].unk48 == 0xFF) &&
                                (func_8008404C(temp_a0_2, (row[0] & 0x7FFF) + (u16)((g_field_player_records[temp_a0_2].type * 0x18) + 0x88)) != 0))
                            {
                                case83_slots = g_field_actor_slots;
                                case83_entries = g_field_actor_bindings;
                                if ((u8)actor->object_index >= 2U)
                                {
                                    var_v0_3 = 0x38;
                                }
                                else
                                {
                                    goto block_59;
                                }
                                goto block_61;
                            }
                        }
                        goto block_57;
                    }
                    temp_a1_3 = row[3];
                    if (temp_a1_3 & 0x8000)
                    {
                        if (func_8008404C(actor->object_index, temp_a1_3 & 0x3FF) == 0)
                        {
                        block_57:
                            actor->command = 0;
                            return;
                        }
                        case83_slots = g_field_actor_slots;
                        case83_entries = g_field_actor_bindings;
                        if ((u8)actor->object_index < 2U)
                        {
                        block_59:
                            var_v0_3 = actor->object_index * 0x1C;
                        }
                        else
                        {
                            var_v0_3 = 0x38;
                        }
                    block_61:
                        slot_offset_83 = (((FieldActorBinding*)&case83_entries[var_v0_3])->slot) * (s32)sizeof(FieldActorSlot);
                        ((FieldActorSlot*)((s32)case83_slots + slot_offset_83))->actor_type = g_field_object_states[actor->object_index].action;
                        goto block_62;
                    }
                block_62:
                    if (row[1] & 0x400)
                    {
                        update_states = g_field_object_states;
                        temp_v0_6 = &update_states[actor->object_index];
                        temp_v0_6->contact.word |= 0x40;
                        temp_v0_7 = &g_field_object_states[actor->object_index];
                        action_mask = ~0x400;
                        temp_v0_7->movement.word = temp_v0_7->movement.word & action_mask;
                        g_field_object_states[actor->object_index].unk4A = 0;
                        temp_v0_8 = &g_field_object_states[actor->object_index];
                        action_mask = ~1;
                        temp_v0_8->unk4C = temp_v0_8->unk4C & action_mask;
                        temp_v1_3 = row[2];
                        if ((temp_v1_3 != 0xFFFF) && (temp_v1_3 != 0))
                        {
                            g_field_object_states[actor->object_index].unk3C = (s32)row[2];
                        }
                    }
                    else if (!(row[0] & 0x8000))
                    {
                        temp_v1_4 = row[2];
                        if ((temp_v1_4 != 0xFFFF) && (temp_v1_4 != 0))
                        {
                            shared_s1 = func_800839F8((s32)actor->object_index, 0);
                            if ((shared_s1 != -1) && (func_80083EEC((s32)actor->object_index, shared_s1, row[2]) != 0))
                            {
                                temp_a3_2 = actor->object_index;
                                g_field_actor_slots[shared_s1].actor_type = g_field_object_states[temp_a3_2].action;
                                field_start_actor_animation(shared_s1, 0, 0);
                            }
                        }
                    }
                    func_8009D4D8(actor, (u8)row[1]);
                    return;
                }
                goto block_57;
            } while (0);
        }
        goto block_57;
    case 0xAD:
        actor->unk33 = 1;
        /* fallthrough */
    case 0x88:
        actor->command = (s16)script[0];
        temp_a1_4 = script[1];
        actor->unk1B = temp_a1_4;
        if (g_field_resource_entries[actor->resource_index].flags & 1)
        {
            actor->animation = g_field_actor_walk_animations[temp_a1_4 >> 5];
        }
        else
        {
            actor->animation = g_field_direction_animation_modes[temp_a1_4 >> 5] + ((actor->unk33 & 1) * 5) + 5;
        }
        call_actor = actor;
        var_a1 = script[2];
        actor->unk24 = 1;
        offset_88 = actor->script_offset;
        offset_88 += 3;
        actor->unk2E = (u16)var_a1;
        call_actor->script_offset = offset_88;
        field_restart_actor_animation(call_actor);
        return;
    case 0x8D:
        actor->command = (s16)script[0];
        temp_v0_10 = script[1] | (actor->animation & 0x80);
        actor->animation = temp_v0_10;
        if (temp_v0_10 & 0x80)
        {
            actor->unk1B = 0;
        }
        else
        {
            actor->unk1B = 0x80;
        }
        call_actor = actor;
        var_a1 = script[2];
        actor->unk24 = 1;
        offset_8d = actor->script_offset;
        offset_8d += 3;
        actor->unk2E = (u16)var_a1;
        call_actor->script_offset = offset_8d;
        field_restart_actor_animation(call_actor);
        return;
    case 0x8F:
        func_8008BF88(actor, script[1], script[2], script[3]);
        actor->script_offset = (u16)actor->script_offset + 4;
        return;
    case 0xB0:
    case 0xB1:
        temp_a1_5 = actor->x;
        if ((temp_a1_5 < 0) || (temp_a3_3 = ((FieldMapBounds*)temp_a2)->width << 8, ((temp_a1_5 < temp_a3_3) == 0)) ||
            (temp_v1_5 = actor->z, (temp_v1_5 < 0)) || (temp_a2 = (s32)(((FieldMapBounds*)temp_a2)->depth << 0x10) >> 7, ((temp_v1_5 < temp_a2) == 0)) ||
            (query_base = g_field_object_states, temp_v1_6 = &query_base[actor->object_index], temp_v0_11 = temp_v1_6->target_x, (temp_v0_11 < 0)) ||
            (temp_v0_11 >= temp_a3_3) || (temp_v0_12 = temp_v1_6->target_z, (temp_v0_12 < 0)) || (temp_v0_12 >= temp_a2))
        {
            call_actor = actor;
            g_field_object_states[actor->object_index].path_index = 0;
            temp_v0_13 = &g_field_object_states[actor->object_index];
            temp_v0_13->path_x = temp_v0_13->target_x;
            temp_v0_14 = &g_field_object_states[actor->object_index];
            temp_v0_14->path_z = (s32)temp_v0_14->target_z;
            var_a1 = 1;
            g_field_object_states[actor->object_index].path_length = 1;
            temp_a2 = script[0];
            call_actor->unk2E = 0xFF;
            call_actor->unk24 = 1;
            offset_query = (u16)call_actor->script_offset + 1;
            call_actor->command = temp_a2;
            call_actor->script_offset = offset_query;
            field_restart_actor_animation(call_actor);
            return;
        }
        else
        {
            sp18.x = temp_a1_5;
            sp18.y = actor->y;
            sp18.z = actor->z;
            if (g_field_object_parts[actor->object_index].footprint == 0x40)
            {
                sp18.width = 0xC;
                sp18.depth = 8;
                sp30.width = 0xC;
                sp30.depth = 8;
            }
            else
            {
                sp18.width = 9;
                sp18.depth = 6;
                sp30.width = 9;
                sp30.depth = 6;
            }
            sp18.height = 0x10;
            sp30.height = 0x10;
            func_8006304C(&sp18);
            sp30.x = g_field_object_states[actor->object_index].target_x;
            sp30.y = g_field_object_states[actor->object_index].target_y;
            sp30.z = g_field_object_states[actor->object_index].target_z;
            var_s2 = func_80060F58(&sp18, &sp30, &g_field_object_states[actor->object_index].path_x, 0);
            call_actor = actor;
            if (var_s2 <= 0)
            {
                g_field_object_states[actor->object_index].path_index = 0;
                temp_v0_16 = &g_field_object_states[actor->object_index];
                temp_v0_16->path_x = (s32)temp_v0_16->target_x;
                temp_v0_17 = &g_field_object_states[actor->object_index];
                temp_v0_17->path_z = (s32)temp_v0_17->target_z;
                var_a1 = 1;
                g_field_object_states[actor->object_index].path_length = 1;
                temp_a2 = script[0];
                call_actor->unk2E = 0xFF;
                call_actor->unk24 = 1;
                queryoff2 = call_actor->script_offset;
                queryoff2 += 1;
                call_actor->command = temp_a2;
                call_actor->script_offset = queryoff2;
                field_restart_actor_animation(call_actor);
                return;
            }
            else
            {
                g_field_object_states[actor->object_index].path_length = var_s2;
                g_field_object_states[actor->object_index].path_index = 0;
                var_a1 = script[0];
                temp_a2 = script[0];
                call_actor->unk2E = 0xFF;
                call_actor->unk24 = 1;
                queryoff3 = (u16)call_actor->script_offset + 1;
                call_actor->command = (s16)var_a1;
                call_actor->script_offset = queryoff3;
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
        case9e_entries = g_field_actor_bindings;
        if ((u8)actor->object_index < 2U)
        {
            var_v0_5 = actor->object_index * 0x1C;
        }
        else
        {
            var_v0_5 = 0x38;
        }
        if (((FieldActorBinding*)(case9e_entries + var_v0_5))->state == 0)
        {
            shared_s1 = script[1] + (script[2] << 8);
            temp_a1_6 = actor->object_index;
            actor->script_offset = (u16)actor->script_offset + 3;
            case9e_states = g_field_object_states;
            if (case9e_states[temp_a1_6].flags & 0x400)
            {
                goto block_154;
            }
            if (D_8010AE54 != 0)
            {
                goto block_155;
            }
            if (func_8008404C(temp_a1_6, shared_s1) == 0)
            {
                goto block_155;
            }
            case9e_states[actor->object_index].movement.word |= 0x8000;
            return;
        }
        break;
    case 0xA9:
        casea9_check_entries = g_field_actor_bindings;
        if ((u8)actor->object_index < 2U)
        {
            var_v0_6 = actor->object_index * 0x1C;
        }
        else
        {
            var_v0_6 = 0x38;
        }
        if (((FieldActorBinding*)(casea9_check_entries + var_v0_6))->state == 0)
        {
            temp_s0_2 = script[3];
            shared_s1 = script[1] + (script[2] << 8);
            actor->script_offset = (u16)actor->script_offset + 4;
            casea9_states = g_field_object_states;
            casea9_states[actor->object_index].unk3C = 0xFFFF;
            if (D_8010AE54 != 0)
            {
                goto block_154;
            }
            if (func_8008404C(actor->object_index, shared_s1) == 0)
            {
                goto block_155;
            }
            temp_v1_8 = actor->object_index;
            casea9_state = &casea9_states[temp_v1_8];
            casea9_slots = g_field_actor_slots;
            casea9_entries = g_field_actor_bindings;
            if (temp_v1_8 < 2U)
            {
                var_v0_7 = temp_v1_8 * 0x1C;
            }
            else
            {
                var_v0_7 = 0x38;
            }
            slot_offset_a9 = (((FieldActorBinding*)&casea9_entries[var_v0_7])->slot) * (s32)sizeof(FieldActorSlot);
            ((FieldActorSlot*)((s32)casea9_slots + slot_offset_a9))->actor_type = temp_s0_2;
            casea9_state->action = temp_s0_2;
            g_field_object_states[actor->object_index].movement.word |= 0x8000;
            return;
        }
        break;
    case 0xBC:
        shared_s1 = 2;
        if ((u8)actor->object_index < 2U)
        {
            shared_s1 = actor->object_index;
        }
        bc_entries = g_field_actor_bindings;
        temp_s2 = (shared_s1 * 0x1C) + bc_entries;
        temp_a0_3 = ((FieldActorBinding*)temp_s2)->state;
        if (((u32)(temp_a0_3 - 1) < 2U) && (((FieldActorBinding*)temp_s2)->owner == actor->object_index))
        {
            var_s6 = 1;
            if (temp_a0_3 != var_s6)
            {
                bc_slots = g_field_actor_slots;
                temp_v1_9 = &bc_slots[((FieldActorBinding*)temp_s2)->slot];
                if (temp_v1_9->track_mask == 0)
                {
                    temp_v1_9->animation = temp_v1_9->default_animation;
                    g_field_object_states[actor->object_index].contact.bytes.target_count = 0;
                    field_start_actor_animation(((FieldActorBinding*)temp_s2)->slot, 0, 0);
                    g_field_object_states[shared_s1].contact.bytes.animation_actor_index = (u8)((FieldActorBinding*)temp_s2)->slot;
                    g_field_actor_slots[((FieldActorBinding*)temp_s2)->slot].unk2A = var_s6;
                    actor->command = 0xBC;
                }
                goto block_167;
            }
        }
        else
        {
            goto block_167;
        }
        break;
    case 0x9F:
        shared_s1 = 2;
        if ((u8)actor->object_index < 2U)
        {
            shared_s1 = actor->object_index;
        }
        track_entries = g_field_actor_bindings;
        temp_v1_10 = (shared_s1 * 0x1C) + track_entries;
        temp_a0_4 = ((FieldActorBinding*)temp_v1_10)->state;
        if (((u32)(temp_a0_4 - 1) < 2U) && (track_owner = actor->object_index, temp_a1_7 = ((FieldActorBinding*)temp_v1_10)->owner, (temp_a1_7 == track_owner)))
        {
            if (temp_a0_4 != 1)
            {
                track_slots = g_field_actor_slots;
                temp_v1_11 = &track_slots[((FieldActorBinding*)temp_v1_10)->slot];
                if (temp_v1_11->track_mask == 0)
                {
                    var_s2 = 0;
                    if (temp_v1_11->default_animation->flags & 0x800)
                    {
                        temp_s0_2 = 0;
                        var_v1_3 = temp_v1_11->tracks;
                    loop_count_tracks:
                    {
                        if (*var_v1_3 != 0)
                        {
                            var_s2 += 1;
                        }
                        temp_s0_2 += 1;
                        var_v1_3 += 1;
                        if (temp_s0_2 < 3)
                        {
                            goto loop_count_tracks;
                        }
                    }
                        track_result = field_start_bound_action_animation(actor->object_index, 0, 0, ((var_s2 - 1) << 0xC) | 0x4400);
                    }
                    else
                    {
                        track_result = field_start_bound_action_animation(temp_a1_7, 0, 0, 0);
                    }
                    if (track_result != 0)
                    {
                        actor->command = 0xBC;
                        g_field_object_states[actor->object_index].contact.bytes.target_count = 0;
                    }
                }
                goto block_140;
            }
        }
        else
        {
        block_140:
            var_v0 = (u16)actor->script_offset + 2;
            actor->script_offset = var_v0;
            return;
        }
        break;
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
        temp_v1 = script[0];
        alias->unk2E = 0xFA;
        value = temp_v1;
        alias->command = value;
        value = script[1];
        alias->unk1B = value;
        states = g_field_object_states;
        value = script[2];
        alias->unk20 = value;
        states[alias->object_index].unk171 = script[3];
        var_a1 = script[4];
        var_v1 = alias->script_offset;
        alias->unk2E = 0xF0;
        alias->unk24 = 1;
        var_v1 += 5;
        alias->animation = var_a1;
        call_actor = alias;
        call_actor->script_offset = var_v1;
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
        sp10.x = script[0] + (script[1] << 8);
        sp10.y = (s16)script[2];
        sp10.w = (s16)script[3];
        sp10.h = (s16)script[4];
        func_8008A4D0(actor, &sp10, script[5] | (script[6] << 8), script[7]);
        return;
    case 0xA1:
        shared_s1 = script[1] + (script[2] << 8);
        actor->script_offset = (u16)actor->script_offset + 3;
        if (D_8010AE54 != 0)
        {
            goto block_154;
        }
        if (func_8008404C(actor->object_index, shared_s1) == 0)
        {
            goto block_155;
        }
        var_v1_4 = D_8010A020;
        if ((u8)actor->object_index < 2U)
        {
            var_v1_4 += actor->object_index;
        }
        else
        {
            var_v1_4 += 2;
        }
        *var_v1_4 = 1;
        return;
    block_154:
    block_155:
        actor->script_index = 0xFF;
        actor->unk10 = 0;
        return;
    case 0xA4:
        temp_a1 = 1;
        if ((u8)actor->object_index < 2U)
        {
            decoded_a4 = script[0];
            actor->unk2E = temp_a1;
            actor->command = decoded_a4;
            a4_old_flags = actor->animation;
            a4_command_flags = script[1];
            actor->unk24 = temp_a1;
            actor->animation = a4_command_flags | (a4_old_flags & 0x80);
            field_restart_actor_animation(actor);
        }
        actor->script_offset = (u16)actor->script_offset + 2;
        return;
    case 0xA5:
        casea5_base = g_field_object_parts;
        temp_v0_19 = &casea5_base[actor->object_index];
        temp_v0_19->flags |= 0x800000;
        goto block_167;
    case 0xA6:
        casea6_mask = 0xFF7FFFFF;
        casea6_base = g_field_object_parts;
        temp_v0_20 = &casea6_base[actor->object_index];
        temp_v0_20->flags &= casea6_mask;
        goto block_167;
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
        var_v0 = (u16)actor->script_offset + 1;
        actor->command = decoded_b2;
        actor->script_offset = var_v0;
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
        goto block_167;
    case 0x0:
    block_167:
        actor->script_offset++;
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
 * @see decomp.me (100%)
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
        return (s32)actor;
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
    contact_state->contact.word = contact_state->contact.word & ~0x20;
    hp_state = &g_field_object_states[actor->object_index];
    hp_state->unk8.word = (hp_state->unk8.word & 0xFF000000) | (hp_state->unk0 & 0xFFFFFF);
    hp_state->unk4.word = hp_state->unk0 & 0xFFFFFF;
    g_field_object_states[actor->object_index].tint_timer = 60;
    movement_state = &g_field_object_states[actor->object_index];
    movement_state->movement.word = movement_state->movement.word | 0x8000;
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
    if ((x < 0) || (map_width = bounds->width << 8, ((x < map_width) == 0)) || (z = actor->z, (z < 0)) ||
        (map_depth = (s32)(bounds->depth << 0x10) >> 7, ((z < map_depth) == 0)) ||
        (state_x = g_field_object_states[actor->object_index].target_x, (state_x < 0)) || (state_x >= map_width) ||
        (state_z = g_field_object_states[actor->object_index].target_z, (state_z < 0)) || (state_z >= map_depth))
    {
        (&g_field_object_states[actor->object_index])->path_index = 0;
        (&g_field_object_states[actor->object_index])->path_x = g_field_actors[target_index].x;
        (&g_field_object_states[actor->object_index])->path_z = g_field_actors[target_index].z;
        g_field_object_states[actor->object_index].path_length = 1;
    }
    else
    {
        start.x = x;
        start.y = actor->y;
        start.z = actor->z;
        if ((&g_field_object_parts[actor->object_index])->footprint == 0x40)
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
        (&D_80105B30[actor->object_index])->target_y = target->y;
        (&D_80105B30[actor->object_index])->target_z = target->z;
        goal.x = target->x;
        goal.y = target->y;
        goal.z = target->z;
        path_length = func_80060F58(&start, &goal, &D_80105B30[actor->object_index].path_x, 0);
        if (path_length <= 0)
        {
            (&FIELD_OBJECT_STATES_AT_TARGET[actor->object_index])->path_index = 0;
            (&FIELD_OBJECT_STATES_AT_TARGET[actor->object_index])->path_x = target->x;
            (&FIELD_OBJECT_STATES_AT_TARGET[actor->object_index])->path_z = target->z;
            FIELD_OBJECT_STATES_AT_TARGET[actor->object_index].path_length = 1;
        }
        else
        {
            (&FIELD_OBJECT_STATES_AT_TARGET[actor->object_index])->path_length = path_length;
            (&FIELD_OBJECT_STATES_AT_TARGET[actor->object_index])->path_index = 0;
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
 * @see decomp.me (100%) TODO
 */
s32 func_8008A580(s32 key, s32 resource_id)
{
    FieldActor* actor;
    s32* flag;
    s32* flags;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return (s32)actor;
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
    FieldObjectState* target;
    FieldObjectState* first_target;
    s32 i;
    s32 index8;

    states = g_field_object_states;
    index8 = object_index * 8;
    source = (FieldObjectState*)((u8*)states + ((index8 + object_index) * 16 - object_index) * 4);
    i = 0;
    if (source->contact.bytes.target_count != 0)
    {
        loop_states = states;
        saved_source = source;
        index8 = object_index * 8;
        do
        {
            current = (FieldObjectState*)((((index8 + object_index) * 16 - object_index) * 4) + (u32)loop_states);
            first_target = (FieldObjectState*)(current->targets[i] * 0x23C + (u32)loop_states);
            first_target->contact.word &= ~0x80;
            target = (FieldObjectState*)(current->targets[i] * 0x23C + (u32)loop_states);
            if (!((target->contact.word >> 5) & 1))
            {
                index8 = object_index * 8;
                if (target->unk4.word != 0)
                {
                    request.source_key = current->key;
                    if ((u8)current->action < 0xB)
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
                    goto stride_update;
                }
            }
            else
            {
            stride_update:
                index8 = object_index * 8;
            }
            i++;
        } while (i < ((FieldObjectState*)((u8*)loop_states + (((index8 + object_index) * 16 - object_index) * 4)))->contact.bytes.target_count);
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
    target->contact.word = target->contact.word & ~0x80;
    if (target->unk4.word == 0)
    {
        return 0;
    }
    source = &states[source_index];
    request.source_key = source->key;
    if ((u8)source->action < 0xB)
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
        target->contact.word = target->contact.word & ~0x80;
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
    FieldObjectState* state;
    u8* states;

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
    states = (u8*)g_field_object_states;
    state = (FieldObjectState*)(states + actor->object_index * 0x23C);
    state->flags = state->flags | 0x10000000;
    if ((u8)actor->object_index < 3U)
    {
        g_field_player_records[actor->object_index].hit_state = 5;
    }
    else if (((FieldObjectState*)(states + actor->object_index * 0x23C))->unk8.word < 0)
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
        switch (((FieldObjectState*)((u8*)g_field_object_states + actor->object_index * 0x23C))->action)
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
            if (!((((FieldObjectState*)((u8*)g_field_object_states + actor->object_index * 0x23C))->contact.word >> 6) & 1))
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

    i = 0;
    target_count = i;
    for (; i < target_keys; i++)
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
 * @see decomp.me (100%)
 */
void func_8008B73C(void)
{
    s32 i;
    s32* flag;
    u8* binding;
    s32 index8;
    s32 index;
    u8* slot;
    s32 slot_offset;
    u8* slots;
    u8* states;

    i = 0;
    slots = (u8*)g_field_actor_slots;
    states = (u8*)g_field_object_states;
    flag = D_8010A020;
    binding = (u8*)g_field_actor_bindings;
restart_slots:
{
    if (*flag != 0 && *(s32*)(binding + 0x0) == 2)
    {
        slot_offset = *(s32*)(binding + 0x18) * 0x244;
        *(s32*)(slot_offset + (u32)slots + 0xC) = *(s32*)(slot_offset + (u32)slots + 0x10);
        field_start_actor_animation(*(s32*)(binding + 0x18), 0, 0);
        index = *(s32*)(binding + 0xC);
        if (index >= 3)
        {
            index = 2;
        }
        index8 = index * 8;
        (states + (((index8 + index) * 0x10) - index) * 4)[0x179] = *(u8*)(binding + 0x18);
        *flag = 0;
        slot = (u8*)(*(s32*)(binding + 0x18) * 0x244 + (u32)slots);
        slot[0x2A] = 1;
    }
    flag += 1;
    i += 1;
    binding += 0x1C;
}
    if (i < 3)
    {
        goto restart_slots;
    }
}
