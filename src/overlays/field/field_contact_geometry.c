/** @file field_contact_geometry.c
 * @brief Resolve field actor contacts, movement, interactions, and attack hits.
 */
#include "common.h"
#include "field_effect_transform.h"
#include "cdrom.h"
#include "field_types.h"
#include "field_actor_runtime.h"
#include "field_effect_types.h"
#include "field_effect_geometry.h"
#include "field_contact_geometry.h"
#include "field_actor_sequence_runtime.h"
#include "vector.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/* These cursors point at a member inside each record, not at its base. */
#define FIELD_CONTAINER(ptr, type, member) ((type*)((u8*)(ptr) - (s32) & ((type*)0)->member))

#define FIELD_CONTACT_RESOURCE_ID 0x5DD
#define FIELD_CONTACT_RESOURCE_BLOCK_COUNT 11
#define FIELD_CONTACT_RESOURCE_ROW_COUNT 24
#define FIELD_CONTACT_RESOURCE_ROW_SIZE 32
#define FIELD_PARTY_ACTOR_COUNT 3
#define FIELD_RUNTIME_ACTOR_COUNT 13
#define FIELD_QUAD_VERTEX_COUNT 4
#define FIELD_MAX_CONTACT_TARGETS 9
#define FIELD_NO_SEGMENT_INTERSECTION 0x80008000
#define FIELD_FACING_INDEX_MASK 0x7F
#define FIELD_FACING_HIGH_BIT 0x80
#define FIELD_OBJECT_GROUP_MASK 0xF
#define FIELD_MOTION_RADIUS_MASK 0x1FF
#define FIELD_ACTOR_INDEX_MASK 0xFF
#define FIELD_CONTACT_RESULT_ACTOR_MASK 0x7FFF
#define FIELD_CONTACT_RESULT_PRESENT 0x8000
#define FIELD_OBJECT_IGNORE_MAP_COLLISION_BIT 23
#define FIELD_ANIMATION_SAME_GROUP 0x01
#define FIELD_RESOURCE_CONTACT_REACTION 0x01
#define FIELD_ACTOR_STATE_UNUSED 0xFF
#define FIELD_ACTOR_STATE_INACTIVE 0xFE
#define FIELD_PROJECTED_DEPTH_SCALE 384
#define FIELD_CONTACT_HEIGHT_SCALE 224
#define FIELD_ACTOR_ACTION_KIND_MASK 0x1E
#define FIELD_ACTOR_ACTION_KIND_ATTACK 8
#define FIELD_MOVEMENT_THRESHOLD_BLOCKED_BIT 13
#define FIELD_MOVEMENT_ACTOR_BLOCKED_BIT 14

#define FIELD_OBJECT_FLAG_CONTACT_FILTER_0004 0x0004
#define FIELD_OBJECT_FLAG_CONTACT_FILTER_0020 0x0020
#define FIELD_OBJECT_FLAG_CONTACT_FILTER_0040 0x0040
#define FIELD_OBJECT_FLAG_CONTACT_FILTER_0080 0x0080
#define FIELD_OBJECT_FLAG_CONTACT_FILTER_0100 0x0100
#define FIELD_OBJECT_FLAG_CONTACT_FILTER_0200 0x0200
#define FIELD_OBJECT_FLAG_CLEAR_ON_HIT 0x0400
#define FIELD_OBJECT_FLAG_CONTACT_FILTER_2000 0x2000

#define FIELD_CONTACT_FLAG_REQUIRE_CONTROLLER 0x01
#define FIELD_CONTACT_FLAG_DISABLED 0x02
#define FIELD_CONTACT_REACTION_BIT_04 0x04
#define FIELD_CONTACT_REACTION_BIT_08 0x08
#define FIELD_CONTACT_REACTION_BIT_10 0x10
#define FIELD_CONTACT_FLAG_NO_HIT_TEST 0x20
#define FIELD_CONTACT_FLAG_IGNORE_BINDING 0x40
#define FIELD_CONTACT_FLAG_TARGETED 0x80

#define FIELD_MOVEMENT_FLAG_THRESHOLD_BLOCKED 0x2000
#define FIELD_MOVEMENT_FLAG_ACTOR_BLOCKED 0x4000
#define FIELD_MOVEMENT_FLAG_NO_CONTACT 0x8000

#define FIELD_INTERACTION_FLAG_ENABLED 0x01
#define FIELD_INTERACTION_FLAG_TRIGGERED 0x02

/* Map dimensions are addressed by a literal LUI/ORI pair in the original. */
#define FIELD_MAP_BOUNDS_ADDRESS 0x801ED400
#define FIELD_MOVE_REQUEST_ADDRESS 0x1F800010
#define FIELD_MOVE_QUERY_ADDRESS 0x1F800080
#define FIELD_ATTACK_SPHERE_0_ADDRESS 0x1F8000A0
#define FIELD_ATTACK_SPHERE_1_ADDRESS 0x1F8000B0
#define FIELD_ATTACK_SPHERE_2_ADDRESS 0x1F8000C0
#define FIELD_GTE_DELTA_ADDRESS 0x1F800080
#define FIELD_GTE_SQUARE_ADDRESS 0x1F800090

/** @brief Loaded field resource metadata used by contact reactions. */
typedef struct
{
    u8* start;
    u8* end;
    u8 state;
    u8 slot_index;
    u8 pad_0xa[6];
    u32 flags;
} FieldResourceEntry;

/** @brief Visual kind and flags in a field object definition. */
typedef struct
{
    u8 pad_0x0[0x2E];
    u8 visual_kind;
    u8 pad_0x2f[5];
    u32 flags;
    u8 pad_0x38[0x10];
} FieldMoveObject;

/** @brief Scratchpad movement request and collision resolver output. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 delta_x;
    s32 delta_y;
    s32 delta_z;
    s32 height_offset;
    s32 contact;
    s32 surface;
    s16 width;
    s16 height_tolerance;
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
            unsigned unknown_bit_16 : 1;
            unsigned unknown_bit_17 : 1;
            unsigned high : 14;
        } bits;
    } packed;
} FieldMoveRequest;

/** @brief Scratchpad position and footprint used by an obstruction query. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s16 extent_x;
    s16 extent_y;
    s16 extent_z;
} FieldMoveQuery;

/** @brief Map dimensions used for fixed-point movement bounds checks. */
typedef struct
{
    s16 x;
    u16 depth;
} FieldMoveBounds;

/** @brief Minimum and span for one movement threshold band. */
typedef struct
{
    u16 min;
    u16 span;
} FieldThreshold;

/** @brief Animation targets and GTE vectors used by a contact scan. */
typedef struct
{
    union
    {
        s32 words[2];
        u8 actor_indices[8];
    } targets;
    VECTOR delta;
    VECTOR squared;
} FieldContactScanWorkspace;

extern FieldMotionRecord g_field_actors[];
extern u8 g_field_actor_sequence_data[];
/** @brief Shared CD resource scratch buffer, also used by audio and dialog loaders. */
extern u8* D_8010D038;
extern FieldMotionRecord g_field_scene_actors[];
extern FieldMoveObject g_field_object_parts[];
extern s32 g_field_active_group;
extern FieldThreshold g_field_group_bounds[];
extern FieldActorState g_field_actor_slots[];
extern FieldObjectRuntime g_field_scene_object_states[];
extern FieldResourceEntry g_field_resource_entries[];
/** @brief Nonzero bypasses party/opponent contact filtering; broader mode semantics are unresolved. */
extern s32 D_8010D020;
/** @brief Last overlap result: zero, or actor index with FIELD_CONTACT_RESULT_PRESENT. */
extern s32 g_field_last_actor_contact;

void func_8008E690(FieldMotionRecord*);
s32 func_8005B368(FieldMoveQuery*);
s32 func_8005B6AC(FieldMoveRequest*);
s32 func_80092988(FieldMotionRecord*, Vec3i*);
s32 func_800839F8(s32, s32);
s32 func_80083EEC(s32, s32, s32);
void func_800A2DD8(s32);
void func_800A3938(s32, s32);
void func_800AF824(s32);
void func_800B22F0(s32 value, u16 entry, FieldObjectRuntime* states);
void func_8008A840(s32, s32);
void func_8008A9D8(s32, s32, s32);
void func_8008BC5C(FieldMotionRecord*);

s32 field_is_outside_group_bounds(FieldMotionRecord* actor, s32* position);
void field_dispatch_object_state_entry(FieldMotionRecord* object, s32 entry_index);

/**
 * @brief Load the field actor sequence bytecode banks from resource 0x5DD.
 *
 * The resource data begins one byte into D_8010D038. It contains 11 blocks
 * of 24 rows, with 32 bytes copied into each destination row.
 */
void field_load_actor_sequence_data(void)
{
    s32 bank_index, row_index, byte_index;
    u8 *source, *row, *destination;
    u8 value;

    cdrom_queue_read(FIELD_CONTACT_RESOURCE_ID, D_8010D038);
    cdrom_wait_queue_empty();
    source = D_8010D038 + 1;
    for (bank_index = 0; bank_index < FIELD_CONTACT_RESOURCE_BLOCK_COUNT; bank_index++)
    {
        for (row_index = 0; row_index < FIELD_CONTACT_RESOURCE_ROW_COUNT; row_index++)
        {
            byte_index = 0;
            row = bank_index * sizeof(u8[FIELD_CONTACT_RESOURCE_ROW_COUNT][FIELD_CONTACT_RESOURCE_ROW_SIZE]) +
                  row_index * sizeof(u8[FIELD_CONTACT_RESOURCE_ROW_SIZE]) + g_field_actor_sequence_data;
            do
            {
                destination = row + byte_index;
                byte_index++;
                value = *source;
                *destination = value;
                source++;
            } while (byte_index < FIELD_CONTACT_RESOURCE_ROW_SIZE);
        }
    }
}

/**
 * @brief Test an actor quad against eligible actors and handle contact reactions.
 * @param quad Four packed screen-space vertices of the tested quadrilateral.
 * @param record Actor record supplying the owner and vertical position.
 * @param contact Output candidate index and packed contact point.
 * @return Zero for no contact, one or two for a collision layer, or three for a handled reaction.
 * @note A failed edge test also writes the FIELD_NO_SEGMENT_INTERSECTION sentinel to contact->point.
 */
s32 field_test_quad_actor_contacts(FieldContactPoint* quad, FieldMotionRecord* record, FieldContactResult* contact)
{
    FieldContactPoint* input_quad;
    FieldMotionRecord* scan_record;
    FieldMotionRecord* owner_record;
    FieldObjectRuntime* scan_state;
    FieldObjectRuntime* owner_state;
    /** @brief Contact centroid and temporary collision workspace. */
    union
    {
        FieldContactPoint center;
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
    s32 request_offset;
    s32 matched_request_offset;
    s32 center_x;
    s32 packed_center;
    s32 center_y;
    s32 vertical_distance;
    u8 owner_index;
    u8 target_count;
    FieldContactPoint* candidate_quad;
    FieldMotionRecord* initial_owner;
    FieldObjectRuntime* target_state;
    FieldObjectRuntime* reaction_state_40;
    FieldObjectRuntime* reaction_state_3f;
    FieldObjectRuntime* reaction_base_3f;
    FieldObjectRuntime* reaction_base_40;
    FieldObjectRuntime* target_base;
    FieldSequenceBinding* request_base;
    FieldSequenceBinding* request;
    FieldActorState* slot_base;
    FieldContactPoint* input_vertex;
    u8* scan_facing_cursor;
    u8* scan_diameter_cursor;
    FieldObjectRuntime* target_list_base;

    owner_index = record->source_object_index;
    initial_owner = &g_field_actors[owner_index];
    owner_record = initial_owner;
    if (initial_owner->motion_parameter == 0)
    {
        return 0;
    }
    goto initialize;
special_3f:
    scan_record->motion_parameter = 0x285;
    func_8008E690(scan_record);
    reaction_base_3f = g_field_object_states;
    reaction_state_3f = &reaction_base_3f[scan_record->source_object_index];
    reaction_state_3f->contact.flags =
        (s32)((reaction_state_3f->contact.flags & ~(FIELD_CONTACT_REACTION_BIT_04 | FIELD_CONTACT_REACTION_BIT_08 | FIELD_CONTACT_REACTION_BIT_10)) |
              FIELD_CONTACT_REACTION_BIT_08);
    return 3;
special_40:
    scan_record->motion_parameter = 0x385;
    func_8008E690(scan_record);
    reaction_base_40 = g_field_object_states;
    reaction_state_40 = &reaction_base_40[scan_record->source_object_index];
    reaction_state_40->contact.flags =
        (s32)((reaction_state_40->contact.flags & ~(FIELD_CONTACT_REACTION_BIT_04 | FIELD_CONTACT_REACTION_BIT_08 | FIELD_CONTACT_REACTION_BIT_10)) |
              FIELD_CONTACT_REACTION_BIT_10);
    return 3;
initialize:
    scan_state = g_field_object_states;
    owner_state = &scan_state[owner_index];
    actor_index = 0;
    scan_record = g_field_actors;
    scan_facing_cursor = (u8*)&g_field_actors[0].facing_or_reward_kind;
    scan_diameter_cursor = (u8*)&scan_state->collision.half.diameter;
    target_base = g_field_object_states;
    request_base = g_field_actor_bindings;
    slot_base = g_field_actor_slots;
scan_actor:
    if ((FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->state != FIELD_ACTOR_STATE_UNUSED) &&
        !(FIELD_CONTAINER(scan_diameter_cursor, FieldObjectRuntime, collision.half.diameter)->object_flags &
          (FIELD_OBJECT_FLAG_CONTACT_FILTER_0080 | FIELD_OBJECT_FLAG_CONTACT_FILTER_0200 | FIELD_OBJECT_FLAG_CONTACT_FILTER_2000)) &&
        ((actor_index < FIELD_PARTY_ACTOR_COUNT) || ((FIELD_CONTAINER(scan_diameter_cursor, FieldObjectRuntime, collision.half.diameter)->group_flags &
                                                      FIELD_OBJECT_GROUP_MASK) == g_field_active_group)) &&
        (FIELD_CONTAINER(scan_diameter_cursor, FieldObjectRuntime, collision.half.diameter)->current_hp != 0) &&
        ((u32)((FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->facing_or_reward_kind & FIELD_FACING_INDEX_MASK) - 0x38) >=
         2U) &&
        (FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->source_object_index != record->source_object_index))
    {
        if (FIELD_CONTAINER(scan_diameter_cursor, FieldObjectRuntime, collision.half.diameter)->contact.flags & FIELD_CONTACT_FLAG_TARGETED)
        {
            eligible = 0;
            if (owner_record->motion_parameter == 0x91)
            {
                target_state = (FieldObjectRuntime*)(owner_record->source_object_index * sizeof(*target_base) + (s32)target_base);
                target_count = target_state->contact.bytes.target_count;
                list_index_or_layer = 0;
                if (target_count != 0)
                {
                    target_list_base = target_state;
                    do
                    {
                        if (target_list_base->targets[list_index_or_layer] == actor_index)
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
        if ((eligible != 0) &&
            !(FIELD_CONTAINER(scan_diameter_cursor, FieldObjectRuntime, collision.half.diameter)->movement.word & FIELD_MOVEMENT_FLAG_NO_CONTACT))
        {
            flags_or_extent = FIELD_CONTAINER(scan_diameter_cursor, FieldObjectRuntime, collision.half.diameter)->contact.flags;
            if (!(flags_or_extent & FIELD_CONTACT_FLAG_NO_HIT_TEST) &&
                (!(flags_or_extent & FIELD_CONTACT_FLAG_REQUIRE_CONTROLLER) ||
                 (slot_base[FIELD_CONTAINER(scan_diameter_cursor, FieldObjectRuntime, collision.half.diameter)->contact.bytes.controller_index]
                      .owner_object_index == record->source_object_index)) &&
                !(flags_or_extent & FIELD_CONTACT_FLAG_DISABLED))
            {
                if (!(flags_or_extent & FIELD_CONTACT_FLAG_IGNORE_BINDING))
                {
                    if ((u8)FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->source_object_index < 2U)
                    {
                        request_offset =
                            FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->source_object_index * sizeof(FieldSequenceBinding);
                    }
                    else
                    {
                        request_offset = 2 * sizeof(FieldSequenceBinding);
                    }

                    request = (FieldSequenceBinding*)((u8*)request_base + request_offset);
                    if (request->owner_object_index == FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->source_object_index)
                    {
                        if ((u32)(request->owner_object_index & FIELD_ACTOR_INDEX_MASK) < 2U)
                        {
                            matched_request_offset = request->owner_object_index * sizeof(FieldSequenceBinding);
                        }
                        else
                        {
                            matched_request_offset = 2 * sizeof(FieldSequenceBinding);
                        }
                        if (((FieldSequenceBinding*)((u8*)request_base + matched_request_offset))->state != 0)
                        {
                            goto next_actor;
                        }
                    }
                }
                candidate_animation = FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->motion_parameter;
                if ((candidate_animation != 0x91) && (candidate_animation != 0x87) && (candidate_animation != 0xAE))
                {
                    if (D_8010D020 != 0 || ((u8)record->source_object_index < FIELD_PARTY_ACTOR_COUNT
                                                ? (u8)FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->source_object_index >= 3U
                                                : (u8)FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->source_object_index < 3U))
                    {
                        vertical_distance =
                            (FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->z - record->z) / FIELD_CONTACT_HEIGHT_SCALE;
                        if (vertical_distance < 0)
                        {
                            vertical_distance = -vertical_distance;
                        }
                        candidate_extent = FIELD_CONTAINER(scan_diameter_cursor, FieldObjectRuntime, collision.half.diameter)->collision.signed_half.extent;
                        flags_or_extent = owner_state->collision.signed_half.extent;
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
                            candidate_quad = (FieldContactPoint*)&scan_state->effect_vertices.points[list_index_or_layer];
                            candidate_edge = 0;
                            do
                            {
                                input_edge = 0;
                                next_edge_offset = ((candidate_edge + 1) & (FIELD_QUAD_VERTEX_COUNT - 1)) * sizeof(*candidate_quad);
                                input_vertex = input_quad;
                                do
                                {
                                    if ((candidate_quad[0].packed != 0) || (candidate_quad[1].packed != 0))
                                    {
                                        intersection = field_intersect_screen_segments(&input_vertex->coord, &input_quad[(input_edge + 1) & 3].coord,
                                                                                       &candidate_quad[candidate_edge].coord,
                                                                                       &((FieldContactPoint*)((u8*)candidate_quad + next_edge_offset))->coord);
                                        contact->point = intersection;
                                        if (intersection != FIELD_NO_SEGMENT_INTERSECTION)
                                        {
                                            if ((u8)FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->source_object_index < 2U)
                                            {
                                                candidate_mode =
                                                    FIELD_CONTAINER(scan_facing_cursor, FieldMotionRecord, facing_or_reward_kind)->facing_or_reward_kind &
                                                    FIELD_FACING_INDEX_MASK;
                                                if (candidate_mode == 0x3F)
                                                {
                                                    goto special_3f;
                                                }
                                                if (candidate_mode == 0x40)
                                                {
                                                    goto special_40;
                                                }
                                            }

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
                                } while (input_edge < FIELD_QUAD_VERTEX_COUNT);
                                candidate_edge += 1;
                            } while (candidate_edge < FIELD_QUAD_VERTEX_COUNT);
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
                            }
                        next_layer:
                            list_index_or_layer -= 4;

                            if (list_index_or_layer < 0)
                            {
                                goto next_actor;
                            }
                            goto scan_layer;
                        }
                    }
                }
            }
        }
    }
next_actor:
    actor_index += 1;
    scan_facing_cursor += sizeof(FieldMotionRecord);
    scan_diameter_cursor += sizeof(FieldObjectRuntime);
    scan_record++;
    scan_state++;
    if (actor_index >= FIELD_RUNTIME_ACTOR_COUNT)
    {
        return 0;
    }
    goto scan_actor;
}

/**
 * @brief Find an integer intersection of two screen-space line segments.
 * @param first_start First segment's starting point.
 * @param first_end First segment's ending point.
 * @param second_start Second segment's starting point.
 * @param second_end Second segment's ending point.
 * @return X in the low 16 bits and Y in the high 16 bits, or FIELD_NO_SEGMENT_INTERSECTION.
 * @note Collinear overlapping segments return the first segment's start.
 * @note Segment bounds are validated by reconstructing each candidate coordinate from endpoint distances.
 */
s32 field_intersect_screen_segments(Vec2s* first_start, Vec2s* first_end, Vec2s* second_start, Vec2s* second_end)
{
    s32 first_start_x_bound, first_end_x_bound;
    s32 first_start_y_bound, first_end_y_bound;
    s32 second_start_x_bound, second_end_x_bound;
    s32 second_start_y_bound, second_end_y_bound;
    s32 second_end_coordinate;
    s32 second_end_x_a;
    s32 second_start_x_value;
    s32 second_start_y_a;
    s32 second_start_y_value_vertical;
    s32 second_start_x_value_general;
    s32 first_end_x_b;
    s32 first_end_y_b;
    s32 first_end_x_collinear;
    s32 second_start_x_b;
    s32 second_start_y_b;
    s32 second_start_x_collinear;
    s32 second_end_x_b;
    s32 second_end_y_b;
    s32 second_end_x_collinear;
    s32 second_start_x_a;
    s32 second_start_y_value;
    s32 second_start_x_value_vertical;
    s32 first_start_x;
    s32 intersection_x;
    s32 intersection_y;
    s32 second_end_x_general;
    s32 second_start_y_general;
    s32 first_start_y;
    s32 first_end_x;
    s32 first_end_y;
    s32 first_start_x_a;
    s32 first_end_x_a;
    s32 first_start_x_b;
    s32 second_end_y_a;
    s32 first_start_y_b;
    s32 first_start_x_collinear;
    s32 first_dy_times_second_dx;
    s32 first_dx_times_second_dy;
    s32 second_dx;
    s32 second_x_delta_vertical;
    s32 packed_y_shift;
    s32 first_dx;
    s32 first_dy;
    s32 second_y_delta_vertical;
    s32 second_y_delta;
    s32 work_value;
    u32 packed_x_or_distance_sum;

    /* Coordinate locals also retain endpoint values before interpolation. */
    first_end_y = first_end->y;
    first_start_y = first_start->y;
    first_dy = first_end_y;
    first_dy -= first_start_y;
    if (first_dy == 0)
    {
        second_end_coordinate = second_end->y;
        second_start_y_value = second_start->y;
        second_y_delta = second_end_coordinate - second_start_y_value;
        intersection_y = first_start_y;
        if (second_y_delta == 0)
        {
            if ((intersection_y == second_start_y_value) &&
                ((second_start_x_a = second_start->x, first_start_x_a = first_start->x, ((second_start_x_a < first_start_x_a) == 0)) ||
                 (second_end_x_a = second_end->x, ((second_end_x_a < first_start_x_a) == 0)) ||
                 (first_end_x_a = first_end->x, ((second_start_x_a < first_end_x_a) == 0)) || (second_end_x_a >= first_end_x_a)) &&
                ((second_start_x_b = second_start->x, first_start_x_b = first_start->x, ((first_start_x_b < second_start_x_b) == 0)) ||
                 (second_end_x_b = second_end->x, ((first_start_x_b < second_end_x_b) == 0)) ||
                 (first_end_x_b = first_end->x, ((first_end_x_b < second_start_x_b) == 0)) || (first_end_x_b >= second_end_x_b)))
            {
                packed_x_or_distance_sum = (u16)first_start->x;
                work_value = intersection_y << 0x10;
                return packed_x_or_distance_sum | work_value;
            }
            return FIELD_NO_SEGMENT_INTERSECTION;
        }
        second_end_coordinate = second_end->x;
        second_start_x_value = second_start->x;
        second_dx = second_end_coordinate - second_start_x_value;
        if (second_dx == 0)
        {
            intersection_x = second_start_x_value;
        }
        else
        {
            intersection_x = ((s32)((intersection_y - second_start_y_value) * second_dx) / second_y_delta) + second_start_x_value;
        }
    }
    else
    {
        first_end_x = first_end->x;
        first_start_x = first_start->x;
        first_dx = first_end_x - first_start_x;
        if (first_dx == 0)
        {
            second_end_coordinate = second_end->x;
            second_start_x_value_vertical = second_start->x;
            packed_y_shift = 16;
            second_x_delta_vertical = second_end_coordinate - second_start_x_value_vertical;
            intersection_x = first_start_x;
            if (second_x_delta_vertical == 0)
            {
                if ((intersection_x == second_start_x_value_vertical) && ((second_start_y_a = second_start->y, ((second_start_y_a < first_start_y) == 0)) ||
                                                                          (second_end_y_a = second_end->y, ((second_end_y_a < first_start_y) == 0)) ||
                                                                          (second_start_y_a >= first_end_y) || (second_end_y_a >= first_end_y)))
                {
                    second_start_y_b = second_start->y;
                    first_start_y_b = first_start->y;
                    if ((first_start_y_b >= second_start_y_b) || (second_end_y_b = second_end->y, ((first_start_y_b < second_end_y_b) == 0)) ||
                        (first_end_y_b = first_end->y, ((first_end_y_b < second_start_y_b) == 0)) || (first_end_y_b >= second_end_y_b))
                    {
                        packed_x_or_distance_sum = intersection_x & 0xFFFF;
                        work_value = first_start->y << packed_y_shift;
                        return packed_x_or_distance_sum | work_value;
                    }
                    return FIELD_NO_SEGMENT_INTERSECTION;
                }
                return FIELD_NO_SEGMENT_INTERSECTION;
            }
            second_end_coordinate = second_end->y;
            second_start_y_value_vertical = second_start->y;
            second_y_delta_vertical = second_end_coordinate - second_start_y_value_vertical;
            if (second_y_delta_vertical == 0)
            {
                intersection_y = second_start_y_value_vertical;
            }
            else
            {
                intersection_y = ((s32)((intersection_x - second_start_x_value_vertical) * second_y_delta_vertical) / second_x_delta_vertical) +
                                 second_start_y_value_vertical;
            }
        }
        else
        {
            second_end_coordinate = second_end->y;
            second_start_y_general = second_start->y;
            second_y_delta = second_end_coordinate - second_start_y_general;
            if (second_y_delta == 0)
            {
                intersection_y = second_start_y_general;
                intersection_x = ((s32)((intersection_y - first_start_y) * first_dx) / first_dy) + first_start_x;
            }
            else
            {
                second_end_x_general = second_end->x;
                second_start_x_value_general = second_start->x;
                second_dx = second_end_x_general - second_start_x_value_general;
                if (second_dx == 0)
                {
                    intersection_x = second_start_x_value_general;
                }
                else
                {
                    first_dy_times_second_dx = first_dy * second_dx;
                    first_dx_times_second_dy = first_dx * second_y_delta;
                    if (first_dy_times_second_dx == first_dx_times_second_dy)
                    {
                        if ((second_start_y_general == (((s32)((second_start_x_value_general - first_start_x) * first_dy) / first_dx) + first_start_y)) &&
                            ((second_start_x_value_general >= first_start_x) || (second_end_x_general >= first_start_x) ||
                             (second_start_x_value_general >= first_end_x) || (second_end_x_general >= first_end_x)))
                        {
                            second_start_x_collinear = second_start->x;
                            first_start_x_collinear = first_start->x;
                            if ((first_start_x_collinear >= second_start_x_collinear) ||
                                (second_end_x_collinear = second_end->x, ((first_start_x_collinear < second_end_x_collinear) == 0)) ||
                                (first_end_x_collinear = first_end->x, ((first_end_x_collinear < second_start_x_collinear) == 0)) ||
                                (first_end_x_collinear >= second_end_x_collinear))
                            {
                                packed_x_or_distance_sum = (u16)first_start->x;
                                work_value = first_start->y << 0x10;
                                return packed_x_or_distance_sum | work_value;
                            }
                            return FIELD_NO_SEGMENT_INTERSECTION;
                        }
                        return FIELD_NO_SEGMENT_INTERSECTION;
                    }
                    intersection_x = ((s32)((((second_start_y_general - ((s32)(second_y_delta * second_start_x_value_general) / second_dx)) - first_start_y) +
                                             ((s32)(first_dy * first_start_x) / first_dx)) *
                                            (first_dx * second_dx)) /
                                      (s32)(first_dy_times_second_dx - first_dx_times_second_dy));
                }

                intersection_y = ((s32)((intersection_x - first_start_x) * first_dy) / first_dx) + first_start_y;
            }
        }
    }

    /* Each weighted quotient must reproduce its candidate coordinate. */
    first_start_x_bound = first_start->x;
    first_end_x_bound = first_end->x;
    work_value = intersection_x - first_start_x_bound;
    second_dx = abs(work_value);
    work_value = intersection_x - first_end_x_bound;
    work_value = abs(work_value);
    packed_x_or_distance_sum = second_dx + work_value;
    if (packed_x_or_distance_sum != 0)
    {
        work_value *= first_start_x_bound;
        second_dx *= first_end_x_bound;
        work_value += second_dx;
        if ((u32)work_value / packed_x_or_distance_sum != intersection_x)
        {
            return FIELD_NO_SEGMENT_INTERSECTION;
        }
    }
    first_start_y_bound = first_start->y;
    first_end_y_bound = first_end->y;
    work_value = intersection_y - first_start_y_bound;
    second_dx = abs(work_value);
    work_value = intersection_y - first_end_y_bound;
    work_value = abs(work_value);
    packed_x_or_distance_sum = second_dx + work_value;
    if (packed_x_or_distance_sum != 0)
    {
        work_value *= first_start_y_bound;
        second_dx *= first_end_y_bound;
        work_value += second_dx;
        if ((u32)work_value / packed_x_or_distance_sum != intersection_y)
        {
            return FIELD_NO_SEGMENT_INTERSECTION;
        }
    }
    second_start_x_bound = second_start->x;
    second_end_x_bound = second_end->x;
    work_value = intersection_x - second_start_x_bound;
    second_dx = abs(work_value);
    work_value = intersection_x - second_end_x_bound;
    work_value = abs(work_value);
    packed_x_or_distance_sum = second_dx + work_value;
    if (packed_x_or_distance_sum != 0)
    {
        work_value *= second_start_x_bound;
        second_dx *= second_end_x_bound;
        work_value += second_dx;
        if ((u32)work_value / packed_x_or_distance_sum != intersection_x)
        {
            return FIELD_NO_SEGMENT_INTERSECTION;
        }
    }
    second_start_y_bound = second_start->y;
    second_end_y_bound = second_end->y;
    work_value = intersection_y - second_start_y_bound;
    second_dx = abs(work_value);
    work_value = intersection_y - second_end_y_bound;
    work_value = abs(work_value);
    packed_x_or_distance_sum = second_dx + work_value;
    if (packed_x_or_distance_sum != 0)
    {
        work_value *= second_start_y_bound;
        second_dx *= second_end_y_bound;
        work_value += second_dx;
        if ((u32)work_value / packed_x_or_distance_sum != intersection_y)
        {
            return FIELD_NO_SEGMENT_INTERSECTION;
        }
    }
    packed_x_or_distance_sum = intersection_x & 0xFFFF;
    work_value = intersection_y << 16;
    return packed_x_or_distance_sum | work_value;
}

/**
 * @brief Resolve a proposed actor move against map and actor collisions.
 * @param actor Actor whose position and persistent collision state are updated.
 * @param position Input movement vector, overwritten with the resolved position.
 * @param mode Collision response mode forwarded to the actor collision helper.
 * @return One when the proposed or resolved position is accepted, zero otherwise.
 * @note Uses movement and collision work areas at scratchpad addresses
 *       0x1F800010 and 0x1F800080.
 */
s32 field_resolve_actor_movement(FieldMotionRecord* actor, s32* position, s32 mode)
{
    Vec3i delta;
    FieldObjectRuntime* height_state;
    FieldObjectRuntime* states;
    s32 hit;
    FieldMoveBounds* bounds = (FieldMoveBounds*)FIELD_MAP_BOUNDS_ADDRESS;
    FieldMoveRequest* mover = (FieldMoveRequest*)FIELD_MOVE_REQUEST_ADDRESS;
    FieldMoveQuery* query = (FieldMoveQuery*)FIELD_MOVE_QUERY_ADDRESS;
    s32 actor_z;
    s32 requested_x;
    s32 actor_x;
    s32 requested_z;
    s32 height;
    s32 can_move;
    u16 actor_motion;
    u16 resolved_motion;
    u16 collision_motion;
    FieldObjectRuntime* movement_state;
    FieldObjectRuntime* candidate_movement_state;
    FieldObjectRuntime* collision_state;
    FieldObjectRuntime* blocked_movement_state;
    FieldObjectRuntime* resolved_collision_state;
    FieldObjectRuntime* blocked_collision_state;

    if (((u32)g_field_object_parts[actor->source_object_index].flags >> FIELD_OBJECT_IGNORE_MAP_COLLISION_BIT) & 1)
    {
        actor->x += position[0];
        actor->y = (s32)(actor->y + position[1]);
        position += 2;
        actor->z += position[0];
        return 1;
    }
    if (g_field_active_group != 0)
    {
        actor_motion = actor->motion_parameter;
        if (((u32)(actor_motion - 0xB0) >= 2U) && ((s16)actor_motion != 0xB5) && (func_80092988(actor, (Vec3i*)position) != 0))
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
            if (actor_z < ((s32)(bounds->depth << 0x10) >> 7))
            {
                mover->x = actor_x;
                mover->y = (s32)actor->y;
                mover->z = (s32)actor->z;
                mover->delta_x = position[0];
                requested_x = mover->delta_x;
                mover->delta_y = (s32)position[1];
                mover->delta_z = position[2];
                requested_z = mover->delta_z;
                mover->height_tolerance = 0x10;
                query->extent_y = 0x10;
                if (g_field_object_parts[actor->source_object_index].visual_kind == 0x40)
                {
                    mover->width = 0xC;
                    query->extent_x = 0xC;
                    mover->packed.h.step = 8;
                    query->extent_z = 8;
                }
                else
                {
                    mover->width = 9;
                    query->extent_x = 9;
                    mover->packed.h.step = 6;
                    query->extent_z = 6;
                }
                mover->height_tolerance = 0x10;
                mover->packed.bits.unknown_bit_17 = 0;
                mover->packed.bits.unknown_bit_16 = 0;
                mover->contact = (s32)g_field_object_states[actor->source_object_index].contact_index;
                mover->surface = (s32)g_field_object_states[actor->source_object_index].surface;
                func_8005B6AC(mover);
                g_field_object_states[actor->source_object_index].contact_index = (s32)mover->contact;
                g_field_object_states[actor->source_object_index].surface = (s32)mover->surface;
                resolved_motion = actor->motion_parameter;
                if (((u32)(resolved_motion - 0xB0) < 2U) || ((s16)resolved_motion == 0xB5))
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
                if (((g_field_active_group == 0) || (actor->flags & FIELD_MOTION_RADIUS_MASK) || (func_8005B368(query) == -1)) &&
                    ((collision_motion = actor->motion_parameter, (((u32)(collision_motion - 0xB0) < 2U) != 0)) || ((s16)collision_motion == 0xB5) ||
                     (g_field_active_group == 0) || (func_80092988(actor, &delta) == 0)))
                {
                    position[0] = mover->x;
                    if (((actor->facing_or_reward_kind & FIELD_FACING_INDEX_MASK) == 0x3D) && ((u8)actor->source_object_index < 2U))
                    {
                        position[1] = position[1] + actor->y;
                    }
                    else
                    {
                        position[1] = mover->y;
                    }
                    states = g_field_object_states;
                    position[2] = (s32)mover->z;
                    height_state = &states[actor->source_object_index];
                    height = mover->height_offset;
                    if (height < 0)
                    {
                        height += 0xFF;
                    }
                    height_state->movement.half.height = (s16)(height >> 8);
                }
                else
                {
                    position[0] = actor->x;
                    position[1] = (s32)actor->y;
                    position[2] = (s32)actor->z;
                }
            }
            else
            {
                g_field_object_states[actor->source_object_index].movement.half.height = 0;
                g_field_object_states[actor->source_object_index].contact_index = -1;
                g_field_object_states[actor->source_object_index].surface = 0;
                position[0] = actor->x;
                position[1] = (s32)actor->y;
                position[2] = (s32)actor->z;
            }
        }
        else
        {
            g_field_object_states[actor->source_object_index].movement.half.height = 0;
            g_field_object_states[actor->source_object_index].contact_index = -1;
            g_field_object_states[actor->source_object_index].surface = 0;
            position[0] = actor->x;
            position[1] = (s32)actor->y;
            position[2] = (s32)actor->z;
        }
    }
    else
    {
        g_field_object_states[actor->source_object_index].movement.half.height = 0;
        g_field_object_states[actor->source_object_index].contact_index = -1;
        g_field_object_states[actor->source_object_index].surface = 0;
        position[0] = actor->x;
        position[1] = (s32)actor->y;
        position[2] = (s32)actor->z;
    }
    g_field_last_actor_contact = 0;
    if (((u32)g_field_object_states[actor->source_object_index].movement.word >> FIELD_MOVEMENT_THRESHOLD_BLOCKED_BIT) & 1)
    {
        if (field_is_outside_group_bounds(actor, &actor->x) == 0)
        {
            movement_state = &g_field_object_states[actor->source_object_index];
            movement_state->movement.word = (s32)(movement_state->movement.word & ~FIELD_MOVEMENT_FLAG_THRESHOLD_BLOCKED);
        }
        can_move = 1;
    }
    else if (field_is_outside_group_bounds(actor, position) == 0)
    {
        can_move = 1;
        candidate_movement_state = &g_field_object_states[actor->source_object_index];
        candidate_movement_state->movement.word = (s32)(candidate_movement_state->movement.word & ~FIELD_MOVEMENT_FLAG_THRESHOLD_BLOCKED);
    }
    else
    {
        hit = field_is_outside_group_bounds(actor, &actor->x);
        can_move = 0;
        if (hit != 0)
        {
            blocked_movement_state = &g_field_object_states[actor->source_object_index];
            blocked_movement_state->movement.word = (s32)(blocked_movement_state->movement.word | FIELD_MOVEMENT_FLAG_THRESHOLD_BLOCKED);
        }
    }
    if (can_move == 0)
    {
        return 0;
    }
    if (((u32)g_field_object_states[actor->source_object_index].movement.word >> FIELD_MOVEMENT_ACTOR_BLOCKED_BIT) & 1)
    {
        actor->x = position[0];
        actor->y = (s32)position[1];
        actor->z = (s32)position[2];
        if (field_find_actor_overlap(actor, (s32*)actor, mode) == 0)
        {
            collision_state = &g_field_object_states[actor->source_object_index];
            collision_state->movement.word = (s32)(collision_state->movement.word & ~FIELD_MOVEMENT_FLAG_ACTOR_BLOCKED);
        }
        return 1;
    }
    if (field_find_actor_overlap(actor, position, mode) == 0)
    {
        actor->x = position[0];
        actor->y = (s32)position[1];
        actor->z = (s32)position[2];
        resolved_collision_state = &g_field_object_states[actor->source_object_index];
        resolved_collision_state->movement.word = (s32)(resolved_collision_state->movement.word & ~FIELD_MOVEMENT_FLAG_ACTOR_BLOCKED);
        return 1;
    }
    if (field_find_actor_overlap(actor, (s32*)actor, mode) != 0)
    {
        blocked_collision_state = &g_field_object_states[actor->source_object_index];
        blocked_collision_state->movement.word = (s32)(blocked_collision_state->movement.word | FIELD_MOVEMENT_FLAG_ACTOR_BLOCKED);
    }
    return 0;
}

/**
 * @brief Test whether an actor X position lies outside the active group bounds.
 *
 * Returns 0 when the actor slot has no active group bits. Otherwise reads the
 * threshold entry for g_field_active_group - 1 and compares the whole-unit X
 * coordinate against its inclusive endpoints.
 *
 * @param actor Actor whose slot flags gate the test.
 * @param position Fixed-point position whose X coordinate is tested.
 * @return 0 inside the band or when the slot is inactive; 1 when outside it.
 *
 */
s32 field_is_outside_group_bounds(FieldMotionRecord* actor, s32* position)
{
    FieldThreshold* threshold;
    FieldThreshold* thresholds;
    s32 band_index;
    u16 minimum;
    s32 minimum_value;
    s32 value;

    if ((g_field_object_states[actor->source_object_index].group_flags & FIELD_OBJECT_GROUP_MASK) == 0)
    {
        return 0;
    }
    thresholds = g_field_group_bounds;
    band_index = g_field_active_group - 1;
    threshold = &thresholds[band_index];
    minimum = threshold->min;
    value = position[0] >> 8;
    if (value < (minimum_value = (s32)minimum))
    {
        return 1;
    }
    return (s32)(minimum + threshold->span) < value;
}

/**
 * @brief Find an overlapping actor and optionally start its contact reaction.
 * @param record Actor whose collision dimensions and resource flags are tested.
 * @param position Position to test against the selected actor group.
 * @param filter_group Nonzero to select the opposing group when filtering is enabled.
 * @return Candidate index plus 0x8000, or zero for no candidate or a handled reaction.
 */
s32 field_find_actor_overlap(FieldMotionRecord* record, s32* position, s32 filter_group)
{
    FieldContactScanWorkspace scratch;
    s32 binding_base_or_owner_index;
    u8* bindings;
    s32 record_center_offset;
    FieldObjectRuntime* scan_state;
    FieldMotionRecord* scan_record;
    s32 test_y;
    s32 animation_slot;
    s32 candidate_contact_flags;
    s32 actor_index;
    s32 candidate_index;
    s32 candidate_end;
    s32 binding_offset;
    s32 active_binding_offset;
    FieldSequenceBinding* binding;
    s8 record_vertical_offset;
    s8 candidate_vertical_offset;
    FieldObjectRuntime* record_state;

    if (filter_group != 0 && D_8010D020 == 0)
    {
        if ((u8)record->source_object_index < FIELD_PARTY_ACTOR_COUNT)
        {
            scan_record = g_field_scene_actors;
            scan_state = g_field_scene_object_states;
            actor_index = 3;
            candidate_end = FIELD_RUNTIME_ACTOR_COUNT;
        }
        else
        {
            scan_record = g_field_actors;
            scan_state = g_field_object_states;
            actor_index = 0;
            candidate_end = FIELD_PARTY_ACTOR_COUNT;
        }
    }
    else
    {
        scan_record = g_field_actors;
        scan_state = g_field_object_states;
        actor_index = 0;
        candidate_end = FIELD_RUNTIME_ACTOR_COUNT;
    }
    if ((s8)record->vertical_offset >= 9)
    {
    return_zero:
        return 0;
    }
    candidate_index = actor_index;
    record_state = &g_field_object_states[record->source_object_index];
    record_center_offset = record_state->collision.half.center_offset << 8;
    for (; candidate_index < candidate_end; candidate_index++, scan_record++, scan_state++)
    {
        if ((scan_record->state == FIELD_ACTOR_STATE_UNUSED) ||
            (scan_state->object_flags & (FIELD_OBJECT_FLAG_CONTACT_FILTER_0004 | FIELD_OBJECT_FLAG_CONTACT_FILTER_0020 | FIELD_OBJECT_FLAG_CONTACT_FILTER_0040 |
                                         FIELD_OBJECT_FLAG_CONTACT_FILTER_0080 | FIELD_OBJECT_FLAG_CONTACT_FILTER_0100 | FIELD_OBJECT_FLAG_CONTACT_FILTER_0200 |
                                         FIELD_OBJECT_FLAG_CONTACT_FILTER_2000)) ||
            (scan_record == record) ||
            (candidate_contact_flags = scan_state->contact.flags, ((candidate_contact_flags & FIELD_CONTACT_FLAG_NO_HIT_TEST) != 0)) ||
            (candidate_contact_flags & 1) || (scan_state->collision.word == 0))
        {
            continue;
        }
        candidate_vertical_offset = (s8)scan_record->vertical_offset;
        record_vertical_offset = (s8)record->vertical_offset;
        test_y = position[1];
        if (((scan_record->y + ((scan_state->bounds.half.top + candidate_vertical_offset) << 8)) >
             (test_y + ((record_state->bounds.half.bottom + record_vertical_offset) << 8))) ||
            ((scan_record->y + ((scan_state->bounds.half.bottom + candidate_vertical_offset) << 8)) <
             (test_y + ((record_state->bounds.half.top + record_vertical_offset) << 8))))
        {
            continue;
        }
        scratch.delta.vx = (scan_record->z - position[2]) >> 8;
        scratch.delta.vy = ((scan_record->x + (scan_state->collision.half.center_offset << 8)) - (position[0] + record_center_offset)) >> 8;
        scratch.delta.vz = 0;
        gte_ldlvl(&scratch.delta);
        gte_sqr0();
        gte_stlvnl(&scratch.squared);
        if (SquareRoot0(scratch.squared.vx + scratch.squared.vy) <
            (((s32)(record_state->collision.half.diameter << 16) >> 17) + ((s32)(scan_state->collision.half.diameter << 16) >> 17)))
        {
            break;
        }
    }
    if (candidate_index == candidate_end)
    {
        g_field_last_actor_contact = 0;
        goto return_zero;
    }
    if (g_field_resource_entries[record->resource_index].state != 0)
    {
        if (g_field_resource_entries[scan_record->resource_index].flags & FIELD_RESOURCE_CONTACT_REACTION)
        {
            if (scan_state->current_hp != 0)
            {
                if (!(scan_state->object_flags & (FIELD_OBJECT_FLAG_CONTACT_FILTER_0080 | FIELD_OBJECT_FLAG_CONTACT_FILTER_0200)))
                {
                    if (!(scan_state->contact.flags & FIELD_CONTACT_FLAG_REQUIRE_CONTROLLER))
                    {
                        actor_index = scan_record->source_object_index;
                        if (!(((u32)g_field_object_states[actor_index].contact.flags >> 6) & 1))
                        {
                            binding_base_or_owner_index = (s32)g_field_actor_bindings;
                            if (actor_index < 2U)
                            {
                                binding_offset = actor_index * sizeof(FieldSequenceBinding);
                            }
                            else
                            {
                                binding_offset = 2 * sizeof(FieldSequenceBinding);
                            }
                            binding = (FieldSequenceBinding*)(binding_base_or_owner_index + binding_offset);
                            binding_base_or_owner_index = scan_record->source_object_index;
                            actor_index = binding->owner_object_index;
                            if (actor_index == binding_base_or_owner_index)
                            {
                                bindings = (u8*)g_field_actor_bindings;
                                if ((u32)(actor_index & FIELD_ACTOR_INDEX_MASK) < 2U)
                                {
                                    active_binding_offset = actor_index * sizeof(FieldSequenceBinding);
                                }
                                else
                                {
                                    active_binding_offset = 2 * sizeof(FieldSequenceBinding);
                                }

                                if (((FieldSequenceBinding*)(bindings + active_binding_offset))->state != 0)
                                {
                                    goto return_zero;
                                }
                            }
                        }

                        if (scan_record->motion_parameter == 0)
                        {
                            animation_slot = func_800839F8(scan_record->source_object_index, 0);
                            if (animation_slot != -1)
                            {
                                scratch.targets.words[0] = (s32)record->source_object_index;

                                if (func_80083EEC(scan_record->source_object_index, animation_slot, 0x2A) != 0)
                                {
                                    if ((u8)scan_record->source_object_index < 2U)
                                    {
                                        func_800A2DD8(scan_record->source_object_index);
                                    }
                                    field_start_actor_animation(animation_slot, 1, scratch.targets.actor_indices);
                                }
                            }
                        }
                    }
                }
            }
            goto return_zero;
        }
    }
    g_field_last_actor_contact = candidate_index + FIELD_CONTACT_RESULT_PRESENT;
    return g_field_last_actor_contact;
}

/**
 * @brief Start the primary actor's contact interaction with an enabled object.
 * @param record Actor motion record to update.
 * @param actor_index Slot index used to select the associated field tables.
 */
void field_start_actor_contact_interaction(FieldMotionRecord* record, s32 actor_index)
{
    u8* resource_data;
    s32 actor_index_x8;
    FieldObjectRuntime* slot_base;
    FieldObjectRuntime* slot;

    if (record->source_object_index != 0)
    {
        return;
    }
    actor_index_x8 = actor_index << 3;
    if (record->motion_parameter != 0)
    {
        return;
    }
    slot_base = g_field_object_states;
    slot = (FieldObjectRuntime*)((u8*)slot_base + ((((actor_index_x8 + actor_index) << 4) - actor_index) << 2));
    if ((slot->interaction_flags & FIELD_INTERACTION_FLAG_ENABLED) == 0)
    {
        return;
    }

    field_dispatch_object_state_entry(&g_field_actors[actor_index], 0);
    record->motion_parameter = 0x95;
    record->position_data.path_time = 0xA;

    if (g_field_resource_entries[record->resource_index].flags & FIELD_RESOURCE_CONTACT_REACTION)
    {
        record->facing_or_reward_kind &= 0x80;
    }
    else
    {
        u8 facing = record->facing_or_reward_kind;
        s32 facing_index = facing & FIELD_FACING_INDEX_MASK;
        record->facing_or_reward_kind = (facing_index % 5) | (facing & FIELD_FACING_HIGH_BIT);
    }

    record->saved_state = 0;
    record->animation_active = 1;
    resource_data = g_field_resource_entries[record->resource_index].start;
    field_restart_actor_animation(record, resource_data);
}

/**
 * @brief Probe ahead of an idle actor and begin its available interaction.
 * @param entry Actor whose position and facing select the interaction target.
 */
void field_probe_actor_interaction(FieldMotionRecord* entry)
{
    s32 position[3];
    s32 actor_index;
    s32 contact_result_or_address;
    u8 direction;
    s32 masked;
    FieldObjectRuntime* actor;
    FieldObjectRuntime* actor_base;

    if (entry->motion_parameter != 0)
    {
        return;
    }
    position[0] = entry->x;
    position[1] = entry->y;
    position[2] = entry->z;
    position[0] += rcos(entry->color_position.fields.position_source * 0x10);
    position[2] -= rsin(entry->color_position.fields.position_source * 0x10);
    contact_result_or_address = field_find_actor_overlap(entry, position, 1);
    actor_index = contact_result_or_address & FIELD_CONTACT_RESULT_ACTOR_MASK;
    if (contact_result_or_address != 0)
    {
        actor_base = g_field_object_states;
        contact_result_or_address = (s32)&actor_base[actor_index];
        actor = (FieldObjectRuntime*)contact_result_or_address;
        if (actor->interaction_kind != 0)
        {
            func_800A3938(0x7D, 0x80);
            func_800AF824(actor_index);
            return;
        }
        if (actor->interaction_flags & FIELD_INTERACTION_FLAG_TRIGGERED)
        {
            field_dispatch_object_state_entry(&g_field_actors[actor_index], 0);
            entry->motion_parameter = 0x81;
            entry->motion_scale = 1;
            if (g_field_resource_entries[entry->resource_index].flags & FIELD_RESOURCE_CONTACT_REACTION)
            {
                entry->facing_or_reward_kind = (u8)(entry->facing_or_reward_kind & FIELD_FACING_HIGH_BIT);
            }
            else
            {
                direction = entry->facing_or_reward_kind;
                masked = direction & FIELD_FACING_INDEX_MASK;
                entry->facing_or_reward_kind = (masked % 5) | (direction & FIELD_FACING_HIGH_BIT);
            }
            entry->saved_state = 0;
            entry->animation_active = 1;
            field_restart_actor_animation(entry, g_field_resource_entries[entry->resource_index].start);
        }
    }
}

/**
 * @brief Forwards an object-state value and selected halfword to the field
 *        state handler.
 *
 * @param object Field object containing the state-array index.
 * @param entry_index Index of the halfword entry to forward.
 */
void field_dispatch_object_state_entry(FieldMotionRecord* object, s32 entry_index)
{
    FieldObjectRuntime* state;

    state = &g_field_object_states[object->source_object_index];
    func_800B22F0(state->record_id, state->state_entries[entry_index], g_field_object_states);
}

/**
 * @brief Collect new effect-centered hit contacts and dispatch their reactions.
 * @param effect Motion record supplying the hit origin and source object index.
 * @param radius Expansion of candidate projected X/Y bounds and the depth gate.
 * @param actor Owner whose target tracks, contact offsets, and reaction selector are updated.
 * @note New targets append to track_object_indices, set active_track_mask, and
 * increase track_count. Eligible records already in that list are skipped.
 * @note This function records contacts and dispatches reactions; it does not directly subtract HP.
 */
void field_collect_effect_hits(FieldMotionRecord* effect, s32 radius, FieldActorState* actor)
{
    FieldMotionRecord* motion_records;
    FieldObjectRuntime* object_states;
    FieldSequenceBinding* controller_slots;
    FieldSequenceBinding* controller;
    s32 bound_x_a;
    s32 bound_y_a;
    s16 candidate_kind;
    s32 bound_x_b;
    s32 bound_y_b;
    s32 min_y;
    s32 max_y;
    s32 min_x;
    s32 max_x;
    FieldMotionRecord* candidate_record;
    FieldMotionRecord* group_records;
    s32 group_index;
    s32 candidate_flags;
    s32 delta_z;
    s32 origin_z;
    s32 depth_projection;
    s32 candidate_z_value;
    s32 candidate_x;
    s32 eligible_for_contact;
    s32 candidate_end;
    s32 binding_offset;
    s32 active_binding_offset;
    s32 rounded_delta_z;
    s32 prior_target_index;
    s32 existing_track_index;
    s32 depth_distance;
    u16 depth_radius;
    s32 origin_x;
    s32 candidate_index;
    u8 source_object_index;
    u8 prior_target_count;
    u8 owner_index;
    u8 track_count;
    u8 previous_state;
    FieldObjectRuntime* source_state;
    FieldObjectRuntime* owner_state_for_append;
    FieldObjectRuntime* owner_state_for_count;
    FieldObjectRuntime* candidate_state;
    FieldObjectRuntime* prior_target_base;

    if (D_8010D020 != 0)
    {
        candidate_index = 0;
        candidate_end = FIELD_RUNTIME_ACTOR_COUNT;
    }
    else if (actor->animation->sync_flags & FIELD_ANIMATION_SAME_GROUP)
    {
        if (actor->owner_object_index < FIELD_PARTY_ACTOR_COUNT)
        {
            candidate_index = 0;
            candidate_end = FIELD_PARTY_ACTOR_COUNT;
        }
        else
        {
            candidate_index = FIELD_PARTY_ACTOR_COUNT;
            candidate_end = FIELD_RUNTIME_ACTOR_COUNT;
        }
    }
    else if (actor->owner_object_index < FIELD_PARTY_ACTOR_COUNT)
    {
        candidate_index = FIELD_PARTY_ACTOR_COUNT;
        candidate_end = FIELD_RUNTIME_ACTOR_COUNT;
    }
    else
    {
        candidate_index = 0;
        candidate_end = FIELD_PARTY_ACTOR_COUNT;
    }
    candidate_record = &g_field_actors[candidate_index];
    candidate_state = &g_field_object_states[candidate_index];
    if (candidate_index < candidate_end)
    {
        group_records = candidate_record;
        group_index = 0;
        motion_records = g_field_actors;
        object_states = g_field_object_states;
        controller_slots = g_field_actor_bindings;
        for (; candidate_index < candidate_end; group_index++, candidate_index++, candidate_record++, candidate_state++)
        {
            if (candidate_state->contact.flags & FIELD_CONTACT_FLAG_TARGETED)
            {
                source_object_index = effect->source_object_index;
                eligible_for_contact = 0;
                if (motion_records[source_object_index].motion_parameter == 0x91)
                {
                    source_state = (FieldObjectRuntime*)(source_object_index * sizeof(*object_states) + (s32)object_states);
                    prior_target_count = source_state->contact.bytes.target_count;
                    prior_target_index = 0;
                    if (prior_target_count != 0)
                    {
                        prior_target_base = source_state;
                        do
                        {
                            if (prior_target_base->targets[prior_target_index++] == candidate_index)
                            {
                                goto contact_allowed;
                            }
                        } while (prior_target_index < (s32)prior_target_count);
                    }
                }
            }
            else
            {
            contact_allowed:
                eligible_for_contact = 1;
            }
            owner_index = actor->owner_object_index;
            if ((candidate_index != owner_index) && ((candidate_state->effect_vertices.words[0] != 0) || (candidate_state->effect_vertices.words[2] != 0)) &&
                ((candidate_state->bounds.words[0] != 0) || (candidate_state->bounds.words[1] != 0)) && (candidate_state->collision.word != 0) &&
                (candidate_kind = candidate_record->motion_parameter, (candidate_kind != 0x91)) && (candidate_kind != 0xAE) && (candidate_kind != 0x87) &&
                (candidate_record->state != FIELD_ACTOR_STATE_UNUSED) && (owner_index != candidate_index) && (candidate_state->current_hp != 0) &&
                (candidate_flags = candidate_state->contact.flags, ((candidate_flags & FIELD_CONTACT_FLAG_REQUIRE_CONTROLLER) == 0)) &&
                ((candidate_index < FIELD_PARTY_ACTOR_COUNT) || ((candidate_state->group_flags & FIELD_OBJECT_GROUP_MASK) == g_field_active_group)) &&
                ((candidate_flags & FIELD_CONTACT_FLAG_NO_HIT_TEST) == 0) && (eligible_for_contact != 0) &&
                !(candidate_state->movement.word & FIELD_MOVEMENT_FLAG_NO_CONTACT))
            {
                if (!(candidate_flags & FIELD_CONTACT_FLAG_IGNORE_BINDING))
                {
                    s32 binding_owner;
                    s32 controller_index;
                    if (candidate_record->source_object_index < 2U)
                    {
                        binding_offset = candidate_record->source_object_index * sizeof(FieldSequenceBinding);
                    }
                    else
                    {
                        binding_offset = 2 * sizeof(FieldSequenceBinding);
                    }

                    controller = (FieldSequenceBinding*)((u8*)controller_slots + binding_offset);
                    controller_index = candidate_record->source_object_index;
                    binding_owner = controller->owner_object_index;
                    if (binding_owner == controller_index)
                    {
                        if ((u32)(binding_owner & FIELD_ACTOR_INDEX_MASK) < 2U)
                        {
                            active_binding_offset = binding_owner * sizeof(FieldSequenceBinding);
                        }
                        else
                        {
                            active_binding_offset = 2 * sizeof(FieldSequenceBinding);
                        }
                        if (((FieldSequenceBinding*)((u8*)controller_slots + active_binding_offset))->state != 0)
                        {
                            continue;
                        }
                    }
                }
                if (!(candidate_state->object_flags &
                      (FIELD_OBJECT_FLAG_CONTACT_FILTER_0080 | FIELD_OBJECT_FLAG_CONTACT_FILTER_0200 | FIELD_OBJECT_FLAG_CONTACT_FILTER_2000)))
                {
                    track_count = actor->track_count;
                    existing_track_index = 0;
                    while (existing_track_index < (s32)track_count && candidate_index != actor->track_object_indices[existing_track_index])
                    {
                        existing_track_index += 1;
                    }
                    if (existing_track_index == actor->track_count)
                    {
                        /* Depth gating precedes expanded projected X/Y bounds. */
                        candidate_z_value = candidate_record->z;
                        origin_z = effect->z;
                        delta_z = candidate_z_value - origin_z;
                        depth_distance = (candidate_z_value - origin_z) / FIELD_PROJECTED_DEPTH_SCALE;
                        depth_radius = candidate_state->collision.half.diameter;
                        if (depth_distance < 0)
                        {
                            depth_distance = -depth_distance;
                        }
                        depth_distance = depth_distance < (radius + ((s32)(depth_radius << 0x10) >> 0x11));
                        if (depth_distance)
                        {
                            bound_x_a = candidate_state->bounds.half.left;
                            bound_x_b = candidate_state->bounds.half.right;
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
                            bound_y_a = candidate_state->bounds.half.top;
                            bound_y_b = candidate_state->bounds.half.bottom;
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
                            candidate_x = candidate_record->x;
                            origin_x = effect->x;
                            if (((candidate_x + (min_x << 8)) < origin_x) && (origin_x < (candidate_x + (max_x << 8))) &&
                                (candidate_x = candidate_record->y, origin_x = effect->y, (((candidate_x + (min_y << 8)) < origin_x) != 0)) &&
                                (origin_x < (candidate_x + (max_y << 8))) &&
                                (object_states[actor->owner_object_index].contact.bytes.target_count < FIELD_MAX_CONTACT_TARGETS))
                            {
                                candidate_state->contact.flags = (s32)(candidate_state->contact.flags | FIELD_CONTACT_FLAG_TARGETED);
                                candidate_state->object_flags = (s32)(candidate_state->object_flags & ~FIELD_OBJECT_FLAG_CLEAR_ON_HIT);
                                owner_state_for_append = (FieldObjectRuntime*)(actor->owner_object_index * sizeof(*object_states) + (s32)object_states);
                                owner_state_for_append->targets[owner_state_for_append->contact.bytes.target_count] = candidate_index;
                                owner_state_for_count = (FieldObjectRuntime*)(actor->owner_object_index * sizeof(*object_states) + (s32)object_states);
                                owner_state_for_count->contact.bytes.target_count = (u8)(owner_state_for_count->contact.bytes.target_count + 1);
                                /* Each new contact becomes an active target track. */
                                actor->active_track_mask = (u8)(actor->active_track_mask | (1 << actor->track_count));
                                actor->track_object_indices[actor->track_count] = candidate_index;
                                if (candidate_record->facing_or_reward_kind & FIELD_FACING_HIGH_BIT)
                                {
                                    actor->track_offsets[actor->track_count].x = (candidate_record->x - effect->x) >> 8;
                                }
                                else
                                {
                                    actor->track_offsets[actor->track_count].x = (effect->x - candidate_record->x) >> 8;
                                }

                                actor->track_offsets[actor->track_count].y =
                                    ((effect->y - candidate_record->y) >> 8) - ((effect->z - candidate_record->z) >> 9);

                                /* Preserve the prior effect state when retiring on contact. */
                                if (effect->flags & FIELD_EFFECT_RETIRE_ON_HIT)
                                {
                                    previous_state = effect->state;
                                    effect->state = FIELD_EFFECT_RETIRED;
                                    effect->height_or_retired_state = previous_state;
                                }
                                actor->track_count = (u8)(actor->track_count + 1);
                                func_8008BC5C(&group_records[group_index]);
                                if ((candidate_index < 2) && !(*(u16*)&group_records[group_index].flags & FIELD_MOTION_RADIUS_MASK))
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
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief Find the first eligible actor within the adjusted GTE distance threshold.
 * @param reference_position Reference position in fixed-point coordinates.
 * @param distance_limit Distance threshold before adding half the candidate radius.
 * @param source_actor Actor selecting the search group and optional self exclusion.
 * @param opposing_group Nonzero to select the opposing group and exclude the source actor.
 * @return Matching actor index, or -1 when no eligible actor is close enough.
 */
s32 field_find_actor_in_range(s32* reference_position, s32 distance_limit, FieldActorState* source_actor, s32 opposing_group)
{
    VECTOR* delta = (VECTOR*)FIELD_GTE_DELTA_ADDRESS;
    VECTOR* squared = (VECTOR*)FIELD_GTE_SQUARE_ADDRESS;
    FieldMotionRecord* candidate_record;
    s32 candidate_index;
    s32 candidate_end;
    u8 candidate_state;
    u8* candidate_z_cursor;
    u8* candidate_object_cursor;
    FieldObjectRuntime* candidate_state_base;
    if (opposing_group != 0)
    {
        if (source_actor->animation->sync_flags & FIELD_ANIMATION_SAME_GROUP)
        {
            candidate_index = 0;
            if (source_actor->owner_object_index >= FIELD_PARTY_ACTOR_COUNT)
            {
                candidate_index = FIELD_PARTY_ACTOR_COUNT;
                goto search_extended_group;
            }
            goto finish_player_group;
        }
        candidate_index = FIELD_PARTY_ACTOR_COUNT;
        if (source_actor->owner_object_index < FIELD_PARTY_ACTOR_COUNT)
        {
        search_extended_group:
            candidate_end = FIELD_RUNTIME_ACTOR_COUNT;
        }
        else
        {
            goto search_player_group;
        }
    }
    else
    {
    search_player_group:
        candidate_index = 0;
    finish_player_group:
        candidate_end = FIELD_PARTY_ACTOR_COUNT;
    }
    candidate_record = &g_field_actors[candidate_index];
    candidate_state_base = &g_field_object_states[candidate_index];
    if (candidate_index < candidate_end)
    {
        candidate_object_cursor = (u8*)&candidate_state_base->collision.half.diameter;
        candidate_z_cursor = (u8*)&((FieldMotionRecord*)candidate_record)->z;
    scan_candidates:
        candidate_state = FIELD_CONTAINER(candidate_z_cursor, FieldMotionRecord, z)->state;
        if (candidate_state == FIELD_ACTOR_STATE_UNUSED ||
            FIELD_CONTAINER(candidate_object_cursor, FieldObjectRuntime, collision.half.diameter)->current_hp == 0 ||
            candidate_state == FIELD_ACTOR_STATE_INACTIVE || (opposing_group != 0 && source_actor->owner_object_index == candidate_index))
        {
            goto next_candidate;
        }
        {
            s32 actor_x = candidate_record->x;
            s32 reference_x;
            do
            {
                reference_x = reference_position[0];
            } while (0);
            delta->vx = (actor_x - reference_x) >> 8;
        }
        delta->vy = (FIELD_CONTAINER(candidate_z_cursor, FieldMotionRecord, z)->y - reference_position[1]) >> 8;
        delta->vz = (FIELD_CONTAINER(candidate_z_cursor, FieldMotionRecord, z)->z - reference_position[2]) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(squared);
        if (SquareRoot0(squared->vx + squared->vy + squared->vz) <
            distance_limit +
                ((s32)(FIELD_CONTAINER(candidate_object_cursor, FieldObjectRuntime, collision.half.diameter)->collision.half.diameter << 16) >> 17))
        {
            return candidate_index;
        }
    next_candidate:
        candidate_index++;
        candidate_z_cursor += sizeof(FieldMotionRecord);
        candidate_record++;
        candidate_object_cursor += sizeof(FieldObjectRuntime);
        if (candidate_index < candidate_end)
        {
            goto scan_candidates;
        }
    }
    return -1;
}

/**
 * @brief Find eligible actors intersecting an attack's collision spheres and apply reactions.
 * @param actor Attacking actor state, including its target list and attack mode.
 * @param part Actor part used to obtain the attack radius and sphere centers.
 * @note Distances use the GTE square operation followed by SquareRoot0.
 */
void field_collect_attack_sphere_hits(FieldActorState* actor, FieldActorPartDef* part)
{
    u8* bindings;
    FieldMotionRecord* motion_records;
    FieldObjectRuntime* object_states;
    VECTOR* delta = (VECTOR*)FIELD_GTE_DELTA_ADDRESS;
    VECTOR* squares = (VECTOR*)FIELD_GTE_SQUARE_ADDRESS;
    s32 target_end;
    s32 sphere_count;
    s32 attack_radius;
    VECTOR* spheres;
    s32 checked_record_offset;
    FieldMotionRecord* checked_record;
    FieldMotionRecord* target_record;
    s32 target_record_offset;
    s16 target_state;
    FieldMotionRecord* target_position;
    s32 target_flags;
    s32 controlled_actor;
    s32 controller_index;
    s32 initial_target_offset;
    s32 eligible_for_hit;
    VECTOR* sphere;
    s32 sphere_index;
    s32 binding_offset;
    s32 active_binding_offset;
    s32 prior_target_index;
    s32 existing_track_index;
    s32 target_index;
    u8 source_index;
    u8 prior_target_count;
    u8 track_count;
    FieldObjectRuntime* source_state;
    FieldObjectRuntime* prior_target_base;
    FieldObjectRuntime* owner_state_for_append;
    FieldObjectRuntime* owner_state_for_count;
    FieldObjectRuntime* target_state_base;
    void* target_z_cursor;

    spheres = (VECTOR*)FIELD_ATTACK_SPHERE_0_ADDRESS;
    attack_radius = field_resolve_effect_extent(actor, part);
    field_resolve_actor_part_anchor(actor, part, (Vec3i*)FIELD_ATTACK_SPHERE_0_ADDRESS, 0);
    if ((actor->action_flags & FIELD_ACTOR_ACTION_KIND_MASK) == FIELD_ACTOR_ACTION_KIND_ATTACK)
    {
        field_resolve_actor_part_anchor(actor, part, (Vec3i*)FIELD_ATTACK_SPHERE_1_ADDRESS, 1);
        field_resolve_actor_part_anchor(actor, part, (Vec3i*)FIELD_ATTACK_SPHERE_2_ADDRESS, 2);
        sphere_count = 3;
    }
    else
    {
        sphere_count = 1;
    }
    if (D_8010D020 != 0)
    {
        target_index = 0;
        target_end = FIELD_RUNTIME_ACTOR_COUNT;
    }
    else if (actor->animation->sync_flags & FIELD_ANIMATION_SAME_GROUP)
    {
        if (actor->owner_object_index < FIELD_PARTY_ACTOR_COUNT)
        {
            target_index = 0;
            target_end = FIELD_PARTY_ACTOR_COUNT;
        }
        else
        {
            target_index = FIELD_PARTY_ACTOR_COUNT;
            target_end = FIELD_RUNTIME_ACTOR_COUNT;
        }
    }
    else if (actor->owner_object_index < FIELD_PARTY_ACTOR_COUNT)
    {
        target_index = FIELD_PARTY_ACTOR_COUNT;
        target_end = FIELD_RUNTIME_ACTOR_COUNT;
    }
    else
    {
        target_index = 0;
        target_end = FIELD_PARTY_ACTOR_COUNT;
    }
    initial_target_offset = target_index * sizeof(FieldMotionRecord);
    target_position = &g_field_actors[target_index];
    target_state_base = &g_field_object_states[target_index];
    if (target_index < target_end)
    {
        void* target_flags_cursor = &target_state_base->object_flags;
        target_z_cursor = (u8*)&target_position->z;
        target_record = target_position;
        target_record_offset = initial_target_offset;
        bindings = g_field_actor_bindings;
        motion_records = g_field_actors;
        object_states = g_field_object_states;
    next_target:
        if (FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->contact.flags & FIELD_CONTACT_FLAG_TARGETED)
        {
            source_index = actor->owner_object_index;
            eligible_for_hit = 0;
            if (motion_records[source_index].motion_parameter == 0x91)
            {
                source_state = (FieldObjectRuntime*)(source_index * sizeof(*object_states) + (s32)object_states);
                prior_target_count = source_state->contact.bytes.target_count;
                prior_target_index = 0;
                if (prior_target_count != 0)
                {
                    prior_target_base = source_state;
                    do
                    {
                        if (prior_target_base->targets[prior_target_index++] == target_index)
                        {
                            goto mark_eligible;
                        }
                    } while (prior_target_index < (s32)prior_target_count);
                }
            }
        }
        else
        {
        mark_eligible:
            eligible_for_hit = 1;
        }
        if ((target_index != actor->owner_object_index) && (FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->collision.word != 0))
        {
            target_state = FIELD_CONTAINER(target_z_cursor, FieldMotionRecord, z)->motion_parameter;
            if ((target_state != 0x91) && (target_state != 0xAE) && (target_state != 0x87) &&
                ((target_index >= 2) || ((FIELD_CONTAINER(target_z_cursor, FieldMotionRecord, z)->facing_or_reward_kind & FIELD_FACING_INDEX_MASK) != 0x3C)) &&
                (FIELD_CONTAINER(target_z_cursor, FieldMotionRecord, z)->state != FIELD_ACTOR_STATE_UNUSED) && (actor->owner_object_index != target_index) &&
                (FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->current_hp != 0))
            {
                target_flags = FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->contact.flags;
                if (!(target_flags & FIELD_CONTACT_FLAG_REQUIRE_CONTROLLER) &&
                    ((((target_index < FIELD_PARTY_ACTOR_COUNT) != 0)) ||
                     (((FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->group_flags & FIELD_OBJECT_GROUP_MASK) ==
                       g_field_active_group))) &&
                    ((target_flags & FIELD_CONTACT_FLAG_NO_HIT_TEST) == 0) && (eligible_for_hit != 0) &&
                    !(FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->movement.word & FIELD_MOVEMENT_FLAG_NO_CONTACT))
                {
                    if (!(target_flags & FIELD_CONTACT_FLAG_IGNORE_BINDING))
                    {
                        if ((u8)FIELD_CONTAINER(target_z_cursor, FieldMotionRecord, z)->source_object_index < 2U)
                        {
                            binding_offset = FIELD_CONTAINER(target_z_cursor, FieldMotionRecord, z)->source_object_index * sizeof(FieldSequenceBinding);
                        }
                        else
                        {
                            binding_offset = 2 * sizeof(FieldSequenceBinding);
                        }
                        {
                            FieldSequenceBinding* controller = (FieldSequenceBinding*)(bindings + binding_offset);
                            controller_index = FIELD_CONTAINER(target_z_cursor, FieldMotionRecord, z)->source_object_index;
                            controlled_actor = controller->owner_object_index;
                        }
                        if (controlled_actor == controller_index)
                        {
                            if ((u32)(controlled_actor & FIELD_ACTOR_INDEX_MASK) < 2U)
                            {
                                active_binding_offset = controlled_actor * sizeof(FieldSequenceBinding);
                            }
                            else
                            {
                                active_binding_offset = 2 * sizeof(FieldSequenceBinding);
                            }
                            if (((FieldSequenceBinding*)(bindings + active_binding_offset))->state != 0)
                            {
                                goto advance_target;
                            }
                        }
                    }

                    if (!(FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->object_flags &
                          (FIELD_OBJECT_FLAG_CONTACT_FILTER_0080 | FIELD_OBJECT_FLAG_CONTACT_FILTER_0200 | FIELD_OBJECT_FLAG_CONTACT_FILTER_2000)))
                    {
                        track_count = actor->track_count;
                        existing_track_index = 0;
                        while (existing_track_index < (s32)track_count && target_index != actor->track_object_indices[existing_track_index])
                        {
                            existing_track_index += 1;
                        }
                        if (existing_track_index == actor->track_count)
                        {
                            sphere = spheres;
                            sphere_index = 0;
                            if (sphere_count != 0)
                            {
                                checked_record_offset = target_record_offset;
                                checked_record = target_record;
                                do
                                {
                                    delta->vx = (s32)((s32)(target_position->x - sphere->vx) >> 8);
                                    delta->vy = (s32)((s32)(FIELD_CONTAINER(target_z_cursor, FieldMotionRecord, z)->y - sphere->vy) >> 8);
                                    delta->vz = (s32)((s32)(FIELD_CONTAINER(target_z_cursor, FieldMotionRecord, z)->z - sphere->vz) >> 8);
                                    gte_ldlvl(delta);
                                    gte_sqr0();
                                    gte_stlvnl(squares);
                                    if ((SquareRoot0(squares->vx + squares->vy + squares->vz) <
                                         (attack_radius +
                                          ((s16)FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->collision.half.diameter >> 1))) &&
                                        ((u8)g_field_object_states[actor->owner_object_index].contact.bytes.target_count < FIELD_MAX_CONTACT_TARGETS))
                                    {
                                        FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->contact.flags =
                                            (s32)(FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->contact.flags |
                                                  FIELD_CONTACT_FLAG_TARGETED);
                                        FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->object_flags =
                                            (s32)(FIELD_CONTAINER(target_flags_cursor, FieldObjectRuntime, object_flags)->object_flags &
                                                  ~FIELD_OBJECT_FLAG_CLEAR_ON_HIT);
                                        owner_state_for_append = &g_field_object_states[actor->owner_object_index];
                                        owner_state_for_append->targets[owner_state_for_append->contact.bytes.target_count] = target_index;
                                        owner_state_for_count = &g_field_object_states[actor->owner_object_index];
                                        owner_state_for_count->contact.bytes.target_count = (u8)(owner_state_for_count->contact.bytes.target_count + 1);
                                        actor->active_track_mask = (u8)(actor->active_track_mask | (1 << actor->track_count));
                                        actor->track_object_indices[actor->track_count] = target_index;
                                        actor->track_count = (u8)(actor->track_count + 1);
                                        if ((target_index < 2) && !(*(u16*)&checked_record->flags & FIELD_MOTION_RADIUS_MASK))
                                        {
                                            func_800A2DD8(target_index);
                                        }
                                        func_8008BC5C((FieldMotionRecord*)(checked_record_offset + (s32)g_field_actors));
                                        if (((u8)actor->hit_reaction < 0xCU) || (g_field_actors[actor->owner_object_index].motion_parameter == 0xBC))
                                        {
                                            func_8008A9D8(actor->owner_object_index, target_index, actor->hit_reaction);
                                        }
                                        else
                                        {
                                            switch (actor->hit_reaction)
                                            {
                                            case 0x50:
                                                func_8008A9D8(actor->owner_object_index, target_index, 0x12U);
                                                break;
                                            case 0x51:
                                                func_8008A9D8(actor->owner_object_index, target_index, 0x13U);
                                                break;
                                            case 0x4E:
                                                func_8008A9D8(actor->owner_object_index, target_index, 0x14U);
                                                break;
                                            case 0x4F:
                                                func_8008A9D8(actor->owner_object_index, target_index, 0x15U);
                                                break;
                                            case 0x3E:
                                                func_8008A9D8(actor->owner_object_index, target_index, 0x19U);
                                                break;
                                            case 0x45:
                                                func_8008A9D8(actor->owner_object_index, target_index, 0x1AU);
                                                break;
                                            default:
                                                func_8008A840(actor->owner_object_index, target_index);
                                                break;
                                            }
                                        }
                                    }
                                    sphere_index += 1;
                                    sphere++;
                                } while (sphere_index < sphere_count);
                            }
                        }
                    }
                }
            }
        }
    advance_target:
        target_index += 1;
        target_z_cursor += sizeof(FieldMotionRecord);
        target_position++;
        target_flags_cursor += sizeof(FieldObjectRuntime);
        target_record++;
        target_record_offset += sizeof(FieldMotionRecord);
        if (target_index < target_end)
        {
            goto next_target;
        }
    }
}

/**
 * @brief Return the GTE-computed distance between positions @p a and @p b.
 * @param a First position (vx/vy/vz).
 * @param b Second position (vx/vy/vz).
 * @return sqrt(sum of squared per-axis deltas), each delta scaled by >> 8.
 */
s32 field_get_position_distance(VECTOR* a, VECTOR* b)
{
    VECTOR* delta = (VECTOR*)FIELD_GTE_DELTA_ADDRESS;
    VECTOR* squared = (VECTOR*)FIELD_GTE_SQUARE_ADDRESS;

    delta->vx = (a->vx - b->vx) >> 8;
    delta->vy = (a->vy - b->vy) >> 8;
    delta->vz = (a->vz - b->vz) >> 8;
    gte_ldlvl(delta);
    gte_sqr0();
    gte_stlvnl(squared);
    return SquareRoot0(squared->vx + squared->vy + squared->vz);
}
