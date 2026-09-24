/**
 * @file field_actor_effects.c
 * @brief Object movement modes and the ground effects drawn around field objects.
 *
 * func_8009D4D8 selects an object's movement mode and seeds its per-mode
 * state. func_8009D9E0 draws one of seven ground effects under an object and
 * advances its radius and angle; the four packet builders that follow emit
 * the Gouraud-shaded translucent quads of those effects. The effect angle of
 * the object being drawn is passed to the builders through D_801178D8,
 * which func_8009E66C reads as a u16 and the others as an s32 (hence the
 * block-scope declarations).
 */

#include "common.h"
#include "controller_internal.h"
#include "field_effect_dispatch.h"
#include "field_effect_render_state.h"
#include "field_effect_types.h"
#include "main.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "sdk/rand.h"

/** @brief Published sample of the controller port at byte offset @p offset. */
#define PORT_SAMPLE(offset) (&((ControllerPortState*)((offset) + ports))->published_sample)

/** @brief Scratchpad words holding the GTE distance input vector. */
#define DISTANCE_DELTA ((s32*)0x1F800000)

/** @brief Scratchpad words receiving the squared distance components. */
#define DISTANCE_SQUARES ((s32*)0x1F800010)

/**
 * @brief Object state whose byte offset from the table start is @p offset.
 * @note The movement-mode points are addressed as byte sums (slot stride plus
 *       point offset); indexing the typed arrays swaps two registers.
 */
#define OBJECT_STATE_AT(offset) ((FieldObjectRuntime*)((offset) + (s32)base))

/**
 * @brief Set an object's movement mode and initialize its per-mode state.
 * @param actor Actor whose object state is changed.
 * @param mode Movement mode 0-6; mode 4 generates three separated random points.
 * @note Mode 4 accepts a new random point only when it is at least 0x40 units
 *       from every earlier point, using the GTE square and SquareRoot0.
 * @see decomp.me (100%)
 */
void func_8009D4D8(FieldMotionRecord* actor, u32 mode)
{
    s32* delta = DISTANCE_DELTA;
    s32* squares = DISTANCE_SQUARES;
    s32 compare_offset;
    s32 point_offset;
    s32 slot_stride;
    s32 slot_stride_y;
    s32 view_x;
    s32 view_z;
    s32 compare_index;
    s32 point_index;
    s32 needs_retry;
    s32 compare_addr;
    u8 object_index;
    FieldObjectRuntime* point;
    FieldObjectRuntime* point_y;
    FieldObjectRuntime* table;
    FieldObjectRuntime* state;

    g_field_object_states[actor->source_object_index].effect_angle = 0;
    switch (mode)
    {
    case 1:
        g_field_object_states[actor->source_object_index].movement.word = (g_field_object_states[actor->source_object_index].movement.word & ~0x3FF) | 0x40;
        return;
    case 2:
        g_field_object_states[actor->source_object_index].movement.word = (g_field_object_states[actor->source_object_index].movement.word & ~0x3FF) | 0x10;
        return;
    case 0:
    case 3:
        g_field_object_states[actor->source_object_index].movement.word = (g_field_object_states[actor->source_object_index].movement.word & ~0x3FF) | 0x1E;
        return;
    case 4:
    {
        FieldObjectRuntime* base;

        point_index = 0;
        table = g_field_object_states;
        base = table;
        state = &base[actor->source_object_index];
        state->movement.word = (state->movement.word & ~0x3FF) | 0x1E;
        do
        {
            needs_retry = 1;
            point_offset = point_index * 4;
            do
            {
                {
                    s32 random_value;
                    s32 offset;
                    FieldObjectRuntime* point;

                    random_value = (rand() >> 7) - 0x80;
                    offset = point_offset + (actor->source_object_index * 0x23C);
                    point = OBJECT_STATE_AT(offset);
                    point->ground_attachment_points[0].x = (s16)random_value;
                }
                {
                    s32 random_value;
                    s32 offset;
                    FieldObjectRuntime* point;

                    random_value = (rand() >> 7) - 0x80;
                    offset = point_offset + (actor->source_object_index * 0x23C);
                    point = OBJECT_STATE_AT(offset);
                    point->ground_attachment_points[0].y = (s16)random_value;
                }
                view_x = g_field_view_offset_x;
                {
                    s32 offset;

                    offset = point_offset + (actor->source_object_index * 0x23C);
                    point = OBJECT_STATE_AT(offset);
                }
                point->ground_attachment_points[0].x = (u16)(((u16)point->ground_attachment_points[0].x - (view_x / 256)) - (actor->x / 256));
                view_z = g_field_view_offset_z;
                {
                    s32 offset;

                    offset = point_offset + (actor->source_object_index * 0x23C);
                    point_y = OBJECT_STATE_AT(offset);
                }
                point_y->ground_attachment_points[0].y = (u16)(((u16)point_y->ground_attachment_points[0].y - (view_z / 256)) - (actor->z / 256));
                for (compare_index = 0; compare_index < point_index; compare_index++)
                {
                    /* Re-read in its own block; without it 30 rows change. */
                    do
                    {
                        object_index = actor->source_object_index;
                    } while (0);
                    compare_offset = compare_index * 4;
                    slot_stride = object_index * 0x23C;
                    compare_addr = compare_offset + slot_stride;
                    compare_addr += (s32)base;
                    {
                        s32 current_offset;

                        current_offset = point_offset + (object_index * 0x23C);
                        current_offset += (s32)base;
                        delta[0] = ((FieldObjectRuntime*)compare_addr)->ground_attachment_points[0].x -
                                   ((FieldObjectRuntime*)current_offset)->ground_attachment_points[0].x;
                    }
                    slot_stride_y = actor->source_object_index * 0x23C;
                    compare_offset += slot_stride_y;
                    compare_offset += (s32)base;
                    {
                        s32 current_offset;

                        current_offset = point_offset + (actor->source_object_index * 0x23C);
                        current_offset += (s32)base;
                        delta[1] = ((FieldObjectRuntime*)compare_offset)->ground_attachment_points[0].y -
                                   ((FieldObjectRuntime*)current_offset)->ground_attachment_points[0].y;
                    }
                    delta[2] = 0;
                    gte_ldlvl(delta);
                    gte_sqr0();
                    gte_stlvnl(squares);
                    if (SquareRoot0(squares[0] + squares[1]) < 0x40)
                    {
                        break;
                    }
                }
                if (compare_index == point_index)
                {
                    needs_retry = 0;
                }
            } while (needs_retry != 0);
            point_index += 1;
        } while (point_index < 3);
        return;
    }
    case 5:
    {
        FieldObjectRuntime* clear_base = g_field_object_states;
        s32 clear_flags = clear_base[actor->source_object_index].movement.word;
        s32 clear_mask = ~0x3FF;

        clear_flags &= clear_mask;
        table = &clear_base[actor->source_object_index];
        table->movement.word = clear_flags;
        clear_base[actor->source_object_index].ground_attachment_points[0].x = 0;
        clear_base[actor->source_object_index].ground_attachment_points[0].y = 0;
        return;
    }
    case 6:
    {
        FieldObjectRuntime* clear_base = g_field_object_states;
        FieldObjectRuntime* clear_slot = &clear_base[actor->source_object_index];
        s32 clear_flags = clear_slot->movement.word;
        s32 clear_mask = ~0x3FF;

        clear_flags &= clear_mask;
        clear_flags |= 0x1E;
        clear_slot->movement.word = clear_flags;
        clear_base[actor->source_object_index].ground_attachment_points[0].x = 0;
        clear_base[actor->source_object_index].ground_attachment_points[0].y = 0;
        return;
    }
    }
}

#undef OBJECT_STATE_AT

/**
 * @brief Look up the radius range of a ground effect kind.
 * @param kind Effect kind (0-6); other values leave both outputs untouched.
 * @param min_radius Receives the radius at which the effect intensity is zero.
 * @param max_radius Receives the radius at which the effect stops growing.
 * @note Declared inline: func_8009D9E0 expands it in place.
 * @see decomp.me (100%) TODO
 */
inline void func_8009D95C(s32 kind, s32* min_radius, s32* max_radius)
{
    switch (kind)
    {
    case 0:
        *min_radius = 0x1E;
        *max_radius = 0x60;
        break;
    case 1:
        *min_radius = 0x40;
        *max_radius = 0x80;
        break;
    case 2:
        *min_radius = 0x10;
        *max_radius = 0x40;
        break;
    case 3:
        *min_radius = 0x1E;
        *max_radius = 0xC8;
        break;
    case 4:
        *min_radius = 0x1E;
        *max_radius = 0x40;
        break;
    case 5:
        *min_radius = 0;
        *max_radius = 0x64;
        break;
    case 6:
        *min_radius = 0x1E;
        *max_radius = 0x40;
        break;
    }
}

/** @brief Runtime state of @p actor's object. */
#define OBJECT_STATE(actor) g_field_object_states[(actor)->source_object_index]

/** @brief Current effect radius of @p actor's object (low ten movement bits). */
#define EFFECT_RADIUS(actor) (OBJECT_STATE(actor).movement.half.flags & FIELD_OBJECT_EFFECT_SCALE_MASK)

u8* func_8009E66C(s32* ordering_table, u8* packet, VECTOR* position, s32 radius);
u8* func_8009FE54(s32* ordering_table, u8* packet, VECTOR* position, s32 radius);
u8* func_800A0B0C(s32* ordering_table, u8* packet, VECTOR* position, s32 extent, s32 forward);
u8* func_800A1344(s32* ordering_table, u8* packet, VECTOR* position, s32 slope, s32 facing);
void func_8001CDAC(s32*, s32*);
extern FieldRenderContext* D_800F2288;

/**
 * @brief Draw and advance an object's ground effect, including controller-driven offsets.
 * @param actor Actor supplying the position, facing flag and object index.
 * @param kind Effect type, from 0 through 6.
 */
void func_8009D9E0(FieldMotionRecord* actor, u32 kind)
{
    extern s32 D_801178D8;
    VECTOR vec[3];
    s16 screen[4];
    VECTOR* position;
    u8* ports = (u8*)CONTROLLER_STATE->ports;
    s32 ratio;
    s32 facing;
    s32 limits[2];
    s32 radius;
    s32 buttons;
    s32 raw_buttons;
    s32 port_offset;
    s32 stick_offset;
    u16 held;
    s32 i;
    s32 draw;
    u8* packet;
    s32* ordering_table;

    ordering_table = (s32*)&D_800F2288->ordering_table;
    packet = (u8*)D_800F2288->packet_cursor;
    func_8009D95C(kind, &limits[0], &limits[1]);
    radius = EFFECT_RADIUS(actor);
    ratio = ((radius - limits[0]) << 8) / (limits[1] - limits[0]);
    facing = actor->facing_or_reward_kind & 0x80;
    OBJECT_STATE(actor).effect_intensity = ratio;
    position = (VECTOR*)&actor->x;
    if (OBJECT_STATE(actor).effect_intensity >= 0x100)
    {
        OBJECT_STATE(actor).effect_intensity = 0xFF;
    }
    draw = 1;
    if (actor->source_object_index == 2)
    {
        if ((g_pad_ctx->unkAA8 & 0x7F) == 4)
        {
            draw = 0;
        }
    }
    D_801178D8 = OBJECT_STATE(actor).effect_angle;
    switch (kind)
    {
    case 0:
        if (draw != 0)
        {
            packet = func_8009E66C(ordering_table, packet, position, radius);
        }
        OBJECT_STATE(actor).effect_angle -= 0x80;
        if (EFFECT_RADIUS(actor) < limits[1])
        {
            OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | ((EFFECT_RADIUS(actor) + 2) & 0x3FF);
        }
        break;
    default:
        break;
    case 1:
        if (draw != 0)
        {
            packet = func_8009FE54(ordering_table, packet, position, radius);
        }
        OBJECT_STATE(actor).effect_angle -= 0x80;
        if (EFFECT_RADIUS(actor) < limits[1])
        {
            OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | ((EFFECT_RADIUS(actor) + 2) & 0x3FF);
        }
        break;
    case 2:
        if (draw != 0)
        {
            packet = func_800A1344(ordering_table, packet, position, radius, facing);
        }
        OBJECT_STATE(actor).effect_angle += 4;
        if (OBJECT_STATE(actor).effect_angle >= 0x50)
        {
            OBJECT_STATE(actor).effect_angle = 0;
        }
        if (EFFECT_RADIUS(actor) < limits[1])
        {
            OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | ((EFFECT_RADIUS(actor) + 1) & 0x3FF);
        }
        break;
    case 3:
        if (OBJECT_STATE(actor).effect_angle < 0)
        {
            OBJECT_STATE(actor).effect_angle = 0;
        }
        if (draw != 0)
        {
            packet = func_800A0B0C(ordering_table, func_800A0B0C(ordering_table, packet, position, radius, 0), position, radius, 1);
        }
        OBJECT_STATE(actor).effect_angle += 4;
        if (OBJECT_STATE(actor).effect_angle >= 0x50)
        {
            OBJECT_STATE(actor).effect_angle = 0;
        }
        if (EFFECT_RADIUS(actor) < limits[1])
        {
            OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | ((EFFECT_RADIUS(actor) + 2) & 0x3FF);
        }
        break;
    case 4:
        for (i = 0; i < 3; i++)
        {
            vec[0].vx = actor->x + (OBJECT_STATE(actor).ground_attachment_points[i].x << 8);
            vec[0].vy = actor->y;
            vec[0].vz = actor->z + (OBJECT_STATE(actor).ground_attachment_points[i].y << 8);
            if (draw != 0)
            {
                packet = func_8009E66C(ordering_table, packet, &vec[0], radius);
            }
        }
        if (EFFECT_RADIUS(actor) < limits[1])
        {
            OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | ((EFFECT_RADIUS(actor) + 2) & 0x3FF);
        }
        OBJECT_STATE(actor).effect_angle -= 0x80;
        break;
    case 5:
        vec[0].vx = actor->x + (OBJECT_STATE(actor).ground_attachment_points[0].x << 8);
        vec[0].vy = actor->y;
        vec[0].vz = actor->z + (OBJECT_STATE(actor).ground_attachment_points[0].y << 8);
        if (draw != 0)
        {
            packet = func_8009E66C(ordering_table, packet, &vec[0], radius);
        }
        OBJECT_STATE(actor).effect_angle -= 0x80;
        if (EFFECT_RADIUS(actor) < limits[1])
        {
            OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | ((EFFECT_RADIUS(actor) + 2) & 0x3FF);
            if (actor->facing_or_reward_kind & 0x80)
            {
                OBJECT_STATE(actor).ground_attachment_points[0].x += 2;
            }
            else
            {
                OBJECT_STATE(actor).ground_attachment_points[0].x -= 2;
            }
        }
        break;
    case 6:
        vec[0].vx = actor->x + (OBJECT_STATE(actor).ground_attachment_points[0].x << 8);
        vec[0].vy = actor->y;
        vec[0].vz = actor->z + (OBJECT_STATE(actor).ground_attachment_points[0].y << 8);
        if (draw != 0)
        {
            packet = func_8009E66C(ordering_table, packet, &vec[0], radius);
        }
        OBJECT_STATE(actor).effect_angle -= 0x80;
        if (EFFECT_RADIUS(actor) < limits[1])
        {
            OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | ((EFFECT_RADIUS(actor) + 1) & 0x3FF);
        }
        if (actor->source_object_index < 2)
        {
            port_offset = actor->source_object_index * sizeof(ControllerPortState);
            if (PORT_SAMPLE(port_offset)->device_type >= 0xFE)
            {
                raw_buttons = 0;
            }
            else
            {
                held = PORT_SAMPLE(port_offset)->held_buttons;
                raw_buttons = (held << 8) | (held >> 8);
            }
            buttons = ((u32)(raw_buttons & 0x40) >> 1) | ((raw_buttons & 0x20) * 2) | ((u32)(raw_buttons & 0x80) >> 3) | ((raw_buttons & 0x10) * 8) |
                      (raw_buttons & 0xFF0F);
            vec[0].vz = 0;
            vec[0].vy = 0;
            vec[0].vx = 0;
            if (buttons & 0x2000)
            {
                vec[0].vx = 0x1000;
            }
            if (buttons & 0x8000)
            {
                vec[0].vx -= 0x1000;
            }
            if (buttons & 0x4000)
            {
                vec[0].vy = -0x1000;
            }
            if (buttons & 0x1000)
            {
                vec[0].vy += 0x1000;
            }
            stick_offset = actor->source_object_index * sizeof(ControllerPortState);
            if (PORT_SAMPLE(stick_offset)->device_type != 0)
            {
                vec[0].vx += PORT_SAMPLE(stick_offset)->left_stick_x * 0x10;
                stick_offset = actor->source_object_index * sizeof(ControllerPortState);
                vec[0].vy -= PORT_SAMPLE(stick_offset)->left_stick_y * 0x10;
            }
            if ((vec[0].vx | vec[0].vy) != 0)
            {
                func_8001CDAC(&vec[0].vx, &vec[1].vx);
                vec[2].vx = actor->x + ((OBJECT_STATE(actor).ground_attachment_points[0].x + (vec[1].vx >> 10)) << 8);
                vec[2].vy = actor->y;
                vec[2].vz = actor->z + ((OBJECT_STATE(actor).ground_attachment_points[0].y + (vec[1].vy >> 10)) << 8);
                screen[0] = 0xA0 + g_field_view_offset_x / 256 + vec[2].vx / 256;
                screen[1] = 0x70 + g_field_view_offset_y / 256 + vec[2].vy / 256 - vec[2].vz / 512 - g_field_view_offset_z / 512;
                if ((screen[0] > 0 || vec[1].vx > 0) && (screen[1] > 0 || vec[1].vy < 0) && (screen[0] < 320 || vec[1].vx < 0) &&
                    (screen[1] < 224 || vec[1].vy > 0))
                {
                    OBJECT_STATE(actor).ground_attachment_points[0].x += vec[1].vx >> 10;
                    OBJECT_STATE(actor).ground_attachment_points[0].y += vec[1].vy >> 10;
                }
            }
        }
        break;
    }
    D_800F2288->packet_cursor = (s32*)packet;
}

/** @brief Project world point @p _tmp into vertex @p _vert of quad @p _poly. */
#define PROJECT_POINT(_poly, _vert, _tmp)                                                                                                                      \
    (_poly)->x##_vert = (s16)(0xA0 + g_field_view_offset_x / 0x100 + (_tmp).vx / 0x100);                                                                       \
    (_poly)->y##_vert = (s16)(0x70 + g_field_view_offset_y / 0x100 + (_tmp).vy / 0x100 - (_tmp).vz / 0x200 - g_field_view_offset_z / 0x200)

/** @brief Gouraud quad at byte offset @p _off from the packet cursor. */
#define POLY_AT(_off) ((POLY_G4*)(cursor + (_off)))

/**
 * @brief Link the packet at the cursor into the ordering table by depth and advance the cursor.
 * @note @p _depth is tested for the clamp; @p _expr is the unclamped bucket index.
 */
#define ADD_DEPTH_ADVANCE(_depth, _expr, _type)                                                                                                                \
    if ((_depth) < 0)                                                                                                                                          \
    {                                                                                                                                                          \
        addPrim(&ordering_table[0], (_type*)cursor);                                                                                                           \
        cursor += sizeof(_type);                                                                                                                               \
    }                                                                                                                                                          \
    else if ((_depth) >= 0x1000)                                                                                                                               \
    {                                                                                                                                                          \
        addPrim(&ordering_table[0xFFF], (_type*)cursor);                                                                                                       \
        cursor += sizeof(_type);                                                                                                                               \
    }                                                                                                                                                          \
    else                                                                                                                                                       \
    {                                                                                                                                                          \
        addPrim(&ordering_table[(_expr)], (_type*)cursor);                                                                                                     \
        cursor += sizeof(_type);                                                                                                                               \
    }

/**
 * @brief Append a translucent shaded dome of quads around a position.
 * @param ordering_table Ordering table with 4096 depth buckets.
 * @param packet Next free primitive-buffer byte.
 * @param position World-space center in fixed-point coordinates.
 * @param radius Dome radius.
 * @return First free byte after the appended primitives.
 * @note The dome is rotated by the effect angle in D_801178D8.
 */
u8* func_8009E66C(s32* ordering_table, u8* packet, VECTOR* position, s32 radius)
{
    extern u16 D_801178D8;
    VECTOR v[6];
    SVECTOR rot;
    MATRIX m0;
    MATRIX m1;
    s32 i;
    s32 x;
    s32 y;
    s32 initial_x;
    s32 initial_y;
    s32 depth;
    u32 center;
    u8* cursor;

    cursor = packet;

    rot.vx = 0;
    rot.vz = 0;
    rot.vy = D_801178D8;
    RotMatrix_gte(&rot, &m0);

    initial_x = (rcos(0) >> 4) * radius;
    initial_y = (rsin(0) >> 4) * radius;

    rot.vx = 0;
    rot.vz = 0;
    rot.vy = D_801178D8 + 0x180;
    RotMatrix_gte(&rot, &m1);

    v[0].vx = initial_x;
    v[0].vy = initial_y;
    v[0].vz = 0;
    ApplyMatrixLV(&m0, &v[0], &v[1]);
    v[0].vx = initial_x;
    v[0].vy = initial_y;
    v[0].vz = 0;
    ApplyMatrixLV(&m1, &v[0], &v[4]);

    v[0].vx = position->vx + v[1].vx;
    v[0].vy = position->vy + v[1].vy;
    v[0].vz = position->vz + v[1].vz;
    PROJECT_POINT(POLY_AT(0x0), 0, v[0]);

    v[0].vx = position->vx - v[1].vx;
    v[0].vy = position->vy + v[1].vy;
    v[0].vz = position->vz - v[1].vz;
    PROJECT_POINT(POLY_AT(0x24), 0, v[0]);

    v[0].vx = position->vx - v[1].vz;
    v[0].vy = position->vy + v[1].vy;
    v[0].vz = position->vz + v[1].vx;
    PROJECT_POINT(POLY_AT(0x48), 0, v[0]);

    v[0].vx = position->vx + v[1].vz;
    v[0].vy = position->vy + v[1].vy;
    v[0].vz = position->vz - v[1].vx;
    PROJECT_POINT(POLY_AT(0x6C), 0, v[0]);

    v[0].vx = position->vx;
    v[0].vy = position->vy;
    v[0].vz = position->vz;
    PROJECT_POINT(POLY_AT(0x0), 1, v[0]);
    center = *(u32*)&POLY_AT(0)->x1;
    *(u32*)&POLY_AT(0)->x3 = center;
    *(u32*)&POLY_AT(0x6C)->x1 = center;
    *(u32*)&POLY_AT(0x6C)->x3 = center;
    *(u32*)&POLY_AT(0x48)->x1 = center;
    *(u32*)&POLY_AT(0x48)->x3 = center;
    *(u32*)&POLY_AT(0x24)->x1 = center;
    *(u32*)&POLY_AT(0x24)->x3 = center;

    v[0].vx = position->vx + v[4].vx;
    v[0].vy = position->vy + v[4].vy;
    v[0].vz = position->vz + v[4].vz;
    PROJECT_POINT(POLY_AT(0x0), 2, v[0]);

    v[0].vx = position->vx - v[4].vx;
    v[0].vy = position->vy + v[4].vy;
    v[0].vz = position->vz - v[4].vz;
    PROJECT_POINT(POLY_AT(0x24), 2, v[0]);

    v[0].vx = position->vx - v[4].vz;
    v[0].vy = position->vy + v[4].vy;
    v[0].vz = position->vz + v[4].vx;
    PROJECT_POINT(POLY_AT(0x48), 2, v[0]);

    v[0].vx = position->vx + v[4].vz;
    v[0].vy = position->vy + v[4].vy;
    v[0].vz = position->vz - v[4].vx;
    PROJECT_POINT(POLY_AT(0x6C), 2, v[0]);

    *(u32*)&POLY_AT(0x0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
    *(u32*)&POLY_AT(0x24)->r0 = (((v[1].vz >> 8) + 0xA0) << 8) & 0xFF00;
    *(u32*)&POLY_AT(0x48)->r0 = ((0xA0 - (v[1].vx >> 8)) << 8) & 0xFF00;
    *(u32*)&POLY_AT(0x6C)->r0 = (((v[1].vx >> 8) + 0xA0) << 8) & 0xFF00;

    *(u32*)&POLY_AT(0x6C)->r1 = 0xA000;
    *(u32*)&POLY_AT(0x48)->r1 = 0xA000;
    *(u32*)&POLY_AT(0x24)->r1 = 0xA000;
    *(u32*)&POLY_AT(0x0)->r1 = 0xA000;
    *(u32*)&POLY_AT(0x24)->r3 = 0;
    *(u32*)&POLY_AT(0x24)->r2 = 0;
    *(u32*)&POLY_AT(0x48)->r3 = 0;
    *(u32*)&POLY_AT(0x48)->r2 = 0;
    *(u32*)&POLY_AT(0x6C)->r3 = 0;
    *(u32*)&POLY_AT(0x6C)->r2 = 0;
    *(u32*)&POLY_AT(0x0)->r3 = 0;
    *(u32*)&POLY_AT(0x0)->r2 = 0;

    SetPolyG4(POLY_AT(0x0));
    SetPolyG4(POLY_AT(0x24));
    SetPolyG4(POLY_AT(0x48));
    SetPolyG4(POLY_AT(0x6C));
    setSemiTrans(POLY_AT(0x0), 1);
    setSemiTrans(POLY_AT(0x24), 1);
    setSemiTrans(POLY_AT(0x48), 1);
    setSemiTrans(POLY_AT(0x6C), 1);

    depth = position->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, position->vz >> 7, POLY_G4);
    depth = position->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, position->vz >> 7, POLY_G4);
    depth = position->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, position->vz >> 7, POLY_G4);
    depth = position->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, position->vz >> 7, POLY_G4);

    setDrawTPage((DR_TPAGE*)cursor, 0, 0, 0x25);
    depth = (position->vz + v[1].vz) >> 7;
    ADD_DEPTH_ADVANCE(depth, (position->vz + v[1].vz) >> 7, DR_TPAGE);

    i = 1;
    do
    {
        x = (rcos(i << 8) >> 4) * radius;
        y = -(rsin(i << 8) >> 4) * radius;
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[2]);
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m1, &v[0], &v[5]);

        v[0].vx = position->vx + v[1].vx;
        v[0].vy = position->vy + v[1].vy;
        v[0].vz = position->vz + v[1].vz;
        PROJECT_POINT(POLY_AT(0), 0, v[0]);
        v[0].vx = position->vx + v[2].vx;
        v[0].vy = position->vy + v[2].vy;
        v[0].vz = position->vz + v[2].vz;
        PROJECT_POINT(POLY_AT(0), 1, v[0]);
        v[0].vx = position->vx + v[4].vx;
        v[0].vy = position->vy + v[4].vy;
        v[0].vz = position->vz + v[4].vz;
        PROJECT_POINT(POLY_AT(0), 2, v[0]);
        v[0].vx = position->vx + v[5].vx;
        v[0].vy = position->vy + v[5].vy;
        v[0].vz = position->vz + v[5].vz;
        PROJECT_POINT(POLY_AT(0), 3, v[0]);

        *(u32*)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32*)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32*)&POLY_AT(0)->r2 = 0;
        *(u32*)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (position->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (position->vz + v[1].vz) >> 7, POLY_G4);

        setDrawTPage((DR_TPAGE*)cursor, 0, 0, 0x25);
        depth = (position->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (position->vz + v[1].vz) >> 7, DR_TPAGE);

        v[0].vx = position->vx - v[1].vz;
        v[0].vy = position->vy + v[1].vy;
        v[0].vz = position->vz + v[1].vx;
        PROJECT_POINT(POLY_AT(0), 0, v[0]);
        v[0].vx = position->vx - v[2].vz;
        v[0].vy = position->vy + v[2].vy;
        v[0].vz = position->vz + v[2].vx;
        PROJECT_POINT(POLY_AT(0), 1, v[0]);
        v[0].vx = position->vx - v[4].vz;
        v[0].vy = position->vy + v[4].vy;
        v[0].vz = position->vz + v[4].vx;
        PROJECT_POINT(POLY_AT(0), 2, v[0]);
        v[0].vx = position->vx - v[5].vz;
        v[0].vy = position->vy + v[5].vy;
        v[0].vz = position->vz + v[5].vx;
        PROJECT_POINT(POLY_AT(0), 3, v[0]);

        *(u32*)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32*)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32*)&POLY_AT(0)->r2 = 0;
        *(u32*)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (position->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (position->vz + v[1].vz) >> 7, POLY_G4);

        setDrawTPage((DR_TPAGE*)cursor, 0, 0, 0x25);
        depth = (position->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (position->vz + v[1].vz) >> 7, DR_TPAGE);

        i++;
        v[1].vx = v[2].vx;
        v[1].vy = v[2].vy;
        v[1].vz = v[2].vz;
        v[4].vx = v[5].vx;
        v[4].vy = v[5].vy;
        v[4].vz = v[5].vz;
    } while (i < 9);

    return cursor;
}

/**
 * @brief Append four rotating strips of shaded, translucent quads.
 * @param ordering_table Ordering table with 4096 depth buckets.
 * @param packet Next free primitive-buffer byte.
 * @param position World-space center in fixed-point coordinates.
 * @param radius Radius used to construct the strips.
 * @return First free byte after the appended primitives.
 * @note unused_matrix and the unused v[] entries size the stack frame.
 */
u8* func_8009FE54(s32* ordering_table, u8* packet, VECTOR* position, s32 radius)
{
    extern s32 D_801178D8;
    VECTOR v[6];
    SVECTOR rot;
    MATRIX m0;
    MATRIX unused_matrix;
    VECTOR p0;
    VECTOR p1;
    s32 distance;
    s32 i;
    s32 j;
    s32 x;
    s32 y;
    s32 inner_x;
    s32 inner_y;
    s32 angle;
    s32 depth;
    u8* cursor;

    cursor = packet;
    i = 0;
    do
    {
        distance = (radius >> 1) + 0x40;
        p0.vx = position->vx;
        p0.vy = position->vy;
        p0.vz = position->vz;
        p1.vx = position->vx;
        p1.vy = position->vy;
        p1.vz = position->vz;
        angle = i << 10;
        x = (rcos(angle - D_801178D8) >> 4) * distance;
        y = (rsin(angle - D_801178D8) >> 4) * distance;
        p0.vx += x;
        p0.vz += y;
        x = (rcos(angle - D_801178D8 - 0x100) >> 4) * distance;
        y = (rsin(angle - D_801178D8 - 0x100) >> 4) * distance;
        p1.vx += x;
        p1.vz += y;
        rot.vx = 0;
        rot.vz = 0;
        rot.vy = D_801178D8 + angle;
        RotMatrix_gte(&rot, &m0);
        x = ((rcos(0) >> 4) * radius) >> 1;
        y = ((rsin(0) >> 4) * radius) >> 1;
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[1]);
        v[0].vx = -radius * 0x80;
        v[0].vy = 0;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[2]);
        v[0].vx = p0.vx + v[1].vx;
        v[0].vy = p0.vy + v[1].vy;
        v[0].vz = p0.vz + v[1].vz;
        PROJECT_POINT(POLY_AT(0), 0, v[0]);
        v[0].vx = p0.vx + v[2].vx;
        v[0].vy = p0.vy + v[2].vy;
        v[0].vz = p0.vz + v[2].vz;
        PROJECT_POINT(POLY_AT(0), 1, v[0]);
        v[0].vx = p1.vx + v[1].vx;
        v[0].vy = p1.vy + v[1].vy;
        v[0].vz = p1.vz + v[1].vz;
        PROJECT_POINT(POLY_AT(0), 2, v[0]);
        v[0].vx = p1.vx + v[2].vx;
        v[0].vy = p1.vy + v[2].vy;
        v[0].vz = p1.vz + v[2].vz;
        PROJECT_POINT(POLY_AT(0), 3, v[0]);
        *(u32*)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 8) & 0xFF00;
        *(u32*)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 8) & 0xFF00;
        *(u32*)&POLY_AT(0)->r2 = 0;
        *(u32*)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (p0.vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, POLY_G4);
        setDrawTPage((DR_TPAGE*)cursor, 0, 0, 0x25);
        depth = (p0.vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, DR_TPAGE);
        j = 1;
        do
        {
            inner_x = ((rcos(j << 8) >> 4) * radius) >> 1;
            inner_y = (-(rsin(j << 8) >> 4) * radius) >> 1;
            v[0].vx = inner_x;
            v[0].vy = inner_y;
            v[0].vz = 0;
            ApplyMatrixLV(&m0, &v[0], &v[2]);
            v[0].vx = p0.vx + v[1].vx;
            v[0].vy = p0.vy + v[1].vy;
            v[0].vz = p0.vz + v[1].vz;
            PROJECT_POINT(POLY_AT(0), 0, v[0]);
            v[0].vx = p0.vx + v[2].vx;
            v[0].vy = p0.vy + v[2].vy;
            v[0].vz = p0.vz + v[2].vz;
            PROJECT_POINT(POLY_AT(0), 1, v[0]);
            v[0].vx = p1.vx + v[1].vx;
            v[0].vy = p1.vy + v[1].vy;
            v[0].vz = p1.vz + v[1].vz;
            PROJECT_POINT(POLY_AT(0), 2, v[0]);
            v[0].vx = p1.vx + v[2].vx;
            v[0].vy = p1.vy + v[2].vy;
            v[0].vz = p1.vz + v[2].vz;
            PROJECT_POINT(POLY_AT(0), 3, v[0]);
            *(u32*)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
            *(u32*)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
            *(u32*)&POLY_AT(0)->r2 = 0;
            *(u32*)&POLY_AT(0)->r3 = 0;
            SetPolyG4(POLY_AT(0));
            setSemiTrans(POLY_AT(0), 1);
            depth = (p0.vz + v[1].vz) >> 7;
            ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, POLY_G4);
            setDrawTPage((DR_TPAGE*)cursor, 0, 0, 0x25);
            depth = (p0.vz + v[1].vz) >> 7;
            ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, DR_TPAGE);
            j++;
            v[1].vx = v[2].vx;
            v[1].vy = v[2].vy;
            v[1].vz = v[2].vz;
        } while (j < 9);
        i++;
    } while (i < 4);
    return cursor;
}

#undef PROJECT_POINT
#undef POLY_AT
#undef ADD_DEPTH_ADVANCE

/** @brief Stack workspace of the strip builders (func_800A0B0C uses only the world vector). */
typedef struct
{
    VECTOR rotated;
    VECTOR world;
    VECTOR unused;
    SVECTOR input;
    MATRIX matrices[5];
} FieldStripWorkspace;

/**
 * @brief Store the three components of @p v as one statement block.
 * @note The block (not a comma expression) keeps the stores ahead of the
 *       following projection in the schedule.
 */
#define SET_VECTOR(v, x, y, z)                                                                                                                                 \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        (v)->vx = (x);                                                                                                                                         \
        (v)->vy = (y);                                                                                                                                         \
        (v)->vz = (z);                                                                                                                                         \
    } while (0)

/**
 * @brief Link @p packet into ordering table bucket @p depth.
 * @note Open-coded addPrim through the caller's tag_mask/addr_mask locals;
 *       addPrim itself allocates the masks differently (97.4% in func_800A0B0C).
 */
#define LINK_PACKET(packet, depth)                                                                                                                             \
    (*(s32*)(packet) = (*(s32*)(packet) & tag_mask) | (ordering_table[depth] & addr_mask),                                                                     \
     ordering_table[depth] = (ordering_table[depth] & tag_mask) | ((s32)(packet) & addr_mask))

/**
 * @see decomp.me (100%)
 * @brief Draw animated curved quad strips extending from a fixed-point position.
 * @param ordering_table Ordering table with 0x1000 depth entries.
 * @param primitive_buffer Destination for the generated GPU packets.
 * @param position World position in signed fixed-point coordinates.
 * @param extent Maximum horizontal extent, tested after each completed strip.
 * @param forward Nonzero extends toward positive X; zero extends toward negative X.
 * @return First byte after the emitted primitives and draw-page command.
 * @note Emits at least one strip and at most four, with nine quads per strip.
 * @note Adjacent strips are separated by twice their 0x1400 fixed-point width.
 */
u8* func_800A0B0C(s32* ordering_table, u8* primitive_buffer, VECTOR* position, s32 extent, s32 forward)
{
    extern s32 D_801178D8;
    s32 trig_angle;
    s32 screen_x;
    s32 camera_y;
    s32 step;
    s32 strip_index;
    s32 first_xy;
    s32 second_xy;
    FieldStripWorkspace work;
    u8* primitive;
    s32 segment_depth;
    s32 closing_depth;
    s32 drawpage_depth;
    s32 angle;
    s32 offset;
    u8* strip_code;
    u8* outer_code;

    s32 addr_mask;
    s32 tag_mask;

    primitive = primitive_buffer;
    addr_mask = 0xFFFFFF;
    strip_index = 0;

    outer_code = primitive;
    offset = (D_801178D8 % 40) << 8;
    do
    {
        tag_mask = 0xFF000000;
        /* Save both initial packed XY values to close the strip after eight steps. */
        if (forward != 0)
        {
            work.world.vx = position->vx + offset;
        }
        else
        {
            work.world.vx = position->vx - offset;
        }
        do
        {
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        } while (0);
        screen_x = 160 + g_field_view_offset_x / 256 + work.world.vx / 256;
        camera_y = g_field_view_offset_y;
        *(s16*)(outer_code + 8) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(outer_code + 10) = 112 + (camera_y >> 8) + work.world.vy / 256 - work.world.vz / 512 - g_field_view_offset_z / 512;
        first_xy = *(s32*)(outer_code + 8);
        if (forward != 0)
        {
            work.world.vx = position->vx + offset + 0x1400;
        }
        else
        {
            work.world.vx = (position->vx - offset) - 0x1400;
        }
        do
        {
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        } while (0);
        screen_x = 160 + g_field_view_offset_x / 256 + work.world.vx / 256;
        camera_y = g_field_view_offset_y;
        *(s16*)(outer_code + 16) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(outer_code + 18) = 112 + (camera_y >> 8) + work.world.vy / 256 - work.world.vz / 512 - g_field_view_offset_z / 512;
        step = 1;
        angle = 0x100;
        strip_code = primitive;
        second_xy = *(s32*)(outer_code + 16);
        /* Every do/while(0) in this function is load-bearing (91.4% with all removed). */
        do
        {
            do
            {
                do
                {
                    do
                    {
                        trig_angle = angle;
                        if (forward != 0)
                        {
                            work.world.vx = position->vx + offset + angle;
                        }
                        else
                        {
                            work.world.vx = (position->vx - offset) - angle;
                        }
                        work.world.vy = position->vy - (rsin(trig_angle) * 2);
                        work.world.vz = position->vz + (rcos(angle) * 2);
                    } while (0);
                    screen_x = 160 + g_field_view_offset_x / 256 + work.world.vx / 256;
                    camera_y = g_field_view_offset_y;
                    *(s16*)(strip_code + 24) = screen_x;
                    if (camera_y < 0)
                    {
                        camera_y += 255;
                    }
                    *(s16*)(strip_code + 26) = 112 + (camera_y >> 8) + work.world.vy / 256 - work.world.vz / 512 - g_field_view_offset_z / 512;
                    do
                    {
                        *(s32*)(strip_code + 44) = *(s32*)(strip_code + 24);
                        do
                        {
                            trig_angle = angle;
                            if (forward != 0)
                            {
                                work.world.vx = position->vx + offset + angle + 0x1400;
                            }
                            else
                            {
                                work.world.vx = ((position->vx - offset) - angle) - 0x1400;
                            }
                            work.world.vy = position->vy - (rsin(trig_angle) * 2);
                            work.world.vz = position->vz + (rcos(angle) * 2);
                        } while (0);
                        screen_x = 160 + g_field_view_offset_x / 256 + work.world.vx / 256;
                        camera_y = g_field_view_offset_y;
                        *(s16*)(strip_code + 32) = screen_x;
                        if (camera_y < 0)
                        {
                            camera_y += 255;
                        }
                        *(s16*)(strip_code + 34) = 112 + (camera_y >> 8) + work.world.vy / 256 - work.world.vz / 512 - g_field_view_offset_z / 512;
                        /* Fade the curved strip from black to blue. */
                        *(s32*)(strip_code + 4) = 0;
                        *(s32*)(strip_code + 12) = 0xA00000;
                        *(s32*)(strip_code + 20) = 0;
                        *(s32*)(strip_code + 28) = 0xA00000;
                        *(s32*)(strip_code + 52) = *(s32*)(strip_code + 32);
                        SetPolyG4((POLY_G4*)primitive);
                        *(strip_code + 7) |= 2;
                        segment_depth = position->vz >> 7;
                        if (segment_depth < 0)
                        {
                            LINK_PACKET(primitive, 0);
                            primitive += 0x24;
                            strip_code += 0x24;
                            outer_code += 0x24;
                        }
                        else if (segment_depth >= 0x1000)
                        {
                            LINK_PACKET(primitive, 0xFFF);
                            primitive += 0x24;
                            strip_code += 0x24;
                            outer_code += 0x24;
                        }
                        else
                        {
                            LINK_PACKET(primitive, position->vz >> 7);
                            primitive += 0x24;
                            strip_code += 0x24;
                            outer_code += 0x24;
                        }
                        angle += 0x100;
                        step++;
                    } while (0);
                } while (0);
            } while (0);
        } while (step < 9);
        *(s32*)(outer_code + 24) = first_xy;
        *(s32*)(outer_code + 32) = second_xy;
        *(s32*)(outer_code + 4) = 0;
        *(s32*)(outer_code + 12) = 0xA000;
        *(s32*)(outer_code + 20) = 0;
        *(s32*)(outer_code + 28) = 0xA000;
        SetPolyG4((POLY_G4*)primitive);
        *(outer_code + 7) |= 2;
        do
        {
            closing_depth = position->vz >> 7;
            if (closing_depth < 0)
            {
                do
                {
                    LINK_PACKET(primitive, 0);
                    primitive += 0x24;
                    outer_code += 0x24;
                } while (0);
            }
            else if (closing_depth >= 0x1000)
            {
                do
                {
                    LINK_PACKET(primitive, 0xFFF);
                    primitive += 0x24;
                    outer_code += 0x24;
                } while (0);
            }
            else
            {
                do
                {
                    LINK_PACKET(primitive, position->vz >> 7);
                    primitive += 0x24;
                    outer_code += 0x24;
                } while (0);
            }
        } while (0);
        offset += 0x1400;
        offset += 0x1400;
    } while (offset < (extent << 8) && ++strip_index < 4);
    *(u8*)(primitive + 3) = 1;
    *(s32*)(primitive + 4) = 0xE1000025;
    drawpage_depth = position->vz >> 7;
    if (drawpage_depth < 0)
    {
        addPrim(&ordering_table[0], primitive);
        primitive += 8;
    }
    else if (drawpage_depth >= 0x1000)
    {
        addPrim(&ordering_table[0xFFF], primitive);
        primitive += 8;
    }
    else
    {
        addPrim(&ordering_table[position->vz >> 7], primitive);
        primitive += 8;
    }
    return primitive;
}

/**
 * @brief Draw four rotating strips of Gouraud-shaded quads around a position.
 * @param ordering_table Depth ordering table with 4096 entries.
 * @param packet First free primitive packet.
 * @param position Fixed-point world-space origin of the effect.
 * @param slope Direction parameter converted into a Y rotation angle.
 * @param facing Selects addition or subtraction of the rotated X offset.
 * @return First free packet following all quads and the draw-page command.
 * @note Full word copies carry packed X/Y pairs into the next segment and
 *       close the strip. The strip cursor points at the primitive code byte.
 * @see decomp.me (100%)
 */
u8* func_800A1344(s32* ordering_table, u8* packet, VECTOR* position, s32 slope, s32 facing)
{
    extern s32 D_801178D8;
    s32 screen_x;
    s32 packet_addr;
    FieldStripWorkspace work;
    s32 strip_index;
    s32 angle;
    s32 first_xy;
    s32 second_xy;
    MATRIX* matrix;
    u8* current_packet;
    s32 segment_depth;
    s32 closing_depth;
    s32 drawpage_depth;
    s32 camera_y;
    s32 addr_mask;
    s32 tag_mask;
    s32 segment_index;
    s32 radius;
    s32 radius_sum;
    s32 first_x;
    s32 segment_outer_x;
    s32 second_x;
    s32 segment_inner_x;
    u8* segment_packet;
    u8* strip_code;

    current_packet = packet;
    angle = ratan2(slope, 0x64);
    strip_index = 0;
    matrix = &work.matrices[2];
    addr_mask = 0xFFFFFF;
    tag_mask = 0xFF000000;
    radius = D_801178D8;
    strip_code = current_packet + 7;
    /* A do/while here lets loop.c split strip_code into two biased givs (95.9% at best). */
next_strip:
{
    /* Identity rotation with zero translation, written as words. */
    ((s32*)&work.matrices[2])[4] = 0x1000;
    ((s32*)&work.matrices[2])[2] = 0x1000;
    ((s32*)&work.matrices[2])[0] = 0x1000;
    ((s32*)&work.matrices[2])[7] = 0;
    ((s32*)&work.matrices[2])[6] = 0;
    ((s32*)&work.matrices[2])[5] = 0;
    ((s32*)&work.matrices[2])[3] = 0;
    ((s32*)&work.matrices[2])[1] = 0;
    RotMatrixY(angle, matrix);
    work.input.vx = (s16)radius;
    work.input.vy = 0;
    work.input.vz = 0;
    gte_SetRotMatrix(matrix);
    gte_ldv0(&work.input);
    gte_rtv0();
    gte_stlvnl(&work.rotated);
    if (facing != 0)
    {
        first_x = position->vx + (work.rotated.vx << 8);
    }
    else
    {
        first_x = position->vx - (work.rotated.vx << 8);
    }
    SET_VECTOR(&work.world, first_x, position->vy + (work.rotated.vy << 8), position->vz + (work.rotated.vz << 8));
    /* The X store follows the facing branch, so only a volatile read reloads it (98.3% plain). */
    screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
    camera_y = g_field_view_offset_y;
    *(s16*)(strip_code + 1) = screen_x;
    if (camera_y < 0)
    {
        camera_y += 255;
    }
    *(s16*)(strip_code + 3) = 112 + (camera_y >> 8) + work.world.vy / 256 - work.world.vz / 512 - g_field_view_offset_z / 512;
    first_xy = *(s32*)(strip_code + 1);
    work.input.vx = radius + 0x14;
    work.input.vy = 0;
    work.input.vz = 0;
    gte_SetRotMatrix(matrix);
    gte_ldv0(&work.input);
    gte_rtv0();
    gte_stlvnl(&work.rotated);
    if (facing != 0)
    {
        second_x = position->vx + (work.rotated.vx << 8);
    }
    else
    {
        second_x = position->vx - (work.rotated.vx << 8);
    }
    SET_VECTOR(&work.world, second_x, position->vy + (work.rotated.vy << 8), position->vz + (work.rotated.vz << 8));
    screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
    camera_y = g_field_view_offset_y;
    *(s16*)(strip_code + 9) = screen_x;
    if (camera_y < 0)
    {
        camera_y += 255;
    }
    *(s16*)(strip_code + 11) = 112 + (camera_y >> 8) + work.world.vy / 256 - work.world.vz / 512 - g_field_view_offset_z / 512;
    segment_index = 1;
    radius_sum = radius;
    segment_packet = current_packet;
    second_xy = *(s32*)(strip_code + 9);
    do
    {
        RotMatrixX(-0x100, matrix);
        work.input.vx = radius + (radius_sum >> 5);
        work.input.vy = 0;
        work.input.vz = 0;
        gte_SetRotMatrix(matrix);
        gte_ldv0(&work.input);
        gte_rtv0();
        gte_stlvnl(&work.rotated);
        if (facing != 0)
        {
            segment_inner_x = position->vx + (work.rotated.vx << 8);
        }
        else
        {
            segment_inner_x = position->vx - (work.rotated.vx << 8);
        }
        SET_VECTOR(&work.world, segment_inner_x, position->vy + (work.rotated.vy << 8), position->vz + (work.rotated.vz << 8));
        screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
        camera_y = g_field_view_offset_y;
        *(s16*)(segment_packet + 24) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(segment_packet + 26) = 112 + (camera_y >> 8) + work.world.vy / 256 - work.world.vz / 512 - g_field_view_offset_z / 512;
        *(s32*)(segment_packet + 44) = *(s32*)(segment_packet + 24);
        work.input.vx = radius + (radius_sum >> 5) + 0x14;
        work.input.vy = 0;
        work.input.vz = 0;
        gte_SetRotMatrix(matrix);
        gte_ldv0(&work.input);
        gte_rtv0();
        gte_stlvnl(&work.rotated);
        if (facing != 0)
        {
            segment_outer_x = position->vx + (work.rotated.vx << 8);
        }
        else
        {
            segment_outer_x = position->vx - (work.rotated.vx << 8);
        }
        SET_VECTOR(&work.world, segment_outer_x, position->vy + (work.rotated.vy << 8), position->vz + (work.rotated.vz << 8));
        screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
        camera_y = g_field_view_offset_y;
        *(s16*)(segment_packet + 32) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(segment_packet + 34) = 112 + (camera_y >> 8) + work.world.vy / 256 - work.world.vz / 512 - g_field_view_offset_z / 512;
        *(s32*)(segment_packet + 4) = 0;
        *(s32*)(segment_packet + 12) = 0xA00000;
        *(s32*)(segment_packet + 20) = 0;
        *(s32*)(segment_packet + 28) = 0xA00000;
        *(s32*)(segment_packet + 52) = *(s32*)(segment_packet + 32);
        SetPolyG4((POLY_G4*)current_packet);
        do
        {
            *(segment_packet + 7) |= 2;
            segment_depth = position->vz >> 7;
            if (segment_depth < 0)
            {
                LINK_PACKET(current_packet, 0);
                current_packet += 0x24;
                segment_packet += 0x24;
                strip_code += 0x24;
            }
            else if (segment_depth >= 0x1000)
            {
                LINK_PACKET(current_packet, 0xFFF);
                current_packet += 0x24;
                segment_packet += 0x24;
                strip_code += 0x24;
            }
            else
            {
                LINK_PACKET(current_packet, position->vz >> 7);
                current_packet += 0x24;
                segment_packet += 0x24;
                strip_code += 0x24;
            }
            segment_index += 1;
            radius_sum += radius;
        } while (0);
    } while (segment_index < 9);
    *(s32*)(strip_code + 17) = first_xy;
    *(s32*)(strip_code + 25) = second_xy;
    *(s32*)(strip_code + -3) = 0;
    *(s32*)(strip_code + 5) = 0xA000;
    *(s32*)(strip_code + 13) = 0;
    *(s32*)(strip_code + 21) = 0xA000;
    SetPolyG4((POLY_G4*)current_packet);
    *strip_code |= 2;
    closing_depth = position->vz >> 7;
    if (closing_depth < 0)
    {
        strip_code += 0x24;
        do
        {
            *(s32*)current_packet = (*(s32*)current_packet & tag_mask) | (ordering_table[0] & addr_mask);
            packet_addr = (s32)current_packet & addr_mask;
        } while (0);
        ordering_table[0] = (ordering_table[0] & tag_mask) | packet_addr;
        current_packet += 0x24;
    }
    else if (closing_depth >= 0x1000)
    {
        LINK_PACKET(current_packet, 0xFFF);
        current_packet += 0x24;
        strip_code += 0x24;
    }
    else
    {
        LINK_PACKET(current_packet, position->vz >> 7);
        current_packet += 0x24;
        strip_code += 0x24;
    }
    radius += 0x50;
    /* Every do/while(0) in this function is load-bearing (91.6% with all removed). */
    do
    {
        if (radius >= 0x140)
        {
            radius -= 0x140;
        }
    } while (0);
    strip_index++;
}
    if (strip_index < 4)
    {
        goto next_strip;
    }
    *(u8*)(current_packet + 3) = 1;
    *(s32*)(current_packet + 4) = 0xE1000025;
    drawpage_depth = position->vz >> 7;
    if (drawpage_depth < 0)
    {
        addPrim(&ordering_table[0], current_packet);
        current_packet += 8;
    }
    else if (drawpage_depth >= 0x1000)
    {
        addPrim(&ordering_table[0xFFF], current_packet);
        current_packet += 8;
    }
    else
    {
        addPrim(&ordering_table[position->vz >> 7], current_packet);
        current_packet += 8;
    }
    return current_packet;
}

#undef LINK_PACKET
#undef SET_VECTOR
