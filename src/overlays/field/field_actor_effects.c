/** @file field_actor_effects.c
 * @brief Actor movement modes and actor effect drawing.
 *
 * Covers func_8009D4D8 (set an actor's movement mode and initialize its
 * per-mode slot state) followed by the actor effect dispatcher and its
 * primitive builders.
 */

#include "common.h"
#include "controller_internal.h"
#define PORT_SAMPLE(offset) (&((ControllerPortState *)((offset) + ports))->published_sample)
#include "field_effect_render_state.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Actor position and runtime slot index in the 0x54-byte record. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padc[0x2E];
    u8 unk3A;
    u8 pad3B[0x19];
} MovementModeActor;

/** @brief Movement mode flags and signed destination coordinates in the runtime slot. */
typedef struct
{
    u8 pad0[0x5C];
    s32 unk5C;
    u8 pad60[0x114];
    s32 unk174;
    u8 pad178[0x18];
    s16 unk190;
    s16 unk192;
    u8 pad194[0xA8];
} MovementModeSlot;

s32 rand(void); /* extern */

/**
 * @see decomp.me (100%)
 * @brief Set an actor's movement mode and initialize its destination offsets.
 * @param actor Actor whose runtime movement fields are changed (index in unk3A).
 * @param mode Movement mode selector; mode 4 generates three separated random points.
 * @note Mode 4 accepts a new random point only when it is at least 0x40 units from
 *       every earlier point, using the GTE square/square-root path for the distance.
 * @note Signed fixed-point positions are divided by 256 with truncation toward zero.
 */
void func_8009D4D8(MovementModeActor *actor, u32 mode)
{
    extern MovementModeSlot g_field_object_states[];
    s32 *delta = (s32 *)0x1F800000;
    s32 *squares = (s32 *)0x1F800010;
    MovementModeSlot *base;
    s32 compare_offset;
    s32 point_offset;
    s32 slot_stride;
    s32 slot_stride_y;
    s32 base_dx;
    s32 base_dy;
    s32 compare_index;
    s32 point_index;
    s32 needs_retry;
    s32 compare_addr;
    u8 slot_index;
    MovementModeSlot *point;
    MovementModeSlot *point_y;
    MovementModeSlot *slot;
    MovementModeSlot *slot_2;
    MovementModeSlot *slot_3;
    MovementModeSlot *slot_4;
    MovementModeSlot *slot_5;

    g_field_object_states[actor->unk3A].unk5C = 0;
    switch (mode)
    {
    case 1:
        g_field_object_states[actor->unk3A].unk174 = (g_field_object_states[actor->unk3A].unk174 & ~0x3FF) | 0x40;
        return;
    case 2:
        g_field_object_states[actor->unk3A].unk174 = (g_field_object_states[actor->unk3A].unk174 & ~0x3FF) | 0x10;
        return;
    case 0:
    case 3:
        g_field_object_states[actor->unk3A].unk174 = (g_field_object_states[actor->unk3A].unk174 & ~0x3FF) | 0x1E;
        return;
    case 4:
    {
        MovementModeSlot *base;
        point_index = 0;
        slot_5 = g_field_object_states;
        base = slot_5;
        slot_4 = &base[actor->unk3A];
        slot_4->unk174 = (s32)((slot_4->unk174 & ~0x3FF) | 0x1E);
        do
        {
            needs_retry = 1;
            point_offset = point_index * 4;
            do
            {
                {
                    s32 random_value;
                    s32 offset;
                    MovementModeSlot *point;
                    random_value = (rand() >> 7) - 0x80;
                    offset = point_offset + (actor->unk3A * 0x23C);
                    point = (MovementModeSlot *)(offset + (s32)base);
                    point->unk190 = (s16)random_value;
                }
                {
                    s32 random_value;
                    s32 offset;
                    MovementModeSlot *point;
                    random_value = (rand() >> 7) - 0x80;
                    offset = point_offset + (actor->unk3A * 0x23C);
                    point = (MovementModeSlot *)(offset + (s32)base);
                    point->unk192 = (s16)random_value;
                }
                base_dx = g_field_view_offset_x;
                {
                    s32 offset;
                    offset = point_offset + (actor->unk3A * 0x23C);
                    point = (MovementModeSlot *)(offset + (s32)base);
                }

                point->unk190 = (u16)(((u16)point->unk190 - (base_dx / 256)) - (actor->unk0 / 256));
                base_dy = g_field_view_offset_z;
                {
                    s32 offset;
                    offset = point_offset + (actor->unk3A * 0x23C);
                    point_y = (MovementModeSlot *)(offset + (s32)base);
                }

                point_y->unk192 = (u16)(((u16)point_y->unk192 - (base_dy / 256)) - (actor->unk8 / 256));
                for (compare_index = 0; compare_index < point_index; compare_index++)
                {
                    do
                    {
                        slot_index = actor->unk3A;
                    } while (0);
                    compare_offset = compare_index * 4;
                    slot_stride = slot_index * 0x23C;
                    compare_addr = compare_offset + slot_stride;
                    compare_addr += (s32)base;
                    {
                        s32 current_offset;
                        current_offset = point_offset + (slot_index * 0x23C);
                        current_offset += (s32)base;
                        delta[0] = ((MovementModeSlot *)compare_addr)->unk190 - ((MovementModeSlot *)current_offset)->unk190;
                    }
                    slot_stride_y = actor->unk3A * 0x23C;
                    compare_offset += slot_stride_y;
                    compare_offset += (s32)base;
                    {
                        s32 current_offset;
                        current_offset = point_offset + (actor->unk3A * 0x23C);
                        current_offset += (s32)base;
                        delta[1] = ((MovementModeSlot *)compare_offset)->unk192 - ((MovementModeSlot *)current_offset)->unk192;
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
        MovementModeSlot *clear_base = g_field_object_states;
        s32 clear_flags = clear_base[actor->unk3A].unk174;
        s32 clear_mask = ~0x3FF;
        clear_flags &= clear_mask;
        slot_5 = &clear_base[actor->unk3A];
        slot_5->unk174 = clear_flags;
        clear_base[actor->unk3A].unk190 = 0;
        clear_base[actor->unk3A].unk192 = 0;
        return;
    }
    case 6:
    {
        MovementModeSlot *clear_base = g_field_object_states;
        MovementModeSlot *clear_slot = &clear_base[actor->unk3A];
        s32 clear_flags = clear_slot->unk174;
        s32 clear_mask = ~0x3FF;
        clear_flags &= clear_mask;
        clear_flags |= 0x1E;
        clear_slot->unk174 = clear_flags;
        clear_base[actor->unk3A].unk190 = 0;
        clear_base[actor->unk3A].unk192 = 0;
        return;
    }
    }
}

/*
 * Consolidated FIELD actor-effect translation unit.
 *
 * Members in ascending address order:
 *   func_8009D95C (from field347.c)
 *   func_8009D9E0 (from func_8009D9E0.c)
 *   func_8009E66C (from func_8009E66C.c)
 *   func_8009FE54 (from func_8009FE54.c)
 *   func_800A0B0C (from func_800A0B0C.c)
 *   func_800A1344 (from func_800A1344.c)
 *
 * func_8009D4D8 (from field_actor_movement_modes.c) precedes these members:
 * the 4-byte gap between its jump table and func_8009D95C's is the compiler's
 * 8-byte jump-table alignment inside this one object.
 *
 * D_801178D8 is read as u16 in func_8009E66C but as s32 in every other member,
 * so it is declared at block scope inside each user with that user's original
 * type and never at file scope (a file-scope copy would conflict).
 */

/* Shared camera globals, same type (s32) in every member that uses them. */

/**
 * @brief Look up a width/height pair for a field text-box style.
 *
 * Selects a fixed (@p arg1, @p arg2) size pair keyed by the style index
 * @p arg0 (0-6); out-of-range indices leave both outputs untouched.
 *
 * @param arg0 Style index (0-6).
 * @param arg1 Receives the first dimension.
 * @param arg2 Receives the second dimension.
 * @note Declared inline: func_8009D9E0 expands it in place.
 * @see decomp.me (100%) TODO
 */
inline void func_8009D95C(s32 arg0, s32 *arg1, s32 *arg2)
{
    switch (arg0)
    {
    case 0:
        *arg1 = 0x1E;
        *arg2 = 0x60;
        break;
    case 1:
        *arg1 = 0x40;
        *arg2 = 0x80;
        break;
    case 2:
        *arg1 = 0x10;
        *arg2 = 0x40;
        break;
    case 3:
        *arg1 = 0x1E;
        *arg2 = 0xC8;
        break;
    case 4:
        *arg1 = 0x1E;
        *arg2 = 0x40;
        break;
    case 5:
        *arg1 = 0;
        *arg2 = 0x64;
        break;
    case 6:
        *arg1 = 0x1E;
        *arg2 = 0x40;
        break;
    }
}

/* ---- func_8009D9E0 support declarations ---- */

#define M2C_FIELD(base, type, offset) (*(type)((unsigned char *)(base) + (offset)))
void func_8001CDAC(s32 *, s32 *);                /* extern */
extern void *D_800F2288;
/** @brief Per-actor effect state; unknown fields preserve the 0x23C stride. */
typedef struct
{
    u8 pad0[0x4A];
    u16 intensity;
    u8 pad4C[0x10];
    s32 angle;
    u8 pad60[0x114];
    union
    {
        u32 word;
        struct
        {
            u16 radius : 10;
            u16 flags : 6;
            u16 upper;
        } bits;
    } state;
    u8 pad178[0x18];
    s16 offsets[3][2];
    u8 pad19C[0xA0];
} EffectRecord;
extern EffectRecord g_field_object_states[];
extern void *g_pad_ctx;

/** @brief Actor fields used by the effect dispatcher. */
typedef struct
{
    s32 position[4];
    u8 pad10[0x11];
    u8 flags;
    u8 pad22[0x18];
    u8 slot;
} Actor;

/**
 * @brief Draw and advance an actor effect, including controller-driven offsets.
 * @param actor Actor supplying the position, facing flag, and effect slot.
 * @param kind Effect type, from 0 through 6.
 */
void func_8009D9E0(Actor *actor, u32 kind)
{
    extern s32 D_801178D8;
    /* Original block-scope prototypes; the definitions below return packet pointers. */
    s32 func_8009E66C(void *, s32, s32 *, s32);
    s32 func_8009FE54(void *, s32, s32 *, s32);
    s32 func_800A0B0C(void *, s32, s32 *, s32, s32);
    s32 func_800A1344(void *, s32, s32 *, s32, s32);
    s32 vec[12];
    s16 screen[4];
    s32 *position;
    u8 *ports = (u8 *)CONTROLLER_STATE->ports;
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
    s32 packet;
    void *ordering_table;

    ordering_table = D_800F2288 + 0x40;
    packet = M2C_FIELD(D_800F2288, s32 *, 0x40B8);
    func_8009D95C(kind, &limits[0], &limits[1]);
    radius = g_field_object_states[actor->slot].state.bits.radius;
    ratio = ((radius - limits[0]) << 8) / (limits[1] - limits[0]);
    facing = actor->flags & 0x80;
    g_field_object_states[actor->slot].intensity = ratio;
    position = actor->position;
    if (g_field_object_states[actor->slot].intensity >= 0x100)
    {
        g_field_object_states[actor->slot].intensity = 0xFF;
    }
    draw = 1;
    if (actor->slot == 2)
    {
        if ((M2C_FIELD(g_pad_ctx, s32 *, 0xAA8) & 0x7F) == 4)
        {
            draw = 0;
        }
    }
    D_801178D8 = g_field_object_states[actor->slot].angle;
    switch (kind)
    {
    case 0:
        if (draw != 0)
        {
            packet = func_8009E66C(ordering_table, packet, position, radius);
        }
        g_field_object_states[actor->slot].angle -= 0x80;
        if (g_field_object_states[actor->slot].state.bits.radius < limits[1])
        {
            g_field_object_states[actor->slot].state.word = (g_field_object_states[actor->slot].state.word & ~0x3FF) | ((g_field_object_states[actor->slot].state.bits.radius + 2) & 0x3FF);
        }
        break;
    default:
        break;
    case 1:
        if (draw != 0)
        {
            packet = func_8009FE54(ordering_table, packet, position, radius);
        }
        g_field_object_states[actor->slot].angle -= 0x80;
        if (g_field_object_states[actor->slot].state.bits.radius < limits[1])
        {
            g_field_object_states[actor->slot].state.word = (g_field_object_states[actor->slot].state.word & ~0x3FF) | ((g_field_object_states[actor->slot].state.bits.radius + 2) & 0x3FF);
        }
        break;
    case 2:
        if (draw != 0)
        {
            packet = func_800A1344(ordering_table, packet, position, radius, facing);
        }
        g_field_object_states[actor->slot].angle += 4;
        if (g_field_object_states[actor->slot].angle >= 0x50)
        {
            g_field_object_states[actor->slot].angle = 0;
        }
        if (g_field_object_states[actor->slot].state.bits.radius < limits[1])
        {
            g_field_object_states[actor->slot].state.word = (g_field_object_states[actor->slot].state.word & ~0x3FF) | ((g_field_object_states[actor->slot].state.bits.radius + 1) & 0x3FF);
        }
        break;
    case 3:
        if (g_field_object_states[actor->slot].angle < 0)
        {
            g_field_object_states[actor->slot].angle = 0;
        }
        if (draw != 0)
        {
            packet = func_800A0B0C(ordering_table, func_800A0B0C(ordering_table, packet, position, radius, 0), position, radius, 1);
        }
        g_field_object_states[actor->slot].angle += 4;
        if (g_field_object_states[actor->slot].angle >= 0x50)
        {
            g_field_object_states[actor->slot].angle = 0;
        }
        if (g_field_object_states[actor->slot].state.bits.radius < limits[1])
        {
            g_field_object_states[actor->slot].state.word = (g_field_object_states[actor->slot].state.word & ~0x3FF) | ((g_field_object_states[actor->slot].state.bits.radius + 2) & 0x3FF);
        }
        break;
    case 4:
        for (i = 0; i < 3; i++)
        {
            vec[0] = actor->position[0] + (g_field_object_states[actor->slot].offsets[i][0] << 8);
            vec[1] = actor->position[1];
            vec[2] = actor->position[2] + (g_field_object_states[actor->slot].offsets[i][1] << 8);
            if (draw != 0)
            {
                packet = func_8009E66C(ordering_table, packet, &vec[0], radius);
            }
        }
        if (g_field_object_states[actor->slot].state.bits.radius < limits[1])
        {
            g_field_object_states[actor->slot].state.word = (g_field_object_states[actor->slot].state.word & ~0x3FF) | ((g_field_object_states[actor->slot].state.bits.radius + 2) & 0x3FF);
        }
        g_field_object_states[actor->slot].angle -= 0x80;
        break;
    case 5:
        vec[0] = actor->position[0] + (g_field_object_states[actor->slot].offsets[0][0] << 8);
        vec[1] = actor->position[1];
        vec[2] = actor->position[2] + (g_field_object_states[actor->slot].offsets[0][1] << 8);
        if (draw != 0)
        {
            packet = func_8009E66C(ordering_table, packet, &vec[0], radius);
        }
        g_field_object_states[actor->slot].angle -= 0x80;
        if (g_field_object_states[actor->slot].state.bits.radius < limits[1])
        {
            g_field_object_states[actor->slot].state.word = (g_field_object_states[actor->slot].state.word & ~0x3FF) | ((g_field_object_states[actor->slot].state.bits.radius + 2) & 0x3FF);
            if (actor->flags & 0x80)
            {
                g_field_object_states[actor->slot].offsets[0][0] += 2;
            }
            else
            {
                g_field_object_states[actor->slot].offsets[0][0] -= 2;
            }
        }
        break;
    case 6:
        vec[0] = actor->position[0] + (g_field_object_states[actor->slot].offsets[0][0] << 8);
        vec[1] = actor->position[1];
        vec[2] = actor->position[2] + (g_field_object_states[actor->slot].offsets[0][1] << 8);
        if (draw != 0)
        {
            packet = func_8009E66C(ordering_table, packet, &vec[0], radius);
        }
        g_field_object_states[actor->slot].angle -= 0x80;
        if (g_field_object_states[actor->slot].state.bits.radius < limits[1])
        {
            g_field_object_states[actor->slot].state.word = (g_field_object_states[actor->slot].state.word & ~0x3FF) | ((g_field_object_states[actor->slot].state.bits.radius + 1) & 0x3FF);
        }
        if ((u8) actor->slot < 2U)
        {
            port_offset = actor->slot * sizeof(ControllerPortState);
            if (PORT_SAMPLE(port_offset)->device_type >= 0xFE)
            {
                raw_buttons = 0;
            }
            else
            {
                held = PORT_SAMPLE(port_offset)->held_buttons;
                raw_buttons = (held << 8) | (held >> 8);
            }
            buttons = ((u32) (raw_buttons & 0x40) >> 1) | ((raw_buttons & 0x20) * 2) | ((u32) (raw_buttons & 0x80) >> 3) | ((raw_buttons & 0x10) * 8) | (raw_buttons & 0xFF0F);
            vec[2] = 0;
            vec[1] = 0;
            vec[0] = 0;
            if (buttons & 0x2000)
            {
                vec[0] = 0x1000;
            }
            if (buttons & 0x8000)
            {
                vec[0] -= 0x1000;
            }
            if (buttons & 0x4000)
            {
                vec[1] = -0x1000;
            }
            if (buttons & 0x1000)
            {
                vec[1] += 0x1000;
            }
            stick_offset = actor->slot * sizeof(ControllerPortState);
            if (PORT_SAMPLE(stick_offset)->device_type != 0)
            {
                vec[0] += PORT_SAMPLE(stick_offset)->left_stick_x * 0x10;
                stick_offset = actor->slot * sizeof(ControllerPortState);
                vec[1] -= PORT_SAMPLE(stick_offset)->left_stick_y * 0x10;
            }
            if ((vec[0] | vec[1]) != 0)
            {
                func_8001CDAC(&vec[0], &vec[4]);
                vec[8] = actor->position[0] + ((g_field_object_states[actor->slot].offsets[0][0] + (vec[4] >> 10)) << 8);
                vec[9] = actor->position[1];
                vec[10] = actor->position[2] + ((g_field_object_states[actor->slot].offsets[0][1] + (vec[5] >> 10)) << 8);
                screen[0] = 0xA0 + g_field_view_offset_x / 256 + vec[8] / 256;
                screen[1] = 0x70 + g_field_view_offset_y / 256 + vec[9] / 256 - vec[10] / 512 - g_field_view_offset_z / 512;
                if ((screen[0] > 0 || vec[4] > 0) &&
                    (screen[1] > 0 || vec[5] < 0) &&
                    (screen[0] < 320 || vec[4] < 0) &&
                    (screen[1] < 224 || vec[5] > 0))
                {
                    g_field_object_states[actor->slot].offsets[0][0] += vec[4] >> 10;
                    g_field_object_states[actor->slot].offsets[0][1] += vec[5] >> 10;
                }
            }
        }
        break;
    }
    M2C_FIELD(D_800F2288, s32 *, 0x40B8) = packet;
}

#undef M2C_FIELD

/* ---- func_8009E66C ---- */

#include "sdk/libgte.h"
#include "sdk/libgpu.h"

#define PROJECT_POINT(_poly, _vert, _tmp) \
    (_poly)->x##_vert = (s16)(0xA0 + g_field_view_offset_x / 0x100 + (_tmp).vx / 0x100); \
    (_poly)->y##_vert = (s16)(0x70 + g_field_view_offset_y / 0x100 + (_tmp).vy / 0x100 - (_tmp).vz / 0x200 - g_field_view_offset_z / 0x200)

#define POLY_AT(_off) ((POLY_G4 *)(primbuf + (_off)))

#define ADD_DEPTH_ADVANCE(_depth, _expr, _type) \
    if ((_depth) < 0) { \
        addPrim(&base[0], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } else if ((_depth) >= 0x1000) { \
        addPrim(&base[0xFFF], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } else { \
        addPrim(&base[(_expr)], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    }

u8 *func_8009E66C(s32 *base, u8 *arg1, VECTOR *pos, s32 radius)
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
    u8 *primbuf;

    primbuf = arg1;

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

    v[0].vx = pos->vx + v[1].vx;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz + v[1].vz;
    PROJECT_POINT(POLY_AT(0x0), 0, v[0]);

    v[0].vx = pos->vx - v[1].vx;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz - v[1].vz;
    PROJECT_POINT(POLY_AT(0x24), 0, v[0]);

    v[0].vx = pos->vx - v[1].vz;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz + v[1].vx;
    PROJECT_POINT(POLY_AT(0x48), 0, v[0]);

    v[0].vx = pos->vx + v[1].vz;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz - v[1].vx;
    PROJECT_POINT(POLY_AT(0x6C), 0, v[0]);

    v[0].vx = pos->vx;
    v[0].vy = pos->vy;
    v[0].vz = pos->vz;
    PROJECT_POINT(POLY_AT(0x0), 1, v[0]);
    center = *(u32 *)&POLY_AT(0)->x1;
    *(u32 *)&POLY_AT(0)->x3 = center;
    *(u32 *)&POLY_AT(0x6C)->x1 = center;
    *(u32 *)&POLY_AT(0x6C)->x3 = center;
    *(u32 *)&POLY_AT(0x48)->x1 = center;
    *(u32 *)&POLY_AT(0x48)->x3 = center;
    *(u32 *)&POLY_AT(0x24)->x1 = center;
    *(u32 *)&POLY_AT(0x24)->x3 = center;

    v[0].vx = pos->vx + v[4].vx;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz + v[4].vz;
    PROJECT_POINT(POLY_AT(0x0), 2, v[0]);

    v[0].vx = pos->vx - v[4].vx;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz - v[4].vz;
    PROJECT_POINT(POLY_AT(0x24), 2, v[0]);

    v[0].vx = pos->vx - v[4].vz;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz + v[4].vx;
    PROJECT_POINT(POLY_AT(0x48), 2, v[0]);

    v[0].vx = pos->vx + v[4].vz;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz - v[4].vx;
    PROJECT_POINT(POLY_AT(0x6C), 2, v[0]);

    *(u32 *)&POLY_AT(0x0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
    *(u32 *)&POLY_AT(0x24)->r0 = (((v[1].vz >> 8) + 0xA0) << 8) & 0xFF00;
    *(u32 *)&POLY_AT(0x48)->r0 = ((0xA0 - (v[1].vx >> 8)) << 8) & 0xFF00;
    *(u32 *)&POLY_AT(0x6C)->r0 = (((v[1].vx >> 8) + 0xA0) << 8) & 0xFF00;

    *(u32 *)&POLY_AT(0x6C)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x48)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x24)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x0)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x24)->r3 = 0;
    *(u32 *)&POLY_AT(0x24)->r2 = 0;
    *(u32 *)&POLY_AT(0x48)->r3 = 0;
    *(u32 *)&POLY_AT(0x48)->r2 = 0;
    *(u32 *)&POLY_AT(0x6C)->r3 = 0;
    *(u32 *)&POLY_AT(0x6C)->r2 = 0;
    *(u32 *)&POLY_AT(0x0)->r3 = 0;
    *(u32 *)&POLY_AT(0x0)->r2 = 0;

    SetPolyG4(POLY_AT(0x0));
    SetPolyG4(POLY_AT(0x24));
    SetPolyG4(POLY_AT(0x48));
    SetPolyG4(POLY_AT(0x6C));
    setSemiTrans(POLY_AT(0x0), 1);
    setSemiTrans(POLY_AT(0x24), 1);
    setSemiTrans(POLY_AT(0x48), 1);
    setSemiTrans(POLY_AT(0x6C), 1);

    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);
    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);
    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);
    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);

    setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
    depth = (pos->vz + v[1].vz) >> 7;
    ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, DR_TPAGE);

    i = 1;
    do {
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

        v[0].vx = pos->vx + v[1].vx;
        v[0].vy = pos->vy + v[1].vy;
        v[0].vz = pos->vz + v[1].vz;
        PROJECT_POINT(POLY_AT(0), 0, v[0]);
        v[0].vx = pos->vx + v[2].vx;
        v[0].vy = pos->vy + v[2].vy;
        v[0].vz = pos->vz + v[2].vz;
        PROJECT_POINT(POLY_AT(0), 1, v[0]);
        v[0].vx = pos->vx + v[4].vx;
        v[0].vy = pos->vy + v[4].vy;
        v[0].vz = pos->vz + v[4].vz;
        PROJECT_POINT(POLY_AT(0), 2, v[0]);
        v[0].vx = pos->vx + v[5].vx;
        v[0].vy = pos->vy + v[5].vy;
        v[0].vz = pos->vz + v[5].vz;
        PROJECT_POINT(POLY_AT(0), 3, v[0]);

        *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r2 = 0;
        *(u32 *)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, POLY_G4);

        setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, DR_TPAGE);

        v[0].vx = pos->vx - v[1].vz;
        v[0].vy = pos->vy + v[1].vy;
        v[0].vz = pos->vz + v[1].vx;
        PROJECT_POINT(POLY_AT(0), 0, v[0]);
        v[0].vx = pos->vx - v[2].vz;
        v[0].vy = pos->vy + v[2].vy;
        v[0].vz = pos->vz + v[2].vx;
        PROJECT_POINT(POLY_AT(0), 1, v[0]);
        v[0].vx = pos->vx - v[4].vz;
        v[0].vy = pos->vy + v[4].vy;
        v[0].vz = pos->vz + v[4].vx;
        PROJECT_POINT(POLY_AT(0), 2, v[0]);
        v[0].vx = pos->vx - v[5].vz;
        v[0].vy = pos->vy + v[5].vy;
        v[0].vz = pos->vz + v[5].vx;
        PROJECT_POINT(POLY_AT(0), 3, v[0]);

        *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r2 = 0;
        *(u32 *)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, POLY_G4);

        setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, DR_TPAGE);

        i++;
        v[1].vx = v[2].vx;
        v[1].vy = v[2].vy;
        v[1].vz = v[2].vz;
        v[4].vx = v[5].vx;
        v[4].vy = v[5].vy;
        v[4].vz = v[5].vz;
    } while (i < 9);

    return primbuf;
}

#undef PROJECT_POINT
#undef POLY_AT
#undef ADD_DEPTH_ADVANCE

/* ---- func_8009FE54 ---- */

#define PROJECT_POINT(_poly, _vert, _tmp) \
    (_poly)->x##_vert = (s16)(0xA0 + g_field_view_offset_x / 0x100 + (_tmp).vx / 0x100); \
    (_poly)->y##_vert = (s16)(0x70 + g_field_view_offset_y / 0x100 + (_tmp).vy / 0x100 - (_tmp).vz / 0x200 - g_field_view_offset_z / 0x200)

#define POLY_AT(_off) ((POLY_G4 *)(primbuf + (_off)))

#define ADD_DEPTH_ADVANCE(_depth, _expr, _type) \
    if ((_depth) < 0) \
    { \
        addPrim(&base[0], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } \
    else if ((_depth) >= 0x1000) \
    { \
        addPrim(&base[0xFFF], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } \
    else \
    { \
        addPrim(&base[(_expr)], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    }

/**
 * @brief Append four rotating strips of shaded, translucent quads.
 * @param base Ordering table with 4096 depth buckets.
 * @param arg1 Next free primitive-buffer byte.
 * @param pos World-space center in fixed-point coordinates.
 * @param radius Radius used to construct the strips.
 * @return First free byte after the appended primitives.
 * @note Matches 100% with gcc272_cdk: 814 instructions, 3256 bytes.
 * @note Keep the vector array and unused matrix to preserve the stack layout.
 */
u8 *func_8009FE54(s32 *base, u8 *arg1, VECTOR *pos, s32 radius)
{
    extern s32 D_801178D8;
    VECTOR v[6];
    SVECTOR rot;
    MATRIX m0;
    MATRIX m1;
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
    u8 *primbuf;

    primbuf = arg1;
    i = 0;
    do
    {
        distance = (radius >> 1) + 0x40;
        p0.vx = pos->vx;
        p0.vy = pos->vy;
        p0.vz = pos->vz;
        p1.vx = pos->vx;
        p1.vy = pos->vy;
        p1.vz = pos->vz;
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
        *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 8) & 0xFF00;
        *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 8) & 0xFF00;
        *(u32 *)&POLY_AT(0)->r2 = 0;
        *(u32 *)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (p0.vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, POLY_G4);
        setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
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
            *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
            *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
            *(u32 *)&POLY_AT(0)->r2 = 0;
            *(u32 *)&POLY_AT(0)->r3 = 0;
            SetPolyG4(POLY_AT(0));
            setSemiTrans(POLY_AT(0), 1);
            depth = (p0.vz + v[1].vz) >> 7;
            ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, POLY_G4);
            setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
            depth = (p0.vz + v[1].vz) >> 7;
            ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, DR_TPAGE);
            j++;
            v[1].vx = v[2].vx;
            v[1].vy = v[2].vy;
            v[1].vz = v[2].vz;
        } while (j < 9);
        i++;
    } while (i < 4);
    return primbuf;
}

#undef PROJECT_POINT
#undef POLY_AT
#undef ADD_DEPTH_ADVANCE

/* ---- func_800A0B0C ---- */

#define ADD_PACKET_A0(depth)                                                                                                                                   \
    (*(s32*)primitive = (*(s32*)primitive & tag_mask) | (ordering_table[depth] & addr_mask),                                                                   \
     ordering_table[depth] = (ordering_table[depth] & tag_mask) | ((s32)primitive & addr_mask))

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
    /* This effect uses the world vector of the shared strip workspace layout. */
    struct
    {
        VECTOR rotated;
        VECTOR world;
        VECTOR unused;
        SVECTOR input;
        MATRIX matrices[5];
    } work;
    u8* primitive;
    s32 segment_depth;
    s32 closing_depth;
    s32 drawpage_depth;
    s32 angle;
    s32 offset;
    s32 first_x;
    s32 outer_x;
    s32 second_x;
    s32 inner_x;
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
            first_x = position->vx + offset;
        }
        else
        {
            first_x = position->vx - offset;
        }
        do
        {
            work.world.vx = first_x;
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        } while (0);
        screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
        camera_y = g_field_view_offset_y;
        *(s16*)(outer_code + 8) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(outer_code + 10) =
            112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - g_field_view_offset_z / 512;
        first_xy = (s32) * (s32*)(outer_code + 8);
        if (forward != 0)
        {
            second_x = position->vx + offset + 0x1400;
        }
        else
        {
            second_x = (position->vx - offset) - 0x1400;
        }
        do
        {
            work.world.vx = second_x;
            work.world.vy = position->vy;
            work.world.vz = position->vz + 0x2000;
        } while (0);
        screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
        camera_y = g_field_view_offset_y;
        *(s16*)(outer_code + 16) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(outer_code + 18) =
            112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - g_field_view_offset_z / 512;
        step = 1;
        angle = 0x100;
        strip_code = primitive;
        second_xy = (s32) * (s32*)(outer_code + 16);
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
                            inner_x = position->vx + offset + angle;
                        }
                        else
                        {
                            inner_x = (position->vx - offset) - angle;
                        }
                        work.world.vx = inner_x;
                        work.world.vy = position->vy - (rsin(trig_angle) * 2);
                        work.world.vz = position->vz + (rcos(angle) * 2);
                    } while (0);
                    screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
                    camera_y = g_field_view_offset_y;
                    *(s16*)(strip_code + 24) = screen_x;
                    if (camera_y < 0)
                    {
                        camera_y += 255;
                    }
                    *(s16*)(strip_code + 26) =
                        112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - g_field_view_offset_z / 512;
                    do
                    {
                        *(s32*)(strip_code + 44) = (s32) * (s32*)(strip_code + 24);
                        do
                        {
                            trig_angle = angle;
                            if (forward != 0)
                            {
                                outer_x = position->vx + offset + angle + 0x1400;
                            }
                            else
                            {
                                outer_x = ((position->vx - offset) - angle) - 0x1400;
                            }
                            work.world.vx = outer_x;
                            work.world.vy = position->vy - (rsin(trig_angle) * 2);
                            work.world.vz = position->vz + (rcos(angle) * 2);
                        } while (0);
                        screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
                        camera_y = g_field_view_offset_y;
                        *(s16*)(strip_code + 32) = screen_x;
                        if (camera_y < 0)
                        {
                            camera_y += 255;
                        }
                        *(s16*)(strip_code + 34) =
                            112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - g_field_view_offset_z / 512;
                        /* Fade the curved strip from black to blue. */
                        *(s32*)(strip_code + 4) = 0;
                        *(s32*)(strip_code + 12) = 0xA00000;
                        *(s32*)(strip_code + 20) = 0;
                        *(s32*)(strip_code + 28) = 0xA00000;
                        *(s32*)(strip_code + 52) = (s32) * (s32*)(strip_code + 32);
                        SetPolyG4((POLY_G4*)primitive);
                        *(strip_code + 7) |= 2;
                        segment_depth = position->vz >> 7;
                        if (segment_depth < 0)
                        {
                            ADD_PACKET_A0(0);
                            primitive += 0x24;
                            strip_code += 0x24;
                            outer_code += 0x24;
                        }
                        else if (segment_depth >= 0x1000)
                        {
                            ADD_PACKET_A0(0xFFF);
                            primitive += 0x24;
                            strip_code += 0x24;
                            outer_code += 0x24;
                        }
                        else
                        {
                            ADD_PACKET_A0(position->vz >> 7);
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
        *(u8*)(outer_code + 7) = (u8)(*(u8*)(outer_code + 7) | 2);
        do
        {
            closing_depth = position->vz >> 7;
            if (closing_depth < 0)
            {
                do
                {
                    ADD_PACKET_A0(0);
                    primitive += 0x24;
                    outer_code += 0x24;
                } while (0);
            }
            else if (closing_depth >= 0x1000)
            {
                do
                {
                    ADD_PACKET_A0(0xFFF);
                    primitive += 0x24;
                    outer_code += 0x24;
                } while (0);
            }
            else
            {
                do
                {
                    ADD_PACKET_A0(position->vz >> 7);
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

#undef ADD_PACKET_A0

/* ---- func_800A1344 ---- */

#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    VECTOR rotated;
    VECTOR world;
    VECTOR unused;
    SVECTOR input;
    MATRIX matrices[5];
} FieldStripWorkspace;

#define ADD_PACKET(depth)                                                                                                                                      \
    (*(s32*)current_packet = (*(s32*)current_packet & tag_mask) | (ordering_table[depth] & addr_mask),                                                         \
     ordering_table[depth] = (ordering_table[depth] & tag_mask) | ((s32)current_packet & addr_mask))

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
next_strip:
    {
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
        do
        {
            work.world.vx = first_x;
            work.world.vy = position->vy + (work.rotated.vy << 8);
            work.world.vz = position->vz + (work.rotated.vz << 8);
        } while (0);
        screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
        camera_y = g_field_view_offset_y;
        *(s16*)(strip_code + 1) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(strip_code + 3) = 112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - g_field_view_offset_z / 512;
        first_xy = *(s32*)(strip_code + 0x1);
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
        do
        {
            work.world.vx = second_x;
            work.world.vy = position->vy + (work.rotated.vy << 8);
            work.world.vz = position->vz + (work.rotated.vz << 8);
        } while (0);
        screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
        camera_y = g_field_view_offset_y;
        *(s16*)(strip_code + 9) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(strip_code + 11) = 112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - g_field_view_offset_z / 512;
        segment_index = 1;
        radius_sum = radius;
        segment_packet = current_packet;
        second_xy = *(s32*)(strip_code + 0x9);
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
            do
            {
                work.world.vx = segment_inner_x;
                work.world.vy = position->vy + (work.rotated.vy << 8);
                work.world.vz = position->vz + (work.rotated.vz << 8);
            } while (0);
            screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
            camera_y = g_field_view_offset_y;
            *(s16*)(segment_packet + 24) = screen_x;
            if (camera_y < 0)
            {
                camera_y += 255;
            }
            *(s16*)(segment_packet + 26) =
                112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - g_field_view_offset_z / 512;
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
            do
            {
                work.world.vx = segment_outer_x;
                work.world.vy = position->vy + (work.rotated.vy << 8);
                work.world.vz = position->vz + (work.rotated.vz << 8);
            } while (0);
            screen_x = 160 + g_field_view_offset_x / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
            camera_y = g_field_view_offset_y;
            *(s16*)(segment_packet + 32) = screen_x;
            if (camera_y < 0)
            {
                camera_y += 255;
            }
            *(s16*)(segment_packet + 34) =
                112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - g_field_view_offset_z / 512;
            *(s32*)(segment_packet + 4) = 0;
            *(s32*)(segment_packet + 12) = 0xA00000;
            *(s32*)(segment_packet + 20) = 0;
            *(s32*)(segment_packet + 28) = 0xA00000;
            *(s32*)(segment_packet + 52) = *(s32*)(segment_packet + 32);
            SetPolyG4((POLY_G4*)current_packet);
            do
            {
                *(u8*)(segment_packet + 7) = (u8)(*(u8*)(segment_packet + 7) | 2);
                segment_depth = position->vz >> 7;
                if (segment_depth < 0)
                {
                    ADD_PACKET(0);
                    current_packet += 0x24;
                    segment_packet += 0x24;
                    strip_code += 0x24;
                }
                else if (segment_depth >= 0x1000)
                {
                    ADD_PACKET(0xFFF);
                    current_packet += 0x24;
                    segment_packet += 0x24;
                    strip_code += 0x24;
                }
                else
                {
                    ADD_PACKET(position->vz >> 7);
                    current_packet += 0x24;
                    segment_packet += 0x24;
                    strip_code += 0x24;
                }
                segment_index += 1;
                radius_sum += radius;
            } while (0);
        } while (segment_index < 9);
        *(s32*)(strip_code + (17)) = first_xy;
        *(s32*)(strip_code + (25)) = second_xy;
        *(s32*)(strip_code + (-3)) = 0;
        *(s32*)(strip_code + (5)) = 0xA000;
        *(s32*)(strip_code + (13)) = 0;
        *(s32*)(strip_code + (21)) = 0xA000;
        SetPolyG4((POLY_G4*)current_packet);
        *(u8*)(strip_code + (0)) = (u8)(*(u8*)(strip_code + (0)) | 2);
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
            ADD_PACKET(0xFFF);
            current_packet += 0x24;
            strip_code += 0x24;
        }
        else
        {
            ADD_PACKET(position->vz >> 7);
            current_packet += 0x24;
            strip_code += 0x24;
        }
        radius += 0x50;
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
    *(u8*)(current_packet + (3)) = 1;
    *(s32*)(current_packet + (4)) = 0xE1000025;
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

#undef ADD_PACKET
