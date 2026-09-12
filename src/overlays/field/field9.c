/**
 * @file field9.c
 * @brief Field actor slot helpers carved from the unk2 segment.
 */

#include "common.h"
#include "field_effect_types.h"


extern FieldActorState g_field_actor_slots[80];
extern FieldMotionRecord D_800FDF58[];
extern FieldMotionRecord D_800FF658[];
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
 * @see decomp.me (100%)
 */
void field_release_actor_if_no_effects(FieldMotionRecord *effect)
{
    FieldActorState *slots;
    FieldActorState *slot;

    if (field_actor_has_live_effects(effect->actor_index) == 0)
    {
        slots = g_field_actor_slots;
        slot = &slots[effect->actor_index];
        slot->is_active = 0;
        slot->unknown_0x23b = 0;
        slot->active_track_mask = 0;
    }
}

/**
 * @brief Check the effect pool for an unretired record owned by an actor.
 * @param actor_index Actor slot index to compare with each record's owner.
 * @return 1 if a matching live effect exists, otherwise 0.
 * @see decomp.me (100%)
 */
s32 field_actor_has_live_effects(s32 actor_index)
{
    s32 effect_index;
    s32 retired_state;
    FieldMotionRecord *effect;

    effect_index = 0;
    retired_state = FIELD_EFFECT_RETIRED;
    effect = D_800FF658;
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
 * @see decomp.me (100%)
 */
void field_set_action_context(s32 recipient_id, s32 source_id, s32 action)
{
    g_field_action_context = (recipient_id << 0x10) | (source_id << 8) | action;
}

/**
 * @brief Resolve an effect's selected target/attachment position in world coordinates.
 * @param rec Record selecting a source and supplying saved positions or effect links.
 * @param part Part definition controlling anchor choice, placement, and facing updates.
 * @param out Destination X/Y/Z; its pad word is untouched.
 * @note Sources 0 and values above 15 leave out unchanged. Source 9 can update
 * the record's facing bit. References at +0x30 use an unsigned halfword read.
 * @see decomp.me (100%)
 */
void field_resolve_effect_position(FieldMotionRecord *rec, FieldActorPartDef *part, FieldVector *out)
{
    FieldMotionRecord *source_record;
    FieldMotionRecord *opposite_record;
    FieldMotionRecord *track_record;
    FieldMotionRecord *owner_record;
    FieldMotionRecord *linked_record;
    FieldObjectPlacement *source_object;
    FieldActorState *slots;
    FieldActorState *owner_slots;
    s32 object_index;
    s32 source_index;
    s32 owner_index;
    s32 placement;
    s32 delta_x;
    s32 position_x;
    s32 x_term;
    s32 offset_x;
    s32 dispatch;
    static void *const dispatch_table[] = {
        &&track_object, &&owner_object, &&saved, &&reference_effect, &&owner_attachment, &&facing_offset, &&track_stored_xz, &&linked_effect, &&relative_side_offset, &&owner_bounds_center, &&track_bounds_center, &&reflect_track_x, &&reflect_track_x, &&extend_track_xz, &&extend_link_xz, 0
    };

    switch (0)
    {
    case 0:
        dispatch = rec->position_source - 1;
        if ((u32) dispatch >= 0xFU)
        {
            break;
        }
        goto *dispatch_table[dispatch];

    track_object:
        slots = g_field_actor_slots;
        out->vx = D_800FDF58[slots[rec->actor_index].track_object_indices[g_field_track_index]].x;
        out->vy = D_800FDF58[slots[rec->actor_index].track_object_indices[g_field_track_index]].y;
        object_index = slots[rec->actor_index].track_object_indices[g_field_track_index];
        do
        {
            track_record = &D_800FDF58[object_index];
        } while (0);
        out->vz = track_record->z;
        return;
    owner_object:
        owner_slots = g_field_actor_slots;
        out->vx = D_800FDF58[owner_slots[rec->actor_index].owner_object_index].x;
        out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].y;
        owner_index = owner_slots[rec->actor_index].owner_object_index;
        do
        {
            owner_record = &D_800FDF58[owner_index];
        } while (0);
        out->vz = owner_record->z;
        return;
    saved:
        out->vx = rec->work_x - D_800F22A0;
        out->vy = rec->work_y - D_800F22A4;
        out->vz = rec->work_z - D_800F22A8;
        return;
    reference_effect:
        if (D_800FF658[(u16) rec->reference_index].state != FIELD_EFFECT_RETIRED)
        {
            out->vx = D_800FF658[(u16) rec->reference_index].x;
            out->vy = D_800FF658[(u16) rec->reference_index].y;
            out->vz = D_800FF658[(u16) rec->reference_index].z;
            return;
        }
        out->vx = rec->x;
        out->vy = rec->y;
        out->vz = rec->z;
        return;
    owner_attachment:
        source_record = &D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index];
        source_object = &D_80105AE0[g_field_actor_slots[rec->actor_index].owner_object_index];
        out->vx = source_record->x + (source_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8);
        out->vy = source_record->y + (source_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].y << 8);
        out->vz = source_record->z;
        return;
    facing_offset:
        placement = ((u32)part->placement_flags >> 0x12) & 0x3F;
        if (placement >= 0xA)
        {
            if (placement < 0x26)
            {
                source_index = g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index];
            }
            else
            {
                source_index = g_field_actor_slots[rec->actor_index].owner_object_index;
            }
        }
        else
        {
            source_index = g_field_actor_slots[rec->actor_index].owner_object_index;
        }
        source_record = &D_800FDF58[source_index];
        if (source_record->facing_or_reward_kind & 0x80)
        {
            position_x = rec->work_x;
            x_term = source_record->x;
            position_x = position_x + x_term;
        }
        else
        {
            x_term = rec->work_x;
            position_x = -x_term;
            position_x += source_record->x;
        }
        out->vx = position_x;
        out->vy = rec->work_y + source_record->y;
        out->vz = rec->work_z + source_record->z;
        return;
    track_stored_xz:
        out->vx = D_80105AE0[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].unknown_0x6c << 8;
        out->vz = D_80105AE0[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].unknown_0x6e << 8;
        out->vy = 0;
        return;
    linked_effect:
        out->vx = D_800FF658[rec->position_data.linked_effect_index].x;
        out->vy = D_800FF658[rec->position_data.linked_effect_index].y;
        linked_record = &D_800FF658[rec->position_data.linked_effect_index];
        out->vz = linked_record->z;
        return;
    relative_side_offset:
        placement = ((u32)part->placement_flags >> 0x12) & 0x3F;
        if (placement < 0xA)
        {
            source_record = &D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index];
            opposite_record = &D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]];
        }
        else
        {
            if (placement < 0x26)
            {
                object_index = g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index];
                source_record = &D_800FDF58[object_index];
            }
            else
            {
                object_index = g_field_actor_slots[rec->actor_index].owner_object_index;
                source_record = &D_800FDF58[object_index];
            }
            opposite_record = &D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index];
        }
        if (source_record->x > opposite_record->x)
        {
            if (((part->placement_flags >> 0xA) & 1) || (part->spawn_flags.word & 0x08000000))
            {
                rec->facing_or_reward_kind = rec->facing_or_reward_kind & 0x7F;
            }
            offset_x = rec->work_x;
            position_x = source_record->x - offset_x;
        }
        else
        {
            if (((part->placement_flags >> 0xA) & 1) || (part->spawn_flags.word & 0x08000000))
            {
                rec->facing_or_reward_kind = rec->facing_or_reward_kind | 0x80;
            }
            position_x = rec->work_x;
            x_term = source_record->x;
            position_x = position_x + x_term;
        }
        out->vx = position_x;
        out->vy = rec->work_y + source_record->y;
        out->vz = rec->work_z + source_record->z;
        return;
    owner_bounds_center:
        source_object = &D_80105AE0[g_field_actor_slots[rec->actor_index].owner_object_index];
        out->vx = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].x + ((source_object->bounds_right + source_object->bounds_left) << 7);
        out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].y + ((source_object->bounds_bottom + source_object->bounds_top) << 7);
        out->vz = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].z;
        return;
    track_bounds_center:
        source_object = &D_80105AE0[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]];
        out->vx = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].x + ((source_object->bounds_right + source_object->bounds_left) << 7);
        out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].y + ((source_object->bounds_bottom + source_object->bounds_top) << 7);
        out->vz = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].z;
        return;
    reflect_track_x:
        delta_x = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].x - D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].x;
        if (rec->position_source == FIELD_POSITION_REFLECT_TRACK_X)
        {
            out->vx = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].x - delta_x;
        }
        else
        {
            out->vx = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].x + delta_x;
        }
        out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].y;
        out->vz = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].z;
        return;
    extend_track_xz:
        out->vx = (D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].x * 2) - D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].x;
        out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].y;
        out->vz = (D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].z * 2) - D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].z;
        return;
    extend_link_xz:
        out->vx = (D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].x * 2) - D_800FF658[rec->position_data.linked_effect_index].x;
        out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].y;
        out->vz = (D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].z * 2) - D_800FF658[rec->position_data.linked_effect_index].z;
        return;
    }
}
