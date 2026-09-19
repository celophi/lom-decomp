/**
 * @file field_effect_update.c
 * @brief Spawn, update, position, and retire field actor effects.
 */

#include "common.h"
#include "field_types.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define FIELD_EFFECT_ORIGIN_ADDRESS 0x1F800000
#define FIELD_EFFECT_VECTOR_ADDRESS 0x1F800010
#define FIELD_EFFECT_TARGET_ADDRESS 0x1F800020
#define FIELD_EFFECT_LOCAL_VECTOR_ADDRESS 0x1F800030
#define FIELD_EFFECT_ROTATED_VECTOR_ADDRESS 0x1F800038
#define FIELD_EFFECT_MATRIX_ADDRESS 0x1F800040
#define FIELD_EFFECT_MOVER_ADDRESS 0x1F800080
#define FIELD_EFFECT_QUERY_ADDRESS 0x1F8000C0

/** @brief Loaded field resource slot used while spawning effects. */
typedef struct
{
    u8 *start;
    u8 *end;
    u8 unknown_0x08;
    u8 slot_index;
    u8 padA[0xE - 0xA];
    s16 unknown_0x0e;
    u32 flags;
} FieldResourceEntry;

extern FieldResourceEntry g_field_resource_entries[];


/** @brief Private spawn-time view of a field motion record. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u32 unknown_0xc;
    s16 rotation_x;
    s16 heading;
    s16 pitch;
    s16 unknown_0x16;
    u8 unknown_0x18;
    u8 unknown_0x19;
    u8 unknown_0x1a;
    u8 position_source;
    union { s32 word; u16 half[2]; struct { unsigned low:13; unsigned placement:2; unsigned bit15:1; unsigned group:2; unsigned bit18:1; unsigned kind:4; unsigned high:9; } bits; } flags;
    u8 position_data;
    u8 facing_or_reward_kind;
    u8 actor_index;
    u8 part_index;
    u8 unknown_0x24;
    u8 state;
    s8 height_or_retired_state;
    u8 saved_state;
    u8 lifetime;
    u8 track_index;
    s16 motion_parameter;
    s16 age;
    u16 motion_scale;
    s16 reference_index;
    u8 rotation_z_16;
    u8 rotation_y_16;
    u8 unknown_0x34;
    u8 unknown_0x35;
    u8 unknown_0x36;
    u8 unknown_0x37;
    u8 unknown_0x38;
    u8 path_group;
    u8 source_object_index;
    u8 unknown_0x3b;
    u8 sprite_height_minus_one;
    u8 previous_effect_index;
    u8 pad3E[0x40 - 0x3E];
    s32 unknown_0x40;
    u32 work_x;
    u32 work_y;
    u32 work_z;
    u8 pad50[0x54 - 0x50];
} FieldSpawnMotionRecord;

/** @brief Object placement view used while resolving effect spawn positions. */
typedef struct
{
    u8 pad0[0xC];
    u32 object_flags;
    u8 pad10[0x60 - 0x10];
    u8 counters[16];
    u8 pad70[0x130 - 0x70];
    Vec2s attachment_points[4];
    s16 bounds_left;
    s16 bounds_top;
    s16 bounds_right;
    s16 bounds_bottom;
    u8 pad148[0x178 - 0x148];
    union { u32 word; u8 bytes[4]; } state_flags;
    u8 pad17C[0x18E - 0x17C];
    u8 unknown_0x18e;
    u8 pad18F[0x190 - 0x18F];
    Vec2s ground_attachment_points[3];
    s32 unknown_0x19c;
    s32 unknown_0x1a0;
    u8 pad1A4[0x1A8 - 0x1A4];
    u8 unknown_0x1a8;
    u8 unknown_0x1a9;
    u8 unknown_0x1aa;
    u8 pad1AB[0x23C - 0x1AB];
} FieldSpawnObjectPlacement;

/** @brief Packed part definition used by the spawn routine. */
typedef struct
{
    u32 track_flags;
    u32 behavior_flags;
    u8 unknown_0x8;
    u8 unknown_0x9;
    u8 unknown_0xa;
    u8 unknown_0xb;
    u8 unknown_0xc;
    u8 unknown_0xd;
    u8 unknown_0xe;
    u8 unknown_0xf;
    u8 unknown_0x10;
    u8 turn_end_age;
    u8 pad12[0x14 - 0x12];
    u32 orientation_flags;
    s16 unknown_0x18;
    u8 unknown_0x1a;
    u8 pad1B;
    u32 unknown_0x1c;
    u8 unknown_0x20;
    u8 rotation_z_track;
    u8 rotation_y_track;
    u8 unknown_0x23;
    u32 effect_flags;
    u32 placement_flags;
    u8 unknown_0x2c;
    u8 palette_selector;
    u8 footprint_scale_x;
    u8 unknown_0x2f;
    u8 pitch_acceleration;
    u8 unknown_0x31;
    u8 unknown_0x32;
    u8 footprint_scale_y;
    u32 spawn_flags;
    s16 offset_x;
    s16 offset_y;
    s16 offset_z;
    s16 pad3E;
    s16 unknown_0x40;
    s16 unknown_0x42;
    s16 unknown_0x44;
    s16 unknown_0x46;
} FieldSpawnPartDef;

/** @brief Actor animation selectors needed while spawning effects. */
typedef struct FieldSpawnAnimationDef
{
    u8 unknown_0x0[2];
    u8 pad2[0xC - 2];
    u16 unknown_0xc;
    u16 unknown_0xe;
    u16 palette_animation;
    u8 pad10[0x14 - 0x12];
    u8 hit_test_mode;
    u8 hit_test_part;
    u8 pad16[0x18 - 0x16];
    u16 sync_flags;
} FieldSpawnAnimationDef;

/** @brief Actor state view used by the spawn routine. */
typedef struct
{
    FieldSpawnPartDef* parts;
    u8 pad4[0xC - 4];
    FieldSpawnAnimationDef* animation;
    u8 pad10[0x14 - 0x10];
    u8 *track_data;
    u8 pad18[0x24 - 0x18];
    u8 is_active;
    u8 part_count;
    u8 hit_reaction;
    u8 unknown_0x27;
    u8 unknown_0x28;
    u8 unknown_0x29;
    u8 unknown_0x2a;
    u8 unknown_0x2b[16];
    u8 active_counts[9][16];
    u8 padCB;
    u16 track_counters[9][16];
    u16 track_ages[9];
    Vec2s track_offsets[9];
    u16 unknown_0x222;
    u32 action_flags;
    u8 owner_object_index;
    u8 track_object_indices[9];
    u8 track_count;
    u8 actor_index;
    u16 unknown_0x234;
    u16 unknown_0x236;
    u8 pad238[2];
    u8 active_track_mask;
    u8 unknown_0x23b;
    u8 pad23C[0x240 - 0x23C];
    u16* unknown_0x240;
} FieldSpawnActorState;

/** @brief Short vector stored in scratchpad during spawn transforms. */
typedef struct
{
    s16 x;
    s16 y;
    s16 z;
    s16 pad;
} FieldSpawnVector;

/** @brief Track binding state used while spawning actor effects. */
typedef struct
{
    u32 state;
    u8 pad4[0xC - 4];
    u32 track_index;
    u8 pad10[0x1C - 0x10];
} FieldSpawnTrackBinding;

extern FieldSpawnPartDef D_800FE3A0[];
extern FieldSpawnTrackBinding D_80105880[];
extern FieldVector D_80105778;
extern s32 g_field_action_context;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern s32 D_80105760;
extern s32 D_80105770;
extern u8 *D_801058D4;
extern s32 g_field_track_index;


/**
 * @brief Spawn an effect record for one actor part and resolve its initial placement.
 * @param actor Actor spawning the effect.
 * @param part_index Part definition to instantiate.
 * @param start First effect slot to inspect when searching for a related effect.
 * @return Spawned effect index, or -1 when the effect cannot be created.
 */
s32 func_8006D79C(FieldSpawnActorState* actor, s32 part_index, s32 start)
{
    extern FieldSpawnMotionRecord D_800FDF58[];
    extern FieldSpawnMotionRecord g_field_effect_records[];
    extern FieldSpawnObjectPlacement D_80105AE0[];
    extern FieldSpawnActorState g_field_actor_slots[];
    s32 half_turn_8bit;
    FieldVector* direction_vector = (FieldVector*) FIELD_EFFECT_ORIGIN_ADDRESS;
    FieldVector* squared_vector = (FieldVector*) FIELD_EFFECT_VECTOR_ADDRESS;
    FieldSpawnVector* local_direction = (FieldSpawnVector*) FIELD_EFFECT_LOCAL_VECTOR_ADDRESS;
    FieldMatrix* rotation_matrix = (FieldMatrix*) FIELD_EFFECT_MATRIX_ADDRESS;
    FieldSpawnMotionRecord* effect;
    FieldSpawnMotionRecord* sibling_effect;
    FieldSpawnMotionRecord* free_effect;
    FieldSpawnObjectPlacement* placement_object;
    FieldSpawnObjectPlacement* owner_object;
    FieldSpawnObjectPlacement* object_table_init;
    FieldSpawnObjectPlacement* owner_source_object;
    FieldSpawnObjectPlacement* owner_placement_state;
    FieldSpawnObjectPlacement* track_source_object;
    FieldSpawnObjectPlacement* owner_placement_guard;
    FieldSpawnObjectPlacement* attached_source_object;
    FieldSpawnObjectPlacement* object_table;
    FieldSpawnObjectPlacement* source_owner_object;
    FieldSpawnMotionRecord* source_record;
    FieldSpawnMotionRecord* record_base;
    FieldSpawnObjectPlacement* owner_object_state;
    FieldSpawnObjectPlacement* track_object_state;
    FieldSpawnPartDef* part;
    s32 rotated_x;
    s32 work_value;
    s32 extent;
    s32 effect_index;
    s32 work_index;
    s32 object_index;
    s32 work_limit;
    s32 direction_z;
    s32 placement_kind;
    s32 work_a;
    s32 sibling_index;
    s32 sibling_part_index;
    u8 track_object_index;
    u8 *resource;

    effect_index = 0;
    work_value = 0xFF;
    record_base = g_field_effect_records;
    free_effect = record_base;
find_slot:
    if (free_effect->state != work_value)
    {
        effect_index++;
        free_effect++;
        if (effect_index < 0x100)
        {
            goto find_slot;
        }
    }
    if (effect_index == 0x100)
    {
        g_field_action_context = 0x10101010;
        return -1;
    }

    effect = (FieldSpawnMotionRecord*)((u32)(effect_index * sizeof(*effect)) + (u32)record_base);
    part = &actor->parts[part_index];
    if ((s32)part->effect_flags < 0)
    {
        effect->previous_effect_index = work_value;
    }

    if ((u32)(((part->placement_flags >> 18) & 0x3F) - 0x2A) < 8U &&
        (((part->placement_flags >> 18) & 0x3F) - 0x22) == part_index)
    {
        return -1;
    }
    if ((u32)(((part->placement_flags >> 18) & 0x3F) - 0x14) < 8U &&
        (((part->placement_flags >> 18) & 0x3F) - 0x14) == part_index)
    {
        return -1;
    }
    if ((u32)(((part->placement_flags >> 18) & 0x3F) - 0x37) < 8U &&
        (((part->placement_flags >> 18) & 0x3F) - 0x37) == part_index)
    {
        return -1;
    }

    if (part->orientation_flags & 0xF0)
    {
        s32 parameter = ((u16 *)&part->effect_flags)[1];
        effect->position_source = parameter & 0xF;
    }
    else
    {
        effect->position_source = (part->behavior_flags >> 8) & 7;
    }
    effect->flags.word = (effect->flags.word & 0xF8FFFFFF) | (((part->behavior_flags >> 13) & 7) << 24);
    effect->flags.word = (effect->flags.word & ~0x600) | ((part->unknown_0x20 >> 6) << 9);
    effect->lifetime = part->unknown_0xd;
    effect->motion_parameter = ((u16*)&part->orientation_flags)[1];
    effect->motion_scale = part->unknown_0x18;
    effect->track_index = g_field_track_index;
    effect->flags.word = ((effect->flags.word & 0x9FFFFFFF) | ((*(u8*)&part->orientation_flags & 3) << 29)) & ~0x1000;
    effect->flags.word = (effect->flags.word & 0xF7FFFFFF) | (((part->spawn_flags >> 18) & 1) << 27);
    effect->flags.word &= ~0x6000;
    effect->flags.word &= 0xFFFBFFFF;
    half_turn_8bit = 128;
    effect->flags.word &= 0xFF87FFFF;
    if (part->effect_flags & 0x800000)
    {
        s32 eval = field_evaluate_parameter_track_at_time(actor, (part->effect_flags >> 25) & 0xF, 0) != 0;
        effect->flags.word = (effect->flags.word & 0xFF7FFFFF) | (eval << 23);
        goto bit23_done;
    scan_slot_found:
        effect->position_data = work_index;
        goto scan_slots_done;
    }
    else
    {
        effect->flags.word = (effect->flags.word & 0xFF7FFFFF) | (((part->behavior_flags >> 1) & 1) << 23);
    }
bit23_done:
    effect->flags.word = effect->flags.word & 0xFFFCFFFF;
    if (effect->position_source == 8)
    {
        s32 scan_ff;
        work_index = 0;
        work_limit = work_index;
        scan_ff = 0xFF;
        work_value = actor->active_counts[g_field_track_index][part_index];
        sibling_effect = g_field_effect_records;
    scan_slots:
        if (sibling_effect->state != scan_ff && sibling_effect->part_index == part->unknown_0x46 && sibling_effect->actor_index == actor->actor_index)
        {
            work_limit = 1;
            if (work_value == 0)
            {
                goto scan_slot_found;
            }
            effect->position_data = work_index;
            work_value--;
        }
        work_index++;
        sibling_effect++;
        if (work_index < 0x100)
        {
            goto scan_slots;
        }
    scan_slots_done:
        if (work_limit == 0)
        {
            effect->state = 0xFF;
            return -1;
        }
    }

    {
        s32 record_type = part->unknown_0xb;
        effect->saved_state = 0;
        effect->unknown_0x34 = 0;
        effect->state = record_type;
    }
    effect->work_x = part->unknown_0x40 << 8;
    effect->work_y = part->unknown_0x42 << 8;
    effect->work_z = part->unknown_0x44 << 8;
    if (((part->placement_flags >> 8) & 1) && (*(u32*)&part->unknown_0x2c & 0x0F000000))
    {
        if (part->spawn_flags & 0x10000)
        {
            s32 limit = (*(u32*)&part->unknown_0x2c >> 24) & 0xF;
            work_index = 0;
            if (limit != 0)
            {
                work_limit = limit;
                object_table_init = D_80105AE0;
                do
                {
                    owner_object = &object_table_init[actor->owner_object_index];
                    if (owner_object->counters[work_index] == 0)
                    {
                        work_index++;
                    }
                    else
                    {
                        owner_object->counters[work_index] = owner_object->counters[work_index] - 1;
                        effect->facing_or_reward_kind = part->unknown_0x1a + work_index;
                        break;
                    }
                } while (work_index < work_limit);
            }
            if (work_index == (part->unknown_0x2f & 0xF))
            {
                effect->state = 0xFF;
                return -1;
            }
        }
        else
        {
            effect->facing_or_reward_kind = part->unknown_0x1a + (((part->unknown_0x2f & 0xF) * rand()) >> 15);
        }
    }
    else
    {
        effect->facing_or_reward_kind = part->unknown_0x1a;
    }

    switch ((s32)((part->placement_flags >> 26) & 3))
    {
    case 0:
        effect->rotation_z_16 = part->rotation_z_track;
        break;
    case 1:
        effect->rotation_z_16 = field_evaluate_parameter_track(actor, part->rotation_z_track & 0xF);
        break;
    case 2:
        effect->rotation_z_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_z_track & 0xF, 0);
        break;
    }
    switch ((s32)((part->placement_flags >> 28) & 3))
    {
    case 0:
        effect->rotation_y_16 = part->rotation_y_track;
        break;
    case 1:
        effect->rotation_y_16 = field_evaluate_parameter_track(actor, part->rotation_y_track & 0xF);
        break;
    case 2:
        effect->rotation_y_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_y_track & 0xF, 0);
        break;
    }
    if ((((part->placement_flags >> 10) & 1) || (part->spawn_flags & 0x08000000)) && effect->position_source == 0 &&
        !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80))
    {
        u8 angle;
        u8 original_angle;

        original_angle = effect->rotation_y_16;
        angle = original_angle;
        if (angle >= 64 && original_angle < 128)
        {
            effect->rotation_y_16 = 128 - original_angle;
        }
        else
        {
            original_angle -= half_turn_8bit;
            effect->rotation_y_16 = -original_angle;
        }
    }
    effect->age = 0;
    actor->active_counts[g_field_track_index][part_index]++;
    effect->saved_state = 0;
    effect->part_index = part_index;
    effect->actor_index = actor->actor_index;
    if ((part->placement_flags >> 25) & 1)
    {
        if ((part->behavior_flags >> 12) & 1)
        {
            effect->unknown_0x18 = field_evaluate_parameter_track(actor, (part->behavior_flags >> 16) & 0xF);
            effect->unknown_0x19 = field_evaluate_parameter_track(actor, (((((u16*)&part->behavior_flags)[1] & 0xF) + 1) & 0xF));
            effect->unknown_0x1a = field_evaluate_parameter_track(actor, (((((u16*)&part->behavior_flags)[1] & 0xF) + 2) & 0xF));
        }
        else
        {
            effect->unknown_0x18 = effect->unknown_0x19 = effect->unknown_0x1a =
                field_evaluate_parameter_track(actor, (part->behavior_flags >> 16) & 0xF);
        }
    }
    effect->flags.word = (effect->flags.word & 0xFFFF7FFF) | ((part->behavior_flags * 0x10) & 0x8000);
    effect->flags.word = (effect->flags.word & 0xEFFFFFFF) | (((part->placement_flags >> 25) & 1) << 28);
    func_80070CB8(actor, part, effect);
    RotMatrix_gte((FieldSpawnVector*)&effect->rotation_x, rotation_matrix);
    RotMatrixZ(effect->rotation_z_16 * 16, rotation_matrix);
    RotMatrixY(effect->rotation_y_16 * 16, rotation_matrix);
    local_direction->x = 0;
    local_direction->y = -0x1000;
    local_direction->z = 0;
    gte_SetRotMatrix(rotation_matrix);
    gte_ldv0(local_direction);
    gte_rtv0();
    gte_stlvnl(direction_vector);
    if (((part->behavior_flags >> 2) & 1) || effect->position_source != 0)
    {
        effect->heading = ratan2(-direction_vector->vz, direction_vector->vx);
        gte_ldlvl(direction_vector);
        gte_sqr0();
        gte_stlvnl(squared_vector);
        effect->pitch = ratan2(SquareRoot0(squared_vector->vx + squared_vector->vz), -direction_vector->vy);
        effect->rotation_x = 0;
        if (effect->pitch < 0)
        {
            effect->pitch = -effect->pitch;
        }
    }
    rotated_x = direction_vector->vx;
    work_index = func_8007E754(actor, part);
    effect->flags.word = (effect->flags.word & ~0x1FF) | (work_index & 0x1FF);
    effect->x = (work_index * direction_vector->vx) >> 4;
    effect->y = (work_index * direction_vector->vy) >> 4;
    effect->z = (work_index * direction_vector->vz) >> 4;
    effect->unknown_0x24 = part->unknown_0x8;
    effect->source_object_index = actor->owner_object_index;
    if (effect->state == 0)
    {
        resource = D_801058D4;
        goto call_res;
    }
    if (effect->state == 1)
    {
        resource = g_field_actor_slots[effect->actor_index].track_data;
        if (resource != 0)
        {
        call_res:
            field_begin_actor_animation_forward(effect, resource);
            goto after_source;
        }
    }
    if (effect->state == 2)
    {
        { FieldSpawnObjectPlacement *owner_slots = D_80105AE0;
          u8 owner_index = actor->owner_object_index;
        owner_object_state = &owner_slots[owner_index]; }
        if (owner_object_state->state_flags.bytes[0] & 1)
        {
            if (owner_object_state->state_flags.bytes[2] != actor->actor_index)
            {
                goto kill_rec;
            }
        }
        record_base = &D_800FDF58[*(u8*)&actor->owner_object_index];
        if (!((part->behavior_flags >> 11) & 1) && !((part->placement_flags >> 25) & 1) && (part->unknown_0x2c >> 5) == 0 &&
            (*(u32*)&part->unknown_0xc & 0xFFFF0000) == 0x80800000 && part->unknown_0x10 == 0x80)
        {
            effect->flags.word |= 0x10008000;
            effect->unknown_0x18 = D_800FE3A0[record_base->source_object_index].unknown_0xe;
            effect->unknown_0x19 = D_800FE3A0[record_base->source_object_index].unknown_0xf;
            effect->unknown_0x1a = D_800FE3A0[record_base->source_object_index].unknown_0x10;
        }
        effect->source_object_index = D_800FDF58[actor->owner_object_index].source_object_index;
        effect->unknown_0x3b = D_800FDF58[actor->owner_object_index].unknown_0x3b;
        effect->unknown_0xc = D_800FDF58[actor->owner_object_index].unknown_0xc;
        effect->facing_or_reward_kind |= D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80;
        effect->flags.word = (effect->flags.word & 0xFFFCFFFF) |
                     ((D_800FDF58[actor->owner_object_index].flags.half[1] & 3) << 16);
        effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (D_800FDF58[actor->owner_object_index].flags.word & 0x780000);
        resource = g_field_resource_entries[D_800FDF58[actor->owner_object_index].unknown_0x3b].start;
        if (resource != 0)
        {
            field_restart_actor_animation(effect, resource);
        }
        goto after_source;
    }
    if (effect->state == 3)
    {
        track_object_index = actor->track_object_indices[g_field_track_index];
        if (track_object_index == 0xFF)
        {
            effect->state = track_object_index;
            actor->active_counts[g_field_track_index][part_index]--;
            actor->track_counters[g_field_track_index][part_index]--;
            return -1;
        }
        if (!((D_80105AE0[actor->track_object_indices[g_field_track_index]].state_flags.word >> 6) & 1))
        {
            s32 offset;
            s32 offset2;
            s32 metadata_index;
            FieldSpawnTrackBinding *bindings = D_80105880;
            if (actor->track_object_indices[g_field_track_index] < 2U)
            {
                offset = actor->track_object_indices[g_field_track_index] * 0x1C;
            }
            else
            {
                offset = 0x38;
            }
            metadata_index = ((FieldSpawnTrackBinding*)((u8*)bindings + offset))->track_index;
            if (metadata_index == actor->track_object_indices[g_field_track_index])
            {
                FieldSpawnTrackBinding *next_bindings = D_80105880;
                if ((u32)(metadata_index & 0xFF) < 2U)
                {
                    offset2 = metadata_index * 0x1C;
                }
                else
                {
                    offset2 = 0x38;
                }
                if (((FieldSpawnTrackBinding*)((u8*)next_bindings + offset2))->state != 0)
                {
                    goto kill_rec;
                }
            }
        }
        {
            FieldSpawnObjectPlacement *table_base = D_80105AE0;
            track_object_state = &table_base[actor->track_object_indices[g_field_track_index]];
        }
        if (track_object_state->state_flags.bytes[0] & 1)
        {
            if (track_object_state->state_flags.bytes[2] != actor->actor_index)
            {
            kill_rec:
                effect->state = 0xFF;
                goto after_source;
            }
        }
        record_base = &D_800FDF58[actor->track_object_indices[g_field_track_index]];
        if (!((part->behavior_flags >> 11) & 1) && !((part->placement_flags >> 25) & 1) && (part->unknown_0x2c >> 5) == 0 &&
            (*(u32*)&part->unknown_0xc & 0xFFFF0000) == 0x80800000 && part->unknown_0x10 == 0x80)
        {
            effect->flags.word |= 0x10008000;
            effect->unknown_0x18 = D_800FE3A0[record_base->source_object_index].unknown_0xe;
            effect->unknown_0x19 = D_800FE3A0[record_base->source_object_index].unknown_0xf;
            effect->unknown_0x1a = D_800FE3A0[record_base->source_object_index].unknown_0x10;
        }
        effect->source_object_index = D_800FDF58[actor->track_object_indices[g_field_track_index]].source_object_index;
        effect->unknown_0x3b = D_800FDF58[actor->track_object_indices[g_field_track_index]].unknown_0x3b;
        effect->unknown_0xc = D_800FDF58[actor->track_object_indices[g_field_track_index]].unknown_0xc;
        effect->facing_or_reward_kind |= D_800FDF58[actor->track_object_indices[g_field_track_index]].facing_or_reward_kind & 0x80;
        effect->flags.word = (effect->flags.word & 0xFFFCFFFF) |
                     ((D_800FDF58[actor->track_object_indices[g_field_track_index]].flags.half[1] & 3) << 16);
        effect->flags.word =
            (effect->flags.word & 0xFF87FFFF) | (D_800FDF58[actor->track_object_indices[g_field_track_index]].flags.word & 0x780000);
        resource = g_field_resource_entries[D_800FDF58[actor->track_object_indices[g_field_track_index]].unknown_0x3b].start;
        if (resource != 0)
        {
            field_restart_actor_animation(effect, resource);
        }
    }

after_source:
    if ((part->track_flags >> 13) & 1)
    {
        effect->motion_scale = field_evaluate_parameter_track(actor, part->unknown_0x18 & 0xF);
    }
    if ((((part->placement_flags >> 10) & 1) || (part->spawn_flags & 0x08000000)) && effect->position_source != 0 &&
        !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80))
    {
        rotated_x = -rotated_x;
        effect->x = -effect->x;
    }
    if ((effect->flags.word & 0x07000000) == 0x05000000)
    {
        effect->x = 0;
        effect->y = 0;
        effect->z = 0;
    }
    if (part->effect_flags & 0x60000000)
    {
        effect->x = 0;
        effect->y = 0;
        effect->z = 0;
    }

    placement_kind = part->placement_flags >> 18;
    placement_kind &= 0x3F;
    switch (placement_kind)
    {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
    case 0x8:
    case 0x9:
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
        if (placement_kind >= 0xA)
        {
            track_object_index = actor->track_object_indices[g_field_track_index];
            if (track_object_index == 0xFF)
            {
                effect->state = track_object_index;
                actor->active_counts[g_field_track_index][part_index]--;
                actor->track_counters[g_field_track_index][part_index]--;
            return -1;
            }
            object_index = actor->track_object_indices[g_field_track_index];
            placement_kind -= 0xA;
source_record = &D_800FDF58[object_index]; placement_object = &D_80105AE0[object_index];
        }
        else
        {
            {
                FieldSpawnObjectPlacement *table_base = D_80105AE0;
                owner_placement_guard = &table_base[actor->owner_object_index];
            }
            if ((owner_placement_guard->state_flags.bytes[0] & 1) && actor->actor_index >= 0x40U &&
                !(((u32)owner_placement_guard->state_flags.word >> 5) & 1) && owner_placement_guard->state_flags.bytes[2] != actor->actor_index)
            {
                goto fail_slot;
            }
            object_index = actor->owner_object_index;
source_record = &D_800FDF58[object_index]; placement_object = &D_80105AE0[object_index];
        }

        if ((part->placement_flags >> 9) & 1)
        {
            extent = abs(placement_object->bounds_right - placement_object->bounds_left);
            part->footprint_scale_x = extent * 2;
        }
        if ((part->placement_flags >> 1) & 1)
        {
            work_a = 0;
            extent = abs(placement_object->bounds_bottom - placement_object->bounds_top);
            part->footprint_scale_y = extent * 2;
        } else { work_a = 0; }
        work_value = work_a;
        switch (placement_kind)
        {
        case 1:
            work_value = (placement_object->bounds_right + placement_object->bounds_left) >> 1;
            work_a = (placement_object->bounds_bottom + placement_object->bounds_top) >> 1;
            break;
        case 2:
            work_value = (placement_object->bounds_right + placement_object->bounds_left) >> 1;
            work_a = 0;
            break;
        case 3:
            work_value = (placement_object->bounds_right + placement_object->bounds_left) >> 1;
            work_a = placement_object->bounds_top;
            break;
        case 4:
            work_value = placement_object->bounds_left;
            work_a = (placement_object->bounds_bottom + placement_object->bounds_top) >> 1;
            break;
        case 5:
            work_value = placement_object->bounds_right;
            work_a = (placement_object->bounds_bottom + placement_object->bounds_top) >> 1;
            break;
        case 6:
            work_value = placement_object->bounds_left;
            work_a = placement_object->bounds_top;
            break;
        case 7:
            work_value = placement_object->bounds_right;
            work_a = placement_object->bounds_top;
            break;
        case 8:
            work_value = placement_object->bounds_left;
            work_a = placement_object->bounds_bottom;
            break;
        case 9:
            work_value = placement_object->bounds_right;
            work_a = placement_object->bounds_bottom;
            break;
        default:
            break;
        }
        work_value <<= 8;
        work_a <<= 8;
        effect->x += source_record->x + work_value;
        effect->y += source_record->y + work_a;
        effect->z += source_record->z;
        if (effect->state == 0xFD)
        {
            object_table = D_80105AE0;
            effect->source_object_index = source_record->source_object_index;
            attached_source_object = &object_table[source_record->source_object_index];
            if (!(attached_source_object->state_flags.bytes[0] & 1) || attached_source_object->state_flags.bytes[2] == actor->actor_index)
            {
                effect->unknown_0xc = source_record->unknown_0xc;
                effect->unknown_0x3b = source_record->unknown_0x3b;
                effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (source_record->flags.word & 0x780000);
                effect->flags.bits.group = source_record->flags.half[1];
                if (!((part->behavior_flags >> 11) & 1) && !((part->placement_flags >> 25) & 1) && (part->unknown_0x2c >> 5) == 0 &&
                    (*(u32*)&part->unknown_0xc & 0xFFFF0000) == 0x80800000 && part->unknown_0x10 == 0x80)
                {
                    effect->flags.word |= 0x10008000;
                    effect->unknown_0x18 = D_800FE3A0[source_record->source_object_index].unknown_0xe;
                    effect->unknown_0x19 = D_800FE3A0[source_record->source_object_index].unknown_0xf;
                    effect->unknown_0x1a = D_800FE3A0[source_record->source_object_index].unknown_0x10;
                }
                if (effect->facing_or_reward_kind == 0xFF)
                {
                    effect->facing_or_reward_kind = source_record->facing_or_reward_kind;
                    effect->saved_state = source_record->saved_state;
                    part->footprint_scale_x = D_800FE3A0[source_record->source_object_index].footprint_scale_x;
                    part->footprint_scale_y = D_800FE3A0[source_record->source_object_index].footprint_scale_y;
                    effect->unknown_0x34 = source_record->unknown_0x34;
                    effect->unknown_0x35 = source_record->unknown_0x35;
                    effect->track_index = source_record->track_index;
                    effect->unknown_0x36 = source_record->unknown_0x36;
                    effect->unknown_0x37 = source_record->unknown_0x37;
                    effect->unknown_0x38 = source_record->unknown_0x38;
                    effect->unknown_0x16 = source_record->unknown_0x16;
                }
                else
                {
                    effect->facing_or_reward_kind |= source_record->facing_or_reward_kind & 0x80;
                    resource = g_field_resource_entries[source_record->unknown_0x3b].start;
                    if (resource != 0)
                    {
                        field_restart_actor_animation(effect, resource);
                    }
                }
            }
            else
            {
                goto mark_dead;
            }
        }
        break;
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x2A:
    case 0x2B:
    case 0x2C:
    case 0x2D:
    case 0x2E:
    case 0x2F:
    case 0x30:
    case 0x31:
        if (placement_kind >= 0x2A)
        {
            goto scanA_high;
        }
        sibling_part_index = placement_kind - 0x14;
        goto scanA_init;

    scanA_copy:
        effect->facing_or_reward_kind = g_field_effect_records[work_a].facing_or_reward_kind;
        effect->saved_state = g_field_effect_records[work_a].saved_state;
        effect->unknown_0x34 = g_field_effect_records[work_a].unknown_0x34;
        effect->unknown_0x35 = g_field_effect_records[work_a].unknown_0x35;
        effect->track_index = g_field_effect_records[work_a].track_index;
        effect->unknown_0x36 = g_field_effect_records[work_a].unknown_0x36;
        effect->unknown_0x37 = g_field_effect_records[work_a].unknown_0x37;
        effect->unknown_0x38 = g_field_effect_records[work_a].unknown_0x38;
        goto scanA_done;

    scanA_high:
        sibling_part_index = placement_kind - 0x22;
    scanA_init:
        work_a = start;
        if (work_a < 0x100)
        {
            do
            {
                if (g_field_effect_records[work_a].state != 0xFF && g_field_effect_records[work_a].part_index == sibling_part_index && g_field_effect_records[work_a].actor_index == actor->actor_index &&
                    ((actor->parts[sibling_part_index].orientation_flags & 4) || g_field_effect_records[work_a].track_index == g_field_track_index))
                {
                    effect->x += g_field_effect_records[work_a].x;
                    effect->y += g_field_effect_records[work_a].y;
                    effect->z += g_field_effect_records[work_a].z;
                    effect->flags.word = (effect->flags.word & ~0x1000) | (g_field_effect_records[work_a].flags.word & 0x1000);
                    if (part->unknown_0x1c & 0x08000000)
                    {
                        effect->rotation_x = ((FieldSpawnVector*)&g_field_effect_records[work_a].rotation_x)->x;
                        effect->heading = ((FieldSpawnVector*)&g_field_effect_records[work_a].rotation_x)->y;
                        effect->pitch = ((FieldSpawnVector*)&g_field_effect_records[work_a].rotation_x)->z;
                    }
                    effect->reference_index = work_a;
                    if (effect->state == 0xFD)
                    {
                        effect->unknown_0x3b = g_field_effect_records[work_a].unknown_0x3b;
                        effect->unknown_0xc = g_field_effect_records[work_a].unknown_0xc;
                        effect->state = g_field_effect_records[work_a].state;
                        effect->source_object_index = g_field_effect_records[work_a].source_object_index;
                        effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (((u32)g_field_effect_records[work_a].flags.word >> 19 & 15) << 19);
                        effect->flags.word = (effect->flags.word & 0xFFFCFFFF) | ((((u32)g_field_effect_records[work_a].flags.word >> 16) & 3) << 16);
                        if (effect->facing_or_reward_kind == 0xFF)
                        {
                            goto scanA_copy;
                        }
                        effect->source_object_index = g_field_effect_records[work_a].source_object_index;
                        effect->state = g_field_effect_records[work_a].state;
                        effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (((u32)g_field_effect_records[work_a].flags.word >> 19 & 15) << 19);
                        effect->flags.word = (effect->flags.word & 0xFFFCFFFF) | ((((u32)g_field_effect_records[work_a].flags.word >> 16) & 3) << 16);
                        effect->saved_state = g_field_effect_records[work_a].saved_state;
                        effect->facing_or_reward_kind = g_field_effect_records[work_a].facing_or_reward_kind;
                        resource = g_field_resource_entries
                                  [D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].unknown_0x3b]
                                      .start;
                        if (resource != 0)
                        {
                            field_begin_actor_animation_forward(effect, resource);
                        }
                    }
                    break;
                }
                work_a++;
            } while (work_a < 0x100);
        }
    scanA_done:
        if (work_a == 0x100)
        {
            goto fail_slot;
        }
        func_8006D79C(actor, part_index, work_a + 1);
        break;

    case 0x25:
        effect->x += (part->offset_x << 8) - D_800F22A0;
        effect->y += (part->offset_y << 8) - D_800F22A4;
        effect->z += (part->offset_z << 8) - D_800F22A8;
        if (effect->state == 0xFD)
        {
            effect->state = 0xFE;
        }
        break;

    case 0x1C:
        effect->x -= D_800F22A0;
        effect->y -= D_800F22A4;
        effect->z -= D_800F22A8;
        effect->flags.word |= 0x1000;
        break;

    case 0x1D:
        effect->y -= 0x7000;
        effect->x -= D_800F22A0;
        effect->z -= D_800F22A8;
        effect->flags.word |= 0x1000;
        effect->y -= D_800F22A4;
        break;

    case 0x1E:
        effect->y += 0x7000;
        effect->x -= D_800F22A0;
        effect->z -= D_800F22A8;
        effect->flags.word |= 0x1000;
        effect->y -= D_800F22A4;
        break;

    case 0x1F:
        effect->x += 0xFFFF6000;
        effect->x -= D_800F22A0;
        effect->y -= D_800F22A4;
        effect->z -= D_800F22A8;
        effect->flags.word |= 0x1000;
        break;

    case 0x20:
        effect->x += 0xA000;
        effect->x -= D_800F22A0;
        effect->y -= D_800F22A4;
        effect->z -= D_800F22A8;
        effect->flags.word |= 0x1000;
        break;

    case 0x21:
        effect->x += 0xFFFF6000;
        effect->y -= 0x7000;
        effect->x -= D_800F22A0;
        effect->z -= D_800F22A8;
        effect->flags.word |= 0x1000;
        effect->y -= D_800F22A4;
        break;

    case 0x22:
        effect->x += 0xA000;
        effect->y -= 0x7000;
        effect->x -= D_800F22A0;
        effect->z -= D_800F22A8;
        effect->flags.word |= 0x1000;
        effect->y -= D_800F22A4;
        break;

    case 0x23:
        effect->x += 0xFFFF6000;
        effect->y += 0x7000;
        effect->x -= D_800F22A0;
        effect->z -= D_800F22A8;
        effect->flags.word |= 0x1000;
        effect->y -= D_800F22A4;
        break;

    case 0x24:
        effect->x += 0xA000;
        effect->y += 0x7000;
        effect->x -= D_800F22A0;
        effect->z -= D_800F22A8;
        effect->flags.word |= 0x1000;
        effect->y -= D_800F22A4;
        break;

    case 0x27:
        {
            FieldSpawnObjectPlacement *table_base = D_80105AE0;
            owner_placement_state = &table_base[actor->owner_object_index];
        }
        if ((owner_placement_state->state_flags.bytes[0] & 1) && owner_placement_state->state_flags.bytes[2] != actor->actor_index &&
            actor->actor_index >= 0x40U && !(((u32)owner_placement_state->state_flags.word >> 5) & 1))
        {
            goto fail_slot;
        }
        source_record = &D_800FDF58[actor->owner_object_index];
        if (((part->placement_flags >> 10) & 1) && !(source_record->facing_or_reward_kind & 0x80))
        {
            effect->x += source_record->x - (part->offset_x << 8);
        }
        else
        {
            effect->x += source_record->x + (part->offset_x << 8);
        }
        effect->y += source_record->y + (part->offset_y << 8);
        effect->z += source_record->z + (part->offset_z << 8);
        if (effect->state == 0xFD && effect->facing_or_reward_kind == 0xFF)
        {
            effect->source_object_index = source_record->source_object_index;
            {
                FieldSpawnObjectPlacement *table_base = D_80105AE0;
                owner_source_object = &table_base[source_record->source_object_index];
            }
            if (owner_source_object->state_flags.bytes[0] & 1)
            {
                if (owner_source_object->state_flags.bytes[2] != actor->actor_index)
                {
                    goto mark_dead;
                }
            }
            effect->unknown_0x3b = source_record->unknown_0x3b;
            effect->unknown_0xc = source_record->unknown_0xc;
            effect->facing_or_reward_kind = source_record->facing_or_reward_kind;
            effect->saved_state = source_record->saved_state;
            effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (source_record->flags.word & 0x780000);
            effect->flags.word = (effect->flags.word & 0xFFFCFFFF) | ((source_record->flags.half[1] & 3) << 16);
            resource = g_field_resource_entries[source_record->unknown_0x3b].start;
            if (resource != 0)
            {
                field_restart_actor_animation(effect, resource);
            }
            break;
        }
        break;

    case 0x28: {
        if (actor->track_object_indices[g_field_track_index] == 0xFF)
        {
            effect->state = 0xFF;
            actor->active_counts[g_field_track_index][part_index]--;
            actor->track_counters[g_field_track_index][part_index]--;
            return -1;
        }
        source_record = &D_800FDF58[actor->track_object_indices[g_field_track_index]];
        if ((part->spawn_flags & 0x08000000) && !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80))
        {
            effect->x += source_record->x - (part->offset_x << 8);
        }
        else if (((part->placement_flags >> 10) & 1) && !(source_record->facing_or_reward_kind & 0x80))
        {
            effect->x += source_record->x - (part->offset_x << 8);
        }
        else
        {
            effect->x += source_record->x + (part->offset_x << 8);
        }
        effect->y += source_record->y + (part->offset_y << 8);
        effect->z += source_record->z + (part->offset_z << 8);
        if (effect->state == 0xFD && effect->facing_or_reward_kind == 0xFF)
        {
            effect->source_object_index = source_record->source_object_index;
            {
                FieldSpawnObjectPlacement *table_base = D_80105AE0;
                track_source_object = &table_base[source_record->source_object_index];
            }
            if (track_source_object->state_flags.bytes[0] & 1)
            {
                if (track_source_object->state_flags.bytes[2] != actor->actor_index)
                {
                    goto mark_dead;
                }
            }
            effect->unknown_0x3b = source_record->unknown_0x3b;
            effect->unknown_0xc = source_record->unknown_0xc;
            effect->facing_or_reward_kind = source_record->facing_or_reward_kind;
            effect->saved_state = source_record->saved_state;
            effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (source_record->flags.word & 0x780000);
            effect->flags.word = (effect->flags.word & 0xFFFCFFFF) | ((source_record->flags.half[1] & 3) << 16);
            resource = g_field_resource_entries[source_record->unknown_0x3b].start;
            if (resource != 0)
            {
                field_restart_actor_animation(effect, resource);
            }
            break;
        }
        break;

    }
    case 0x29:
        placement_object = &D_80105AE0[actor->owner_object_index];
        source_record = &D_800FDF58[actor->owner_object_index];
        effect->x += source_record->x + (placement_object->attachment_points[(part->effect_flags >> 21) & 3].x << 8);
        effect->y += source_record->y + (placement_object->attachment_points[(part->effect_flags >> 21) & 3].y << 8);
        effect->z += source_record->z;
        if (effect->state == 0xFD)
        {
            effect->source_object_index = source_record->source_object_index;
            effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (source_record->flags.word & 0x780000);
            effect->flags.word = (effect->flags.word & 0xFFFCFFFF) | ((source_record->flags.half[1] & 3) << 16);
            {
                FieldSpawnObjectPlacement *table_base = D_80105AE0;
                source_owner_object = &table_base[source_record->source_object_index];
            }
            goto check_owner;
        }
        break;

    case 0x32:
        {
            FieldSpawnObjectPlacement *table_base = D_80105AE0;
            placement_object = &table_base[actor->owner_object_index];
        }
        source_record = &D_800FDF58[actor->owner_object_index];
        effect->x += source_record->x + (placement_object->attachment_points[(part->effect_flags >> 21) & 3].x << 8);
        effect->y += source_record->y;
        effect->z += source_record->z + (part->offset_z << 8);
        if (((part->placement_flags >> 10) & 1) && !(source_record->facing_or_reward_kind & 0x80))
        {
            effect->x = effect->x - (part->offset_x << 8);
        }
        else
        {
            effect->x = effect->x + (part->offset_x << 8);
        }
        if (effect->state == 0xFD)
        {
            effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (source_record->flags.word & 0x780000);
            effect->flags.word = (effect->flags.word & 0xFFFCFFFF) | ((source_record->flags.half[1] & 3) << 16);
            effect->source_object_index = source_record->source_object_index;
            {
                FieldSpawnObjectPlacement *table_base = D_80105AE0;
                source_owner_object = &table_base[source_record->source_object_index];
            }
        check_owner:
            if (!(source_owner_object->state_flags.bytes[0] & 1) || source_owner_object->state_flags.bytes[2] == actor->actor_index)
            {
                effect->state = 2;
                if (effect->facing_or_reward_kind == 0xFF)
                {
                    effect->unknown_0x3b = source_record->unknown_0x3b;
                    effect->unknown_0xc = source_record->unknown_0xc;
                    effect->facing_or_reward_kind = source_record->facing_or_reward_kind;
                    effect->saved_state = source_record->saved_state;
                }
                resource = g_field_resource_entries[source_record->unknown_0x3b].start;
            maybe_attach:
                if (resource != 0)
                {
                    field_restart_actor_animation(effect, resource);
                }
                break;
            }
            goto mark_dead;
        }
        break;

    case 0x33:
        placement_object = &D_80105AE0[actor->owner_object_index];
        source_record = &D_800FDF58[actor->owner_object_index];
        effect->x += source_record->x + (placement_object->ground_attachment_points[D_80105760].x << 8);
        effect->y += source_record->y;
        effect->z += source_record->z + (placement_object->ground_attachment_points[D_80105760].y << 8);
        effect->flags.bits.placement = *(u16*)&D_80105760;
        break;

    case 0x34: {
        if (actor->track_object_indices[g_field_track_index] == 0xFF)
        {
            effect->state = 0xFF;
            actor->active_counts[g_field_track_index][part_index]--;
            actor->track_counters[g_field_track_index][part_index]--;
            return -1;
        }
        source_record = &D_800FDF58[actor->track_object_indices[g_field_track_index]];
        if (!(source_record->facing_or_reward_kind & 0x80))
        {
            effect->x += source_record->x + (actor->track_offsets[g_field_track_index].x << 8);
        }
        else
        {
            effect->x += source_record->x - (actor->track_offsets[g_field_track_index].x << 8);
        }
        effect->y += source_record->y + (actor->track_offsets[g_field_track_index].y << 8);
        effect->z += source_record->z;
        break;

    }
    case 0x35:
        effect->x += D_80105778.vx;
        effect->y += D_80105778.vy;
        effect->z += D_80105778.vz;
        break;

    case 0x36:
        effect->x += part->offset_x << 8;
        effect->y += part->offset_y << 8;
        effect->z += part->offset_z << 8;
    check_dead:
        if (effect->state == 0xFD)
        {
        mark_dead:
            effect->state = 0xFE;
        }
        break;

    scanB_copy:
        effect->facing_or_reward_kind = g_field_effect_records[sibling_index].facing_or_reward_kind;
        effect->saved_state = g_field_effect_records[sibling_index].saved_state;
        effect->unknown_0x34 = g_field_effect_records[sibling_index].unknown_0x34;
        effect->unknown_0x35 = g_field_effect_records[sibling_index].unknown_0x35;
        effect->track_index = g_field_effect_records[sibling_index].track_index;
        effect->unknown_0x36 = g_field_effect_records[sibling_index].unknown_0x36;
        effect->unknown_0x37 = g_field_effect_records[sibling_index].unknown_0x37;
        effect->unknown_0x38 = g_field_effect_records[sibling_index].unknown_0x38;
        goto scanB_done;

    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
        sibling_index = start;
        work_value = placement_kind - 0x37;
        if (sibling_index < 0x100)
        {
            do
            {
                if (g_field_effect_records[sibling_index].state != 0xFF && g_field_effect_records[sibling_index].part_index == work_value && g_field_effect_records[sibling_index].actor_index == actor->actor_index &&
                    ((actor->parts[work_value].orientation_flags & 4) || g_field_effect_records[sibling_index].track_index == g_field_track_index))
                {
                    effect->x += g_field_effect_records[sibling_index].x;
                    effect->y += g_field_effect_records[sibling_index].y;
                    effect->z += g_field_effect_records[sibling_index].z;
                    if ((part->spawn_flags & 0x08000000) && !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80))
                    {
                        effect->x -= part->offset_x << 8;
                    }
                    else if (((part->placement_flags >> 10) & 1) && !(g_field_effect_records[sibling_index].facing_or_reward_kind & 0x80))
                    {
                        effect->x -= part->offset_x << 8;
                    }
                    else
                    {
                        effect->x += part->offset_x << 8;
                    }
                    effect->y += part->offset_y << 8;
                    effect->z += part->offset_z << 8;
                    effect->flags.word = (effect->flags.word & ~0x1000) | (g_field_effect_records[sibling_index].flags.word & 0x1000);
                    if (part->unknown_0x1c & 0x08000000)
                    {
                        effect->rotation_x = ((FieldSpawnVector*)&g_field_effect_records[sibling_index].rotation_x)->x;
                        effect->heading = ((FieldSpawnVector*)&g_field_effect_records[sibling_index].rotation_x)->y;
                        effect->pitch = ((FieldSpawnVector*)&g_field_effect_records[sibling_index].rotation_x)->z;
                    }
                    effect->reference_index = sibling_index;
                    if (effect->state == 0xFD)
                    {
                        effect->unknown_0x3b = g_field_effect_records[sibling_index].unknown_0x3b;
                        effect->unknown_0xc = g_field_effect_records[sibling_index].unknown_0xc;
                        effect->state = g_field_effect_records[sibling_index].state;
                        effect->source_object_index = g_field_effect_records[sibling_index].source_object_index;
                        effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (((u32)g_field_effect_records[sibling_index].flags.word >> 19 & 15) << 19);
                        effect->flags.word = (effect->flags.word & 0xFFFCFFFF) | ((((u32)g_field_effect_records[sibling_index].flags.word >> 16) & 3) << 16);
                        if (effect->facing_or_reward_kind == 0xFF)
                        {
                            goto scanB_copy;
                        }
                        effect->source_object_index = g_field_effect_records[sibling_index].source_object_index;
                        effect->state = g_field_effect_records[sibling_index].state;
                        effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (((u32)g_field_effect_records[sibling_index].flags.word >> 19 & 15) << 19);
                        effect->flags.word = (effect->flags.word & 0xFFFCFFFF) | ((((u32)g_field_effect_records[sibling_index].flags.word >> 16) & 3) << 16);
                        effect->saved_state = g_field_effect_records[sibling_index].saved_state;
                        effect->facing_or_reward_kind = g_field_effect_records[sibling_index].facing_or_reward_kind;
                        resource = g_field_resource_entries
                                  [D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].unknown_0x3b]
                                      .start;
                        if (resource != 0)
                        {
                            field_begin_actor_animation_forward(effect, resource);
                        }
                    }
                    break;
                }
                sibling_index++;
            } while (sibling_index < 0x100);
        }
    scanB_done:
        if (sibling_index != 0x100)
        {
            goto scanB_recurse;
        }
    fail_slot:
        effect->state = 0xFF;
        actor->active_counts[g_field_track_index][part_index]--;
    drop_slot:
        actor->track_counters[g_field_track_index][part_index]--;
        return -1;
    scanB_recurse:
        func_8006D79C(actor, part_index, sibling_index + 1);
        break;
    }

        switch ((part->effect_flags >> 29) & 3)
        {
        case 1:
            direction_vector->vx = (D_800FDF58[actor->owner_object_index].x - effect->x) >> 8;
            direction_vector->vy = (D_800FDF58[actor->owner_object_index].y - effect->y) >> 8;
            direction_z = (direction_vector->vz = (D_800FDF58[actor->owner_object_index].z - effect->z) >> 8);
            effect->heading = ratan2(-direction_z, direction_vector->vx);
            gte_ldlvl(direction_vector);
            gte_sqr0();
            gte_stlvnl(squared_vector);
            effect->pitch = ratan2(SquareRoot0(squared_vector->vx + squared_vector->vz), -direction_vector->vy);
            effect->rotation_x = 0;
            effect->x = D_800FDF58[actor->owner_object_index].x;
            effect->y = D_800FDF58[actor->owner_object_index].y;
            effect->z = D_800FDF58[actor->owner_object_index].z;
            break;
        case 2:
            direction_vector->vx = (D_800FDF58[actor->track_object_indices[g_field_track_index]].x - effect->x) >> 8;
            direction_vector->vy = (D_800FDF58[actor->track_object_indices[g_field_track_index]].y - effect->y) >> 8;
            direction_z = (D_800FDF58[actor->track_object_indices[g_field_track_index]].z - effect->z) >> 8;
            direction_vector->vz = direction_z;
            effect->heading = ratan2(-direction_z, direction_vector->vx);
            gte_ldlvl(direction_vector);
            gte_sqr0();
            gte_stlvnl(squared_vector);
            effect->pitch = ratan2(SquareRoot0(squared_vector->vx + squared_vector->vz), -direction_vector->vy);
            effect->rotation_x = 0;
            effect->x = D_800FDF58[actor->track_object_indices[g_field_track_index]].x;
            effect->y = D_800FDF58[actor->track_object_indices[g_field_track_index]].y;
            effect->z = D_800FDF58[actor->track_object_indices[g_field_track_index]].z;
            break;
        }
        if ((effect->flags.word & 0x07000000) == 0x05000000)
        {
            func_800A1D98(effect, func_8007E754(actor, part), (part->placement_flags >> 15) & 1, D_80105770);
            effect->position_data = 0;
            effect->path_group = D_80105770;
            func_800A1D48(&effect->position_data, effect, D_80105770);
            D_80105770 = D_80105770 + 1;
            if (D_80105770 == 0x20)
            {
                D_80105770 = 0;
            }
        }
        if (part->unknown_0x1c & 0x10000000)
        {
            field_swap_effect_position_source(effect, part);
        }
        if (!(effect->flags.word & 0x07000000) && effect->position_source != 0)
        {
            func_80070EF0(effect, part);
        }
        if ((part->placement_flags >> 3) & 1)
        {
            if (part->spawn_flags & 0x80000)
            {
                effect->height_or_retired_state = effect->y >> 8;
                effect->y -= field_evaluate_parameter_track_at_time(actor, *(u8*)&part->placement_flags >> 4, 0) << 8;
            }
            else
            {
                effect->y = (-field_evaluate_parameter_track_at_time(actor, (part->placement_flags >> 4) & 0xF, 0)) << 8;
            }
        }
        if (part->placement_flags & 1)
        {
            effect->z += 0x80;
        }
        switch ((s32)(((u32)effect->flags.word >> 29) & 3))
        {
        case 1:
            effect->motion_parameter = field_evaluate_parameter_track(actor, ((u16*)&part->orientation_flags)[1] & 0xF);
            break;
        case 2:
            effect->motion_parameter = field_evaluate_parameter_track_at_time(actor, ((u16*)&part->orientation_flags)[1] & 0xF, 0);
            break;
        case 3:
            field_resolve_effect_position(effect, part, direction_vector);
            direction_vector->vx -= effect->x;
            direction_vector->vy -= effect->y;
            direction_vector->vz -= effect->z;
            direction_vector->vx >>= 8;
            direction_vector->vy >>= 8;
            direction_vector->vz >>= 8;
            gte_ldlvl(direction_vector);
            gte_sqr0();
            gte_stlvnl(squared_vector);
            { s32 distance = SquareRoot0(squared_vector->vx + squared_vector->vy + squared_vector->vz) << 8;
            if (((s16*)&part->orientation_flags)[1] != 0)
            {
                effect->motion_parameter = (u32)(distance / ((s16*)&part->orientation_flags)[1]) >> 2;
            }
            else
            {
                effect->motion_parameter = (u32)distance >> 2;
            }
            }
            break;
        }
        if (part->spawn_flags & 0x08000000)
        {
            effect->facing_or_reward_kind &= 0x7F;
        }
        if ((((part->placement_flags >> 10) & 1) || (part->spawn_flags & 0x08000000)) &&
            !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80))
        {
            effect->facing_or_reward_kind ^= 0x80;
        }
        if ((part->placement_flags >> 14) & 1)
        {
            if (rotated_x < 0)
            {
                effect->facing_or_reward_kind ^= 0x80;
            }
            if (part->unknown_0x9 == 0)
            {
                effect->facing_or_reward_kind = (effect->facing_or_reward_kind & 0x7F) | (D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80);
            }
        }
        if (part->spawn_flags & 0x200000)
        {
            effect->facing_or_reward_kind ^= 0x80;
        }
        if ((actor->action_flags & 0x1E) == 8 && ((part->placement_flags >> 18) & 0x3F) == 0x33)
        {
            D_80105760 = D_80105760 + 1;
            if (D_80105760 < 3)
            {
                actor->active_counts[g_field_track_index][part_index]--;
                actor->track_counters[g_field_track_index][part_index]--;
                if (func_8006D79C(actor, part_index, 0) == -1)
                {
                    actor->active_counts[g_field_track_index][part_index]++;
                    actor->track_counters[g_field_track_index][part_index]++;
                }
            }
        }
        if ((u32)((effect->state + 2) & 0xFF) >= 2U)
        {
            if (actor->track_object_indices[g_field_track_index] != 0xFF)
            {
                if ((actor->animation->sync_flags & 0x14) == 0x14 && (actor->animation->sync_flags >> 12) == part_index)
                {
                    D_800FDF58[actor->track_object_indices[g_field_track_index]].state = 0xFE;
                    D_80105AE0[actor->track_object_indices[g_field_track_index]].state_flags.word |= 1;
                    D_80105AE0[actor->track_object_indices[g_field_track_index]].state_flags.bytes[2] = actor->actor_index;
                    ((u8*)&actor->action_flags)[1] = 1;
                }
            }
            if ((actor->animation->sync_flags & 0xA) == 0xA)
            {
                if (((actor->animation->sync_flags >> 8) & 0xF) == part_index)
                {
                    D_800FDF58[actor->owner_object_index].state = 0xFE;
                    D_80105AE0[actor->owner_object_index].state_flags.word |= 1;
                    D_80105AE0[actor->owner_object_index].state_flags.bytes[2] = actor->actor_index;
                }
            }
        }
        field_dispatch_actor_audio_event(actor, 2, part_index);
        field_dispatch_actor_audio_event(actor, 5, part_index);
        return effect_index;
}

#include "field_effect_types.h"
#include "field_actor_palette.h"
#include "field_mesh_render.h"


/** @brief Rotation fields written while initializing a spawned effect. */
typedef struct
{
    u8 pad0[0x10];
    s16 rotation_x;
    s16 heading;
    s16 pitch;
} FieldEffectRotationResult;

/** @brief Four-halfword GTE vector; the fourth halfword is padding. */
typedef struct
{
    s16 x;
    s16 y;
    s16 z;
    s16 pad;
} FieldSVector;

/**
 * @brief World-position footprint queried against field collision bounds.
 * @note Matches FieldCollisionQuery in field_collision.c: X/Z extents and Y tolerance.
 */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u16 width;
    s16 height_tolerance;
    u16 depth;
} FieldEffectCollisionQuery;

/**
 * @brief Position, step, and contact state used by the field collision resolver.
 * @note The low halfword of mode is footprint depth. Bits 16 and 17 select
 *       special resolution paths whose distinct meanings are not yet known.
 */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 move_x;
    s32 move_y;
    s32 move_z;
    s32 resolved_height;
    void* collision_node;
    s32 contact_flags;
    u16 width;
    s16 height_tolerance;
    union
    {
        u32 word;
        struct
        {
            u32 depth : 16;
            u32 unknown_16 : 1;
            u32 unknown_17 : 1;
            u32 unknown_high : 14;
        } bits;
    } mode;
} FieldEffectCollisionMover;

/** @brief Per-slot reward counters selected by reward kind. */
typedef struct
{
    u8 pad0[0x244];
    u8 counters[0x24];
} FieldRewardCounterView;

/** @brief Selected map dimensions used for effect movement bounds. */
typedef struct
{
    s16 width;
    s16 height;
} FieldEffectMapBounds;

/** @brief Camera translation used while updating effect movement. */
typedef struct
{
    u8 pad0[4];
    s32 x;
    s32 y;
    s32 z;
} FieldEffectCamera;

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

/** @brief Rotation fields for successive effect records. */
typedef struct
{
    FieldSVector angles;
    u8 pad8[0x54 - 8];
} FieldEffectRotationEntry;

/**
 * @brief Placement selector within the part flags word.
 */
typedef struct
{
    unsigned int lower : 18;
    unsigned int opcode : 6;
    unsigned int upper : 8;
} FieldPlacementBits;

/** @brief High-halfword actor action kinds handled as collectible rewards. */
typedef enum
{
    FIELD_PICKUP_EXPERIENCE_OR_CURRENCY = 31,
    FIELD_PICKUP_ITEM = 32,
    FIELD_PICKUP_RESTORE_QUARTER = 33,
    FIELD_PICKUP_RESTORE_HALF = 34
} FieldPickupAction;

/** @brief Bit positions in the part's behavior flags. */
#define FIELD_PART_PITCH_ACCELERATION_BIT 2
#define FIELD_PART_GROUND_BOUNCE_BIT 6

/** @brief Masks in the part's effect flags. */
#define FIELD_PART_MAP_COLLISION 0x00100000

/** @brief Bit positions in the part's placement flags. */
#define FIELD_PART_HEIGHT_TRACK_BIT 3

/** @brief Masks in the part's spawn flags. */
#define FIELD_PART_HEIGHT_FROM_BASE 0x00080000
#define FIELD_PART_SKIP_MAP_COLLISION 0x00800000
#define FIELD_PART_CAMERA_BOUNDS 0x01000000
#define FIELD_PART_GROUND_STOP 0x02000000
#define FIELD_PART_RETIRE_ON_COLLISION 0x10000000

#define FIELD_PICKUP_DELAY 0x10U
#define FIELD_PICKUP_DISTANCE 5
#define FIELD_PICKUP_SOUND 0x1F
#define FIELD_EFFECT_MOTION_KIND_MASK 0x07000000
#define FIELD_EFFECT_MOTION_PATH 0x05000000
#define FIELD_EFFECT_MOTION_HOMING 0x01000000
#define FIELD_EFFECT_DISTANCE_MASK 0x1FF
#define FIELD_EFFECT_ORIENTATION_LOCK 8

/** @brief Angle units used by the field rotation helpers. */
#define FIELD_ANGLE_TURN 0x1000
#define FIELD_ANGLE_HALF_TURN 0x800
#define FIELD_ANGLE_QUARTER_TURN 0x400
#define FIELD_ANGLE_MASK 0xFFF
#define FIELD_EFFECT_TURN_THRESHOLD 0x200
#define FIELD_EFFECT_TURN_STEP 0x80
#define FIELD_EFFECT_COLLISION_WIDTH 0xC
#define FIELD_EFFECT_COLLISION_DEPTH 8
#define FIELD_EFFECT_COLLISION_HEIGHT 0x10
/** @brief Skip initial surface lookup/inheritance; ordinary movement collisions still run. */
#define FIELD_EFFECT_SKIP_SURFACE_PREPASS ((void *) -2)
#define FIELD_EFFECT_CAMERA_X_MIN 0x500
#define FIELD_EFFECT_CAMERA_X_MAX 0x13B00
#define FIELD_EFFECT_CAMERA_Z_SPAN 0x1E800
#define FIELD_EFFECT_MAP_BOUNDS_ADDRESS 0x801ED400
#define FIELD_EFFECT_CAMERA_ADDRESS 0x801ED480

extern s32 g_field_track_index;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern FieldActorState g_field_actor_slots[80];
extern FieldMotionRecord D_800FDF58[];
extern FieldObjectPlacement D_80105AE0[];
extern FieldVector D_80105778;
extern s32 D_80105760;
/**
 * @brief Packed action context: action in low bits, source at bit 8, recipient at bit 16.
 * @note Also written on pool exhaustion and advanced during collision attempts;
 * the broader protocol of those writes is still unresolved.
 */
extern s32 g_field_action_context;
void field_set_action_context(s32 recipient_id, s32 source_id, s32 action);
extern s32 D_800FE754;
/** @brief Suppress repeated pickup audio until the next frame-command build. */
extern s32 g_field_pickup_sound_played;
extern FieldRewardCounterView D_800FD818[];

/**
 * @brief Roll one particle spawn record's scale and rotation fields from a
 *        part's parameter tracks, falling back to fixed part values or a
 *        random roll where a track is not assigned.
 * @param actor Owning actor supplying per-track counters.
 * @param part Part definition supplying rotation tracks and fallback values.
 * @param rotation_result Output rotation fields for the spawned effect.
 * @return Scaled pitch contribution added to @p rotation_result.
 */
s32 func_80070CB8(FieldActorState *actor, FieldActorPartDef *part, FieldEffectRotationResult *rotation_result)
{
    s16 angle;
    s32 random_product;
    s32 track_value;
    s32 track_scale;

    rotation_result->rotation_x = 0;
    if (part->unknown_0x1e != 0)
    {
        angle = ((0x1000 / part->unknown_0x1e) * actor->track_counters[g_field_track_index][part->unknown_0x32] + 0x400) & 0xFFF;
        rotation_result->heading = angle;
    }
    else
    {
        angle = (u32) rand() >> 3;
        rotation_result->heading = angle;
    }
    actor->track_counters[g_field_track_index][part->unknown_0x32]++;

    if ((part->track_flags >> 0xB) & 1)
    {
        track_scale = field_evaluate_parameter_track(actor, part->unknown_0x9 & 0xF);
        random_product = track_scale * (rand() << 3);
    }
    else
    {
        random_product = part->unknown_0x9 * (rand() << 3);
    }
    rotation_result->pitch = random_product >> 0xF;

    if ((part->track_flags >> 0xC) & 1)
    {
        track_value = field_evaluate_parameter_track(actor, part->unknown_0xa & 0xF);
    }
    else
    {
        track_value = part->unknown_0xa;
    }
    track_value *= 8;
    rotation_result->pitch += track_value;
    return track_value;
}

/**
 * @brief Move an effect to its source and preserve its old position as the new source.
 * @param effect Record whose position and position-source selection are exchanged.
 * @param part Part definition controlling the selected position source.
 */
void field_swap_effect_position_source(FieldMotionRecord *effect, FieldActorPartDef *part)
{
    FieldVector source_position;
    s32 unused[2]; /* Required by the original stack layout. */

    if (effect->position_source != FIELD_POSITION_NONE)
    {
        field_resolve_effect_position(effect, part, &source_position);
        effect->position_source = FIELD_POSITION_SAVED;
        effect->work_x = effect->x + D_800F22A0;
        effect->work_y = effect->y + D_800F22A4;
        effect->work_z = effect->z + D_800F22A8;
        effect->x = source_position.vx;
        effect->y = source_position.vy;
        effect->z = source_position.vz;
    }
}

/**
 * @brief Face a record toward the delta between its last stored position and
 *        a freshly rolled position, deriving both a horizontal-plane heading
 *        and vertical pitch.
 * @param effect Record whose heading and pitch are updated.
 * @param part Part definition used to resolve the position source.
 */
void func_80070EF0(FieldMotionRecord *effect, FieldActorPartDef *part)
{
    FieldVector target_position;
    FieldVector direction;
    FieldVector squared_direction;

    field_resolve_effect_position(effect, part, &target_position);
    direction.vx = (target_position.vx - effect->x) >> 8;
    direction.vy = (target_position.vy - effect->y) >> 8;
    direction.vz = (target_position.vz - effect->z) >> 8;

    gte_ldlvl(&direction);
    gte_sqr0();
    gte_stlvnl(&squared_direction);

    effect->heading = ratan2(-direction.vz, direction.vx);
    if (effect->heading < 0)
    {
        effect->heading += 0x1000;
    }

    if (direction.vy != 0)
    {
        effect->pitch = ratan2(SquareRoot0(squared_direction.vx + squared_direction.vz), -direction.vy);
    }
    else
    {
        effect->pitch = 0x400;
    }
    if (effect->pitch < 0)
    {
        effect->pitch += 0x1000;
    }
    effect->rotation_x = 0;
}


extern FieldMotionRecord g_field_effect_records[];
/** @brief Rotation-field view starting at g_field_effect_records + 0x10. */
extern FieldEffectRotationEntry D_800FF668[];

void field_update_effect_record(FieldMotionRecord *record, FieldActorPartDef *part, FieldActorState *actor);

/**
 * @brief Per-frame actor tick: advances every active effect record owned by
 *        this actor (culling off-screen ones, spawning chained effects on
 *        expiry), then refreshes the actor's palette-track texture pages.
 * @param actor_state Actor being ticked.
 */
void func_8007100C(FieldActorState *actor_state)
{
    FieldActorState *actor;
    FieldMotionRecord *effect;
    FieldActorPartDef *part;
    FieldActorPartDef *palette_part;
    void *palette_buffer;
    s32 part_index;
    s32 new_effect_index;
    s32 palette_changed;
    FieldMotionRecord *new_effect;
    s32 screen_depth;
    u8 previous_state;
    RECT palette_rect;
    FieldSVector screen_position;
    FieldVector segment_delta;
    FieldVector segment_angles;

    actor = actor_state;
    for (effect = g_field_effect_records; effect != &g_field_effect_records[FIELD_EFFECT_ACTIVE_RECORD_COUNT]; effect++)
    {
        if (effect->actor_index == actor->actor_index && effect->state != FIELD_EFFECT_RETIRED)
        {
            part = &actor->parts[effect->part_index];
            g_field_track_index = effect->track_index;
            field_update_effect_record(effect, part, actor);
            effect->age++;
            if (((part->behavior_flags >> 4) & 3) == 1 && (u16) effect->age == effect->lifetime)
            {
                previous_state = effect->state;
                effect->state = FIELD_EFFECT_RETIRED;
                effect->height_or_retired_state = previous_state;
            }
            if (((part->behavior_flags >> 4) & 3) == 3)
            {
                screen_position.x = (D_800F22A0 / 256) + (u32) (effect->x / 256 + 0xA0);
                screen_position.y = 0x70 + D_800F22A4 / 256 + effect->y / 256 - effect->z / 512 - D_800F22A8 / 512;
                if ((u16) (screen_position.x + 0x140) >= 0x3C1 || screen_position.y >= 0x1E1 || screen_position.y < -0xF0)
                {
                    previous_state = effect->state;
                    effect->state = FIELD_EFFECT_RETIRED;
                    effect->height_or_retired_state = previous_state;
                }
            }
            if (effect->state == FIELD_EFFECT_RETIRED)
            {
                func_80071500(effect, part);
            }
            if (part->effect_flags < 0 && effect->state != FIELD_EFFECT_RETIRED)
            {
                new_effect_index = func_8006D79C(actor, part->unknown_0x23 & 0xF, 0);
                if (new_effect_index != -1)
                {
                    new_effect = &g_field_effect_records[new_effect_index];
                    new_effect->x = effect->x;
                    new_effect->y = effect->y;
                    new_effect->z = effect->z;
                    segment_delta.vx = effect->x - g_field_effect_records[effect->previous_effect_index].x;
                    segment_delta.vy = effect->y - g_field_effect_records[effect->previous_effect_index].y;
                    segment_delta.vz = effect->z - g_field_effect_records[effect->previous_effect_index].z;
                    g_field_effect_records[effect->previous_effect_index].next_effect_index = new_effect_index;
                    screen_position.x = segment_delta.vx >> 8;
                    screen_depth = segment_delta.vz >> 9;
                    screen_position.y = (segment_delta.vy >> 8) - screen_depth;
                    segment_delta.vz = 0;
                    segment_delta.vx = screen_position.y;
                    segment_delta.vy = -screen_position.x;
                    func_8001CDAC(&segment_delta, &segment_angles, screen_depth);
                    new_effect->rotation_x = segment_angles.vx >> 6;
                    new_effect->heading = segment_angles.vy >> 6;
                    new_effect->pitch = segment_angles.vz >> 6;
                    new_effect->work_x = 0;
                    new_effect->work_y = 0;
                    new_effect->previous_effect_index = effect->previous_effect_index;
                    new_effect->next_effect_index = 0xFF;
                    effect->previous_effect_index = new_effect_index;
                }
            }
        }
    }

    if (actor->owner_object_index < 2)
    {
        palette_buffer = &g_field_actor_clut_buffers[actor->owner_object_index << 0xA];
    }
    else
    {
        palette_buffer = g_field_shared_clut_buffer;
    }
    part_index = 0;
    palette_changed = 0;
    if (actor->part_count != 0)
    {
        do
        {
            do
            {
                palette_part = (FieldActorPartDef *) (part_index * 0x48 + (s32) actor->parts);
            } while (0);
            if ((palette_part->track_flags >> 0x15) & 1)
            {
                do
                {
                    field_interpolate_palette_track(actor, palette_part->unknown_0x1c, palette_buffer, (u8 *) palette_buffer + 0x200);
                    palette_changed++;
                } while (0);
            }
            part_index++;
        } while (part_index < actor->part_count);
    }
    if (palette_changed != 0)
    {
        if (actor->owner_object_index < 2)
        {
            palette_rect.x = 0;
            palette_rect.y = (actor->owner_object_index * 2) + 0x1EF;
            palette_rect.w = 0x10;
            palette_rect.h = 1;
        }
        else
        {
            palette_rect.y = 0x1F3;
            palette_rect.w = 0x10;
            palette_rect.x = 0;
            palette_rect.h = 1;
        }
        LoadImage(&palette_rect, (u8 *) palette_buffer + 0x200);
    }
    field_update_actor_palette_animation(actor);
    func_8008332C(actor, actor->parts, actor->part_count);
}

/**
 * @brief Retire an effect, spawn configured follow-up effects, and synchronize actor state.
 * @param effect Effect record being retired.
 * @param part Part definition controlling follow-up effects and synchronization.
 */
void func_80071500(FieldMotionRecord* effect, FieldActorPartDef* part)
{
    FieldActorState* actor;
    FieldActorPartDef* spawn_part;
    FieldMotionRecord* new_effect;
    FieldMotionRecord* spawned_effect;
    FieldMotionRecord* effect_pool;
    FieldObjectPlacement* object;
    FieldObjectPlacement* object_table;
    FieldVector target_position;
    FieldVector direction;
    FieldVector squared_direction;
    s32 spawn_slot, spawn_mask;
    s32 new_effect_index;
    s32 spawn_count;
    s32 placement_kind;
    s16 animation_state;
    u8 next_effect_index;

    g_field_actor_slots[effect->actor_index].active_counts[effect->track_index][part->unknown_0x32]--;

    actor = &g_field_actor_slots[effect->actor_index];
    field_dispatch_actor_audio_event(actor, 3, effect->part_index);

    if (*(u32*)&part->unknown_0x2c & 0xF0000000)
    {
        if (effect->height_or_retired_state == -1)
        {
            effect->state = 0xFE;
        }
        else
        {
            effect->state = effect->height_or_retired_state;
        }

        effect_pool = g_field_effect_records;
        for (spawn_slot = 0, spawn_mask = 1; spawn_slot < 4; spawn_slot++, spawn_mask <<= 1)
        {
            if ((*(u32*)&part->unknown_0x2c >> 0x1C) & spawn_mask)
            {
                D_80105778.vx = effect->x;
                D_80105778.vy = effect->y;
                D_80105760 = 0;
                D_80105778.vz = effect->z;

                spawn_part = &g_field_actor_slots[effect->actor_index]
                                  .parts[(part->spawn_flags.halves.part_selectors >> (spawn_slot * 4)) & 0xF];
                spawn_count = 1;
                placement_kind = 0x35;
                if (((spawn_part->placement_flags >> 0x12) & 0x3F) == placement_kind)
                {
                    if (spawn_part->unknown_0xc != 0)
                    {
                        spawn_count = spawn_part->unknown_0xc;
                    }
                }

                if (spawn_count != 0)
                {
                    do
                    {
                        new_effect_index = func_8006D79C(&g_field_actor_slots[effect->actor_index],
                                                (part->spawn_flags.halves.part_selectors >> (spawn_slot * 4)) & 0xF, 0);
                        if (new_effect_index != -1)
                        {
                            spawned_effect = (FieldMotionRecord*)(new_effect_index * (s32)sizeof(FieldMotionRecord) + (s32)effect_pool);
                            if (!(((u8*)&spawned_effect->flags)[3] & 7) && (spawned_effect->position_source != 0))
                            {
                                new_effect = spawned_effect;
                                field_resolve_effect_position(new_effect, part, &target_position);
                                direction.vx = (target_position.vx - new_effect->x) >> 8;
                                direction.vy = (target_position.vy - new_effect->y) >> 8;
                                direction.vz = (target_position.vz - new_effect->z) >> 8;

                                gte_ldlvl(&direction);
                                gte_sqr0();
                                gte_stlvnl(&squared_direction);

                                new_effect->heading = ratan2(-direction.vz, direction.vx);
                                if (new_effect->heading < 0)
                                {
                                    new_effect->heading += 0x1000;
                                }

                                if (direction.vy != 0)
                                {
                                    new_effect->pitch = ratan2(SquareRoot0(squared_direction.vx + squared_direction.vz), -direction.vy);
                                }
                                else
                                {
                                    new_effect->pitch = 0x400;
                                }
                                if (new_effect->pitch < 0)
                                {
                                    new_effect->pitch += 0x1000;
                                }
                                new_effect->rotation_x = 0;
                            }
                        }
                        spawn_count--;
                    } while (spawn_count != 0);
                }
            }
        }
    }
    effect->state = FIELD_EFFECT_RETIRED;

    if (((actor->animation->sync_flags & 0x14) == 0x14) && ((actor->animation->sync_parts >> 0xD) == effect->part_index))
    {
        if (actor->track_object_indices[effect->track_index] != 0xFF)
        {
            object_table = D_80105AE0;
            object = &object_table[actor->track_object_indices[effect->track_index]];
            if (((u8*)&object->state_flags)[2] == actor->actor_index)
            {
                animation_state = D_800FDF58[actor->track_object_indices[effect->track_index]].motion_parameter;
                if ((animation_state != 0x90 && animation_state != 0x94) || (object->object_flags & 0x200))
                {
                    D_800FDF58[actor->track_object_indices[effect->track_index]].state = 0;
                }
                else
                {
                    D_800FDF58[actor->track_object_indices[effect->track_index]].state = 0xFE;
                }
                D_80105AE0[actor->track_object_indices[effect->track_index]].state_flags &= ~1;
            }
        }
    }

    if (((actor->animation->sync_flags & 0xA) == 0xA) &&
        (((actor->animation->sync_parts >> 0xA) & 7) == effect->part_index))
    {
        object = &D_80105AE0[actor->owner_object_index];
        if (((u8*)&object->state_flags)[2] == actor->actor_index)
        {
            animation_state = D_800FDF58[actor->owner_object_index].motion_parameter;
            if ((animation_state != 0x90 && animation_state != 0x94) || (object->object_flags & 0x200))
            {
                D_800FDF58[actor->owner_object_index].state = 0;
            }
            else
            {
                D_800FDF58[actor->owner_object_index].state = 0xFE;
            }
            D_80105AE0[actor->owner_object_index].state_flags &= ~1;
        }
    }

    if (effect->part_index == ((actor->animation->sync_parts & 0x1F) - 1))
    {
        D_800FDF58[actor->owner_object_index].x = effect->x;
        D_800FDF58[actor->owner_object_index].y = effect->y;
        D_800FDF58[actor->owner_object_index].z = effect->z;
        D_800FDF58[actor->owner_object_index].y = 0;
        D_800FDF58[actor->owner_object_index].facing_or_reward_kind =
            (D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x7F) | (effect->facing_or_reward_kind & 0x80);
    }

    if (effect->part_index == (((actor->animation->sync_parts >> 5) & 0x1F) - 1))
    {
        if (actor->track_object_indices[effect->track_index] != 0xFF)
        {
            D_800FDF58[actor->track_object_indices[effect->track_index]].x = effect->x;
            D_800FDF58[actor->track_object_indices[effect->track_index]].y = effect->y;
            D_800FDF58[actor->track_object_indices[effect->track_index]].z = effect->z;
            D_800FDF58[actor->track_object_indices[effect->track_index]].y = 0;
            D_800FDF58[actor->track_object_indices[effect->track_index]].facing_or_reward_kind =
                (D_800FDF58[actor->track_object_indices[effect->track_index]].facing_or_reward_kind & 0x7F) |
                (effect->facing_or_reward_kind & 0x80);
        }
    }

    if (effect->height_or_retired_state == 5)
    {
        next_effect_index = effect->next_effect_index;
        if (next_effect_index != 0xFF)
        {
            FieldMotionRecord* records = g_field_effect_records;
            s32 next_index = effect->next_effect_index;
            records[next_index].previous_effect_index = 0xFF;
        }
    }
}


/**
 * @brief Advance an actor-owned effect's placement, motion, pickups, and hit contacts.
 * @param record Active effect record.
 * @param part Part definition selecting parameter tracks and placement behavior.
 * @param actor Actor that owns the effect.
 */
void field_update_effect_record(FieldMotionRecord *record, FieldActorPartDef *part, FieldActorState *actor)
{
    FieldVector *target_position;
    FieldEffectCamera *camera;
    FieldEffectMapBounds *map_bounds;
    FieldEffectCollisionMover *mover;
    FieldEffectCollisionQuery *query;
    FieldVector *work_vector;
    FieldSVector *local_vector;
    FieldSVector *rotated_vector;
    FieldMatrix *rotation;
    FieldVector *placement_origin;
    FieldMotionRecord *reference_record;
    FieldObjectPlacement *reference_object;
    u32 flags;
    u32 record_flags;
    s32 selector;
    s32 x, y, z;
    s32 offset_or_angle;
    s32 current_angle;
    s32 dx, dy;
    void *initial_surface;
    s32 state_or_delta;
    s32 flags_byte;
    s32 slot;
    s32 recipient_index;
    u8 reference_state;
    u8 retired_state;

    camera = (FieldEffectCamera *) FIELD_EFFECT_CAMERA_ADDRESS;
    map_bounds = (FieldEffectMapBounds *) FIELD_EFFECT_MAP_BOUNDS_ADDRESS;
    mover = (FieldEffectCollisionMover *) FIELD_EFFECT_MOVER_ADDRESS;
    query = (FieldEffectCollisionQuery *) FIELD_EFFECT_QUERY_ADDRESS;
    work_vector = (FieldVector *) FIELD_EFFECT_VECTOR_ADDRESS;
    target_position = (FieldVector *) FIELD_EFFECT_TARGET_ADDRESS;
    local_vector = (FieldSVector *) FIELD_EFFECT_LOCAL_VECTOR_ADDRESS;
    rotated_vector = (FieldSVector *) FIELD_EFFECT_ROTATED_VECTOR_ADDRESS;
    rotation = (FieldMatrix *) FIELD_EFFECT_MATRIX_ADDRESS;
    placement_origin = (FieldVector *) FIELD_EFFECT_ORIGIN_ADDRESS;

    /* Sample transparency and motion tracks; the caller advances age afterward. */
    flags = part->effect_flags;
    if (flags & FIELD_EFFECT_SEMITRANSPARENT)
    {
        s32 semitransparent = field_evaluate_parameter_track_at_time(actor, (flags >> 0x19) & 0xF, (u16) ((u16) record->age)) != 0;
        record->flags = (record->flags & ~FIELD_EFFECT_SEMITRANSPARENT) | (semitransparent << 0x17);
    }
    if ((record->flags & 0x60000000) == 0x40000000)
    {
        record->motion_parameter = field_evaluate_parameter_track_at_time(actor, ((s16 *) &part->orientation_flags)[1] & 0xF, ((u16) record->age));
    }

    /* Linked segment records update their projected work vector and return early. */
    if (record->state == 5)
    {
        field_build_effect_part_matrix(record, part, rotation, actor);
        gte_SetRotMatrix(rotation);
        {
            u16 segment_x = record->heading;
            local_vector->y = 0;
            local_vector->x = segment_x;
        }
        local_vector->z = record->rotation_x;
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stlvnl(work_vector);
        record->work_x = work_vector->vx;
        record->work_y = work_vector->vy;
        return;
    }

    /* Path mode advances a wrapping byte parameter and interpolates world X/Z. */
    record_flags = record->flags;
    if ((record_flags & FIELD_EFFECT_MOTION_KIND_MASK) == FIELD_EFFECT_MOTION_PATH)
    {
        record->position_data.path_time = record->position_data.path_time + record->motion_parameter;
        func_800A1D48(&record->position_data.path_time, record, record->path_group);
    }

    /* Attached effects rebuild an origin, then add the rotated local displacement. */
    else if ((u32) ((record_flags >> 0x18) & 7) >= 2U)
    {
        if ((((u32) part->placement_flags >> 0x1A) & 3) == 2)
        {
            record->rotation_z_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_z_track & 0xF, ((u16) record->age));
        }
        if ((((u32) part->placement_flags >> 0x1C) & 3) == 2)
        {
            record->rotation_y_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_y_track & 0xF, ((u16) record->age));
        }
        if ((record->flags & 0x600) == 0x400)
        {
            u32 selector_high = *(u32 *) &part->unknown_0x1c >> 29;
            local_vector->y = -field_evaluate_parameter_track_at_time(actor, ((((*(u32 *) &part->unknown_0x20) & 0x3F) * 8) | selector_high) & 0xF, (u16) record->age);
        }
        else
        {
            local_vector->y = -((u16) record->flags & FIELD_EFFECT_DISTANCE_MASK);
        }
        if ((*(u32 *) &part->unknown_0x1c) & 0x01000000)
        {
            local_vector->y = (s16) ((D_80105AE0[actor->owner_object_index].scale_percent & 0x3FF) * (s16) (u16) local_vector->y / 100);
        }
        {
            u32 bounds_flags;
            s32 placement_kind;
            bounds_flags = part->placement_flags;
            placement_kind = (bounds_flags >> 0x12) & 0x3F;
            if (placement_kind < 0x14)
            {
                if ((bounds_flags >> 0x10) & 1)
                {
                    s32 previous_distance = local_vector->y;
                    if ((u32) (placement_kind - 0xA) >= 0x1CU)
                    {
                        dy = (D_80105AE0[actor->owner_object_index].bounds_right - D_80105AE0[actor->owner_object_index].bounds_left) >> 1;
                    }
                    else
                    {
                        dy = (D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_right - D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_left) >> 1;
                    }
                    dy = abs(dy);
                    local_vector->y = previous_distance - dy;
                    bounds_flags = part->placement_flags;
                }
                if ((bounds_flags >> 0x11) & 1)
                {
                    s32 previous_distance = local_vector->y;
                    if ((u32) (((bounds_flags >> 0x12) & 0x3F) - 0xA) >= 0x1CU)
                    {
                        dy = (D_80105AE0[actor->owner_object_index].bounds_bottom - D_80105AE0[actor->owner_object_index].bounds_top) >> 1;
                    }
                    else
                    {
                        dy = (D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_bottom - D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_top) >> 1;
                    }
                    dy = abs(dy);
                    local_vector->y = previous_distance - dy;
                }
            }
        }
        local_vector->x = 0;
        local_vector->z = 0;
        record->heading = record->heading + record->motion_parameter;
        RotMatrix_gte((FieldSVector *) &record->rotation_x, rotation);
        RotMatrixZ(record->rotation_z_16 * 0x10, rotation);
        RotMatrixY(record->rotation_y_16 * 0x10, rotation);
        gte_SetRotMatrix(rotation);
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stsv(rotated_vector);

        /* 0..9 select owner bounds; 10..19 select the current track object bounds. */
        selector = ((FieldPlacementBits *) &part->placement_flags)->opcode;
        switch (selector)
        {
            case 0x0: case 0x1: case 0x2: case 0x3: case 0x4:
            case 0x5: case 0x6: case 0x7: case 0x8: case 0x9:
            case 0xA: case 0xB: case 0xC: case 0xD: case 0xE:
            case 0xF: case 0x10: case 0x11: case 0x12: case 0x13:
            {
                s32 offset_y;
                FieldObjectPlacement *owner;
                FieldObjectPlacement *owner_base;

                if ((s32) selector >= 0xA)
                {
                    slot = actor->track_object_indices[g_field_track_index];
                    selector -= 0xA;
                    reference_record = &D_800FDF58[slot];
                    reference_object = &D_80105AE0[slot];
                }
                else
                {
                    slot = actor->owner_object_index;
                    reference_record = &D_800FDF58[slot];
                    reference_object = &D_80105AE0[slot];
                }
                owner_base = D_80105AE0;
                owner = &owner_base[actor->owner_object_index];
                flags_byte = *(u8 *) &owner->state_flags;
                if ((flags_byte & 1) && ((u8) actor->actor_index >= 0x40U))
                {
                    if (!(((u32) owner->state_flags >> 5) & 1))
                    {
                        offset_y = 0x800000;
                        offset_or_angle = offset_y;
                        selector = -1;
                    }
                    else
                    {
                        offset_y = 0;
                        offset_or_angle = offset_y;
                    }
                }
                else
                {
                    offset_y = 0;
                    offset_or_angle = offset_y;
                }
                switch (selector)
                {
                    case 1:
                        offset_or_angle = (reference_object->bounds_right + reference_object->bounds_left) >> 1;
                        offset_y = (reference_object->bounds_bottom + reference_object->bounds_top) >> 1;
                        break;
                    case 2:
                        offset_y = 0;
                        offset_or_angle = (reference_object->bounds_right + reference_object->bounds_left) >> 1;
                        break;
                    case 3:
                        offset_y = reference_object->bounds_top;
                        offset_or_angle = (reference_object->bounds_right + reference_object->bounds_left) >> 1;
                        break;
                    case 4:
                        offset_or_angle = reference_object->bounds_left;
                        offset_y = (reference_object->bounds_bottom + reference_object->bounds_top) >> 1;
                        break;
                    case 5:
                        offset_or_angle = reference_object->bounds_right;
                        offset_y = (reference_object->bounds_bottom + reference_object->bounds_top) >> 1;
                        break;
                    case 6:
                        offset_or_angle = reference_object->bounds_left;
                        offset_y = reference_object->bounds_top;
                        break;
                    case 7:
                        offset_or_angle = reference_object->bounds_right;
                        offset_y = reference_object->bounds_top;
                        break;
                    case 8:
                        offset_or_angle = reference_object->bounds_left;
                        offset_y = reference_object->bounds_bottom;
                        break;
                    case 9:
                        offset_or_angle = reference_object->bounds_right;
                        offset_y = reference_object->bounds_bottom;
                        break;
                }
                offset_or_angle <<= 8;
                placement_origin->vx = reference_record->x + offset_or_angle;
                offset_y <<= 8;
                placement_origin->vy = reference_record->y + offset_y;
                placement_origin->vz = reference_record->z;
                break;
            }
            case 0x14: case 0x15: case 0x16: case 0x17: case 0x18:
            case 0x19: case 0x1A: case 0x1B: case 0x2A: case 0x2B:
            case 0x2C: case 0x2D: case 0x2E: case 0x2F: case 0x30:
            case 0x31:
            {
                FieldMotionRecord *effect;
                FieldMotionRecord *effect_base;
                effect_base = g_field_effect_records;
                effect = &effect_base[(u16) record->reference_index];
                reference_state = effect->state;
                if (reference_state != FIELD_EFFECT_RETIRED)
                {
                    placement_origin->vx = effect->x;
                    placement_origin->vy = g_field_effect_records[((u16) record->reference_index)].y;
                    placement_origin->vz = g_field_effect_records[((u16) record->reference_index)].z;
                    if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        RotMatrix_gte((FieldSVector *) &g_field_effect_records[((u16) record->reference_index)].rotation_x, rotation);
                        gte_SetRotMatrix(rotation);
                        gte_ldv0(rotated_vector);
                        gte_rtv0();
                        gte_stsv(local_vector);
                        rotated_vector->x = local_vector->x;
                        rotated_vector->y = local_vector->y;
                        rotated_vector->z = local_vector->z;
                    }
                    break;
                }
                record->state = reference_state;
                return;
            }
            case 0x25:
                placement_origin->vx = (part->offset_x << 8) - D_800F22A0;
                placement_origin->vy = (part->offset_y << 8) - D_800F22A4;
                placement_origin->vz = (part->offset_z << 8) - D_800F22A8;
                break;
            case 0x1D:
                placement_origin->vx = 0;
                placement_origin->vy = -0x7000;
                placement_origin->vz = 0;
                break;
            case 0x1E:
                placement_origin->vx = 0;
                placement_origin->vy = 0x7000;
                placement_origin->vz = 0;
                break;
            case 0x1F:
                placement_origin->vx = 0xFFFF6000;
                placement_origin->vy = 0;
                placement_origin->vz = 0;
                break;
            case 0x20:
                placement_origin->vx = 0xA000;
                placement_origin->vy = 0;
                placement_origin->vz = 0;
                break;
            case 0x21:
                placement_origin->vx = 0xFFFF6000;
                placement_origin->vy = -0x7000;
                placement_origin->vz = 0;
                break;
            case 0x22:
                placement_origin->vx = 0xA000;
                placement_origin->vy = -0x7000;
                placement_origin->vz = 0;
                break;
            case 0x23:
                placement_origin->vx = 0xFFFF6000;
                placement_origin->vy = 0x7000;
                placement_origin->vz = 0;
                break;
            case 0x24:
                placement_origin->vx = 0xA000;
                placement_origin->vy = 0x7000;
                placement_origin->vz = 0;
                break;
            case 0x27:
                reference_record = &D_800FDF58[actor->owner_object_index];
                if ((((u32) part->placement_flags >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x + (part->offset_x << 8);
                }
                placement_origin->vy = reference_record->y + (part->offset_y << 8);
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                break;
            case 0x28:
                reference_record = &D_800FDF58[actor->track_object_indices[g_field_track_index]];
                if ((part->spawn_flags.word & 0x08000000) && !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else if ((((u32) part->placement_flags >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x + (part->offset_x << 8);
                }
                placement_origin->vy = reference_record->y + (part->offset_y << 8);
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                break;
            case 0x29:
                reference_record = &D_800FDF58[actor->owner_object_index];
                reference_object = &D_80105AE0[actor->owner_object_index];
                placement_origin->vx = reference_record->x;
                placement_origin->vy = reference_record->y + (reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].y << 8);
                placement_origin->vz = reference_record->z;
                placement_origin->vx += reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8;
                break;
            case 0x32:
                reference_record = &D_800FDF58[actor->owner_object_index];
                reference_object = &D_80105AE0[actor->owner_object_index];
                placement_origin->vx = reference_record->x + (reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8);
                placement_origin->vy = reference_record->y;
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                if ((((u32) part->placement_flags >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else
                {
                    placement_origin->vx += part->offset_x << 8;
                }
                break;
            case 0x33:
                reference_record = &D_800FDF58[actor->owner_object_index];
                reference_object = &D_80105AE0[actor->owner_object_index];
                placement_origin->vx = reference_record->x + (reference_object->ground_attachment_points[((u32) record->flags >> 0xD) & 3].x << 8);
                placement_origin->vy = reference_record->y;
                placement_origin->vz = reference_record->z + (reference_object->ground_attachment_points[((u32) record->flags >> 0xD) & 3].y << 8);
                break;
            case 0x34:
                reference_record = &D_800FDF58[actor->track_object_indices[g_field_track_index]];
                if (!(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x + (actor->track_offsets[g_field_track_index].x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x - (actor->track_offsets[g_field_track_index].x << 8);
                }
                placement_origin->vy = reference_record->y + (actor->track_offsets[g_field_track_index].y << 8);
                placement_origin->vz = reference_record->z;
                break;
            case 0x36:
                placement_origin->vx = part->offset_x << 8;
                placement_origin->vy = part->offset_y << 8;
                placement_origin->vz = part->offset_z << 8;
                break;
            case 0x37: case 0x38: case 0x39: case 0x3A: case 0x3B:
            case 0x3C: case 0x3D: case 0x3E:
            {
                FieldMotionRecord *effect;
                FieldMotionRecord *effect_base;
                effect_base = g_field_effect_records;
                effect = &effect_base[(u16) record->reference_index];
                reference_state = effect->state;
                if (reference_state == FIELD_EFFECT_RETIRED)
                {
                    record->state = reference_state;
                    return;
                }
                placement_origin->vx = effect->x;
                placement_origin->vy = g_field_effect_records[(u16) record->reference_index].y;
                placement_origin->vz = g_field_effect_records[(u16) record->reference_index].z;
                if ((part->spawn_flags.word & 0x08000000) && !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else if ((((u32) part->placement_flags >> 0xA) & 1) && !(g_field_effect_records[(u16) record->reference_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else
                {
                    placement_origin->vx += part->offset_x << 8;
                }
                placement_origin->vy += part->offset_y << 8;
                placement_origin->vz += part->offset_z << 8;
                if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                {
                    RotMatrix_gte(&D_800FF668[(u16) record->reference_index].angles, rotation);
                    gte_SetRotMatrix(rotation);
                    gte_ldv0(rotated_vector);
                    gte_rtv0();
                    gte_stsv(local_vector);
                    rotated_vector->x = local_vector->x;
                    rotated_vector->y = local_vector->y;
                    rotated_vector->z = local_vector->z;
                }
                break;
            }
            default:
                placement_origin->pad = 0;
                placement_origin->vz = 0;
                placement_origin->vy = 0;
                placement_origin->vx = 0;
                break;
            case 0x26:
                break;
        }
        record->x = ((s16) rotated_vector->x << 8) + placement_origin->vx;
        record->y = (((s32) (rotated_vector->y << 0x10)) >> 8) + placement_origin->vy;
        z = (((s32) (rotated_vector->z << 0x10)) >> 8) + placement_origin->vz;
        record->z = z;
        if (part->placement_flags & 1)
        {
            record->z = z + 0x80;
        }
        flags = part->placement_flags;
        if ((flags >> FIELD_PART_HEIGHT_TRACK_BIT) & 1)
        {
            if (part->spawn_flags.word & FIELD_PART_HEIGHT_FROM_BASE)
            {
                record->y = (record->height_or_retired_state - field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) record->age))) << 8;
            }
            else
            {
                record->y = -field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) record->age)) << 8;
            }
        }
        state_or_delta = ((u8 *) &record->flags)[3] & 7;
        switch (state_or_delta)
        {
            case 3:
                record->y = record->y + (((u16) record->age) << 9);
                break;
            case 4:
                record->y = record->y - (((u16) record->age) << 9);
                break;
        }
        if ((((u32) part->behavior_flags >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) && ((record->pitch + part->pitch_acceleration) < 0x800))
        {
            record->pitch = (u16) record->pitch + part->pitch_acceleration;
        }
    }
    else
    {
        {
            u16 speed = record->motion_parameter;
            local_vector->z = 0;
            local_vector->x = 0;
            local_vector->y = -speed << 2;
        }
        RotMatrix_gte((FieldSVector *) &record->rotation_x, rotation);
        if (!(((u32) part->behavior_flags >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) && (record->position_source == 0))
        {
            RotMatrixZ(record->rotation_z_16 * 0x10, rotation);
            RotMatrixY(record->rotation_y_16 * 0x10, rotation);
        }
        gte_SetRotMatrix(rotation);
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stsv(rotated_vector);

        /* Free movement uses a rotated step and a fresh collision probe each update. */
        initial_surface = FIELD_EFFECT_SKIP_SURFACE_PREPASS;
        if ((((record->flags & 0x60000000) != 0x40000000) || ((s16) record->motion_parameter != 0)) && ((((u32) part->behavior_flags >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) || ((s32) part->placement_flags < 0)))
        {
            s32 pitch_cosine;
            s16 adjusted_pitch;

            pitch_cosine = rcos(record->pitch) - (part->pitch_acceleration * 8);
            adjusted_pitch = ratan2(rsin(record->pitch), pitch_cosine);
            record->pitch = adjusted_pitch;
            if (adjusted_pitch < 0x400)
            {
                record->motion_parameter = (((s16) record->motion_parameter * 0xF) >> 4) - 1;
            }
            else
            {
                record->motion_parameter = (((s16) record->motion_parameter << 5) / 30) + 1;
            }
            if (((s16) record->motion_parameter < 0x14) && (record->pitch < 0x400))
            {
                record->pitch = 0x800 - (u16) record->pitch;
                record->motion_parameter = 0x14;
            }
        }

        if ((part->effect_flags & FIELD_PART_MAP_COLLISION) && !(part->spawn_flags.word & FIELD_PART_SKIP_MAP_COLLISION))
        {
            z = record->x;
            if ((z < 0) || (z >= (map_bounds->width << 8)) || ((z = record->z), (z < 0)) || (z >= ((s32) (map_bounds->height << 0x10) >> 7)))
            {
                if (!(part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION))
                {
                    rotated_vector->x = 0;
                    rotated_vector->z = 0;
                }
                else
                {
                    record->state = FIELD_EFFECT_RETIRED;
                }
            }
            else
            {
                s32 position_z;
                s32 position_x;
                /* Advance the packed context during a collision attempt; protocol remains unresolved. */
                g_field_action_context += 0x100;
                mover->x = record->x;
                mover->y = 0;
                mover->z = record->z;
                mover->move_x = rotated_vector->x;
                mover->move_y = 0;
                mover->move_z = rotated_vector->z;
                mover->width = FIELD_EFFECT_COLLISION_WIDTH;
                mover->mode.bits.depth = FIELD_EFFECT_COLLISION_DEPTH;
                mover->height_tolerance = FIELD_EFFECT_COLLISION_HEIGHT;
                mover->collision_node = initial_surface;
                mover->contact_flags = 0;
                mover->mode.bits.unknown_17 = 0;
                mover->mode.bits.unknown_16 = 0;
                if (func_8005B6AC(mover) & 3)
                {
                    rotated_vector->z = 0;
                    rotated_vector->x = 0;
                    if (part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION)
                    {
                        record->state = FIELD_EFFECT_RETIRED;
                    }
                }
                else if (!(part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION))
                {
                    position_x = mover->x;
                    query->width = FIELD_EFFECT_COLLISION_WIDTH;
                    query->height_tolerance = FIELD_EFFECT_COLLISION_HEIGHT;
                    do
                    {
                        query->depth = FIELD_EFFECT_COLLISION_DEPTH;
                    } while (0);
                    query->x = position_x;
                    position_z = mover->z;
                    {
                        s32 query_y = record->y;
                        query->z = position_z;
                        query->y = query_y;
                    }
                    if ((D_800FE754 != 0) && (func_8005B368(query) != -1))
                    {
                        rotated_vector->z = 0;
                        rotated_vector->x = 0;
                    }
                    else
                    {
                        rotated_vector->x = (u16) mover->x - (u16) record->x;
                        rotated_vector->z = (u16) mover->z - (u16) record->z;
                    }
                }
            }
        }

        if (part->spawn_flags.word & FIELD_PART_CAMERA_BOUNDS)
        {
            s32 camera_x;
            s32 next_z;
            dx = record->x + (s16) rotated_vector->x;
            camera_x = -camera->x;
            if (((camera_x + FIELD_EFFECT_CAMERA_X_MIN) < dx) && (dx < (camera_x + FIELD_EFFECT_CAMERA_X_MAX)))
            {
                next_z = record->z + (s16) rotated_vector->z;
                x = -camera->z;
                if ((x < next_z) && (next_z < (x + FIELD_EFFECT_CAMERA_Z_SPAN)))
                {
                    record->x = dx;
                    record->z = record->z + (s16) rotated_vector->z;
                }
            }
        }
        else
        {
            record->x = record->x + (s16) rotated_vector->x;
            record->z = record->z + (s16) rotated_vector->z;
        }
        {
            u32 ground_flags;
            y = record->y + (s16) rotated_vector->y;
            record->y = y;
            if ((part->spawn_flags.word & FIELD_PART_GROUND_STOP) && (y >= 0) && (((ground_flags = record->flags, state_or_delta = ground_flags & 0x60000000), (state_or_delta == 0)) || (state_or_delta == 0x40000000)))
            {
                record->flags = ground_flags & 0x9FFFFFFF;
                record->y = 0;
                record->motion_parameter = 0;
            }
        }
        flags = part->placement_flags;
        if ((flags >> FIELD_PART_HEIGHT_TRACK_BIT) & 1)
        {
            if (part->spawn_flags.word & FIELD_PART_HEIGHT_FROM_BASE)
            {
                record->y = (record->height_or_retired_state - field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) record->age))) << 8;
            }
            else
            {
                record->y = -field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) record->age)) << 8;
            }
        }

        /* Resolve a target and either stop nearby or steer the heading and pitch. */
        if (record->position_source != 0)
        {
            s32 target_dy, target_dz;

            field_resolve_effect_position(record, part, target_position);
            work_vector->vx = (record->x - target_position->vx) >> 8;
            work_vector->vy = (record->y - target_position->vy) >> 8;
            target_dz = (record->z - target_position->vz) >> 8;
            work_vector->vz = target_dz;
            if (((u32) (work_vector->vx + 0xF) < 0x1FU) && (target_dz >= -0xF) && (target_dz < 0x10))
            {
                target_dy = work_vector->vy;
                if ((target_dy >= -0xF) && (work_vector->vy < 0x10))
                {
                    if ((((u32) part->behavior_flags >> 4) & 3) == 2)
                    {
                        retired_state = record->state;
                        record->state = FIELD_EFFECT_RETIRED;
                        record->height_or_retired_state = (s8) retired_state;
                        return;
                    }
                    if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        record->motion_parameter = 0;
                        record->flags = record->flags & 0x9FFFFFFF;
                    }
                }
            }
            if (!(record->flags & FIELD_EFFECT_MOTION_KIND_MASK) && !(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
            {
                FieldVector new_pos;
                FieldVector delta;
                FieldVector delta_squared;
                s16 target_heading;

                field_resolve_effect_position(record, part, &new_pos);
                delta.vx = (new_pos.vx - record->x) >> 8;
                delta.vy = (new_pos.vy - record->y) >> 8;
                delta.vz = (new_pos.vz - record->z) >> 8;
                gte_ldlvl(&delta);
                gte_sqr0();
                gte_stlvnl(&delta_squared);
                target_heading = ratan2(-delta.vz, delta.vx);
                record->heading = target_heading;
                if (target_heading < 0)
                {
                    record->heading = target_heading + FIELD_ANGLE_TURN;
                }
                if (delta.vy != 0)
                {
                    record->pitch = ratan2(SquareRoot0(delta_squared.vx + delta_squared.vz), -delta.vy);
                }
                else
                {
                    record->pitch = FIELD_ANGLE_QUARTER_TURN;
                }
                if (record->pitch < 0)
                {
                    record->pitch = (u16) record->pitch + FIELD_ANGLE_TURN;
                }
                record->rotation_x = 0;
            }
            if ((record->flags & FIELD_EFFECT_MOTION_KIND_MASK) == FIELD_EFFECT_MOTION_HOMING)
            {

                s16 next_heading;

                gte_ldlvl(work_vector);
                gte_sqr0();
                gte_stlvnl(target_position);
                offset_or_angle = ratan2(work_vector->vz, -work_vector->vx);
                if (offset_or_angle < 0)
                {
                    offset_or_angle += FIELD_ANGLE_TURN;
                }
                if (((u16) record->age) == part->turn_end_age)
                {
                    record->flags = record->flags & 0xF8FFFFFF;
                    if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        record->heading = (u16) offset_or_angle;
                        if (work_vector->vz != 0)
                        {
                            record->pitch = ratan2(SquareRoot0(target_position->vx + target_position->vz), work_vector->vy);
                        }
                        else
                        {
                            record->pitch = FIELD_ANGLE_QUARTER_TURN;
                        }
                        if (record->pitch < 0)
                        {
                            record->pitch = (u16) record->pitch + FIELD_ANGLE_TURN;
                        }
                    }
                }
                else
                {
                    current_angle = (s16) record->heading;
                    selector = (offset_or_angle - current_angle) & FIELD_ANGLE_MASK;
                    if ((u32) (selector - 8) >= 0xFF1U)
                    {
                        record->heading = (u16) offset_or_angle;
                    }
                    else
                    {
                        if (selector >= 0x801)
                        {
                            s32 reverse_turn = FIELD_ANGLE_TURN - selector;
                            if (reverse_turn < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                next_heading = current_angle - (reverse_turn >> 2);
                            }
                            else
                            {
                                next_heading = current_angle - FIELD_EFFECT_TURN_STEP;
                            }
                        }
                        else if (selector < FIELD_EFFECT_TURN_THRESHOLD)
                        {
                            next_heading = current_angle + (selector >> 2);
                        }
                        else
                        {
                            next_heading = current_angle + FIELD_EFFECT_TURN_STEP;
                        }
                        record->heading = next_heading;
                    }
                    if ((s16) record->heading < 0)
                    {
                        record->heading = (u16) record->heading + FIELD_ANGLE_TURN;
                    }
                    {
                        offset_or_angle = ratan2(SquareRoot0(target_position->vx + target_position->vz), work_vector->vy);
                        current_angle = record->pitch;
                        if (offset_or_angle < current_angle)
                        {
                            state_or_delta = current_angle - offset_or_angle;
                            if (state_or_delta < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                record->pitch = current_angle - (state_or_delta >> 2);
                            }
                            else
                            {
                                record->pitch = current_angle - FIELD_EFFECT_TURN_STEP;
                            }
                        }
                        else if (current_angle < offset_or_angle)
                        {
                            state_or_delta = offset_or_angle - current_angle;
                            if (state_or_delta < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                record->pitch = current_angle + (state_or_delta >> 2);
                            }
                            else
                            {
                                record->pitch = current_angle + FIELD_EFFECT_TURN_STEP;
                            }
                        }
                        if (record->pitch < 0)
                        {
                            record->pitch = (u16) record->pitch + FIELD_ANGLE_TURN;
                        }
                        record->rotation_x = 0;
                    }
                }
            }
        }
    }
    /* After the pickup delay, find a nearby recipient and consume the reward effect. */
    if (!(actor->action_flags & 1))
    {
        state_or_delta = ((u16 *) &actor->action_flags)[1];
        switch (state_or_delta)
        {
            case FIELD_PICKUP_EXPERIENCE_OR_CURRENCY:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(record, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;
                        s32 counter_slot;
                        FieldRewardCounterView *counter_base;
                        u32 counter_index;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, record->facing_or_reward_kind - 0x16);
                        counter_base = D_800FD818;
                        counter_index = record->facing_or_reward_kind;
                        counter_slot = recipient_index < 3 ? recipient_index : 2;
                        counter_base[counter_slot].counters[counter_index] = counter_base[recipient_index < 3 ? recipient_index : 2].counters[counter_index] + 1;
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, record->facing_or_reward_kind - 0x16);
                        field_release_actor_if_no_effects(record);
                        return;
                    }
                }
                break;
            case FIELD_PICKUP_ITEM:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(record, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 4);
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 4);
                        field_release_actor_if_no_effects(record);
                        return;
                    }
                }
                break;
            /* Restore 64/256 of maximum capacity, then play the quarter-restore animation. */
            case FIELD_PICKUP_RESTORE_QUARTER:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(record, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 5);
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 5);
                        func_80092C24(&D_800FDF58[recipient_index], 0x2C);
                        field_release_actor_if_no_effects(record);
                        return;
                    }
                }
                break;
            /* Restore 128/256 of maximum capacity. */
            case FIELD_PICKUP_RESTORE_HALF:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(record, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 6);
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 6);
                        func_80092C24(&D_800FDF58[recipient_index], 0x2D);
                        field_release_actor_if_no_effects(record);
                        return;
                    }
                }
                break;
            default:
                break;
        }
    }
    /* Collect effect-centered hits, reflect below-ground motion, then scale speed. */
    {
        FieldActorAnimationDef *anim = actor->animation;
        if ((anim->hit_test_mode == FIELD_HIT_TEST_EFFECT_BOUNDS) && (anim->hit_test_part == record->part_index))
        {
            field_collect_effect_hits(record, anim->hit_radius, actor);
        }
    }
    if (((u32) part->behavior_flags >> FIELD_PART_GROUND_BOUNCE_BIT) & 1)
    {
        s32 ground_y = record->y;
        if (ground_y > 0)
        {
            s32 old_pitch;
            s16 old_motion;

            record->y = -ground_y;
            old_pitch = (u16) record->pitch;
            old_motion = record->motion_parameter;
            record->pitch = FIELD_ANGLE_HALF_TURN - old_pitch;
            record->motion_parameter = old_motion / 2;
            field_dispatch_actor_audio_event(actor, 4, record->part_index, old_pitch);
        }
    }
    record->motion_parameter = (record->motion_parameter * record->motion_scale) >> 8;
}


extern FieldActorState g_field_actor_slots[80];
extern FieldMotionRecord D_800FDF58[];
extern FieldMotionRecord g_field_effect_records[];
extern FieldObjectPlacement D_80105AE0[];
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern s32 g_field_track_index;
extern s32 g_field_action_context;

/**
 * @brief Release an effect's owning actor after its last live effect retires.
 * @param effect Motion record whose actor_index identifies the owning actor slot.
 * @note Callers retire the effect before this check. Other live effects keep the
 * actor active; this function does not retire records or free their storage.
 */
void field_release_actor_if_no_effects(FieldMotionRecord *effect)
{
    FieldActorState *actor_slots;
    FieldActorState *actor_slot;

    if (field_actor_has_live_effects(effect->actor_index) == 0)
    {
        actor_slots = g_field_actor_slots;
        actor_slot = &actor_slots[effect->actor_index];
        actor_slot->is_active = 0;
        actor_slot->unknown_0x23b = 0;
        actor_slot->active_track_mask = 0;
    }
}

/**
 * @brief Check the effect pool for an unretired record owned by an actor.
 * @param actor_index Actor slot index to compare with each record's owner.
 * @return 1 if a matching live effect exists, otherwise 0.
 */
s32 field_actor_has_live_effects(s32 actor_index)
{
    s32 effect_index;
    s32 retired_state;
    FieldMotionRecord *effect;

    effect_index = 0;
    retired_state = FIELD_EFFECT_RETIRED;
    effect = g_field_effect_records;
    for (; effect_index < FIELD_EFFECT_POOL_COUNT; effect_index++)
    {
        if (effect->state != retired_state && effect->actor_index == actor_index)
        {
            return 1;
        }
        effect++;
    }
    return 0;
}

/**
 * @brief Pack recipient, source, and action identifiers into shared action context.
 * @param recipient_id Identifier placed starting at bit 16.
 * @param source_id Identifier placed starting at bit 8.
 * @param action Action/reward selector placed in the low bits.
 * @note Inputs are not masked here; preserve their original word-wide behavior.
 */
void field_set_action_context(s32 recipient_id, s32 source_id, s32 action)
{
    g_field_action_context = (recipient_id << 0x10) | (source_id << 8) | action;
}

/**
 * @brief Resolve an effect's selected target/attachment position in world coordinates.
 * @param effect Record selecting a source and supplying saved positions or effect links.
 * @param part Part definition controlling anchor choice, placement, and facing updates.
 * @param position Destination X/Y/Z; its pad word is untouched.
 * @note Sources 0 and values above 15 leave out unchanged. Source 9 can update
 * the record's facing bit. References at +0x30 use an unsigned halfword read.
 * The dispatch table retains its trailing null word at jtbl_80050144.
 */
void field_resolve_effect_position(FieldMotionRecord *effect, FieldActorPartDef *part, FieldVector *position)
{
    FieldMotionRecord *source_record;
    FieldMotionRecord *opposite_record;
    FieldMotionRecord *track_record;
    FieldMotionRecord *owner_record;
    FieldMotionRecord *linked_record;
    FieldObjectPlacement *source_object;
    FieldActorState *actors;
    FieldActorState *owner_actors;
    s32 object_index;
    s32 source_index;
    s32 owner_index;
    s32 placement;
    s32 delta_x;
    s32 position_x;
    s32 source_x;
    s32 horizontal_offset;
    s32 dispatch;
    static void *const dispatch_table[] =
    {
        &&track_object, &&owner_object, &&saved, &&reference_effect,
        &&owner_attachment, &&facing_offset, &&track_stored_xz, &&linked_effect,
        &&relative_side_offset, &&owner_bounds_center, &&track_bounds_center,
        &&reflect_track_x, &&reflect_track_x, &&extend_track_xz, &&extend_link_xz, 0
    };

    dispatch = effect->position_source - FIELD_POSITION_TRACK_OBJECT;
    if ((u32) dispatch >= FIELD_POSITION_EXTEND_LINK_XZ)
    {
        return;
    }
    goto *dispatch_table[dispatch];

track_object:
    actors = g_field_actor_slots;
    position->vx = D_800FDF58[actors[effect->actor_index].track_object_indices[g_field_track_index]].x;
    position->vy = D_800FDF58[actors[effect->actor_index].track_object_indices[g_field_track_index]].y;
    object_index = actors[effect->actor_index].track_object_indices[g_field_track_index];
    do
    {
        track_record = &D_800FDF58[object_index];
    } while (0);
    position->vz = track_record->z;
    return;
owner_object:
    owner_actors = g_field_actor_slots;
    position->vx = D_800FDF58[owner_actors[effect->actor_index].owner_object_index].x;
    position->vy = D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].y;
    owner_index = owner_actors[effect->actor_index].owner_object_index;
    do
    {
        owner_record = &D_800FDF58[owner_index];
    } while (0);
    position->vz = owner_record->z;
    return;
saved:
    position->vx = effect->work_x - D_800F22A0;
    position->vy = effect->work_y - D_800F22A4;
    position->vz = effect->work_z - D_800F22A8;
    return;
reference_effect:
    if (g_field_effect_records[(u16) effect->reference_index].state != FIELD_EFFECT_RETIRED)
    {
        position->vx = g_field_effect_records[(u16) effect->reference_index].x;
        position->vy = g_field_effect_records[(u16) effect->reference_index].y;
        position->vz = g_field_effect_records[(u16) effect->reference_index].z;
        return;
    }
    position->vx = effect->x;
    position->vy = effect->y;
    position->vz = effect->z;
    return;
owner_attachment:
    source_record = &D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index];
    source_object = &D_80105AE0[g_field_actor_slots[effect->actor_index].owner_object_index];
    position->vx = source_record->x + (source_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8);
    position->vy = source_record->y + (source_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].y << 8);
    position->vz = source_record->z;
    return;
facing_offset:
    placement = ((u32) part->placement_flags >> 0x12) & 0x3F;
    if (placement >= 0xA)
    {
        if (placement < 0x26)
        {
            source_index = g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index];
        }
        else
        {
            source_index = g_field_actor_slots[effect->actor_index].owner_object_index;
        }
    }
    else
    {
        source_index = g_field_actor_slots[effect->actor_index].owner_object_index;
    }
    source_record = &D_800FDF58[source_index];
    if (source_record->facing_or_reward_kind & 0x80)
    {
        position_x = effect->work_x;
        source_x = source_record->x;
        position_x = position_x + source_x;
    }
    else
    {
        source_x = effect->work_x;
        position_x = -source_x;
        position_x += source_record->x;
    }
    position->vx = position_x;
    position->vy = effect->work_y + source_record->y;
    position->vz = effect->work_z + source_record->z;
    return;
track_stored_xz:
    position->vx = D_80105AE0[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].unknown_0x6c << 8;
    position->vz = D_80105AE0[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].unknown_0x6e << 8;
    position->vy = 0;
    return;
linked_effect:
    position->vx = g_field_effect_records[effect->position_data.linked_effect_index].x;
    position->vy = g_field_effect_records[effect->position_data.linked_effect_index].y;
    linked_record = &g_field_effect_records[effect->position_data.linked_effect_index];
    position->vz = linked_record->z;
    return;
relative_side_offset:
    placement = ((u32) part->placement_flags >> 0x12) & 0x3F;
    if (placement < 0xA)
    {
        source_record = &D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index];
        opposite_record = &D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]];
    }
    else
    {
        if (placement < 0x26)
        {
            object_index = g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index];
            source_record = &D_800FDF58[object_index];
        }
        else
        {
            object_index = g_field_actor_slots[effect->actor_index].owner_object_index;
            source_record = &D_800FDF58[object_index];
        }
        opposite_record = &D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index];
    }
    if (source_record->x > opposite_record->x)
    {
        if (((part->placement_flags >> 0xA) & 1) || (part->spawn_flags.word & 0x08000000))
        {
            effect->facing_or_reward_kind = effect->facing_or_reward_kind & 0x7F;
        }
        horizontal_offset = effect->work_x;
        position_x = source_record->x - horizontal_offset;
    }
    else
    {
        if (((part->placement_flags >> 0xA) & 1) || (part->spawn_flags.word & 0x08000000))
        {
            effect->facing_or_reward_kind = effect->facing_or_reward_kind | 0x80;
        }
        position_x = effect->work_x;
        source_x = source_record->x;
        position_x = position_x + source_x;
    }
    position->vx = position_x;
    position->vy = effect->work_y + source_record->y;
    position->vz = effect->work_z + source_record->z;
    return;
owner_bounds_center:
    source_object = &D_80105AE0[g_field_actor_slots[effect->actor_index].owner_object_index];
    position->vx = D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].x + ((source_object->bounds_right + source_object->bounds_left) << 7);
    position->vy = D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].y + ((source_object->bounds_bottom + source_object->bounds_top) << 7);
    position->vz = D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].z;
    return;
track_bounds_center:
    source_object = &D_80105AE0[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]];
    position->vx = D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].x + ((source_object->bounds_right + source_object->bounds_left) << 7);
    position->vy = D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].y + ((source_object->bounds_bottom + source_object->bounds_top) << 7);
    position->vz = D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].z;
    return;
reflect_track_x:
    delta_x = D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].x - D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].x;
    if (effect->position_source == FIELD_POSITION_REFLECT_TRACK_X)
    {
        position->vx = D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].x - delta_x;
    }
    else
    {
        position->vx = D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].x + delta_x;
    }
    position->vy = D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].y;
    position->vz = D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].z;
    return;
extend_track_xz:
    position->vx = (D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].x * 2) - D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].x;
    position->vy = D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].y;
    position->vz = (D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].z * 2) - D_800FDF58[g_field_actor_slots[effect->actor_index].owner_object_index].z;
    return;
extend_link_xz:
    position->vx = (D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].x * 2) - g_field_effect_records[effect->position_data.linked_effect_index].x;
    position->vy = D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].y;
    position->vz = (D_800FDF58[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].z * 2) - g_field_effect_records[effect->position_data.linked_effect_index].z;
    return;
}
