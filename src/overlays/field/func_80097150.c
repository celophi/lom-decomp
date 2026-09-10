#include "common.h"
#include "sdk/libgte.h"

/** @brief Packed screen point with signed 16-bit coordinates. */
typedef union
{
    s32 packed;
    /** @brief Coordinate view of the packed word. */
    struct
    {
        s16 x;
        s16 y;
    } coord;
} Point;

/** @brief Actor index and packed point returned by the collision scan. */
typedef struct
{
    s32 actor;
    s32 point;
} Contact;

/** @brief Actor position and animation fields at their original 0x54-byte stride. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padc[0x21 - 12];
    u8 unk21;
    u8 pad22[3];
    u8 unk25;
    u8 pad26[4];
    s16 animation;
    u8 pad2c[14];
    u8 actor_index;
    u8 pad3b[0x54 - 0x3B];
} Record;

/** @brief Actor collision state and target list at their original 0x23C-byte stride. */
typedef struct
{
    u8 pad0[0x12E];
    s16 extent;
    u8 pad130[0x178 - 0x130];
    /** @brief Word flags and byte selectors share the same storage. */
    union
    {
        u32 flags;
        /** @brief Runtime ownership and active-target selectors. */
        struct
        {
            u8 byte0;
            u8 byte1;
            u8 slot;
            u8 count;
        } bytes;
    } status;
    u8 pad17c[4];
    u8 targets[0x23C - 0x180];
} State;

/** @brief Actor-slot ownership field at its original 0x244-byte stride. */
typedef struct
{
    u8 pad0[0x228];
    u8 owner_index;
    u8 pad229[0x244 - 0x229];
} ActorSlot;

/** @brief Animation request fields at their original 0x1C-byte stride. */
typedef struct
{
    s32 unk0;
    u8 pad4[8];
    s32 actor_index;
    u8 pad10[12];
} Request;

extern Record D_800FDF58[];
extern State D_80105AE0[];
extern ActorSlot g_field_actor_slots[];
extern Request D_80105880[];
extern s32 D_800FE754, D_8010D020;
void func_8008E690(Record *);
s32 func_800978AC(Point *, Point *, Point *, Point *);
#define READ_S16(p, o) (*(s16 *)((u8 *)(p) + (o)))
#define READ_U8(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define READ_U32(p, o) (*(u32 *)((u8 *)(p) + (o)))
#define READ_S32(p, o) (*(s32 *)((u8 *)(p) + (o)))

/**
 * @brief Test an actor quad against eligible actors and handle contact reactions.
 * @param quad Four packed screen-space vertices of the tested quadrilateral.
 * @param record Actor record supplying the owner and vertical position.
 * @param contact Output candidate index and packed contact point.
 * @return Zero for no contact, one or two for a collision layer, or three for a handled reaction.
 * @note A failed edge test also writes the 0x80008000 sentinel to contact->point.
 * @note The target's strict NormalClip signs and signed distance rounding are preserved.
 */
s32 func_80097150(Point *quad, Record *record, Contact *contact)
{
    Point *input_quad;
    Record *scan_record;
    Record *owner_record;
    u8 *scan_state;
    State *owner_state;
    /** @brief Centroid and unused bytes in the original 0x28-byte local workspace. */
    union
    {
        Point center;
        u8 storage[0x28];
    } scratch;
    s16 candidate_animation;
    s32 owner_extent;
    s32 candidate_extent;
    s32 candidate_flags;
    s32 requested_actor;
    s32 intersection;
    s32 candidate_mode;
    s32 centroid_layer;
    s32 eligible;
    s32 actor_index;
    s32 input_edge;
    s32 candidate_edge;
    s32 next_edge_offset;
    s32 list_index_or_layer;
    s32 edge_layer;
    s32 unused_flags;
    s32 request_offset;
    s32 matched_request_offset;
    s32 unused_layer_offset;
    s32 center_x_or_point;
    s32 center_y;
    s32 vertical_distance;
    u8 owner_index;
    u8 target_count;
    Point *candidate_quad;
    Record *initial_owner;
    State *target_state;
    State *reaction_state_40;
    State *reaction_state_3f;
    State *state_base;
    Request *request_base;
    ActorSlot *slot_base;
    Point *input_vertex;
    u8 *scan_mode;
    u8 *scan_extent;
    u8 *target_list_base;

    owner_index = record->actor_index;
    initial_owner = &D_800FDF58[owner_index];
    owner_record = initial_owner;
    if (initial_owner->animation == 0)
    {
        return 0;
    }
    goto initialize;
special_3f:
    scan_record->animation = 0x285;
    func_8008E690(scan_record);
    state_base = D_80105AE0;
    reaction_state_3f = &state_base[scan_record->actor_index];
    reaction_state_3f->status.flags = (s32)((reaction_state_3f->status.flags & ~0x1C) | 8);
    return 3;
special_40:
    scan_record->animation = 0x385;
    func_8008E690(scan_record);
    state_base = D_80105AE0;
    reaction_state_40 = &state_base[scan_record->actor_index];
    reaction_state_40->status.flags = (s32)((reaction_state_40->status.flags & ~0x1C) | 0x10);
    return 3;
initialize:
    scan_state = (u8 *)D_80105AE0;
    owner_state = (State *)(scan_state + owner_index * 0x23C);
    actor_index = 0;
    scan_record = D_800FDF58;
    scan_mode = (u8 *)D_800FDF58 + 0x21;
    scan_extent = scan_state + 0x12E;
scan_actor:
    if ((READ_U8(scan_mode, 0x4) != 0xFF) && !(READ_U32(scan_extent, -0x122) & 0x2280) &&
        ((actor_index < 3) || ((READ_U32(scan_extent, -0x11e) & 0xF) == D_800FE754)) &&
        (READ_U32(scan_extent, -0x12a) != 0) && ((u32)((READ_U8(scan_mode, 0x0) & 0x7F) - 0x38) >= 2U) &&
        (READ_U8(scan_mode, 0x19) != record->actor_index))
    {
        if (READ_U32(scan_extent, 0x4a) & 0x80)
        {
            eligible = 0;
            if (owner_record->animation == 0x91)
            {
                state_base = D_80105AE0;
                target_state = &state_base[owner_record->actor_index];
                target_count = target_state->status.bytes.count;
                list_index_or_layer = 0;
                if (target_count != 0)
                {
                    target_list_base = (u8 *)target_state;
                    do
                    {
                        if (READ_U8(target_list_base, 0x180 + list_index_or_layer) == actor_index)
                        {
                            goto eligible_candidate;
                        }
                        list_index_or_layer++;
                    } while (list_index_or_layer < target_count);
                }
            }
        }
        else
        {
        eligible_candidate:
            eligible = 1;
        }
        if ((eligible != 0) && !(READ_U32(scan_extent, 0x46) & 0x8000))
        {
            candidate_flags = READ_U32(scan_extent, 0x4a);
            slot_base = g_field_actor_slots;
            if (!(candidate_flags & 0x20) &&
                (!(candidate_flags & 1) ||
                 (slot_base[READ_U8(scan_extent, 0x4c)].owner_index == record->actor_index)) &&
                !(candidate_flags & 2))
            {
                if (!(candidate_flags & 0x40))
                {
                    if ((u8)READ_U8(scan_mode, 0x19) < 2U)
                    {
                        request_offset = READ_U8(scan_mode, 0x19) * 0x1C;
                    }
                    else
                    {
                        request_offset = 0x38;
                    }
                    request_base = D_80105880;
                    requested_actor = ((Request *)((u8 *)request_base + request_offset))->actor_index;
                    if (requested_actor == READ_U8(scan_mode, 0x19))
                    {
                        if ((u32)(requested_actor & 0xFF) < 2U)
                        {
                            matched_request_offset = requested_actor * 0x1C;
                        }
                        else
                        {
                            matched_request_offset = 0x38;
                        }
                        request_base = D_80105880;
                        if (((Request *)((u8 *)request_base + matched_request_offset))->unk0 == 0)
                        {
                            goto check_animation;
                        }
                        goto next_actor;
                    }
                    goto check_animation;
                }
            check_animation:
                candidate_animation = READ_S16(scan_mode, 0x9);
                if ((candidate_animation != 0x91) && (candidate_animation != 0x87) &&
                    (candidate_animation != 0xAE))
                {
                    if (D_8010D020 == 0)
                    {
                        if ((u8)record->actor_index < 3U)
                        {
                            if ((u8)READ_U8(scan_mode, 0x19) < 3U)
                            {
                                actor_index += 1;
                                goto advance_actor;
                            }
                            goto check_height;
                        }
                        if ((u8)READ_U8(scan_mode, 0x19) < 3U)
                        {
                            goto opposing_group;
                        }
                        goto next_actor;
                    }
                opposing_group:
                check_height:
                    vertical_distance = (READ_S32(scan_mode, -0x19) - record->unk8) / 224;
                    if (vertical_distance < 0)
                    {
                        vertical_distance = -vertical_distance;
                    }
                    candidate_extent = READ_S16(scan_extent, 0x0);
                    owner_extent = owner_state->extent;
                    if (candidate_extent < 0)
                    {
                        candidate_extent = -candidate_extent;
                    }
                    if (owner_extent < 0)
                    {
                        owner_extent = -owner_extent;
                    }
                    owner_extent += candidate_extent;
                    vertical_distance = vertical_distance < owner_extent;
                    if (vertical_distance)
                    {
                        list_index_or_layer = 4;
                        input_quad = quad;

                    scan_layer:
                        candidate_edge = 0;
                        candidate_quad = (Point *)(scan_state + (list_index_or_layer * 4 + 0x148));
                        do
                        {
                            input_edge = 0;
                            next_edge_offset = ((candidate_edge + 1) & 3) * 4;
                            input_vertex = input_quad;
                            do
                            {
                                if ((candidate_quad[0].packed != 0) || (candidate_quad[1].packed != 0))
                                {
                                    intersection =
                                        func_800978AC(input_vertex, input_quad + ((input_edge + 1) & 3),
                                                      candidate_quad + candidate_edge,
                                                      (Point *)((u8 *)candidate_quad + next_edge_offset));
                                    contact->point = intersection;
                                    if (intersection != 0x80008000)
                                    {
                                        if ((u8)READ_U8(scan_mode, 0x19) < 2U)
                                        {
                                            candidate_mode = READ_U8(scan_mode, 0x0) & 0x7F;
                                            if (candidate_mode != 0x3F)
                                            {
                                                if (candidate_mode != 0x40)
                                                {
                                                    goto edge_contact;
                                                }
                                                goto special_40;
                                            }
                                            goto special_3f;
                                        }
                                    edge_contact:
                                        edge_layer = list_index_or_layer;
                                        contact->actor = actor_index;
                                        if (list_index_or_layer < 0)
                                        {
                                            edge_layer = list_index_or_layer + 3;
                                        }
                                        return (edge_layer >> 2) + 1;
                                    }
                                }
                                input_edge += 1;
                                input_vertex++;
                            } while (input_edge < 4);
                            candidate_edge += 1;
                        } while (candidate_edge < 4);
                        if ((candidate_quad[0].packed != 0) || (candidate_quad[1].packed != 0))
                        {
                            center_x_or_point =
                                quad[0].coord.x + quad[1].coord.x + quad[2].coord.x + quad[3].coord.x;
                            if (center_x_or_point < 0)
                            {
                                center_x_or_point += 3;
                            }
                            scratch.center.coord.x = (u16)(center_x_or_point >> 2);
                            center_y = quad[0].coord.y + quad[1].coord.y + quad[2].coord.y + quad[3].coord.y;
                            if (center_y < 0)
                            {
                                center_y += 3;
                            }
                            scratch.center.coord.y = (s16)(center_y >> 2);
                            if (NormalClip(scratch.center.packed, candidate_quad[0].packed,
                                           candidate_quad[1].packed) < 0)
                            {
                                if ((NormalClip(scratch.center.packed, candidate_quad[1].packed,
                                                candidate_quad[2].packed) < 0) &&
                                    (NormalClip(scratch.center.packed, candidate_quad[2].packed,
                                                candidate_quad[3].packed) < 0))
                                {
                                    if (NormalClip(scratch.center.packed, candidate_quad[3].packed,
                                                   candidate_quad[0].packed) >= 0)
                                    {
                                        goto next_layer;
                                    }
                                    goto centroid_contact;
                                }
                                goto next_layer;
                            }
                            if ((NormalClip(scratch.center.packed, candidate_quad[1].packed,
                                            candidate_quad[2].packed) > 0) &&
                                (NormalClip(scratch.center.packed, candidate_quad[2].packed,
                                            candidate_quad[3].packed) > 0))
                            {
                                if (NormalClip(scratch.center.packed, candidate_quad[3].packed,
                                               candidate_quad[0].packed) > 0)
                                {
                                centroid_contact:
                                    centroid_layer = list_index_or_layer;
                                    center_x_or_point =
                                        (s32)((u16)scratch.center.coord.x | (scratch.center.coord.y << 16));
                                    contact->actor = actor_index;
                                    contact->point = center_x_or_point;
                                    if (list_index_or_layer < 0)
                                    {
                                        centroid_layer = list_index_or_layer + 3;
                                    }
                                    return (centroid_layer >> 2) + 1;
                                }
                            }
                            goto next_layer;
                        }
                    next_layer:
                        list_index_or_layer -= 4;

                        if (list_index_or_layer < 0)
                        {
                            goto next_actor;
                        }
                        goto scan_layer;
                    }
                    goto next_actor;
                }
                goto next_actor;
            }
        }
        goto next_actor;
    }
next_actor:
    actor_index += 1;
advance_actor:
    scan_mode += 0x54;
    scan_extent += 0x23C;
    scan_record++;
    scan_state += 0x23C;
    if (actor_index >= 0xD)
    {
        return 0;
    }
    goto scan_actor;
}
