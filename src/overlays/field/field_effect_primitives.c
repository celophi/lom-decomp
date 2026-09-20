/** @file field_effect_primitives.c
 * @brief Build shaded rings, fans, trails, and line effects.
 */

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
 * @see decomp.me (99.95%) WIP
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
        /* Line effects reuse animation_active as their segment count. */
        segment_count = 1;
    }
    angle_step = (ONE / 2) / segment_count;

    field_resolve_effect_position(effect, part, &FIELD_RADIAL_SCRATCH->origin);

    segment = segment_count - 1;

    origin = &FIELD_RADIAL_SCRATCH->origin;
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
