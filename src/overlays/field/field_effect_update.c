/**
 * @file field_effect_update.c
 * @brief Spawn, update, position, retire, and dispatch rendering of field actor effects.
 */

#include "common.h"
#include "field_effect_transform.h"
#include "field_effect_render_state.h"
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
    s16 motion_divisor;
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
    u8 motion_remainder;
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
    u8 tint_red;
    u8 tint_green;
    u8 tint_blue;
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

extern FieldSpawnPartDef g_field_object_parts[];
extern FieldSpawnTrackBinding g_field_actor_bindings[];
extern VECTOR D_80105778;
extern s32 g_field_action_context;

extern s32 D_80105760;
extern s32 D_80105770;
extern u8 *D_801058D4;
extern s32 g_field_track_index;

/**
 * @brief Look up the track binding that serves an object slot.
 * @param object_index Object slot; slots 0 and 1 are the players, every other slot shares entry 2.
 * @return Binding record for the slot.
 */
static inline FieldSpawnTrackBinding *field_get_track_binding(s32 object_index)
{
    FieldSpawnTrackBinding *bindings = g_field_actor_bindings;
    s32 offset = (u8) object_index < 2 ? object_index * sizeof(FieldSpawnTrackBinding) : 2 * sizeof(FieldSpawnTrackBinding);

    return (FieldSpawnTrackBinding *) ((u8 *) bindings + offset);
}

/**
 * @brief Spawn an effect record for one actor part and resolve its initial placement.
 * @param actor Actor spawning the effect.
 * @param part_index Part definition to instantiate.
 * @param start First effect slot to inspect when searching for a related effect.
 * @return Spawned effect index, or -1 when the effect cannot be created.
 */
s32 func_8006D79C(FieldSpawnActorState* actor, s32 part_index, s32 start)
{
    extern FieldSpawnMotionRecord g_field_actors[];
    extern FieldSpawnMotionRecord g_field_effect_records[];
    extern FieldSpawnObjectPlacement g_field_object_states[];
    extern FieldSpawnActorState g_field_actor_slots[];
    s32 half_turn_8bit;
    VECTOR* direction_vector = (VECTOR*) FIELD_EFFECT_ORIGIN_ADDRESS;
    VECTOR* squared_vector = (VECTOR*) FIELD_EFFECT_VECTOR_ADDRESS;
    FieldSpawnVector* local_direction = (FieldSpawnVector*) FIELD_EFFECT_LOCAL_VECTOR_ADDRESS;
    MATRIX* rotation_matrix = (MATRIX*) FIELD_EFFECT_MATRIX_ADDRESS;
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
    s32 binding_index;
    u8 track_object_index;
    u8 *resource;

    effect_index = 0;
    work_value = 0xFF;
    record_base = g_field_effect_records;
    free_effect = record_base;
    for (; effect_index < 0x100; effect_index++, free_effect++)
    {
        if (free_effect->state == work_value)
        {
            break;
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

    if (((part->placement_flags >> 18) & 0x3F) >= 0x2A && ((part->placement_flags >> 18) & 0x3F) < 0x32 &&
        ((part->placement_flags >> 18) & 0x3F) - 0x22 == part_index)
    {
        return -1;
    }
    if (((part->placement_flags >> 18) & 0x3F) >= 0x14 && ((part->placement_flags >> 18) & 0x3F) < 0x1C &&
        ((part->placement_flags >> 18) & 0x3F) - 0x14 == part_index)
    {
        return -1;
    }
    if (((part->placement_flags >> 18) & 0x3F) >= 0x37 && ((part->placement_flags >> 18) & 0x3F) < 0x3F &&
        ((part->placement_flags >> 18) & 0x3F) - 0x37 == part_index)
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
    }
    else
    {
        effect->flags.word = (effect->flags.word & 0xFF7FFFFF) | (((part->behavior_flags >> 1) & 1) << 23);
    }
    effect->flags.word = effect->flags.word & 0xFFFCFFFF;
    if (effect->position_source == 8)
    {
        s32 scan_ff;
        work_index = 0;
        work_limit = work_index;
        scan_ff = 0xFF;
        work_value = actor->active_counts[g_field_track_index][part_index];
        for (; work_index < 0x100; work_index++)
        {
            if (g_field_effect_records[work_index].state != scan_ff && g_field_effect_records[work_index].part_index == part->unknown_0x46 && g_field_effect_records[work_index].actor_index == actor->actor_index)
            {
                work_limit = 1;
                if (work_value == 0)
                {
                    effect->position_data = work_index;
                    break;
                }
                effect->position_data = work_index;
                work_value--;
            }
        }
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
                object_table_init = g_field_object_states;
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
        !(g_field_actors[actor->owner_object_index].facing_or_reward_kind & 0x80))
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
    work_index = field_resolve_effect_extent(actor, part);
    effect->flags.word = (effect->flags.word & ~0x1FF) | (work_index & 0x1FF);
    effect->x = (work_index * direction_vector->vx) >> 4;
    effect->y = (work_index * direction_vector->vy) >> 4;
    effect->z = (work_index * direction_vector->vz) >> 4;
    effect->unknown_0x24 = part->unknown_0x8;
    effect->source_object_index = actor->owner_object_index;
    if (effect->state == 0)
    {
        field_begin_actor_animation_forward(effect, D_801058D4);
    }
    else if (effect->state == 1 && (resource = g_field_actor_slots[effect->actor_index].track_data) != 0)
    {
        field_begin_actor_animation_forward(effect, resource);
    }
    else if (effect->state == 2)
    {
        { FieldSpawnObjectPlacement *owner_slots = g_field_object_states;
          u8 owner_index = actor->owner_object_index;
        owner_object_state = &owner_slots[owner_index]; }
        if ((owner_object_state->state_flags.bytes[0] & 1) && owner_object_state->state_flags.bytes[2] != actor->actor_index)
        {
            effect->state = 0xFF;
        }
        else
        {
            record_base = &g_field_actors[*(u8*)&actor->owner_object_index];
            if (!((part->behavior_flags >> 11) & 1) && !((part->placement_flags >> 25) & 1) && (part->unknown_0x2c >> 5) == 0 &&
                (*(u32*)&part->unknown_0xc & 0xFFFF0000) == 0x80800000 && part->unknown_0x10 == 0x80)
            {
                effect->flags.word |= 0x10008000;
                effect->unknown_0x18 = g_field_object_parts[record_base->source_object_index].unknown_0xe;
                effect->unknown_0x19 = g_field_object_parts[record_base->source_object_index].unknown_0xf;
                effect->unknown_0x1a = g_field_object_parts[record_base->source_object_index].unknown_0x10;
            }
            effect->source_object_index = g_field_actors[actor->owner_object_index].source_object_index;
            effect->unknown_0x3b = g_field_actors[actor->owner_object_index].unknown_0x3b;
            effect->unknown_0xc = g_field_actors[actor->owner_object_index].unknown_0xc;
            effect->facing_or_reward_kind |= g_field_actors[actor->owner_object_index].facing_or_reward_kind & 0x80;
            effect->flags.word = (effect->flags.word & 0xFFFCFFFF) |
                         ((g_field_actors[actor->owner_object_index].flags.half[1] & 3) << 16);
            effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (g_field_actors[actor->owner_object_index].flags.word & 0x780000);
            resource = g_field_resource_entries[g_field_actors[actor->owner_object_index].unknown_0x3b].start;
            if (resource != 0)
            {
                field_restart_actor_animation(effect, resource);
            }
        }
    }
    else if (effect->state == 3)
    {
        track_object_index = actor->track_object_indices[g_field_track_index];
        if (track_object_index == 0xFF)
        {
            effect->state = track_object_index;
            actor->active_counts[g_field_track_index][part_index]--;
            actor->track_counters[g_field_track_index][part_index]--;
            return -1;
        }
        if ((!((g_field_object_states[actor->track_object_indices[g_field_track_index]].state_flags.word >> 6) & 1) &&
             (binding_index = field_get_track_binding(actor->track_object_indices[g_field_track_index])->track_index) == actor->track_object_indices[g_field_track_index] &&
             field_get_track_binding(binding_index)->state != 0) ||
            ((g_field_object_states[actor->track_object_indices[g_field_track_index]].state_flags.bytes[0] & 1) &&
             g_field_object_states[actor->track_object_indices[g_field_track_index]].state_flags.bytes[2] != actor->actor_index))
        {
            effect->state = 0xFF;
        }
        else
        {
            record_base = &g_field_actors[actor->track_object_indices[g_field_track_index]];
            if (!((part->behavior_flags >> 11) & 1) && !((part->placement_flags >> 25) & 1) && (part->unknown_0x2c >> 5) == 0 &&
                (*(u32*)&part->unknown_0xc & 0xFFFF0000) == 0x80800000 && part->unknown_0x10 == 0x80)
            {
                effect->flags.word |= 0x10008000;
                effect->unknown_0x18 = g_field_object_parts[record_base->source_object_index].unknown_0xe;
                effect->unknown_0x19 = g_field_object_parts[record_base->source_object_index].unknown_0xf;
                effect->unknown_0x1a = g_field_object_parts[record_base->source_object_index].unknown_0x10;
            }
            effect->source_object_index = g_field_actors[actor->track_object_indices[g_field_track_index]].source_object_index;
            effect->unknown_0x3b = g_field_actors[actor->track_object_indices[g_field_track_index]].unknown_0x3b;
            effect->unknown_0xc = g_field_actors[actor->track_object_indices[g_field_track_index]].unknown_0xc;
            effect->facing_or_reward_kind |= g_field_actors[actor->track_object_indices[g_field_track_index]].facing_or_reward_kind & 0x80;
            effect->flags.word = (effect->flags.word & 0xFFFCFFFF) |
                         ((g_field_actors[actor->track_object_indices[g_field_track_index]].flags.half[1] & 3) << 16);
            effect->flags.word =
                (effect->flags.word & 0xFF87FFFF) | (g_field_actors[actor->track_object_indices[g_field_track_index]].flags.word & 0x780000);
            resource = g_field_resource_entries[g_field_actors[actor->track_object_indices[g_field_track_index]].unknown_0x3b].start;
            if (resource != 0)
            {
                field_restart_actor_animation(effect, resource);
            }
        }
    }

    if ((part->track_flags >> 13) & 1)
    {
        effect->motion_scale = field_evaluate_parameter_track(actor, part->unknown_0x18 & 0xF);
    }
    if ((((part->placement_flags >> 10) & 1) || (part->spawn_flags & 0x08000000)) && effect->position_source != 0 &&
        !(g_field_actors[actor->owner_object_index].facing_or_reward_kind & 0x80))
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
source_record = &g_field_actors[object_index]; placement_object = &g_field_object_states[object_index];
        }
        else
        {
            {
                FieldSpawnObjectPlacement *table_base = g_field_object_states;
                owner_placement_guard = &table_base[actor->owner_object_index];
            }
            if ((owner_placement_guard->state_flags.bytes[0] & 1) && actor->actor_index >= 0x40U &&
                !(((u32)owner_placement_guard->state_flags.word >> 5) & 1) && owner_placement_guard->state_flags.bytes[2] != actor->actor_index)
            {
                effect->state = 0xFF;
                actor->active_counts[g_field_track_index][part_index]--;
                actor->track_counters[g_field_track_index][part_index]--;
                return -1;
            }
            object_index = actor->owner_object_index;
source_record = &g_field_actors[object_index]; placement_object = &g_field_object_states[object_index];
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
            object_table = g_field_object_states;
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
                    effect->unknown_0x18 = g_field_object_parts[source_record->source_object_index].unknown_0xe;
                    effect->unknown_0x19 = g_field_object_parts[source_record->source_object_index].unknown_0xf;
                    effect->unknown_0x1a = g_field_object_parts[source_record->source_object_index].unknown_0x10;
                }
                if (effect->facing_or_reward_kind == 0xFF)
                {
                    effect->facing_or_reward_kind = source_record->facing_or_reward_kind;
                    effect->saved_state = source_record->saved_state;
                    part->footprint_scale_x = g_field_object_parts[source_record->source_object_index].footprint_scale_x;
                    part->footprint_scale_y = g_field_object_parts[source_record->source_object_index].footprint_scale_y;
                    effect->unknown_0x34 = source_record->unknown_0x34;
                    effect->unknown_0x35 = source_record->unknown_0x35;
                    effect->track_index = source_record->track_index;
                    effect->motion_remainder = source_record->motion_remainder;
                    effect->unknown_0x37 = source_record->unknown_0x37;
                    effect->unknown_0x38 = source_record->unknown_0x38;
                    effect->motion_divisor = source_record->motion_divisor;
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
                effect->state = 0xFE;
                break;
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
        if (placement_kind < 0x2A)
        {
            sibling_part_index = placement_kind - 0x14;
        }
        else
        {
            sibling_part_index = placement_kind - 0x22;
        }
        for (work_a = start; work_a < 0x100; work_a++)
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
                    effect->facing_or_reward_kind = g_field_effect_records[work_a].facing_or_reward_kind;
                    effect->saved_state = g_field_effect_records[work_a].saved_state;
                    effect->unknown_0x34 = g_field_effect_records[work_a].unknown_0x34;
                    effect->unknown_0x35 = g_field_effect_records[work_a].unknown_0x35;
                    effect->track_index = g_field_effect_records[work_a].track_index;
                    effect->motion_remainder = g_field_effect_records[work_a].motion_remainder;
                    effect->unknown_0x37 = g_field_effect_records[work_a].unknown_0x37;
                    effect->unknown_0x38 = g_field_effect_records[work_a].unknown_0x38;
                        break;
                    }
                    effect->source_object_index = g_field_effect_records[work_a].source_object_index;
                    effect->state = g_field_effect_records[work_a].state;
                    effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (((u32)g_field_effect_records[work_a].flags.word >> 19 & 15) << 19);
                    effect->flags.word = (effect->flags.word & 0xFFFCFFFF) | ((((u32)g_field_effect_records[work_a].flags.word >> 16) & 3) << 16);
                    effect->saved_state = g_field_effect_records[work_a].saved_state;
                    effect->facing_or_reward_kind = g_field_effect_records[work_a].facing_or_reward_kind;
                    resource = g_field_resource_entries
                              [g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].unknown_0x3b]
                                  .start;
                    if (resource != 0)
                    {
                        field_begin_actor_animation_forward(effect, resource);
                    }
                }
                break;
            }
        }
        if (work_a == 0x100)
        {
            effect->state = 0xFF;
            actor->active_counts[g_field_track_index][part_index]--;
            actor->track_counters[g_field_track_index][part_index]--;
            return -1;
        }
        func_8006D79C(actor, part_index, work_a + 1);
        break;

    case 0x25:
        effect->x += (part->offset_x << 8) - g_field_view_offset_x;
        effect->y += (part->offset_y << 8) - g_field_view_offset_y;
        effect->z += (part->offset_z << 8) - g_field_view_offset_z;
        if (effect->state == 0xFD)
        {
            effect->state = 0xFE;
        }
        break;

    case 0x1C:
        effect->x -= g_field_view_offset_x;
        effect->y -= g_field_view_offset_y;
        effect->z -= g_field_view_offset_z;
        effect->flags.word |= 0x1000;
        break;

    case 0x1D:
        effect->y -= 0x7000;
        effect->x -= g_field_view_offset_x;
        effect->z -= g_field_view_offset_z;
        effect->flags.word |= 0x1000;
        effect->y -= g_field_view_offset_y;
        break;

    case 0x1E:
        effect->y += 0x7000;
        effect->x -= g_field_view_offset_x;
        effect->z -= g_field_view_offset_z;
        effect->flags.word |= 0x1000;
        effect->y -= g_field_view_offset_y;
        break;

    case 0x1F:
        effect->x += 0xFFFF6000;
        effect->x -= g_field_view_offset_x;
        effect->y -= g_field_view_offset_y;
        effect->z -= g_field_view_offset_z;
        effect->flags.word |= 0x1000;
        break;

    case 0x20:
        effect->x += 0xA000;
        effect->x -= g_field_view_offset_x;
        effect->y -= g_field_view_offset_y;
        effect->z -= g_field_view_offset_z;
        effect->flags.word |= 0x1000;
        break;

    case 0x21:
        effect->x += 0xFFFF6000;
        effect->y -= 0x7000;
        effect->x -= g_field_view_offset_x;
        effect->z -= g_field_view_offset_z;
        effect->flags.word |= 0x1000;
        effect->y -= g_field_view_offset_y;
        break;

    case 0x22:
        effect->x += 0xA000;
        effect->y -= 0x7000;
        effect->x -= g_field_view_offset_x;
        effect->z -= g_field_view_offset_z;
        effect->flags.word |= 0x1000;
        effect->y -= g_field_view_offset_y;
        break;

    case 0x23:
        effect->x += 0xFFFF6000;
        effect->y += 0x7000;
        effect->x -= g_field_view_offset_x;
        effect->z -= g_field_view_offset_z;
        effect->flags.word |= 0x1000;
        effect->y -= g_field_view_offset_y;
        break;

    case 0x24:
        effect->x += 0xA000;
        effect->y += 0x7000;
        effect->x -= g_field_view_offset_x;
        effect->z -= g_field_view_offset_z;
        effect->flags.word |= 0x1000;
        effect->y -= g_field_view_offset_y;
        break;

    case 0x27:
        {
            FieldSpawnObjectPlacement *table_base = g_field_object_states;
            owner_placement_state = &table_base[actor->owner_object_index];
        }
        if ((owner_placement_state->state_flags.bytes[0] & 1) && owner_placement_state->state_flags.bytes[2] != actor->actor_index &&
            actor->actor_index >= 0x40U && !(((u32)owner_placement_state->state_flags.word >> 5) & 1))
        {
            effect->state = 0xFF;
            actor->active_counts[g_field_track_index][part_index]--;
            actor->track_counters[g_field_track_index][part_index]--;
            return -1;
        }
        source_record = &g_field_actors[actor->owner_object_index];
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
                FieldSpawnObjectPlacement *table_base = g_field_object_states;
                owner_source_object = &table_base[source_record->source_object_index];
            }
            if (owner_source_object->state_flags.bytes[0] & 1)
            {
                if (owner_source_object->state_flags.bytes[2] != actor->actor_index)
                {
                    effect->state = 0xFE;
                    break;
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
        source_record = &g_field_actors[actor->track_object_indices[g_field_track_index]];
        if ((part->spawn_flags & 0x08000000) && !(g_field_actors[actor->owner_object_index].facing_or_reward_kind & 0x80))
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
                FieldSpawnObjectPlacement *table_base = g_field_object_states;
                track_source_object = &table_base[source_record->source_object_index];
            }
            if (track_source_object->state_flags.bytes[0] & 1)
            {
                if (track_source_object->state_flags.bytes[2] != actor->actor_index)
                {
                    effect->state = 0xFE;
                    break;
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
        placement_object = &g_field_object_states[actor->owner_object_index];
        source_record = &g_field_actors[actor->owner_object_index];
        effect->x += source_record->x + (placement_object->attachment_points[(part->effect_flags >> 21) & 3].x << 8);
        effect->y += source_record->y + (placement_object->attachment_points[(part->effect_flags >> 21) & 3].y << 8);
        effect->z += source_record->z;
        if (effect->state == 0xFD)
        {
            effect->source_object_index = source_record->source_object_index;
            effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (source_record->flags.word & 0x780000);
            effect->flags.word = (effect->flags.word & 0xFFFCFFFF) | ((source_record->flags.half[1] & 3) << 16);
            {
                FieldSpawnObjectPlacement *table_base = g_field_object_states;
                source_owner_object = &table_base[source_record->source_object_index];
            }
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
                if (resource != 0)
                {
                    field_restart_actor_animation(effect, resource);
                }
                break;
            }
            effect->state = 0xFE;
            break;
        }
        break;

    case 0x32:
        {
            FieldSpawnObjectPlacement *table_base = g_field_object_states;
            placement_object = &table_base[actor->owner_object_index];
        }
        source_record = &g_field_actors[actor->owner_object_index];
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
                FieldSpawnObjectPlacement *table_base = g_field_object_states;
                source_owner_object = &table_base[source_record->source_object_index];
            }
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
                if (resource != 0)
                {
                    field_restart_actor_animation(effect, resource);
                }
                break;
            }
            effect->state = 0xFE;
            break;
        }
        break;

    case 0x33:
        placement_object = &g_field_object_states[actor->owner_object_index];
        source_record = &g_field_actors[actor->owner_object_index];
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
        source_record = &g_field_actors[actor->track_object_indices[g_field_track_index]];
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
        if (effect->state == 0xFD)
        {
            effect->state = 0xFE;
        }
        break;


    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
        work_value = placement_kind - 0x37;
        for (sibling_index = start; sibling_index < 0x100; sibling_index++)
        {
            if (g_field_effect_records[sibling_index].state != 0xFF && g_field_effect_records[sibling_index].part_index == work_value && g_field_effect_records[sibling_index].actor_index == actor->actor_index &&
                ((actor->parts[work_value].orientation_flags & 4) || g_field_effect_records[sibling_index].track_index == g_field_track_index))
            {
                effect->x += g_field_effect_records[sibling_index].x;
                effect->y += g_field_effect_records[sibling_index].y;
                effect->z += g_field_effect_records[sibling_index].z;
                if ((part->spawn_flags & 0x08000000) && !(g_field_actors[actor->owner_object_index].facing_or_reward_kind & 0x80))
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
            effect->facing_or_reward_kind = g_field_effect_records[sibling_index].facing_or_reward_kind;
            effect->saved_state = g_field_effect_records[sibling_index].saved_state;
            effect->unknown_0x34 = g_field_effect_records[sibling_index].unknown_0x34;
            effect->unknown_0x35 = g_field_effect_records[sibling_index].unknown_0x35;
            effect->track_index = g_field_effect_records[sibling_index].track_index;
            effect->motion_remainder = g_field_effect_records[sibling_index].motion_remainder;
            effect->unknown_0x37 = g_field_effect_records[sibling_index].unknown_0x37;
            effect->unknown_0x38 = g_field_effect_records[sibling_index].unknown_0x38;
                        break;
                    }
                    effect->source_object_index = g_field_effect_records[sibling_index].source_object_index;
                    effect->state = g_field_effect_records[sibling_index].state;
                    effect->flags.word = (effect->flags.word & 0xFF87FFFF) | (((u32)g_field_effect_records[sibling_index].flags.word >> 19 & 15) << 19);
                    effect->flags.word = (effect->flags.word & 0xFFFCFFFF) | ((((u32)g_field_effect_records[sibling_index].flags.word >> 16) & 3) << 16);
                    effect->saved_state = g_field_effect_records[sibling_index].saved_state;
                    effect->facing_or_reward_kind = g_field_effect_records[sibling_index].facing_or_reward_kind;
                    resource = g_field_resource_entries
                              [g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].unknown_0x3b]
                                  .start;
                    if (resource != 0)
                    {
                        field_begin_actor_animation_forward(effect, resource);
                    }
                }
                break;
            }
        }
        if (sibling_index == 0x100)
        {
            effect->state = 0xFF;
            actor->active_counts[g_field_track_index][part_index]--;
            actor->track_counters[g_field_track_index][part_index]--;
            return -1;
        }
        func_8006D79C(actor, part_index, sibling_index + 1);
        break;
    }

        switch ((part->effect_flags >> 29) & 3)
        {
        case 1:
            direction_vector->vx = (g_field_actors[actor->owner_object_index].x - effect->x) >> 8;
            direction_vector->vy = (g_field_actors[actor->owner_object_index].y - effect->y) >> 8;
            direction_z = (direction_vector->vz = (g_field_actors[actor->owner_object_index].z - effect->z) >> 8);
            effect->heading = ratan2(-direction_z, direction_vector->vx);
            gte_ldlvl(direction_vector);
            gte_sqr0();
            gte_stlvnl(squared_vector);
            effect->pitch = ratan2(SquareRoot0(squared_vector->vx + squared_vector->vz), -direction_vector->vy);
            effect->rotation_x = 0;
            effect->x = g_field_actors[actor->owner_object_index].x;
            effect->y = g_field_actors[actor->owner_object_index].y;
            effect->z = g_field_actors[actor->owner_object_index].z;
            break;
        case 2:
            direction_vector->vx = (g_field_actors[actor->track_object_indices[g_field_track_index]].x - effect->x) >> 8;
            direction_vector->vy = (g_field_actors[actor->track_object_indices[g_field_track_index]].y - effect->y) >> 8;
            direction_z = (g_field_actors[actor->track_object_indices[g_field_track_index]].z - effect->z) >> 8;
            direction_vector->vz = direction_z;
            effect->heading = ratan2(-direction_z, direction_vector->vx);
            gte_ldlvl(direction_vector);
            gte_sqr0();
            gte_stlvnl(squared_vector);
            effect->pitch = ratan2(SquareRoot0(squared_vector->vx + squared_vector->vz), -direction_vector->vy);
            effect->rotation_x = 0;
            effect->x = g_field_actors[actor->track_object_indices[g_field_track_index]].x;
            effect->y = g_field_actors[actor->track_object_indices[g_field_track_index]].y;
            effect->z = g_field_actors[actor->track_object_indices[g_field_track_index]].z;
            break;
        }
        if ((effect->flags.word & 0x07000000) == 0x05000000)
        {
            func_800A1D98(effect, field_resolve_effect_extent(actor, part), (part->placement_flags >> 15) & 1, D_80105770);
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
            !(g_field_actors[actor->owner_object_index].facing_or_reward_kind & 0x80))
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
                effect->facing_or_reward_kind = (effect->facing_or_reward_kind & 0x7F) | (g_field_actors[actor->owner_object_index].facing_or_reward_kind & 0x80);
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
        if (effect->state != 0xFE && effect->state != 0xFF)
        {
            if (actor->track_object_indices[g_field_track_index] != 0xFF)
            {
                if ((actor->animation->sync_flags & 0x14) == 0x14 && (actor->animation->sync_flags >> 12) == part_index)
                {
                    g_field_actors[actor->track_object_indices[g_field_track_index]].state = 0xFE;
                    g_field_object_states[actor->track_object_indices[g_field_track_index]].state_flags.word |= 1;
                    g_field_object_states[actor->track_object_indices[g_field_track_index]].state_flags.bytes[2] = actor->actor_index;
                    ((u8*)&actor->action_flags)[1] = 1;
                }
            }
            if ((actor->animation->sync_flags & 0xA) == 0xA)
            {
                if (((actor->animation->sync_flags >> 8) & 0xF) == part_index)
                {
                    g_field_actors[actor->owner_object_index].state = 0xFE;
                    g_field_object_states[actor->owner_object_index].state_flags.word |= 1;
                    g_field_object_states[actor->owner_object_index].state_flags.bytes[2] = actor->actor_index;
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

extern FieldActorState g_field_actor_slots[80];
extern FieldMotionRecord g_field_actors[];
extern VECTOR D_80105778;
extern s32 D_80105760;
/**
 * @brief Packed action context: action in low bits, source at bit 8, recipient at bit 16.
 * @note Also written on pool exhaustion and advanced during collision attempts;
 * the broader protocol of those writes is still unresolved.
 */
extern s32 g_field_action_context;
void field_set_action_context(s32 recipient_id, s32 source_id, s32 action);
extern s32 g_field_active_group;
/** @brief Suppress repeated pickup audio until the next frame-command build. */
extern s32 g_field_pickup_sound_played;
extern FieldRewardCounterView g_field_player_records[];

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
    if (part->palette_extent.fields.angular_divisions != 0)
    {
        angle = ((0x1000 / part->palette_extent.fields.angular_divisions) * actor->track_counters[g_field_track_index][part->unknown_0x32] + 0x400) & 0xFFF;
        rotation_result->heading = angle;
    }
    else
    {
        angle = (u32) rand() >> 3;
        rotation_result->heading = angle;
    }
    actor->track_counters[g_field_track_index][part->unknown_0x32]++;

    if ((part->track_flags.word >> 0xB) & 1)
    {
        track_scale = field_evaluate_parameter_track(actor, part->unknown_0x9 & 0xF);
        random_product = track_scale * (rand() << 3);
    }
    else
    {
        random_product = part->unknown_0x9 * (rand() << 3);
    }
    rotation_result->pitch = random_product >> 0xF;

    if ((part->track_flags.word >> 0xC) & 1)
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
    VECTOR source_position;
    s32 unused[2]; /* Required by the original stack layout. */

    if (effect->color_position.fields.position_source != FIELD_POSITION_NONE)
    {
        field_resolve_effect_position(effect, part, &source_position);
        effect->color_position.fields.position_source = FIELD_POSITION_SAVED;
        effect->work_x = effect->x + g_field_view_offset_x;
        effect->work_y = effect->y + g_field_view_offset_y;
        effect->work_z = effect->z + g_field_view_offset_z;
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
    VECTOR target_position;
    VECTOR direction;
    VECTOR squared_direction;

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
    u8 previous_state;
    RECT palette_rect;
    FieldSVector screen_position;
    VECTOR segment_delta;
    VECTOR segment_angles;

    actor = actor_state;
    for (effect = g_field_effect_records; effect != &g_field_effect_records[FIELD_EFFECT_ACTIVE_RECORD_COUNT]; effect++)
    {
        if (effect->actor_index == actor->actor_index && effect->state != FIELD_EFFECT_RETIRED)
        {
            part = &actor->parts[effect->part_index];
            g_field_track_index = effect->track_index;
            field_update_effect_record(effect, part, actor);
            effect->age++;
            if (((part->behavior_flags.word >> 4) & 3) == 1 && (u16) effect->age == effect->lifetime)
            {
                previous_state = effect->state;
                effect->state = FIELD_EFFECT_RETIRED;
                effect->height_or_retired_state = previous_state;
            }
            if (((part->behavior_flags.word >> 4) & 3) == 3)
            {
                screen_position.x = (g_field_view_offset_x / 256) + (u32) (effect->x / 256 + 0xA0);
                screen_position.y = 0x70 + g_field_view_offset_y / 256 + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512;
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
                new_effect_index = func_8006D79C(actor, part->rotation_extent.fields.unknown_0x23 & 0xF, 0);
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
                    screen_position.y = (segment_delta.vy >> 8) - (segment_delta.vz >> 9);
                    segment_delta.vz = 0;
                    segment_delta.vx = screen_position.y;
                    segment_delta.vy = -screen_position.x;
                    func_8001CDAC(&segment_delta, &segment_angles);
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
    palette_changed = 0;
    for (part_index = 0; part_index < actor->part_count; part_index++)
    {
        palette_part = (FieldActorPartDef *) (part_index * (s32) sizeof(FieldActorPartDef) + (s32) actor->parts);
        if ((palette_part->track_flags.word >> 0x15) & 1)
        {
            field_interpolate_palette_track(actor, palette_part->palette_extent.fields.palette_track, palette_buffer, (u8 *) palette_buffer + 0x200);
            palette_changed++;
        }
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
    FieldObjectRuntime* object;
    FieldObjectRuntime* object_table;
    VECTOR target_position;
    VECTOR direction;
    VECTOR squared_direction;
    s32 spawn_slot, spawn_mask;
    s32 new_effect_index;
    s32 spawn_count;
    s32 placement_kind;
    s16 animation_state;
    u8 next_effect_index;

    g_field_actor_slots[effect->actor_index].active_counts[effect->track_index][part->unknown_0x32]--;

    actor = &g_field_actor_slots[effect->actor_index];
    field_dispatch_actor_audio_event(actor, 3, effect->part_index);

    if (part->appearance.word & 0xF0000000)
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
            if ((part->appearance.word >> 0x1C) & spawn_mask)
            {
                D_80105778.vx = effect->x;
                D_80105778.vy = effect->y;
                D_80105760 = 0;
                D_80105778.vz = effect->z;

                spawn_part = &g_field_actor_slots[effect->actor_index]
                                  .parts[(part->spawn_flags.halves.part_selectors >> (spawn_slot * 4)) & 0xF];
                spawn_count = 1;
                placement_kind = 0x35;
                if (((spawn_part->placement_flags.word >> 0x12) & 0x3F) == placement_kind)
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
                            if (!(((u8*)&spawned_effect->flags)[3] & 7) && (spawned_effect->color_position.fields.position_source != 0))
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
            object_table = g_field_object_states;
            object = &object_table[actor->track_object_indices[effect->track_index]];
            if (object->contact.bytes.controller_index == actor->actor_index)
            {
                animation_state = g_field_actors[actor->track_object_indices[effect->track_index]].motion_parameter;
                if ((animation_state != 0x90 && animation_state != 0x94) || (object->object_flags & 0x200))
                {
                    g_field_actors[actor->track_object_indices[effect->track_index]].state = 0;
                }
                else
                {
                    g_field_actors[actor->track_object_indices[effect->track_index]].state = 0xFE;
                }
                g_field_object_states[actor->track_object_indices[effect->track_index]].contact.flags &= ~1;
            }
        }
    }

    if (((actor->animation->sync_flags & 0xA) == 0xA) &&
        (((actor->animation->sync_parts >> 0xA) & 7) == effect->part_index))
    {
        object = &g_field_object_states[actor->owner_object_index];
        if (object->contact.bytes.controller_index == actor->actor_index)
        {
            animation_state = g_field_actors[actor->owner_object_index].motion_parameter;
            if ((animation_state != 0x90 && animation_state != 0x94) || (object->object_flags & 0x200))
            {
                g_field_actors[actor->owner_object_index].state = 0;
            }
            else
            {
                g_field_actors[actor->owner_object_index].state = 0xFE;
            }
            g_field_object_states[actor->owner_object_index].contact.flags &= ~1;
        }
    }

    if (effect->part_index == ((actor->animation->sync_parts & 0x1F) - 1))
    {
        g_field_actors[actor->owner_object_index].x = effect->x;
        g_field_actors[actor->owner_object_index].y = effect->y;
        g_field_actors[actor->owner_object_index].z = effect->z;
        g_field_actors[actor->owner_object_index].y = 0;
        g_field_actors[actor->owner_object_index].facing_or_reward_kind =
            (g_field_actors[actor->owner_object_index].facing_or_reward_kind & 0x7F) | (effect->facing_or_reward_kind & 0x80);
    }

    if (effect->part_index == (((actor->animation->sync_parts >> 5) & 0x1F) - 1))
    {
        if (actor->track_object_indices[effect->track_index] != 0xFF)
        {
            g_field_actors[actor->track_object_indices[effect->track_index]].x = effect->x;
            g_field_actors[actor->track_object_indices[effect->track_index]].y = effect->y;
            g_field_actors[actor->track_object_indices[effect->track_index]].z = effect->z;
            g_field_actors[actor->track_object_indices[effect->track_index]].y = 0;
            g_field_actors[actor->track_object_indices[effect->track_index]].facing_or_reward_kind =
                (g_field_actors[actor->track_object_indices[effect->track_index]].facing_or_reward_kind & 0x7F) |
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
 * @see decomp.me (100%) https://decomp.me/scratch/i1ZHZ
 */
void field_update_effect_record(FieldMotionRecord *record, FieldActorPartDef *part, FieldActorState *actor)
{
    VECTOR *target_position;
    FieldEffectCamera *camera;
    FieldEffectMapBounds *map_bounds;
    FieldEffectCollisionMover *mover;
    FieldEffectCollisionQuery *query;
    VECTOR *work_vector;
    FieldSVector *local_vector;
    FieldSVector *rotated_vector;
    MATRIX *rotation;
    VECTOR *placement_origin;
    FieldMotionRecord *reference_record;
    FieldObjectRuntime *reference_object;
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
    work_vector = (VECTOR *) FIELD_EFFECT_VECTOR_ADDRESS;
    target_position = (VECTOR *) FIELD_EFFECT_TARGET_ADDRESS;
    local_vector = (FieldSVector *) FIELD_EFFECT_LOCAL_VECTOR_ADDRESS;
    rotated_vector = (FieldSVector *) FIELD_EFFECT_ROTATED_VECTOR_ADDRESS;
    rotation = (MATRIX *) FIELD_EFFECT_MATRIX_ADDRESS;
    placement_origin = (VECTOR *) FIELD_EFFECT_ORIGIN_ADDRESS;

    /* Sample transparency and motion tracks; the caller advances age afterward. */
    flags = part->effect_flags;
    if (flags & FIELD_EFFECT_SEMITRANSPARENT)
    {
        s32 semitransparent = field_evaluate_parameter_track_at_time(actor, (flags >> 0x19) & 0xF, (u16) ((u16) record->age)) != 0;
        record->flags = (record->flags & ~FIELD_EFFECT_SEMITRANSPARENT) | (semitransparent << 0x17);
    }
    if ((record->flags & 0x60000000) == 0x40000000)
    {
        record->motion_parameter = field_evaluate_parameter_track_at_time(actor, ((s16 *) &part->orientation_flags.word)[1] & 0xF, ((u16) record->age));
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
        if ((((u32) part->placement_flags.word >> 0x1A) & 3) == 2)
        {
            record->rotation_z_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_extent.fields.rotation_z_track & 0xF, ((u16) record->age));
        }
        if ((((u32) part->placement_flags.word >> 0x1C) & 3) == 2)
        {
            record->rotation_y_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_extent.fields.rotation_y_track & 0xF, ((u16) record->age));
        }
        if ((record->flags & 0x600) == 0x400)
        {
            u32 selector_high = part->palette_extent.word >> 29;
            local_vector->y = -field_evaluate_parameter_track_at_time(actor, ((((part->rotation_extent.word) & 0x3F) * 8) | selector_high) & 0xF, (u16) record->age);
        }
        else
        {
            local_vector->y = -((u16) record->flags & FIELD_EFFECT_DISTANCE_MASK);
        }
        if ((part->palette_extent.word) & 0x01000000)
        {
            local_vector->y = (s16) ((g_field_object_states[actor->owner_object_index].movement.half.flags & FIELD_OBJECT_EFFECT_SCALE_MASK) * (s16) (u16) local_vector->y / 100);
        }
        {
            u32 bounds_flags;
            s32 placement_kind;
            bounds_flags = part->placement_flags.word;
            placement_kind = (bounds_flags >> 0x12) & 0x3F;
            if (placement_kind < 0x14)
            {
                if ((bounds_flags >> 0x10) & 1)
                {
                    s32 previous_distance = local_vector->y;
                    if (placement_kind < 0xA || placement_kind >= 0x26)
                    {
                        dy = (g_field_object_states[actor->owner_object_index].bounds.half.right - g_field_object_states[actor->owner_object_index].bounds.half.left) >> 1;
                    }
                    else
                    {
                        dy = (g_field_object_states[actor->track_object_indices[g_field_track_index]].bounds.half.right - g_field_object_states[actor->track_object_indices[g_field_track_index]].bounds.half.left) >> 1;
                    }
                    dy = abs(dy);
                    local_vector->y = previous_distance - dy;
                    bounds_flags = part->placement_flags.word;
                }
                if ((bounds_flags >> 0x11) & 1)
                {
                    s32 previous_distance = local_vector->y;
                    if (((bounds_flags >> 0x12) & 0x3F) < 0xA || ((bounds_flags >> 0x12) & 0x3F) >= 0x26)
                    {
                        dy = (g_field_object_states[actor->owner_object_index].bounds.half.bottom - g_field_object_states[actor->owner_object_index].bounds.half.top) >> 1;
                    }
                    else
                    {
                        dy = (g_field_object_states[actor->track_object_indices[g_field_track_index]].bounds.half.bottom - g_field_object_states[actor->track_object_indices[g_field_track_index]].bounds.half.top) >> 1;
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
        selector = ((FieldPlacementBits *) &part->placement_flags.word)->opcode;
        switch (selector)
        {
            case 0x0: case 0x1: case 0x2: case 0x3: case 0x4:
            case 0x5: case 0x6: case 0x7: case 0x8: case 0x9:
            case 0xA: case 0xB: case 0xC: case 0xD: case 0xE:
            case 0xF: case 0x10: case 0x11: case 0x12: case 0x13:
            {
                s32 offset_y;
                FieldObjectRuntime *owner;
                FieldObjectRuntime *owner_base;

                if ((s32) selector >= 0xA)
                {
                    slot = actor->track_object_indices[g_field_track_index];
                    selector -= 0xA;
                    reference_record = &g_field_actors[slot];
                    reference_object = &g_field_object_states[slot];
                }
                else
                {
                    slot = actor->owner_object_index;
                    reference_record = &g_field_actors[slot];
                    reference_object = &g_field_object_states[slot];
                }
                owner_base = g_field_object_states;
                owner = &owner_base[actor->owner_object_index];
                flags_byte = owner->contact.bytes.flags_low;
                if ((flags_byte & 1) && ((u8) actor->actor_index >= 0x40U))
                {
                    if (!(((u32) owner->contact.flags >> 5) & 1))
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
                        offset_or_angle = (reference_object->bounds.half.right + reference_object->bounds.half.left) >> 1;
                        offset_y = (reference_object->bounds.half.bottom + reference_object->bounds.half.top) >> 1;
                        break;
                    case 2:
                        offset_y = 0;
                        offset_or_angle = (reference_object->bounds.half.right + reference_object->bounds.half.left) >> 1;
                        break;
                    case 3:
                        offset_y = reference_object->bounds.half.top;
                        offset_or_angle = (reference_object->bounds.half.right + reference_object->bounds.half.left) >> 1;
                        break;
                    case 4:
                        offset_or_angle = reference_object->bounds.half.left;
                        offset_y = (reference_object->bounds.half.bottom + reference_object->bounds.half.top) >> 1;
                        break;
                    case 5:
                        offset_or_angle = reference_object->bounds.half.right;
                        offset_y = (reference_object->bounds.half.bottom + reference_object->bounds.half.top) >> 1;
                        break;
                    case 6:
                        offset_or_angle = reference_object->bounds.half.left;
                        offset_y = reference_object->bounds.half.top;
                        break;
                    case 7:
                        offset_or_angle = reference_object->bounds.half.right;
                        offset_y = reference_object->bounds.half.top;
                        break;
                    case 8:
                        offset_or_angle = reference_object->bounds.half.left;
                        offset_y = reference_object->bounds.half.bottom;
                        break;
                    case 9:
                        offset_or_angle = reference_object->bounds.half.right;
                        offset_y = reference_object->bounds.half.bottom;
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
                    if (!(part->orientation_flags.word & FIELD_EFFECT_ORIENTATION_LOCK))
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
                placement_origin->vx = (part->offset_x << 8) - g_field_view_offset_x;
                placement_origin->vy = (part->offset_y << 8) - g_field_view_offset_y;
                placement_origin->vz = (part->offset_z << 8) - g_field_view_offset_z;
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
                reference_record = &g_field_actors[actor->owner_object_index];
                if ((((u32) part->placement_flags.word >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
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
                reference_record = &g_field_actors[actor->track_object_indices[g_field_track_index]];
                if ((part->spawn_flags.word & 0x08000000) && !(g_field_actors[actor->owner_object_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else if ((((u32) part->placement_flags.word >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
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
                reference_record = &g_field_actors[actor->owner_object_index];
                reference_object = &g_field_object_states[actor->owner_object_index];
                placement_origin->vx = reference_record->x;
                placement_origin->vy = reference_record->y + (reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].y << 8);
                placement_origin->vz = reference_record->z;
                placement_origin->vx += reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8;
                break;
            case 0x32:
                reference_record = &g_field_actors[actor->owner_object_index];
                reference_object = &g_field_object_states[actor->owner_object_index];
                placement_origin->vx = reference_record->x + (reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8);
                placement_origin->vy = reference_record->y;
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                if ((((u32) part->placement_flags.word >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else
                {
                    placement_origin->vx += part->offset_x << 8;
                }
                break;
            case 0x33:
                reference_record = &g_field_actors[actor->owner_object_index];
                reference_object = &g_field_object_states[actor->owner_object_index];
                placement_origin->vx = reference_record->x + (reference_object->ground_attachment_points[((u32) record->flags >> 0xD) & 3].x << 8);
                placement_origin->vy = reference_record->y;
                placement_origin->vz = reference_record->z + (reference_object->ground_attachment_points[((u32) record->flags >> 0xD) & 3].y << 8);
                break;
            case 0x34:
                reference_record = &g_field_actors[actor->track_object_indices[g_field_track_index]];
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
                if ((part->spawn_flags.word & 0x08000000) && !(g_field_actors[actor->owner_object_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else if ((((u32) part->placement_flags.word >> 0xA) & 1) && !(g_field_effect_records[(u16) record->reference_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else
                {
                    placement_origin->vx += part->offset_x << 8;
                }
                placement_origin->vy += part->offset_y << 8;
                placement_origin->vz += part->offset_z << 8;
                if (!(part->orientation_flags.word & FIELD_EFFECT_ORIENTATION_LOCK))
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
        if (part->placement_flags.word & 1)
        {
            record->z = z + 0x80;
        }
        flags = part->placement_flags.word;
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
        if ((((u32) part->behavior_flags.word >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) && ((record->pitch + part->pitch_acceleration) < 0x800))
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
        if (!(((u32) part->behavior_flags.word >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) && (record->color_position.fields.position_source == 0))
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
        if ((((record->flags & 0x60000000) != 0x40000000) || ((s16) record->motion_parameter != 0)) && ((((u32) part->behavior_flags.word >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) || ((s32) part->placement_flags.word < 0)))
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
                    if ((g_field_active_group != 0) && (func_8005B368(query) != -1))
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
        flags = part->placement_flags.word;
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
        if (record->color_position.fields.position_source != 0)
        {
            s32 target_dy, target_dz;

            field_resolve_effect_position(record, part, target_position);
            work_vector->vx = (record->x - target_position->vx) >> 8;
            work_vector->vy = (record->y - target_position->vy) >> 8;
            target_dz = (record->z - target_position->vz) >> 8;
            work_vector->vz = target_dz;
            if ((work_vector->vx >= -0xF) && (work_vector->vx < 0x10) && (target_dz >= -0xF) && (target_dz < 0x10))
            {
                target_dy = work_vector->vy;
                if ((target_dy >= -0xF) && (work_vector->vy < 0x10))
                {
                    if ((((u32) part->behavior_flags.word >> 4) & 3) == 2)
                    {
                        retired_state = record->state;
                        record->state = FIELD_EFFECT_RETIRED;
                        record->height_or_retired_state = (s8) retired_state;
                        return;
                    }
                    if (!(part->orientation_flags.word & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        record->motion_parameter = 0;
                        record->flags = record->flags & 0x9FFFFFFF;
                    }
                }
            }
            if (!(record->flags & FIELD_EFFECT_MOTION_KIND_MASK) && !(part->orientation_flags.word & FIELD_EFFECT_ORIENTATION_LOCK))
            {
                VECTOR new_pos;
                VECTOR delta;
                VECTOR delta_squared;
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
                    if (!(part->orientation_flags.word & FIELD_EFFECT_ORIENTATION_LOCK))
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
                    if (selector < 8 || selector > 0xFF8)
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
                    recipient_index = field_find_actor_in_range(record, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectRuntime *recipient_object;
                        FieldObjectRuntime *object_base;
                        s32 counter_slot;
                        FieldRewardCounterView *counter_base;
                        u32 counter_index;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = g_field_object_states;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, record->facing_or_reward_kind - 0x16);
                        counter_base = g_field_player_records;
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
                    recipient_index = field_find_actor_in_range(record, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectRuntime *recipient_object;
                        FieldObjectRuntime *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = g_field_object_states;
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
                    recipient_index = field_find_actor_in_range(record, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectRuntime *recipient_object;
                        FieldObjectRuntime *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = g_field_object_states;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 5);
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 5);
                        func_80092C24(&g_field_actors[recipient_index], 0x2C);
                        field_release_actor_if_no_effects(record);
                        return;
                    }
                }
                break;
            /* Restore 128/256 of maximum capacity. */
            case FIELD_PICKUP_RESTORE_HALF:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = field_find_actor_in_range(record, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectRuntime *recipient_object;
                        FieldObjectRuntime *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = g_field_object_states;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 6);
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 6);
                        func_80092C24(&g_field_actors[recipient_index], 0x2D);
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
    if (((u32) part->behavior_flags.word >> FIELD_PART_GROUND_BOUNCE_BIT) & 1)
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
extern FieldMotionRecord g_field_actors[];
extern FieldMotionRecord g_field_effect_records[];

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
 */
void field_resolve_effect_position(FieldMotionRecord *effect, FieldActorPartDef *part, VECTOR *position)
{
    FieldMotionRecord *source_record;
    FieldMotionRecord *opposite_record;
    FieldMotionRecord *track_record;
    FieldMotionRecord *owner_record;
    FieldMotionRecord *linked_record;
    FieldObjectRuntime *source_object;
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

    switch (effect->color_position.fields.position_source)
    {
    case FIELD_POSITION_TRACK_OBJECT:
        actors = g_field_actor_slots;
        position->vx = g_field_actors[actors[effect->actor_index].track_object_indices[g_field_track_index]].x;
        position->vy = g_field_actors[actors[effect->actor_index].track_object_indices[g_field_track_index]].y;
        object_index = actors[effect->actor_index].track_object_indices[g_field_track_index];
        do
        {
            track_record = &g_field_actors[object_index];
        } while (0);
        position->vz = track_record->z;
        return;
    case FIELD_POSITION_OWNER_OBJECT:
        owner_actors = g_field_actor_slots;
        position->vx = g_field_actors[owner_actors[effect->actor_index].owner_object_index].x;
        position->vy = g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].y;
        owner_index = owner_actors[effect->actor_index].owner_object_index;
        do
        {
            owner_record = &g_field_actors[owner_index];
        } while (0);
        position->vz = owner_record->z;
        return;
    case FIELD_POSITION_SAVED:
        position->vx = effect->work_x - g_field_view_offset_x;
        position->vy = effect->work_y - g_field_view_offset_y;
        position->vz = effect->work_z - g_field_view_offset_z;
        return;
    case FIELD_POSITION_REFERENCE_EFFECT:
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
    case FIELD_POSITION_OWNER_ATTACHMENT:
        source_record = &g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index];
        source_object = &g_field_object_states[g_field_actor_slots[effect->actor_index].owner_object_index];
        position->vx = source_record->x + (source_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8);
        position->vy = source_record->y + (source_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].y << 8);
        position->vz = source_record->z;
        return;
    case FIELD_POSITION_FACING_OFFSET:
        placement = ((u32) part->placement_flags.word >> 0x12) & 0x3F;
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
        source_record = &g_field_actors[source_index];
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
    case FIELD_POSITION_TRACK_STORED_XZ:
        position->vx = g_field_object_states[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].position_history[0].x << 8;
        position->vz = g_field_object_states[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].position_history[0].z << 8;
        position->vy = 0;
        return;
    case FIELD_POSITION_LINKED_EFFECT:
        position->vx = g_field_effect_records[effect->position_data.linked_effect_index].x;
        position->vy = g_field_effect_records[effect->position_data.linked_effect_index].y;
        linked_record = &g_field_effect_records[effect->position_data.linked_effect_index];
        position->vz = linked_record->z;
        return;
    case FIELD_POSITION_RELATIVE_SIDE_OFFSET:
        placement = ((u32) part->placement_flags.word >> 0x12) & 0x3F;
        if (placement < 0xA)
        {
            source_record = &g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index];
            opposite_record = &g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]];
        }
        else
        {
            if (placement < 0x26)
            {
                object_index = g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index];
                source_record = &g_field_actors[object_index];
            }
            else
            {
                object_index = g_field_actor_slots[effect->actor_index].owner_object_index;
                source_record = &g_field_actors[object_index];
            }
            opposite_record = &g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index];
        }
        if (source_record->x > opposite_record->x)
        {
            if (((part->placement_flags.word >> 0xA) & 1) || (part->spawn_flags.word & 0x08000000))
            {
                effect->facing_or_reward_kind = effect->facing_or_reward_kind & 0x7F;
            }
            horizontal_offset = effect->work_x;
            position_x = source_record->x - horizontal_offset;
        }
        else
        {
            if (((part->placement_flags.word >> 0xA) & 1) || (part->spawn_flags.word & 0x08000000))
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
    case FIELD_POSITION_OWNER_BOUNDS_CENTER:
        source_object = &g_field_object_states[g_field_actor_slots[effect->actor_index].owner_object_index];
        position->vx = g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].x + ((source_object->bounds.half.right + source_object->bounds.half.left) << 7);
        position->vy = g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].y + ((source_object->bounds.half.bottom + source_object->bounds.half.top) << 7);
        position->vz = g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].z;
        return;
    case FIELD_POSITION_TRACK_BOUNDS_CENTER:
        source_object = &g_field_object_states[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]];
        position->vx = g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].x + ((source_object->bounds.half.right + source_object->bounds.half.left) << 7);
        position->vy = g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].y + ((source_object->bounds.half.bottom + source_object->bounds.half.top) << 7);
        position->vz = g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].z;
        return;
    case FIELD_POSITION_REFLECT_TRACK_X:
    case FIELD_POSITION_EXTEND_TRACK_X:
        delta_x = g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].x - g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].x;
        if (effect->color_position.fields.position_source == FIELD_POSITION_REFLECT_TRACK_X)
        {
            position->vx = g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].x - delta_x;
        }
        else
        {
            position->vx = g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].x + delta_x;
        }
        position->vy = g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].y;
        position->vz = g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].z;
        return;
    case FIELD_POSITION_EXTEND_TRACK_XZ:
        position->vx = (g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].x * 2) - g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].x;
        position->vy = g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].y;
        position->vz = (g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].z * 2) - g_field_actors[g_field_actor_slots[effect->actor_index].owner_object_index].z;
        return;
    case FIELD_POSITION_EXTEND_LINK_XZ:
        position->vx = (g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].x * 2) - g_field_effect_records[effect->position_data.linked_effect_index].x;
        position->vy = g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].y;
        position->vz = (g_field_actors[g_field_actor_slots[effect->actor_index].track_object_indices[g_field_track_index]].z * 2) - g_field_effect_records[effect->position_data.linked_effect_index].z;
        return;
    }
}


#include "field_effect_transform.h"
#include "field_effect_render_state.h"
#include "field_types.h"
#include "field_effect_types.h"
#include "field_mesh_render.h"
#include "field_effect_dispatch.h"
#include "field_effect_primitives.h"
#include "sdk/libgpu.h"
#include "sdk/inline_c.h"

#define FIELD_EFFECT_SCRATCH_MATRIX ((void *) 0x1F800000)
#define FIELD_EFFECT_SCRATCH_SCREEN_ORIGIN ((Vec2s *) 0x1F800040)
#define FIELD_EFFECT_SCRATCH_FOOTPRINT ((s16 *) 0x1F800064)
#define FIELD_EFFECT_SCREEN_CENTER_X 160
#define FIELD_EFFECT_SCREEN_CENTER_Y 112
#define FIELD_EFFECT_OT_MAX_DEPTH 0xFFF

#define FIELD_SPRITE_FRAME_FOOTPRINT 0x20
#define FIELD_SPRITE_FRAME_FLIP_X 0x40
#define FIELD_SPRITE_FRAME_FLIP_Y 0x80
#define FIELD_SPRITE_TEXTURE_MODE_MASK 3
#define FIELD_PART_EFFECT_FOOTPRINT 0x00100000

typedef enum
{
    FIELD_EFFECT_RENDER_SPRITE = 0,
    FIELD_EFFECT_RENDER_ACTOR_SPRITE = 1,
    FIELD_EFFECT_RENDER_OWNER_SPRITE = 2,
    FIELD_EFFECT_RENDER_TRACK_SPRITE = 3,
    FIELD_EFFECT_RENDER_RADIAL_FAN = 4,
    FIELD_EFFECT_RENDER_TRAIL = 5,
    FIELD_EFFECT_RENDER_RIBBON = 6,
    FIELD_EFFECT_RENDER_RADIAL_LINES = 0xF6,
    FIELD_EFFECT_RENDER_MESH_2 = 0xF7,
    FIELD_EFFECT_RENDER_MESH_1 = 0xF8,
    FIELD_EFFECT_RENDER_MESH_0 = 0xF9,
    FIELD_EFFECT_RENDER_RING = 0xFA,
    FIELD_EFFECT_RENDER_FAN = 0xFB,
    FIELD_EFFECT_RENDER_MARKER = 0xFC,
    FIELD_EFFECT_RENDER_LINKED_SPRITE = 0xFD
} FieldEffectRenderState;


extern FieldActorState g_field_actor_slots[80];
extern FieldMotionRecord g_field_actors[];
extern FieldMotionRecord g_field_effect_records[256];
extern FieldMotionRecord g_field_effect_records_end;
extern s32 g_field_track_index;
extern u8 *D_801058D4;

extern u16 g_field_texture_slot_flags[];

static s32 *field_render_effect_sprite_frames(FieldMotionRecord *effect, s32 *packet_cursor, s32 *ordering_table, u8 *frame_data);

/**
 * @brief Render each active field effect according to its render state.
 * @param render_context Ordering table and primitive packet cursor.
 * @see decomp.me (100%)
 */
void field_render_effects(FieldRenderContext *render_context)
{
    FieldMotionRecord *effect;
    FieldObjectRuntime *object_state;
    FieldActorPartDef *part;
    u32 *ordering_table;
    FieldResourceEntry *resources;
    s32 *packet_cursor;
    s32 frame_result;
    s32 object_index;
    s32 actor_or_object_index;
    s32 actor_address;
    FieldActorState *actor;
    s32 part_offset;
    s32 value;

    effect = g_field_effect_records;
    resources = g_field_resource_entries;
    ordering_table = &render_context->ordering_table;
    packet_cursor = render_context->packet_cursor;

    if (effect != &g_field_effect_records[FIELD_EFFECT_ACTIVE_RECORD_COUNT])
    {
        do
        {
            g_field_track_index = effect->track_index;
            if (effect->state != FIELD_EFFECT_DISABLED && effect->state != FIELD_EFFECT_RETIRED)
            {
                switch (effect->state)
                {
                case FIELD_EFFECT_RENDER_RADIAL_LINES:
                    packet_cursor = (s32*)field_render_effect_radial_lines(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_MESH_2:
                case FIELD_EFFECT_RENDER_MESH_1:
                case FIELD_EFFECT_RENDER_MESH_0:
                    if (g_field_actor_slots[effect->actor_index].parts[effect->part_index].rotation_extent.fields.unknown_0x23 < 8)
                    {
                        packet_cursor = field_render_lit_effect_mesh(effect, FIELD_EFFECT_RENDER_MESH_0 - effect->state, packet_cursor, ordering_table);
                    }
                    else
                    {
                        packet_cursor = field_render_effect_mesh(effect, FIELD_EFFECT_RENDER_MESH_0 - effect->state, packet_cursor, ordering_table);
                    }
                    break;

                case FIELD_EFFECT_RENDER_RING:
                    packet_cursor = (s32*)field_render_effect_ring(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_FAN:
                    packet_cursor = (s32*)field_render_effect_fan(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_MARKER:
                    packet_cursor = (s32*)field_render_effect_marker(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_LINKED_SPRITE:
                    actor_or_object_index = (s32) &g_field_actor_slots[effect->actor_index];
                    part = &((FieldActorState *) actor_or_object_index)->parts[effect->part_index];
                    value = ((u32) part->placement_flags.word >> 0x12) & 0x3F;
                    if ((value >= 0xA && value < 0x14) || value == 0x28)
                    {
                        actor_or_object_index = ((FieldActorState *) actor_or_object_index)->track_object_indices[effect->track_index];
                        value = actor_or_object_index << 2;
                        value += actor_or_object_index;
                        value <<= 2;
                        value += actor_or_object_index;
                        value <<= 2;
                        value += (s32) g_field_actors;
                        object_state = &g_field_object_states[actor_or_object_index];
                        value = *(u8 *) (value + 0x3B);
                        frame_result = (s32) resources[value].start;
                    }
                    else
                    {
                        actor_or_object_index = ((FieldActorState *) actor_or_object_index)->owner_object_index;
                        value = actor_or_object_index << 2;
                        value += actor_or_object_index;
                        value <<= 2;
                        value += actor_or_object_index;
                        value <<= 2;
                        value += (s32) g_field_actors;
                        object_state = &g_field_object_states[actor_or_object_index];
                        value = *(u8 *) (value + 0x3B);
                        frame_result = (s32) resources[value].start;
                    }
                    object_state->linked_effect_index = (s8) (effect - g_field_effect_records);
                    if (frame_result != 0)
                    {
                        frame_result = field_advance_actor_part_animation_frame(effect, (u8 *) frame_result);
                        if (frame_result != 0)
                        {
                            if (frame_result >= 0)
                            {
                                packet_cursor = func_80077FB4(effect, packet_cursor, ordering_table, frame_result, (object_state->contact.bytes.flags_low & 1) ^ 1, part);
                            }
                            else
                            {
                                packet_cursor = func_80075C88(effect, packet_cursor, ordering_table, frame_result, (object_state->contact.bytes.flags_low & 1) ^ 1, part);
                            }
                        }
                    }
                    break;

                case FIELD_EFFECT_RENDER_SPRITE:
                    frame_result = field_advance_actor_part_animation_frame(effect, D_801058D4);
                    if (frame_result != 0)
                    {
                        packet_cursor = field_render_effect_sprite_frames(effect, packet_cursor, ordering_table, (u8 *) frame_result);
                    }
                    break;

                case FIELD_EFFECT_RENDER_ACTOR_SPRITE:
                    frame_result = (s32) g_field_actor_slots[effect->actor_index].track_data;
                    if (frame_result != 0)
                    {
                        frame_result = field_advance_actor_part_animation_frame(effect, (u8 *) frame_result);
                        if (frame_result != 0)
                        {
                            packet_cursor = field_render_effect_sprite_frames(effect, packet_cursor, ordering_table, (u8 *) frame_result);
                        }
                    }
                    break;

                case FIELD_EFFECT_RENDER_OWNER_SPRITE:
                    actor = &g_field_actor_slots[effect->actor_index];
                    actor_or_object_index = effect->part_index;
                    part_offset = actor_or_object_index * sizeof(FieldActorPartDef);
                    actor_or_object_index = (s32) actor->parts;
                    object_index = actor->owner_object_index;
                    part = (FieldActorPartDef *) (actor_or_object_index + part_offset);
                    value = object_index << 2;
                    value += object_index;
                    value <<= 2;
                    value += object_index;
                    value <<= 2;
                    value += (s32) g_field_actors;
                    object_state = &g_field_object_states[object_index];
                    value = *(u8 *) (value + 0x3B);
                    frame_result = (s32) resources[value].start;
                    object_state->linked_effect_index = (s8) (effect - g_field_effect_records);
                    if (frame_result != 0)
                    {
                        frame_result = field_advance_actor_part_animation_frame(effect, (u8 *) frame_result);
                        if (frame_result != 0)
                        {
                            if (frame_result >= 0)
                            {
                                packet_cursor = func_80077FB4(effect, packet_cursor, ordering_table, frame_result, (object_state->contact.bytes.flags_low & 1) ^ 1, part);
                            }
                            else
                            {
                                packet_cursor = func_80075C88(effect, packet_cursor, ordering_table, frame_result, (object_state->contact.bytes.flags_low & 1) ^ 1, part);
                            }
                        }
                    }
                    break;

                case FIELD_EFFECT_RENDER_TRACK_SPRITE:
                    value = effect->actor_index;
                    actor_address = (s32) &g_field_actor_slots[value];
                    value = effect->part_index;
                    part = &((FieldActorState *) actor_address)->parts[value];
                    value = effect->track_index;
                    object_index = ((FieldActorState *) actor_address)->track_object_indices[value];
                    value = object_index << 2;
                    value += object_index;
                    value <<= 2;
                    value += object_index;
                    value <<= 2;
                    value += (s32) g_field_actors;
                    object_state = &g_field_object_states[object_index];
                    value = *(u8 *) (value + 0x3B);
                    frame_result = (s32) resources[value].start;
                    object_state->linked_effect_index = (s8) (effect - g_field_effect_records);
                    if (frame_result != 0)
                    {
                        frame_result = field_advance_actor_part_animation_frame(effect, (u8 *) frame_result);
                        if (frame_result != 0)
                        {
                            if (frame_result >= 0)
                            {
                                packet_cursor = func_80077FB4(effect, packet_cursor, ordering_table, frame_result, (object_state->contact.bytes.flags_low & 1) ^ 1, part);
                            }
                            else
                            {
                                packet_cursor = func_80075C88(effect, packet_cursor, ordering_table, frame_result, (object_state->contact.bytes.flags_low & 1) ^ 1, part);
                            }
                        }
                    }
                    break;

                case FIELD_EFFECT_RENDER_RADIAL_FAN:
                    packet_cursor = (s32*)field_render_effect_radial_fan(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_TRAIL:
                    packet_cursor = (s32*)field_render_effect_trail(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_RIBBON:
                    packet_cursor = field_render_effect_ribbon(effect, packet_cursor, ordering_table);
                    break;
                }
            }
            effect++;
        } while (effect != &g_field_effect_records_end);
    }

    render_context->packet_cursor = packet_cursor;
}

/**
 * @brief Render the sprite frames for one field effect.
 * @param effect Effect state and world position.
 * @param packet_cursor Current primitive packet cursor.
 * @param ordering_table Depth ordering table.
 * @param frame_data Encoded sprite-frame data.
 * @return Updated primitive packet cursor.
 * @see decomp.me (100%)
 */
static s32 *field_render_effect_sprite_frames(FieldMotionRecord *effect, s32 *packet_cursor, s32 *ordering_table, u8 *frame_data)
{
    Vec2s *screen_origin = FIELD_EFFECT_SCRATCH_SCREEN_ORIGIN;
    s16 *shadow_footprint = FIELD_EFFECT_SCRATCH_FOOTPRINT;
    MATRIX* matrix = FIELD_EFFECT_SCRATCH_MATRIX;
    s32 frame_count;
    s32 shadow_count;
    s32 texture_slot;
    FieldPrimitiveColor packed_color;
    FieldActorState *actor;
    FieldActorPartDef *part;
    FieldActorState *actors;
    s32 height_minus_one;
    s32 width_or_mode;
    s32 frame_x;
    s32 frame_y_or_clut;
    s32 value0;
    s32 value1;
    s32 clut_x;
    s32 clut;
    u16 *texture_slot_flags;

    shadow_count = 0;
    texture_slot = 2;
    actors = g_field_actor_slots;
    part = &actors[effect->actor_index].parts[effect->part_index];
    actor = &actors[effect->actor_index];
    if (actor->owner_object_index < 2)
    {
        texture_slot = actor->owner_object_index;
    }
    field_build_effect_part_matrix(effect, part, FIELD_EFFECT_SCRATCH_MATRIX, actor);
    gte_SetRotMatrix(FIELD_EFFECT_SCRATCH_MATRIX);

    screen_origin->x = FIELD_EFFECT_SCREEN_CENTER_X + g_field_view_offset_x / 256 + effect->x / 256;
    screen_origin->y = FIELD_EFFECT_SCREEN_CENTER_Y + g_field_view_offset_y / 256 + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512;

    frame_count = *frame_data++;
    field_resolve_effect_part_color(actor, effect, part, &packed_color);

    if (frame_count != 0)
    {
        texture_slot_flags = g_field_texture_slot_flags;
        do
        {
            u8 flags = frame_data[7];
            if (!(flags & FIELD_SPRITE_FRAME_FOOTPRINT))
            {
                value1 = packed_color.signed_word;
                setlen(packet_cursor, 9);
                *(s32 *) &((POLY_FT4 *) packet_cursor)->r0 = value1;
                setcode(packet_cursor, 0x2C);
                setSemiTrans((POLY_FT4 *) packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
                width_or_mode = frame_data[4];
                height_minus_one = frame_data[5] - 1;
                effect->sprite_height_minus_one = height_minus_one;
                {
                    s8 frame_y = frame_data[1];
                    frame_y_or_clut = frame_y;
                }
                if (!(effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
                {
                    frame_x = *(s8 *) frame_data;
                }
                else
                {
                    frame_x = -*(s8 *) frame_data - width_or_mode;
                }
                width_or_mode -= 1;
                if (((Vec2s *) part)->y & 1)
                {
                    frame_y_or_clut -= FIELD_EFFECT_SCREEN_CENTER_Y;
                }
                field_project_effect_sprite_quad(effect, screen_origin, (POLY_FT4*)packet_cursor, width_or_mode, height_minus_one, frame_x, frame_y_or_clut, (FieldSpriteFrame*)frame_data, matrix);

                if ((frame_data[7] ^ (effect->facing_or_reward_kind >> 1)) & FIELD_SPRITE_FRAME_FLIP_X)
                {
                    value0 = frame_data[2];
                    ((POLY_FT4 *) packet_cursor)->u3 = value0;
                    ((POLY_FT4 *) packet_cursor)->u1 = value0;
                    value0 += width_or_mode;
                    ((POLY_FT4 *) packet_cursor)->u2 = value0;
                    ((POLY_FT4 *) packet_cursor)->u0 = value0;
                }
                else
                {
                    value0 = frame_data[2];
                    ((POLY_FT4 *) packet_cursor)->u2 = value0;
                    ((POLY_FT4 *) packet_cursor)->u0 = value0;
                    value0 += width_or_mode;
                    ((POLY_FT4 *) packet_cursor)->u3 = value0;
                    ((POLY_FT4 *) packet_cursor)->u1 = value0;
                }
                if (frame_data[7] & FIELD_SPRITE_FRAME_FLIP_Y)
                {
                    value0 = frame_data[3];
                    ((POLY_FT4 *) packet_cursor)->v3 = value0;
                    ((POLY_FT4 *) packet_cursor)->v2 = value0;
                    value0 += height_minus_one;
                    ((POLY_FT4 *) packet_cursor)->v1 = value0;
                    ((POLY_FT4 *) packet_cursor)->v0 = value0;
                }
                else
                {
                    value0 = frame_data[3];
                    ((POLY_FT4 *) packet_cursor)->v1 = value0;
                    ((POLY_FT4 *) packet_cursor)->v0 = value0;
                    value0 += height_minus_one;
                    ((POLY_FT4 *) packet_cursor)->v3 = value0;
                    ((POLY_FT4 *) packet_cursor)->v2 = value0;
                }
                width_or_mode = frame_data[7] & FIELD_SPRITE_TEXTURE_MODE_MASK;
                if (width_or_mode == 2)
                {
                    frame_y_or_clut = 0x1F2;
                    if (actor->owner_object_index < 2)
                    {
                        *(s16 *) &((POLY_FT4 *) packet_cursor)->tpage = ((texture_slot_flags[texture_slot] & 3) << 7) | (((u32) part->behavior_flags.word >> 0x11) & 0x60) | 0x10 | ((((actor->owner_object_index << 6) + 0x340) & 0x3FF) >> 6);
                        frame_y_or_clut = (actor->owner_object_index * 2) + 0x1EE;
                    }
                    else
                    {
                        value0 = texture_slot;
                        value0 <<= 1;
                        value0 += (s32) texture_slot_flags;
                        value1 = ((*(u16 *) value0 & 3) << 7) | (((u32) part->behavior_flags.word >> 0x11) & 0x60);
                        value1 |= 5;
                        *(s16 *) &((POLY_FT4 *) packet_cursor)->tpage = value1;
                    }
                }
                else
                {
                    frame_y_or_clut = (width_or_mode * 2) + 0x1EA;
                    value0 = (((width_or_mode << 6) + 0x180) & 0x3FF) >> 6;
                    value1 = ((u32) part->behavior_flags.word >> 0x11) & 0x60;
                    value1 |= value0;
                    *(s16 *) &((POLY_FT4 *) packet_cursor)->tpage = value1;
                }

                if (((u32) part->track_flags.word >> 0x15) & 1)
                {
                    *(s16 *) &((POLY_FT4 *) packet_cursor)->clut = (frame_y_or_clut + 1) << 6;
                }
                else
                {
                    value0 = part->placement_flags.word >> 0xC;
                    switch (value0 & 3)
                    {
                    case 1:
                    {
                        u8 uv = part->appearance.fields.palette_selector;
                        clut_x = uv & 0xF;
                        if (uv >= 0x10)
                        {
                            value0 = (frame_y_or_clut + 1) << 6;
                        }
                        else
                        {
                            value0 = frame_y_or_clut << 6;
                        }
                        value0 |= clut_x;
                        *(s16 *) &((POLY_FT4 *) packet_cursor)->clut = value0;
                        break;
                    }
                    case 2:
                        if (actor->owner_object_index >= 3)
                        {
                            clut = frame_y_or_clut << 6;
                            value0 = part->appearance.fields.palette_selector & 0x3F;
                            clut |= value0;
                            *(s16 *) &((POLY_FT4 *) packet_cursor)->clut = clut;
                            break;
                        }
                        /* Player slots use the frame's own palette, like mode 0. */
                    case 0:
                        clut = frame_y_or_clut << 6;
                        value0 = frame_data[6] & 0x3F;
                        clut |= value0;
                        *(s16 *) &((POLY_FT4 *) packet_cursor)->clut = clut;
                        break;
                    }
                }

                {
                    if ((effect->flags & FIELD_EFFECT_SCREEN_SPACE) || ((value1 = effect->z >> 7), value1 < 0))
                    {
                        addPrim(&ordering_table[0], packet_cursor);
                        packet_cursor = (s32 *) ((u8 *) packet_cursor + sizeof(POLY_FT4));
                    }
                    else if (value1 > FIELD_EFFECT_OT_MAX_DEPTH)
                    {
                        addPrim(&ordering_table[FIELD_EFFECT_OT_MAX_DEPTH], packet_cursor);
                        packet_cursor = (s32 *) ((u8 *) packet_cursor + sizeof(POLY_FT4));
                    }
                    else
                    {
                        addPrim(&ordering_table[effect->z >> 7], packet_cursor);
                        packet_cursor = (s32 *) ((u8 *) packet_cursor + sizeof(POLY_FT4));
                    }
                }
            }
            else if (((flags & 0xF) == 2) && (part->effect_flags & FIELD_PART_EFFECT_FOOTPRINT))
            {
                shadow_footprint[0] = (*(s8 *) frame_data * part->appearance.fields.footprint_scale_x) >> 6;
                shadow_footprint[1] = ((s8) frame_data[1] * part->footprint_scale_y) >> 6;
                shadow_footprint[2] = ((s8) frame_data[2] * part->appearance.fields.footprint_scale_x) >> 6;
                shadow_footprint[3] = ((s8) frame_data[3] * part->footprint_scale_y) >> 6;
                shadow_footprint[4] = ((s8) frame_data[4] * part->appearance.fields.footprint_scale_x) >> 6;
                shadow_footprint[5] = ((s8) frame_data[5] * part->footprint_scale_y) >> 6;
                shadow_footprint[6] = ((s8) frame_data[6] * part->appearance.fields.footprint_scale_x) >> 6;
                shadow_count += 1;
                shadow_footprint[7] = ((s8) frame_data[8] * part->footprint_scale_y) >> 6;
            }
            frame_data += 9;
            frame_count -= 1;
            value0 = *(s32 *) &((POLY_FT4 *) packet_cursor)[-1].r0;
            *(s32 *) &((POLY_FT4 *) packet_cursor)->r0 = value0;
        } while (frame_count != 0);
    }

    if (shadow_count != 0)
    {
        do { do { do { do { do { packet_cursor = field_render_actor_ground_shadow(effect, packet_cursor, ordering_table, shadow_footprint); } while (0); } while (0); } while (0); } while (0); } while (0);
    }
    return packet_cursor;
}

/** @file field_effect_frames.c
 * @brief Process effect animation frames and build effect primitives, transforms, and projected geometry.
 */

/* field11 */
/**
 * @file field11.c
 * @brief Field animation-frame audio/visual processor, carved from the top
 *        of the unk2 segment (the single-function slot right after
 *        field_effect_dispatch.c's field_render_effects).
 */

#include "common.h"
#include "field_effect_transform.h"
#include "field_effect_render_state.h"
#include "field_types.h"
#include "field_effect_geometry.h"
#include "sdk/libgpu.h"





typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} FrameSVector;

typedef struct
{
    s32 unk0;  /* 0x00 */
    s32 unk4;  /* 0x04 */
    s32 unk8;  /* 0x08 */
    u32 unkC;  /* 0x0C */
    s16 unk10; /* 0x10 */
    s16 unk12; /* 0x12 */
    s16 unk14; /* 0x14 */
    s16 unk16; /* 0x16 */
    u8 unk18;  /* 0x18 */
    u8 unk19;  /* 0x19 */
    u8 unk1A;  /* 0x1A */
    u8 unk1B;  /* 0x1B */
    s32 unk1C; /* 0x1C (halfword view at 0x1E) */
    u8 unk20;  /* 0x20 */
    u8 unk21;  /* 0x21 */
    u8 unk22;  /* 0x22 */
    u8 unk23;  /* 0x23 */
    u8 unk24;  /* 0x24 */
    u8 unk25;  /* 0x25 */
    s8 unk26;  /* 0x26 */
    u8 unk27;  /* 0x27 */
    u8 unk28;  /* 0x28 */
    u8 unk29;  /* 0x29 */
    s16 unk2A; /* 0x2A */
    s16 unk2C; /* 0x2C */
    u16 unk2E; /* 0x2E */
    s16 unk30; /* 0x30 */
    u8 unk32;  /* 0x32 */
    u8 unk33;  /* 0x33 */
    u8 unk34;  /* 0x34 */
    u8 unk35;  /* 0x35 */
    u8 unk36;  /* 0x36 */
    u8 unk37;  /* 0x37 */
    u8 unk38;  /* 0x38 */
    u8 unk39;  /* 0x39 */
    u8 unk3A;  /* 0x3A */
    u8 unk3B;  /* 0x3B */
    u32 unk3C; /* 0x3C */
    s32 unk40; /* 0x40 */
    u32 unk44; /* 0x44 */
    u32 unk48; /* 0x48 */
    u32 unk4C; /* 0x4C */
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

/* This function needs the full 32-bit unk34 (masked against 0x100000), unlike
 * the u16 view used by field8.c/field10.c's local copy of the same struct. */
typedef struct
{
    u32 unk0;  /* 0x00 */
    u32 unk4;  /* 0x04 */
    u8 unk8;   /* 0x08 */
    u8 unk9;   /* 0x09 */
    u8 unkA;   /* 0x0A */
    u8 unkB;   /* 0x0B */
    u8 unkC;   /* 0x0C */
    u8 unkD;   /* 0x0D */
    u8 unkE;   /* 0x0E */
    u8 unkF;   /* 0x0F */
    u8 unk10;  /* 0x10 */
    u8 unk11;  /* 0x11 */
    u8 pad12[0x14 - 0x12];
    u32 unk14; /* 0x14 (halfword view at 0x16) */
    s16 unk18; /* 0x18 */
    u8 unk1A;  /* 0x1A */
    u8 pad1B;
    u32 unk1C; /* 0x1C */
    u8 unk20;
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u32 unk24; /* 0x24 */
    u32 unk28; /* 0x28 */
    u8 unk2C;
    u8 unk2D;
    u8 unk2E;
    u8 unk2F;
    u8 pad30;
    u8 unk31;
    u8 pad32;
    u8 unk33;
    u32 unk34; /* 0x34 */
    s16 unk38;
    s16 unk3A;
    s16 unk3C;
    s16 pad3E;
    s16 unk40;
    s16 unk42;
    s16 unk44;
    s16 unk46;
} FramePartDef;

typedef struct
{
    u8 pad0[0x14];
    u8 unk14; /* 0x14 */
    u8 unk15; /* 0x15 */
    u8 pad16;
    u8 unk17; /* 0x17 */
    u16 unk18;
    u16 unk1A;
} FrameAnimationDef;

typedef struct
{
    FramePartDef *unk0;
    u8 pad4[0xC - 4];
    FrameAnimationDef *unkC;
    u8 pad10[0x14 - 0x10];
    u8 *unk14;
    u8 pad18[0x24 - 0x18];
    u8 unk24;
    u8 unk25;
    u8 unk26;
    u8 unk27;
    u8 unk28;
    u8 unk29;
    u8 unk2A;
    u8 unk2B[16];
    u8 unk3B[9][16];
    u8 padCB;
    u16 unkCC[9][16];
    u16 unk1EC[9];
    Vec2s unk1FE[9];
    u16 unk222;
    u32 unk224;
    u8 unk228;
    u8 unk229[9];
    u8 unk232;
    u8 unk233;
    u16 unk234;
    u16 unk236;
    u8 pad238[2];
    u8 unk23A;
    u8 unk23B;
    u8 pad23C[0x240 - 0x23C];
    u16 *unk240;
} FrameActorState;

/* Local view of the shared per-track "slot" record; only the fields this
 * function touches are named, matching the per-file minimal-view convention
 * already used by field7.c/field8.c/field10.c for the same real struct. */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;   /* 0x0C */
    u8 pad10[0x3C - 0x10];
    s32 unk3C;  /* 0x3C */
    u8 pad40[0x12C - 0x40];
    s16 unk12C; /* 0x12C */
    s16 unk12E; /* 0x12E */
    s16 unk130; /* 0x130 */
    s16 unk132; /* 0x132 */
    s16 unk134; /* 0x134 */
    s16 unk136; /* 0x136 */
    s16 unk138; /* 0x138 */
    s16 unk13A; /* 0x13A */
    s16 unk13C; /* 0x13C */
    s16 unk13E; /* 0x13E */
    s16 unk140; /* 0x140 */
    s16 unk142; /* 0x142 */
    s16 unk144; /* 0x144 */
    s16 unk146; /* 0x146 */
    u8 pad148[0x170 - 0x148];
    u8 unk170;  /* 0x170 */
    u8 pad171[0x174 - 0x171];
    s32 unk174; /* 0x174 */
    s32 unk178; /* 0x178 (byte views at 0x179/0x17A/0x17B) */
    u8 pad17C[0x180 - 0x17C];
    u8 unk180;  /* 0x180 */
    u8 pad181[0x18D - 0x181];
    u8 unk18D;  /* 0x18D */
    u8 unk18E;  /* 0x18E */
    u8 pad18F[0x23C - 0x18F];
} Struct_D80105AE0;

typedef struct
{
    u8 pad0[0x258];
    u8 unk258;
    u8 pad259[0x268 - 0x259];
} Struct_D800FD818;

typedef struct
{
    s32 unk0;
    u8 pad4[0xC - 4];
    s32 unkC;
    u8 pad10[0x1C - 0x10];
} Struct_D80105880;

typedef struct
{
    u8 *start;
    u8 *end;
    u8 unk8;
    u8 slot_index;
    u16 unkA;
    u8 padC[2];
    s16 unkE;
    u32 flags;
} FrameResourceEntry;

/* Output of field_test_quad_actor_contacts: resolved track/actor index plus the screen-space
 * x/y it computed for it. */
typedef struct
{
    s32 index;
    s16 x;
    s16 y;
} TrackPlacement;

#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/* Frame-processor views of shared field arrays (declared with their canonical types above). */
#define FRAME_PLAYER_RECORDS ((Struct_D800FD818 *) g_field_player_records)
#define FRAME_ACTORS ((Struct_D800FDF58 *) g_field_actors)
#define FRAME_OBJECT_STATES ((Struct_D80105AE0 *) g_field_object_states)
#define FRAME_ACTOR_BINDINGS ((Struct_D80105880 *) g_field_actor_bindings)
#define FRAME_ACTOR_SLOTS ((FrameActorState *) g_field_actor_slots)
#define FRAME_RESOURCE_ENTRIES ((FrameResourceEntry *) g_field_resource_entries)


/**
 * @brief Field animation-frame audio/visual processor for the "negative
 *        item" resource variant, spawning per-frame billboard primitives and
 *        panning any attached sound cue relative to the camera.
 * @param rec Effect record.
 * @param cursor Vertex-buffer cursor pointer, threaded and returned.
 * @param base Ordering-table / primitive base array.
 * @param item Animation data blob for this frame.
 * @param flag Selects which of the actor's two audio channels to update.
 * @param part Part definition supplying flags and placement selectors.
 * @return Updated cursor pointer.
 * @see decomp.me (100%)
 * @note Matching requires maspsx to keep consecutive labels before an
 *       inserted load-delay NOP, as original ASPSX 2.67 does. Fixed upstream
 *       in maspsx #143 (tools/maspsx at 3629944 or later).
 */
s32* func_80075C88(Struct_D800FDF58* rec, s32* cursor, s32* base, u8* item, s32 flag, FramePartDef* part)
{
    typedef struct
    {
        unsigned addr : 24;
        unsigned len : 8;
    } PrimitiveTag;
    extern int abs(int);
    MATRIX* mtx = (MATRIX*)0x1F800000;
    Vec2s* sxy = (Vec2s*)0x1F800040;
    s32 item_count;
    VECTOR* gte_out = (VECTOR*)0x1F800044;
    FrameSVector* dir = (FrameSVector*)0x1F800054;
    s16* sp64 = (s16*)0x1F800064;
    Vec2s* sp68;
    Vec2s* sp6C;
    FrameActorState* actor;
    FrameActorState* actor_base;
    s32 sp74;
    s32 sp78;
    TrackPlacement sp28;
    struct
    {
        s32 sp30;
        u8 pad34[0x24];
    } scratch;

    POLY_FT4* poly;
    s32 var_a3;
    u8* var_a1;
    s16 temp_a0_4;
    s32 var_v0_27;
    s32 temp_a0_2;
    s32 placement_flags;
    s32 temp_a1_2;
    Struct_D80105880* clamp_base;
    s32 temp_a3_2;
    s32 temp_s7;
    s32 temp_v0_12;
    s32 temp_v0_18;
    s32 temp_v0_6;
    s32 temp_v0_7;
    s32 temp_v0_8;
    s32 temp_v0_9;
    s32 temp_v1_13;
    s32 temp_v1_17;
    s32 temp_v1_19;
    s32 temp_v1_20;
    s32 temp_v1_23;
    s32 temp_v1_26;
    s32 temp_v1_27;
    s32 temp_v1_28;
    s32 temp_v1_2;
    s32 temp_v1_34;
    s32 var_s1;
    s32 var_s2;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_8;
    s32 var_v1_10;
    s32 var_v1_11;
    s32 var_v1_4;
    s32 var_v1_6;
    s32 var_s0;
    u16 temp_a1;
    u16 temp_v0_15;
    u16 temp_v0_16;
    u16 temp_v1_29;
    u16 temp_v1_30;
    u16 temp_v1_31;
    u16 temp_v1_32;
    u32 temp_a0_3;
    u32 temp_v1_18;
    u32 temp_v1_33;
    u32 temp_v1_3;
    u8 temp_a0;
    u8 temp_v0_2;
    u8 temp_v0_3;
    u8 temp_v0_4;
    u8 temp_v0_5;
    u8 temp_v1;
    u32 temp_v1_10;
    u8 temp_v1_11;
    u8 temp_v1_12;
    u8 temp_v1_21;
    u8 temp_v1_4;
    s32 temp_v1_5;
    s32 temp_v1_6;
    s32 temp_v1_7;
    s32 temp_v1_8;
    u8 temp_v1_9;
    u8 var_a0_3;
    u8 var_a0_4;
    u8 var_a0_5;
    u8 var_a0_6;
    Struct_D80105AE0* slot;
    Struct_D80105AE0* temp_v0_13;
    Struct_D80105AE0* temp_v0_14;
    Struct_D80105AE0* temp_v0_17;
    Struct_D80105AE0* temp_v0_20;
    Struct_D800FDF58* temp_v0_19;
    Struct_D80105AE0* temp_v1_24;
    FrameActorState* temp_v1_25;

    sp78 = 0;
    sp68 = (Vec2s*)0x1F800080;
    sp6C = (Vec2s*)0x1F800094;
    slot = &FRAME_OBJECT_STATES[rec->unk3A];
    *(s32*)&slot->unk12C = 0;
    if (flag == 0)
    {
        s32* zero_ptr;
        s32 zero_count = 7;
        zero_ptr = (s32*)((u8*)slot + 0x1C);
        do
        {
            *(s32*)((u8*)zero_ptr + 0x148) = 0;
            zero_count -= 1;
            zero_ptr -= 1;
        } while (zero_count >= 0);
        *(s32*)&slot->unk144 = 0;
        *(s32*)&slot->unk140 = 0;
    }
    actor = &FRAME_ACTOR_SLOTS[rec->unk22];
    field_build_effect_part_matrix(rec, part, mtx, actor);
    gte_SetRotMatrix(mtx);

    sxy->x = 0xA0 + g_field_view_offset_x / 256 + rec->unk0 / 256;
    temp_a1 = 0x70 + g_field_view_offset_y / 256 + rec->unk4 / 256 - rec->unk8 / 512 - g_field_view_offset_z / 512;
    sxy->y = temp_a1;
    temp_v1 = rec->unk37;
    temp_a0 = rec->unk38;
    if ((temp_v1 | temp_a0) != 0)
    {
        temp_v1_2 = (s8)temp_v1 + ((s32)(((s8)temp_a0 - (s8)temp_v1) * rec->unk34) / (s32)rec->unk35);
        sp74 = temp_v1_2;
        sxy->y = (u16)(temp_a1 - temp_v1_2);
    }
    else
    {
        sp74 = 0;
    }
    field_extract_effect_quad_corners8(rec, item, sp64);
    temp_v1_3 = part->unk34;
    if (temp_v1_3 & 0x100000)
    {
        field_apply_effect_quad_center_offset(rec, sxy, sp64, mtx, (temp_v1_3 >> 0x14) & 1);
    }
    slot->unk140 = (u16)sp64[0];
    slot->unk142 = (u16)sp64[1];
    slot->unk144 = (u16)sp64[4];
    slot->unk146 = (u16)sp64[5];
    item_count = *item++;
    if (item_count != 0)
    {
        poly = (POLY_FT4*)cursor;
        do
        {
            temp_v1_4 = item[7];
            if (!(temp_v1_4 & 0x20))
            {
                if ((u8)rec->unk3A >= 3U || (temp_v1_4 & 3) != 2 || item[6] != 2 || FRAME_PLAYER_RECORDS[rec->unk3A].unk258 == 0)
                {
                    field_resolve_effect_part_color(actor, rec, part, (FieldPrimitiveColor*)&((P_TAG*)cursor)->r0);
                    setPolyFT4(poly);
                    setSemiTrans(poly, rec->unk1C & 0x800000);
                    var_s2 = item[4];
                    var_s1 = (s8)item[1];
                    temp_s7 = item[5] - 1;
                    if (!(rec->unk21 & 0x80))
                    {
                        var_s0 = (s8)*item;
                        var_s2 -= 1;
                    }
                    else
                    {
                        var_s0 = -(s8)*item - var_s2;
                        var_s2 -= 1;
                    }
                    field_project_effect_sprite_quad(rec, sxy, (POLY_FT4*)cursor, var_s2, temp_s7, (s32)var_s0, var_s1, (FieldSpriteFrame*)item, mtx);

                    {
                        if ((item[7] ^ ((u8)rec->unk21 >> 1)) & 0x40)
                        {
                            temp_v0_2 = item[2];
                            poly->u1 = temp_v0_2;
                            poly->u3 = temp_v0_2;
                            temp_v1_5 = poly->u1 + var_s2;
                            var_a0_3 = 0xFF;
                            if (temp_v1_5 != 0x100)
                            {
                                var_a0_3 = temp_v1_5;
                            }
                            poly->u2 = var_a0_3;
                            poly->u0 = var_a0_3;
                        }
                        else
                        {
                            temp_v0_3 = item[2];
                            poly->u0 = temp_v0_3;
                            poly->u2 = temp_v0_3;
                            temp_v1_6 = poly->u0 + var_s2;
                            var_a0_4 = 0xFF;
                            if (temp_v1_6 != 0x100)
                            {
                                var_a0_4 = temp_v1_6;
                            }
                            poly->u3 = var_a0_4;
                            poly->u1 = var_a0_4;
                        }
                        if (item[7] & 0x80)
                        {
                            temp_v0_4 = item[3];
                            poly->v2 = temp_v0_4;
                            poly->v3 = temp_v0_4;
                            temp_v1_7 = poly->v2 + temp_s7;
                            var_a0_5 = 0xFF;
                            if (temp_v1_7 != 0x100)
                            {
                                var_a0_5 = temp_v1_7;
                            }
                            poly->v1 = var_a0_5;
                            poly->v0 = var_a0_5;
                        }
                        else
                        {
                            temp_v0_5 = item[3];
                            poly->v0 = temp_v0_5;
                            poly->v1 = temp_v0_5;
                            temp_v1_8 = poly->v0 + temp_s7;
                            var_a0_6 = 0xFF;
                            if (temp_v1_8 != 0x100)
                            {
                                var_a0_6 = temp_v1_8;
                            }
                            poly->v3 = var_a0_6;
                            poly->v2 = var_a0_6;
                        }
                        if ((item[7] & 3) == 2)
                        {
                            poly->v0 = (u8)(poly->v0 | 0x80);
                            poly->v1 = (u8)(poly->v1 | 0x80);
                            poly->v2 = (u8)(poly->v2 | 0x80);
                            poly->v3 = (u8)(poly->v3 | 0x80);
                        }
                    }

                    temp_a1_2 = rec->unkC;
                    temp_a0_2 = temp_a1_2 << 7;
                    if (temp_a1_2 >= 2)
                    {
                        temp_a0_2 = temp_a1_2 << 6;
                        if (temp_a1_2 >= 9)

                        {
                            s32 v0;
                            s32 v1;
                            s32 a0;
                            v0 = temp_a1_2 - 9;
                            v0 <<= 6;
                            v1 = item[7];
                            v1 &= 3;
                            v1 <<= 6;
                            v1 += 0x3C0;
                            v1 -= v0;
                            v1 &= 0x3FF;
                            a0 = part->unk4;
                            v1 = (s32)v1 >> 6;
                            a0 = (s32)((u32)a0 >> 0x11);
                            a0 &= 0x60;
                            a0 |= 0x10;
                            a0 |= v1;
                            poly->tpage = (s16)a0;
                        }

                        else
                        {
                            var_v1_4 = item[7];
                            var_v1_4 &= 3;
                            var_v1_4 <<= 6;
                            var_v1_4 += 0x340;
                            var_v1_4 -= temp_a0_2;
                            var_v1_4 = (var_v1_4 & 0x3FF) >> 6;
                            var_v0_5 = ((u32)part->unk4 >> 0x11) & 0x60;
                            goto block_58;
                        }
                    }
                    else
                    {
                        temp_a0_2 = 0x380 - temp_a0_2;
                        var_v1_4 = ((u32)part->unk4 >> 0x11) & 0x60;
                        if (item[7] & 3)
                        {
                            var_v0_6 = (temp_a0_2 + 0x40) & 0x3FF;
                        }
                        else
                        {
                            var_v0_6 = temp_a0_2 & 0x3FF;
                        }
                        var_v0_5 = var_v0_6 >> 6;
                    block_58:
                        poly->tpage = (s16)(var_v1_4 | var_v0_5);
                    }
                    temp_a0_3 = rec->unk1C;
                    if ((temp_a0_3 & 0x7F0000) && (item[6] == 0) && ((item[7] & 3) != 2))
                    {
                        if (temp_a0_3 & 0x40000)
                        {
                            if (!(((u32)part->unk28 >> 0xC) & 3))
                            {
                                poly->clut = (s16)(((rec->unk3B + 0x1F4) << 6) | 9);
                            }
                            else
                            {
                                temp_v1_9 = part->unk2D;
                                if (temp_v1_9 >= 0x40U)
                                {
                                    var_s1 = 0x1F2;
                                }
                                else
                                {
                                    var_s1 = (temp_v1_9 >> 4) + 0x1EA;
                                }
                                temp_v0_6 = ((u32)part->unk28 >> 0xC) & 3;
                                switch (temp_v0_6)
                                {
                                case 1:
                                    poly->clut = getClut((part->unk2D & 0xF) << 4, var_s1);
                                    break;
                                case 2:
                                    poly->clut = (s16)(((rec->unk3B + 0x1F4) << 6) | 9);
                                    break;
                                }
                            }
                        }
                        else if (temp_a0_3 & 0x780000)
                        {
                            if (!(((u32)part->unk28 >> 0xC) & 3))
                            {
                                poly->clut = ((rec->unk3B + 0x1F4) << 6) | ((temp_a0_3 >> 0x13) & 0xF);
                            }
                            else
                            {
                                temp_v1_10 = part->unk2D;
                                if (temp_v1_10 >= 0x40U)
                                {
                                    var_s1 = 0x1F2;
                                }
                                else
                                {
                                    var_s1 = (temp_v1_10 >> 4) + 0x1EA;
                                }
                                temp_v0_7 = ((u32)part->unk28 >> 0xC) & 3;
                                switch (temp_v0_7)
                                {
                                case 1:
                                    poly->clut = getClut((part->unk2D & 0xF) << 4, var_s1);
                                    break;
                                case 2:
                                    poly->clut = ((rec->unk3B + 0x1F4) << 6) | (((u32)rec->unk1C >> 0x13) & 0xF);
                                    break;
                                }
                            }
                        }
                        else
                        {
                            if (!(((u32)part->unk28 >> 0xC) & 3))
                            {
                                poly->clut = (s16)(((((temp_a0_3 >> 16) & 3) + 0x1EF) << 6) | 0x10);
                            }
                            else
                            {
                                temp_v1_11 = part->unk2D;
                                if (temp_v1_11 >= 0x40U)
                                {
                                    var_s1 = 0x1F2;
                                }
                                else
                                {
                                    var_s1 = (temp_v1_11 >> 4) + 0x1EA;
                                }
                                temp_v0_8 = ((u32)part->unk28 >> 0xC) & 3;
                                switch (temp_v0_8)
                                {
                                case 1:
                                    poly->clut = getClut((part->unk2D & 0xF) << 4, var_s1);
                                    break;
                                case 2:
                                    poly->clut = ((((*(u16*)((u8*)rec + 0x1E)) & 3) + 0x1EF) << 6) | 0x10;
                                    break;
                                }
                            }
                        }
                    }
                    else if ((!(((u32)part->unk28 >> 0xC) & 3)))
                    {
                        if ((item[7] & 3) != 2)
                        {
                            if (rec->unk3B == 8)
                            {
                                poly->clut = (item[6] & 0x3F) | 0x7A80;
                            }
                            else
                            {
                                poly->clut = getClut(item[6] << 4, rec->unk3B + 0x1F4);
                            }
                            if (item[6] == 0xB)
                            {
                                setSemiTrans(poly, 1);
                            }
                        }
                        else
                        {
                            if (item[6] == 1)
                            {
                                setSemiTrans(poly, 1);
                            }
                            poly->clut = getClut((s32)((item[6] * 0x10) + 0xC0), rec->unk3B + 0x1F4);
                        }
                    }
                    else
                    {
                        temp_v1_12 = part->unk2D;
                        if (temp_v1_12 >= 0x40U)
                        {
                            var_s1 = 0x1F2;
                            if ((u8)actor->unk228 < 2U)
                            {
                                var_s1 = (actor->unk228 * 2) + 0x1EE;
                            }
                        }
                        else
                        {
                            var_s1 = (temp_v1_12 >> 4) + 0x1EA;
                        }
                        temp_v0_9 = ((u32)part->unk28 >> 0xC) & 3;
                        switch (temp_v0_9)
                        {
                        case 1:
                            poly->clut = getClut((part->unk2D & 0xF) << 4, var_s1);
                            break;
                        case 2:
                            if ((u8)actor->unk228 >= 3U)
                            {
                                s32 clut_hi2;
                                s32 clut_lo2;
                                clut_hi2 = var_s1 << 6;
                                clut_lo2 = part->unk2D;
                                poly->clut = (clut_hi2) | (clut_lo2 & 0xF);
                                break;
                            }
                            if ((item[7] & 3) != 2)
                            {
                                var_v0_8 = (rec->unk3B + 0x1F4) << 6;
                                var_v1_6 = item[6] & 0x3F;
                                poly->clut = (s16)(var_v0_8 | var_v1_6);
                            }
                            else
                            {
                                if (item[6] == 1)
                                {
                                    setSemiTrans(poly, 1);
                                }
                                poly->clut = getClut((s32)((item[6] * 0x10) + 0xC0), rec->unk3B + 0x1F4);
                                break;
                            }
                            break;
                        }
                    }
                    temp_v1_13 = (s32)rec->unk8 >> 7;
                    if (temp_v1_13 < 0)
                    {
                        addPrim(base, cursor);
                        poly++;
                        cursor = (s32*)((u8*)cursor + 0x28);
                    }
                    else if (temp_v1_13 >= 0x1000)
                    {
                        addPrim(&base[0xFFF], cursor);
                        poly++;
                        cursor = (s32*)((u8*)cursor + 0x28);
                    }
                    else
                    {
                        poly++;
                        setaddr(cursor, getaddr(&base[temp_v1_13])),
                        setaddr(&base[(s32)rec->unk8 >> 7], cursor);
                        cursor = (s32*)((u8*)cursor + 0x28);
                    }
                }
            }
            else if ((flag == 0) || ((temp_v1_17 = temp_v1_4 & 0xF) == 2) || ((slot->unk178 & 1) && (temp_v1_17 == 0)))
            {
                temp_v1_18 = item[7] & 0xF;
                switch (temp_v1_18)
                {
                case 0:
                    var_a3 = 0;
                    field_transform_effect_quad_vertices8(rec, slot, item, var_a3, sxy, dir, gte_out);

                    break;
                case 3:
                    field_transform_effect_quad_vertices8(rec, slot, item, 4, sxy, dir, gte_out);
                    break;
                case 6:
                    temp_v1_19 = slot->unk3C;
                    if (!(temp_v1_19 & 0x8000) && !(slot->unk174 & 0x1800) && (temp_v1_19 != 0xFFFF) &&
                        (((temp_a0_4 = rec->unk2A, (temp_a0_4 == 0x91)) && ((rec->unk21 & 0x7F) == 0x2E)) || (temp_a0_4 == 0x85) || (temp_a0_4 == 0x98)) &&
                        !(rec->unk3C & 0x01000000))
                    {
                        var_s0 = func_800839F8(rec->unk3A, 0);
                        if (var_s0 != -1U)
                        {
                            if (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0)
                            {
                                ((u8*)&slot->unk178)[1] = var_s0;
                                field_start_actor_animation(var_s0, 0, NULL);

                                {
                                    if (rec->unk21 & 0x80)
                                    {
                                        dir->unk0 = (s16)(s8)(u8)item[1];
                                        dir->unk2 = 0;
                                        dir->unk4 = (s16) - (s8)*item;
                                    }
                                    else
                                    {
                                        dir->unk0 = (s16)(s8)(u8)item[1];
                                        dir->unk2 = 0;
                                        dir->unk4 = (s16)(s8)*item;
                                    }
                                    gte_ldv0(dir);
                                    gte_rtv0();
                                    gte_stlvnl(gte_out);
                                    slot->unk130 = (u16)gte_out->vx;
                                    slot->unk132 = (u16)gte_out->vy;
                                    if (rec->unk21 & 0x80)
                                    {
                                        dir->unk0 = (s16)(s8)item[3];
                                        dir->unk2 = 0;
                                        dir->unk4 = (s16) - (s8)item[2];
                                    }
                                    else
                                    {
                                        dir->unk0 = (s16)(s8)item[3];
                                        dir->unk2 = 0;
                                        dir->unk4 = (s16)(s8)item[2];
                                    }
                                    gte_ldv0(dir);
                                    gte_rtv0();
                                    gte_stlvnl(gte_out);
                                    slot->unk134 = (u16)gte_out->vx;
                                    slot->unk136 = (u16)gte_out->vy;
                                    if (rec->unk21 & 0x80)
                                    {
                                        dir->unk0 = (s16)(s8)item[5];
                                        dir->unk2 = 0;
                                        dir->unk4 = (s16) - (s8)item[4];
                                    }
                                    else
                                    {
                                        dir->unk0 = (s16)(s8)item[5];
                                        dir->unk2 = 0;
                                        dir->unk4 = (s16)(s8)item[4];
                                    }
                                    gte_ldv0(dir);
                                    gte_rtv0();
                                    gte_stlvnl(gte_out);
                                    slot->unk138 = (u16)gte_out->vx;
                                    slot->unk13A = (u16)gte_out->vy;
                                    if (rec->unk21 & 0x80)
                                    {
                                        dir->unk0 = (s16)(s8)item[8];
                                        dir->unk2 = 0;
                                        dir->unk4 = (s16) - (s8)item[6];
                                    }
                                    else
                                    {
                                        dir->unk0 = (s16)(s8)item[8];
                                        dir->unk2 = 0;
                                        dir->unk4 = (s16)(s8)item[6];
                                    }
                                    gte_ldv0(dir);
                                    gte_rtv0();
                                    gte_stlvnl(gte_out);
                                    slot->unk13C = (u16)gte_out->vx;
                                    slot->unk13E = (u16)gte_out->vy;
                                }
                            }
                        }
                        slot->unk3C = 0xFFFF;
                        slot->unk174 = (slot->unk174 & ~0x1800) | 0x1000;
                    }
                    break;
                case 1:
                    if ((part->unk24 & 0x100000) && !(slot->unk174 & 0x1800) && ((((u8*)&slot->unk178)[3] == 0) || (rec->unk2A == 0x91)) &&
                        (((temp_v1_20 = slot->unk3C, (temp_v1_20 != 0xFFFF)) && (temp_v1_20 != 0)) || (actor->unkC->unk14 == 3)))
                    {
                        if (rec->unk21 & 0x80)
                        {
                            dir->unk0 = (s16)(s8)(u8)item[1];
                            dir->unk2 = 0;
                            dir->unk4 = (s16) - (s8)*item;
                        }
                        else
                        {
                            dir->unk0 = (s16)(s8)(u8)item[1];
                            dir->unk2 = 0;
                            dir->unk4 = (s16)(s8)*item;
                        }
                        gte_ldv0(dir);
                        gte_rtv0();
                        gte_stlvnl(gte_out);
                        sp68[0].x = (s16)(sxy->x + gte_out->vx);
                        sp68[0].y = (s16)(sxy->y + gte_out->vy);
                        if (rec->unk21 & 0x80)
                        {
                            dir->unk0 = (s16)(s8)item[3];
                            dir->unk2 = 0;
                            dir->unk4 = (s16) - (s8)item[2];
                        }
                        else
                        {
                            dir->unk0 = (s16)(s8)item[3];
                            dir->unk2 = 0;
                            dir->unk4 = (s16)(s8)item[2];
                        }
                        gte_ldv0(dir);
                        gte_rtv0();
                        gte_stlvnl(gte_out);
                        sp68[1].x = (s16)(sxy->x + gte_out->vx);
                        sp68[1].y = (s16)(sxy->y + gte_out->vy);
                        if (rec->unk21 & 0x80)
                        {
                            dir->unk0 = (s16)(s8)item[5];
                            dir->unk2 = 0;
                            dir->unk4 = (s16) - (s8)item[4];
                        }
                        else
                        {
                            dir->unk0 = (s16)(s8)item[5];
                            dir->unk2 = 0;
                            dir->unk4 = (s16)(s8)item[4];
                        }
                        gte_ldv0(dir);
                        gte_rtv0();
                        gte_stlvnl(gte_out);
                        sp68[2].x = (s16)(sxy->x + gte_out->vx);
                        sp68[2].y = (s16)(sxy->y + gte_out->vy);
                        if (rec->unk21 & 0x80)
                        {
                            dir->unk0 = (s16)(s8)item[8];
                            dir->unk2 = 0;
                            dir->unk4 = (s16) - (s8)item[6];
                        }
                        else
                        {
                            dir->unk0 = (s16)(s8)item[8];
                            dir->unk2 = 0;
                            dir->unk4 = (s16)(s8)item[6];
                        }
                        gte_ldv0(dir);
                        gte_rtv0();
                        gte_stlvnl(gte_out);
                        sp68[3].x = (s16)(sxy->x + gte_out->vx);
                        sp68[3].y = (s16)(sxy->y + gte_out->vy);
                        temp_v0_12 = field_test_quad_actor_contacts(sp68, rec, &sp28);
                        if (temp_v0_12 == 1)
                        {
                            temp_v0_12 = sp28.index;
                            temp_v0_13 = &FRAME_OBJECT_STATES[temp_v0_12];
                            temp_v0_13->unkC = (s32)(temp_v0_13->unkC & ~0x400);
                            *(((u8*)&slot->unk178)[3] + (u8*)slot + 0x180) = (u8)sp28.index;
                            temp_v1_21 = ((u8*)&slot->unk178)[3];
                            if (temp_v1_21 < 9U)
                            {
                                ((u8*)&slot->unk178)[3] = (u8)(temp_v1_21 + 1);
                            }
                            if (actor->unkC->unk14 == 3)
                            {
                                actor->unk23A = (u8)(actor->unk23A | (1 << actor->unk232));
                                actor->unk229[actor->unk232] = (u8)sp28.index;
                                sp6C->x = 0xA0 + g_field_view_offset_x / 256 + FRAME_ACTORS[sp28.index].unk0 / 256;
                                sp6C->y = 0x70 + g_field_view_offset_y / 256 + FRAME_ACTORS[sp28.index].unk4 / 256 - FRAME_ACTORS[sp28.index].unk8 / 512 - g_field_view_offset_z / 512;
                                if (FRAME_ACTORS[sp28.index].unk21 & 0x80)
                                {
                                    actor->unk1FE[actor->unk232].x = sp6C->x - sp28.x;
                                }
                                else
                                {
                                    actor->unk1FE[actor->unk232].x = sp28.x - sp6C->x;
                                }
                                actor->unk1FE[actor->unk232].y = (s16)(sp28.y - sp6C->y);
                                actor->unk232 = (u8)(actor->unk232 + 1);
                                temp_v0_14 = &FRAME_OBJECT_STATES[sp28.index];
                                temp_v0_14->unk178 = (s32)(temp_v0_14->unk178 | 0x80);

                                func_8008A840(actor->unk228, sp28.index);
                                temp_v0_15 = sp28.x - sxy->x;
                                temp_a1_2 = ~0x1800;
                                placement_flags = (slot->unk174 & temp_a1_2);
                                slot->unk13C = temp_v0_15;
                                slot->unk138 = temp_v0_15;
                                slot->unk134 = temp_v0_15;
                                slot->unk130 = temp_v0_15;
                                temp_v0_16 = sp28.y - sxy->y;
                                placement_flags |= 0x1000;
                                slot->unk174 = placement_flags;
                                slot->unk13E = temp_v0_16;
                                slot->unk13A = temp_v0_16;
                                slot->unk136 = temp_v0_16;
                                slot->unk132 = temp_v0_16;
                            }
                            else
                            {
                                temp_v1_23 = slot->unk3C;
                                if (temp_v1_23 == 0x24)
                                {
                                    if (FRAME_ACTORS[sp28.index].unk25 != 0)
                                    {
                                        break;
                                    }
                                    slot->unk178 = (u32)(slot->unk178 | 2);
                                    slot->unk170 = (u8)sp28.index;
                                    temp_v0_17 = &FRAME_OBJECT_STATES[sp28.index];
                                    temp_v0_17->unkC = (s32)(temp_v0_17->unkC | 0x2000);
                                }
                                else
                                {
                                    if ((temp_v1_23 == 0x59) || (temp_v1_23 == 0x66) || (temp_v1_23 == 0x2B))
                                    {
                                        if (!(((u32)FRAME_OBJECT_STATES[sp28.index].unk178 >> 6) & 1))
                                        {
                                            clamp_base = FRAME_ACTOR_BINDINGS;
                                            var_v1_10 = sp28.index < 3 ? sp28.index : 2;
                                            temp_v0_18 = clamp_base[var_v1_10].unkC;
                                            if (temp_v0_18 == sp28.index)
                                            {
                                                var_v1_11 = temp_v0_18 < 3 ? temp_v0_18 : 2;
                                                if (clamp_base[var_v1_11].unk0 == 0)
                                                {
                                                    goto block_208;
                                                }
                                                goto block_209;
                                            }
                                            goto block_208;
                                        }
                                    block_208:
                                        if ((var_a1 = (u8*)&FRAME_OBJECT_STATES[sp28.index], var_a1[0x178] & 1))
                                        {
                                        block_209:
                                            slot->unk3C = 0;
                                        }
                                        else
                                        {
                                            temp_v0_19 = &FRAME_ACTORS[sp28.index];
                                            temp_v0_19->unk21 = (u8)(temp_v0_19->unk21 & 0x7F);
                                            if (sp28.index < 2)
                                            {
                                                func_800A2DD8(sp28.index);
                                                FRAME_OBJECT_STATES[sp28.index].unk18D = 0;
                                                FRAME_ACTORS[sp28.index].unk30 = 0;
                                            }
                                        }
                                    }
                                    else
                                    {
                                    temp_v1_24 = &FRAME_OBJECT_STATES[sp28.index];
                                    var_s1 = 1;
                                    if (!(*(u8*)&temp_v1_24->unk178 & 1) ||
                                        ((actor_base = FRAME_ACTOR_SLOTS, temp_v1_25 = &actor_base[((u8*)&temp_v1_24->unk178)[2]],
                                          (temp_v1_25->unk24 != 0)) &&
                                         (temp_v1_25->unk228 == rec->unk3A)))
                                    {
                                        temp_v1_26 = rec->unk21 & 0x7F;
                                        switch (temp_v1_26)
                                        {
                                        case 0x48:
                                            var_s1 = func_8008A9D8(rec->unk3A, sp28.index, 0x10);
                                            break;
                                        case 0x49:
                                            var_s1 = func_8008A9D8(rec->unk3A, sp28.index, 0x11);
                                            break;
                                        case 0x3E:
                                            var_s1 = func_8008A9D8(rec->unk3A, sp28.index, 0x19);
                                            break;
                                        case 0x45:
                                            var_s1 = func_8008A9D8(rec->unk3A, sp28.index, 0x1A);
                                            break;
                                        default:
                                            var_s1 = func_8008A840(rec->unk3A, sp28.index);
                                            break;
                                        }
                                    }
                                    if (sp28.index < 2)
                                    {
                                        Struct_D800FDF58* entry;
                                        entry = &FRAME_ACTORS[sp28.index];
                                        if (!((*(u16*)&entry->unk1C) & 0x1FF))
                                        {
                                            func_800A2DD8(sp28.index);
                                        }
                                    }
                                    if (var_s1 == 1)
                                    {
                                        temp_v1_27 = slot->unk3C;
                                        if ((temp_v1_27 == 1) || (temp_v1_27 == 3) || (temp_v1_27 == 0x10))
                                        {
                                            slot->unk3C = 0x1E;
                                        }
                                    }
                                    }
                                }
                                    if (slot->unk3C & 0x8000)
                                    {
                                        scratch.sp30 = sp28.index;
                                        if (rec->unk2A == 0x91)
                                        {
                                            if (((u8*)&slot->unk178)[1] != 0xFF)
                                            {
                                                temp_v0_20 = &FRAME_OBJECT_STATES[sp28.index];
                                                temp_v0_20->unk178 = (s32)(temp_v0_20->unk178 | 0x80);
                                                field_start_actor_animation(((u8*)&slot->unk178)[1], 1, &scratch.sp30);
                                            }
                                        }
                                        else if (field_start_bound_action_animation(rec->unk3A, 1, &scratch.sp30, slot->unk3C) != 0)
                                        {
                                            slot->unk174 = (s32)((slot->unk174 & ~0x1800) | 0x1000);
                                        }
                                        temp_v1_29 = sp28.x - sxy->x;
                                        slot->unk13C = temp_v1_29;
                                        slot->unk138 = temp_v1_29;
                                        slot->unk134 = temp_v1_29;
                                        slot->unk130 = temp_v1_29;
                                        temp_v1_30 = sp28.y - sxy->y;
                                        slot->unk13E = temp_v1_30;
                                        slot->unk13A = temp_v1_30;
                                        slot->unk136 = temp_v1_30;
                                        slot->unk132 = temp_v1_30;
                                    }
                                    else if (slot->unk3C != 0)
                                    {
                                        var_s0 = func_800839F8(rec->unk3A, 0);
                                        if (var_s0 != -1U)
                                        {
                                            temp_v1_28 = ((u32)slot->unk178 >> 2) & 7;
                                            switch (temp_v1_28)
                                            {
                                            case 2:
                                                slot->unk3C = 0x74;
                                                break;
                                            case 4:
                                                slot->unk3C = 0x75;
                                                break;
                                            default:
                                                slot->unk3C = slot->unk3C;
                                                break;
                                            }
                                            if (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0)
                                            {
                                                ((u8*)&slot->unk178)[1] = var_s0;
                                                scratch.sp30 = sp28.index;
                                                field_start_actor_animation(var_s0, 1, &scratch.sp30);
                                                temp_v1_29 = sp28.x - sxy->x;
                                                slot->unk13C = temp_v1_29;
                                                slot->unk138 = temp_v1_29;
                                                slot->unk134 = temp_v1_29;
                                                slot->unk130 = temp_v1_29;
                                                temp_v1_30 = sp28.y - sxy->y;
                                                slot->unk13E = temp_v1_30;
                                                slot->unk13A = temp_v1_30;
                                                slot->unk136 = temp_v1_30;
                                                slot->unk132 = temp_v1_30;
                                            }
                                        }
                                    }
                                    slot->unk3C = 0xFFFF;
                                    slot->unk174 = (slot->unk174 & ~0x1800) | 0x1000;
                                    break;
                            }
                        }
                        else
                        {
                            if (temp_v0_12 == 2)
                            {
                                slot->unk3C = 0x1E;
                                var_s0 = func_800839F8(rec->unk3A, 0);
                                if (var_s0 != -1U)
                                {
                                    if (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0)
                                    {
                                        ((u8*)&slot->unk178)[1] = var_s0;
                                        scratch.sp30 = sp28.index;
                                        field_start_actor_animation(var_s0, 1, &scratch.sp30);
                                        {
                                            temp_v1_31 = sp28.x - sxy->x;
                                            slot->unk13C = temp_v1_31;
                                            slot->unk138 = temp_v1_31;
                                            slot->unk134 = temp_v1_31;
                                            slot->unk130 = temp_v1_31;
                                            temp_v1_32 = sp28.y - sxy->y;
                                            slot->unk13E = temp_v1_32;
                                            slot->unk13A = temp_v1_32;
                                            slot->unk136 = temp_v1_32;
                                            slot->unk132 = temp_v1_32;
                                        }
                                    }
                                }
                                slot->unk3C = 0xFFFF;
                                slot->unk174 = (slot->unk174 & ~0x1800) | 0x1000;
                                break;
                            }
                            if (temp_v0_12 == 3)
                            {
                                slot->unk3C = 0xFFFF;
                                slot->unk174 = (slot->unk174 & ~0x1800) | 0x1000;
                                break;
                            }
                            break;
                        }
                    }
                    break;
                case 5:
                    if ((rec->unk34 == 0) && !(rec->unk3C & 0x01000000))
                    {
                        temp_v1_33 = (u16)FRAME_RESOURCE_ENTRIES[rec->unk3B].unkA >> 0xC;
                        if (temp_v1_33 != 1)
                        {
                            if ((s32)temp_v1_33 < 2)
                            {
                                if (temp_v1_33 == 0)
                                {

                                    func_800A3938(FRAME_RESOURCE_ENTRIES[rec->unk3B].unkA & 0xFFF, field_get_actor_sound_pan(rec->unk3A));
                                }
                            }
                        }
                        else
                        {

                            func_800A39A8(FRAME_RESOURCE_ENTRIES[rec->unk3B].unkA & 0xFFF, field_get_actor_sound_pan(rec->unk3A), rec->unk3B - 3, rec->unk3A);
                        }
                    }
                    break;
                case 2:
                    if (part->unk24 & 0x100000)
                    {
                        field_unpack_effect_quad_corners8(sp64, rec->unk21 & 0x80, (s8*)item);
                        sp78 += 1;
                    }
                    break;
                case 4:
                    if ((slot->unk174 & 0x1800) != 0x800)
                    {
                        temp_v1_34 = slot->unk3C;
                        if ((temp_v1_34 != 0xFFFF) && (temp_v1_34 != 0))
                        {
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16)(s8)(u8)item[1];
                                dir->unk2 = 0;
                                dir->unk4 = (s16) - (s8)*item;
                            }
                            else
                            {
                                dir->unk0 = (s16)(s8)(u8)item[1];
                                dir->unk2 = 0;
                                dir->unk4 = (s16)(s8)*item;
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk130 = (u16)gte_out->vx;
                            slot->unk132 = (u16)(gte_out->vy - sp74);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16)(s8)item[3];
                                dir->unk2 = 0;
                                dir->unk4 = (s16) - (s8)item[2];
                            }
                            else
                            {
                                dir->unk0 = (s16)(s8)item[3];
                                dir->unk2 = 0;
                                dir->unk4 = (s16)(s8)item[2];
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk134 = (u16)gte_out->vx;
                            slot->unk136 = (u16)(gte_out->vy - sp74);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16)(s8)item[5];
                                dir->unk2 = 0;
                                dir->unk4 = (s16) - (s8)item[4];
                            }
                            else
                            {
                                dir->unk0 = (s16)(s8)item[5];
                                dir->unk2 = 0;
                                dir->unk4 = (s16)(s8)item[4];
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk138 = (u16)gte_out->vx;
                            slot->unk13A = (u16)(gte_out->vy - sp74);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16)(s8)item[8];
                                dir->unk2 = 0;
                                dir->unk4 = (s16) - (s8)item[6];
                            }
                            else
                            {
                                dir->unk0 = (s16)(s8)item[8];
                                dir->unk2 = 0;
                                dir->unk4 = (s16)(s8)item[6];
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk13C = (u16)gte_out->vx;
                            temp_a3_2 = slot->unk3C;
                            slot->unk13E = (u16)(gte_out->vy - sp74);
                            if (temp_a3_2 & 0x8000)
                            {
                                if (field_start_bound_action_animation(rec->unk3A, 0, NULL, temp_a3_2) != 0)
                                {
                                    slot->unk174 = (s32)((slot->unk174 & ~0x1800) | 0x1000);
                                }
                            }
                            else
                            {
                                var_s0 = func_800839F8(rec->unk3A, 0);
                                if (var_s0 != -1U)
                                {
                                    if (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0)
                                    {
                                        ((u8*)&slot->unk178)[1] = var_s0;
                                        field_start_actor_animation(var_s0, 0, NULL);
                                    }
                                }
                            }
                            slot->unk3C = 0xFFFF;
                            slot->unk174 = (slot->unk174 & ~0x1800) | 0x800;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            item += 9;
        } while (--item_count != 0);
    }
    if (sp78 != 0)
    {
        s16* p = sp64;
        while (p != sp64 + 8)
        {
            p[0] = ((p[0] * part->unk2E >> 6) * g_field_effect_track_scale.x) >> 12;
            p[1] = ((p[1] * part->unk33 >> 6) * g_field_effect_track_scale.z) >> 12;
            p += 2;
        }
        slot->unk12C = (s16)((s32)(sp64[2] + sp64[0]) >> 1);
        var_v0_27 = abs(sp64[2] - sp64[0]);
        slot->unk12E = var_v0_27;
        if (slot->unk12E == 0)
        {
            slot->unk12E = abs((s16)sp64[4] - sp64[0]);
            slot->unk12C = (s16)((s32)((s16)sp64[4] + sp64[0]) >> 1);
        }
        cursor = field_render_actor_ground_shadow(rec, cursor, base, sp64);
    }
    return cursor;
}


/* field12 */
/**
 * @file field12.c
 * @brief Field animation-frame audio/visual processor, carved from the top
 *        of the unk2 segment (the single-function slot right after
 *        field11.c's func_80075C88).
 */

#include "common.h"
#include "field_types.h"
typedef struct
{
    u32 addr : 24;
    u32 len : 8;
} PrimitiveTag;








/* This function needs the full 32-bit unk34 (masked against 0x100000), unlike
 * the u16 view used by field8.c/field10.c's local copy of the same struct. */






/* Local view of the shared per-track "slot" record; only the fields this
 * function touches are named, matching the per-file minimal-view convention
 * already used by field7.c/field8.c/field10.c/field11.c for the same real
 * struct. */






/* Output of field_test_quad_actor_contacts: resolved track/actor index plus the screen-space
 * x/y it computed for it. */


#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"



/**
 * @brief Field animation-frame audio/visual processor for the "non-negative
 *        item" resource variant, spawning per-frame billboard primitives and
 *        panning any attached sound cue relative to the camera.
 * @param rec Effect record.
 * @param cursor Vertex-buffer cursor pointer, threaded and returned.
 * @param base Ordering-table / primitive base array.
 * @param item Animation data blob for this frame.
 * @param flag Selects which of the actor's two audio channels to update.
 * @param part Part definition supplying flags and placement selectors.
 * @return Updated cursor pointer.
 * @note Sibling of field11.c's func_80075C88 (called for the item < 0 case
 *       there, this one for item >= 0); same struct types, GTE pan-vector
 *       macros, and dual int-pointer/byte-pointer cursor idiom, but a simpler
 *       placement-opcode dispatch (no 0x7F0000-flags branch tree) and a wider
 *       11-byte (0xB) per-frame record with 16-bit byte-pair deltas instead of
 *       func_80075C88's 8-bit signed deltas.
 * @note gte_dmpsx_compat.h supplies the COP2 words used by the GTE macros.
 * @see decomp.me (100%)
 */
s32 *func_80077FB4(Struct_D800FDF58 *rec, s32 *cursor, s32 *base, u8 *item, s32 flag, FramePartDef *part)
{
    MATRIX *mtx = (MATRIX *) 0x1F800000;
    Vec2s *sxy = (Vec2s *) 0x1F800040;
    s32 sp5C;
    VECTOR *gte_out = (VECTOR *) 0x1F800044;
    FrameSVector *dir = (FrameSVector *) 0x1F800054;
    s16 *sp60 = (s16 *) 0x1F800064;
    Vec2s *sp64;
    Vec2s *sp68;
    FrameActorState *actor;
    s32 sp70;
    s32 sp74;
    TrackPlacement sp28;
    struct { s32 sp30; u8 pad34[0x24]; } scratch;
    u8 *var_s1;
    POLY_FT4 *poly;
    s16 temp_v1_24;
    s32 var_s0;
    s32 *var_a2;
    s32 temp_a0_3;
    s32 temp_a1_2;
    s32 temp_s6;
    s32 temp_t0_call;
    s32 temp_v0_5;
    s32 temp_v1_10;
    s32 temp_v1_11;
    s32 temp_v1_15;
    s32 temp_v1_17;
    s32 temp_v1_20;
    s32 temp_v1_23;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1_2;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_9;
    s32 var_v1;
    s32 var_a1;
    s32 var_v1_2;
    s32 var_v1_3;
    u16 temp_a1;
    u16 temp_v0_7;
    u16 temp_v0_8;
    u16 temp_v1_21;
    u16 temp_v1_22;
    u16 var_v1_7;
    u32 temp_v1_16;
    u32 temp_v1_25;
    u32 temp_v1_3;
    u8 temp_a0;
    s32 temp_s4;
    s32 temp_t1;
    u8 temp_v0;
    u8 temp_v0_2;
    u8 temp_v0_3;
    u8 temp_v0_4;
    u8 temp_v1;
    u8 temp_v1_18;
    u8 temp_v1_4;
    s32 temp_v1_5;
    s32 temp_v1_6;
    s32 temp_v1_7;
    s32 temp_v1_8;
    u8 temp_v1_9;
    u8 var_a0_3;
    u8 var_a0_4;
    u8 var_a0_5;
    u8 var_a0_6;
    Struct_D80105AE0 *slot;
    Struct_D80105AE0 *temp_v0_6;

    sp74 = 0;
    sp64 = (Vec2s *) 0x1F800080;
    sp68 = (Vec2s *) 0x1F800094;
    slot = &FRAME_OBJECT_STATES[rec->unk3A];
    actor = &FRAME_ACTOR_SLOTS[rec->unk22];
    *(s32 *) &slot->unk12C = 0;
    {
        s32 *zero_ptr = (s32 *) ((u8 *) slot + 0x1C);
        var_a1 = 7;
        do
        {
            *(s32 *) ((u8 *) zero_ptr + 0x148) = 0;
            var_a1 -= 1;
            zero_ptr -= 1;
        } while (var_a1 >= 0);
    }
    *(s32 *) &slot->unk144 = 0;
    *(s32 *) &slot->unk140 = 0;
    field_build_effect_part_matrix(rec, part, mtx, actor);
    gte_SetRotMatrix(mtx);

    sxy->x = 0xA0 + g_field_view_offset_x / 256 + rec->unk0 / 256;
    temp_a1 = 0x70 + g_field_view_offset_y / 256 + rec->unk4 / 256 - rec->unk8 / 512 - g_field_view_offset_z / 512;
    sxy->y = temp_a1;
    temp_v1 = rec->unk37;
    temp_a0 = rec->unk38;
    if ((temp_v1 | temp_a0) != 0)
    {
        temp_v1_2 = (s8) temp_v1 + ((s32) (((s8) temp_a0 - (s8) temp_v1) * rec->unk34) / (s32) rec->unk35);
        sp70 = temp_v1_2;
        sxy->y = (u16) (temp_a1 - temp_v1_2);
    }
    else
    {
        sp70 = 0;
    }
    field_extract_effect_quad_corners16(rec, item, sp60);
    temp_v1_3 = part->unk34;
    if (temp_v1_3 & 0x100000)
    {
        field_apply_effect_quad_center_offset(rec, sxy, sp60, mtx, (temp_v1_3 >> 0x14) & 1);
    }
    slot->unk140 = (u16) sp60[0];
    slot->unk142 = (u16) sp60[1];
    slot->unk144 = (u16) sp60[4];
    slot->unk146 = (u16) sp60[5];
    temp_t1 = *item;
    sp5C = (s32) temp_t1;
    item += 1;
    if (temp_t1 != 0)
    {
        var_s1 = item + 0x11;
        poly = (POLY_FT4 *) cursor;
        do
        {
            temp_v1_4 = var_s1[-0xA];
            if (!(temp_v1_4 & 0x20))
            {
                field_resolve_effect_part_color(actor, rec, part, (FieldPrimitiveColor*)&((P_TAG*)cursor)->r0);
                setPolyFT4(poly);
                setSemiTrans(poly, rec->unk1C & 0x800000);
                temp_s4 = var_s1[-0xD];
                temp_s6 = var_s1[-0xC] - 1;
                temp_t0_call = (s32) (s16) (var_s1[-0x10] + (var_s1[-0x7] << 8));
                var_s0 = (s16) (*item + (var_s1[-0x8] << 8));
                if (rec->unk21 & 0x80)
                {
                    var_s0 = -var_s0 - temp_s4;
                }
                temp_s4 -= 1;
                field_project_effect_sprite_quad(rec, sxy, (POLY_FT4*)cursor, temp_s4, temp_s6, (s32) var_s0, temp_t0_call, (FieldSpriteFrame*)item, mtx);
                if ((var_s1[-0xA] ^ ((u8) rec->unk21 >> 1)) & 0x40)
                {
                    temp_v0 = var_s1[-0xF];
                    poly->u1 = temp_v0;
                    poly->u3 = temp_v0;
                    temp_v1_5 = poly->u1 + temp_s4;
                    var_a0_3 = 0xFF;
                    if (temp_v1_5 != 0x100)
                    {
                        var_a0_3 = temp_v1_5;
                    }
                    poly->u2 = var_a0_3;
                    poly->u0 = var_a0_3;
                }
                else
                {
                    temp_v0_2 = var_s1[-0xF];
                    poly->u0 = temp_v0_2;
                    poly->u2 = temp_v0_2;
                    temp_v1_6 = poly->u0 + temp_s4;
                    var_a0_4 = 0xFF;
                    if (temp_v1_6 != 0x100)
                    {
                        var_a0_4 = temp_v1_6;
                    }
                    poly->u3 = var_a0_4;
                    poly->u1 = var_a0_4;
                }
                if (var_s1[-0xA] & 0x80)
                {
                    temp_v0_3 = var_s1[-0xE];
                    poly->v2 = temp_v0_3;
                    poly->v3 = temp_v0_3;
                    temp_v1_7 = poly->v2 + temp_s6;
                    var_a0_5 = 0xFF;
                    if (temp_v1_7 != 0x100)
                    {
                        var_a0_5 = temp_v1_7;
                    }
                    poly->v1 = var_a0_5;
                    poly->v0 = var_a0_5;
                }
                else
                {
                    temp_v0_4 = var_s1[-0xE];
                    poly->v0 = temp_v0_4;
                    poly->v1 = temp_v0_4;
                    temp_v1_8 = poly->v0 + temp_s6;
                    var_a0_6 = 0xFF;
                    if (temp_v1_8 != 0x100)
                    {
                        var_a0_6 = temp_v1_8;
                    }
                    poly->v3 = var_a0_6;
                    poly->v2 = var_a0_6;
                }
                var_a1 = rec->unkC;
                temp_a0_3 = var_a1 << 7;
                if (var_a1 >= 2)
                {
                    temp_a0_3 = var_a1 << 6;
                    if (var_a1 >= 9)
                    {
                        {
                            s32 v0;
                            s32 v1;
                            s32 a0;
                            v0 = var_a1 - 9;
                            v0 <<= 6;
                            v1 = var_s1[-0xA];
                            v1 &= 3;
                            v1 <<= 6;
                            v1 += 0x3C0;
                            v1 -= v0;
                            v1 &= 0x3FF;
                            a0 = part->unk4;
                            v1 = (s32) v1 >> 6;
                            a0 = (s32) ((u32) a0 >> 0x11);
                            a0 &= 0x60;
                            a0 |= 0x10;
                            a0 |= v1;
                            poly->tpage = (s16) a0;
                        }
                    }
                    else
                    {
                        var_v1_3 = var_s1[-0xA];
                        var_v1_3 &= 3;
                        var_v1_3 <<= 6;
                        var_v1_3 += 0x340;
                        var_v1_3 -= temp_a0_3;
                        var_v1_3 = (var_v1_3 & 0x3FF) >> 6;
                        var_v0_5 = ((u32) part->unk4 >> 0x11) & 0x60;
                        goto block_48;
                    }
                }
                else
                {
                    temp_a0_3 = 0x380 - temp_a0_3;
                    var_v1_3 = ((u32) part->unk4 >> 0x11) & 0x60;
                    if (var_s1[-0xA] & 3)
                    {
                        var_v0_6 = (temp_a0_3 + 0x40) & 0x3FF;
                    }
                    else
                    {
                        var_v0_6 = temp_a0_3 & 0x3FF;
                    }
                    var_v0_5 = var_v0_6 >> 6;
block_48:
                    poly->tpage = (s16) (var_v1_3 | var_v0_5);
                }
                if (!(((u32)part->unk28 >> 12) & 3))
                {
                    if (var_s1[-0xB] == 0xB)
                    {
                        setSemiTrans(poly, 1);
                    }
                    temp_v0_5 = rec->unk3B;
                    temp_v1_10 = var_s1[-0xB];
                    temp_v0_5 += 0x1F4;
                    temp_v0_5 <<= 6;
                    temp_v1_10 &= 0x3F;
                    poly->clut = (s16)(temp_v0_5 | temp_v1_10);
                }
                else
                {
                    temp_v1_9 = part->unk2D;
                    if (temp_v1_9 >= 0x40U)
                    {
                        temp_t0_call = 0x1F2;
                        if ((u8)actor->unk228 < 2U)
                        {
                            temp_t0_call = (actor->unk228 * 2) + 0x1EE;
                        }
                    }
                    else
                    {
                        temp_t0_call = (temp_v1_9 >> 4) + 0x1EA;
                    }
                    temp_v1_10 = ((u32)part->unk28 >> 0xC) & 3;
                    switch (temp_v1_10)
                    {
                    case 1:
                    {
                        temp_v1_10 = temp_t0_call << 6;
                        temp_v0_5 = part->unk2D & 0xF;
                        temp_v1_10 |= temp_v0_5;
                        poly->clut = (s16)temp_v1_10;
                    }
                    break;
                    case 2:
                        if (actor->unk228 >= 3)
                        {
                            temp_v1_10 = temp_t0_call << 6;
                            temp_v0_5 = part->unk2D & 0xF;
                            temp_v1_10 |= temp_v0_5;
                            poly->clut = (s16)temp_v1_10;
                        }
                        else
                        {
                            if (var_s1[-0xB] == 0xB)
                            {
                                setSemiTrans(poly, 1);
                            }
                            temp_v0_5 = rec->unk3B;
                            temp_v1_10 = var_s1[-0xB];
                            temp_v0_5 += 0x1F4;
                            temp_v0_5 <<= 6;
                            temp_v1_10 &= 0x3F;
                            poly->clut = (s16)(temp_v0_5 | temp_v1_10);
                        }
                        break;
                    }
                }
                temp_v1_11 = (s32) rec->unk8 >> 7;
                if (temp_v1_11 < 0)
                {
                    addPrim(base, cursor);
                    poly++;
                    cursor = (s32*)((u8*)cursor + 0x28);
                }
                else if (temp_v1_11 >= 0x1000)
                {
                    addPrim(&base[0xFFF], cursor);
                    poly++;
                    cursor = (s32*)((u8*)cursor + 0x28);
                }
                else
                {
                    poly++;
                    setaddr(cursor, getaddr(&base[temp_v1_11])),
                    setaddr(&base[(s32)rec->unk8 >> 7], cursor);
                    cursor = (s32*)((u8*)cursor + 0x28);
                }
                var_s1 += 0xB;
                item += 0xB;
            }
            else
            {
                temp_v1_15 = temp_v1_4 & 0xF;
                if ((flag == 0) || (temp_v1_15 == 2) || ((slot->unk178 & 1) && (temp_v1_15 == 0)))
                {
                    temp_v1_16 = var_s1[-0xA] & 0xF;
                    switch (temp_v1_16)
                    {
                    case 0:
                        field_transform_effect_quad_vertices16(rec, slot, item, 0, sxy, dir, gte_out);
                        break;
                    case 3:
                        field_transform_effect_quad_vertices16(rec, slot, item, 4, sxy, dir, gte_out);
                        break;
                    case 1:
                        if (part->unk24 & 0x100000)
                        {
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(*item + (var_s1[-0x10] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (*item + (var_s1[-0x10] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            sp64[0].x = (s16) (sxy->x + gte_out->vx);
                            sp64[0].y = (s16) (sxy->y + gte_out->vy);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0xD] + (var_s1[-0xC] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0xD] + (var_s1[-0xC] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            sp64[1].x = (s16) (sxy->x + gte_out->vx);
                            sp64[1].y = (s16) (sxy->y + gte_out->vy);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0x8] + (var_s1[-0x7] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0x8] + (var_s1[-0x7] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            sp64[2].x = (s16) (sxy->x + gte_out->vx);
                            sp64[2].y = (s16) (sxy->y + gte_out->vy);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0x4] + (var_s1[-0x3] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0x4] + (var_s1[-0x3] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            sp64[3].x = (s16) (sxy->x + gte_out->vx);
                            sp64[3].y = (s16) (sxy->y + gte_out->vy);
                            if (!(slot->unk174 & 0x1800) && (((temp_v1_17 = slot->unk3C, (temp_v1_17 != 0xFFFF)) && (temp_v1_17 != 0)) || (actor->unkC->unk14 == 3)))
                            {
                                var_s0 = field_test_quad_actor_contacts(sp64, rec, &sp28);
                                if (var_s0 == 1)
                                {
                                    {
                                        Struct_D80105AE0 *slots_base = FRAME_OBJECT_STATES;
                                        temp_v0_6 = &slots_base[sp28.index];
                                    }
                                    temp_v0_6->unkC = (s32) (temp_v0_6->unkC & ~0x400);
                                    {
                                        u8 *targets = (u8 *)slot + 0x180;
                                        targets[((u8 *)&slot->unk178)[3]] = sp28.index;
                                    }
                                    temp_v1_18 = ((u8 *) &slot->unk178)[3];
                                    if (temp_v1_18 < 9U)
                                    {
                                        ((u8 *) &slot->unk178)[3] = (u8) (temp_v1_18 + 1);
                                    }
                                    if (actor->unkC->unk14 == 3)
                                    {
                                        actor->unk23A = (u8) (actor->unk23A | (var_s0 << actor->unk232));
                                        actor->unk229[actor->unk232] = (u8) sp28.index;
                                        {
                                            s32 a0;
                                            s32 a1;
                                            Struct_D800FDF58 *entry;

                                            a0 = g_field_view_offset_x;
                                            if (a0 < 0)
                                            {
                                                a0 += 0xFF;
                                            }
                                            var_v1_3 = FRAME_ACTORS[sp28.index].unk0;
                                            a1 = a0 >> 8;
                                            if (var_v1_3 < 0)
                                            {
                                                var_v1_3 += 0xFF;
                                            }
                                            a0 = g_field_view_offset_y;
                                            do
                                            {
                                                var_v0_5 = var_v1_3 >> 8;
                                                var_v0_5 += 0xA0;
                                                sp68->x = (u16)(a1 + var_v0_5);
                                            } while (0);
                                            if (a0 < 0)
                                            {
                                                a0 += 0xFF;
                                            }
                                            entry = &FRAME_ACTORS[sp28.index];
                                            var_v0_5 = entry->unk4;
                                            a1 = a0 >> 8;
                                            if (var_v0_5 < 0)
                                            {
                                                var_v0_5 += 0xFF;
                                            }
                                            a0 = entry->unk8;
                                            var_v0_5 >>= 8;
                                            var_v0_5 += 0x70;
                                            a1 += var_v0_5;
                                            if (a0 < 0)
                                            {
                                                a0 += 0x1FF;
                                            }
                                            var_v1_3 = g_field_view_offset_z;
                                            var_v0_5 = a0 >> 9;
                                            a0 = a1 - var_v0_5;
                                            if (var_v1_3 < 0)
                                            {
                                                var_v1_3 += 0x1FF;
                                            }
                                            var_v0_5 = var_v1_3 >> 9;
                                            sp68->y = (u16) (a0 - var_v0_5);
                                        }
                                        if (FRAME_ACTORS[sp28.index].unk21 & 0x80)
                                        {
                                            actor->unk1FE[actor->unk232].x = sp68->x - sp28.x;
                                        }
                                        else
                                        {
                                            actor->unk1FE[actor->unk232].x = sp28.x - sp68->x;
                                        }
                                        actor->unk1FE[actor->unk232].y = (s16) (sp28.y - sp68->y);
                                        actor->unk232 = (u8) (actor->unk232 + 1);
                                        func_8008A840(actor->unk228, sp28.index);
                                        do
                                        {
                                            do
                                            {
                                                {
                                                    s32 placement_flags;
                                                    temp_a1_2 = ~0x1800;
                                                    temp_v0_7 = sp28.x - sxy->x;
                                                    placement_flags = slot->unk174 & temp_a1_2;
                                                    slot->unk13C = temp_v0_7;
                                                    slot->unk138 = temp_v0_7;
                                                    slot->unk134 = temp_v0_7;
                                                    slot->unk130 = temp_v0_7;
                                                    temp_v0_8 = sp28.y - sxy->y;
                                                    placement_flags |= 0x1000;
                                                    slot->unk174 = placement_flags;
                                                    slot->unk13E = temp_v0_8;
                                                    slot->unk13A = temp_v0_8;
                                                    slot->unk136 = temp_v0_8;
                                                    slot->unk132 = temp_v0_8;
                                                }
                                            } while (0);
                                        } while (0);
                                    }
                                    else
                                    {
                                        if ((func_8008A840(rec->unk3A, sp28.index) == var_s0) && ((temp_v1_20 = slot->unk3C, (temp_v1_20 == var_s0)) || (temp_v1_20 == 3) || (temp_v1_20 == 0x10)))
                                        {
                                            slot->unk3C = 0x1E;
                                        }
                                        var_a1_2 = 1;
                                        if (slot->unk3C & 0x8000)
                                        {
                                            scratch.sp30 = sp28.index;
                                            var_a2 = &scratch.sp30;
block_176:
                                            if (field_start_bound_action_animation(rec->unk3A, var_a1_2, var_a2, slot->unk3C) != 0)
                                            {
                                                slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                                            }
                                        }
                                        else
                                        {
                                            var_s0 = func_800839F8(rec->unk3A, 0);
                                            if (var_s0 != -1)
                                            {
                                                if (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0)
                                                {
                                                    ((u8 *) &slot->unk178)[1] = var_s0;
                                                    scratch.sp30 = sp28.index;
                                                    field_start_actor_animation(var_s0, 1, &scratch.sp30);
                                                    temp_v1_21 = sp28.x - sxy->x;
                                                    slot->unk13C = temp_v1_21;
                                                    slot->unk138 = temp_v1_21;
                                                    slot->unk134 = temp_v1_21;
                                                    slot->unk130 = temp_v1_21;
                                                    var_v1_7 = sp28.y - sxy->y;
                                                    goto block_126;
                                                }
                                            }
                                        }
                                        goto block_182;
                                    }
                                }
                                else
                                {
                                    if (var_s0 == 2)
                                    {
                                        slot->unk3C = 0x1E;
                                        var_s0 = func_800839F8(rec->unk3A, 0);
                                        if (var_s0 != -1)
                                        {
                                            if (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0)
                                            {
                                                ((u8 *) &slot->unk178)[1] = var_s0;
                                                scratch.sp30 = sp28.index;
                                                field_start_actor_animation(var_s0, 1, &scratch.sp30);
                                                temp_v1_22 = sp28.x - sxy->x;
                                                slot->unk13C = temp_v1_22;
                                                slot->unk138 = temp_v1_22;
                                                slot->unk134 = temp_v1_22;
                                                slot->unk130 = temp_v1_22;
                                                var_v1_7 = sp28.y - sxy->y;
block_126:
                                                slot->unk13E = var_v1_7;
                                                slot->unk13A = var_v1_7;
                                                slot->unk136 = var_v1_7;
                                                slot->unk132 = var_v1_7;
                                            }
                                        }
block_182:
                                        slot->unk3C = 0xFFFF;
                                        goto block_183;
                                    }
                                    if (var_s0 == 3)
                                    {
                                        slot->unk3C = 0xFFFF;
block_183:
                                        slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                                    }
                                }
                            }
                        }
                        break;
                    case 6:
                        temp_v1_23 = slot->unk3C;
                        if (!(temp_v1_23 & 0x8000) && (temp_v1_23 != 0xFFFF) && (((temp_v1_24 = rec->unk2A, (temp_v1_24 == 0x91)) || (temp_v1_24 == 0x85) || (temp_v1_24 == 0x86) || (temp_v1_24 == 0x98))) && !(rec->unk3C & 0x01000000))
                        {
                            var_s0 = func_800839F8(rec->unk3A, 0);
                            if ((var_s0 != -1) && (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0))
                            {
                                ((u8 *) &slot->unk178)[1] = var_s0;
                                field_start_actor_animation(var_s0, 0, NULL);
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) -(*item + (var_s1[-0x10] << 8));
                                }
                                else
                                {
                                    dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) (*item + (var_s1[-0x10] << 8));
                                }
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk130 = (u16) gte_out->vx;
                                slot->unk132 = (u16) gte_out->vy;
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) -(var_s1[-0xD] + (var_s1[-0xC] << 8));
                                }
                                else
                                {
                                    dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) (var_s1[-0xD] + (var_s1[-0xC] << 8));
                                }
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk134 = (u16) gte_out->vx;
                                slot->unk136 = (u16) gte_out->vy;
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) -(var_s1[-0x8] + (var_s1[-0x7] << 8));
                                }
                                else
                                {
                                    dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) (var_s1[-0x8] + (var_s1[-0x7] << 8));
                                }
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk138 = (u16) gte_out->vx;
                                slot->unk13A = (u16) gte_out->vy;
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) -(var_s1[-0x4] + (var_s1[-0x3] << 8));
                                }
                                else
                                {
                                    dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) (var_s1[-0x4] + (var_s1[-0x3] << 8));
                                }
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk13C = (u16) gte_out->vx;
                                slot->unk13E = (u16) gte_out->vy;
                            }
                            goto block_183;
                        }
                        break;
                    case 5:
                        if ((rec->unk34 == 0) && !(rec->unk3C & 0x01000000))
                        {
                            FrameResourceEntry *resources = FRAME_RESOURCE_ENTRIES;
                            u8 idx = rec->unk3B;
                            temp_v1_25 = (u16) resources[idx].unkA >> 0xC;
                            if (temp_v1_25 != 1)
                            {
                                if ((s32) temp_v1_25 < 2)
                                {
                                    if (temp_v1_25 == 0)
                                    {
                                        func_800A3938(resources[rec->unk3B].unkA & 0xFFF, field_get_actor_sound_pan(rec->unk3A));
                                    }
                                }
                            }
                            else
                            {
                                func_800A39A8(resources[rec->unk3B].unkA & 0xFFF, field_get_actor_sound_pan(rec->unk3A), rec->unk3B - 3, rec->unk3A);
                            }
                        }
                        break;
                    case 2:
                        if (part->unk24 & 0x100000)
                        {
                            field_unpack_effect_quad_corners16(sp60, rec->unk21 & 0x80, item);
                            sp74 += 1;
                        }
                        break;
                    case 4:
                        if ((slot->unk3C != 0xFFFF) && !(slot->unk174 & 0x1800))
                        {
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(*item + (var_s1[-0x10] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (*item + (var_s1[-0x10] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk130 = (u16) gte_out->vx;
                            slot->unk132 = (u16) (gte_out->vy - sp70);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0xD] + (var_s1[-0xC] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0xD] + (var_s1[-0xC] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk134 = (u16) gte_out->vx;
                            slot->unk136 = (u16) (gte_out->vy - sp70);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0x8] + (var_s1[-0x7] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0x8] + (var_s1[-0x7] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk138 = (u16) gte_out->vx;
                            slot->unk13A = (u16) (gte_out->vy - sp70);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0x4] + (var_s1[-0x3] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0x4] + (var_s1[-0x3] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk13C = (u16) gte_out->vx;
                            slot->unk13E = (u16) (gte_out->vy - sp70);
                            if (slot->unk3C & 0x8000)
                            {
                                if (field_start_bound_action_animation(rec->unk3A, 0, NULL, slot->unk3C) != 0)
                                {
                                    slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                                }
                            }
                            else
                            {
                                var_s0 = func_800839F8(rec->unk3A, 0);
                                if (var_s0 != -1)
                                {
                                    if (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0)
                                    {
                                        ((u8 *) &slot->unk178)[1] = var_s0;
                                        field_start_actor_animation(var_s0, 0, NULL);
                                    }
                                }
                            }
                            slot->unk3C = 0xFFFF;
                            slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                        }
                        break;
                    }
                }
                var_s1 += 0x11;
                item += 0x11;
            }
            sp5C -= 1;
        } while (sp5C != 0);
    }
    if (sp74 != 0)
    {
        s16 *p = sp60;
        while (p != sp60 + 8)
        {
            p[0] = ((p[0] * part->unk2E >> 6) * g_field_effect_track_scale.x) >> 12;
            p[1] = ((p[1] * part->unk33 >> 6) * g_field_effect_track_scale.z) >> 12;
            p += 2;
        }
        var_v0_9 = abs(sp60[2] - sp60[0]);
        slot->unk12E = (s16) ((var_v0_9 * 7) / 10);
        slot->unk12C = (s16) ((s32) (sp60[2] + sp60[0]) >> 1);
        cursor = field_render_actor_ground_shadow(rec, cursor, base, sp60);
    }
    return cursor;
}

#include "field_effect_primitives.h"
#include "field_effect_transform.h"
#include "field_effect_render_state.h"
#include "sdk/libgpu.h"
#include "sdk/rand.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define FIELD_EFFECT_OT_SIZE 4096
#define FIELD_EFFECT_OT_DEPTH_SHIFT 7
#define FIELD_EFFECT_CENTER_X 160
#define FIELD_EFFECT_CENTER_Y 112
#define FIELD_RING_SEGMENTS 32
#define FIELD_RING_ANGLE_STEP (ONE / FIELD_RING_SEGMENTS)
#define FIELD_RADIAL_MAX_SEGMENTS 20
#define FIELD_PART_ORIENTED_MARKER_SHIFT 3
#define FIELD_GPU_ADDRESS_MASK 0x00FFFFFF
#define FIELD_GPU_LENGTH_MASK 0xFF000000
#define FIELD_RADIAL_SCRATCH ((FieldRadialScratch*)0x1F800000)

/** @brief GPU coordinates accessed individually or as a packed XY word. */
typedef union
{
    s32 word;
    struct
    {
        s16 x;
        s16 y;
    } signed_pair;
    struct
    {
        u16 x;
        u16 y;
    } unsigned_pair;
} FieldScreenPoint;

/** @brief Unsigned projected origin used by ring geometry. */
typedef struct
{
    u16 x;
    u16 y;
} FieldScreenPair;

/** @brief Gouraud triangle packet with packed color and coordinate access. */
typedef struct
{
    s32 tag;
    FieldPrimitiveColor color0;
    FieldScreenPoint xy0;
    FieldPrimitiveColor color1;
    FieldScreenPoint xy1;
    FieldPrimitiveColor color2;
    FieldScreenPoint xy2;
} FieldGouraudTriangle;

/** @brief Gouraud line packet with packed color and coordinate access. */
typedef struct
{
    s32 tag;
    FieldPrimitiveColor color0;
    FieldScreenPoint xy0;
    FieldPrimitiveColor color1;
    FieldScreenPoint xy1;
} FieldGouraudLine;

/** @brief Flat quad packet with paired signed and unsigned coordinate views. */
typedef struct
{
    s32 tag;
    FieldPrimitiveColor color0;
    FieldScreenPoint xy0;
    FieldScreenPoint xy1;
    FieldScreenPoint xy2;
    FieldScreenPoint xy3;
} FieldFlatQuad;

/** @brief Flat line packet with packed color and coordinate access. */
typedef struct
{
    s32 tag;
    FieldPrimitiveColor color0;
    FieldScreenPoint xy0;
    FieldScreenPoint xy1;
} FieldFlatLine;

/** @brief Shared scratchpad workspace for radial projection and random rotations. */
typedef struct
{
    VECTOR origin;
    VECTOR transformed;
    union
    {
        VECTOR vector;
        Vec2s screen;
    } center;
    VECTOR delta;
    VECTOR jitter;
    SVECTOR direction;
    MATRIX matrices[FIELD_RADIAL_MAX_SEGMENTS];
} FieldRadialScratch;

extern FieldActorState g_field_actor_slots[80];
extern FieldMotionRecord g_field_effect_records[FIELD_EFFECT_ACTIVE_RECORD_COUNT];

/**
 * @brief Emit a 32-segment shaded ring, using three Gouraud triangles per segment.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Packet cursor after the geometry and texture-page command.
 * @see decomp.me (100%)
 */
u8* field_render_effect_ring(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    FieldScreenPair screen_origin;
    SVECTOR direction;
    VECTOR transformed;
    MATRIX matrix;
    FieldActorPartDef* part;
    SVECTOR* direction_ptr;
    VECTOR* transformed_ptr;
    s32 next_angle;
    s16 rim_y;
    s16 inner_start_x;
    s16 inner_start_y;
    s16 inner_end_x;
    s16 outer_start_x;
    s16 outer_y;
    FieldGouraudTriangle* next_packet;
    FieldActorState* actor;
    s32 scaled_cosine;
    s32 address_mask;
    s32 length_mask;
    s32 outer_next_angle;
    s32 packed_color;
    s32 second_depth;
    s32 third_depth;
    s32 packet_tag;
    s32 first_depth;
    s32 camera_x;
    s32 inner_angle;
    s32 outer_angle;
    s32 segment;
    s32 next_scaled_cosine;
    s32 angle;
    s32 outer_cosine;
    s32 radius;
    FieldGouraudTriangle* second_triangle;
    FieldGouraudTriangle* packets;

    packets = (FieldGouraudTriangle*)packet_cursor;
    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];
    field_build_effect_part_matrix(effect, part, &matrix, actor);
    gte_SetRotMatrix(&matrix);

    camera_x = g_field_view_offset_x / 256;
    screen_origin.x = (u16)(camera_x + (effect->x / 256 + FIELD_EFFECT_CENTER_X));
    screen_origin.y = (u16)(FIELD_EFFECT_CENTER_Y + g_field_view_offset_y / 256 + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512);
    field_resolve_effect_part_color(actor, effect, part, &packets->color0);
    setPolyG3(packets);
    setSemiTrans(packets, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
    segment = 0;
    /* This render mode reuses animation_active as the inner-radius scale. */
    radius = effect->animation_active;
    direction_ptr = &direction;
    transformed_ptr = &transformed;
    address_mask = FIELD_GPU_ADDRESS_MASK;
    length_mask = FIELD_GPU_LENGTH_MASK;
    angle = segment;
    do
    {
        next_angle = (segment + 1) << 7;
        packets[0].xy1.unsigned_pair.x = screen_origin.x;
        packets[0].xy1.unsigned_pair.y = screen_origin.y;
        scaled_cosine = (rcos(angle) >> 6) * radius;
        direction.vy = 0;
        direction.vx = (s16)(scaled_cosine >> 8);
        direction.vz = (s16)((s32)((rsin(angle) >> 6) * radius) >> 8);
        gte_ldv0(direction_ptr);
        gte_rtv0();
        gte_stlvnl(transformed_ptr);
        inner_start_x = screen_origin.x + (u16)transformed.vx;
        packets[0].xy0.signed_pair.x = inner_start_x;
        packets[1].xy0.signed_pair.x = inner_start_x;
        inner_start_y = screen_origin.y + (u16)transformed.vy;
        packets[0].xy0.signed_pair.y = inner_start_y;
        packets[1].xy0.signed_pair.y = inner_start_y;
        if (segment == (FIELD_RING_SEGMENTS - 1))
        {
            next_scaled_cosine = (rcos(0) >> 6) * radius;
            inner_angle = 0;
            direction.vy = 0;
            direction.vx = (s16)(next_scaled_cosine >> 8);
            direction.vz = (s16)((s32)((rsin(inner_angle) >> 6) * radius) >> 8);
        }
        else
        {
            next_scaled_cosine = (rcos(next_angle) >> 6) * radius;
            inner_angle = next_angle;
            direction.vy = 0;
            direction.vx = (s16)(next_scaled_cosine >> 8);
            direction.vz = (s16)((s32)((rsin(inner_angle) >> 6) * radius) >> 8);
        }
        gte_ldv0(direction_ptr);
        gte_rtv0();
        gte_stlvnl(transformed_ptr);
        second_triangle = &packets[1];
        inner_end_x = screen_origin.x + (u16)transformed.vx;
        packets[0].xy2.signed_pair.x = inner_end_x;
        second_triangle[0].xy2.signed_pair.x = inner_end_x;
        packets[2].xy0.signed_pair.x = inner_end_x;
        rim_y = screen_origin.y + (u16)transformed.vy;
        packets[0].xy2.signed_pair.y = rim_y;
        second_triangle[0].xy2.signed_pair.y = rim_y;
        packets[2].xy0.signed_pair.y = rim_y;
        direction.vx = (s16)(rcos(angle) >> 6);
        direction.vy = 0;
        direction.vz = (s16)(rsin(angle) >> 6);
        gte_ldv0(direction_ptr);
        gte_rtv0();
        gte_stlvnl(transformed_ptr);
        outer_start_x = screen_origin.x + (u16)transformed.vx;
        second_triangle[0].xy1.signed_pair.x = outer_start_x;
        packets[2].xy1.signed_pair.x = outer_start_x;
        outer_y = screen_origin.y + (u16)transformed.vy;
        second_triangle[0].xy1.signed_pair.y = outer_y;
        packets[2].xy1.signed_pair.y = outer_y;
        if (segment == (FIELD_RING_SEGMENTS - 1))
        {
            outer_cosine = rcos(0);
            outer_angle = 0;
            direction.vx = (s16)(outer_cosine >> 6);
            direction.vy = 0;
            direction.vz = (s16)(rsin(outer_angle) >> 6);
        }
        else
        {
            outer_next_angle = angle + FIELD_RING_ANGLE_STEP;
            outer_cosine = rcos(outer_next_angle);
            outer_angle = outer_next_angle;
            direction.vx = (s16)(outer_cosine >> 6);
            direction.vy = 0;
            direction.vz = (s16)(rsin(outer_angle) >> 6);
        }
        gte_ldv0(direction_ptr);
        gte_rtv0();
        gte_stlvnl(transformed_ptr);
        outer_y = (s16)(screen_origin.x + (u16)transformed.vx);
        packets[2].xy2.signed_pair.x = outer_y;
        packed_color = packets[0].color0.signed_word;
        rim_y = (s16)(screen_origin.y + (u16)transformed.vy);
        packet_tag = packets[0].tag;
        packets[0].color1.signed_word = 0;
        packets[2].color2.signed_word = 0;
        packets[2].color1.signed_word = 0;
        packets[1].color1.signed_word = 0;
        packets[2].color0.signed_word = packed_color;
        packets[1].color2.signed_word = packed_color;
        packets[1].color0.signed_word = packed_color;
        packets[0].color2.signed_word = packed_color;
        packets[1].tag = packet_tag;
        packets[2].tag = packet_tag;
        packets[3].tag = packet_tag;
        packets[3].color0.signed_word = packets[0].color0.signed_word;
        packets[2].xy2.signed_pair.y = rim_y;
        first_depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
        next_packet = &packets[1];
        if (first_depth < 0)
        {
            packets->tag = (packets->tag & length_mask) | (ordering_table[0] & address_mask);
            ordering_table[0] = (ordering_table[0] & length_mask) | ((s32)packets & address_mask);
            packets = next_packet;
        }
        else if (first_depth >= FIELD_EFFECT_OT_SIZE)
        {
            packets->tag = (packets->tag & length_mask) | (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & address_mask);
            ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] = (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & length_mask) | ((s32)packets & address_mask);
            packets = next_packet;
        }
        else
        {
            packets->tag = (packets->tag & length_mask) | (ordering_table[effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT] & address_mask);
            ordering_table[effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT] =
                (ordering_table[effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT] & length_mask) | ((s32)packets & address_mask);
            packets = next_packet;
        }
        second_depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
        if (second_depth < 0)
        {
            packets->tag = (packets->tag & length_mask) | (ordering_table[0] & address_mask);
            ordering_table[0] = (ordering_table[0] & length_mask) | ((s32)packets & address_mask);
            packets++;
        }
        else if (second_depth >= FIELD_EFFECT_OT_SIZE)
        {
            packets->tag = (packets->tag & length_mask) | (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & address_mask);
            ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] = (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & length_mask) | ((s32)packets & address_mask);
            packets++;
        }
        else
        {
            packets->tag = (packets->tag & length_mask) | (ordering_table[effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT] & address_mask);
            ordering_table[effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT] =
                (ordering_table[effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT] & length_mask) | ((s32)packets & address_mask);
            packets++;
        }
        third_depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
        if (third_depth < 0)
        {
            packets->tag = (packets->tag & length_mask) | (ordering_table[0] & address_mask);
            ordering_table[0] = (ordering_table[0] & length_mask) | ((s32)packets & address_mask);
            packets++;
        }
        else if (third_depth >= FIELD_EFFECT_OT_SIZE)
        {
            packets->tag = (packets->tag & length_mask) | (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & address_mask);
            ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] = (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & length_mask) | ((s32)packets & address_mask);
            packets++;
        }
        else
        {
            packets->tag = (packets->tag & length_mask) | (ordering_table[effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT] & address_mask);
            ordering_table[effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT] =
                (ordering_table[effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT] & length_mask) | ((s32)packets & address_mask);
            packets++;
        }
        angle += FIELD_RING_ANGLE_STEP;
        if (segment == (FIELD_RING_SEGMENTS - 1))
        {
            break;
        }
        segment += 1;
    } while (1);
    return field_emit_effect_texture_page(effect, part, (u8*)packets, ordering_table);
}

/**
 * @brief Emit a shaded circular fan with two to 32 segments and four triangles per segment.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Packet cursor after the geometry and texture-page command.
 * @see decomp.me (100%)
 */
u8* field_render_effect_fan(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    SVECTOR screen_origin;
    SVECTOR direction;
    VECTOR transformed;
    MATRIX matrix;
    FieldActorPartDef* part;
    s32 first_half_angle;
    SVECTOR* direction_ptr;
    VECTOR* transformed_ptr;
    s32 half_angle;
    s32 angle;
    s32 next_angle;
    s16 inner_end_y;
    s16 middle_end_x;
    s16 inner_end_x;
    FieldGouraudTriangle* next_packet;
    FieldActorState* actor;
    FieldActorState* slots;
    s32* first_entry;
    s32* second_entry;
    s32* third_entry;
    s32* fourth_entry;
    s32 half_step;
    s32 next_color;
    s32 angle_quotient;
    s16 angle_short;
    s32 middle_angle;
    s32 outer_next_angle;
    s32 packed_color;
    s32 ot_word;
    s32 second_depth;
    s32 third_depth;
    s32 fourth_depth;
    s32 packet_tag;
    s32 depth_or_y;
    s32 outer_angle;
    s32 angle_step;
    s32 segment;
    s32 inner_cosine;
    s32 middle_cosine;
    s32 outer_cosine;
    u16 center_y;
    u16 offset_y;
    s32 segment_count_delta;
    s32 inner_last_segment;
    s32 middle_last_segment;
    s32 outer_last_segment;
    s32 mask_low;
    s32 mask_high;
    u8 segment_count;
    FieldGouraudTriangle* third_triangle;
    FieldGouraudTriangle* fourth_triangle;
    FieldGouraudTriangle* second_triangle;
    FieldGouraudTriangle* packets;
    FieldActorPartDef* selected_part;

    packets = (FieldGouraudTriangle*)packet_cursor;
    slots = g_field_actor_slots;
    actor = &slots[effect->actor_index];
    selected_part = &actor->parts[effect->part_index];
    part = selected_part;
    field_build_effect_part_matrix(effect, part, &matrix, actor);
    gte_SetRotMatrix(&matrix);

    screen_origin.vx = (u16)((g_field_view_offset_x / 256) + ((effect->x / 256) + FIELD_EFFECT_CENTER_X));
    screen_origin.vy =
        (u16)(((((g_field_view_offset_y / 256) + FIELD_EFFECT_CENTER_Y) + (effect->y / 256)) - (effect->z / 512)) - (g_field_view_offset_z / 512));
    field_resolve_effect_part_color(actor, effect, part, &packets->color0);
    setPolyG3(packets);
    setSemiTrans(packets, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
    segment_count = 2;
    /* This part byte selects the fan segment count; other uses are unresolved. */
    segment_count_delta = part->unknown_0x8 - 2;
    if ((u32)segment_count_delta < (FIELD_RING_SEGMENTS - 1U))
    {
        segment_count = *(volatile u8*)&part->unknown_0x8;
    }
    angle_quotient = ONE / (s32)segment_count;
    angle_short = angle_quotient;
    angle_step = angle_short;
    if ((ONE % (s32)segment_count) != 0)
    {
        angle_step += 1;
    }
    segment = 0;
    direction_ptr = &direction;
    transformed_ptr = &transformed;
    mask_low = FIELD_GPU_ADDRESS_MASK;
    mask_high = FIELD_GPU_LENGTH_MASK;
    half_step = angle_quotient >> 1;
    first_half_angle = half_step;
    angle_short = half_step;
    half_angle = angle_short;
    angle = 0;
    next_angle = angle_step;
next_segment:
{
    /* Copy the two screen coordinates as one GPU word. */
    packets[0].xy1.word = *(s32*)&screen_origin;
    direction.vx = (s16)(rcos(angle) >> 8);
    direction.vy = 0;
    direction.vz = (s16)(rsin(angle) >> 8);
    gte_ldv0(direction_ptr);
    gte_rtv0();
    gte_stlvnl(transformed_ptr);
    {
        u32 coordinate;
        u16 offset;
        coordinate = (u16)screen_origin.vx;
        offset = (u16)transformed.vx;
        coordinate += offset;
        packets[0].xy0.signed_pair.x = coordinate;
        packets[1].xy0.signed_pair.x = coordinate;
        coordinate = (u16)screen_origin.vy;
        offset = (u16)transformed.vy;
        coordinate += offset;
        inner_last_segment = segment_count - 1;
        packets[0].xy0.signed_pair.y = coordinate;
        packets[1].xy0.signed_pair.y = coordinate;
    }
    if (segment == inner_last_segment)
    {
        inner_cosine = rcos(0);
        direction.vx = (s16)(inner_cosine >> 8);
        direction.vy = 0;
        direction.vz = (s16)(rsin(0) >> 8);
    }
    else
    {
        inner_cosine = rcos(next_angle);
        direction.vx = (s16)(inner_cosine >> 8);
        direction.vy = 0;
        direction.vz = (s16)(rsin(next_angle) >> 8);
    }
    gte_ldv0(direction_ptr);
    gte_rtv0();
    gte_stlvnl(transformed_ptr);
    second_triangle = &packets[1];
    fourth_triangle = &packets[3];
    inner_end_x = screen_origin.vx + (u16)transformed.vx;
    packets[0].xy2.signed_pair.x = inner_end_x;
    second_triangle[0].xy2.signed_pair.x = inner_end_x;
    fourth_triangle[0].xy0.signed_pair.x = inner_end_x;
    packets[2].xy0.signed_pair.x = inner_end_x;
    inner_end_y = screen_origin.vy + (u16)transformed.vy;
    packets[0].xy2.signed_pair.y = inner_end_y;
    second_triangle[0].xy2.signed_pair.y = inner_end_y;
    fourth_triangle[0].xy0.signed_pair.y = inner_end_y;
    packets[2].xy0.signed_pair.y = inner_end_y;
    direction.vx = (s16)(rcos(half_angle) >> 6);
    direction.vy = 0;
    direction.vz = (s16)(rsin(half_angle) >> 6);
    gte_ldv0(direction_ptr);
    gte_rtv0();
    gte_stlvnl(transformed_ptr);
    {
        u32 coordinate;
        u16 offset;
        coordinate = (u16)screen_origin.vx;
        offset = (u16)transformed.vx;
        coordinate += offset;
        second_triangle[0].xy1.signed_pair.x = coordinate;
        packets[2].xy1.signed_pair.x = coordinate;
        coordinate = (u16)screen_origin.vy;
        offset = (u16)transformed.vy;
        coordinate += offset;
        middle_last_segment = segment_count - 1;
        second_triangle[0].xy1.signed_pair.y = coordinate;
        packets[2].xy1.signed_pair.y = coordinate;
    }
    if (segment == middle_last_segment)
    {
        middle_cosine = rcos(0);
        direction.vx = (s16)(middle_cosine >> 7);
        direction.vy = 0;
        direction.vz = (s16)(rsin(0) >> FIELD_EFFECT_OT_DEPTH_SHIFT);
    }
    else
    {
        middle_angle = angle + angle_step;
        middle_cosine = rcos(middle_angle);
        direction.vx = (s16)(middle_cosine >> 7);
        direction.vy = 0;
        direction.vz = (s16)(rsin(middle_angle) >> 7);
    }
    gte_ldv0(direction_ptr);
    gte_rtv0();
    gte_stlvnl(transformed_ptr);
    third_triangle = &packets[2];
    middle_end_x = screen_origin.vx + (u16)transformed.vx;
    third_triangle[0].xy2.signed_pair.x = middle_end_x;
    packets[3].xy1.signed_pair.x = middle_end_x;
    depth_or_y = (u16)screen_origin.vy + (u16)transformed.vy;
    outer_last_segment = segment_count - 1;
    third_triangle[0].xy2.signed_pair.y = depth_or_y;
    packets[3].xy1.signed_pair.y = depth_or_y;
    if (segment == outer_last_segment)
    {
        outer_cosine = rcos(first_half_angle);
        outer_angle = first_half_angle;
        direction.vx = (s16)(outer_cosine >> 6);
        direction.vy = 0;
        direction.vz = (s16)(rsin(outer_angle) >> 6);
    }
    else
    {
        outer_next_angle = half_angle + angle_step;
        outer_cosine = rcos(outer_next_angle);
        outer_angle = outer_next_angle;
        direction.vx = (s16)(outer_cosine >> 6);
        direction.vy = 0;
        direction.vz = (s16)(rsin(outer_angle) >> 6);
    }
    gte_ldv0(direction_ptr);
    gte_rtv0();
    gte_stlvnl(transformed_ptr);
    packets[3].xy2.signed_pair.x = (s16)(screen_origin.vx + (u16)transformed.vx);
    packed_color = packets[0].color0.signed_word;
    center_y = screen_origin.vy;
    offset_y = (u16)transformed.vy;
    packet_tag = packets[0].tag;
    packets[3].color2.signed_word = 0;
    packets[3].color1.signed_word = 0;
    packets[2].color2.signed_word = 0;
    packets[2].color1.signed_word = 0;
    packets[1].color1.signed_word = 0;
    next_color = packets[0].color0.signed_word;
    packets[3].color0.signed_word = packed_color;
    packets[2].color0.signed_word = packed_color;
    packets[0].color1.signed_word = packed_color;
    packets[1].color2.signed_word = packed_color;
    packets[1].color0.signed_word = packed_color;
    packets[0].color2.signed_word = packed_color;
    packets[1].tag = packet_tag;
    packets[2].tag = packet_tag;
    packets[3].tag = packet_tag;
    packets[4].tag = packet_tag;
    packets[4].color0.signed_word = next_color;
    packets[3].xy2.signed_pair.y = (s16)(center_y + offset_y);
    depth_or_y = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
    next_packet = &packets[1];
    if (depth_or_y < 0)
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[0] & mask_low);
        ordering_table[0] = (ordering_table[0] & mask_high) | ((s32)packets & mask_low);
        packets = next_packet;
    }
    else if (depth_or_y >= FIELD_EFFECT_OT_SIZE)
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & mask_low);
        ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] = (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & mask_high) | ((s32)packets & mask_low);
        packets = next_packet;
    }
    else
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[depth_or_y] & mask_low);
        first_entry = (s32*)((effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
        ot_word = *first_entry;
        *first_entry = (ot_word & mask_high) | ((s32)packets & mask_low);
        packets = next_packet;
    }

    second_depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
    if (second_depth < 0)
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[0] & mask_low);
        ordering_table[0] = (ordering_table[0] & mask_high) | ((s32)packets & mask_low);
        packets++;
    }
    else if (second_depth >= FIELD_EFFECT_OT_SIZE)
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & mask_low);
        ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] = (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & mask_high) | ((s32)packets & mask_low);
        packets++;
    }
    else
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[second_depth] & mask_low);
        second_entry = (s32*)((effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
        ot_word = *second_entry;
        *second_entry = (ot_word & mask_high) | ((s32)packets & mask_low);
        packets++;
    }

    third_depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
    if (third_depth < 0)
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[0] & mask_low);
        ordering_table[0] = (ordering_table[0] & mask_high) | ((s32)packets & mask_low);
        packets++;
    }
    else if (third_depth >= FIELD_EFFECT_OT_SIZE)
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & mask_low);
        ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] = (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & mask_high) | ((s32)packets & mask_low);
        packets++;
    }
    else
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[third_depth] & mask_low);
        third_entry = (s32*)((effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
        ot_word = *third_entry;
        *third_entry = (ot_word & mask_high) | ((s32)packets & mask_low);
        packets++;
    }

    fourth_depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
    if (fourth_depth < 0)
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[0] & mask_low);
        ordering_table[0] = (ordering_table[0] & mask_high) | ((s32)packets & mask_low);
        packets++;
    }
    else if (fourth_depth >= FIELD_EFFECT_OT_SIZE)
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & mask_low);
        ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] = (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & mask_high) | ((s32)packets & mask_low);
        packets++;
    }
    else
    {
        packets[0].tag = (packets[0].tag & mask_high) | (ordering_table[fourth_depth] & mask_low);
        fourth_entry = (s32*)((effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
        ot_word = *fourth_entry;
        *fourth_entry = (ot_word & mask_high) | ((s32)packets & mask_low);
        packets++;
    }
    if (segment == (segment_count - 1))
    {
        goto finished;
    }
    segment += 1;
    half_angle += angle_step;
    angle += angle_step;
    next_angle += angle_step;
    goto next_segment;
}
finished:
    return field_emit_effect_texture_page(effect, part, (u8*)packets, ordering_table);
}

/**
 * @brief Emit a fading line from an effect to its target or a rotated local offset.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Packet cursor after the geometry and texture-page command.
 * @see decomp.me (100%)
 */
u8* field_render_effect_marker(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    SVECTOR screen_origin;
    SVECTOR direction;
    VECTOR transformed;
    VECTOR target_position;
    MATRIX matrix;
    FieldActorState* actor;
    FieldActorPartDef* part;

    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];
    field_build_effect_part_matrix(effect, part, &matrix, actor);
    gte_SetRotMatrix(&matrix);

    screen_origin.vx = (s16)(FIELD_EFFECT_CENTER_X + g_field_view_offset_x / 256 + effect->x / 256);
    screen_origin.vy = (s16)(FIELD_EFFECT_CENTER_Y + g_field_view_offset_y / 256 + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512);

    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&((P_TAG*)packet_cursor)->r0);
    setLineG2((LINE_G2*)packet_cursor);
    setSemiTrans((LINE_G2*)packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
    ((FieldGouraudLine*)packet_cursor)[0].color1.signed_word = 0;

    if (effect->color_position.fields.position_source != 0)
    {
        ((FieldGouraudLine*)packet_cursor)[0].xy0.word = *(s32*)&screen_origin;
        field_resolve_effect_position(effect, part, &target_position);
        screen_origin.vx = (s16)(FIELD_EFFECT_CENTER_X + g_field_view_offset_x / 256 + target_position.vx / 256);
        screen_origin.vy =
            (s16)(FIELD_EFFECT_CENTER_Y + g_field_view_offset_y / 256 + target_position.vy / 256 - target_position.vz / 512 - g_field_view_offset_z / 512);
        ((FieldGouraudLine*)packet_cursor)[0].xy1.word = *(s32*)&screen_origin;
    }
    else if ((part->behavior_flags.word >> FIELD_PART_ORIENTED_MARKER_SHIFT) & 1)
    {
        direction.vx = -30;
        direction.vy = 0;
        direction.vz = 0;
        gte_ldv0(&direction);
        gte_rtv0();
        gte_stlvnl(&transformed);
        ((FieldGouraudLine*)packet_cursor)[0].xy0.signed_pair.x = screen_origin.vx + (u16)transformed.vx;
        {
            s16 y = screen_origin.vy + (u16)transformed.vy;
            ((FieldGouraudLine*)packet_cursor)[0].xy1.word = *(s32*)&screen_origin;
            ((FieldGouraudLine*)packet_cursor)[0].xy0.signed_pair.y = y;
        }
    }
    else
    {
        ((FieldGouraudLine*)packet_cursor)[0].xy1.word = *(s32*)&screen_origin;
        ((FieldGouraudLine*)packet_cursor)[0].xy0.word = *(s32*)&screen_origin;
    }

    {
        s32 index;
        index = effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
        if (index < 0)
        {
            addPrim(&ordering_table[0], packet_cursor);
            packet_cursor += sizeof(LINE_G2);
        }
        else if (index >= FIELD_EFFECT_OT_SIZE)
        {
            addPrim(&ordering_table[(FIELD_EFFECT_OT_SIZE - 1)], packet_cursor);
            packet_cursor += sizeof(LINE_G2);
        }
        else
        {
            {
                s32 ot_word = ordering_table[index];
                setaddr(packet_cursor, ot_word);
            }
            setaddr(&ordering_table[effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT], packet_cursor);
            packet_cursor += sizeof(LINE_G2);
        }
    }
    return field_emit_effect_texture_page(effect, part, packet_cursor, ordering_table);
}

/**
 * @brief Join an effect to its live predecessor with a flat-shaded quad.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Advanced packet cursor; unchanged for a trail without a live predecessor.
 * @see decomp.me (100%)
 */
u8* field_render_effect_trail(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    FieldActorState* actor;
    FieldActorPartDef* part;
    s32 depth;
    s32 camera_x;
    s32 effect_x;
    s32 camera_y;

    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];

    if (effect->previous_effect_index != FIELD_EFFECT_RETIRED && g_field_effect_records[effect->previous_effect_index].state != FIELD_EFFECT_RETIRED)
    {
        camera_x = g_field_view_offset_x / 256;
        effect_x = effect->x / 256 + FIELD_EFFECT_CENTER_X;
        camera_y = g_field_view_offset_y;
        ((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.x = (u16)(camera_x + effect_x);

        ((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.y =
            (u16)(FIELD_EFFECT_CENTER_Y + camera_y / 256 + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512);

        ((FieldFlatQuad*)packet_cursor)[0].xy2.word = ((FieldFlatQuad*)packet_cursor)[0].xy0.word;
        ((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.x = (u16)(((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.x - (u16)effect->work_x);
        ((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.y = (u16)(((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.y - (u16)effect->work_y);
        ((FieldFlatQuad*)packet_cursor)[0].xy2.unsigned_pair.x = (u16)(((FieldFlatQuad*)packet_cursor)[0].xy2.unsigned_pair.x + (u16)effect->work_x);
        ((FieldFlatQuad*)packet_cursor)[0].xy2.unsigned_pair.y = (u16)(((FieldFlatQuad*)packet_cursor)[0].xy2.unsigned_pair.y + (u16)effect->work_y);

        ((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.x =
            (u16)(g_field_view_offset_x / 256 + (g_field_effect_records[effect->previous_effect_index].x / 256 + FIELD_EFFECT_CENTER_X));

        ((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.y =
            (u16)(FIELD_EFFECT_CENTER_Y + camera_y / 256 + g_field_effect_records[effect->previous_effect_index].y / 256 -
                  g_field_effect_records[effect->previous_effect_index].z / 512 - g_field_view_offset_z / 512);

        ((FieldFlatQuad*)packet_cursor)[0].xy3.word = ((FieldFlatQuad*)packet_cursor)[0].xy1.word;
        ((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.x =
            (u16)(((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.x - (u16)g_field_effect_records[effect->previous_effect_index].work_x);
        ((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.y =
            (u16)(((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.y - (u16)g_field_effect_records[effect->previous_effect_index].work_y);
        ((FieldFlatQuad*)packet_cursor)[0].xy3.unsigned_pair.x =
            (u16)(((FieldFlatQuad*)packet_cursor)[0].xy3.unsigned_pair.x + (u16)g_field_effect_records[effect->previous_effect_index].work_x);
        ((FieldFlatQuad*)packet_cursor)[0].xy3.unsigned_pair.y =
            (u16)(((FieldFlatQuad*)packet_cursor)[0].xy3.unsigned_pair.y + (u16)g_field_effect_records[effect->previous_effect_index].work_y);

        field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&((P_TAG*)packet_cursor)->r0);

        setPolyF4((POLY_F4*)packet_cursor);
        setSemiTrans((POLY_F4*)packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

        depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
        if (depth < 0)
        {
            addPrim(&ordering_table[0], (POLY_F4*)packet_cursor);
            packet_cursor += sizeof(POLY_F4);
        }
        else if (depth >= FIELD_EFFECT_OT_SIZE)
        {
            addPrim(&ordering_table[(FIELD_EFFECT_OT_SIZE - 1)], (POLY_F4*)packet_cursor);
            packet_cursor += sizeof(POLY_F4);
        }
        else
        {
            addPrim(&ordering_table[(s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT], (POLY_F4*)packet_cursor);
            packet_cursor += sizeof(POLY_F4);
        }

        packet_cursor = field_emit_effect_texture_page(effect, part, packet_cursor, ordering_table);
    }

    return packet_cursor;
}

/**
 * @brief Emit a closed line loop with alternating inner and outer radii.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Packet cursor after the geometry and texture-page command.
 * @see decomp.me (100%)
 */
u8* field_render_effect_radial_fan(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    MATRIX* matrix;
    SVECTOR* direction;
    FieldActorPartDef* part;
    VECTOR* transformed;
    s32 radius;
    Vec2s* screen_origin;
    FieldActorState* actor;
    FieldFlatLine* next_line;
    s32 angle;
    s32 segment_count;
    s32 first_endpoint;
    s32 segment;
    s32 screen_x;
    s32 camera_y;
    s32 depth;

    transformed = &FIELD_RADIAL_SCRATCH->transformed;
    screen_origin = &FIELD_RADIAL_SCRATCH->center.screen;
    direction = &FIELD_RADIAL_SCRATCH->direction;
    matrix = FIELD_RADIAL_SCRATCH->matrices;

    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];

    screen_origin->x = (s16)(FIELD_EFFECT_CENTER_X + g_field_view_offset_x / 256 + effect->x / 256);
    screen_origin->y = (s16)(FIELD_EFFECT_CENTER_Y + g_field_view_offset_y / 256 + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512);

    field_build_effect_part_matrix(effect, part, matrix, actor);
    gte_SetRotMatrix(matrix);

    screen_x = FIELD_EFFECT_CENTER_X + g_field_view_offset_x / 256 + effect->x / 256;
    camera_y = g_field_view_offset_y;
    ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.x = (s16)screen_x;
    if (camera_y < 0)
    {
        camera_y += 255;
    }
    ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.y =
        (s16)(FIELD_EFFECT_CENTER_Y + (camera_y >> 8) + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512);

    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&((P_TAG*)packet_cursor)->r0);

    /* Line effects reuse animation_active as their segment count. */
    segment_count = 1;
    if (effect->animation_active != 0)
    {
        segment_count = effect->animation_active;
    }

    /* This mode interprets the shared selector byte as an outer-radius scale. */
    radius = (u32)((part->rotation_extent.fields.unknown_0x23 + 1) * 5) >> 4;

    direction->vx = (s16)((u32)(rsin(0) * 5) >> 8);
    direction->vy = 0;
    direction->vz = (s16)((s32)(rcos(0) * 80) >> 12);

    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(transformed);

    ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.x = (s16)(screen_origin->x + *(s16*)&transformed->vx);
    ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.y = (s16)(screen_origin->y + *(s16*)&transformed->vy);
    first_endpoint = ((FieldFlatLine*)packet_cursor)[0].xy0.word;

    segment = 1;
    if (segment < segment_count)
    {
        do
        {
            next_line = &((FieldFlatLine*)packet_cursor)[1];
            do
            {
                angle = segment << 12;
            } while (0);
            if (segment & 1)
            {
                angle /= segment_count;
                direction->vx = (s16)((rsin(angle) * radius) >> 12);
                direction->vy = 0;
                direction->vz = (s16)((rcos(angle) * radius) >> 12);
            }
            else
            {
                angle /= segment_count;
                direction->vx = (s16)((u32)(rsin(angle) * 5) >> 8);
                direction->vy = 0;
                direction->vz = (s16)((s32)(rcos(angle) * 80) >> 12);
            }

            setLineF2(next_line - 1);
            setSemiTrans(next_line - 1, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

            gte_ldv0(direction);
            gte_rtv0();
            gte_stlvnl(transformed);

            next_line[-1].xy1.signed_pair.x = (s16)(screen_origin->x + *(s16*)&transformed->vx);
            next_line[-1].xy1.signed_pair.y = (s16)(screen_origin->y + *(s16*)&transformed->vy);
            ((FieldFlatLine*)packet_cursor)[1].xy0.word = ((FieldFlatLine*)packet_cursor)[0].xy1.word;
            ((FieldFlatLine*)packet_cursor)[1].color0.signed_word = ((FieldFlatLine*)packet_cursor)[0].color0.signed_word;

            depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
            if (depth < 0)
            {
                s32 packet_address;
                packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
                setaddr(packet_cursor, getaddr(&ordering_table[0]));
                packet_cursor += sizeof(LINE_F2);
                ordering_table[0] = (ordering_table[0] & FIELD_GPU_LENGTH_MASK) | packet_address;
            }
            else if (depth >= FIELD_EFFECT_OT_SIZE)
            {
                s32 packet_address;
                packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
                setaddr(packet_cursor, getaddr(&ordering_table[(FIELD_EFFECT_OT_SIZE - 1)]));
                packet_cursor += sizeof(LINE_F2);
                ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] = (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & FIELD_GPU_LENGTH_MASK) | packet_address;
            }
            else
            {
                s32 packet_address;
                s32* entry;
                packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
                setaddr(packet_cursor, getaddr(&ordering_table[depth]));
                entry = (s32*)((effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
                packet_cursor += sizeof(LINE_F2);
                *entry = (*entry & FIELD_GPU_LENGTH_MASK) | packet_address;
            }
            segment++;
        } while (segment < segment_count);
    }

    setLineF2(packet_cursor);
    setSemiTrans(packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
    ((FieldFlatLine*)packet_cursor)[0].xy1.word = first_endpoint;

    depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
    if (depth < 0)
    {
        s32 packet_address;
        packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
        setaddr(packet_cursor, getaddr(&ordering_table[0]));
        packet_cursor += sizeof(LINE_F2);
        ordering_table[0] = (ordering_table[0] & FIELD_GPU_LENGTH_MASK) | packet_address;
    }
    else if (depth >= FIELD_EFFECT_OT_SIZE)
    {
        s32 packet_address;
        packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
        setaddr(packet_cursor, getaddr(&ordering_table[(FIELD_EFFECT_OT_SIZE - 1)]));
        packet_cursor += sizeof(LINE_F2);
        ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] = (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & FIELD_GPU_LENGTH_MASK) | packet_address;
    }
    else
    {
        s32 packet_address;
        s32* entry;
        s32 ot_word;
        packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
        ot_word = ordering_table[depth];
        setaddr(packet_cursor, ot_word);
        entry = (s32*)((effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
        packet_cursor += sizeof(LINE_F2);
        *entry = (*entry & FIELD_GPU_LENGTH_MASK) | packet_address;
    }

    packet_cursor = field_emit_effect_texture_page(effect, part, packet_cursor, ordering_table);

    return packet_cursor;
}

/**
 * @brief Emit a jittered line chain between an effect and its resolved target.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Packet cursor after the geometry and texture-page command.
 * @see decomp.me (100%)
 * @see decomp.me (99.95%) Previous WIP
 */
u8* field_render_effect_radial_lines(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    FieldActorPartDef* part;
    FieldActorState* actor;
    VECTOR* position;
    VECTOR* center;
    VECTOR* delta;
    VECTOR* jitter;
    SVECTOR* direction;
    MATRIX* matrix;
    FieldFlatLine* next_line;
    s32 segment_count;
    s32 segment;
    s32 arc_height;
    s32 angle_step;
    s32 depth;
    VECTOR* origin;

    origin = &FIELD_RADIAL_SCRATCH->origin;
    position = &FIELD_RADIAL_SCRATCH->transformed;
    center = &FIELD_RADIAL_SCRATCH->center.vector;
    delta = &FIELD_RADIAL_SCRATCH->delta;
    jitter = &FIELD_RADIAL_SCRATCH->jitter;
    direction = &FIELD_RADIAL_SCRATCH->direction;

    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];

    matrix = FIELD_RADIAL_SCRATCH->matrices;
    field_build_effect_part_matrix(effect, part, matrix, actor);
    gte_SetRotMatrix(matrix);

    {
        s32 camera_x;
        s32 position_x;
        s32 camera_y;
        camera_x = g_field_view_offset_x / 256;
        position_x = effect->x / 256;
        camera_y = g_field_view_offset_y;
        ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.x = camera_x + (s16)(position_x + FIELD_EFFECT_CENTER_X);
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.y =
            FIELD_EFFECT_CENTER_Y + (camera_y >> 8) + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512;
    }

    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&((P_TAG*)packet_cursor)->r0);

    setLineF2(packet_cursor);
    setSemiTrans(packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

    matrix = FIELD_RADIAL_SCRATCH->matrices;

    /* Line effects reuse animation_active as their segment count. */
    segment_count = FIELD_RADIAL_MAX_SEGMENTS;
    if (effect->animation_active < FIELD_RADIAL_MAX_SEGMENTS)
    {
        segment_count = effect->animation_active;
    }
    if (segment_count <= 0)
    {
        segment_count = 1;
    }
    angle_step = (ONE / 2) / segment_count;

    field_resolve_effect_position(effect, part, origin);

    segment = segment_count - 1;

    center->vx = (origin->vx + effect->x) >> 1;
    center->vy = origin->vy;
    center->vz = (origin->vz + effect->z) >> 1;
    delta->vx = (origin->vx - effect->x) >> 1;
    delta->vy = effect->y - origin->vy;
    delta->vz = (origin->vz - effect->z) >> 1;

    if (segment > 0)
    {
        do
        {
            direction->vx = 0;
            direction->vy = (s16)((rand() << 12) >> 15);
            direction->vz = (s16)((rand() << 12) >> 16);
            RotMatrix_gte(direction, matrix);
            segment--;
            matrix++;
        } while (segment > 0);
    }

    matrix = FIELD_RADIAL_SCRATCH->matrices;

    if (part->behavior_flags.bytes.low >> 7)
    {
        direction->vx = 0;
        direction->vy = (s16)((part->behavior_flags.word >> 28) << 8);
        direction->vz = 0;
    }
    else
    {
        /* Clear both vector words, including the SDK padding halfword. */
        *(s32*)&direction->vz = 0;
        *(s32*)&direction->vx = 0;
    }

    if (((part->track_flags.word >> 6) & 3) != 0)
    {
        arc_height = (part->track_flags.word >> 26) << 9;
    }
    else
    {
        arc_height = 0;
    }

    segment = segment_count - 1;
    if (segment > 0)
    {
        next_line = &((FieldFlatLine*)packet_cursor)[1];
        do
        {
            next_line[0].color0.signed_word = next_line[-1].color0.signed_word;
            setLineF2(next_line - 1);
            setSemiTrans(next_line - 1, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

            gte_SetRotMatrix(matrix);
            gte_ldv0(direction);
            gte_rtv0();
            gte_stlvnl(jitter);

            if (arc_height != 0)
            {
                position->vy = center->vy + (delta->vy * segment) / segment_count -
                               ((s32)(((part->track_flags.word >> 26) << 9) * rsin(segment * angle_step)) >> 12) + jitter->vy;
            }
            else
            {
                position->vy = center->vy + (delta->vy * segment) / segment_count + jitter->vy;
            }

            position->vx = ((delta->vx * rcos(segment * angle_step)) >> 12) + center->vx + jitter->vx;
            position->vz = ((delta->vz * rcos(segment * angle_step)) >> 12) + center->vz + jitter->vz;

            {
                s32 camera_x;
                s32 position_x;
                s32 camera_y;
                camera_x = g_field_view_offset_x / 256;
                position_x = position->vx / 256;
                camera_y = g_field_view_offset_y;
                next_line[-1].xy1.signed_pair.x = camera_x + (s16)(position_x + FIELD_EFFECT_CENTER_X);
                if (camera_y < 0)
                {
                    camera_y += 255;
                }
                next_line[-1].xy1.signed_pair.y =
                    FIELD_EFFECT_CENTER_Y + (camera_y >> 8) + position->vy / 256 - position->vz / 512 - g_field_view_offset_z / 512;
            }
            next_line[0].xy0.word = next_line[-1].xy1.word;

            depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
            if (depth < 0)
            {
                s32 packet_address;
                next_line++;
                packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
                setaddr(packet_cursor, getaddr(&ordering_table[0]));
                packet_cursor += sizeof(LINE_F2);
                ordering_table[0] = (ordering_table[0] & FIELD_GPU_LENGTH_MASK) | packet_address;
            }
            else if (depth >= FIELD_EFFECT_OT_SIZE)
            {
                s32 packet_address;
                next_line++;
                packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
                setaddr(packet_cursor, getaddr(&ordering_table[(FIELD_EFFECT_OT_SIZE - 1)]));
                packet_cursor += sizeof(LINE_F2);
                ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] = (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & FIELD_GPU_LENGTH_MASK) | packet_address;
            }
            else
            {
                s32 packet_address;
                s32* entry;
                next_line++;
                packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
                setaddr(packet_cursor, getaddr(&ordering_table[depth]));
                entry = (s32*)((effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
                packet_cursor += sizeof(LINE_F2);
                *entry = (*entry & FIELD_GPU_LENGTH_MASK) | packet_address;
            }

            segment--;
            matrix++;
        } while (segment > 0);
    }

    setLineF2(packet_cursor);
    setSemiTrans(packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

    {
        s32 camera_x;
        s32 position_x;
        s32 camera_y;
        camera_x = g_field_view_offset_x / 256;
        position_x = origin->vx / 256;
        camera_y = g_field_view_offset_y;
        ((FieldFlatLine*)packet_cursor)[0].xy1.signed_pair.x = camera_x + (s16)(position_x + FIELD_EFFECT_CENTER_X);
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        camera_y = FIELD_EFFECT_CENTER_Y + (camera_y >> 8) + origin->vy / 256;
        camera_y -= origin->vz / 512;
        ((FieldFlatLine*)packet_cursor)[0].xy1.signed_pair.y = camera_y - g_field_view_offset_z / 512;
    }

    depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
    if (depth < 0)
    {
        s32 packet_address;
        packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
        setaddr(packet_cursor, getaddr(&ordering_table[0]));
        packet_cursor += sizeof(LINE_F2);
        ordering_table[0] = (ordering_table[0] & FIELD_GPU_LENGTH_MASK) | packet_address;
    }
    else if (depth >= FIELD_EFFECT_OT_SIZE)
    {
        s32 packet_address;
        packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
        setaddr(packet_cursor, getaddr(&ordering_table[(FIELD_EFFECT_OT_SIZE - 1)]));
        packet_cursor += sizeof(LINE_F2);
        ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] = (ordering_table[(FIELD_EFFECT_OT_SIZE - 1)] & FIELD_GPU_LENGTH_MASK) | packet_address;
    }
    else
    {
        s32 packet_address;
        s32* entry;
        packet_address = (s32)packet_cursor & FIELD_GPU_ADDRESS_MASK;
        setaddr(packet_cursor, getaddr(&ordering_table[depth]));
        entry = (s32*)((effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
        packet_cursor += sizeof(LINE_F2);
        *entry = (*entry & FIELD_GPU_LENGTH_MASK) | packet_address;
    }

    packet_cursor = field_emit_effect_texture_page(effect, part, packet_cursor, ordering_table);

    return packet_cursor;
}

#include "common.h"
#include "field_effect_transform.h"
#include "field_effect_types.h"
#include "field_effect_render_state.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/rand.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define FIELD_RIBBON_MAX_SEGMENTS 20
#define FIELD_RIBBON_FRAME_COUNT 12
#define FIELD_RIBBON_UV_VARIANT 0x01
#define FIELD_RIBBON_FLIP_U 0x80
#define FIELD_RIBBON_FLIP_V 0x40
#define FIELD_EFFECT_OT_SIZE 4096
#define FIELD_EFFECT_OT_DEPTH_SHIFT 7
#define FIELD_EFFECT_CENTER_X 160
#define FIELD_EFFECT_CENTER_Y 112
#define FIELD_ANGLE_QUARTER_TURN (ONE / 4)
#define FIELD_ANGLE_HALF_TURN (ONE / 2)
#define FIELD_ANGLE_MASK (ONE - 1)
#define FIELD_TRACK_INDEX_MASK 0xF
#define FIELD_EFFECT_TRACK_COLOR 0x00008000
#define FIELD_EFFECT_LITERAL_COLOR 0x10000000
#define FIELD_PART_REVERSE_ROTATION_WITH_FACING 0x00020000
#define FIELD_PART_SCALE_DISTANCE_BY_TARGET 0x01000000
#define FIELD_PART_SCALE_DISTANCE_BY_OWNER 0x01000000
#define FIELD_PART_SCALE_Z_BY_OWNER 0x02000000
#define FIELD_PART_SCALE_X_BY_OWNER 0x04000000
#define FIELD_RIBBON_SCRATCH ((FieldRibbonScratch*)0x1F800000)
#define FIELD_MATRIX_SCRATCH ((FieldMatrixScratch*)0x1F8000C0)
#define FIELD_SPRITE_SCRATCH ((FieldSpriteScratch*)0x1F800100)

#define FIELD_GPU_ADDRESS_MASK 0x00FFFFFF
#define FIELD_GPU_LENGTH_MASK 0xFF000000
#define FIELD_TRACK_COUNT 16
#define FIELD_EFFECT_TARGET_PITCH (ONE / 12)
#define FIELD_EFFECT_DEFAULT_PITCH (ONE / 16)
#define FIELD_PART_SCALE_TRACK_SHIFT 7
#define FIELD_PART_ROTATION_MODE_SHIFT 6
#define FIELD_PART_ROTATION_TRACK_SHIFT 26
#define FIELD_PART_ROTATE_QUARTER_X_SHIFT 17
#define FIELD_PART_PROJECT_SCALE_SHIFT 23
#define FIELD_PART_ORIENTED_SHIFT 3
#define FIELD_PART_AIM_AT_TARGET_SHIFT 11
#define FIELD_PART_STRETCH_TO_TARGET_SHIFT 2
#define FIELD_PART_RGB_TRACKS_SHIFT 12
#define FIELD_PART_RANDOM_EXTENT_SHIFT 15
#define FIELD_PART_ADD_HALF_WIDTH_SHIFT 16
#define FIELD_PART_ADD_HALF_HEIGHT_SHIFT 17
#define FIELD_PART_ATTACHMENT_SHIFT 18
#define FIELD_PART_COLOR_TRACK_SHIFT 5
#define FIELD_PART_RED_TRACK 0x4
#define FIELD_PART_GREEN_TRACK 0x2
#define FIELD_PART_BLUE_TRACK 0x1
#define FIELD_ATTACHMENT_MASK 0x3F
#define FIELD_ATTACHMENT_GROUP_COUNT 8
#define FIELD_ATTACHMENT_LINKED_BOUNDS_COUNT 28
#define FIELD_PART_OWNER_BOUNDS 2

/** @brief Attachment category boundaries encoded in part placement flags. */
typedef enum
{
    FIELD_ATTACHMENT_LINKED_BOUNDS_FIRST = 10,
    FIELD_ATTACHMENT_POINT_FIRST = 20,
    FIELD_ATTACHMENT_POINT_SECOND_BIAS = 34,
    FIELD_ATTACHMENT_POINT_SECOND = 42,
    FIELD_ATTACHMENT_UNRESOLVED_FIRST = 55
} FieldEffectAttachmentCategory;

/** @brief SDK matrix with word access to its rotation coefficients and padding. */
typedef union
{
    MATRIX matrix;
    struct
    {
        u32 rotation[5];
        s32 translation[3];
    } packed;
} FieldMatrixStorage;

/** @brief SDK angle vector with word access to paired halfwords. */
typedef union
{
    SVECTOR vector;
    struct
    {
        s32 xy;
        s32 z_pad;
    } packed;
} FieldRotationAngles;

/** @brief A packed XY pair, UV pair, and texture-page or palette halfword. */
typedef struct
{
    s32 xy;
    u16 uv;
    u16 page;
} FieldPackedQuadVertex;

/** @brief SDK quad with packed views for copying ribbon edges and color. */
typedef union
{
    POLY_FT4 gpu;
    struct
    {
        s32 tag;
        FieldPrimitiveColor color;
        FieldPackedQuadVertex vertices[4];
    } packed;
} FieldRibbonQuad;

/** @brief Ribbon path vectors followed by one rotation matrix per segment. */
typedef struct
{
    VECTOR endpoint;
    VECTOR position;
    VECTOR midpoint;
    VECTOR half_delta;
    VECTOR delta;
    FieldRotationAngles direction;
    MATRIX rotations[FIELD_RIBBON_MAX_SEGMENTS];
} FieldRibbonScratch;

/** @brief Temporary rotation and distance calculations for a part matrix. */
typedef struct
{
    SVECTOR direction;
    VECTOR scale;
    VECTOR delta;
    VECTOR squared_delta;
} FieldMatrixScratch;

/** @brief GTE output with unsigned low halfwords for GPU vertex additions. */
typedef union
{
    VECTOR vector;
    struct
    {
        u16 x;
        u16 x_high;
        u16 y;
        u16 y_high;
        u16 z;
        u16 z_high;
        u16 pad[2];
    } low;
} FieldSpriteProjection;

/** @brief Scratchpad vectors and local tilt matrix for sprite projection. */
typedef struct
{
    SVECTOR rotated_vertex;
    SVECTOR vertex;
    FieldSpriteProjection projected;
    FieldMatrixStorage rotation;
} FieldSpriteScratch;

extern FieldMotionRecord g_field_actors[];
extern FieldActorState g_field_actor_slots[80];
extern s32 g_field_track_index;

s32 field_evaluate_parameter_track(FieldActorState* actor, s32 track);
s32 field_evaluate_parameter_track_at_time(FieldActorState* actor, u32 track, u16 time);

/**
 * @brief Render a textured ribbon from an effect to its resolved target.
 * @param effect Position, facing, color, age, and segment count source.
 * @param packet_cursor Receives consecutive POLY_FT4 packets.
 * @param ordering_table Depth-indexed GPU ordering table.
 * @return Cursor after the last emitted quad.
 * @note animation_active supplies the segment count for this effect kind.
 * @see decomp.me (99.94%) WIP
 */
u8* field_render_effect_ribbon(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    FieldRibbonQuad* quad = (FieldRibbonQuad*)packet_cursor;
    FieldActorPartDef* part;
    FieldActorState* state;
    VECTOR* position;
    VECTOR* midpoint;
    VECTOR* half_delta;
    VECTOR* delta;
    MATRIX* rotation;
    VECTOR* endpoint;
    s32 segment_count;
    s32 i;
    s32 arc_height;
    s32 step;
    s32 angle;
    s32 edge_x;
    FieldRotationAngles* direction;
    s32 depth;
    u8 uv_flags;

    position = &FIELD_RIBBON_SCRATCH->position;
    midpoint = &FIELD_RIBBON_SCRATCH->midpoint;
    half_delta = &FIELD_RIBBON_SCRATCH->half_delta;
    delta = &FIELD_RIBBON_SCRATCH->delta;
    direction = &FIELD_RIBBON_SCRATCH->direction;

    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    state = &g_field_actor_slots[effect->actor_index];
    field_build_effect_part_matrix(effect, part, FIELD_RIBBON_SCRATCH->rotations, state);

    rotation = FIELD_RIBBON_SCRATCH->rotations;
    uv_flags = g_field_ribbon_frame_flags[((u16)effect->age) % FIELD_RIBBON_FRAME_COUNT];
    quad->packed.vertices[0].uv = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 0];
    quad->packed.vertices[1].uv = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 1];
    quad->packed.vertices[2].uv = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 2];
    quad->packed.vertices[3].uv = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 3];

    if (uv_flags & FIELD_RIBBON_FLIP_U)
    {
        i = quad->gpu.u0;
        quad->gpu.u0 = quad->gpu.u1;
        quad->gpu.u1 = i;
        i = quad->gpu.u2;
        quad->gpu.u2 = quad->gpu.u3;
        quad->gpu.u3 = i;
    }
    if (uv_flags & FIELD_RIBBON_FLIP_V)
    {
        i = quad->gpu.v0;
        quad->gpu.v0 = quad->gpu.v2;
        quad->gpu.v2 = i;
        i = quad->gpu.v1;
        quad->gpu.v1 = quad->gpu.v3;
        quad->gpu.v3 = i;
    }

    {
        u32 behavior_flags = part->behavior_flags.word;
        quad->gpu.clut = getClut(80, 492);
        quad->gpu.tpage = (u16)(((behavior_flags >> 17) & 0x60) | getTPage(0, 0, 448, 0));
    }

    gte_SetRotMatrix(rotation);

    {
        s32 view_x;
        s32 screen_x;
        s32 view_y;
        view_x = g_field_view_offset_x / 256;
        screen_x = effect->x / 256 + FIELD_EFFECT_CENTER_X;
        view_y = g_field_view_offset_y;
        quad->gpu.x0 = (s16)(view_x + screen_x);
        if (view_y < 0)
        {
            view_y += 255;
        }
        quad->gpu.y0 = (s16)(FIELD_EFFECT_CENTER_Y + (view_y >> 8) + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512);
    }

    field_resolve_effect_part_color(state, effect, part, &quad->packed.color);
    setPolyFT4(&quad->gpu);
    setSemiTrans(&quad->gpu, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

    rotation = FIELD_RIBBON_SCRATCH->rotations;
    segment_count = FIELD_RIBBON_MAX_SEGMENTS;
    if (effect->animation_active < FIELD_RIBBON_MAX_SEGMENTS)
    {
        segment_count = effect->animation_active;
    }
    endpoint = &FIELD_RIBBON_SCRATCH->endpoint;
    if (segment_count <= 0)
    {
        segment_count = 1;
    }
    step = FIELD_ANGLE_HALF_TURN / segment_count;

    field_resolve_effect_position(effect, part, &FIELD_RIBBON_SCRATCH->endpoint);
    {
        midpoint->vx = (FIELD_RIBBON_SCRATCH->endpoint.vx + effect->x) >> 1;
        midpoint->vy = FIELD_RIBBON_SCRATCH->endpoint.vy;
        midpoint->vz = (FIELD_RIBBON_SCRATCH->endpoint.vz + effect->z) >> 1;
        delta->vx = endpoint->vx - effect->x;
        half_delta->vx = delta->vx >> 1;
        delta->vy = effect->y - FIELD_RIBBON_SCRATCH->endpoint.vy;
        half_delta->vy = delta->vy;
        i = segment_count - 1;
        delta->vz = FIELD_RIBBON_SCRATCH->endpoint.vz - effect->z;
        half_delta->vz = delta->vz >> 1;
    }

    for (; i > 0; i--, rotation++)
    {
        direction->vector.vx = 0;
        direction->vector.vy = (s16)((rand() << 12) >> 15);
        direction->vector.vz = (s16)((rand() << 12) >> 16);
        RotMatrix_gte(&direction->vector, rotation);
    }

    rotation = FIELD_RIBBON_SCRATCH->rotations;

    if ((part->behavior_flags.bytes.low) >> FIELD_PART_SCALE_TRACK_SHIFT)
    {
        direction->vector.vx = 0;
        direction->vector.vy = (s16)((part->behavior_flags.word >> 28) << 8);
        direction->vector.vz = 0;
    }
    else
    {
        direction->packed.z_pad = 0;
        direction->packed.xy = 0;
    }

    if (((part->track_flags.word >> FIELD_PART_ROTATION_MODE_SHIFT) & 3) != 0)
    {
        arc_height = (part->track_flags.word >> FIELD_PART_ROTATION_TRACK_SHIFT) << 10;
    }
    else
    {
        arc_height = 0;
    }
    arc_height = (arc_height * rand()) >> 15;
    if (rand() & 1)
    {
        arc_height = -arc_height;
    }

    angle = ratan2(delta->vx, delta->vy + (delta->vz >> 1));
    edge_x = (rcos(angle) * part->footprint_scale_y) >> 12;
    angle = (rsin(angle) * part->footprint_scale_y) >> 12;
    if (uv_flags & FIELD_RIBBON_UV_VARIANT)
    {
        angle *= 2;
        edge_x *= 2;
    }

    i = segment_count - 1;
    quad->gpu.x2 = quad->gpu.x0 + edge_x;
    quad->gpu.y2 = quad->gpu.y0 + angle;
    quad->gpu.y0 -= angle;
    quad->gpu.x0 -= edge_x;

    if (i > 0)
    {
        s32 angle_step;
        s32 addr_mask = FIELD_GPU_ADDRESS_MASK;
        s32 tag_mask = FIELD_GPU_LENGTH_MASK;
        for (angle_step = i * step; i > 0; angle_step -= step, i--, rotation++)
        {
            depth = quad->packed.color.signed_word;
            quad[1].packed.color.signed_word = depth;
            setPolyFT4(&quad->gpu);
            setSemiTrans(&quad->gpu, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

            gte_SetRotMatrix(rotation);
            gte_ldv0(direction);
            gte_rtv0();
            gte_stlvnl(delta);

            if (arc_height != 0)
            {
                position->vy = midpoint->vy + (half_delta->vy * i) / segment_count - ((arc_height * rsin(angle_step)) >> 12) + delta->vy;
            }
            else
            {
                position->vy = midpoint->vy + (half_delta->vy * i) / segment_count + delta->vy;
            }
            position->vx = ((half_delta->vx * rcos(angle_step)) >> 12) + midpoint->vx + delta->vx;
            position->vz = ((half_delta->vz * rcos(angle_step)) >> 12) + midpoint->vz + delta->vz;

            {
                s32 view_x;
                s32 screen_x;
                s32 view_y;
                view_x = g_field_view_offset_x / 256;
                screen_x = position->vx / 256 + FIELD_EFFECT_CENTER_X;
                view_y = g_field_view_offset_y;
                quad->gpu.x1 = (s16)(view_x + screen_x);
                if (view_y < 0)
                {
                    view_y += 255;
                }
                quad->gpu.y1 = (s16)(FIELD_EFFECT_CENTER_Y + (view_y >> 8) + position->vy / 256 - position->vz / 512 - g_field_view_offset_z / 512);
            }
            quad->gpu.x3 = quad->gpu.x1 + edge_x;
            quad->gpu.y3 = quad->gpu.y1 + angle;
            quad->gpu.y1 -= angle;
            quad->gpu.x1 -= edge_x;

            {
                u16 first_uv;
                u16 clut;
                first_uv = quad->packed.vertices[0].uv;
                quad[1].packed.vertices[3].uv = quad->packed.vertices[3].uv;
                quad[1].packed.vertices[0].uv = first_uv;
                quad[1].gpu.tpage = quad->gpu.tpage;
                quad[1].packed.vertices[2].xy = quad->packed.vertices[3].xy;
                quad[1].packed.vertices[0].xy = quad->packed.vertices[1].xy;
                clut = quad->gpu.clut;
                quad[1].packed.vertices[1].uv = quad->packed.vertices[1].uv;
                quad[1].packed.vertices[2].uv = quad->packed.vertices[2].uv;
                quad[1].gpu.clut = clut;
            }

            depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
            if (depth < 0)
            {
                s32 addr = (s32)quad & addr_mask;
                quad->packed.tag = (quad->packed.tag & tag_mask) | (ordering_table[0] & addr_mask);
                quad++;
                ordering_table[0] = (ordering_table[0] & tag_mask) | addr;
            }
            else if (depth >= FIELD_EFFECT_OT_SIZE)
            {
                s32 addr = (s32)quad & addr_mask;
                quad->packed.tag = (quad->packed.tag & tag_mask) | (ordering_table[FIELD_EFFECT_OT_SIZE - 1] & addr_mask);
                quad++;
                ordering_table[FIELD_EFFECT_OT_SIZE - 1] = (ordering_table[FIELD_EFFECT_OT_SIZE - 1] & tag_mask) | addr;
            }
            else
            {
                s32 addr;
                s32* entry;
                addr = (s32)quad & addr_mask;
                quad->packed.tag = (quad->packed.tag & tag_mask) | (ordering_table[depth] & addr_mask);
                entry = (s32*)(((s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
                quad++;
                *entry = (*entry & tag_mask) | addr;
            }
        }
    }

    setPolyFT4(&quad->gpu);
    setSemiTrans(&quad->gpu, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

    {
        s32 view_x;
        s32 screen_x;
        s32 view_y;
        view_x = g_field_view_offset_x / 256;
        screen_x = endpoint->vx / 256 + FIELD_EFFECT_CENTER_X;
        view_y = g_field_view_offset_y;
        quad->gpu.x1 = (s16)(view_x + screen_x);
        if (view_y < 0)
        {
            view_y += 255;
        }
        {
            s32 depth;
            view_y = FIELD_EFFECT_CENTER_Y + (view_y >> 8) + endpoint->vy / 256;
            depth = endpoint->vz;
            view_y -= depth / 512;
            quad->gpu.y1 = view_y - g_field_view_offset_z / 512;
        }
    }
    quad->gpu.x3 = quad->gpu.x1 + edge_x;
    quad->gpu.y3 = quad->gpu.y1 + angle;
    quad->gpu.y1 -= angle;
    quad->gpu.x1 -= edge_x;

    depth = (s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
    if (depth < 0)
    {
        s32 addr = (s32)quad & FIELD_GPU_ADDRESS_MASK;
        setaddr(quad, getaddr(&ordering_table[0]));
        quad++;
        ordering_table[0] = (ordering_table[0] & FIELD_GPU_LENGTH_MASK) | addr;
    }
    else if (depth >= FIELD_EFFECT_OT_SIZE)
    {
        s32 addr = (s32)quad & FIELD_GPU_ADDRESS_MASK;
        setaddr(quad, getaddr(&ordering_table[FIELD_EFFECT_OT_SIZE - 1]));
        quad++;
        ordering_table[FIELD_EFFECT_OT_SIZE - 1] = (ordering_table[FIELD_EFFECT_OT_SIZE - 1] & FIELD_GPU_LENGTH_MASK) | addr;
    }
    else
    {
        s32 addr;
        s32* entry;
        s32 srcval;
        addr = (s32)quad & FIELD_GPU_ADDRESS_MASK;
        srcval = ordering_table[depth];
        setaddr(quad, srcval);
        entry = (s32*)(((s32)effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
        quad++;
        *entry = (*entry & FIELD_GPU_LENGTH_MASK) | addr;
    }
    return (u8*)quad;
}

/**
 * @brief Build an effect part's rotation, target alignment, and animated scale.
 * @param effect Position, orientation, and track sampling age.
 * @param part Rotation, alignment, and scale selectors.
 * @param matrix Receives the resulting transform.
 * @param actor Owner supplying parameter tracks and object bindings.
 * @return Unspecified; callers use only the matrix output.
 * @see decomp.me (100%)
 */
s32 field_build_effect_part_matrix(FieldMotionRecord* effect, FieldActorPartDef* part, MATRIX* matrix, FieldActorState* actor)
{
    FieldMatrixStorage* storage = (FieldMatrixStorage*)matrix;
    SVECTOR* direction = &FIELD_MATRIX_SCRATCH->direction;
    VECTOR* scale = &FIELD_MATRIX_SCRATCH->scale;
    VECTOR* delta = &FIELD_MATRIX_SCRATCH->delta;
    VECTOR* squared_delta = &FIELD_MATRIX_SCRATCH->squared_delta;
    s32 angle;
    s32 final_angle;
    s32 axis;
    s32 scale_mode;
    u32 distance;
    s32 rotation_mode;
    s32 target_component;
    s32 next_scale_track;
    u32 flags;

    storage->packed.rotation[4] = ONE;
    storage->packed.rotation[2] = ONE;
    storage->packed.rotation[0] = ONE;
    matrix->t[2] = 0;
    matrix->t[1] = 0;
    matrix->t[0] = 0;
    storage->packed.rotation[3] = 0;
    storage->packed.rotation[1] = 0;

    flags = part->track_flags.word;
    rotation_mode = (flags >> FIELD_PART_ROTATION_MODE_SHIFT) & 3;
    if (rotation_mode != 0)
    {
        switch (rotation_mode)
        {
        case 1:
            angle = field_evaluate_parameter_track_at_time(actor, flags >> 26, (u16)effect->age) << 4;
            axis = part->track_flags.bytes.high;
            axis &= 3;
            break;
        case 2:
            angle = effect->heading;
            axis = 1;
            break;
        case 3:
            angle = (((flags >> FIELD_PART_ROTATION_TRACK_SHIFT) * (u16)effect->age) << 4) & FIELD_ANGLE_MASK;
            axis = flags >> 24;
            axis &= 3;
            break;
        default:
            break;
        }

        if (part->spawn_flags.word & FIELD_PART_REVERSE_ROTATION_WITH_FACING)
        {
            if (!(g_field_actors[actor->owner_object_index].facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
            {
                angle = -angle;
            }
        }

        switch (axis)
        {
        case 0:
            RotMatrixX(angle, matrix);
            break;
        case 1:
            RotMatrixY(angle, matrix);
            break;
        case 2:
            RotMatrixZ(angle, matrix);
            break;
        case 3:
            RotMatrixZ(angle, matrix);
            RotMatrixY(angle, matrix);
            RotMatrixX(angle, matrix);
            break;
        }
    }

    if ((part->track_flags.word >> FIELD_PART_ROTATE_QUARTER_X_SHIFT) & 1)
    {
        RotMatrixX(FIELD_ANGLE_QUARTER_TURN, matrix);
    }
    if (part->rotation_y_16 != 0)
    {
        RotMatrixY(part->rotation_y_16 << 4, matrix);
    }

    if ((part->behavior_flags.word >> FIELD_PART_ORIENTED_SHIFT) & 1)
    {
        RotMatrixZ(FIELD_ANGLE_QUARTER_TURN, matrix);
        RotMatrixY(FIELD_ANGLE_QUARTER_TURN, matrix);
        if ((part->placement_flags.word >> FIELD_PART_AIM_AT_TARGET_SHIFT) & 1)
        {
            field_resolve_effect_position(effect, part, scale);
            target_component = scale->vz;
            angle = ratan2(effect->z - target_component, scale->vx - effect->x);
            delta->vx = (scale->vx - effect->x) >> 8;
            delta->vy = (scale->vy - effect->y) >> 8;
            delta->vz = (scale->vz - effect->z) >> 8;
            gte_ldlvl(delta);
            gte_sqr0();
            gte_stlvnl(squared_delta);
            distance = SquareRoot0(squared_delta->vx + squared_delta->vz + squared_delta->vy);
            target_component = scale->vy;
            RotMatrixZ(ratan2(distance << 8, effect->y - target_component), matrix);
            RotMatrixY(angle, matrix);
            final_angle = FIELD_EFFECT_TARGET_PITCH;
        }
        else
        {
            RotMatrixZ(effect->pitch, matrix);
            RotMatrixY(effect->heading, matrix);
            RotMatrixZ(effect->rotation_z_16 << 4, matrix);
            RotMatrixY(effect->rotation_y_16 << 4, matrix);
            final_angle = FIELD_EFFECT_DEFAULT_PITCH;
        }
    }
    else
    {
        RotMatrixY(FIELD_ANGLE_QUARTER_TURN, matrix);
        final_angle = FIELD_ANGLE_QUARTER_TURN;
    }
    RotMatrixX(final_angle, matrix);

    if ((part->placement_flags.word >> FIELD_PART_STRETCH_TO_TARGET_SHIFT) & 1)
    {
        field_resolve_effect_position(effect, part, scale);
        delta->vx = (scale->vx - effect->x) >> 8;
        delta->vy = (scale->vy - effect->y) >> 8;
        delta->vz = (scale->vz - effect->z) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(squared_delta);
        distance = SquareRoot0(squared_delta->vx + squared_delta->vy + squared_delta->vz);
        if (distance == 0)
        {
            distance = 1;
        }
        scale->vy = ONE;
        scale->vz = ONE;
        if (effect->sprite_height_minus_one != 0)
        {
            scale->vx = (distance << 12) / effect->sprite_height_minus_one;
        }
        else
        {
            scale->vx = (distance << 12) >> 6;
        }
        ScaleMatrix(matrix, scale);
    }

    if ((part->track_flags.word >> FIELD_PART_PROJECT_SCALE_SHIFT) & 1)
    {
        if ((part->behavior_flags.word >> FIELD_PART_ORIENTED_SHIFT) & 1)
        {
            direction->vx = -ONE;
            direction->vy = 0;
            direction->vz = 0;
            gte_SetRotMatrix(matrix);
            gte_ldv0(direction);
            gte_rtv0();
            gte_stlvnl(scale);
            direction->vx = 0;
            direction->vy = FIELD_ANGLE_QUARTER_TURN;
            direction->vz = FIELD_ANGLE_QUARTER_TURN;
            RotMatrix_gte(direction, matrix);
            RotMatrixZ(ratan2(scale->vy, scale->vx) + FIELD_ANGLE_QUARTER_TURN, matrix);
            gte_ldlvl(scale);
            gte_sqr0();
            gte_stlvnl(squared_delta);
            scale->vx = SquareRoot0(squared_delta->vx + squared_delta->vy);
            scale->vz = ONE;
            scale->vy = ONE;
            ScaleMatrix(matrix, scale);
        }
    }

    scale->vy = ONE;
    /* Stat-derived strength reduces the footprint in ten-percent steps. */
    if (part->palette_extent.word & FIELD_PART_SCALE_Z_BY_OWNER)
    {
        scale->vz = (part->appearance.fields.footprint_scale_x -
                     ((part->appearance.fields.footprint_scale_x *
                       ((s32)(FIELD_OBJECT_FOOTPRINT_STRENGTH_RANGE - g_field_object_states[actor->owner_object_index].effect_footprint_strength) >> 6)) /
                      10))
                    << 6;
    }
    else
    {
        scale->vz = part->appearance.fields.footprint_scale_x << 6;
    }
    if (part->palette_extent.word & FIELD_PART_SCALE_X_BY_OWNER)
    {
        scale->vx = (part->footprint_scale_y -
                     ((part->footprint_scale_y *
                       ((s32)(FIELD_OBJECT_FOOTPRINT_STRENGTH_RANGE - g_field_object_states[actor->owner_object_index].effect_footprint_strength) >> 6)) /
                      10))
                    << 6;
    }
    else
    {
        scale->vx = part->footprint_scale_y << 6;
    }
    ScaleMatrix(matrix, scale);

    g_field_effect_track_scale.x = ONE;
    g_field_effect_track_scale.y = ONE;
    g_field_effect_track_scale.z = ONE;

    if ((part->behavior_flags.word >> FIELD_PART_SCALE_TRACK_SHIFT) & 1)
    {
        scale->vz = ONE;
        scale->vy = ONE;
        scale->vx = ONE;
        flags = part->behavior_flags.word;
        scale_mode = (flags >> 20) & 3;
        switch (scale_mode)
        {
        case 0:
            scale->vz = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
            break;
        case 1:
            scale->vx = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
            break;
        case 2:
        {
            u32 value = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
            scale->vz = value;
            scale->vy = value;
            scale->vx = value;
            break;
        }
        case 3:
            scale->vz = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
            next_scale_track = (part->behavior_flags.word >> 28) + 1;
            scale->vx = field_evaluate_parameter_track_at_time(actor, next_scale_track % FIELD_TRACK_COUNT, (u16)effect->age) << 4;
            break;
        }
        g_field_effect_track_scale.x = (u16)scale->vx;
        g_field_effect_track_scale.y = (u16)scale->vy;
        g_field_effect_track_scale.z = (u16)scale->vz;
        ScaleMatrix(matrix, scale);
    }
}

/**
 * @brief Resolve a part's literal or animated primitive color.
 * @param actor Owner supplying parameter tracks.
 * @param effect Color flags, packed literal color, and track sampling age.
 * @param part Literal channel values and track selectors.
 * @param out Four-byte color storage; the literal path also copies its fourth byte.
 * @see decomp.me (100%)
 */
void field_resolve_effect_part_color(FieldActorState* actor, FieldMotionRecord* effect, FieldActorPartDef* part, FieldPrimitiveColor* out)
{
    s32 flags;
    u8 value;

    flags = effect->flags;
    if (!(flags & FIELD_EFFECT_TRACK_COLOR))
    {
        out->channels.r = ((part->appearance.word >> FIELD_PART_COLOR_TRACK_SHIFT) & FIELD_PART_RED_TRACK)
                              ? field_evaluate_parameter_track_at_time(actor, part->red_or_track & FIELD_TRACK_INDEX_MASK, (u16)effect->age)
                              : part->red_or_track;
        out->channels.g = ((part->appearance.word >> FIELD_PART_COLOR_TRACK_SHIFT) & FIELD_PART_GREEN_TRACK)
                              ? field_evaluate_parameter_track_at_time(actor, part->green_or_track & FIELD_TRACK_INDEX_MASK, (u16)effect->age)
                              : part->green_or_track;
        out->channels.b = ((part->appearance.word >> FIELD_PART_COLOR_TRACK_SHIFT) & FIELD_PART_BLUE_TRACK)
                              ? field_evaluate_parameter_track_at_time(actor, part->blue_or_track & FIELD_TRACK_INDEX_MASK, (u16)effect->age)
                              : part->blue_or_track;
        return;
    }

    if (flags & FIELD_EFFECT_LITERAL_COLOR)
    {
        /* The fourth byte travels with RGB and is overwritten by the packet builder. */
        out->word = effect->color_position.word;
        return;
    }

    {
        u32 part_flags = part->behavior_flags.word;
        if ((part_flags >> FIELD_PART_RGB_TRACKS_SHIFT) & 1)
        {
            out->channels.r =
                field_evaluate_parameter_track_at_time(actor, (part_flags >> FIELD_PART_ADD_HALF_WIDTH_SHIFT) & FIELD_TRACK_INDEX_MASK, (u16)effect->age);
            out->channels.g = field_evaluate_parameter_track_at_time(actor, (part->behavior_flags.halves.high & FIELD_TRACK_INDEX_MASK) + 1, (u16)effect->age);
            out->channels.b = field_evaluate_parameter_track_at_time(actor, (part->behavior_flags.halves.high & FIELD_TRACK_INDEX_MASK) + 2, (u16)effect->age);
            return;
        }

        value = field_evaluate_parameter_track_at_time(actor, (part_flags >> FIELD_PART_ADD_HALF_WIDTH_SHIFT) & FIELD_TRACK_INDEX_MASK, (u16)effect->age);
        out->channels.r = out->channels.g = out->channels.b = value;
    }
}

/**
 * @brief Append the effect's texture-page command at its clamped depth.
 * @param effect Supplies the ordering-table depth.
 * @param part Supplies the semi-transparency blend mode.
 * @param packet_cursor Receives one DR_TPAGE packet.
 * @param ordering_table Depth-indexed GPU ordering table.
 * @return Cursor immediately after the texture-page packet.
 * @see decomp.me (100%)
 */
u8* field_emit_effect_texture_page(FieldMotionRecord* effect, FieldActorPartDef* part, u8* packet_cursor, s32* ordering_table)
{
    s32 index;
    s32* entry;
    s32 srcval;

    setDrawTPage(packet_cursor, 0, 0, getTPage(0, (part->behavior_flags.word >> 22), 320, 0));

    index = effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT;
    if (index < 0)
    {
        addPrim(&ordering_table[0], packet_cursor);
        packet_cursor += sizeof(DR_TPAGE);
    }
    else if (index >= FIELD_EFFECT_OT_SIZE)
    {
        addPrim(&ordering_table[FIELD_EFFECT_OT_SIZE - 1], packet_cursor);
        packet_cursor += sizeof(DR_TPAGE);
    }
    else
    {
        srcval = ordering_table[index];
        setaddr(packet_cursor, srcval);
        entry = (s32*)((effect->z >> FIELD_EFFECT_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (u32)ordering_table);
        setaddr(entry, packet_cursor);
        packet_cursor += sizeof(DR_TPAGE);
    }
    return packet_cursor;
}

/**
 * @brief Project sprite corners, applying optional tilt and facing reversal.
 * @param effect Supplies the horizontal facing flag.
 * @param origin Screen-space origin, read as unsigned low halfwords.
 * @param quad POLY_FT4 packet receiving projected coordinates.
 * @param width Sprite width.
 * @param height Sprite height.
 * @param x Horizontal offset from the anchor.
 * @param y Vertical offset from the anchor.
 * @param frame Sprite record with tilt in 16 GTE angle units per step.
 * @param matrix Caller rotation matrix, already loaded into the GTE on entry.
 * @see decomp.me (100%)
 */
void field_project_effect_sprite_quad(FieldMotionRecord* effect, Vec2s* origin, POLY_FT4* quad, s32 width, s32 height, s32 x, s32 y, FieldSpriteFrame* frame,
                                      MATRIX* matrix)
{
    SVECTOR* rotated_vertex = &FIELD_SPRITE_SCRATCH->rotated_vertex;
    SVECTOR* vertex = &FIELD_SPRITE_SCRATCH->vertex;
    FieldSpriteProjection* out = &FIELD_SPRITE_SCRATCH->projected;
    FieldMatrixStorage* tilt_matrix = &FIELD_SPRITE_SCRATCH->rotation;

    /* The output uses unsigned low halves, including wrapped negative coordinates. */
    if (frame->tilt_16 == 0)
    {
        rotated_vertex->vx = y;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x0 = (u16)origin->x + out->low.x;
        quad->y0 = (u16)origin->y + out->low.y;

        rotated_vertex->vx = y;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x + width;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x1 = (u16)origin->x + out->low.x;
        quad->y1 = (u16)origin->y + out->low.y;

        rotated_vertex->vx = y + height;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x2 = (u16)origin->x + out->low.x;
        quad->y2 = (u16)origin->y + out->low.y;

        rotated_vertex->vx = y + height;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x + width;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x3 = (u16)origin->x + out->low.x;
        quad->y3 = (u16)origin->y + out->low.y;
        return;
    }

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        tilt_matrix->packed.rotation[4] = ONE;
        tilt_matrix->packed.rotation[2] = ONE;
        tilt_matrix->packed.rotation[0] = ONE;
        tilt_matrix->matrix.t[2] = 0;
        tilt_matrix->matrix.t[1] = 0;
        tilt_matrix->matrix.t[0] = 0;
        tilt_matrix->packed.rotation[3] = 0;
        tilt_matrix->packed.rotation[1] = 0;

        vertex->vx = y;
        vertex->vy = 0;
        vertex->vz = x + width;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x1 = (u16)origin->x + out->low.x;
        quad->y1 = (u16)origin->y + out->low.y;

        vertex->vx = 0;
        vertex->vy = -(frame->tilt_16 << 4);
        vertex->vz = 0;
        RotMatrix_gte(vertex, &tilt_matrix->matrix);

        vertex->vx = 0;
        vertex->vy = 0;
        vertex->vz = -width;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x0 = (u16)quad->x1 + out->low.x;
        quad->y0 = (u16)quad->y1 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = 0;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x3 = (u16)quad->x1 + out->low.x;
        quad->y3 = (u16)quad->y1 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = -width;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x2 = (u16)quad->x1 + out->low.x;
        quad->y2 = (u16)quad->y1 + out->low.y;
    }
    else
    {
        tilt_matrix->packed.rotation[4] = ONE;
        tilt_matrix->packed.rotation[2] = ONE;
        tilt_matrix->packed.rotation[0] = ONE;
        tilt_matrix->matrix.t[2] = 0;
        tilt_matrix->matrix.t[1] = 0;
        tilt_matrix->matrix.t[0] = 0;
        tilt_matrix->packed.rotation[3] = 0;
        tilt_matrix->packed.rotation[1] = 0;

        vertex->vx = y;
        vertex->vy = 0;
        vertex->vz = x;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x0 = (u16)origin->x + out->low.x;
        quad->y0 = (u16)origin->y + out->low.y;

        vertex->vx = 0;
        vertex->vy = frame->tilt_16 << 4;
        vertex->vz = 0;
        RotMatrix_gte(vertex, &tilt_matrix->matrix);

        vertex->vx = 0;
        vertex->vy = 0;
        vertex->vz = width;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x1 = (u16)quad->x0 + out->low.x;
        quad->y1 = (u16)quad->y0 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = 0;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x2 = (u16)quad->x0 + out->low.x;
        quad->y2 = (u16)quad->y0 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = width;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x3 = (u16)quad->x0 + out->low.x;
        quad->y3 = (u16)quad->y0 + out->low.y;
    }
}

/**
 * @brief Decode signed byte geometry corners and optionally mirror them.
 * @param out Receives four interleaved x/y pairs.
 * @param flip Nonzero to reverse corner order and negate x coordinates.
 * @param item Packed corner record; byte 7 is not a coordinate.
 * @see decomp.me (100%)
 */
void field_unpack_effect_quad_corners8(s16* out, s32 flip, s8* item)
{
    if (flip)
    {
        out[0] = -item[2];
        out[1] = item[3];
        out[2] = -item[0];
        out[3] = item[1];
        out[4] = -item[6];
        out[5] = item[8];
        out[6] = -item[4];
        out[7] = item[5];
    }
    else
    {
        out[0] = item[0];
        out[1] = item[1];
        out[2] = item[2];
        out[3] = item[3];
        out[4] = item[4];
        out[5] = item[5];
        out[6] = item[6];
        out[7] = item[8];
    }
}

/**
 * @brief Decode little-endian geometry corners and optionally mirror them.
 * @param out Receives four interleaved x/y pairs.
 * @param mirror Nonzero to reverse corner order and negate x coordinates.
 * @param item Packed corner record; byte 7 interrupts the second y component.
 * @see decomp.me (100%)
 */
void field_unpack_effect_quad_corners16(s16* out, s32 mirror, u8* item)
{
    if (mirror != 0)
    {
        out[0] = -(item[4] + (item[5] << 8));
        out[1] = item[6] + (item[8] << 8);
        out[2] = -(item[0] + (item[1] << 8));
        out[3] = item[2] + (item[3] << 8);
        out[4] = -(item[13] + (item[14] << 8));
        out[5] = item[15] + (item[16] << 8);
        out[6] = -(item[9] + (item[10] << 8));
        out[7] = item[11] + (item[12] << 8);
    }
    else
    {
        out[0] = item[0] + (item[1] << 8);
        out[1] = item[2] + (item[3] << 8);
        out[2] = item[4] + (item[5] << 8);
        out[3] = item[6] + (item[8] << 8);
        out[4] = item[9] + (item[10] << 8);
        out[5] = item[11] + (item[12] << 8);
        out[6] = item[13] + (item[14] << 8);
        out[7] = item[15] + (item[16] << 8);
    }
}

/**
 * @brief Resolve a part's extent from tracks, distance, randomness, and bounds.
 * @param actor Owner supplying tracks and linked object indices.
 * @param part Packed extent value, modifiers, and attachment category.
 * @return Extent after scaling and optional bounding-box half-extents.
 * @note TODO: categories 55-62 do not initialize the part index on this path.
 * @see decomp.me (100%)
 */
s32 field_resolve_effect_extent(FieldActorState* actor, FieldActorPartDef* part)
{
    VECTOR delta;
    VECTOR squared_delta;
    s32 extent;
    s32 part_index;
    s32 half_extent;
    u32 flags;
    FieldActorState* owner;

    {
        u32 value_flags = part->rotation_extent.word;
        s32 value_mode = (value_flags >> FIELD_PART_ROTATION_MODE_SHIFT) & 3;
        owner = actor;
        switch (value_mode)
        {
        case 0:
        {
            s32 value_low_bits = part->palette_extent.word >> 29;
            extent = ((value_flags & 0x3F) << 3) | value_low_bits;
            break;
        }
        case 1:
        {
            u32 value_low_bits = part->palette_extent.word >> 29;
            extent = field_evaluate_parameter_track(owner, (((value_flags & 0x3F) << 3) | value_low_bits) & FIELD_TRACK_INDEX_MASK);
            break;
        }
        case 2:
        {
            u32 value_low_bits = part->palette_extent.word;
            extent = field_evaluate_parameter_track_at_time(owner, (((value_flags & 0x3F) << 3) | (value_low_bits >> 29)) & FIELD_TRACK_INDEX_MASK, 0);
            break;
        }
        }
    }

    if (part->effect_flags & FIELD_PART_SCALE_DISTANCE_BY_TARGET)
    {
        delta.vx = (g_field_actors[owner->track_object_indices[g_field_track_index]].x - g_field_actors[owner->owner_object_index].x) >> 8;
        delta.vy = (g_field_actors[owner->track_object_indices[g_field_track_index]].y - g_field_actors[owner->owner_object_index].y) >> 8;
        delta.vz = (g_field_actors[owner->track_object_indices[g_field_track_index]].z - g_field_actors[owner->owner_object_index].z) >> 8;
        gte_ldlvl(&delta);
        gte_sqr0();
        gte_stlvnl(&squared_delta);
        extent = (SquareRoot0(squared_delta.vx + squared_delta.vy + squared_delta.vz) * extent) / 100;
    }

    if ((part->placement_flags.word >> FIELD_PART_RANDOM_EXTENT_SHIFT) & 1)
    {
        extent = (extent * rand()) >> 15;
    }

    if (part->palette_extent.word & FIELD_PART_SCALE_DISTANCE_BY_OWNER)
    {
        extent = ((g_field_object_states[owner->owner_object_index].movement.half.flags & FIELD_OBJECT_EFFECT_SCALE_MASK) * extent) / 100;
    }

    flags = part->placement_flags.word;
    {
        s32 category = (flags >> FIELD_PART_ATTACHMENT_SHIFT) & FIELD_ATTACHMENT_MASK;
        if (category < FIELD_ATTACHMENT_POINT_FIRST)
        {
            if ((flags >> FIELD_PART_ADD_HALF_WIDTH_SHIFT) & 1)
            {
                if ((u32)(category - FIELD_ATTACHMENT_LINKED_BOUNDS_FIRST) >= FIELD_ATTACHMENT_LINKED_BOUNDS_COUNT)
                {
                    half_extent = (g_field_object_states[owner->owner_object_index].bounds.half.right -
                                   g_field_object_states[owner->owner_object_index].bounds.half.left) >>
                                  1;
                }
                else
                {
                    half_extent = (g_field_object_states[owner->track_object_indices[g_field_track_index]].bounds.half.right -
                                   g_field_object_states[owner->track_object_indices[g_field_track_index]].bounds.half.left) >>
                                  1;
                }
                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }
                extent += half_extent;
            }
            if ((part->placement_flags.word >> FIELD_PART_ADD_HALF_HEIGHT_SHIFT) & 1)
            {
                if ((u32)(((part->placement_flags.word >> FIELD_PART_ATTACHMENT_SHIFT) & FIELD_ATTACHMENT_MASK) - FIELD_ATTACHMENT_LINKED_BOUNDS_FIRST) >=
                    FIELD_ATTACHMENT_LINKED_BOUNDS_COUNT)
                {
                    half_extent = (g_field_object_states[owner->owner_object_index].bounds.half.bottom -
                                   g_field_object_states[owner->owner_object_index].bounds.half.top) >>
                                  1;
                }
                else
                {
                    half_extent = (g_field_object_states[owner->track_object_indices[g_field_track_index]].bounds.half.bottom -
                                   g_field_object_states[owner->track_object_indices[g_field_track_index]].bounds.half.top) >>
                                  1;
                }
                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }
                extent += half_extent;
            }
        }
    }

    {
        s32 category = (part->placement_flags.word >> FIELD_PART_ATTACHMENT_SHIFT) & FIELD_ATTACHMENT_MASK;
        if ((u32)(category - FIELD_ATTACHMENT_POINT_SECOND) < FIELD_ATTACHMENT_GROUP_COUNT ||
            (u32)(category - FIELD_ATTACHMENT_POINT_FIRST) < FIELD_ATTACHMENT_GROUP_COUNT ||
            (u32)(category - FIELD_ATTACHMENT_UNRESOLVED_FIRST) < FIELD_ATTACHMENT_GROUP_COUNT)
        {
            s32 attachment_category = (part->placement_flags.word >> FIELD_PART_ATTACHMENT_SHIFT) & FIELD_ATTACHMENT_MASK;
            if (attachment_category < FIELD_ATTACHMENT_UNRESOLVED_FIRST)
            {
                part_index = attachment_category - FIELD_ATTACHMENT_POINT_FIRST;
                if (attachment_category >= FIELD_ATTACHMENT_POINT_SECOND)
                {
                    part_index = attachment_category - FIELD_ATTACHMENT_POINT_SECOND_BIAS;
                }
            }

            if ((part->placement_flags.halves.high) & 1)
            {
                if (owner->parts[part_index].effect_kind == FIELD_PART_OWNER_BOUNDS)
                {
                    half_extent = (g_field_object_states[owner->owner_object_index].bounds.half.right -
                                   g_field_object_states[owner->owner_object_index].bounds.half.left) >>
                                  1;
                }
                else
                {
                    half_extent = (g_field_object_states[owner->track_object_indices[g_field_track_index]].bounds.half.right -
                                   g_field_object_states[owner->track_object_indices[g_field_track_index]].bounds.half.left) >>
                                  1;
                }
                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }
                extent += half_extent;
            }

            if ((part->placement_flags.word >> FIELD_PART_ADD_HALF_HEIGHT_SHIFT) & 1)
            {
                if (owner->parts[part_index].effect_kind == FIELD_PART_OWNER_BOUNDS)
                {
                    half_extent = (g_field_object_states[owner->owner_object_index].bounds.half.bottom -
                                   g_field_object_states[owner->owner_object_index].bounds.half.top) >>
                                  1;
                }
                else
                {
                    half_extent = (g_field_object_states[owner->track_object_indices[g_field_track_index]].bounds.half.bottom -
                                   g_field_object_states[owner->track_object_indices[g_field_track_index]].bounds.half.top) >>
                                  1;
                }
                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }
                extent += half_extent;
            }
        }
    }

    return extent;
}

/** @file field_effect_geometry.c
 * @brief Project actor-attached effect geometry and update its per-slot screen-space caches.
 */

#include "common.h"
#include "field_effect_transform.h"
#include "field_effect_render_state.h"
#include "field_types.h"
#include "field_effect_types.h"
#include "field_effect_geometry.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

#define FIELD_PART_ANCHOR_MODE_SHIFT 18
#define FIELD_PART_ANCHOR_MODE_MASK 0x3F
#define FIELD_PART_SCALE_X_FROM_BOUNDS_SHIFT 9
#define FIELD_PART_SCALE_Y_FROM_BOUNDS_SHIFT 1
#define FIELD_PART_MIRROR_X_WITH_FACING_SHIFT 10
#define FIELD_PART_ATTACHMENT_INDEX_SHIFT 21
#define FIELD_PART_ATTACHMENT_INDEX_MASK 3
#define FIELD_PART_MIRROR_X_WITH_OWNER 0x08000000

extern FieldMotionRecord g_field_actors[];
extern FieldMotionRecord g_field_effect_records[];
extern s32 g_field_track_index;

/**
 * @brief Resolve the world-space anchor point for an actor part.
 * @param actor Owning actor state.
 * @param part Actor part definition selecting the anchor mode.
 * @param out Receives the resolved x/y/z anchor.
 * @param attachment_index Attachment point index used by ground-relative anchor modes.
 * @see decomp.me (100.00%)
 */
void field_resolve_actor_part_anchor(FieldActorState *actor, FieldActorPartDef *part, Vec3i *out, s32 attachment_index)
{
    s32 anchor_mode;
    s32 object_index;
    FieldMotionRecord *object_record;
    FieldObjectRuntime *object;
    s32 value;
    s32 index;

    anchor_mode = part->placement_flags.word >> FIELD_PART_ANCHOR_MODE_SHIFT;
    anchor_mode &= FIELD_PART_ANCHOR_MODE_MASK;
    switch (anchor_mode)
    {
    case 0x00: case 0x01: case 0x02: case 0x03: case 0x04:
    case 0x05: case 0x06: case 0x07: case 0x08: case 0x09:
    case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E:
    case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13:
    {
        if (anchor_mode >= 0xA)
        {
            object_index = actor->track_object_indices[g_field_track_index];
            anchor_mode -= 0xA;
            object_record = &g_field_actors[object_index];
            object = &g_field_object_states[object_index];
        }
        else
        {
            object_index = actor->owner_object_index;
            object_record = &g_field_actors[object_index];
            object = &g_field_object_states[object_index];
        }
        if ((part->placement_flags.word >> FIELD_PART_SCALE_X_FROM_BOUNDS_SHIFT) & 1)
        {
            part->appearance.fields.footprint_scale_x = (*(u8 *)&object->bounds.half.right - *(u8 *)&object->bounds.half.left) * 2;
        }
        if ((part->placement_flags.word >> FIELD_PART_SCALE_Y_FROM_BOUNDS_SHIFT) & 1)
        {
            index = 0;
            part->footprint_scale_y = (*(u8 *)&object->bounds.half.bottom - *(u8 *)&object->bounds.half.top) * 2;
        }
        else
        {
            index = 0;
        }
        value = index;
        switch (anchor_mode)
        {
        case 1:
            value = (object->bounds.half.right + object->bounds.half.left) >> 1;
            index = (object->bounds.half.bottom + object->bounds.half.top) >> 1;
            break;
        case 2:
            value = (object->bounds.half.right + object->bounds.half.left) >> 1;
            index = 0;
            break;
        case 3:
            value = (object->bounds.half.right + object->bounds.half.left) >> 1;
            index = object->bounds.half.top;
            break;
        case 4:
            value = object->bounds.half.left;
            index = (object->bounds.half.bottom + object->bounds.half.top) >> 1;
            break;
        case 5:
            value = object->bounds.half.right;
            index = (object->bounds.half.bottom + object->bounds.half.top) >> 1;
            break;
        case 6:
            value = object->bounds.half.left;
            index = object->bounds.half.top;
            break;
        case 7:
            value = object->bounds.half.right;
            index = object->bounds.half.top;
            break;
        case 8:
            value = object->bounds.half.left;
            index = object->bounds.half.bottom;
            break;
        case 9:
            value = object->bounds.half.right;
            index = object->bounds.half.bottom;
            break;
        }
        value <<= 8;
        out->x = object_record->x + value;
        index <<= 8;
        out->y = object_record->y + index;
        out->z = object_record->z;
        return;
    }

    case 0x14: case 0x15: case 0x16: case 0x17:
    case 0x18: case 0x19: case 0x1A: case 0x1B:
    case 0x2A: case 0x2B: case 0x2C: case 0x2D:
    case 0x2E: case 0x2F: case 0x30: case 0x31:
        value = anchor_mode - 0x14;
        if (anchor_mode >= 0x2A)
        {
            value = anchor_mode - 0x22;
        }
        index = 0;
        do
        {
            if (g_field_effect_records[index].state != FIELD_EFFECT_RETIRED && g_field_effect_records[index].part_index == value &&
                g_field_effect_records[index].actor_index == actor->actor_index && g_field_effect_records[index].track_index == 0)
            {
                out->x = g_field_effect_records[index].x;
                out->y = g_field_effect_records[index].y;
                out->z = g_field_effect_records[index].z;
                return;
            }
            index++;
        } while (index < FIELD_EFFECT_ACTIVE_RECORD_COUNT);
        return;

    case 0x25:
        out->x = (part->offset_x << 8) - g_field_view_offset_x;
        out->y = (part->offset_y << 8) - g_field_view_offset_y;
        out->z = (part->offset_z << 8) - g_field_view_offset_z;
        return;
    case 0x1C:
        out->x = -g_field_view_offset_x;
        out->y = -g_field_view_offset_y;
        out->z = -g_field_view_offset_z;
        return;
    case 0x1D:
        out->y = -0x7000;
        out->x = -g_field_view_offset_x;
        out->y -= g_field_view_offset_y;
        out->z = -g_field_view_offset_z;
        return;
    case 0x1E:
        out->y = 0x7000;
        out->x = -g_field_view_offset_x;
        out->y -= g_field_view_offset_y;
        out->z = -g_field_view_offset_z;
        return;
    case 0x1F:
        out->x = 0xFFFF6000;
        out->x -= g_field_view_offset_x;
        out->y = -g_field_view_offset_y;
        out->z = -g_field_view_offset_z;
        return;
    case 0x20:
        out->x = 0xA000;
        out->x -= g_field_view_offset_x;
        out->y = -g_field_view_offset_y;
        out->z = -g_field_view_offset_z;
        return;
    case 0x21:
        out->x = 0xFFFF6000;
        out->y = -0x7000;
        out->x -= g_field_view_offset_x;
        out->y -= g_field_view_offset_y;
        out->z = -g_field_view_offset_z;
        return;
    case 0x22:
        out->x = 0xA000;
        out->y = -0x7000;
        out->x -= g_field_view_offset_x;
        out->y -= g_field_view_offset_y;
        out->z = -g_field_view_offset_z;
        return;
    case 0x23:
        out->x = 0xFFFF6000;
        out->y = 0x7000;
        out->x -= g_field_view_offset_x;
        out->y -= g_field_view_offset_y;
        out->z = -g_field_view_offset_z;
        return;
    case 0x24:
        out->x = 0xA000;
        out->y = 0x7000;
        out->x -= g_field_view_offset_x;
        out->y -= g_field_view_offset_y;
        out->z = -g_field_view_offset_z;
        return;
    case 0x26:
        return;

    case 0x27:
        object_record = &g_field_actors[actor->owner_object_index];
        if ((((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) ||
             (part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER)) &&
            !(object_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
        {
            out->x = object_record->x - (part->offset_x << 8);
        }
        else
        {
            out->x = object_record->x + (part->offset_x << 8);
        }
        out->y = object_record->y + (part->offset_y << 8);
        out->z = object_record->z + (part->offset_z << 8);
        return;

    case 0x28:
        object_record = &g_field_actors[actor->track_object_indices[g_field_track_index]];
        if ((part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER) &&
            !(g_field_actors[actor->owner_object_index].facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
        {
            out->x = object_record->x - (part->offset_x << 8);
        }
        else if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
                 !(object_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
        {
            out->x = object_record->x - (part->offset_x << 8);
        }
        else
        {
            out->x = object_record->x + (part->offset_x << 8);
        }
        out->y = object_record->y + (part->offset_y << 8);
        out->z = object_record->z + (part->offset_z << 8);
        return;

    case 0x29:
        object_index = actor->owner_object_index;
        object_record = &g_field_actors[object_index];
        object = &g_field_object_states[object_index];
        out->x = object_record->x +
                 (object->attachment_points[((u32)part->effect_flags >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].x << 8);
        out->y = object_record->y +
                 (object->attachment_points[((u32)part->effect_flags >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].y << 8);
        out->z = object_record->z;
        return;

    case 0x32:
        object_index = actor->owner_object_index;
        object_record = &g_field_actors[object_index];
        object = &g_field_object_states[object_index];
        out->x = object_record->x +
                 (object->attachment_points[((u32)part->effect_flags >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].x << 8);
        out->y = object_record->y;
        out->z = object_record->z + (part->offset_z << 8);
        if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) && !(object_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
        {
            out->x -= part->offset_x << 8;
        }
        else
        {
            out->x += part->offset_x << 8;
        }
        return;

    case 0x33:
        object_index = actor->owner_object_index;
        object_record = &g_field_actors[object_index];
        object = &g_field_object_states[object_index];
        out->x = object_record->x + (object->ground_attachment_points[attachment_index].x << 8);
        out->y = object_record->y;
        out->z = object_record->z + (object->ground_attachment_points[attachment_index].y << 8);
        return;

    case 0x36:
        out->x = part->offset_x << 8;
        out->y = part->offset_y << 8;
        out->z = part->offset_z << 8;
        return;

    case 0x37: case 0x38: case 0x39: case 0x3A:
    case 0x3B: case 0x3C: case 0x3D: case 0x3E:
        value = anchor_mode - 0x37;
        index = 0;
        do
        {
            if (g_field_effect_records[index].state != FIELD_EFFECT_RETIRED && g_field_effect_records[index].part_index == value &&
                g_field_effect_records[index].actor_index == actor->actor_index && g_field_effect_records[index].track_index == 0)
            {
                out->x = g_field_effect_records[index].x;
                out->y = g_field_effect_records[index].y;
                out->z = g_field_effect_records[index].z;
            }
            index++;
        } while (index < FIELD_EFFECT_ACTIVE_RECORD_COUNT);
        return;

    default:
        out->x = 0;
        out->y = 0;
        out->z = 0;
        return;
    }
}

/**
 * @brief Extract four signed-byte quad corners from an effect frame.
 * @param effect Effect record whose facing flag selects the mirrored layout.
 * @param frame_data Encoded frame entry list; the first byte is the entry count.
 * @param corners Receives four x/y corner pairs.
 * @see decomp.me (100.00%)
 */
void field_extract_effect_quad_corners8(FieldMotionRecord *effect, u8 *frame_data, s16 *corners)
{
    s32 count;

    corners[0] = corners[1] = 0;
    corners[2] = corners[3] = 0;
    corners[4] = corners[5] = 0;
    corners[6] = corners[7] = 0;

    count = *frame_data++;
    if (count != 0)
    {
        do
        {
            if (frame_data[7] & 0x20)
            {
                if ((frame_data[7] & 0xF) == 2)
                {
                    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                    {
                        corners[0] = -(s8)frame_data[2];
                        corners[1] = (s8)frame_data[3];
                        corners[2] = -(s8)frame_data[0];
                        corners[3] = (s8)frame_data[1];
                        corners[4] = -(s8)frame_data[6];
                        corners[5] = (s8)frame_data[8];
                        corners[6] = -(s8)frame_data[4];
                        corners[7] = (s8)frame_data[5];
                    }
                    else
                    {
                        corners[0] = (s8)frame_data[0];
                        corners[1] = (s8)frame_data[1];
                        corners[2] = (s8)frame_data[2];
                        corners[3] = (s8)frame_data[3];
                        corners[4] = (s8)frame_data[4];
                        corners[5] = (s8)frame_data[5];
                        corners[6] = (s8)frame_data[6];
                        corners[7] = (s8)frame_data[8];
                    }
                }
            }
            frame_data += 9;
            count--;
        } while (count != 0);
    }
}

/**
 * @brief Extract four 16-bit quad corners from a variable-stride effect frame.
 * @param effect Effect record whose facing flag selects the mirrored layout.
 * @param frame_data Encoded frame entry list; the first byte is the entry count.
 * @param corners Receives four x/y corner pairs.
 * @see decomp.me (100.00%)
 */
void field_extract_effect_quad_corners16(FieldMotionRecord *effect, u8 *frame_data, s16 *corners)
{
    s32 count;

    corners[0] = corners[1] = 0;
    corners[2] = corners[3] = 0;
    corners[4] = corners[5] = 0;
    corners[6] = corners[7] = 0;

    count = *frame_data++;
    if (count != 0)
    {
        do
        {
            if (frame_data[7] & 0x20)
            {
                if ((frame_data[7] & 0xF) == 2)
                {
                    field_unpack_effect_quad_corners16(corners, effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED, frame_data);
                }
                frame_data += 0x11;
            }
            else
            {
                frame_data += 0xB;
            }
            count--;
        } while (count != 0);
    }
}

#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/**
 * @brief Apply the actor rotation to a bounding-box centre and accumulate it
 *        into the screen-space position.
 * @param effect Effect record whose facing flag mirrors the horizontal center offset.
 * @param screen_origin Screen-space position accumulator.
 * @param quad_bounds Bounding-box extents (indices 0/1/2/5 used in mode 1).
 * @param fallback_depth Horizontal offset used when mode does not derive it from the quad.
 * @param mode Zero skips adjustment; one derives the center from the quad; other values use fallback_depth.
 * @see decomp.me (100.00%)
 */
void field_apply_effect_quad_center_offset(FieldMotionRecord *effect, DVECTOR *screen_origin, s16 *quad_bounds, s32 fallback_depth, s32 mode)
{
    SVECTOR offset;
    VECTOR rotated;
    s32 center_y;
    s32 center_x = fallback_depth;

    if (mode != 0)
    {
        if (mode == 1)
        {
            center_x = (quad_bounds[2] + quad_bounds[0]) / 2;
            center_y = (quad_bounds[5] + quad_bounds[1]) / 2;
        }

        offset.vx = -center_y;
        offset.vy = 0;
        offset.vz = -center_x;
        gte_ldv0(&offset);
        gte_rtv0();
        gte_stlvnl(&rotated);

        if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
        {
            center_x = -center_x;
        }
        screen_origin->vx += (s16)rotated.vx + center_x;
        screen_origin->vy += (s16)rotated.vy + center_y;
    }
}

/**
 * @brief Transform four signed-byte quad vertices into screen space.
 * @param effect Effect record whose facing flag mirrors the horizontal component.
 * @param object Object placement receiving the transformed vertices.
 * @param quad_data Encoded signed-byte corner pairs.
 * @param vertex_index Base vertex index into object->effect_vertices.
 * @param screen_origin Screen-space origin added to every transformed vertex.
 * @param direction Scratch direction vector fed to the GTE.
 * @param gte_out Scratch GTE output vector.
 * @see decomp.me (100.00%)
 */
void field_transform_effect_quad_vertices8(FieldMotionRecord *effect, FieldObjectRuntime *object, u8 *quad_data, s32 vertex_index,
                                           Vec2s *screen_origin, SVECTOR *direction, VECTOR *gte_out)
{
    s16 depth_component;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = (s8)quad_data[1];
        direction->vy = 0;
        depth_component = -(s8)quad_data[0];
    }
    else
    {
        direction->vx = (s8)quad_data[1];
        direction->vy = 0;
        depth_component = (s8)quad_data[0];
    }
    direction->vz = depth_component;
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    if (vertex_index == 0)
    {
        object->bounds.half.left = (u16)gte_out->vx;
        object->bounds.half.top = (u16)gte_out->vy - (s8)effect->vertical_offset;
    }
    object->effect_vertices.points[vertex_index].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index].y = (u16)gte_out->vy + (u16)screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = (s8)quad_data[3];
        direction->vy = 0;
        depth_component = -(s8)quad_data[2];
    }
    else
    {
        direction->vx = (s8)quad_data[3];
        direction->vy = 0;
        depth_component = (s8)quad_data[2];
    }
    direction->vz = depth_component;
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    object->effect_vertices.points[vertex_index + 1].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index + 1].y = (u16)gte_out->vy + (u16)screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = (s8)quad_data[5];
        direction->vy = 0;
        depth_component = -(s8)quad_data[4];
    }
    else
    {
        direction->vx = (s8)quad_data[5];
        direction->vy = 0;
        depth_component = (s8)quad_data[4];
    }
    direction->vz = depth_component;
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    if (vertex_index == 0)
    {
        object->bounds.half.right = (u16)gte_out->vx;
        object->bounds.half.bottom = (u16)gte_out->vy - (s8)effect->vertical_offset;
    }
    object->effect_vertices.points[vertex_index + 2].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index + 2].y = (u16)gte_out->vy + (u16)screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = (s8)quad_data[8];
        direction->vy = 0;
        depth_component = -(s8)quad_data[6];
    }
    else
    {
        direction->vx = (s8)quad_data[8];
        direction->vy = 0;
        depth_component = (s8)quad_data[6];
    }
    direction->vz = depth_component;
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    object->effect_vertices.points[vertex_index + 3].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index + 3].y = (u16)gte_out->vy + (u16)screen_origin->y;
}

/**
 * @brief Transform four little-endian 16-bit quad vertices into screen space.
 * @param effect Effect record whose facing flag mirrors the horizontal component.
 * @param object Object placement receiving the transformed vertices.
 * @param quad_data Encoded little-endian 16-bit corner pairs.
 * @param vertex_index Base vertex index into object->effect_vertices.
 * @param screen_origin Screen-space origin added to every transformed vertex.
 * @param direction Scratch direction vector fed to the GTE.
 * @param gte_out Scratch GTE output vector.
 * @see decomp.me (100.00%)
 */
void field_transform_effect_quad_vertices16(FieldMotionRecord *effect, FieldObjectRuntime *object, u8 *quad_data, s32 vertex_index,
                                            Vec2s *screen_origin, SVECTOR *direction, VECTOR *gte_out)
{
    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = quad_data[2] + (quad_data[3] << 8);
        direction->vy = 0;
        direction->vz = -(quad_data[0] + (quad_data[1] << 8));
    }
    else
    {
        direction->vx = quad_data[2] + (quad_data[3] << 8);
        direction->vy = 0;
        direction->vz = quad_data[0] + (quad_data[1] << 8);
    }
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    if (vertex_index == 0)
    {
        object->bounds.half.left = (u16)gte_out->vx;
        object->bounds.half.top = (u16)gte_out->vy;
    }
    object->effect_vertices.points[vertex_index].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index].y = (u16)gte_out->vy + (u16)screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = quad_data[6] + (quad_data[8] << 8);
        direction->vy = 0;
        direction->vz = -(quad_data[4] + (quad_data[5] << 8));
    }
    else
    {
        direction->vx = quad_data[6] + (quad_data[8] << 8);
        direction->vy = 0;
        direction->vz = quad_data[4] + (quad_data[5] << 8);
    }
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    object->effect_vertices.points[vertex_index + 1].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index + 1].y = (u16)gte_out->vy + (u16)screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = quad_data[0xB] + (quad_data[0xC] << 8);
        direction->vy = 0;
        direction->vz = -(quad_data[9] + (quad_data[0xA] << 8));
    }
    else
    {
        direction->vx = quad_data[0xB] + (quad_data[0xC] << 8);
        direction->vy = 0;
        direction->vz = quad_data[9] + (quad_data[0xA] << 8);
    }
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    if (vertex_index == 0)
    {
        object->bounds.half.right = (u16)gte_out->vx;
        object->bounds.half.bottom = (u16)gte_out->vy;
    }
    object->effect_vertices.points[vertex_index + 2].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index + 2].y = (u16)gte_out->vy + (u16)screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = quad_data[0xF] + (quad_data[0x10] << 8);
        direction->vy = 0;
        direction->vz = -(quad_data[0xD] + (quad_data[0xE] << 8));
    }
    else
    {
        direction->vx = quad_data[0xF] + (quad_data[0x10] << 8);
        direction->vy = 0;
        direction->vz = quad_data[0xD] + (quad_data[0xE] << 8);
    }
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    object->effect_vertices.points[vertex_index + 3].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index + 3].y = (u16)gte_out->vy + (u16)screen_origin->y;
}
