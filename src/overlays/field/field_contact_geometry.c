#include "common.h"
#include "field_types.h"
#include "field_effect_types.h"
#include "vector.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/*
 * field_contact_geometry - merged translation unit.
 *
 * Consolidates the actor contact/collision-geometry functions in the FIELD
 * overlay vram range 0x800970B0 .. 0x8009A2A4 into one TU. Function bodies are
 * preserved verbatim from their original per-function sources; only the shared
 * declaration environment has been reconciled:
 *   - Parameter record types that two functions defined under the same name
 *     ("Record") are kept distinct at file scope (ContactRecord, ReactRecord).
 *   - Externs whose type differs per function (D_800FDF58, D_80105AE0,
 *     D_80105880, g_field_resource_entries, ...) are declared at block scope
 *     inside each using function with that function's original type.
 *   - Forward calls to members defined later use file-scope prototypes with the
 *     callee's real signature; pointer-type mismatches at call sites are
 *     codegen-identical.
 */

/* Shared byte/halfword/word accessors (union of the per-file macro sets). */
#define READ_S16(p, o) (*(s16 *)((u8 *)(p) + (o)))
#define READ_U16(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define READ_S32(p, o) (*(s32 *)((u8 *)(p) + (o)))
#define READ_U32(p, o) (*(u32 *)((u8 *)(p) + (o)))
#define READ_S8(p, o) (*(s8 *)((u8 *)(p) + (o)))
#define READ_U8(p, o) (*(u8 *)((u8 *)(p) + (o)))
/** @brief Byte access in a partially recovered actor layout. */
#define U8_AT(p, o) (*(u8 *)((s32)(p) + (o)))
/** @brief Unsigned halfword access in an actor or part record. */
#define U16_AT(p, o) (*(u16 *)((s32)(p) + (o)))
/** @brief Signed halfword access in an actor state record. */
#define S16_AT(p, o) (*(s16 *)((s32)(p) + (o)))
/** @brief Word access in actor flags, positions, and scratch vectors. */
#define S32_AT(p, o) (*(s32 *)((s32)(p) + (o)))

/* ------------------------------------------------------------------------- */
/* File-scope parameter record types (kept distinct across members)          */
/* ------------------------------------------------------------------------- */

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

/** @brief Actor position and animation fields at their original 0x54-byte stride (func_80097150). */
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
} ContactRecord;

/** @brief Three fixed-point position or displacement components. */
typedef struct
{
    s32 x, y, z;
} FieldMoveVector;

/** @brief Position, state and index in a 0x54-byte actor record. */
typedef struct
{
    s32 x, y, z;
    u8 padc[0x10];
    u32 flags;
    u8 surface, state;
    u8 pad22[8];
    s16 kind;
    u8 pad2c[0xE];
    u8 index;
    u8 pad3b[0x19];
} FieldMoveActor;

/** @brief D_80105AE0 actor slot as viewed by func_80098748; only the 0x10 flags word is read. */
typedef struct
{
    u8 pad0[0x10];
    s32 flags; /* 0x10 */
} FieldActorSlot;

/** @brief Caller struct holding the actor slot index at 0x3A (func_80098748). */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A; /* 0x3A */
} FieldActor;

/** @brief Actor record position and state selectors, with the original 0x54-byte stride (func_800987DC). */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padc[0x25-12];
    u8 unk25;
    u8 pad26[4];
    s16 unk2a;
    u8 pad2c[11];
    s8 unk37;
    u8 pad38[2];
    u8 unk3a;
    u8 unk3b;
    u8 pad3c[0x18];
} ReactRecord;

/** @brief Field actor record with the state-array index at 0x3A (func_80098C7C). */
typedef struct
{
    u8 pad0[0x20];
    u8 unk20;
    u8 unk21;
    u8 pad22[0x24 - 0x22];
    u8 unk24;
    u8 pad25[0x27 - 0x25];
    u8 unk27;
    u8 pad28[0x2A - 0x28];
    s16 unk2A;
    u8 pad2C[0x3A - 0x2C];
    u8 unk3A;
    u8 unk3B;
    u8 pad3C[0x54 - 0x3C];
} Struct_D800FDF58;

/** @brief Actor position, facing, and animation state in a 0x54-byte entry (func_80098DD4). */
typedef struct
{
    s32 unk0, unk4, unk8;
    u8 padC[0x1B - 0xC];
    u8 unk1B;
    u8 pad1C[5];
    u8 unk21;
    u8 pad22[2];
    u8 unk24;
    u8 pad25[2];
    u8 unk27;
    u8 pad28[2];
    s16 unk2A;
    u8 pad2C[2];
    s16 unk2E;
    u8 pad30[11];
    u8 unk3B;
    u8 pad3C[0x54 - 0x3C];
} Entry;

/** @brief Field object containing the state-array index (func_80098FC4). */
typedef struct FieldObject80098FC4
{
    u8 pad0[0x3A];
    u8 stateIndex;
} FieldObject80098FC4;

/* ------------------------------------------------------------------------- */
/* Forward prototypes for members defined later in this TU. Pointer-type      */
/* differences at call sites are codegen-identical.                          */
/* ------------------------------------------------------------------------- */

s32 func_800978AC(Vec2s *, Vec2s *, Vec2s *, Vec2s *);
s32 func_80098748(FieldActor *, s32 *);
s32 func_800987DC(ReactRecord *, ReactRecord *, s32);

/* ------------------------------------------------------------------------- */
/* func_800970B0 (0x800970B0)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Loads resource 0x5DD and copies its packed 11x24x32-byte payload.
 *
 * The resource data begins one byte into D_8010D038. Rows are copied into
 * D_8010AED0 using a 0x300-byte outer stride and a 0x20-byte row stride.
 *
 * @note gcc272_cdk, 100% match.
 */
void func_800970B0(void)
{
    extern u8 D_8010AED0[];
    extern u8 *D_8010D038;
    extern void cdrom_queue_read(s32 resource_index, void *dst_buffer);
    extern void cdrom_wait_queue_empty(void);

    s32 i, j, k;
    u8 *src, *row, *dst;
    u8 value;

    cdrom_queue_read(0x5DD, D_8010D038);
    cdrom_wait_queue_empty();
    src = D_8010D038 + 1;
    for (i = 0; i < 11; i++) {
        for (j = 0; j < 24; j++) {
            k = 0;
            row = (i * 0x300 + j * 0x20) + D_8010AED0;
            do {
                dst = row + k;
                k++;
                value = *src;
                *dst = value;
                src++;
            } while (k < 32);
        }
    }
}

/* ------------------------------------------------------------------------- */
/* func_80097150 (0x80097150)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Test an actor quad against eligible actors and handle contact reactions.
 * @param quad Four packed screen-space vertices of the tested quadrilateral.
 * @param record Actor record supplying the owner and vertical position.
 * @param contact Output candidate index and packed contact point.
 * @return Zero for no contact, one or two for a collision layer, or three for a handled reaction.
 * @note A failed edge test also writes the 0x80008000 sentinel to contact->point.
 * @note The target's strict NormalClip signs and signed distance rounding are preserved.
 * @see decomp.me (100%)
 */
s32 func_80097150(Point *quad, ContactRecord *record, Contact *contact)
{
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

    extern ContactRecord D_800FDF58[];
    extern State D_80105AE0[];
    extern ActorSlot g_field_actor_slots[];
    extern Request D_80105880[];
    extern s32 D_800FE754, D_8010D020;
    void func_8008E690(ContactRecord *);

    Point* input_quad;
    ContactRecord* scan_record;
    ContactRecord* owner_record;
    u8* scan_state;
    State* owner_state;
    /** @brief Centroid and unused bytes in the original 0x28-byte local workspace. */
    union
    {
        Point center;
        u8 storage[0x28];
    } scratch;
    s16 candidate_animation;
    s32 candidate_extent;
    s32 flags_or_extent;

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
    s32 target_address;
    s32 request_offset;
    s32 matched_request_offset;
    s32 center_x;
    s32 packed_center;
    s32 center_y;
    s32 vertical_distance;
    u8 owner_index;
    u8 target_count;
    Point* candidate_quad;
    ContactRecord* initial_owner;
    State* target_state;
    State* reaction_state_40;
    State* reaction_state_3f;
    State* reaction_base_3f;
    State* reaction_base_40;
    State* target_base;
    Request* request_base;
    Request* request;
    ActorSlot* slot_base;
    Point* input_vertex;
    u8* scan_mode;
    u8* scan_extent;
    u8* target_list_base;

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
    reaction_base_3f = D_80105AE0;
    reaction_state_3f = &reaction_base_3f[scan_record->actor_index];
    reaction_state_3f->status.flags = (s32)((reaction_state_3f->status.flags & ~0x1C) | 8);
    return 3;
special_40:
    scan_record->animation = 0x385;
    func_8008E690(scan_record);
    reaction_base_40 = D_80105AE0;
    reaction_state_40 = &reaction_base_40[scan_record->actor_index];
    reaction_state_40->status.flags = (s32)((reaction_state_40->status.flags & ~0x1C) | 0x10);
    return 3;
initialize:
    scan_state = (u8*)D_80105AE0;
    owner_state = (State*)(scan_state + owner_index * 0x23C);
    actor_index = 0;
    scan_record = D_800FDF58;
    scan_mode = (u8*)D_800FDF58 + 0x21;
    scan_extent = scan_state + 0x12E;
    target_base = D_80105AE0;
    request_base = D_80105880;
    slot_base = g_field_actor_slots;
scan_actor:
    if ((READ_U8(scan_mode, 0x4) != 0xFF) && !(READ_U32(scan_extent, -0x122) & 0x2280) &&
        ((actor_index < 3) || ((READ_U32(scan_extent, -0x11e) & 0xF) == D_800FE754)) && (READ_U32(scan_extent, -0x12a) != 0) &&
        ((u32)((READ_U8(scan_mode, 0x0) & 0x7F) - 0x38) >= 2U) && (READ_U8(scan_mode, 0x19) != record->actor_index))
    {
        if (READ_U32(scan_extent, 0x4a) & 0x80)
        {
            eligible = 0;
            if (owner_record->animation == 0x91)
            {
                target_address = owner_record->actor_index * 0x23C;
                target_address += (s32)target_base;
                target_state = (State*)target_address;
                target_count = target_state->status.bytes.count;
                list_index_or_layer = 0;
                if (target_count != 0)
                {
                    target_list_base = (u8*)target_state;
                    do
                    {
                        if (READ_U8(target_list_base + list_index_or_layer, 0x180) == actor_index)
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
            flags_or_extent = READ_U32(scan_extent, 0x4a);
            if (!(flags_or_extent & 0x20) && (!(flags_or_extent & 1) || (slot_base[READ_U8(scan_extent, 0x4c)].owner_index == record->actor_index)) &&
                !(flags_or_extent & 2))
            {
                if (!(flags_or_extent & 0x40))
                {
                    if ((u8)READ_U8(scan_mode, 0x19) < 2U)
                    {
                        request_offset = READ_U8(scan_mode, 0x19) * 0x1C;
                    }
                    else
                    {
                        request_offset = 0x38;
                    }

                    request = (Request*)((u8*)request_base + request_offset);
                    if (request->actor_index == READ_U8(scan_mode, 0x19))
                    {
                        if ((u32)(request->actor_index & 0xFF) < 2U)
                        {
                            matched_request_offset = request->actor_index * 0x1C;
                        }
                        else
                        {
                            matched_request_offset = 0x38;
                        }
                        if (((Request*)((u8*)request_base + matched_request_offset))->unk0 == 0)
                        {
                            goto check_animation;
                        }
                        goto next_actor;
                    }
                    goto check_animation;
                }
            check_animation:
                candidate_animation = READ_S16(scan_mode, 0x9);
                if ((candidate_animation != 0x91) && (candidate_animation != 0x87) && (candidate_animation != 0xAE))
                {
                    if (D_8010D020 == 0)
                    {
                        if ((u8)record->actor_index < 3U)
                        {
                            if ((u8)READ_U8(scan_mode, 0x19) < 3U)
                            {
                                goto next_actor;
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
                    flags_or_extent = owner_state->extent;
                    if (candidate_extent < 0)
                    {
                        candidate_extent = -candidate_extent;
                    }
                    if (flags_or_extent < 0)
                    {
                        flags_or_extent = -flags_or_extent;
                    }
                    flags_or_extent += candidate_extent;
                    vertical_distance = vertical_distance < flags_or_extent;
                    if (vertical_distance)
                    {
                        list_index_or_layer = 4;
                        input_quad = quad;

                    scan_layer:
                        candidate_quad = (Point*)(scan_state + (list_index_or_layer * 4 + 0x148));
                        candidate_edge = 0;
                        do
                        {
                            input_edge = 0;
                            next_edge_offset = ((candidate_edge + 1) & 3) * 4;
                            input_vertex = input_quad;
                            do
                            {
                                if ((candidate_quad[0].packed != 0) || (candidate_quad[1].packed != 0))
                                {
                                    intersection = func_800978AC(input_vertex, input_quad + ((input_edge + 1) & 3), candidate_quad + candidate_edge,
                                                                 (Point*)((u8*)candidate_quad + next_edge_offset));
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
                            center_x = quad[0].coord.x + quad[1].coord.x + quad[2].coord.x + quad[3].coord.x;
                            if (center_x < 0)
                            {
                                center_x += 3;
                            }
                            scratch.center.coord.x = (u16)(center_x >> 2);
                            center_y = quad[0].coord.y + quad[1].coord.y + quad[2].coord.y + quad[3].coord.y;
                            if (center_y < 0)
                            {
                                center_y += 3;
                            }
                            scratch.center.coord.y = (s16)(center_y >> 2);
                            do
                            {
                                if (NormalClip(scratch.center.packed, candidate_quad[0].packed, candidate_quad[1].packed) >= 0)
                                {
                                    break;
                                }
                                if (NormalClip(scratch.center.packed, candidate_quad[1].packed, candidate_quad[2].packed) >= 0 ||
                                    NormalClip(scratch.center.packed, candidate_quad[2].packed, candidate_quad[3].packed) >= 0 ||
                                    NormalClip(scratch.center.packed, candidate_quad[3].packed, candidate_quad[0].packed) >= 0)
                                {
                                    goto next_layer;
                                }
                                goto centroid_contact;
                            } while (0);
                            if ((NormalClip(scratch.center.packed, candidate_quad[1].packed, candidate_quad[2].packed) > 0) &&
                                (NormalClip(scratch.center.packed, candidate_quad[2].packed, candidate_quad[3].packed) > 0))
                            {
                                if (NormalClip(scratch.center.packed, candidate_quad[3].packed, candidate_quad[0].packed) > 0)
                                {
                                centroid_contact:
                                    centroid_layer = list_index_or_layer;
                                    packed_center = (s32)((u16)scratch.center.coord.x | (scratch.center.coord.y << 16));
                                    contact->actor = actor_index;
                                    contact->point = packed_center;
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

/* ------------------------------------------------------------------------- */
/* func_800978AC (0x800978AC)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Find an integer intersection of two screen-space line segments.
 * @param first_start First segment's starting point.
 * @param first_end First segment's ending point.
 * @param second_start Second segment's starting point.
 * @param second_end Second segment's ending point.
 * @return X in the low 16 bits and Y in the high 16 bits, or 0x80008000.
 * @note Collinear overlapping segments return the first segment's start.
 * @note Preserve full-width candidates and the unsigned weighted bounds checks.
 */
s32 func_800978AC(Vec2s *first_start, Vec2s *first_end, Vec2s *second_start, Vec2s *second_end)
{
    s32 endpoint_a, endpoint_b, distance_a;
    s32 endpoint_a0, endpoint_b0;
    s32 endpoint_a1, endpoint_b1;
    s32 endpoint_a2, endpoint_b2;
    s32 endpoint_a3, endpoint_b3;
    s32 load_end;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_a0_3;
    s32 temp_a0_4;
    s32 temp_a0_5;
    s32 temp_a0_6;
    s32 temp_a0_7;
    s32 temp_a0_8;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_a1_3;
    s32 temp_a1_4;
    s32 temp_a1_5;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_a2_3;
    s32 temp_a2_4;
    s32 temp_a3;
    s32 temp_a3_2;
    s32 temp_a3_3;
    s32 temp_a3_4;
    s32 temp_t0;
    s32 temp_t1;
    s32 temp_t1_3;
    s32 temp_t1_4;
    s32 first_start_x;
    s32 intersection_x;
    s32 intersection_y;
    s32 first_start_y;
    s32 first_end_x;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 temp_v1_5;
    s32 temp_v1_6;
    s32 temp_v1_9;
    s32 cross_yx;
    s32 cross_xy;
    s32 second_dx;
    s32 temp_t0_3;
    s32 first_dx;
    s32 first_dy;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v1;
    s32 temp_v1_7;
    s32 second_dy;
    s32 var_t0;
    s32 var_t0_2;
    s32 var_t0_3;
    s32 var_t0_4;
    s32 value;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    u32 packed_x_or_sum;
    u32 temp_v1_10;
    u32 temp_v1_11;
    u32 temp_v1_12;
    u32 temp_v1_13;

    /* Coordinate locals also retain endpoint values before interpolation. */
    intersection_y = first_end->y;
    first_start_y = first_start->y;
    first_dy = intersection_y - first_start_y;
    if (first_dy == 0)
    {
        load_end = second_end->y;
        temp_t1 = second_start->y;
        temp_v1 = load_end - temp_t1;
        intersection_y = first_start_y;
        if (temp_v1 == 0)
        {
            if ((intersection_y == temp_t1) && ((temp_t0 = second_start->x, temp_v1_2 = first_start->x, ((temp_t0 < temp_v1_2) == 0)) || (temp_a0 = second_end->x, ((temp_a0 < temp_v1_2) == 0)) || (temp_v1_3 = first_end->x, ((temp_t0 < temp_v1_3) == 0)) || (temp_a0 >= temp_v1_3)) && ((temp_a2 = second_start->x, temp_v1_4 = first_start->x, ((temp_v1_4 < temp_a2) == 0)) || (temp_a3 = second_end->x, ((temp_v1_4 < temp_a3) == 0)) || (temp_a1 = first_end->x, ((temp_a1 < temp_a2) == 0)) || (temp_a1 >= temp_a3)))
            {
                packed_x_or_sum = (u16) first_start->x;
                value = intersection_y << 0x10;
                goto pack_result;
            }
            goto no_intersection;
        }
        load_end = second_end->x;
        temp_a0_2 = second_start->x;
        second_dx = load_end - temp_a0_2;
        if (second_dx == 0)
        {
            intersection_x = temp_a0_2;
        }
        else
        {
            intersection_x = ((s32) ((intersection_y - temp_t1) * second_dx) / temp_v1) + temp_a0_2;
        }
        goto check_bounds;
    }
    first_end_x = first_end->x;
    first_start_x = first_start->x;
    first_dx = first_end_x - first_start_x;
    if (first_dx == 0)
    {
        load_end = second_end->x;
        temp_t1_3 = second_start->x;
        temp_t0_3 = load_end - temp_t1_3;
        intersection_x = first_start_x;
        if (temp_t0_3 == 0)
        {
            if ((intersection_x == temp_t1_3) && ((temp_a0_3 = second_start->y, ((temp_a0_3 < first_start_y) == 0)) || (temp_v1_5 = second_end->y, ((temp_v1_5 < first_start_y) == 0)) || (temp_a0_3 >= intersection_y) || (temp_v1_5 >= intersection_y)))
            {
                temp_a2_2 = second_start->y;
                temp_v1_6 = first_start->y;
                if ((temp_v1_6 >= temp_a2_2) || (temp_a3_2 = second_end->y, ((temp_v1_6 < temp_a3_2) == 0)) || (temp_a1_2 = first_end->y, ((temp_a1_2 < temp_a2_2) == 0)) || (temp_a1_2 >= temp_a3_2))
                {
                    packed_x_or_sum = intersection_x & 0xFFFF;
                    value = first_start->y << 0x10;
                    goto pack_result;
                }
                goto reject_overlap;
            }
            goto no_intersection;
        }
        load_end = second_end->y;
        temp_a0_4 = second_start->y;
        temp_v1_7 = load_end - temp_a0_4;
        if (temp_v1_7 == 0)
        {
            intersection_y = temp_a0_4;
        }
        else
        {
            intersection_y = ((s32) ((intersection_x - temp_t1_3) * temp_v1_7) / temp_t0_3) + temp_a0_4;
        }
        goto check_bounds;
    }
    load_end = second_end->y;
    intersection_x = second_start->y;
    second_dy = load_end - intersection_x;
    intersection_y = intersection_x;
    if (second_dy == 0)
    {
        intersection_x = ((s32) ((intersection_y - first_start_y) * first_dx) / first_dy) + first_start_x;
        goto check_bounds;
    }
    intersection_y = second_end->x;
    temp_a0_5 = second_start->x;
    second_dx = intersection_y - temp_a0_5;
    if (second_dx == 0)
    {
        intersection_x = temp_a0_5;
        goto calculate_y;
    }
    cross_yx = first_dy * second_dx;
    cross_xy = first_dx * second_dy;
    if (cross_yx == cross_xy)
    {
        if ((intersection_x == (((s32) ((temp_a0_5 - first_start_x) * first_dy) / first_dx) + first_start_y)) && ((temp_a0_5 >= first_start_x) || (intersection_y >= first_start_x) || (temp_a0_5 >= first_end_x) || (intersection_y >= first_end_x)))
        {
            temp_a2_3 = second_start->x;
            temp_v1_9 = first_start->x;
            if ((temp_v1_9 >= temp_a2_3) || (temp_a3_3 = second_end->x, ((temp_v1_9 < temp_a3_3) == 0)) || (temp_a1_3 = first_end->x, ((temp_a1_3 < temp_a2_3) == 0)) || (temp_a1_3 >= temp_a3_3))
            {
                packed_x_or_sum = (u16) first_start->x;
                value = first_start->y << 0x10;
                goto pack_result;
            }
            goto reject_overlap;
        }
        goto no_intersection;
    }
    intersection_x = ((s32) ((((intersection_x - ((s32) (second_dy * temp_a0_5) / second_dx)) - first_start_y) + ((s32) (first_dy * first_start_x) / first_dx)) * (first_dx * second_dx)) / (s32) (cross_yx - cross_xy));
calculate_y:
    intersection_y = ((s32) ((intersection_x - first_start_x) * first_dy) / first_dx) + first_start_y;
check_bounds:
    /* Each weighted quotient must reproduce its candidate coordinate. */
    endpoint_a0 = first_start->x;
    endpoint_b0 = first_end->x;
    value = intersection_x - endpoint_a0;
    distance_a = value;
    if (value < 0)
    {
        distance_a = -distance_a;
    }
    value = intersection_x - endpoint_b0;
    if (value < 0)
    {
        value = -value;
    }
    packed_x_or_sum = distance_a + value;
    if (packed_x_or_sum != 0)
    {
        value *= endpoint_a0;
        distance_a *= endpoint_b0;
        value += distance_a;
        if ((u32)value / packed_x_or_sum != intersection_x)
        {
            goto no_intersection;
        }
    }
    endpoint_a1 = first_start->y;
    endpoint_b1 = first_end->y;
    value = intersection_y - endpoint_a1;
    distance_a = value;
    if (value < 0)
    {
        distance_a = -distance_a;
    }
    value = intersection_y - endpoint_b1;
    if (value < 0)
    {
        value = -value;
    }
    packed_x_or_sum = distance_a + value;
    if (packed_x_or_sum != 0)
    {
        value *= endpoint_a1;
        distance_a *= endpoint_b1;
        value += distance_a;
        if ((u32)value / packed_x_or_sum != intersection_y)
        {
            goto no_intersection;
        }
    }
    endpoint_a2 = second_start->x;
    endpoint_b2 = second_end->x;
    value = intersection_x - endpoint_a2;
    distance_a = value;
    if (value < 0)
    {
        distance_a = -distance_a;
    }
    value = intersection_x - endpoint_b2;
    if (value < 0)
    {
        value = -value;
    }
    packed_x_or_sum = distance_a + value;
    if (packed_x_or_sum != 0)
    {
        value *= endpoint_a2;
        distance_a *= endpoint_b2;
        value += distance_a;
        if ((u32)value / packed_x_or_sum != intersection_x)
        {
            goto no_intersection;
        }
    }
    endpoint_a3 = second_start->y;
    endpoint_b3 = second_end->y;
    value = intersection_y - endpoint_a3;
    distance_a = value;
    if (value < 0)
    {
        distance_a = -distance_a;
    }
    value = intersection_y - endpoint_b3;
    if (value < 0)
    {
        value = -value;
    }
    packed_x_or_sum = distance_a + value;
    if (packed_x_or_sum != 0)
    {
        value *= endpoint_a3;
        distance_a *= endpoint_b3;
        value += distance_a;
        if ((u32)value / packed_x_or_sum != intersection_y)
        {
            goto no_intersection;
        }
    }
    goto valid;
reject_overlap:
no_intersection:
    return 0x80008000;
valid:
    packed_x_or_sum = intersection_x & 0xFFFF;
    value = intersection_y << 16;
pack_result:
    return packed_x_or_sum | value;
}

/* ------------------------------------------------------------------------- */
/* func_80097FA0 (0x80097FA0)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Resolve a proposed actor move against map and actor collisions.
 * @param actor Actor whose position and persistent collision state are updated.
 * @param position Input movement vector, overwritten with the resolved position.
 * @param mode Collision response mode forwarded to the actor collision helper.
 * @return One when the proposed or resolved position is accepted, zero otherwise.
 * @note Uses mover and probe records at scratchpad addresses 0x1F800010 and
 *       0x1F800080. The query result is consumed as a full return-register value.
 * @see decomp.me (100%)
 */
s32 func_80097FA0(FieldMoveActor* actor, s32* position, s32 mode)
{
    /** @brief Visual kind and flags in a 0x48-byte object record. */
    typedef struct
    {
        u8 pad0[0x2E];
        u8 visual_kind;
        u8 pad2f[5];
        u32 flags;
        u8 pad38[0x10];
    } FieldMoveObject;
    /** @brief Packed collision flags, height and contact in a 0x23C-byte state. */
    typedef struct
    {
        u8 pad0[0x174]; /** @brief Collision flags and signed height share one word. */
        union
        {
            u32 word;
            struct
            {
                u16 flags;
                s16 height;
            } h;
        } packed;
        u8 pad178[0x24];
        s32 contact, surface;
        u8 pad1a4[0x98];
    } FieldMoveState;
    /** @brief Scratchpad mover request and collision resolver output. */
    typedef struct
    {
        s32 x, y, z, unkc, unk10, unk14, unk18, contact, surface;
        s16 width, height_tolerance; /** @brief Step halfword and request bits also accessed as a full word. */
        union
        {
            s32 word;
            struct
            {
                s16 step;
                u16 flags;
            } h;
            struct
            {
                unsigned step : 16;
                unsigned bit16 : 1;
                unsigned bit17 : 1;
                unsigned high : 14;
            } bits;
        } packed;
    } FieldMoveRequest;
    /** @brief Scratchpad position and footprint used by the obstruction query. */
    typedef struct
    {
        s32 x, y, z;
        s16 unkc, unke, unk10;
    } FieldMoveQuery;
    /** @brief Map dimensions used for the fixed-point bounds check. */
    typedef struct
    {
        s16 x;
        u16 unk2;
    } FieldMoveBounds;

    s32 func_8005B368(FieldMoveQuery*);
    s32 func_8005B6AC(FieldMoveRequest*);
    s32 func_80092988(FieldMoveActor*, FieldMoveVector*);
    extern FieldMoveObject D_800FE3A0[];
    extern FieldMoveState D_80105AE0[];
    extern s32 D_800FE754, D_8010D024;

    FieldMoveVector delta;
    FieldMoveState* height_state;
    FieldMoveState* states;
    s32 hit;
    FieldMoveBounds* bounds = (FieldMoveBounds*)0x801ED400;
    FieldMoveRequest* mover = (FieldMoveRequest*)0x1F800010;
    FieldMoveQuery* query = (FieldMoveQuery*)0x1F800080;
    s32 actor_z;
    s32 requested_x;
    s32 actor_x;
    s32 requested_z;
    s32 height;
    s32 can_move;
    u16 temp_v1;
    u16 temp_v1_4;
    u16 temp_v1_5;
    FieldMoveState* temp_v0;
    FieldMoveState* temp_v0_2;
    FieldMoveState* temp_v0_3;
    FieldMoveState* temp_v1_6;
    FieldMoveState* temp_v1_7;
    FieldMoveState* temp_v1_8;

    if (((u32)D_800FE3A0[actor->index].flags >> 0x17) & 1)
    {
        actor->x += position[0];
        actor->y = (s32)(actor->y + position[1]);
        position += 2;
        actor->z += position[0];
        return 1;
    }
    if (D_800FE754 != 0)
    {
        temp_v1 = actor->kind;
        if (((u32)(temp_v1 - 0xB0) >= 2U) && ((s16)temp_v1 != 0xB5) && (func_80092988(actor, (FieldMoveVector*)position) != 0))
        {
            position[0] = 0;
        }
    }
    actor_x = actor->x;
    if ((actor_x >= 0) && (actor_x < (bounds->x << 8)))
    {
        actor_z = actor->z;
        if (actor_z >= 0)
        {
            if (actor_z < ((s32)(bounds->unk2 << 0x10) >> 7))
            {
                mover->x = actor_x;
                mover->y = (s32)actor->y;
                mover->z = (s32)actor->z;
                mover->unkc = position[0];
                requested_x = mover->unkc;
                mover->unk10 = (s32)position[1];
                mover->unk14 = position[2];
                requested_z = mover->unk14;
                mover->height_tolerance = 0x10;
                query->unke = 0x10;
                if (D_800FE3A0[actor->index].visual_kind == 0x40)
                {
                    mover->width = 0xC;
                    query->unkc = 0xC;
                    mover->packed.h.step = 8;
                    query->unk10 = 8;
                }
                else
                {
                    mover->width = 9;
                    query->unkc = 9;
                    mover->packed.h.step = 6;
                    query->unk10 = 6;
                }
                mover->height_tolerance = 0x10;
                mover->packed.bits.bit17 = 0;
                mover->packed.bits.bit16 = 0;
                mover->contact = (s32)D_80105AE0[actor->index].contact;
                mover->surface = (s32)D_80105AE0[actor->index].surface;
                func_8005B6AC(mover);
                D_80105AE0[actor->index].contact = (s32)mover->contact;
                D_80105AE0[actor->index].surface = (s32)mover->surface;
                temp_v1_4 = actor->kind;
                if (((u32)(temp_v1_4 - 0xB0) < 2U) || ((s16)temp_v1_4 == 0xB5))
                {
                    delta.x = requested_x - actor->x;
                    delta.y = 0;
                    delta.z = requested_z - actor->z;
                }
                else
                {
                    delta.x = mover->x - actor->x;
                    delta.y = 0;
                    delta.z = mover->z - actor->z;
                }
                query->x = mover->x;
                {
                    s32 z = mover->z;
                    s32 y = position[1] + actor->y;
                    query->z = z;
                    query->y = y;
                }
                if (((D_800FE754 == 0) || (actor->flags & 0x1FF) || (func_8005B368(query) == -1)) &&
                    ((temp_v1_5 = actor->kind, (((u32)(temp_v1_5 - 0xB0) < 2U) != 0)) || ((s16)temp_v1_5 == 0xB5) || (D_800FE754 == 0) ||
                     (func_80092988(actor, &delta) == 0)))
                {
                    position[0] = mover->x;
                    if (((actor->state & 0x7F) == 0x3D) && ((u8)actor->index < 2U))
                    {
                        position[1] = position[1] + actor->y;
                    }
                    else
                    {
                        position[1] = mover->y;
                    }
                    states = D_80105AE0;
                    position[2] = (s32)mover->z;
                    height_state = &states[actor->index];
                    height = mover->unk18;
                    if (height < 0)
                    {
                        height += 0xFF;
                    }
                    height_state->packed.h.height = (s16)(height >> 8);
                }
                else
                {
                    goto block_34;
                }
            }
            else
            {
                goto block_33;
            }
        }
        else
        {
            goto block_33;
        }
    }
    else
    {
    block_33:
        D_80105AE0[actor->index].packed.h.height = 0;
        D_80105AE0[actor->index].contact = -1;
        D_80105AE0[actor->index].surface = 0;
    block_34:
        position[0] = actor->x;
        position[1] = (s32)actor->y;
        position[2] = (s32)actor->z;
    }
    D_8010D024 = 0;
    if (((u32)D_80105AE0[actor->index].packed.word >> 0xD) & 1)
    {
        if (func_80098748(actor, actor) == 0)
        {
            temp_v0 = &D_80105AE0[actor->index];
            temp_v0->packed.word = (s32)(temp_v0->packed.word & ~0x2000);
        }
        can_move = 1;
    }
    else if (func_80098748(actor, position) == 0)
    {
        can_move = 1;
        temp_v0_2 = &D_80105AE0[actor->index];
        temp_v0_2->packed.word = (s32)(temp_v0_2->packed.word & ~0x2000);
    }
    else
    {
        hit = func_80098748(actor, actor);
        can_move = 0;
        if (hit != 0)
        {
            temp_v1_6 = &D_80105AE0[actor->index];
            temp_v1_6->packed.word = (s32)(temp_v1_6->packed.word | 0x2000);
        }
    }
    if (can_move == 0)
    {
        return 0;
    }
    if (((u32)D_80105AE0[actor->index].packed.word >> 0xE) & 1)
    {
        actor->x = position[0];
        actor->y = (s32)position[1];
        actor->z = (s32)position[2];
        if (func_800987DC(actor, actor, mode) == 0)
        {
            temp_v0_3 = &D_80105AE0[actor->index];
            temp_v0_3->packed.word = (s32)(temp_v0_3->packed.word & ~0x4000);
        }
        return 1;
    }
    if (func_800987DC(actor, position, mode) == 0)
    {
        actor->x = position[0];
        actor->y = (s32)position[1];
        actor->z = (s32)position[2];
        temp_v1_7 = &D_80105AE0[actor->index];
        temp_v1_7->packed.word = (s32)(temp_v1_7->packed.word & ~0x4000);
        return 1;
    }
    if (func_800987DC(actor, actor, mode) != 0)
    {
        temp_v1_8 = &D_80105AE0[actor->index];
        temp_v1_8->packed.word = (s32)(temp_v1_8->packed.word | 0x4000);
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
/* func_80098748 (0x80098748)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Classify a packed value against the active D_800FF610 threshold band.
 *
 * Returns 0 when the actor slot selected by @p arg0 has no low nibble set in its
 * flags word. Otherwise reads the threshold entry for the current
 * @c D_800FE754 - 1 index and compares the high 24 bits of @c *arg1 against it:
 * 1 when below the band minimum, 1 when above (min + span), else 0.
 *
 * @param arg0 Actor whose slot flags gate the test.
 * @param arg1 Pointer to the packed value; the top 24 bits are the sample.
 * @return 0 inside the band or when the slot is inactive; 1 when outside it.
 *
 * @see decomp.me (100%) TODO
 */
s32 func_80098748(FieldActor *arg0, s32 *arg1)
{
    /** @brief {min, span} threshold pair from the D_800FF610 table (stride 4). */
    typedef struct
    {
        u16 min;  /* 0x00 */
        u16 span; /* 0x02 */
    } FieldThreshold;

    extern s32 D_800FE754;
    extern FieldThreshold D_800FF610[];
    extern u8 D_80105AE0[];

    FieldThreshold *rec;
    FieldThreshold *b;
    s32 idx;
    u16 lo;
    s32 new_var;
    s32 val;

    if ((((FieldActorSlot *)(D_80105AE0 + arg0->unk3A * 0x23C))->flags & 0xF) == 0)
    {
        return 0;
    }
    b = D_800FF610;
    idx = D_800FE754 - 1;
    rec = &b[idx];
    lo = rec->min;
    val = *arg1 >> 8;
    if (val < (new_var = (s32)lo))
    {
        return 1;
    }
    return (s32)(lo + rec->span) < val;
}

/* ------------------------------------------------------------------------- */
/* func_800987DC (0x800987DC)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Find an overlapping actor and optionally start its contact reaction.
 * @param record Actor whose collision dimensions and resource flags are tested.
 * @param position Position to test against the selected actor group.
 * @param filter_group Nonzero to select the opposing group when filtering is enabled.
 * @return Candidate index plus 0x8000, or zero for no candidate or a handled reaction.
 * @note The SDK GTE macros preserve the original squared-distance calculation.
 */
s32 func_800987DC(ReactRecord *record, ReactRecord *position, s32 filter_group)
{
    /** @brief Runtime actor collision and animation fields, with the original 0x23C-byte stride. */
    typedef struct
    {
        u32 unk0;
        u32 unk4;
        u32 unk8;
        u32 unkc;
        u8 pad10[0x12C-16];
        /** @brief Word and halfword views of the packed collision dimensions. */
        union
        {
            u32 word;
            /** @brief Signed center offset and packed unsigned diameter. */
            struct
            {
                s16 offset;
                u16 diameter;
            } h;
        } collision;
        u8 pad130[0x142-0x130];
        s16 unk142;
        u8 pad144[2];
        s16 unk146;
        u8 pad148[0x178-0x148];
        u32 unk178;
        u8 pad17c[0x23C-0x17C];
    } State;

    /** @brief Resource classification and reaction flags, with the original 0x14-byte stride. */
    typedef struct
    {
        u8 pad0[8];
        u8 unk8;
        u8 pad9[7];
        u32 unk10;
    } Resource;

    /** @brief Actor animation request, with the original 0x1C-byte stride. */
    typedef struct
    {
        s32 unk0;
        u8 pad4[8];
        s32 unkc;
        u8 pad10[12];
    } Request;

    /** @brief Animation target list and GTE vectors at their original stack offsets. */
    typedef struct
    {
        s32 targets[2];
        VECTOR delta;
        VECTOR squared;
    } ScanWorkspace;

    extern ReactRecord D_800FDF58[];
    extern ReactRecord D_800FE054[];
    extern State D_80105AE0[];
    extern State D_80106194[];
    extern Request D_80105880[];
    extern Resource g_field_resource_entries[];
    extern s32 D_8010D020,D_8010D024;
    s32 func_800839F8(s32,s32);
    s32 func_80083EEC(s32,s32,s32);
    void func_800A2DD8(s32);
    void field_start_actor_animation(s32,s32,s32 *);

    ScanWorkspace scratch;
    s32 request_base;
    u8 *matched_request_base;
    s32 record_offset;
    State *scan_state;
    u8 *scan_position;
    ReactRecord *scan_record;
    s32 record_y;
    s32 scan_y;
    s32 animation_slot;
    s32 scan_flags;
    s32 actor_index;
    s32 candidate_index;
    s32 candidate_end;
    s32 unused_result;
    s32 request_offset;
    s32 matched_request_offset;
    Request *request_entry;
    s8 record_height;
    s8 scan_height;
    State *record_state;
    u8 *scan_dimensions;

    if (filter_group != 0)
    {

        if (D_8010D020 == 0)
        {

            if ((u8) record->unk3a < 3U)
            {

                scan_record = D_800FE054;
                scan_state = D_80106194;
                actor_index = 3;
                goto scan_to_last;
            }
            scan_record = D_800FDF58;
            scan_state = D_80105AE0;
            actor_index = 0;
            candidate_end = 3;
        }
        else
        {
            goto scan_all;
        }
    }
    else
    {
scan_all:
        scan_record = D_800FDF58;
        scan_state = D_80105AE0;
        actor_index = 0;
scan_to_last:
        candidate_end = 0xD;
    }
    do
        {
            do
    {
        candidate_index = actor_index;
    } while (0);
        } while (0);
    if (record->unk37 < 9)
    {
        goto collision_scan;
    }
return_zero:
    return 0;
collision_scan:
    record_state = &D_80105AE0[record->unk3a];
    record_offset = record_state->collision.h.offset << 8;
    if (candidate_index < candidate_end)
    {

        scan_dimensions = (u8 *)scan_state + 0x12E;
        scan_position = (u8 *)scan_record + 8;
scan_next:
        do
        {
            if ((READ_U8(scan_position, 0x1D) == 0xFF) ||
                (READ_U32(scan_dimensions, -0x122) & 0x23E4) ||
                (scan_record == record) ||
                (scan_flags = READ_U32(scan_dimensions, 0x4A), ((scan_flags & 0x20) != 0)) ||
                (scan_flags & 1) ||
                (READ_S32(scan_dimensions, -2) == 0))
            {
                break;
            }
            scan_height = READ_S8(scan_position, 0x2F);
            record_height = record->unk37;
            record_y = position->unk4;
            if (((READ_S32(scan_position, -4) + ((READ_S16(scan_dimensions, 0x14) + scan_height) << 8)) > (record_y + ((record_state->unk146 + record_height) << 8))) ||
                ((READ_S32(scan_position, -4) + ((READ_S16(scan_dimensions, 0x18) + scan_height) << 8)) < (record_y + ((record_state->unk142 + record_height) << 8))))
            {
                break;
            }
            scratch.delta.vx = (READ_S32(scan_position, 0) - position->unk8) >> 8;
            scratch.delta.vy = ((scan_record->unk0 + (READ_S16(scan_dimensions, -2) << 8)) - (position->unk0 + record_offset)) >> 8;
            scratch.delta.vz = 0;
            gte_ldlvl(&scratch.delta);
            gte_sqr0();
            gte_stlvnl(&scratch.squared);
            if (SquareRoot0(scratch.squared.vx + scratch.squared.vy) <
                (((s32)(record_state->collision.h.diameter << 16) >> 17) + ((s32)(READ_U16(scan_dimensions, 0) << 16) >> 17)))
            {
                goto candidate_found;
            }
        } while (0);

        do
        {
            candidate_index += 1;
        } while (0);
        scan_position += 0x54;
        scan_record += 1;
        scan_dimensions += 0x23C;
        scan_state += 1;
        do
        {
            if (candidate_index < candidate_end)
            {
                goto scan_next;
            }
        } while (0);
candidate_found:
        ;
    }
    if (candidate_index == candidate_end)
    {

        D_8010D024 = 0;
        goto return_zero;
    }
    if ((&g_field_resource_entries[record->unk3b])->unk8 != 0)
    {

        if ((&g_field_resource_entries[scan_record->unk3b])->unk10 & 1)
        {

            if (scan_state->unk4 != 0)
            {

                if (!(scan_state->unkc & 0x280))
                {

                    if (!(scan_state->unk178 & 1))
                    {

                        actor_index = scan_record->unk3a;
                        if (!(((u32) (&D_80105AE0[actor_index])->unk178 >> 6) & 1))
                        {

                            request_base = (s32)D_80105880;
                            if (actor_index < 2U)
                            {

                                request_offset = actor_index * 0x1C;
                            }
                            else
                            {
                                request_offset = 0x38;
                            }
                            request_entry = (Request *)(request_base + request_offset);
                            request_base = scan_record->unk3a;
                            actor_index = request_entry->unkc;
                            if (actor_index == request_base)
                            {

                                matched_request_base = (u8 *)D_80105880;
                                if ((u32) (actor_index & 0xFF) < 2U)
                                {

                                    matched_request_offset = actor_index * 0x1C;
                                }
                                else
                                {
                                    matched_request_offset = 0x38;
                                }

                                if (((Request *)(matched_request_base + matched_request_offset))->unk0 == 0)
                                {

                                    goto start_reaction;
                                }
                                goto return_zero;
                            }
                            goto start_reaction;
                        }
start_reaction:

                        if (scan_record->unk2a == 0)
                        {

                            animation_slot = func_800839F8(scan_record->unk3a, 0);
                            if (animation_slot != -1)
                            {

                                scratch.targets[0] = (s32) record->unk3a;

                                if (func_80083EEC(scan_record->unk3a, animation_slot, 0x2A) != 0)
                                {

                                    if ((u8) scan_record->unk3a < 2U)
                                    {

                                        func_800A2DD8(scan_record->unk3a);
                                    }
                                    field_start_actor_animation(animation_slot, 1, &scratch.targets[0]);
                                    goto return_zero;
                                }
                                goto return_zero;
                            }
                            goto return_zero;
                        }
                        goto return_zero;
                    }
                }
            }
            goto return_zero;
        }
        goto report_candidate;
    }
report_candidate:
    D_8010D024 = candidate_index + 0x8000;
    return D_8010D024;
}

/* ------------------------------------------------------------------------- */
/* func_80098C7C (0x80098C7C)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Initialize a field actor resource state when its slot is active.
 * @param arg0 Actor state record to update.
 * @param arg1 Slot index used to select the associated field tables.
 */
void func_80098C7C(Struct_D800FDF58 *arg0, s32 arg1)
{
    typedef struct
    {
        u8 *start;
        u8 *end;
        u8 unk8;
        u8 slot_index;
        u8 padA[0xE - 0xA];
        s16 unkE;
        u32 flags;
    } FieldResourceEntry;

    typedef struct
    {
        u8 pad0[0x18];
        u16 unk18;
        u8 pad1A[0x23C - 0x1A];
    } SlotA;

    extern SlotA D_80105AE0[];
    extern Struct_D800FDF58 D_800FDF58[];
    extern FieldResourceEntry g_field_resource_entries[];

    u8 *res;
    s32 idx8;
    SlotA *slot_base;
    SlotA *slot;

    if (arg0->unk3A != 0)
    {
        return;
    }
    idx8 = arg1 << 3;
    if (arg0->unk2A != 0)
    {
        return;
    }
    slot_base = D_80105AE0;
    slot = (SlotA *)((u8 *)slot_base + ((((idx8 + arg1) << 4) - arg1) << 2));
    if ((slot->unk18 & 1) == 0)
    {
        return;
    }

    func_80098FC4((Struct_D800FDF58 *)((arg1 * 0x54) + (s32)D_800FDF58), 0);
    arg0->unk2A = 0x95;
    arg0->unk20 = 0xA;

    if (g_field_resource_entries[arg0->unk3B].flags & 1)
    {
        arg0->unk21 &= 0x80;
    }
    else
    {
        u8 b = arg0->unk21;
        s32 masked = b & 0x7F;
        arg0->unk21 = (masked % 5) | (b & 0x80);
    }

    arg0->unk27 = 0;
    arg0->unk24 = 1;
    res = g_field_resource_entries[arg0->unk3B].start;
    field_restart_actor_animation(arg0, res);
}

/* ------------------------------------------------------------------------- */
/* func_80098DD4 (0x80098DD4)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Probe ahead of an idle actor and begin its available interaction.
 * @param entry Actor whose position and facing select the interaction target.
 */
void func_80098DD4(Entry *entry)
{
    /** @brief Collision interaction fields in a 0x23C-byte actor record. */
    typedef struct
    {
        u8 pad0[0x18];
        u16 unk18;
        u8 pad1A[0x18E - 0x1A];
        u8 unk18E;
        u8 pad18F[0x23C - 0x18F];
    } Actor;
    /** @brief Resource data pointer and direction flags. */
    typedef struct
    {
        u8 *start;
        u8 pad4[12];
        u32 flags;
    } Resource;
    extern Entry D_800FDF58[];
    extern Actor D_80105AE0[];
    extern Resource g_field_resource_entries[];
    extern void func_800A3938(s32, s32);
    extern void func_800AF824(s32);
    extern s32 rcos(s32);
    extern s32 rsin(s32);

    s32 position[3];
    s32 actor_index;
    s32 result_or_address;
    u8 direction;
    s32 masked;
    Actor *actor;
    Actor *actor_base;

    if (entry->unk2A != 0)
    {
        return;
    }
    position[0] = entry->unk0;
    position[1] = entry->unk4;
    position[2] = entry->unk8;
    position[0] += rcos(entry->unk1B * 0x10);
    position[2] -= rsin(entry->unk1B * 0x10);
    result_or_address = func_800987DC(entry, position, 1);
    actor_index = result_or_address & 0x7FFF;
    if (result_or_address != 0)
    {
        actor_base = D_80105AE0;
        /* Reuse the result carrier for the actor address to preserve register allocation. */
        result_or_address = (s32)&actor_base[actor_index];
        actor = (Actor *)result_or_address;
        if (actor->unk18E != 0)
        {
            func_800A3938(0x7D, 0x80);
            func_800AF824(actor_index);
            return;
        }
        if (actor->unk18 & 2)
        {
            func_80098FC4(&D_800FDF58[actor_index], 0);
            entry->unk2A = 0x81;
            entry->unk2E = 1;
            if (g_field_resource_entries[entry->unk3B].flags & 1)
            {
                entry->unk21 = (u8)(entry->unk21 & 0x80);
            }
            else
            {
                direction = entry->unk21;
                masked = direction & 0x7F;
                entry->unk21 = (masked % 5) | (direction & 0x80);
            }
            entry->unk27 = 0;
            entry->unk24 = 1;
            field_restart_actor_animation(entry, g_field_resource_entries[entry->unk3B].start);
        }
    }
}

/* ------------------------------------------------------------------------- */
/* func_80098FC4 (0x80098FC4)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Forwards an object-state value and selected halfword to the field
 *        state handler.
 *
 * @param object Field object containing the state-array index.
 * @param entryIndex Index of the halfword entry to forward.
 */
void func_80098FC4(FieldObject80098FC4 *object, s32 entryIndex)
{
    typedef struct FieldState80098FC4
    {
        u8 pad0[0x14];
        s32 value;
        u8 pad18[2];
        u16 entries[2];
        u8 pad1E[0x23C - 0x1E];
    } FieldState80098FC4;

    extern FieldState80098FC4 D_80105AE0[];
    extern void func_800B22F0(s32 value, u16 entry, FieldState80098FC4 *states);

    FieldState80098FC4 *state;

    state = &D_80105AE0[object->stateIndex];
    func_800B22F0(state->value, state->entries[entryIndex], D_80105AE0);
}

/* ------------------------------------------------------------------------- */
/* field_collect_effect_hits (0x80099018)                                    */
/* ------------------------------------------------------------------------- */

/**
 * @brief Collect new effect-centered hit contacts and dispatch their reactions.
 * @note 100% match (gcc272_cdk): 509 instructions, 0x7F4 bytes, 0x38-byte frame.
 * The controller, Y-offset, and retirement scopes preserve compiler loop notes;
 * split cursor advances retain the original register-allocation priorities.
 * @param effect Motion record supplying the hit origin and source object index.
 * @param radius Expansion of candidate projected X/Y bounds and the depth gate.
 * @param actor Owner whose target tracks, contact offsets, and reaction selector are updated.
 * @note New targets append to track_object_indices, set active_track_mask, and
 * increase track_count. Eligible records already in that list are skipped.
 * @note Candidate byte cursors preserve packed flag accesses and the current
 * compiler's repeated-read behavior. This function does not directly subtract HP.
 */
void field_collect_effect_hits(FieldMotionRecord* effect, s32 radius, FieldActorState* actor)
{
    void func_8008A840(s32, s32);
    void func_8008A9D8(s32, s32, s32);
    void func_8008BC5C(FieldMotionRecord*);
    void func_800A2DD8(s32);
    extern FieldMotionRecord D_800FDF58[];
    extern u8 D_80105880[], D_80105AE0[];
    extern s32 D_800FE754, D_8010D020;

    typedef struct
    {
        u8 pad[12];
        s32 object;
    } Controller;
    FieldMotionRecord* motion_records;
    u8* actor_slots;
    u8* controller_slots;
    Controller* controller;
    s32 controller_index;
    s32 bound_x_a;
    s32 bound_y_a;
    s16 candidate_kind;
    s32 bound_x_b;
    s32 bound_y_b;
    s32 min_y;
    s32 max_y;
    s32 min_x;
    s32 max_x;
    s32* candidate_position;
    s32* candidate_record;
    s32 candidate_flags;
    s32 delta_z;
    s32 origin_z;
    s32 depth_projection;
    s32 candidate_z_value;
    s32 candidate_x;
    s32 eligible_for_contact;
    s32 end_index;
    s32 controller_offset;
    s32 active_controller_offset;
    s32 rounded_delta_z;
    s32 existing_hit_index;
    s32 existing_contact_index;
    s32 depth_distance;
    u16 depth_radius;
    s32 origin_x;
    s32 candidate_index;
    u8 source_object_index;
    u8 previous_hit_count;
    u8 owner_index;
    u8 contact_count;
    u8 previous_state;
    u8* source_object;
    u8* append_object;
    u8* count_object;
    u8* candidate_flags_address;
    u8* candidate_base;
    u8* candidate_depth_address;
    u8* prior_hit_cursor;
    u8* prior_hit_base;
    u8* contact_cursor;

    if (D_8010D020 != 0)
    {
        candidate_index = 0;
        end_index = 13;
    }
    else if (actor->animation->sync_flags & 1)
    {
        if (actor->owner_object_index < 3)
        {
            candidate_index = 0;
            end_index = 3;
        }
        else
        {
            candidate_index = 3;
            end_index = 13;
        }
    }
    else if (actor->owner_object_index < 3)
    {
        candidate_index = 3;
        end_index = 13;
    }
    else
    {
        candidate_index = 0;
        end_index = 3;
    }
    candidate_position = (s32*)((u8*)D_800FDF58 + candidate_index * 0x54);
    candidate_base = (candidate_index * 0x23C) + D_80105AE0;
    if (candidate_index < end_index)
    {
        candidate_flags_address = candidate_base + 0xC;
        candidate_depth_address = (u8*)candidate_position + 8;
        candidate_record = candidate_position;
        motion_records = D_800FDF58;
        actor_slots = D_80105AE0;
        controller_slots = D_80105880;
    next_candidate:
    {
        if (*(s32*)(candidate_flags_address + (364)) & 0x80)
        {
            source_object_index = effect->source_object_index;
            eligible_for_contact = 0;
            if (motion_records[source_object_index].motion_parameter == 0x91)
            {
                source_object = (u8*)((source_object_index * 0x23C) + (s32)actor_slots);
                previous_hit_count = *(u8*)(source_object + (379));
                existing_hit_index = 0;
                if (previous_hit_count != 0)
                {
                    prior_hit_base = source_object;
                find_prior_hit:
                    prior_hit_cursor = prior_hit_base + existing_hit_index;
                    existing_hit_index += 1;
                    if (*(u8*)(prior_hit_cursor + (384)) != candidate_index)
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
        if ((candidate_index != owner_index) && ((*(s32*)(candidate_flags_address + (316)) != 0) || (*(s32*)(candidate_flags_address + (324)) != 0)) &&
            ((*(s32*)(candidate_flags_address + (308)) != 0) || (*(s32*)(candidate_flags_address + (312)) != 0)) &&
            (*(s32*)(candidate_flags_address + (288)) != 0) && (candidate_kind = *(s16*)(candidate_depth_address + (34)), (candidate_kind != 0x91)) &&
            (candidate_kind != 0xAE) && (candidate_kind != 0x87) && (*(u8*)(candidate_depth_address + (29)) != 0xFF) && (owner_index != candidate_index) &&
            (*(s32*)(candidate_flags_address + (-8)) != 0) && (candidate_flags = *(s32*)(candidate_flags_address + (364)), ((candidate_flags & 1) == 0)) &&
            ((candidate_index < 3) || ((*(s32*)(candidate_flags_address + 4) & 0xF) == D_800FE754)) && ((candidate_flags & 0x20) == 0) &&
            (eligible_for_contact != 0) && !(*(s32*)(candidate_flags_address + (360)) & 0x8000))
        {
            if (!(candidate_flags & 0x40))
            {
                if ((u8) * (u8*)(candidate_depth_address + (50)) < 2U)
                {
                    controller_offset = *(u8*)(candidate_depth_address + (50)) * 0x1C;
                }
                else
                {
                    controller_offset = 0x38;
                }
                /* Keep the address calculation separate from the two loads. */
                do
                {
                    controller = (Controller*)(controller_slots + controller_offset);
                } while (0);
                controller_index = *(u8*)(candidate_depth_address + 50);
                candidate_flags = controller->object;
                if (candidate_flags == controller_index)
                {
                    if ((u32)(candidate_flags & 0xFF) < 2U)
                    {
                        active_controller_offset = candidate_flags * 0x1C;
                    }
                    else
                    {
                        active_controller_offset = 0x38;
                    }
                    if (*(s32*)(controller_slots + active_controller_offset) == 0)
                    {
                        goto check_unique_contact;
                    }
                    goto advance_candidate;
                }
                goto check_unique_contact;
            }
        check_unique_contact:
            if (!(*(s32*)(candidate_flags_address + (0)) & 0x2280))
            {
                contact_count = actor->track_count;
                existing_contact_index = 0;
                if (contact_count != 0)
                {
                find_existing_contact:
                    contact_cursor = (u8*)actor + existing_contact_index;
                    if (candidate_index != *(u8*)(contact_cursor + (553)))
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
                    candidate_z_value = *(s32*)(candidate_depth_address + (0));
                    origin_z = effect->z;
                    delta_z = candidate_z_value - origin_z;
                    depth_distance = (candidate_z_value - origin_z) / 384;
                    depth_radius = *(u16*)(candidate_flags_address + 290);
                    if (depth_distance < 0)
                    {
                        depth_distance = -depth_distance;
                    }
                    depth_distance = depth_distance < (radius + ((s32)(depth_radius << 0x10) >> 0x11));
                    if (depth_distance)
                    {
                        bound_x_a = *(s16*)(candidate_flags_address + 0x134);
                        bound_x_b = *(s16*)(candidate_flags_address + 0x138);
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
                        bound_y_a = *(s16*)(candidate_flags_address + (310));
                        bound_y_b = *(s16*)(candidate_flags_address + (314));
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
                        if (delta_z < 0)
                        {
                            rounded_delta_z = delta_z + 0x1FF;
                        }
                        else
                        {
                            rounded_delta_z = delta_z;
                        }
                        delta_z = rounded_delta_z >> 9;
                        depth_projection = (s32)(delta_z + ((u32)rounded_delta_z >> 0x1F)) >> 1;
                        min_y -= depth_projection;
                        max_y -= depth_projection;
                        candidate_x = *candidate_position;
                        origin_x = effect->x;
                        if (((candidate_x + (min_x << 8)) < origin_x) && (origin_x < (candidate_x + (max_x << 8))) &&
                            (candidate_x = *(s32*)(candidate_depth_address + (-4)), origin_x = effect->y, (((candidate_x + (min_y << 8)) < origin_x) != 0)) &&
                            (origin_x < (candidate_x + (max_y << 8))) && ((u8) * (u8*)(actor_slots + actor->owner_object_index * 0x23C + 0x17B) < 9U))
                        {
                            *(s32*)(candidate_flags_address + (364)) = (s32)(*(s32*)(candidate_flags_address + (364)) | 0x80);
                            *(s32*)(candidate_flags_address + (0)) = (s32)(*(s32*)(candidate_flags_address + (0)) & ~0x400);
                            append_object = (u8*)((actor->owner_object_index * 0x23C) + (s32)actor_slots);
                            *(u8*)(append_object + append_object[0x17B] + 0x180) = candidate_index;
                            count_object = (u8*)((actor->owner_object_index * 0x23C) + (s32)actor_slots);
                            *(u8*)(count_object + (379)) = (u8)(*(u8*)(count_object + (379)) + 1);
                            /* Each new contact becomes an active target track. */
                            actor->active_track_mask = (u8)(actor->active_track_mask | (1 << actor->track_count));
                            actor->track_object_indices[actor->track_count] = candidate_index;
                            if (*(u8*)(candidate_depth_address + 25) & 0x80)
                            {
                                actor->track_offsets[actor->track_count].x = (*candidate_position - effect->x) >> 8;
                            }
                            else
                            {
                                actor->track_offsets[actor->track_count].x = (effect->x - *candidate_position) >> 8;
                            }
                            do
                            {
                                actor->track_offsets[actor->track_count].y =
                                    ((effect->y - *(s32*)(candidate_depth_address - 4)) >> 8) - ((effect->z - *(s32*)candidate_depth_address) >> 9);
                            } while (0);
                            /* Retirement preserves the previous state in byte +0x26. */
                            if (effect->flags & FIELD_EFFECT_RETIRE_ON_HIT)
                            {
                                do
                                {
                                    previous_state = effect->state;
                                    effect->state = FIELD_EFFECT_RETIRED;
                                    effect->height_or_retired_state = previous_state;
                                } while (0);
                            }
                            actor->track_count = (u8)(actor->track_count + 1);
                            func_8008BC5C((FieldMotionRecord*)candidate_record);
                            if ((candidate_index < 2) && !(*(u16*)((u8*)candidate_record + 0x1C) & 0x1FF))
                            {
                                func_800A2DD8(candidate_index);
                            }
                            if (((u8)actor->hit_reaction < 0xCU) || (motion_records[actor->owner_object_index].motion_parameter == 0xBC))
                            {
                                func_8008A9D8(actor->owner_object_index, candidate_index, actor->hit_reaction);
                            }
                            else
                            {
                                switch (actor->hit_reaction)
                                {
                                case 0x34:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x16U);
                                    break;
                                case 0x50:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x12U);
                                    break;
                                case 0x51:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x13U);
                                    break;
                                case 0x4E:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x14U);
                                    break;
                                case 0x4F:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x15U);
                                    break;
                                case 0x3E:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x19U);
                                    break;
                                case 0x45:
                                    func_8008A9D8(actor->owner_object_index, candidate_index, 0x1AU);
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
    advance_candidate:
        /* Split advances retain the original GCC cursor allocation. */
        candidate_record += 20;
        candidate_record += 1;
        candidate_index += 1;
        candidate_depth_address += 0x54;
        candidate_position += 21;
        candidate_flags_address += 0x234;
        candidate_flags_address += 4;
        candidate_flags_address += 4;
    }
        if (candidate_index < end_index)
        {
            goto next_candidate;
        }
    }
}

/* ------------------------------------------------------------------------- */
/* func_8009980C (0x8009980C)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Find the first eligible actor within the adjusted GTE distance threshold.
 * @param arg0 Reference position in fixed-point coordinates.
 * @param arg1 Distance threshold before adding half the candidate radius.
 * @param arg2 Actor selecting the search group and optional self exclusion.
 * @param arg3 Nonzero to select the opposing group and exclude the input actor.
 * @return Matching actor index, or -1 when no eligible actor is close enough.
 */
s32 func_8009980C(s32 *arg0, s32 arg1, u8 *arg2, s32 arg3)
{
    extern u8 D_800FDF58[];
    extern u8 D_80105AE0[];

    s32 *delta = (s32 *)0x1F800080;
    s32 *sqr = (s32 *)0x1F800090;
    u8 *var_s3;
    s32 var_s0;
    s32 var_s4;
    s32 var_v0;
    u8 temp_v1;
    u8 *var_s1;
    u8 *var_s2;
    u8 *actor_base;
    if (arg3 != 0)
    {
        if (*(u16 *)(*(u8 **)(arg2 + 0xC) + 0x18) & 1)
        {
            var_s0 = 0;
            if (arg2[0x228] >= 3U)
            {
                var_s0 = 3;
                goto block_5;
            }
            goto block_7;
        }
        var_s0 = 3;
        if (arg2[0x228] < 3U)
        {
        block_5:
            var_s4 = 0xD;
        }
        else
        {
            goto block_6;
        }
    }
    else
    {
    block_6:
        var_s0 = 0;
    block_7:
        var_s4 = 3;
    }
    var_s3 = var_s0 * 0x54 + D_800FDF58;
    actor_base = var_s0 * 0x23C + D_80105AE0;
    var_v0 = -1;
    if (var_s0 < var_s4)
    {
        var_s2 = actor_base + 0x12E;
        var_s1 = var_s3 + 8;
    loop_10:
        temp_v1 = var_s1[0x1D];
        if (temp_v1 == 0xFF || *(s32 *)(var_s2 - 0x12A) == 0 || temp_v1 == 0xFE ||
            (arg3 != 0 && arg2[0x228] == var_s0))
        {
            goto next;
        }
        {
            s32 actor_x = *(s32 *)var_s3;
            s32 reference_x;
            do
            {
                reference_x = arg0[0];
            } while (0);
            delta[0] = (actor_x - reference_x) >> 8;
        }
        delta[1] = (*(s32 *)(var_s1 - 4) - arg0[1]) >> 8;
        delta[2] = (*(s32 *)var_s1 - arg0[2]) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(sqr);
        if (SquareRoot0(sqr[0] + sqr[1] + sqr[2]) < arg1 + ((s32)(*(u16 *)var_s2 << 16) >> 17))
        {
            return var_s0;
        }
    next:
        var_s0++;
        var_s1 += 0x54;
        var_s3 += 0x54;
        var_s2 += 0x23C;
        if (var_s0 < var_s4)
        {
            goto loop_10;
        }
    }
    return -1;
}

/* ------------------------------------------------------------------------- */
/* func_80099A48 (0x80099A48)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Find eligible actors intersecting an attack's collision spheres and apply reactions.
 * @param actor Attacking actor state, including its target list and attack mode.
 * @param part Actor part used to obtain the attack radius and sphere centers.
 * @note Eligibility checks retain repeated reads of actor flags and target counts.
 * @note Distances use the GTE square operation followed by SquareRoot0.
 * @see decomp.me (100%)
 */
void func_80099A48(void* actor, void* part)
{
    s32 func_8007E754(void*, void*);
    void func_8007ECEC(void*, void*, void*, s32);
    s32 func_8008A840(s32, s32);
    s32 func_8008A9D8(s32, s32, s32);
    void func_8008BC5C(void*);
    void func_800A2DD8(s32);
    extern u8 D_800FDF58[], D_80105880[], D_80105AE0[];
    extern s32 D_800FE754, D_8010D020;
    /** @brief State halfword within a 0x54-byte actor position record. */
    typedef struct
    {
        u8 pad[0x2A];
        s16 state;
    } FieldTargetState;
    /** @brief Existing target count within a 0x23C-byte actor slot. */
    typedef struct
    {
        u8 pad[0x17B];
        u8 count;
    } FieldActorTargetCount;
    /** @brief Accessed fields of a 0x1C-byte controller slot. */
    typedef struct
    {
        s32 active;
        u8 pad[8];
        s32 object;
    } FieldTargetController;

    u8* controllers;
    u8* positions;
    u8* actors;
    s32* delta = (s32*)0x1F800080;
    s32* squares = (s32*)0x1F800090;
    s32 target_end;
    s32 sphere_count;
    s32 attack_radius;
    s32 sphere_base;
    s32 checked_position_offset;
    s32* checked_position;
    s32* position_cursor;
    s32 position_offset;
    s16 target_state;
    s32* target_position;
    s32 target_flags;
    s32 controlled_actor;
    s32 controller_index;
    s32 initial_position_offset;
    s32 eligible;
    s32 sphere_address;
    s32 sphere_index;
    s32 controller_offset;
    s32 active_controller_offset;
    s32 existing_index;
    s32 hit_index;
    s32 target_index;
    u8 source_index;
    u8 existing_count;
    u8 hit_count;
    void* source_slot;
    void* existing_base;
    void* existing_cursor;
    void* append_slot;
    void* count_slot;
    void* target_base;
    void* target_z_address;

    sphere_base = 0x1F8000A0;
    attack_radius = func_8007E754(actor, part);
    func_8007ECEC(actor, part, (void*)0x1F8000A0, 0);
    if ((S32_AT(actor, 0x224) & 0x1E) == 8)
    {
        func_8007ECEC(actor, part, (void*)0x1F8000B0, 1);
        func_8007ECEC(actor, part, (void*)0x1F8000C0, 2);
        sphere_count = 3;
    }
    else
    {
        sphere_count = 1;
    }
    if (D_8010D020 != 0)
    {
        target_index = 0;
        target_end = 13;
    }
    else if (U16_AT(S32_AT(actor, 0xC), 0x18) & 1)
    {
        if (U8_AT(actor, 0x228) < 3)
        {
            target_index = 0;
            target_end = 3;
        }
        else
        {
            target_index = 3;
            target_end = 13;
        }
    }
    else if (U8_AT(actor, 0x228) < 3)
    {
        target_index = 3;
        target_end = 13;
    }
    else
    {
        target_index = 0;
        target_end = 3;
    }
    initial_position_offset = target_index * 0x54;
    target_position = (s32*)(initial_position_offset + (s32)D_800FDF58);
    target_base = (void*)((target_index * 0x23C) + (s32)D_80105AE0);
    if (target_index < target_end)
    {
        void* target_flags_address = target_base + 0xC;
        target_z_address = (u8*)target_position + 8;
        position_cursor = target_position;
        position_offset = initial_position_offset;
        controllers = D_80105880;
        positions = D_800FDF58;
        actors = D_80105AE0;
    next_target:
        if (S32_AT(target_flags_address, 0x16C) & 0x80)
        {
            source_index = U8_AT(actor, 0x228);
            eligible = 0;
            if (((FieldTargetState*)(positions + source_index * 0x54))->state == 0x91)
            {
                source_slot = (void*)((source_index * 0x23C) + (s32)actors);
                existing_count = U8_AT(source_slot, 0x17B);
                existing_index = 0;
                if (existing_count != 0)
                {
                    existing_base = source_slot;
                scan_existing_targets:
                    existing_cursor = existing_base + existing_index;
                    existing_index += 1;
                    if (U8_AT(existing_cursor, 0x180) != target_index)
                    {
                        if (existing_index >= (s32)existing_count)
                        {
                        }
                        else
                        {
                            goto scan_existing_targets;
                        }
                    }
                    else
                    {
                        goto mark_eligible;
                    }
                }
            }
        }
        else
        {
        mark_eligible:
            eligible = 1;
        }
        if ((target_index != U8_AT(actor, 0x228)) && (S32_AT(target_flags_address, 0x120) != 0))
        {
            target_state = S16_AT(target_z_address, 0x22);
            if ((target_state != 0x91) && (target_state != 0xAE) && (target_state != 0x87) &&
                ((target_index >= 2) || ((U8_AT(target_z_address, 0x19) & 0x7F) != 0x3C)) && (U8_AT(target_z_address, 0x1D) != 0xFF) &&
                (U8_AT(actor, 0x228) != target_index) && (S32_AT(target_flags_address, -0x8) != 0))
            {
                target_flags = S32_AT(target_flags_address, 0x16C);
                if (!(target_flags & 1) && ((((target_index < 3) != 0)) || (((S32_AT(target_flags_address, 0x4) & 0xF) == D_800FE754))) &&
                    ((target_flags & 0x20) == 0) && (eligible != 0) && !(S32_AT(target_flags_address, 0x168) & 0x8000))
                {
                    if (!(target_flags & 0x40))
                    {
                        if ((u8)U8_AT(target_z_address, 0x32) < 2U)
                        {
                            controller_offset = U8_AT(target_z_address, 0x32) * 0x1C;
                        }
                        else
                        {
                            controller_offset = 0x38;
                        }
                        {
                            FieldTargetController* controller = (FieldTargetController*)(controllers + controller_offset);
                            controller_index = U8_AT(target_z_address, 0x32);
                            controlled_actor = controller->object;
                        }
                        if (controlled_actor == controller_index)
                        {
                            if ((u32)(controlled_actor & 0xFF) < 2U)
                            {
                                active_controller_offset = controlled_actor * 0x1C;
                            }
                            else
                            {
                                active_controller_offset = 0x38;
                            }
                            if (S32_AT(controllers, active_controller_offset) == 0)
                            {
                                goto check_target_list;
                            }
                        }
                        else
                        {
                            goto check_target_list;
                        }
                    }
                    else
                    {
                    check_target_list:
                        if (!(S32_AT(target_flags_address, 0x0) & 0x2280))
                        {
                            hit_count = U8_AT(actor, 0x232);
                            hit_index = 0;
                            if (hit_count != 0)
                            {
                            scan_hit_targets:
                                if (target_index != U8_AT(actor + hit_index, 0x229))
                                {
                                    hit_index += 1;
                                    if (hit_index < (s32)hit_count)
                                    {
                                        goto scan_hit_targets;
                                    }
                                }
                            }
                            if (hit_index == U8_AT(actor, 0x232))
                            {
                                sphere_address = sphere_base;
                                sphere_index = 0;
                                if (sphere_count != 0)
                                {
                                    checked_position_offset = position_offset;
                                    checked_position = position_cursor;
                                    do
                                    {
                                        delta[0] = (s32)((s32)(*target_position - S32_AT(sphere_address, 0x0)) >> 8);
                                        delta[1] = (s32)((s32)(S32_AT(target_z_address, -0x4) - S32_AT(sphere_address, 0x4)) >> 8);
                                        delta[2] = (s32)((s32)(S32_AT(target_z_address, 0x0) - S32_AT(sphere_address, 0x8)) >> 8);
                                        gte_ldlvl(delta);
                                        gte_sqr0();
                                        gte_stlvnl(squares);
                                        if ((SquareRoot0(squares[0] + squares[1] + squares[2]) <
                                             (attack_radius + ((s16)U16_AT(target_flags_address, 0x122) >> 1))) &&
                                            ((u8)((FieldActorTargetCount*)(D_80105AE0 + U8_AT(actor, 0x228) * 0x23C))->count < 9U))
                                        {
                                            S32_AT(target_flags_address, 0x16C) = (s32)(S32_AT(target_flags_address, 0x16C) | 0x80);
                                            S32_AT(target_flags_address, 0x0) = (s32)(S32_AT(target_flags_address, 0x0) & ~0x400);
                                            append_slot = (void*)((U8_AT(actor, 0x228) * 0x23C) + (s32)D_80105AE0);
                                            U8_AT(append_slot, U8_AT(append_slot, 0x17B) + 0x180) = target_index;
                                            count_slot = (void*)((U8_AT(actor, 0x228) * 0x23C) + (s32)D_80105AE0);
                                            U8_AT(count_slot, 0x17B) = (u8)(U8_AT(count_slot, 0x17B) + 1);
                                            U8_AT(actor, 0x23A) = (u8)(U8_AT(actor, 0x23A) | (1 << U8_AT(actor, 0x232)));
                                            U8_AT(actor, U8_AT(actor, 0x232) + 0x229) = target_index;
                                            U8_AT(actor, 0x232) = (u8)(U8_AT(actor, 0x232) + 1);
                                            if ((target_index < 2) && !(U16_AT(checked_position, 0x1C) & 0x1FF))
                                            {
                                                func_800A2DD8(target_index);
                                            }
                                            func_8008BC5C((void*)(checked_position_offset + (s32)D_800FDF58));
                                            if (((u8)U8_AT(actor, 0x26) < 0xCU) ||
                                                (((FieldTargetState*)(D_800FDF58 + U8_AT(actor, 0x228) * 0x54))->state == 0xBC))
                                            {
                                                func_8008A9D8(U8_AT(actor, 0x228), target_index, U8_AT(actor, 0x26));
                                            }
                                            else
                                            {
                                                switch (U8_AT(actor, 0x26))
                                                {
                                                case 0x50:
                                                    func_8008A9D8(U8_AT(actor, 0x228), target_index, 0x12U);
                                                    break;
                                                case 0x51:
                                                    func_8008A9D8(U8_AT(actor, 0x228), target_index, 0x13U);
                                                    break;
                                                case 0x4E:
                                                    func_8008A9D8(U8_AT(actor, 0x228), target_index, 0x14U);
                                                    break;
                                                case 0x4F:
                                                    func_8008A9D8(U8_AT(actor, 0x228), target_index, 0x15U);
                                                    break;
                                                case 0x3E:
                                                    func_8008A9D8(U8_AT(actor, 0x228), target_index, 0x19U);
                                                    break;
                                                case 0x45:
                                                    func_8008A9D8(U8_AT(actor, 0x228), target_index, 0x1AU);
                                                    break;
                                                default:
                                                    func_8008A840(U8_AT(actor, 0x228), target_index);
                                                    break;
                                                }
                                            }
                                        }
                                        sphere_index += 1;
                                        sphere_address += 0x10;
                                    } while (sphere_index < sphere_count);
                                }
                            }
                        }
                    }
                }
            }
        }
        target_index += 1;
        target_z_address += 0x54;
        target_position = (s32*)((u8*)target_position + 0x54);
        target_flags_address += 0x23C;
        position_cursor = (s32*)((u8*)position_cursor + 0x54);
        position_offset += 0x54;
        if (target_index < target_end)
        {
            goto next_target;
        }
    }
}

/* ------------------------------------------------------------------------- */
/* func_8009A204 (0x8009A204)                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Return the GTE-computed distance between positions @p a and @p b.
 * @param a First position (vx/vy/vz).
 * @param b Second position (vx/vy/vz).
 * @return sqrt(sum of squared per-axis deltas), each delta scaled by >> 8.
 */
s32 func_8009A204(FieldVector *a, FieldVector *b)
{
    FieldVector *delta = (FieldVector *)0x1F800080;
    FieldVector *sqr = (FieldVector *)0x1F800090;

    delta->vx = (a->vx - b->vx) >> 8;
    delta->vy = (a->vy - b->vy) >> 8;
    delta->vz = (a->vz - b->vz) >> 8;
    gte_ldlvl(delta);
    gte_sqr0();
    gte_stlvnl(sqr);
    return SquareRoot0(sqr->vx + sqr->vy + sqr->vz);
}
