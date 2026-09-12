#include "common.h"
#include "field_effect_types.h"

void func_8008A840(s32, s32);
void func_8008A9D8(s32, s32, s32);
void func_8008BC5C(FieldMotionRecord *);
void func_800A2DD8(s32);
extern FieldMotionRecord D_800FDF58[];
extern u8 D_80105880[], D_80105AE0[];
extern s32 D_800FE754, D_8010D020;
/**
 * @brief Collect new effect-centered hit contacts and dispatch their reactions.
 * @param effect Motion record supplying the hit origin and source object index.
 * @param radius Expansion of candidate projected X/Y bounds and the depth gate.
 * @param actor Owner whose target tracks, contact offsets, and reaction selector are updated.
 * @note New targets append to track_object_indices, set active_track_mask, and
 * increase track_count. Eligible records already in that list are skipped.
 * @note Candidate byte cursors preserve packed flag accesses and the current
 * compiler's repeated-read behavior. This function does not directly subtract HP.
 */
void field_collect_effect_hits(FieldMotionRecord *effect, s32 radius, FieldActorState *actor)
{
    s32 bound_x_a;
    s32 bound_y_a;
    s16 candidate_kind;
    s32 bound_x_b;
    s32 bound_y_b;
    s32 min_y;
    s32 max_y;
    s32 min_x;
    s32 max_x;
    s32 *candidate_position;
    s32 *candidate_record;
    s32 origin_y;
    s32 candidate_flags;
    s32 controlled_object;
    s32 origin_x;
    s32 delta_z;
    s32 projected_min_y;
    s32 projected_max_y;
    s32 origin_z;
    s32 depth_projection;
    s32 candidate_z_value;
    s32 candidate_x;
    s32 candidate_y;
    s32 contact_origin_x;
    s32 eligible_for_contact;
    s32 end_index;
    s32 suppression_flags;
    s32 controller_offset;
    s32 active_controller_offset;
    s32 rounded_delta_z;
    s32 contact_offset;
    s32 existing_hit_index;
    s32 existing_contact_index;
    s32 depth_distance;
    s32 contact_target_x;
    s32 candidate_index;
    u8 source_object_index;
    u8 previous_hit_count;
    u8 owner_index;
    u8 contact_count;
    u8 reaction_selector;
    u8 previous_state;
    u8 *track_offset_address;
    u8 *source_object;
    u8 *append_object;
    u8 *count_object;
    u8 *candidate_flags_address;
    u8 *candidate_depth_address;
    u8 *prior_hit_cursor;
    u8 *contact_cursor;

    /* Choose party/nonparty candidates from the owner and resource flags. */
    if (D_8010D020 != 0)
    {
        candidate_index = 0;
        end_index = 0xD;
    }
    else
    {
        if (actor->animation->sync_flags & 1)
        {
            candidate_index = 0;
            if ((u8)actor->owner_object_index >= 3U)
            {
                candidate_index = 3;
                goto scan_nonparty;
            }
            goto scan_party;
        }
        candidate_index = 3;
        if ((u8)actor->owner_object_index < 3U)
        {
        scan_nonparty:
            end_index = 0xD;
        }
        else
        {
            candidate_index = 0;
        scan_party:
            end_index = 3;
        }
    }
    candidate_position = (s32 *)((u8 *)D_800FDF58 + candidate_index * 0x54);
    candidate_flags_address = (candidate_index * 0x23C) + D_80105AE0;
    if (candidate_index < end_index)
    {
        candidate_flags_address += 0xC;
        candidate_depth_address = (u8 *)candidate_position + 8;
        candidate_record = candidate_position;
    next_candidate:
    {
        if (*(s32 *)(candidate_flags_address + (364)) & 0x80)
        {
            source_object_index = effect->source_object_index;
            eligible_for_contact = 0;
            if (D_800FDF58[source_object_index].motion_parameter == 0x91)
            {
                source_object = (source_object_index * 0x23C) + D_80105AE0;
                previous_hit_count = *(u8 *)(source_object + (379));
                existing_hit_index = 0;
                if (previous_hit_count != 0)
                {
                find_prior_hit:
                    prior_hit_cursor = source_object + existing_hit_index;
                    existing_hit_index += 1;
                    if (*(u8 *)(prior_hit_cursor + (384)) != candidate_index)
                    {
                        if (existing_hit_index >= (s32)previous_hit_count)
                        {
                        }
                        else
                        {
                            goto find_prior_hit;
                        }
                    }
                    else
                    {
                        goto contact_allowed;
                    }
                }
            }
        }
        else
        {
        contact_allowed:
            eligible_for_contact = 1;
        }
        owner_index = actor->owner_object_index;
        if ((candidate_index != owner_index) &&
            ((*(s32 *)(candidate_flags_address + (316)) != 0) || (*(s32 *)(candidate_flags_address + (324)) != 0)) &&
            ((*(s32 *)(candidate_flags_address + (308)) != 0) || (*(s32 *)(candidate_flags_address + (312)) != 0)) &&
            (*(s32 *)(candidate_flags_address + (288)) != 0) && (candidate_kind = *(s16 *)(candidate_depth_address + (34)), (candidate_kind != 0x91)) &&
            (candidate_kind != 0xAE) && (candidate_kind != 0x87) && (*(u8 *)(candidate_depth_address + (29)) != 0xFF) &&
            (owner_index != candidate_index) && (*(s32 *)(candidate_flags_address + (-8)) != 0) &&
            (candidate_flags = *(s32 *)(candidate_flags_address + (364)), ((candidate_flags & 1) == 0)) &&
            ((suppression_flags = candidate_flags & 0x20, ((candidate_index < 3) != 0)) ||
             (suppression_flags = candidate_flags & 0x20, ((*(s32 *)(candidate_flags_address + (4)) & 0xF) == D_800FE754))) &&
            (suppression_flags == 0) && (eligible_for_contact != 0) && !(*(s32 *)(candidate_flags_address + (360)) & 0x8000))
        {
            if (!(candidate_flags & 0x40))
            {
                if ((u8) * (u8 *)(candidate_depth_address + (50)) < 2U)
                {
                    controller_offset = *(u8 *)(candidate_depth_address + (50)) * 0x1C;
                }
                else
                {
                    controller_offset = 0x38;
                }
                controlled_object = *(s32 *)(D_80105880 + controller_offset + 0xC);
                if (controlled_object == *(u8 *)(candidate_depth_address + (50)))
                {
                    if ((u32)(controlled_object & 0xFF) < 2U)
                    {
                        active_controller_offset = controlled_object * 0x1C;
                    }
                    else
                    {
                        active_controller_offset = 0x38;
                    }
                    if (*(s32 *)(D_80105880 + active_controller_offset) == 0)
                    {
                        goto check_unique_contact;
                    }
                    goto advance_candidate;
                }
                goto check_unique_contact;
            }
        check_unique_contact:
            if (!(*(s32 *)(candidate_flags_address + (0)) & 0x2280))
            {
                contact_count = actor->track_count;
                existing_contact_index = 0;
                if (contact_count != 0)
                {
                find_existing_contact:
                    contact_cursor = (u8 *)actor + existing_contact_index;
                    if (candidate_index != *(u8 *)(contact_cursor + (553)))
                    {
                        existing_contact_index += 1;
                        if (existing_contact_index < (s32)contact_count)
                        {
                            goto find_existing_contact;
                        }
                    }
                }
                if (existing_contact_index == actor->track_count)
                {
                    /* Depth gating precedes expanded projected X/Y bounds. */
                    candidate_z_value = *(s32 *)(candidate_depth_address + (0));
                    origin_z = effect->z;
                    delta_z = candidate_z_value - origin_z;
                    depth_distance = (candidate_z_value - origin_z) / 384;
                    if (depth_distance < 0)
                    {
                        depth_distance = -depth_distance;
                    }
                    if (depth_distance < (radius + ((s32)(*(u16 *)(candidate_flags_address + (290)) << 0x10) >> 0x11)))
                    {
                        bound_x_a = *(s16 *)(candidate_flags_address + 0x134);
                        bound_x_b = *(s16 *)(candidate_flags_address + 0x138);
                        if (bound_x_a < bound_x_b)
                        {
                            min_x = bound_x_a;
                            max_x = bound_x_b;
                        }
                        else
                        {
                            min_x = bound_x_b;
                            max_x = bound_x_a;
                        }
                        bound_y_a = *(s16 *)(candidate_flags_address + (310));
                        bound_y_b = *(s16 *)(candidate_flags_address + (314));
                        if (bound_y_a < bound_y_b)
                        {
                            min_y = bound_y_a;
                            max_y = bound_y_b;
                        }
                        else
                        {
                            min_y = bound_y_b;
                            max_y = bound_y_a;
                        }
                        min_x -= radius;
                        max_x += radius;
                        min_y -= radius;
                        max_y += radius;
                        rounded_delta_z = delta_z;
                        if (delta_z < 0)
                        {
                            rounded_delta_z = delta_z + 0x1FF;
                        }
                        depth_projection = (s32)((rounded_delta_z >> 9) + ((u32)rounded_delta_z >> 0x1F)) >> 1;
                        projected_min_y = min_y - depth_projection;
                        projected_max_y = max_y - depth_projection;
                        candidate_x = *candidate_position;
                        origin_x = effect->x;
                        if (((candidate_x + (min_x << 8)) < origin_x) && (origin_x < (candidate_x + (max_x << 8))) &&
                            (candidate_y = *(s32 *)(candidate_depth_address + (-4)), origin_y = effect->y,
                             (((candidate_y + (projected_min_y << 8)) < origin_y) != 0)) &&
                            (origin_y < (candidate_y + (projected_max_y << 8))) &&
                            ((u8) * (u8 *)(D_80105AE0 + actor->owner_object_index * 0x23C + 0x17B) < 9U))
                        {
                            *(s32 *)(candidate_flags_address + (364)) = (s32)(*(s32 *)(candidate_flags_address + (364)) | 0x80);
                            *(s32 *)(candidate_flags_address + (0)) = (s32)(*(s32 *)(candidate_flags_address + (0)) & ~0x400);
                            append_object = (actor->owner_object_index * 0x23C) + D_80105AE0;
                            *(u8 *)(append_object + append_object[0x17B] + 0x180) = candidate_index;
                            count_object = (actor->owner_object_index * 0x23C) + D_80105AE0;
                            *(u8 *)(count_object + (379)) = (u8)(*(u8 *)(count_object + (379)) + 1);
                            /* Each new contact becomes an active target track. */
                            actor->active_track_mask = (u8)(actor->active_track_mask | (1 << actor->track_count));
                            actor->track_object_indices[actor->track_count] = candidate_index;
                            if (*(u8 *)(candidate_depth_address + (25)) & 0x80)
                            {
                                contact_target_x = *candidate_position;
                                contact_origin_x = effect->x;
                                contact_offset = actor->track_count * 4;
                            }
                            else
                            {
                                contact_target_x = effect->x;
                                contact_origin_x = *candidate_position;
                                contact_offset = actor->track_count * 4;
                            }
                            *(s16 *)((u8 *)actor + contact_offset + 0x1FE) = (s16)((s32)(contact_target_x - contact_origin_x) >> 8);
                            track_offset_address = (u8 *)actor + (actor->track_count * 4);
                            *(s16 *)(track_offset_address + (512)) = (s16)(((s32)(effect->y - *(s32 *)(candidate_depth_address + (-4))) >> 8) -
                                                                ((s32)(effect->z - *(s32 *)(candidate_depth_address + (0))) >> 9));
                            /* Retirement preserves the previous state in byte +0x26. */
                            if (effect->flags & FIELD_EFFECT_RETIRE_ON_HIT)
                            {
                                previous_state = effect->state;
                                effect->state = FIELD_EFFECT_RETIRED;
                                effect->height_or_retired_state = previous_state;
                            }
                            actor->track_count = (u8)(actor->track_count + 1);
                            func_8008BC5C((FieldMotionRecord *)candidate_record);
                            if ((candidate_index < 2) && !(((FieldMotionRecord *)candidate_record)->flags & 0x1FF))
                            {
                                func_800A2DD8(candidate_index);
                            }
                            if (((u8)actor->hit_reaction < 0xCU) || (D_800FDF58[actor->owner_object_index].motion_parameter == 0xBC))
                            {
                                func_8008A9D8(actor->owner_object_index, candidate_index, actor->hit_reaction);
                                candidate_record += 21;
                            }
                            else
                            {
                                reaction_selector = actor->hit_reaction;
                                switch (reaction_selector)
                                {
                                case 0x34:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x16U);
                                    candidate_record += 21;
                                    break;
                                case 0x50:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x12U);
                                    candidate_record += 21;
                                    break;
                                case 0x51:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x13U);
                                    candidate_record += 21;
                                    break;
                                case 0x4E:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x14U);
                                    candidate_record += 21;
                                    break;
                                case 0x4F:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x15U);
                                    candidate_record += 21;
                                    break;
                                case 0x3E:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x19U);
                                    candidate_record += 21;
                                    break;
                                case 0x45:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x1AU);
                                    candidate_record += 21;
                                    break;
                                default:
                                    func_8008A840(actor->owner_object_index, candidate_index);
                                    goto advance_candidate;
                                }
                            }
                        }
                        else
                        {
                            goto advance_candidate;
                        }
                    }
                    else
                    {
                        goto advance_candidate;
                    }
                }
                else
                {
                    goto advance_candidate;
                }
            }
            else
            {
                goto advance_candidate;
            }
        }
        else
        {
        advance_candidate:
            candidate_record += 21;
        }
        candidate_index += 1;
        candidate_depth_address += 0x54;
        candidate_position += 21;
        candidate_flags_address += 0x23C;
    }
        if (candidate_index < end_index)
        {
            goto next_candidate;
        }
    }
}
