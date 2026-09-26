/** @file field_contact_geometry.c
 * @brief Resolve field actor contacts, movement, interactions, and attack hits.
 */
#include "common.h"
#include "field_calls.h"
#include "field_actor.h"
#include "field_actor_behavior.h"
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

/** @brief Object state @p index of @p base; the original adds the scaled index before the base. */
#define FIELD_OBJECT_STATE_AT(base, index) ((FieldObjectRuntime*)((index) * sizeof(FieldObjectRuntime) + (s32)(base)))

/** @brief CD resource holding the actor sequence bytecode banks. */
#define FIELD_SEQUENCE_RESOURCE_ID 0x5DD
#define FIELD_SEQUENCE_BANK_COUNT 11
#define FIELD_SEQUENCE_ROW_COUNT 24
#define FIELD_SEQUENCE_ROW_SIZE 32

#define FIELD_QUAD_VERTEX_COUNT 4
/** @brief Most targets one object can collect (FieldObjectRuntime::targets). */
#define FIELD_MAX_CONTACT_TARGETS 9
/** @brief field_intersect_screen_segments result when the segments do not meet. */
#define FIELD_NO_SEGMENT_INTERSECTION 0x80008000

/** @brief field_find_actor_overlap result bit set with the overlapped object index in the low bits. */
#define FIELD_CONTACT_RESULT_PRESENT 0x8000
#define FIELD_CONTACT_RESULT_ACTOR_MASK 0x7FFF

/** @brief FieldMoveObject::flags bit 23: the object moves without map collision. */
#define FIELD_PART_IGNORE_MAP_COLLISION_BIT 23
/** @brief FieldMoveObject::scale_z of a full-size object (the large collision footprint). */
#define FIELD_PART_FULL_SCALE 0x40
/** @brief Collision footprint of a moving actor (map units), normal and large. */
#define FIELD_MOVE_WIDTH 9
#define FIELD_MOVE_STEP 6
#define FIELD_MOVE_LARGE_WIDTH 12
#define FIELD_MOVE_LARGE_STEP 8
#define FIELD_MOVE_HEIGHT_TOLERANCE 16

/** @brief FieldActorAnimationDef::sync_flags bit: the animation hits its own group, not the opposing one. */
#define FIELD_ANIMATION_SAME_GROUP 0x01
/** @brief FieldResourceEntry::flags bit: the resource has an action table and eight-direction animations. */
#define FIELD_RESOURCE_HAS_ACTIONS 0x01

/** @brief FieldActorState::action_flags bits holding the action kind; kind 8 swings three attack spheres. */
#define FIELD_ACTOR_ACTION_KIND_MASK 0x1E
#define FIELD_ACTOR_ACTION_KIND_ATTACK 8

/** @brief Actor animations with a special contact rule (FieldActor::animation without the facing bit). */
#define FIELD_ANIMATION_38 0x38 /**< 0x38 and 0x39 are never touched by effect quads. */
#define FIELD_ANIMATION_39 0x39
#define FIELD_ANIMATION_3C 0x3C /**< A player in this animation is not hit by attack spheres. */
#define FIELD_ANIMATION_3D 0x3D /**< A player in this animation keeps its height while moving. */
#define FIELD_ANIMATION_3F 0x3F /**< A player in this animation answers an effect quad with action 2. */
#define FIELD_ANIMATION_40 0x40 /**< A player in this animation answers an effect quad with action 3. */

/** @brief FieldActorState::hit_reaction values below this are passed on as the hit action; higher ones are mapped. */
#define FIELD_HIT_REACTION_DIRECT_COUNT 12
/** @brief Actors this high (FieldActor::height) or higher overlap nothing. */
#define FIELD_OVERLAP_MAX_HEIGHT 9
/** @brief Bit number of FIELD_CONTACT_IGNORE_BINDING. */
#define FIELD_CONTACT_IGNORE_BINDING_BIT 6
/** @brief Builtin animation a touched object plays towards the toucher. */
#define FIELD_TOUCH_ANIMATION 0x2A
/** @brief Frames an actor waits in FIELD_ACTOR_COMMAND_SEQUENCE_WAIT before its contact interaction runs. */
#define FIELD_CONTACT_INTERACTION_DELAY 10
/** @brief Sound played when an interaction probe finds a talk target. */
#define FIELD_SOUND_INTERACT 0x7D
#define FIELD_SOUND_PAN_CENTER 0x80

/** @brief Object-state contact bits set by the effect-quad answer of FIELD_ANIMATION_3F / _40. */
#define FIELD_CONTACT_QUAD_ANSWER_MASK 0x1C
#define FIELD_CONTACT_QUAD_ANSWER_2 0x08
#define FIELD_CONTACT_QUAD_ANSWER_3 0x10
/** @brief Object-state flag bits of unknown meaning tested by the contact scans. */
#define FIELD_OBJECT_FLAG_0004 0x0004
#define FIELD_OBJECT_FLAG_0020 0x0020
#define FIELD_OBJECT_FLAG_0040 0x0040
#define FIELD_OBJECT_FLAG_0080 0x0080
#define FIELD_OBJECT_FLAG_0100 0x0100
/** @brief Object-state flag bit cleared when an object becomes a target. */
#define FIELD_OBJECT_FLAG_CLEAR_ON_HIT 0x0400
/** @brief Object-state flag bits that keep an object out of overlap tests. */
#define FIELD_OBJECT_NO_OVERLAP_FLAGS \
    (FIELD_OBJECT_FLAG_0004 | FIELD_OBJECT_FLAG_0020 | FIELD_OBJECT_FLAG_0040 | FIELD_OBJECT_FLAG_0100 | FIELD_OBJECT_UNTARGETABLE_FLAGS)

/** @brief Object-state movement bits: the object stands outside its group bounds / overlaps another actor. */
#define FIELD_MOVEMENT_OUTSIDE_BOUNDS_BIT 13
#define FIELD_MOVEMENT_OVERLAPPING_BIT 14
#define FIELD_MOVEMENT_OUTSIDE_BOUNDS 0x2000
#define FIELD_MOVEMENT_OVERLAPPING 0x4000

/** @brief Object-state interaction_flags bits. */
#define FIELD_INTERACTION_FLAG_ENABLED 0x01   /**< Touching the object starts its interaction. */
#define FIELD_INTERACTION_FLAG_TRIGGERED 0x02 /**< The action button starts the object's interaction. */

/** @brief Scale of the vertical distance tested against the collision extents. */
#define FIELD_CONTACT_HEIGHT_SCALE 224
/** @brief Scale of the depth distance tested against an effect hit radius. */
#define FIELD_PROJECTED_DEPTH_SCALE 384

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

/** @brief View of g_field_object_parts (FieldObjectPart in field_actor_tables.h, which this file cannot include). */
typedef struct
{
    u8 pad_0x0[0x2E];
    u8 scale_z;
    u8 pad_0x2f[5];
    u32 flags;
    u8 pad_0x38[0x10];
} FieldMoveObject;

/**
 * @brief Mover handed to field_collision_move_mover (same layout as in field_collision.c).
 * @note height and resolved_height are negated heights; collision_node and flags persist in the object state.
 */
struct FieldCollisionMover
{
    s32 x;
    s32 height;
    s32 z;
    s32 move_x;
    s32 move_height;
    s32 move_z;
    s32 resolved_height;
    s32 collision_node;
    s32 flags;
    s16 footprint_width;
    s16 height_bias;
    union
    {
        s32 mode_flags;
        struct
        {
            s16 footprint_depth;
            u16 flags;
        } h;
        struct
        {
            unsigned footprint_depth : 16;
            unsigned airborne_low : 1;
            unsigned airborne_high : 1;
            unsigned unused : 14;
        } bits;
    } mode;
};

/** @brief Map size words of the field map header (FieldMapBounds in field_actor_tables.h). */
typedef struct
{
    s16 width;
    u16 depth;
} FieldMoveBounds;

/** @brief Horizontal range of an actor group, in whole map units (same layout as in field_actor_camera.c). */
typedef struct
{
    u16 left;
    u16 width;
} FieldGroupBounds;

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

extern u8 g_field_actor_sequence_data[FIELD_SEQUENCE_BANK_COUNT][FIELD_SEQUENCE_ROW_COUNT][FIELD_SEQUENCE_ROW_SIZE];
extern u8* g_field_cd_buffer;
extern FieldMoveObject g_field_object_parts[];
extern s32 g_field_active_group;
extern FieldGroupBounds g_field_group_bounds[];
extern FieldActorState g_field_actor_slots[];
extern FieldResourceEntry g_field_resource_entries[];
extern s32 g_field_duel_mode;
/** @brief Last field_find_actor_overlap result: zero, or the object index plus FIELD_CONTACT_RESULT_PRESENT. */
extern s32 g_field_last_actor_contact;

/* Defined as (s32, s32) in field_interaction_start.c; the original call also loads the state table into $a2. */
void field_start_interaction(s32 value, u16 entry, FieldObjectRuntime* states);
s32 field_resolve_contact_hit(s32 source_index, s32 target_index);
s32 field_resolve_object_hit(s32 source_index, s32 target_index, s32 action);
void field_release_object_link(FieldActor* actor);
/* Defined as (FieldActor*) in field_actor_runtime.c; the calls here also pass the resource start in $a1. */
void field_restart_actor_animation();

static s32 field_intersect_screen_segments(Vec2s* first_start, Vec2s* first_end, Vec2s* second_start, Vec2s* second_end);
static s32 field_is_outside_group_bounds(FieldActor* actor, s32* position);
static void field_dispatch_object_state_entry(FieldActor* actor, s32 entry_index);

/**
 * @brief Load the actor sequence bytecode banks from their CD resource.
 * @note The resource data starts one byte into the CD buffer.
 */
void field_load_actor_sequence_data(void)
{
    s32 bank_index;
    s32 row_index;
    s32 byte_index;
    u8* source;
    u8* row;

    cdrom_queue_read(FIELD_SEQUENCE_RESOURCE_ID, g_field_cd_buffer);
    cdrom_wait_queue_empty();
    source = g_field_cd_buffer + 1;
    for (bank_index = 0; bank_index < FIELD_SEQUENCE_BANK_COUNT; bank_index++)
    {
        for (row_index = 0; row_index < FIELD_SEQUENCE_ROW_COUNT; row_index++)
        {
            byte_index = 0;
            /* Offset summed before the base: &g_field_actor_sequence_data[bank][row] adds the other way round. */
            row = (u8*)g_field_actor_sequence_data +
                  (bank_index * sizeof(g_field_actor_sequence_data[0]) + row_index * sizeof(g_field_actor_sequence_data[0][0]));
            do
            {
                row[byte_index] = *source;
                byte_index++;
                source++;
            } while (byte_index < FIELD_SEQUENCE_ROW_SIZE);
        }
    }
}
/**
 * @brief Test an effect quad against the field objects and handle the players' quad answers.
 * @param quad Four packed screen-space vertices of the effect quad.
 * @param effect Effect record; its owner object is excluded and supplies the height extent.
 * @param contact Receives the touched object index and the contact point.
 * @return 0 for no contact, 1 or 2 for the touched collision layer (lower, upper), 3 when a player answered.
 * @note A failed edge test also stores FIELD_NO_SEGMENT_INTERSECTION in contact->point.
 */
s32 field_test_quad_actor_contacts(FieldContactPoint* quad, FieldMotionRecord* effect, FieldContactResult* contact)
{
    FieldContactPoint* input_quad;
    FieldActor* target;
    FieldActor* owner;
    FieldObjectRuntime* target_state;
    FieldObjectRuntime* owner_state;
    union
    {
        FieldContactPoint center;
        u8 storage[0x28];
    } scratch;
    s16 command;
    s32 target_extent;
    s32 flags_or_extent;
    s32 intersection;
    s32 animation;
    s32 eligible;
    s32 target_index;
    s32 input_edge;
    s32 quad_edge;
    s32 next_edge_offset;
    s32 layer_or_index;
    s32 binding_offset;
    s32 owner_binding_offset;
    s32 center_x;
    s32 packed_center;
    s32 center_y;
    s32 in_reach;
    u8 owner_index;
    u8 target_count;
    FieldContactPoint* target_quad;
    FieldObjectRuntime* list_state;
    FieldObjectRuntime* object_states;
    FieldSequenceBinding* bindings;
    FieldSequenceBinding* binding;
    FieldActorState* slots;
    FieldContactPoint* input_vertex;
    FieldObjectRuntime* target_list;

    owner_index = effect->source_object_index;
    owner = &g_field_actors[owner_index];
    if (owner->command == FIELD_ACTOR_COMMAND_NONE)
    {
        return 0;
    }
    target_state = g_field_object_states;
    owner_state = &target_state[owner_index];
    target_index = 0;
    target = g_field_actors;
    object_states = g_field_object_states;
    bindings = g_field_actor_bindings;
    slots = g_field_actor_slots;
    for (; target_index < FIELD_ACTOR_COUNT; target_index++, target++, target_state++)
    {
        if ((target->presence == FIELD_ACTOR_UNUSED) || (target_state->object_flags & FIELD_OBJECT_UNTARGETABLE_FLAGS) ||
            ((target_index >= FIELD_PARTY_COUNT) && ((target_state->group_flags & FIELD_OBJECT_GROUP_MASK) != g_field_active_group)) ||
            (target_state->current_hp == 0) ||
            ((target->animation & FIELD_ANIMATION_INDEX_MASK) >= FIELD_ANIMATION_38 &&
             (target->animation & FIELD_ANIMATION_INDEX_MASK) <= FIELD_ANIMATION_39) ||
            (target->object_index == effect->source_object_index))
        {
            continue;
        }
        if (target_state->contact.flags & FIELD_CONTACT_TARGETED)
        {
            /* An object already targeted is only touched again by its owner's technique. */
            eligible = 0;
            if (owner->command == FIELD_ACTOR_COMMAND_TECHNIQUE)
            {
                list_state = FIELD_OBJECT_STATE_AT(object_states, owner->object_index);
                target_count = list_state->contact.bytes.target_count;
                layer_or_index = 0;
                if (target_count != 0)
                {
                    target_list = list_state;
                    do
                    {
                        if (target_list->targets[layer_or_index] != target_index)
                        {
                            continue;
                        }
                        eligible = 1;
                        break;
                    } while (++layer_or_index < target_count);
                }
            }
        }
        else
        {
            eligible = 1;
        }
        if ((eligible == 0) || (target_state->movement.word & FIELD_MOVEMENT_TINT_FLASH))
        {
            continue;
        }
        flags_or_extent = target_state->contact.flags;
        if ((flags_or_extent & FIELD_CONTACT_NO_HIT_TEST) ||
            ((flags_or_extent & FIELD_CONTACT_ANIMATION_HIDDEN) &&
             (slots[target_state->contact.bytes.controller_index].owner_object_index != effect->source_object_index)) ||
            (flags_or_extent & FIELD_CONTACT_LINKED))
        {
            continue;
        }
        if (!(flags_or_extent & FIELD_CONTACT_IGNORE_BINDING))
        {
            /* Skip an object whose own bound animation is running. */
            if (target->object_index < FIELD_PLAYER_COUNT)
            {
                binding_offset = target->object_index * sizeof(FieldSequenceBinding);
            }
            else
            {
                binding_offset = FIELD_PLAYER_COUNT * sizeof(FieldSequenceBinding);
            }
            binding = (FieldSequenceBinding*)((u8*)bindings + binding_offset);
            if (binding->owner_object_index == target->object_index)
            {
                if ((u32)(binding->owner_object_index & 0xFF) < FIELD_PLAYER_COUNT)
                {
                    owner_binding_offset = binding->owner_object_index * sizeof(FieldSequenceBinding);
                }
                else
                {
                    owner_binding_offset = FIELD_PLAYER_COUNT * sizeof(FieldSequenceBinding);
                }
                if (((FieldSequenceBinding*)((u8*)bindings + owner_binding_offset))->state != 0)
                {
                    continue;
                }
            }
        }
        command = target->command;
        if ((command == FIELD_ACTOR_COMMAND_TECHNIQUE) || (command == FIELD_ACTOR_COMMAND_INSTRUMENT) ||
            (command == FIELD_ACTOR_COMMAND_DEFEAT_DELAY))
        {
            continue;
        }
        /* Party effects only touch non-party objects and vice versa, unless duel mode is on. */
        if (g_field_duel_mode == 0 &&
            (effect->source_object_index < FIELD_PARTY_COUNT ? target->object_index < FIELD_PARTY_COUNT : target->object_index >= FIELD_PARTY_COUNT))
        {
            continue;
        }
        in_reach = (target->z - effect->z) / FIELD_CONTACT_HEIGHT_SCALE;
        if (in_reach < 0)
        {
            in_reach = -in_reach;
        }
        target_extent = target_state->collision.signed_half.extent;
        flags_or_extent = owner_state->collision.signed_half.extent;
        if (target_extent < 0)
        {
            target_extent = -target_extent;
        }
        if (flags_or_extent < 0)
        {
            flags_or_extent = -flags_or_extent;
        }
        flags_or_extent += target_extent;
        in_reach = in_reach < flags_or_extent;
        if (!in_reach)
        {
            continue;
        }
        /* Test the object's two collision quads, the upper layer (vertices 4-7) first. */
        layer_or_index = 4;
        input_quad = quad;
        for (; layer_or_index >= 0; layer_or_index -= 4)
        {
            target_quad = (FieldContactPoint*)&target_state->effect_vertices.points[layer_or_index];
            for (quad_edge = 0; quad_edge < FIELD_QUAD_VERTEX_COUNT; quad_edge++)
            {
                input_edge = 0;
                next_edge_offset = ((quad_edge + 1) & (FIELD_QUAD_VERTEX_COUNT - 1)) * sizeof(*target_quad);
                input_vertex = input_quad;
                for (; input_edge < FIELD_QUAD_VERTEX_COUNT; input_edge++, input_vertex++)
                {
                    if ((target_quad[0].packed != 0) || (target_quad[1].packed != 0))
                    {
                        intersection = field_intersect_screen_segments(&input_vertex->coord, &input_quad[(input_edge + 1) & 3].coord,
                                                                       &target_quad[quad_edge].coord,
                                                                       &((FieldContactPoint*)((u8*)target_quad + next_edge_offset))->coord);
                        contact->point = intersection;
                        if (intersection != FIELD_NO_SEGMENT_INTERSECTION)
                        {
                            if (target->object_index < FIELD_PLAYER_COUNT)
                            {
                                animation = target->animation & FIELD_ANIMATION_INDEX_MASK;
                                if (animation == FIELD_ANIMATION_3F)
                                {
                                    target->command = FIELD_ACTION_COMMAND(2);
                                    field_prepare_actor_action(target);
                                    g_field_object_states[target->object_index].contact.flags =
                                        (g_field_object_states[target->object_index].contact.flags & ~FIELD_CONTACT_QUAD_ANSWER_MASK) | FIELD_CONTACT_QUAD_ANSWER_2;
                                    return 3;
                                }
                                if (animation == FIELD_ANIMATION_40)
                                {
                                    target->command = FIELD_ACTION_COMMAND(3);
                                    field_prepare_actor_action(target);
                                    g_field_object_states[target->object_index].contact.flags =
                                        (g_field_object_states[target->object_index].contact.flags & ~FIELD_CONTACT_QUAD_ANSWER_MASK) | FIELD_CONTACT_QUAD_ANSWER_3;
                                    return 3;
                                }
                            }
                            contact->actor = target_index;
                            return layer_or_index / 4 + 1;
                        }
                    }
                }
            }
            if ((target_quad[0].packed != 0) || (target_quad[1].packed != 0))
            {
                /* No edge crossed: test whether the centre of the effect quad lies inside the object quad. */
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
                if (NormalClip(scratch.center.packed, target_quad[0].packed, target_quad[1].packed) < 0)
                {
                    if ((NormalClip(scratch.center.packed, target_quad[1].packed, target_quad[2].packed) < 0) &&
                        (NormalClip(scratch.center.packed, target_quad[2].packed, target_quad[3].packed) < 0) &&
                        (NormalClip(scratch.center.packed, target_quad[3].packed, target_quad[0].packed) < 0))
                    {
                        packed_center = (u16)scratch.center.coord.x | (scratch.center.coord.y << 16);
                        contact->actor = target_index;
                        contact->point = packed_center;
                        return layer_or_index / 4 + 1;
                    }
                }
                else if ((NormalClip(scratch.center.packed, target_quad[1].packed, target_quad[2].packed) > 0) &&
                         (NormalClip(scratch.center.packed, target_quad[2].packed, target_quad[3].packed) > 0) &&
                         (NormalClip(scratch.center.packed, target_quad[3].packed, target_quad[0].packed) > 0))
                {
                    packed_center = (u16)scratch.center.coord.x | (scratch.center.coord.y << 16);
                    contact->actor = target_index;
                    contact->point = packed_center;
                    return layer_or_index / 4 + 1;
                }
            }
        }
    }
    return 0;
}
/**
 * @brief Find the integer intersection point of two screen-space segments.
 * @param a0 Start of the first segment.
 * @param a1 End of the first segment.
 * @param b0 Start of the second segment.
 * @param b1 End of the second segment.
 * @return The point packed as x | (y << 16), or FIELD_NO_SEGMENT_INTERSECTION.
 * @note Overlapping collinear segments return the start of the first segment.
 * @note The endpoint coordinates are kept in one local per use: the loads and their order follow the original.
 */
static s32 field_intersect_screen_segments(Vec2s* a0, Vec2s* a1, Vec2s* b0, Vec2s* b1)
{
    s32 a0_x_bound, a1_x_bound;
    s32 a0_y_bound, a1_y_bound;
    s32 b0_x_bound, b1_x_bound;
    s32 b0_y_bound, b1_y_bound;
    s32 b1_coordinate;
    s32 b1_x_a;
    s32 b0_x_value;
    s32 b0_y_a;
    s32 b0_y_value_vertical;
    s32 b0_x_value_general;
    s32 a1_x_b;
    s32 a1_y_b;
    s32 a1_x_collinear;
    s32 b0_x_b;
    s32 b0_y_b;
    s32 b0_x_collinear;
    s32 b1_x_b;
    s32 b1_y_b;
    s32 b1_x_collinear;
    s32 b0_x_a;
    s32 b0_y_value;
    s32 b0_x_value_vertical;
    s32 a0_x;
    s32 x;
    s32 y;
    s32 b1_x_general;
    s32 b0_y_general;
    s32 a0_y;
    s32 a1_x;
    s32 a1_y;
    s32 a0_x_a;
    s32 a1_x_a;
    s32 a0_x_b;
    s32 b1_y_a;
    s32 a0_y_b;
    s32 a0_x_collinear;
    s32 a_slope;
    s32 b_slope;
    s32 b_dx;
    s32 b_dx_vertical;
    s32 y_shift;
    s32 a_dx;
    s32 a_dy;
    s32 b_dy_vertical;
    s32 b_dy;
    s32 work;
    u32 packed_x_or_total;

    a1_y = a1->y;
    a0_y = a0->y;
    a_dy = a1_y;
    a_dy -= a0_y;
    if (a_dy == 0)
    {
        b1_coordinate = b1->y;
        b0_y_value = b0->y;
        b_dy = b1_coordinate - b0_y_value;
        y = a0_y;
        if (b_dy == 0)
        {
            if ((y == b0_y_value) &&
                ((b0_x_a = b0->x, a0_x_a = a0->x, ((b0_x_a < a0_x_a) == 0)) ||
                 (b1_x_a = b1->x, ((b1_x_a < a0_x_a) == 0)) ||
                 (a1_x_a = a1->x, ((b0_x_a < a1_x_a) == 0)) || (b1_x_a >= a1_x_a)) &&
                ((b0_x_b = b0->x, a0_x_b = a0->x, ((a0_x_b < b0_x_b) == 0)) ||
                 (b1_x_b = b1->x, ((a0_x_b < b1_x_b) == 0)) ||
                 (a1_x_b = a1->x, ((a1_x_b < b0_x_b) == 0)) || (a1_x_b >= b1_x_b)))
            {
                packed_x_or_total = (u16)a0->x;
                work = y << 0x10;
                return packed_x_or_total | work;
            }
            return FIELD_NO_SEGMENT_INTERSECTION;
        }
        b1_coordinate = b1->x;
        b0_x_value = b0->x;
        b_dx = b1_coordinate - b0_x_value;
        if (b_dx == 0)
        {
            x = b0_x_value;
        }
        else
        {
            x = ((s32)((y - b0_y_value) * b_dx) / b_dy) + b0_x_value;
        }
    }
    else
    {
        a1_x = a1->x;
        a0_x = a0->x;
        a_dx = a1_x - a0_x;
        if (a_dx == 0)
        {
            b1_coordinate = b1->x;
            b0_x_value_vertical = b0->x;
            y_shift = 16; /* The shift held in a local: a literal 16 changes the schedule. */
            b_dx_vertical = b1_coordinate - b0_x_value_vertical;
            x = a0_x;
            if (b_dx_vertical == 0)
            {
                if ((x == b0_x_value_vertical) && ((b0_y_a = b0->y, ((b0_y_a < a0_y) == 0)) ||
                                                                          (b1_y_a = b1->y, ((b1_y_a < a0_y) == 0)) ||
                                                                          (b0_y_a >= a1_y) || (b1_y_a >= a1_y)))
                {
                    b0_y_b = b0->y;
                    a0_y_b = a0->y;
                    if ((a0_y_b >= b0_y_b) || (b1_y_b = b1->y, ((a0_y_b < b1_y_b) == 0)) ||
                        (a1_y_b = a1->y, ((a1_y_b < b0_y_b) == 0)) || (a1_y_b >= b1_y_b))
                    {
                        packed_x_or_total = x & 0xFFFF;
                        work = a0->y << y_shift;
                        return packed_x_or_total | work;
                    }
                    return FIELD_NO_SEGMENT_INTERSECTION;
                }
                return FIELD_NO_SEGMENT_INTERSECTION;
            }
            b1_coordinate = b1->y;
            b0_y_value_vertical = b0->y;
            b_dy_vertical = b1_coordinate - b0_y_value_vertical;
            if (b_dy_vertical == 0)
            {
                y = b0_y_value_vertical;
            }
            else
            {
                y = ((s32)((x - b0_x_value_vertical) * b_dy_vertical) / b_dx_vertical) +
                                 b0_y_value_vertical;
            }
        }
        else
        {
            b1_coordinate = b1->y;
            b0_y_general = b0->y;
            b_dy = b1_coordinate - b0_y_general;
            if (b_dy == 0)
            {
                y = b0_y_general;
                x = ((s32)((y - a0_y) * a_dx) / a_dy) + a0_x;
            }
            else
            {
                b1_x_general = b1->x;
                b0_x_value_general = b0->x;
                b_dx = b1_x_general - b0_x_value_general;
                if (b_dx == 0)
                {
                    x = b0_x_value_general;
                }
                else
                {
                    a_slope = a_dy * b_dx;
                    b_slope = a_dx * b_dy;
                    if (a_slope == b_slope)
                    {
                        if ((b0_y_general == (((s32)((b0_x_value_general - a0_x) * a_dy) / a_dx) + a0_y)) &&
                            ((b0_x_value_general >= a0_x) || (b1_x_general >= a0_x) ||
                             (b0_x_value_general >= a1_x) || (b1_x_general >= a1_x)))
                        {
                            b0_x_collinear = b0->x;
                            a0_x_collinear = a0->x;
                            if ((a0_x_collinear >= b0_x_collinear) ||
                                (b1_x_collinear = b1->x, ((a0_x_collinear < b1_x_collinear) == 0)) ||
                                (a1_x_collinear = a1->x, ((a1_x_collinear < b0_x_collinear) == 0)) ||
                                (a1_x_collinear >= b1_x_collinear))
                            {
                                packed_x_or_total = (u16)a0->x;
                                work = a0->y << 0x10;
                                return packed_x_or_total | work;
                            }
                            return FIELD_NO_SEGMENT_INTERSECTION;
                        }
                        return FIELD_NO_SEGMENT_INTERSECTION;
                    }
                    x = ((s32)((((b0_y_general - ((s32)(b_dy * b0_x_value_general) / b_dx)) - a0_y) +
                                             ((s32)(a_dy * a0_x) / a_dx)) *
                                            (a_dx * b_dx)) /
                                      (s32)(a_slope - b_slope));
                }

                y = ((s32)((x - a0_x) * a_dy) / a_dx) + a0_y;
            }
        }
    }

    /* The point must lie between the ends of both segments on both axes (the distance-weighted mean of the ends is the point). */
    a0_x_bound = a0->x;
    a1_x_bound = a1->x;
    work = x - a0_x_bound;
    b_dx = abs(work);
    work = x - a1_x_bound;
    work = abs(work);
    packed_x_or_total = b_dx + work;
    if (packed_x_or_total != 0)
    {
        work *= a0_x_bound;
        b_dx *= a1_x_bound;
        work += b_dx;
        if ((u32)work / packed_x_or_total != x)
        {
            return FIELD_NO_SEGMENT_INTERSECTION;
        }
    }
    a0_y_bound = a0->y;
    a1_y_bound = a1->y;
    work = y - a0_y_bound;
    b_dx = abs(work);
    work = y - a1_y_bound;
    work = abs(work);
    packed_x_or_total = b_dx + work;
    if (packed_x_or_total != 0)
    {
        work *= a0_y_bound;
        b_dx *= a1_y_bound;
        work += b_dx;
        if ((u32)work / packed_x_or_total != y)
        {
            return FIELD_NO_SEGMENT_INTERSECTION;
        }
    }
    b0_x_bound = b0->x;
    b1_x_bound = b1->x;
    work = x - b0_x_bound;
    b_dx = abs(work);
    work = x - b1_x_bound;
    work = abs(work);
    packed_x_or_total = b_dx + work;
    if (packed_x_or_total != 0)
    {
        work *= b0_x_bound;
        b_dx *= b1_x_bound;
        work += b_dx;
        if ((u32)work / packed_x_or_total != x)
        {
            return FIELD_NO_SEGMENT_INTERSECTION;
        }
    }
    b0_y_bound = b0->y;
    b1_y_bound = b1->y;
    work = y - b0_y_bound;
    b_dx = abs(work);
    work = y - b1_y_bound;
    work = abs(work);
    packed_x_or_total = b_dx + work;
    if (packed_x_or_total != 0)
    {
        work *= b0_y_bound;
        b_dx *= b1_y_bound;
        work += b_dx;
        if ((u32)work / packed_x_or_total != y)
        {
            return FIELD_NO_SEGMENT_INTERSECTION;
        }
    }
    packed_x_or_total = x & 0xFFFF;
    work = y << 16;
    return packed_x_or_total | work;
}
/**
 * @brief Move an actor by a displacement, resolved against the map, the group bounds and the other actors.
 * @param actor Actor to move; its position and blocked flags are updated.
 * @param position In: displacement (x, y, z). Out: the resolved position.
 * @param mode Forwarded to field_find_actor_overlap as its filter_group.
 * @return 1 when the actor moved (or was already overlapping), 0 when the move was refused.
 */
s32 field_resolve_actor_movement(FieldActor* actor, s32* position, s32 mode)
{
    Vec3i delta;
    s32 hit;
    FieldMoveBounds* bounds = (FieldMoveBounds*)FIELD_MAP_BOUNDS_ADDRESS;
    struct FieldCollisionMover* mover = (struct FieldCollisionMover*)FIELD_MOVE_REQUEST_ADDRESS;
    FieldCollisionQuery* query = (FieldCollisionQuery*)FIELD_MOVE_QUERY_ADDRESS;
    s32 requested_x;
    s32 actor_x;
    s32 requested_z;
    s32 can_move;
    u16 command;
    u16 resolved_command;

    if (((u32)g_field_object_parts[actor->object_index].flags >> FIELD_PART_IGNORE_MAP_COLLISION_BIT) & 1)
    {
        actor->x += position[0];
        actor->y += position[1];
        /* position[2] through a stepped pointer: a direct index swaps two saved registers. */
        position += 2;
        actor->z += position[0];
        return 1;
    }
    if (g_field_active_group != 0)
    {
        command = actor->command;
        if (((command < FIELD_ACTOR_COMMAND_WALK_PATH) || (command > FIELD_ACTOR_COMMAND_RUN_PATH)) && ((s16)command != FIELD_ACTOR_COMMAND_LEAVE_PATH) &&
            (field_move_leaves_screen(actor, (Vec3i*)position) != 0))
        {
            position[0] = 0;
        }
    }
    actor_x = actor->x;
    if ((actor_x >= 0) && (actor_x < (bounds->width << 8)) && (actor->z >= 0) && (actor->z < ((s32)(bounds->depth << 16) >> 7)))
    {
        mover->x = actor_x;
        mover->height = actor->y;
        mover->z = actor->z;
        mover->move_x = position[0];
        requested_x = mover->move_x;
        mover->move_height = position[1];
        mover->move_z = position[2];
        requested_z = mover->move_z;
        mover->height_bias = FIELD_MOVE_HEIGHT_TOLERANCE;
        query->height_tolerance = FIELD_MOVE_HEIGHT_TOLERANCE;
        if (g_field_object_parts[actor->object_index].scale_z == FIELD_PART_FULL_SCALE)
        {
            mover->footprint_width = FIELD_MOVE_LARGE_WIDTH;
            query->width = FIELD_MOVE_LARGE_WIDTH;
            mover->mode.h.footprint_depth = FIELD_MOVE_LARGE_STEP;
            query->depth = FIELD_MOVE_LARGE_STEP;
        }
        else
        {
            mover->footprint_width = FIELD_MOVE_WIDTH;
            query->width = FIELD_MOVE_WIDTH;
            mover->mode.h.footprint_depth = FIELD_MOVE_STEP;
            query->depth = FIELD_MOVE_STEP;
        }
        mover->height_bias = FIELD_MOVE_HEIGHT_TOLERANCE;
        mover->mode.bits.airborne_high = 0;
        mover->mode.bits.airborne_low = 0;
        mover->collision_node = g_field_object_states[actor->object_index].contact_index;
        mover->flags = g_field_object_states[actor->object_index].surface;
        field_collision_move_mover(mover);
        g_field_object_states[actor->object_index].contact_index = mover->collision_node;
        g_field_object_states[actor->object_index].surface = mover->flags;
        resolved_command = actor->command;
        if (((resolved_command >= FIELD_ACTOR_COMMAND_WALK_PATH) && (resolved_command <= FIELD_ACTOR_COMMAND_RUN_PATH)) ||
            ((s16)resolved_command == FIELD_ACTOR_COMMAND_LEAVE_PATH))
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
        query->y = position[1] + actor->y;
        query->z = mover->z;
        /* field_collision_hit_markers is called as returning int: the original compares the s16 result unextended. */
        if (((g_field_active_group == 0) || (actor->control.word & FIELD_CONTROL_MODE_MASK) ||
             (((s32 (*)(struct FieldCollisionQuery*))field_collision_hit_markers)(query) == -1)) &&
            ((((u16)actor->command >= FIELD_ACTOR_COMMAND_WALK_PATH) && ((u16)actor->command <= FIELD_ACTOR_COMMAND_RUN_PATH)) ||
             (actor->command == FIELD_ACTOR_COMMAND_LEAVE_PATH) || (g_field_active_group == 0) || (field_move_leaves_screen(actor, &delta) == 0)))
        {
            position[0] = mover->x;
            if (((actor->animation & FIELD_ANIMATION_INDEX_MASK) == FIELD_ANIMATION_3D) && (actor->object_index < FIELD_PLAYER_COUNT))
            {
                position[1] = position[1] + actor->y;
            }
            else
            {
                position[1] = mover->height;
            }
            position[2] = mover->z;
            g_field_object_states[actor->object_index].movement.half.height = mover->resolved_height / 256;
        }
        else
        {
            position[0] = actor->x;
            position[1] = actor->y;
            position[2] = actor->z;
        }
    }
    else
    {
        g_field_object_states[actor->object_index].movement.half.height = 0;
        g_field_object_states[actor->object_index].contact_index = -1;
        g_field_object_states[actor->object_index].surface = 0;
        position[0] = actor->x;
        position[1] = actor->y;
        position[2] = actor->z;
    }
    g_field_last_actor_contact = 0;
    if (((u32)g_field_object_states[actor->object_index].movement.word >> FIELD_MOVEMENT_OUTSIDE_BOUNDS_BIT) & 1)
    {
        if (field_is_outside_group_bounds(actor, &actor->x) == 0)
        {
            g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_OUTSIDE_BOUNDS;
        }
        can_move = 1;
    }
    else if (field_is_outside_group_bounds(actor, position) == 0)
    {
        can_move = 1;
        g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_OUTSIDE_BOUNDS;
    }
    else
    {
        hit = field_is_outside_group_bounds(actor, &actor->x);
        can_move = 0;
        if (hit != 0)
        {
            g_field_object_states[actor->object_index].movement.word |= FIELD_MOVEMENT_OUTSIDE_BOUNDS;
        }
    }
    if (can_move == 0)
    {
        return 0;
    }
    if (((u32)g_field_object_states[actor->object_index].movement.word >> FIELD_MOVEMENT_OVERLAPPING_BIT) & 1)
    {
        actor->x = position[0];
        actor->y = position[1];
        actor->z = position[2];
        if (field_find_actor_overlap(actor, &actor->x, mode) == 0)
        {
            g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_OVERLAPPING;
        }
        return 1;
    }
    if (field_find_actor_overlap(actor, position, mode) == 0)
    {
        actor->x = position[0];
        actor->y = position[1];
        actor->z = position[2];
        g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_OVERLAPPING;
        return 1;
    }
    if (field_find_actor_overlap(actor, &actor->x, mode) != 0)
    {
        g_field_object_states[actor->object_index].movement.word |= FIELD_MOVEMENT_OVERLAPPING;
    }
    return 0;
}
/**
 * @brief Test whether a position lies outside the X band of the active group.
 * @param actor Actor tested; objects without a group are never outside.
 * @param position Fixed-point position; only X is tested.
 * @return 1 when X is outside the band of g_field_active_group, otherwise 0.
 */
static s32 field_is_outside_group_bounds(FieldActor* actor, s32* position)
{
    FieldGroupBounds* bounds;
    FieldGroupBounds* group_bounds;
    s32 group_index;
    s32 minimum;
    s32 value;

    if ((g_field_object_states[actor->object_index].group_flags & FIELD_OBJECT_GROUP_MASK) == 0)
    {
        return 0;
    }
    /* Indexing g_field_group_bounds[g_field_active_group - 1] directly changes the codegen. */
    group_bounds = g_field_group_bounds;
    group_index = g_field_active_group - 1;
    bounds = &group_bounds[group_index];
    minimum = bounds->left;
    value = position[0] >> 8;
    if (value < minimum)
    {
        return 1;
    }
    return minimum + bounds->width < value;
}
/**
 * @brief Find the first object an actor would overlap at a position; a touched idle object turns to it.
 * @param actor Actor being tested (its own record is skipped).
 * @param position Fixed-point position to test.
 * @param filter_group Nonzero to test only the opposing group (unless duel mode is on).
 * @return The overlapped object index plus FIELD_CONTACT_RESULT_PRESENT, or 0 for none or a touch reaction.
 */
s32 field_find_actor_overlap(FieldActor* actor, s32* position, s32 filter_group)
{
    FieldContactScanWorkspace scratch;
    s32 base_or_index;
    u8* bindings;
    s32 actor_center_offset;
    FieldObjectRuntime* target_state;
    FieldActor* target;
    s32 position_y;
    s32 animation_slot;
    s32 contact_flags;
    s32 index;
    s32 target_index;
    s32 target_end;
    s32 binding_offset;
    s32 owner_binding_offset;
    FieldSequenceBinding* binding;
    s8 actor_height;
    s8 target_height;
    FieldObjectRuntime* actor_state;

    if (filter_group != 0 && g_field_duel_mode == 0)
    {
        if (actor->object_index < FIELD_PARTY_COUNT)
        {
            target = &g_field_actors[FIELD_PARTY_COUNT];
            target_state = &g_field_object_states[FIELD_PARTY_COUNT];
            index = FIELD_PARTY_COUNT;
            target_end = FIELD_ACTOR_COUNT;
        }
        else
        {
            target = g_field_actors;
            target_state = g_field_object_states;
            index = 0;
            target_end = FIELD_PARTY_COUNT;
        }
    }
    else
    {
        target = g_field_actors;
        target_state = g_field_object_states;
        index = 0;
        target_end = FIELD_ACTOR_COUNT;
    }
    if (actor->height >= FIELD_OVERLAP_MAX_HEIGHT)
    {
    /* The later failures jump here; written as plain returns, jump.c cross-jumps this block into a later copy. */
    return_zero:
        return 0;
    }
    target_index = index;
    actor_state = &g_field_object_states[actor->object_index];
    actor_center_offset = actor_state->collision.half.center_offset << 8;
    for (; target_index < target_end; target_index++, target++, target_state++)
    {
        if ((target->presence == FIELD_ACTOR_UNUSED) ||
            (target_state->object_flags & FIELD_OBJECT_NO_OVERLAP_FLAGS) ||
            (target == actor) ||
            (contact_flags = target_state->contact.flags, ((contact_flags & FIELD_CONTACT_NO_HIT_TEST) != 0)) ||
            (contact_flags & FIELD_CONTACT_ANIMATION_HIDDEN) || (target_state->collision.word == 0))
        {
            continue;
        }
        target_height = target->height;
        actor_height = actor->height;
        position_y = position[1];
        if (((target->y + ((target_state->bounds.half.top + target_height) << 8)) >
             (position_y + ((actor_state->bounds.half.bottom + actor_height) << 8))) ||
            ((target->y + ((target_state->bounds.half.bottom + target_height) << 8)) <
             (position_y + ((actor_state->bounds.half.top + actor_height) << 8))))
        {
            continue;
        }
        scratch.delta.vx = (target->z - position[2]) >> 8;
        scratch.delta.vy = ((target->x + (target_state->collision.half.center_offset << 8)) - (position[0] + actor_center_offset)) >> 8;
        scratch.delta.vz = 0;
        gte_ldlvl(&scratch.delta);
        gte_sqr0();
        gte_stlvnl(&scratch.squared);
        if (SquareRoot0(scratch.squared.vx + scratch.squared.vy) <
            (((s32)(actor_state->collision.half.diameter << 16) >> 17) + ((s32)(target_state->collision.half.diameter << 16) >> 17)))
        {
            break;
        }
    }
    if (target_index == target_end)
    {
        g_field_last_actor_contact = 0;
        goto return_zero;
    }
    if (g_field_resource_entries[actor->resource_index].state != 0)
    {
        if (g_field_resource_entries[target->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS)
        {
            if (target_state->current_hp != 0)
            {
                if (!(target_state->object_flags & (FIELD_OBJECT_FLAG_0080 | FIELD_OBJECT_FLAG_KNOCKED_OUT)))
                {
                    if (!(target_state->contact.flags & FIELD_CONTACT_ANIMATION_HIDDEN))
                    {
                        index = target->object_index;
                        if (!(((u32)g_field_object_states[index].contact.flags >> FIELD_CONTACT_IGNORE_BINDING_BIT) & 1))
                        {
                            /* One local holds the binding table address and then the object index. */
                            base_or_index = (s32)g_field_actor_bindings;
                            if ((u32)index < FIELD_PLAYER_COUNT)
                            {
                                binding_offset = index * sizeof(FieldSequenceBinding);
                            }
                            else
                            {
                                binding_offset = FIELD_PLAYER_COUNT * sizeof(FieldSequenceBinding);
                            }
                            binding = (FieldSequenceBinding*)(base_or_index + binding_offset);
                            base_or_index = target->object_index;
                            index = binding->owner_object_index;
                            if (index == base_or_index)
                            {
                                bindings = (u8*)g_field_actor_bindings;
                                if ((u32)(index & 0xFF) < FIELD_PLAYER_COUNT)
                                {
                                    owner_binding_offset = index * sizeof(FieldSequenceBinding);
                                }
                                else
                                {
                                    owner_binding_offset = FIELD_PLAYER_COUNT * sizeof(FieldSequenceBinding);
                                }

                                if (((FieldSequenceBinding*)(bindings + owner_binding_offset))->state != 0)
                                {
                                    goto return_zero;
                                }
                            }
                        }

                        if (target->command == FIELD_ACTOR_COMMAND_NONE)
                        {
                            animation_slot = field_find_free_actor_slot(target->object_index, 0);
                            if (animation_slot != -1)
                            {
                                scratch.targets.words[0] = (s32)actor->object_index;

                                if (field_start_builtin_animation(target->object_index, animation_slot, FIELD_TOUCH_ANIMATION) != 0)
                                {
                                    if (target->object_index < FIELD_PLAYER_COUNT)
                                    {
                                        field_command_history_clear(target->object_index);
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
    g_field_last_actor_contact = target_index + FIELD_CONTACT_RESULT_PRESENT;
    return g_field_last_actor_contact;
}
/**
 * @brief Start the leader's touch interaction with an object it walked into.
 * @param actor Moving actor; only an idle object 0 (the leader) starts interactions.
 * @param object_index Object that was touched.
 */
void field_start_actor_contact_interaction(FieldActor* actor, s32 object_index)
{
    u8* resource_data;
    s32 object_index_x8;
    FieldObjectRuntime* states;
    FieldObjectRuntime* state;

    if (actor->object_index != 0)
    {
        return;
    }
    object_index_x8 = object_index << 3;
    if (actor->command != FIELD_ACTOR_COMMAND_NONE)
    {
        return;
    }
    states = g_field_object_states;
    /* object_index * sizeof(FieldObjectRuntime), spelled as shifts: the target starts it before the command test. */
    state = (FieldObjectRuntime*)((u8*)states + ((((object_index_x8 + object_index) << 4) - object_index) << 2));
    if ((state->interaction_flags & FIELD_INTERACTION_FLAG_ENABLED) == 0)
    {
        return;
    }

    field_dispatch_object_state_entry(&g_field_actors[object_index], 0);
    actor->command = FIELD_ACTOR_COMMAND_SEQUENCE_WAIT;
    actor->command_param = FIELD_CONTACT_INTERACTION_DELAY;

    if (g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS)
    {
        actor->animation &= FIELD_ANIMATION_FACING;
    }
    else
    {
        u8 animation = actor->animation;
        s32 animation_index = animation & FIELD_ANIMATION_INDEX_MASK;
        actor->animation = (animation_index % FIELD_ANIMATION_DIRECTIONS) | (animation & FIELD_ANIMATION_FACING);
    }

    actor->animation_frame = 0;
    actor->animation_active = 1;
    resource_data = g_field_resource_entries[actor->resource_index].start;
    field_restart_actor_animation(actor, resource_data);
}
/**
 * @brief Look for an object just ahead of an idle actor and start its talk or action interaction.
 * @param actor Idle actor (the leader); the probe point lies one unit ahead in its direction.
 */
void field_probe_actor_interaction(FieldActor* actor)
{
    s32 position[3];
    s32 object_index;
    s32 result_or_state;
    u8 animation;
    s32 animation_index;
    FieldObjectRuntime* state;
    FieldObjectRuntime* states;

    if (actor->command != FIELD_ACTOR_COMMAND_NONE)
    {
        return;
    }
    position[0] = actor->x;
    position[1] = actor->y;
    position[2] = actor->z;
    position[0] += rcos(actor->direction * 16);
    position[2] -= rsin(actor->direction * 16);
    result_or_state = field_find_actor_overlap(actor, position, 1);
    object_index = result_or_state & FIELD_CONTACT_RESULT_ACTOR_MASK;
    if (result_or_state != 0)
    {
        states = g_field_object_states;
        /* One local holds the overlap result and then the state address; a direct pointer changes the allocation. */
        result_or_state = (s32)&states[object_index];
        state = (FieldObjectRuntime*)result_or_state;
        if (state->interaction_kind != 0)
        {
            field_play_sound(FIELD_SOUND_INTERACT, FIELD_SOUND_PAN_CENTER);
            field_pick_up_item_actor(object_index);
            return;
        }
        if (state->interaction_flags & FIELD_INTERACTION_FLAG_TRIGGERED)
        {
            field_dispatch_object_state_entry(&g_field_actors[object_index], 0);
            actor->command = FIELD_ACTOR_COMMAND_STEP;
            actor->animation_state = 1;
            if (g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS)
            {
                actor->animation = (u8)(actor->animation & FIELD_ANIMATION_FACING);
            }
            else
            {
                animation = actor->animation;
                animation_index = animation & FIELD_ANIMATION_INDEX_MASK;
                actor->animation = (animation_index % FIELD_ANIMATION_DIRECTIONS) | (animation & FIELD_ANIMATION_FACING);
            }
            actor->animation_frame = 0;
            actor->animation_active = 1;
            field_restart_actor_animation(actor, g_field_resource_entries[actor->resource_index].start);
        }
    }
}
/**
 * @brief Run one of an object's state entries through the object script dispatcher.
 * @param actor Object whose state entry runs.
 * @param entry_index Entry of FieldObjectRuntime::state_entries to run.
 */
static void field_dispatch_object_state_entry(FieldActor* actor, s32 entry_index)
{
    FieldObjectRuntime* state;

    state = &g_field_object_states[actor->object_index];
    field_start_interaction(state->record_id, state->state_entries[entry_index], g_field_object_states);
}
/**
 * @brief Hit every new object whose projected bounds contain an effect's position.
 * @param effect Effect record; its position is the hit point and its owner is the attacker.
 * @param radius Hit radius, added to the object bounds and to the depth test.
 * @param actor Animation actor of the attack; each hit object becomes one of its tracks.
 * @note Objects already tracked by @p actor are not hit again; a hit object is also added to the owner's targets.
 */
void field_collect_effect_hits(FieldMotionRecord* effect, s32 radius, FieldActorState* actor)
{
    FieldActor* actors;
    FieldObjectRuntime* states;
    FieldSequenceBinding* bindings;
    FieldSequenceBinding* binding;
    s32 bound_x_a;
    s32 bound_y_a;
    s16 command;
    s32 bound_x_b;
    s32 bound_y_b;
    s32 min_y;
    s32 max_y;
    s32 min_x;
    s32 max_x;
    FieldActor* candidate;
    s32 contact_flags;
    s32 delta_z;
    s32 origin_z;
    s32 depth_projection;
    s32 candidate_z;
    s32 candidate_position;
    s32 eligible;
    s32 candidate_end;
    s32 binding_offset;
    s32 owner_binding_offset;
    s32 prior_index;
    s32 track_index;
    s32 depth_distance;
    u16 diameter;
    s32 origin_position;
    s32 candidate_index;
    u8 source_index;
    u8 prior_count;
    u8 owner_index;
    u8 track_count;
    u8 previous_state;
    FieldObjectRuntime* source_state;
    FieldObjectRuntime* candidate_state;
    FieldObjectRuntime* prior_list;

    if (g_field_duel_mode != 0)
    {
        candidate_index = 0;
        candidate_end = FIELD_ACTOR_COUNT;
    }
    else if (actor->animation->sync_flags & FIELD_ANIMATION_SAME_GROUP)
    {
        if (actor->owner_object_index < FIELD_PARTY_COUNT)
        {
            candidate_index = 0;
            candidate_end = FIELD_PARTY_COUNT;
        }
        else
        {
            candidate_index = FIELD_PARTY_COUNT;
            candidate_end = FIELD_ACTOR_COUNT;
        }
    }
    else if (actor->owner_object_index < FIELD_PARTY_COUNT)
    {
        candidate_index = FIELD_PARTY_COUNT;
        candidate_end = FIELD_ACTOR_COUNT;
    }
    else
    {
        candidate_index = 0;
        candidate_end = FIELD_PARTY_COUNT;
    }
    candidate = &g_field_actors[candidate_index];
    candidate_state = &g_field_object_states[candidate_index];
    if (candidate_index < candidate_end)
    {
        actors = g_field_actors;
        states = g_field_object_states;
        bindings = g_field_actor_bindings;
        for (; candidate_index < candidate_end; candidate_index++, candidate++, candidate_state++)
        {
            if (candidate_state->contact.flags & FIELD_CONTACT_TARGETED)
            {
                source_index = effect->source_object_index;
                eligible = 0;
                if (actors[source_index].command == FIELD_ACTOR_COMMAND_TECHNIQUE)
                {
                    source_state = FIELD_OBJECT_STATE_AT(states, source_index);
                    prior_count = source_state->contact.bytes.target_count;
                    prior_index = 0;
                    if (prior_count != 0)
                    {
                        prior_list = source_state;
                        do
                        {
                            if (prior_list->targets[prior_index] != candidate_index)
                            {
                                continue;
                            }
                            eligible = 1;
                            break;
                        } while (++prior_index < (s32)prior_count);
                    }
                }
            }
            else
            {
                eligible = 1;
            }
            owner_index = actor->owner_object_index;
            if ((candidate_index == owner_index) || ((candidate_state->effect_vertices.words[0] == 0) && (candidate_state->effect_vertices.words[2] == 0)) ||
                ((candidate_state->bounds.words[0] == 0) && (candidate_state->bounds.words[1] == 0)) || (candidate_state->collision.word == 0))
            {
                continue;
            }
            command = candidate->command;
            if ((command == FIELD_ACTOR_COMMAND_TECHNIQUE) || (command == FIELD_ACTOR_COMMAND_DEFEAT_DELAY) || (command == FIELD_ACTOR_COMMAND_INSTRUMENT) || (candidate->presence == FIELD_ACTOR_UNUSED) ||
                (owner_index == candidate_index) || (candidate_state->current_hp == 0))
            {
                continue;
            }
            contact_flags = candidate_state->contact.flags;
            if ((contact_flags & FIELD_CONTACT_ANIMATION_HIDDEN) ||
                ((candidate_index >= FIELD_PARTY_COUNT) && ((candidate_state->group_flags & FIELD_OBJECT_GROUP_MASK) != g_field_active_group)) ||
                (contact_flags & FIELD_CONTACT_NO_HIT_TEST) || (eligible == 0) ||
                (candidate_state->movement.word & FIELD_MOVEMENT_TINT_FLASH))
            {
                continue;
            }
            if (!(contact_flags & FIELD_CONTACT_IGNORE_BINDING))
            {
                s32 binding_owner;
                s32 object_index;
                if (candidate->object_index < FIELD_PLAYER_COUNT)
                {
                    binding_offset = candidate->object_index * sizeof(FieldSequenceBinding);
                }
                else
                {
                    binding_offset = FIELD_PLAYER_COUNT * sizeof(FieldSequenceBinding);
                }

                binding = (FieldSequenceBinding*)((u8*)bindings + binding_offset);
                object_index = candidate->object_index;
                binding_owner = binding->owner_object_index;
                if (binding_owner == object_index)
                {
                    if ((u32)(binding_owner & 0xFF) < FIELD_PLAYER_COUNT)
                    {
                        owner_binding_offset = binding_owner * sizeof(FieldSequenceBinding);
                    }
                    else
                    {
                        owner_binding_offset = FIELD_PLAYER_COUNT * sizeof(FieldSequenceBinding);
                    }
                    if (((FieldSequenceBinding*)((u8*)bindings + owner_binding_offset))->state != 0)
                    {
                        continue;
                    }
                }
            }
            if (candidate_state->object_flags &
                FIELD_OBJECT_UNTARGETABLE_FLAGS)
            {
                continue;
            }
            track_count = actor->track_count;
            track_index = 0;
            while (track_index < (s32)track_count && candidate_index != actor->track_object_indices[track_index])
            {
                track_index += 1;
            }
            if (track_index != actor->track_count)
            {
                continue;
            }
            /* Depth first, then the object's screen bounds widened by the radius and shifted by half the depth. */
            candidate_z = candidate->z;
            origin_z = effect->z;
            delta_z = candidate_z - origin_z;
            depth_distance = (candidate_z - origin_z) / FIELD_PROJECTED_DEPTH_SCALE;
            diameter = candidate_state->collision.half.diameter;
            if (depth_distance < 0)
            {
                depth_distance = -depth_distance;
            }
            depth_distance = depth_distance < (radius + ((s32)(diameter << 0x10) >> 0x11));
            if (!depth_distance)
            {
                continue;
            }
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
            delta_z /= 512;
            depth_projection = delta_z / 2;
            min_y -= depth_projection;
            max_y -= depth_projection;
            candidate_position = candidate->x;
            origin_position = effect->x;
            if (((candidate_position + (min_x << 8)) >= origin_position) || (origin_position >= (candidate_position + (max_x << 8))))
            {
                continue;
            }
            candidate_position = candidate->y;
            origin_position = effect->y;
            if (((candidate_position + (min_y << 8)) >= origin_position) || (origin_position >= (candidate_position + (max_y << 8))) ||
                (states[actor->owner_object_index].contact.bytes.target_count >= FIELD_MAX_CONTACT_TARGETS))
            {
                continue;
            }
            candidate_state->contact.flags |= FIELD_CONTACT_TARGETED;
            candidate_state->object_flags &= ~FIELD_OBJECT_FLAG_CLEAR_ON_HIT;
            states[actor->owner_object_index].targets[states[actor->owner_object_index].contact.bytes.target_count] = candidate_index;
            states[actor->owner_object_index].contact.bytes.target_count++;
            actor->active_track_mask |= 1 << actor->track_count;
            actor->track_object_indices[actor->track_count] = candidate_index;
            if (candidate->animation & FIELD_ANIMATION_FACING)
            {
                actor->track_offsets[actor->track_count].x = (candidate->x - effect->x) >> 8;
            }
            else
            {
                actor->track_offsets[actor->track_count].x = (effect->x - candidate->x) >> 8;
            }

            actor->track_offsets[actor->track_count].y = ((effect->y - candidate->y) >> 8) - ((effect->z - candidate->z) >> 9);

            if (effect->flags & FIELD_EFFECT_RETIRE_ON_HIT)
            {
                previous_state = effect->state;
                effect->state = FIELD_EFFECT_RETIRED;
                effect->height_or_retired_state = previous_state;
            }
            actor->track_count++;
            field_release_object_link(&actors[candidate_index]);
            if ((candidate_index < FIELD_PLAYER_COUNT) && !(actors[candidate_index].control.half[0] & FIELD_CONTROL_MODE_MASK))
            {
                field_command_history_clear(candidate_index);
            }
            if (((u8)actor->hit_reaction < FIELD_HIT_REACTION_DIRECT_COUNT) || (actors[actor->owner_object_index].command == FIELD_ACTOR_COMMAND_WAIT_BOUND_ACTOR))
            {
                field_resolve_object_hit(actor->owner_object_index, candidate_index, actor->hit_reaction);
            }
            else
            {
                switch (actor->hit_reaction)
                {
                case 0x34:
                    field_resolve_object_hit(actor->owner_object_index, candidate_index, 0x16U);
                    break;
                case 0x50:
                    field_resolve_object_hit(actor->owner_object_index, candidate_index, 0x12U);
                    break;
                case 0x51:
                    field_resolve_object_hit(actor->owner_object_index, candidate_index, 0x13U);
                    break;
                case 0x4E:
                    field_resolve_object_hit(actor->owner_object_index, candidate_index, 0x14U);
                    break;
                case 0x4F:
                    field_resolve_object_hit(actor->owner_object_index, candidate_index, 0x15U);
                    break;
                case 0x3E:
                    field_resolve_object_hit(actor->owner_object_index, candidate_index, 0x19U);
                    break;
                case 0x45:
                    field_resolve_object_hit(actor->owner_object_index, candidate_index, 0x1AU);
                    break;
                default:
                    field_resolve_contact_hit(actor->owner_object_index, candidate_index);
                    break;
                }
            }
        }
    }
}
/**
 * @brief Find the first live object within a distance of a position.
 * @param reference_position Fixed-point position (x, y, z).
 * @param distance_limit Distance in whole units, extended by half the object's collision diameter.
 * @param source_actor Animation actor whose owner and hit group select the searched objects.
 * @param opposing_group 0 to search the party; nonzero to search the group the actor hits, without its owner.
 * @return The object index, or -1 when no object is close enough.
 */
s32 field_find_actor_in_range(s32* reference_position, s32 distance_limit, FieldActorState* source_actor, s32 opposing_group)
{
    VECTOR* delta = (VECTOR*)FIELD_GTE_DELTA_ADDRESS;
    VECTOR* squared = (VECTOR*)FIELD_GTE_SQUARE_ADDRESS;
    FieldActor* candidate;
    FieldObjectRuntime* candidate_state;
    s32 candidate_index;
    s32 candidate_end;
    u8 presence;

    if (opposing_group == 0)
    {
        candidate_index = 0;
        candidate_end = FIELD_PARTY_COUNT;
    }
    else if (source_actor->animation->sync_flags & FIELD_ANIMATION_SAME_GROUP)
    {
        if (source_actor->owner_object_index < FIELD_PARTY_COUNT)
        {
            candidate_index = 0;
            candidate_end = FIELD_PARTY_COUNT;
        }
        else
        {
            candidate_index = FIELD_PARTY_COUNT;
            candidate_end = FIELD_ACTOR_COUNT;
        }
    }
    else if (source_actor->owner_object_index < FIELD_PARTY_COUNT)
    {
        candidate_index = FIELD_PARTY_COUNT;
        candidate_end = FIELD_ACTOR_COUNT;
    }
    else
    {
        candidate_index = 0;
        candidate_end = FIELD_PARTY_COUNT;
    }
    candidate = &g_field_actors[candidate_index];
    candidate_state = &g_field_object_states[candidate_index];
    for (; candidate_index < candidate_end; candidate_index++, candidate++, candidate_state++)
    {
        presence = candidate->presence;
        if (presence == FIELD_ACTOR_UNUSED || candidate_state->current_hp == 0 || presence == FIELD_ACTOR_HIDDEN ||
            (opposing_group != 0 && source_actor->owner_object_index == candidate_index))
        {
            continue;
        }
        delta->vx = (candidate->x - reference_position[0]) >> 8;
        delta->vy = (candidate->y - reference_position[1]) >> 8;
        delta->vz = (candidate->z - reference_position[2]) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(squared);
        if (SquareRoot0(squared->vx + squared->vy + squared->vz) < distance_limit + ((s32)(candidate_state->collision.half.diameter << 16) >> 17))
        {
            return candidate_index;
        }
    }
    return -1;
}
/**
 * @brief Hit every new object inside one of an attack's hit spheres.
 * @param actor Animation actor of the attack; each hit object becomes one of its tracks.
 * @param part Attacking part; it gives the sphere radius and centres (three for a swing, else one).
 */
void field_collect_attack_sphere_hits(FieldActorState* actor, FieldActorPartDef* part)
{
    u8* bindings;
    FieldActor* actors;
    FieldObjectRuntime* states;
    VECTOR* delta = (VECTOR*)FIELD_GTE_DELTA_ADDRESS;
    VECTOR* squares = (VECTOR*)FIELD_GTE_SQUARE_ADDRESS;
    s32 target_end;
    s32 sphere_count;
    s32 attack_radius;
    VECTOR* spheres;
    FieldActor* target;
    s16 command;
    s32 contact_flags;
    s32 binding_owner;
    s32 object_index;
    s32 eligible;
    VECTOR* sphere;
    s32 sphere_index;
    s32 binding_offset;
    s32 prior_index;
    s32 track_index;
    s32 target_index;
    u8 owner_index;
    u8 prior_count;
    u8 track_count;
    FieldObjectRuntime* owner_state;
    FieldObjectRuntime* prior_list;
    FieldObjectRuntime* target_state;

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
    if (g_field_duel_mode != 0)
    {
        target_index = 0;
        target_end = FIELD_ACTOR_COUNT;
    }
    else if (actor->animation->sync_flags & FIELD_ANIMATION_SAME_GROUP)
    {
        if (actor->owner_object_index < FIELD_PARTY_COUNT)
        {
            target_index = 0;
            target_end = FIELD_PARTY_COUNT;
        }
        else
        {
            target_index = FIELD_PARTY_COUNT;
            target_end = FIELD_ACTOR_COUNT;
        }
    }
    else if (actor->owner_object_index < FIELD_PARTY_COUNT)
    {
        target_index = FIELD_PARTY_COUNT;
        target_end = FIELD_ACTOR_COUNT;
    }
    else
    {
        target_index = 0;
        target_end = FIELD_PARTY_COUNT;
    }
    target = &g_field_actors[target_index];
    target_state = &g_field_object_states[target_index];
    if (target_index < target_end)
    {
        bindings = g_field_actor_bindings;
        actors = g_field_actors;
        states = g_field_object_states;
        for (; target_index < target_end; target_index++, target++, target_state++)
        {
            if (target_state->contact.flags & FIELD_CONTACT_TARGETED)
            {
                owner_index = actor->owner_object_index;
                eligible = 0;
                if (g_field_actors[owner_index].command == FIELD_ACTOR_COMMAND_TECHNIQUE)
                {
                    owner_state = FIELD_OBJECT_STATE_AT(states, owner_index);
                    prior_count = owner_state->contact.bytes.target_count;
                    prior_index = 0;
                    if (prior_count != 0)
                    {
                        prior_list = owner_state;
                        do
                        {
                            if (prior_list->targets[prior_index] != target_index)
                            {
                                continue;
                            }
                            eligible = 1;
                            break;
                        } while (++prior_index < prior_count);
                    }
                }
            }
            else
            {
                eligible = 1;
            }
            if ((target_index == actor->owner_object_index) || (target_state->collision.word == 0))
            {
                continue;
            }
            command = target->command;
            if ((command == FIELD_ACTOR_COMMAND_TECHNIQUE) || (command == FIELD_ACTOR_COMMAND_DEFEAT_DELAY) || (command == FIELD_ACTOR_COMMAND_INSTRUMENT) ||
                ((target_index < FIELD_PLAYER_COUNT) && ((target->animation & FIELD_ANIMATION_INDEX_MASK) == FIELD_ANIMATION_3C)) ||
                (target->presence == FIELD_ACTOR_UNUSED) || (actor->owner_object_index == target_index) ||
                (target_state->current_hp == 0))
            {
                continue;
            }
            contact_flags = target_state->contact.flags;
            if ((contact_flags & FIELD_CONTACT_ANIMATION_HIDDEN) ||
                ((target_index >= FIELD_PARTY_COUNT) && ((target_state->group_flags & FIELD_OBJECT_GROUP_MASK) != g_field_active_group)) ||
                (contact_flags & FIELD_CONTACT_NO_HIT_TEST) || (eligible == 0) ||
                (target_state->movement.word & FIELD_MOVEMENT_TINT_FLASH))
            {
                continue;
            }
            if (!(contact_flags & FIELD_CONTACT_IGNORE_BINDING))
            {
                if (target->object_index < FIELD_PLAYER_COUNT)
                {
                    binding_offset = target->object_index * sizeof(FieldSequenceBinding);
                }
                else
                {
                    binding_offset = FIELD_PLAYER_COUNT * sizeof(FieldSequenceBinding);
                }
                object_index = target->object_index;
                binding_owner = ((FieldSequenceBinding*)((u8*)bindings + binding_offset))->owner_object_index;
                if (binding_owner == object_index)
                {
                    if ((u32)(binding_owner & 0xFF) < FIELD_PLAYER_COUNT)
                    {
                        binding_offset = binding_owner * sizeof(FieldSequenceBinding);
                    }
                    else
                    {
                        binding_offset = FIELD_PLAYER_COUNT * sizeof(FieldSequenceBinding);
                    }
                    if (((FieldSequenceBinding*)((u8*)bindings + binding_offset))->state != 0)
                    {
                        continue;
                    }
                }
            }
            if (target_state->object_flags &
                FIELD_OBJECT_UNTARGETABLE_FLAGS)
            {
                continue;
            }
            track_count = actor->track_count;
            track_index = 0;
            while (track_index < (s32)track_count && target_index != actor->track_object_indices[track_index])
            {
                track_index += 1;
            }
            if (track_index != actor->track_count)
            {
                continue;
            }
            sphere = spheres;
            for (sphere_index = 0; sphere_index < sphere_count; sphere_index++, sphere++)
            {
                delta->vx = (target->x - sphere->vx) >> 8;
                delta->vy = (target->y - sphere->vy) >> 8;
                delta->vz = (target->z - sphere->vz) >> 8;
                gte_ldlvl(delta);
                gte_sqr0();
                gte_stlvnl(squares);
                if ((SquareRoot0(squares->vx + squares->vy + squares->vz) <
                     (attack_radius + ((s16)target_state->collision.half.diameter >> 1))) &&
                    ((u8)g_field_object_states[actor->owner_object_index].contact.bytes.target_count < FIELD_MAX_CONTACT_TARGETS))
                {
                    target_state->contact.flags |= FIELD_CONTACT_TARGETED;
                    target_state->object_flags &= ~FIELD_OBJECT_FLAG_CLEAR_ON_HIT;
                    g_field_object_states[actor->owner_object_index].targets[g_field_object_states[actor->owner_object_index].contact.bytes.target_count] = target_index;
                    g_field_object_states[actor->owner_object_index].contact.bytes.target_count++;
                    actor->active_track_mask |= 1 << actor->track_count;
                    actor->track_object_indices[actor->track_count] = target_index;
                    actor->track_count++;
                    if ((target_index < FIELD_PLAYER_COUNT) && !(actors[target_index].control.half[0] & FIELD_CONTROL_MODE_MASK))
                    {
                        field_command_history_clear(target_index);
                    }
                    field_release_object_link(&g_field_actors[target_index]);
                    if (((u8)actor->hit_reaction < FIELD_HIT_REACTION_DIRECT_COUNT) || (g_field_actors[actor->owner_object_index].command == FIELD_ACTOR_COMMAND_WAIT_BOUND_ACTOR))
                    {
                        field_resolve_object_hit(actor->owner_object_index, target_index, actor->hit_reaction);
                    }
                    else
                    {
                        switch (actor->hit_reaction)
                        {
                        case 0x50:
                            field_resolve_object_hit(actor->owner_object_index, target_index, 0x12U);
                            break;
                        case 0x51:
                            field_resolve_object_hit(actor->owner_object_index, target_index, 0x13U);
                            break;
                        case 0x4E:
                            field_resolve_object_hit(actor->owner_object_index, target_index, 0x14U);
                            break;
                        case 0x4F:
                            field_resolve_object_hit(actor->owner_object_index, target_index, 0x15U);
                            break;
                        case 0x3E:
                            field_resolve_object_hit(actor->owner_object_index, target_index, 0x19U);
                            break;
                        case 0x45:
                            field_resolve_object_hit(actor->owner_object_index, target_index, 0x1AU);
                            break;
                        default:
                            field_resolve_contact_hit(actor->owner_object_index, target_index);
                            break;
                        }
                    }
                }
            }
        }
    }
}
/**
 * @brief Distance in whole units between two fixed-point positions.
 * @param a First position.
 * @param b Second position.
 * @return The distance, from the GTE squares of the whole-unit deltas.
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
