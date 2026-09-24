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
#include "field_calls.h"
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

/** @brief Runtime state of @p actor's object. */
#define OBJECT_STATE(actor) g_field_object_states[(actor)->source_object_index]

/**
 * @brief Set an object's movement mode and initialize its per-mode state.
 * @param actor Actor whose object state is changed.
 * @param mode Movement mode 0-6; mode 4 generates three separated random points.
 * @note Mode 4 accepts a new random point only when it is at least 0x40 units
 *       from every earlier point, using the GTE square and SquareRoot0.
 * @note Modes 0 and 3 have separate, identical bodies; jump2 merges them into
 *       one jump-table target, but the extra copy is what makes the actor
 *       pointer outrank the table address in register allocation.
 */
void func_8009D4D8(FieldMotionRecord* actor, u32 mode)
{
    s32* delta = DISTANCE_DELTA;
    s32* squares = DISTANCE_SQUARES;
    s32 point_index;
    s32 compare_index;
    s32 needs_retry;

    OBJECT_STATE(actor).effect_angle = 0;
    switch (mode)
    {
    case 1:
        OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | 0x40;
        return;
    case 2:
        OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | 0x10;
        return;
    case 0:
        OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | 0x1E;
        return;
    case 3:
        OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | 0x1E;
        return;
    case 4:
        OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | 0x1E;
        for (point_index = 0; point_index < 3; point_index++)
        {
            needs_retry = 1;
            do
            {
                OBJECT_STATE(actor).ground_attachment_points[point_index].x = (rand() >> 7) - 0x80;
                OBJECT_STATE(actor).ground_attachment_points[point_index].y = (rand() >> 7) - 0x80;
                OBJECT_STATE(actor).ground_attachment_points[point_index].x =
                    OBJECT_STATE(actor).ground_attachment_points[point_index].x - g_field_view_offset_x / 256 - actor->x / 256;
                OBJECT_STATE(actor).ground_attachment_points[point_index].y =
                    OBJECT_STATE(actor).ground_attachment_points[point_index].y - g_field_view_offset_z / 256 - actor->z / 256;
                for (compare_index = 0; compare_index < point_index; compare_index++)
                {
                    delta[0] = OBJECT_STATE(actor).ground_attachment_points[compare_index].x -
                               OBJECT_STATE(actor).ground_attachment_points[point_index].x;
                    delta[1] = OBJECT_STATE(actor).ground_attachment_points[compare_index].y -
                               OBJECT_STATE(actor).ground_attachment_points[point_index].y;
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
        }
        return;
    case 5:
        OBJECT_STATE(actor).movement.word &= ~0x3FF;
        OBJECT_STATE(actor).ground_attachment_points[0].x = 0;
        OBJECT_STATE(actor).ground_attachment_points[0].y = 0;
        return;
    case 6:
        OBJECT_STATE(actor).movement.word = (OBJECT_STATE(actor).movement.word & ~0x3FF) | 0x1E;
        OBJECT_STATE(actor).ground_attachment_points[0].x = 0;
        OBJECT_STATE(actor).ground_attachment_points[0].y = 0;
        return;
    }
}

/**
 * @brief Look up the radius range of a ground effect kind.
 * @param kind Effect kind (0-6); other values leave both outputs untouched.
 * @param min_radius Receives the radius at which the effect intensity is zero.
 * @param max_radius Receives the radius at which the effect stops growing.
 * @note Declared inline: func_8009D9E0 expands it in place.
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
 * @brief Draw animated curved quad strips extending from a fixed-point position.
 * @param ordering_table Ordering table with 0x1000 depth entries.
 * @param packet Destination for the generated GPU packets.
 * @param position World position in signed fixed-point coordinates.
 * @param extent Maximum horizontal extent, tested after each completed strip.
 * @param forward Nonzero extends toward positive X; zero extends toward negative X.
 * @return First byte after the emitted primitives and draw-page command.
 * @note Emits at least one strip and at most four, with nine quads per strip.
 * @note Adjacent strips are separated by twice their 0x1400 fixed-point width.
 * @note Each arm of the forward test sets the whole world vector; jump2
 *       cross-jumps the shared stores back together after scheduling, so the
 *       projection that follows stays in its own scheduling block.
 */
u8* func_800A0B0C(s32* ordering_table, u8* packet, VECTOR* position, s32 extent, s32 forward)
{
    extern s32 D_801178D8;
    FieldStripWorkspace work;
    s32 step;
    s32 strip_index;
    s32 angle;
    s32 offset;
    s32 first_xy;
    s32 second_xy;
    s32 depth;
    u8* cursor;

    cursor = packet;
    strip_index = 0;
    offset = (D_801178D8 % 40) << 8;
    do
    {
        if (forward != 0)
        {
            work.world.vx = position->vx + offset;
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        }
        else
        {
            work.world.vx = position->vx - offset;
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        }
        PROJECT_POINT(POLY_AT(0), 0, work.world);
        /* Keep both starting vertices to close the strip after eight steps. */
        first_xy = *(s32*)&POLY_AT(0)->x0;
        if (forward != 0)
        {
            work.world.vx = position->vx + offset + 0x1400;
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        }
        else
        {
            work.world.vx = position->vx - offset - 0x1400;
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        }
        PROJECT_POINT(POLY_AT(0), 1, work.world);
        second_xy = *(s32*)&POLY_AT(0)->x1;
        step = 1;
        angle = 0x100;
        do
        {
            if (forward != 0)
            {
                work.world.vx = position->vx + offset + angle;
                work.world.vy = position->vy - rsin(angle) * 2;
                work.world.vz = position->vz + rcos(angle) * 2;
            }
            else
            {
                work.world.vx = position->vx - offset - angle;
                work.world.vy = position->vy - rsin(angle) * 2;
                work.world.vz = position->vz + rcos(angle) * 2;
            }
            PROJECT_POINT(POLY_AT(0), 2, work.world);
            *(s32*)&POLY_AT(0x24)->x0 = *(s32*)&POLY_AT(0)->x2;
            if (forward != 0)
            {
                work.world.vx = position->vx + offset + angle + 0x1400;
                work.world.vy = position->vy - rsin(angle) * 2;
                work.world.vz = position->vz + rcos(angle) * 2;
            }
            else
            {
                work.world.vx = position->vx - offset - angle - 0x1400;
                work.world.vy = position->vy - rsin(angle) * 2;
                work.world.vz = position->vz + rcos(angle) * 2;
            }
            PROJECT_POINT(POLY_AT(0), 3, work.world);
            /* Fade the curved strip from black to blue. */
            *(u32*)&POLY_AT(0)->r0 = 0;
            *(u32*)&POLY_AT(0)->r1 = 0xA00000;
            *(u32*)&POLY_AT(0)->r2 = 0;
            *(u32*)&POLY_AT(0)->r3 = 0xA00000;
            *(s32*)&POLY_AT(0x24)->x1 = *(s32*)&POLY_AT(0)->x3;
            SetPolyG4(POLY_AT(0));
            setSemiTrans(POLY_AT(0), 1);
            depth = position->vz >> 7;
            ADD_DEPTH_ADVANCE(depth, position->vz >> 7, POLY_G4);
            angle += 0x100;
            step++;
        } while (step < 9);
        *(s32*)&POLY_AT(0)->x2 = first_xy;
        *(s32*)&POLY_AT(0)->x3 = second_xy;
        *(u32*)&POLY_AT(0)->r0 = 0;
        *(u32*)&POLY_AT(0)->r1 = 0xA000;
        *(u32*)&POLY_AT(0)->r2 = 0;
        *(u32*)&POLY_AT(0)->r3 = 0xA000;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = position->vz >> 7;
        ADD_DEPTH_ADVANCE(depth, position->vz >> 7, POLY_G4);
        offset += 0x2800;
    } while (offset < (extent << 8) && ++strip_index < 4);
    setDrawTPage((DR_TPAGE*)cursor, 0, 0, 0x25);
    depth = position->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, position->vz >> 7, DR_TPAGE);
    return cursor;
}

/**
 * @brief Rotate (@p _radius, 0, 0) by @p matrix and store position +/- the result in work.world.
 * @note Each arm of the facing test sets the whole world vector; jump2
 *       cross-jumps the shared stores back together after scheduling, which
 *       is why the projection re-reads work.world.vx from the stack.
 */
#define STRIP_POINT(_radius)                                                                                                                                   \
    work.input.vx = (_radius);                                                                                                                                 \
    work.input.vy = 0;                                                                                                                                         \
    work.input.vz = 0;                                                                                                                                         \
    gte_SetRotMatrix(matrix);                                                                                                                                  \
    gte_ldv0(&work.input);                                                                                                                                     \
    gte_rtv0();                                                                                                                                                \
    gte_stlvnl(&work.rotated);                                                                                                                                 \
    if (facing != 0)                                                                                                                                           \
    {                                                                                                                                                          \
        work.world.vx = position->vx + (work.rotated.vx << 8);                                                                                                 \
        work.world.vy = position->vy + (work.rotated.vy << 8);                                                                                                 \
        work.world.vz = position->vz + (work.rotated.vz << 8);                                                                                                 \
    }                                                                                                                                                          \
    else                                                                                                                                                       \
    {                                                                                                                                                          \
        work.world.vx = position->vx - (work.rotated.vx << 8);                                                                                                 \
        work.world.vy = position->vy + (work.rotated.vy << 8);                                                                                                 \
        work.world.vz = position->vz + (work.rotated.vz << 8);                                                                                                 \
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
 *       close the strip.
 */
u8* func_800A1344(s32* ordering_table, u8* packet, VECTOR* position, s32 slope, s32 facing)
{
    extern s32 D_801178D8;
    FieldStripWorkspace work;
    s32 strip_index;
    s32 angle;
    s32 first_xy;
    s32 second_xy;
    MATRIX* matrix;
    s32 depth;
    s32 segment_index;
    s32 radius;
    s32 radius_sum;
    u8* cursor;

    cursor = packet;
    angle = ratan2(slope, 0x64);
    strip_index = 0;
    matrix = &work.matrices[2];
    radius = D_801178D8;
    do
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
        STRIP_POINT(radius);
        PROJECT_POINT(POLY_AT(0), 0, work.world);
        first_xy = *(s32*)&POLY_AT(0)->x0;
        STRIP_POINT(radius + 0x14);
        PROJECT_POINT(POLY_AT(0), 1, work.world);
        second_xy = *(s32*)&POLY_AT(0)->x1;
        segment_index = 1;
        radius_sum = radius;
        do
        {
            RotMatrixX(-0x100, matrix);
            STRIP_POINT(radius + (radius_sum >> 5));
            PROJECT_POINT(POLY_AT(0), 2, work.world);
            *(s32*)&POLY_AT(0x24)->x0 = *(s32*)&POLY_AT(0)->x2;
            STRIP_POINT(radius + (radius_sum >> 5) + 0x14);
            PROJECT_POINT(POLY_AT(0), 3, work.world);
            *(u32*)&POLY_AT(0)->r0 = 0;
            *(u32*)&POLY_AT(0)->r1 = 0xA00000;
            *(u32*)&POLY_AT(0)->r2 = 0;
            *(u32*)&POLY_AT(0)->r3 = 0xA00000;
            *(s32*)&POLY_AT(0x24)->x1 = *(s32*)&POLY_AT(0)->x3;
            SetPolyG4(POLY_AT(0));
            setSemiTrans(POLY_AT(0), 1);
            depth = position->vz >> 7;
            ADD_DEPTH_ADVANCE(depth, position->vz >> 7, POLY_G4);
            segment_index++;
            radius_sum += radius;
        } while (segment_index < 9);
        *(s32*)&POLY_AT(0)->x2 = first_xy;
        *(s32*)&POLY_AT(0)->x3 = second_xy;
        *(u32*)&POLY_AT(0)->r0 = 0;
        *(u32*)&POLY_AT(0)->r1 = 0xA000;
        *(u32*)&POLY_AT(0)->r2 = 0;
        *(u32*)&POLY_AT(0)->r3 = 0xA000;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = position->vz >> 7;
        ADD_DEPTH_ADVANCE(depth, position->vz >> 7, POLY_G4);
        radius += 0x50;
        if (radius >= 0x140)
        {
            radius -= 0x140;
        }
        strip_index++;
    } while (strip_index < 4);
    setDrawTPage((DR_TPAGE*)cursor, 0, 0, 0x25);
    depth = position->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, position->vz >> 7, DR_TPAGE);
    return cursor;
}

#undef STRIP_POINT
#undef PROJECT_POINT
#undef POLY_AT
#undef ADD_DEPTH_ADVANCE
