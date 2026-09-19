/** @file field_effect_geometry.c
 * @brief Project actor-attached effect geometry and update its per-slot screen-space caches.
 */

#include "common.h"
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

extern FieldMotionRecord D_800FDF58[];
extern FieldMotionRecord g_field_effect_records[];
extern FieldObjectPlacement D_80105AE0[];
extern s32 g_field_track_index;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;


void func_8007E5FC(s16 *out, s32 mirror, u8 *item);

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
    FieldObjectPlacement *object;
    s32 value;
    s32 index;

    anchor_mode = part->placement_flags >> FIELD_PART_ANCHOR_MODE_SHIFT;
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
            object_record = &D_800FDF58[object_index];
            object = &D_80105AE0[object_index];
        }
        else
        {
            object_index = actor->owner_object_index;
            object_record = &D_800FDF58[object_index];
            object = &D_80105AE0[object_index];
        }
        if ((part->placement_flags >> FIELD_PART_SCALE_X_FROM_BOUNDS_SHIFT) & 1)
        {
            part->footprint_scale_x = (*(u8 *)&object->bounds_right - *(u8 *)&object->bounds_left) * 2;
        }
        if ((part->placement_flags >> FIELD_PART_SCALE_Y_FROM_BOUNDS_SHIFT) & 1)
        {
            index = 0;
            part->footprint_scale_y = (*(u8 *)&object->bounds_bottom - *(u8 *)&object->bounds_top) * 2;
        }
        else
        {
            index = 0;
        }
        value = index;
        switch (anchor_mode)
        {
        case 1:
            value = (object->bounds_right + object->bounds_left) >> 1;
            index = (object->bounds_bottom + object->bounds_top) >> 1;
            break;
        case 2:
            value = (object->bounds_right + object->bounds_left) >> 1;
            index = 0;
            break;
        case 3:
            value = (object->bounds_right + object->bounds_left) >> 1;
            index = object->bounds_top;
            break;
        case 4:
            value = object->bounds_left;
            index = (object->bounds_bottom + object->bounds_top) >> 1;
            break;
        case 5:
            value = object->bounds_right;
            index = (object->bounds_bottom + object->bounds_top) >> 1;
            break;
        case 6:
            value = object->bounds_left;
            index = object->bounds_top;
            break;
        case 7:
            value = object->bounds_right;
            index = object->bounds_top;
            break;
        case 8:
            value = object->bounds_left;
            index = object->bounds_bottom;
            break;
        case 9:
            value = object->bounds_right;
            index = object->bounds_bottom;
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
        out->x = (part->offset_x << 8) - D_800F22A0;
        out->y = (part->offset_y << 8) - D_800F22A4;
        out->z = (part->offset_z << 8) - D_800F22A8;
        return;
    case 0x1C:
        out->x = -D_800F22A0;
        out->y = -D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x1D:
        out->y = -0x7000;
        out->x = -D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x1E:
        out->y = 0x7000;
        out->x = -D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x1F:
        out->x = 0xFFFF6000;
        out->x -= D_800F22A0;
        out->y = -D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x20:
        out->x = 0xA000;
        out->x -= D_800F22A0;
        out->y = -D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x21:
        out->x = 0xFFFF6000;
        out->y = -0x7000;
        out->x -= D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x22:
        out->x = 0xA000;
        out->y = -0x7000;
        out->x -= D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x23:
        out->x = 0xFFFF6000;
        out->y = 0x7000;
        out->x -= D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x24:
        out->x = 0xA000;
        out->y = 0x7000;
        out->x -= D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x26:
        return;

    case 0x27:
        object_record = &D_800FDF58[actor->owner_object_index];
        if ((((part->placement_flags >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) ||
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
        object_record = &D_800FDF58[actor->track_object_indices[g_field_track_index]];
        if ((part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER) &&
            !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
        {
            out->x = object_record->x - (part->offset_x << 8);
        }
        else if (((part->placement_flags >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
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
        object_record = &D_800FDF58[object_index];
        object = &D_80105AE0[object_index];
        out->x = object_record->x +
                 (object->attachment_points[((u32)part->effect_flags >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].x << 8);
        out->y = object_record->y +
                 (object->attachment_points[((u32)part->effect_flags >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].y << 8);
        out->z = object_record->z;
        return;

    case 0x32:
        object_index = actor->owner_object_index;
        object_record = &D_800FDF58[object_index];
        object = &D_80105AE0[object_index];
        out->x = object_record->x +
                 (object->attachment_points[((u32)part->effect_flags >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].x << 8);
        out->y = object_record->y;
        out->z = object_record->z + (part->offset_z << 8);
        if (((part->placement_flags >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) && !(object_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
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
        object_record = &D_800FDF58[object_index];
        object = &D_80105AE0[object_index];
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
                    func_8007E5FC(corners, effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED, frame_data);
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
void field_transform_effect_quad_vertices8(FieldMotionRecord *effect, FieldObjectPlacement *object, u8 *quad_data, s32 vertex_index,
                                           Vec2s *screen_origin, SVECTOR *direction, FieldVector *gte_out)
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
        object->bounds_left = (u16)gte_out->vx;
        object->bounds_top = (u16)gte_out->vy - (s8)effect->unknown_0x37;
    }
    object->effect_vertices[vertex_index].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices[vertex_index].y = (u16)gte_out->vy + (u16)screen_origin->y;

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
    object->effect_vertices[vertex_index + 1].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices[vertex_index + 1].y = (u16)gte_out->vy + (u16)screen_origin->y;

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
        object->bounds_right = (u16)gte_out->vx;
        object->bounds_bottom = (u16)gte_out->vy - (s8)effect->unknown_0x37;
    }
    object->effect_vertices[vertex_index + 2].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices[vertex_index + 2].y = (u16)gte_out->vy + (u16)screen_origin->y;

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
    object->effect_vertices[vertex_index + 3].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices[vertex_index + 3].y = (u16)gte_out->vy + (u16)screen_origin->y;
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
void field_transform_effect_quad_vertices16(FieldMotionRecord *effect, FieldObjectPlacement *object, u8 *quad_data, s32 vertex_index,
                                            Vec2s *screen_origin, SVECTOR *direction, FieldVector *gte_out)
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
        object->bounds_left = (u16)gte_out->vx;
        object->bounds_top = (u16)gte_out->vy;
    }
    object->effect_vertices[vertex_index].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices[vertex_index].y = (u16)gte_out->vy + (u16)screen_origin->y;

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
    object->effect_vertices[vertex_index + 1].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices[vertex_index + 1].y = (u16)gte_out->vy + (u16)screen_origin->y;

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
        object->bounds_right = (u16)gte_out->vx;
        object->bounds_bottom = (u16)gte_out->vy;
    }
    object->effect_vertices[vertex_index + 2].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices[vertex_index + 2].y = (u16)gte_out->vy + (u16)screen_origin->y;

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
    object->effect_vertices[vertex_index + 3].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices[vertex_index + 3].y = (u16)gte_out->vy + (u16)screen_origin->y;
}
