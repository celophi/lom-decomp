#include "common.h"
#include "field_types.h"
/** @brief Byte access within partially recovered actor and controller records. */
#define U8_AT(p, o) (*(u8 *)((s32)(p) + (o)))
#define S8_AT(p, o) (*(s8 *)((s32)(p) + (o)))
#define U16_AT(p, o) (*(u16 *)((s32)(p) + (o)))
#define S16_AT(p, o) (*(s16 *)((s32)(p) + (o)))
#define S32_AT(p, o) (*(s32 *)((s32)(p) + (o)))
void func_8001CDAC(s32 *, s32 *);
s32 func_8005B368(void *);
s32 func_8005B6AC(void *);
s32 func_8006751C(s32);
void func_8008EF0C(void *);
s16 func_80091914(void *, s32);
void func_80091AC8(void *, s32);
s32 func_80092988(void *, s32 *);
s32 func_800987DC(void *, void *, s32);
void func_80098C7C(void *, s32);
void func_800A2594(s32, s32);
s32 func_800A6490(void);
s32 func_800B0850(void);
void func_8008E690(void *);
void func_8008EBA4(void *, s32, s32);
extern s32 D_800F22A0, D_800F22A4, D_800F22A8, D_800FE754, D_8010AE64, D_80122B20;
extern u8 D_8010AE84;
/** @brief Resource flags selecting movement-state handling. */
typedef struct
{
    u8 pad00[0x10];
    u32 flags;
} MovementResource;
/** @brief Actor metadata containing the collision-scale selector at offset 0x2E. */
typedef struct
{
    u8 pad00[0x2E];
    u8 scale;
    u8 pad2F[0x19];
} MovementScale;
/** @brief Accessed fields of a 0x23C-byte actor slot; flags overlap the height halfword. */
typedef struct
{
    u8 pad00[0xC];
    s32 flags;
    u8 pad10[0x164];
    union
    {
        s32 flags;
        struct
        {
            s16 low;
            s16 height;
        } half;
    } state;
    u8 pad178[0x24];
    s32 polygon;
    s32 region;
    u8 pad1A4[0x98];
} MovementSlot;
extern MovementScale D_800FE3A0[];
extern MovementSlot D_80105AE0[];
extern MovementResource g_field_resource_entries[];
/** @brief Scratchpad collision request with coordinates, displacement, and packed bounds. */
typedef struct
{
    s32 x, y, z, dx, dy, dz, height, polygon, region;
    s16 radius_x, radius_y;
    union
    {
        s32 flags;
        s16 radius_z;
    } tail;
} MovementCollision;
/** @brief Actor movement flags at offset 0x1C. */
typedef struct
{
    u8 pad00[0x1C];
    u32 flags;
} MovementFlags;
/** @brief Contiguous input, destination, and displacement vectors with projected screen
 * coordinates. */
typedef struct
{
    FieldVector input, motion, change;
    s16 screen_x, screen_y;
} MovementWork;
/**
 * @brief Apply controller movement, resolve collisions, and update an actor's movement state.
 * @param actor Actor position and state record.
 * @param pad_index Controller bank index.
 * @return The actor's two-bit movement state, or zero when input processing is blocked.
 * @note The collision request occupies scratchpad 0x1F800000; the secondary probe uses 0x1F800040.
 * @note Contiguous work vectors retain the target's stack layout and screen-coordinate stores.
 * @note WIP: 99.264120% gcc272_cdk; only the s0/s1 register assignment differs.
 */
s32 func_8008DC54(s32 *actor, s32 pad_index)
{
    MovementWork work;
    u8 *pad_base = (u8 *)0x801ED600;
    u8 *map = (u8 *)0x801ED400;
    u8 *probe = (u8 *)0x1F800040;
    MovementCollision *collision = (MovementCollision *)0x1F800000;
    s16 actor_state;
    s16 speed_divisor;
    s32 input_z;
    s32 input_x;
    s32 motion_x;
    s32 sentinel_or_reaction;
    s32 buttons_offset;
    s32 analog_offset;
    s32 collision_x;
    s32 motion_z;
    s32 actor_x;
    s32 actor_z;
    s32 collision_z;
    s32 record_input;
    s32 controller_state;
    s32 has_input;
    s32 speed_step;
    u16 buttons_word;
    u8 pad_status;
    void *pad;
    void *old_slot;
    void *new_slot;

    buttons_offset = pad_index * 0xAE;
    if ((u8)U8_AT(pad_base, buttons_offset) >= 0xFEU)
    {
        D_8010AE64 = 0;
    }
    else
    {
        buttons_word = U16_AT(pad_base, buttons_offset + 2);
        D_8010AE64 = ((buttons_word << 8) & 0xFF00) | (buttons_word >> 8);
    }
    D_8010AE64 = ((u32)(D_8010AE64 & 0x40) >> 1) | ((D_8010AE64 & 0x20) * 2) |
                 ((u32)(D_8010AE64 & 0x80) >> 3) | ((D_8010AE64 & 0x10) * 8) | (D_8010AE64 & ~0xF0);
    actor_state = S16_AT(actor, 0x2A);
    record_input = 0;
    if (actor_state != 0x86)
    {
        record_input = actor_state != 0x96;
    }
    func_800A2594(pad_index, record_input);
    if (D_80122B20 != 0)
    {
        D_8010AE64 = 0;
    }
    if (S16_AT(actor, 0x2A) != 0)
    {
        func_8008EF0C(actor);
        return 0;
    }
    if (func_800B0850() != 0)
    {
        return 0;
    }
    work.input.vx = 0;
    work.input.vy = 0;
    work.input.vz = 0;
    ((MovementFlags *)actor)->flags = (u32)(((MovementFlags *)actor)->flags & ~0x600);
    if ((D_80122B20 == 0) && (g_field_resource_entries[U8_AT(actor, 0x3B)].flags & 1))
    {
        S16_AT(actor, 0x2A) = func_80091914(actor, pad_index);
    }
    if (func_800A6490() == 0)
    {
        controller_state = func_8006751C(0);
        sentinel_or_reaction = -1;
        if (controller_state != sentinel_or_reaction)
        {
            return 0;
        }
        if (func_8006751C(1) != sentinel_or_reaction)
        {
            return 0;
        }
    }
    input_z = 0;
    analog_offset = pad_index * 0xAE;
    pad = pad_base + analog_offset;
    pad_status = U8_AT(pad_base, analog_offset);
    input_x = input_z;
    if (pad_status != 0 && D_80122B20 == 0)
    {
        if (pad_status >= 0xFEU)
        {
            work.input.vx = 0;
            work.input.vz = 0;
        }
        else
        {
            input_x = work.input.vx = S16_AT(pad, 0xC);
            input_z = work.input.vz = -S16_AT(pad, 0xE);
        }
    }
    has_input = input_x | input_z;
    if (has_input == 0)
    {
        if (D_8010AE64 & 0x2000)
        {
            ((MovementFlags *)actor)->flags =
                (u32)((((MovementFlags *)actor)->flags & ~0x600) | 0x200);
            input_x += 0x1000;
            work.input.vx += 0x1000;
        }
        if (D_8010AE64 & 0x8000)
        {
            ((MovementFlags *)actor)->flags =
                (u32)((((MovementFlags *)actor)->flags & ~0x600) | 0x200);
            input_x -= 0x1000;
            work.input.vx -= 0x1000;
        }
        if (D_8010AE64 & 0x4000)
        {
            input_z -= 0x1000;
            work.input.vz -= 0x1000;
            ((MovementFlags *)actor)->flags =
                (u32)((((MovementFlags *)actor)->flags & ~0x600) | 0x200);
        }
        if (D_8010AE64 & 0x1000)
        {
            input_z += 0x1000;
            work.input.vz += 0x1000;
            goto mark_moving;
        }
    }
    else
    {
    mark_moving:
        ((MovementFlags *)actor)->flags = (u32)((((MovementFlags *)actor)->flags & ~0x600) | 0x200);
    }
    if (D_80105AE0[U8_AT(actor, 0x3A)].flags & 8)
    {
        input_x = -input_x;
        input_z = -input_z;
        work.input.vx = -work.input.vx;
        work.input.vz = -work.input.vz;
    }
    func_8001CDAC(&work.input.vx, &work.motion.vx);
    speed_divisor = S16_AT(actor, 0x16);
    if (speed_divisor != 0)
    {
        speed_step = (s8)S8_AT(actor, 0x36) / speed_divisor;
        S8_AT(actor, 0x36) = (s8)((u8)S8_AT(actor, 0x36) - speed_step);
    }
    else
    {
        speed_step = 0;
    }
    motion_x = (s32)(work.motion.vx * speed_step) >> 4;
    work.motion.vx = motion_x;
    motion_z = (s32)(work.motion.vz * speed_step) >> 4;
    work.motion.vz = motion_z;
    if (D_8010AE84 == 0)
    {
        if (func_80092988(actor, &work.motion.vx) == 0)
        {
            collision->x = S32_AT(actor, 0x0);
            collision->y = S32_AT(actor, 0x4);
            collision->z = S32_AT(actor, 0x8);
            actor_x = S32_AT(actor, 0x0);
            if ((actor_x >= 0) && (actor_x < (S16_AT(map, 0) << 8)) &&
                (actor_z = S32_AT(actor, 0x8), (actor_z >= 0)) &&
                (actor_z < ((s32)(U16_AT(map, 2) << 0x10) >> 7)))
            {
                collision->dy = 0;
                collision->radius_y = 0x10;
                S16_AT(probe, 0xE) = 0x10;
                collision->dx = work.motion.vx;
                collision->dz = work.motion.vz;
                if (D_800FE3A0[U8_AT(actor, 0x3A)].scale == 0x40)
                {
                    collision->radius_x = 0xC;
                    S16_AT(probe, 0xC) = 0xC;
                    collision->tail.radius_z = 8;
                    S16_AT(probe, 0x10) = 8;
                }
                else
                {
                    collision->radius_x = 9;
                    S16_AT(probe, 0xC) = 9;
                    collision->tail.radius_z = 6;
                    S16_AT(probe, 0x10) = 6;
                }

                collision->tail.flags &= 0xFFFDFFFF;
                collision->tail.flags &= 0xFFFEFFFF;
                collision->polygon = D_80105AE0[U8_AT(actor, 0x3A)].polygon;
                collision->region = D_80105AE0[U8_AT(actor, 0x3A)].region;
                if ((func_8005B6AC(collision) & 3) == 3)
                {
                    ((MovementFlags *)actor)->flags =
                        (u32)(((MovementFlags *)actor)->flags & ~0x600);
                }
                D_80105AE0[U8_AT(actor, 0x3A)].polygon = (s32)collision->polygon;
                D_80105AE0[U8_AT(actor, 0x3A)].region = (s32)collision->region;
                S32_AT(actor, 0x4) = (s32)collision->y;
                D_80105AE0[U8_AT(actor, 0x3A)].state.half.height = collision->height / 256;
            }
            else
            {
                D_80105AE0[U8_AT(actor, 0x3A)].polygon = -1;
                D_80105AE0[U8_AT(actor, 0x3A)].region = 0;
                D_80105AE0[U8_AT(actor, 0x3A)].state.half.height = 0;
            }
            collision_x = collision->x;
            collision_z = collision->z;
            work.motion.vx = collision_x;
            work.motion.vz = collision_z;
            work.change.vx = collision_x - S32_AT(actor, 0x0);
            work.change.vy = 0;
            work.change.vz = collision_z - S32_AT(actor, 0x8);
            if (func_80092988(actor, &work.change.vx) != 0)
            {
                work.motion.vx = S32_AT(actor, 0x0);
                work.motion.vz = S32_AT(actor, 0x8);
                ((MovementFlags *)actor)->flags = (u32)(((MovementFlags *)actor)->flags & ~0x600);
            }
            S32_AT(probe, 0x0) = (s32)collision->x;
            S32_AT(probe, 0x4) = (s32)S32_AT(actor, 0x4);
            S32_AT(probe, 0x8) = (s32)collision->z;
            if ((D_800FE754 != 0) && (func_8005B368(probe) != -1))
            {
                goto cancel_movement;
            }
        }
        else
        {
        cancel_movement:
            work.motion.vx = S32_AT(actor, 0x0);
            work.motion.vz = S32_AT(actor, 0x8);
            ((MovementFlags *)actor)->flags = (u32)(((MovementFlags *)actor)->flags & ~0x600);
        }
    }
    else
    {
        work.motion.vx = motion_x + S32_AT(actor, 0x0);
        work.motion.vz = motion_z + S32_AT(actor, 0x8);
    }
    work.motion.vy = S32_AT(actor, 0x4);
    if (S16_AT(actor, 0x2A) == 0)
    {
        func_8008EBA4(actor, input_x, input_z);
    }
    else
    {
        func_8008E690(actor);
    }
    work.screen_x = D_800F22A0 / 256 + (s16)(work.motion.vx / 256 + 160);
    work.screen_y = D_800F22A4 / 256 + (s16)(work.motion.vy / 256 + 112) - work.motion.vz / 512 -
                    D_800F22A8 / 512;
    if (S16_AT(actor, 0x2A) == 0)
    {
        if (((u32)D_80105AE0[U8_AT(actor, 0x3A)].state.flags >> 0xE) & 1)
        {
            S32_AT(actor, 0x0) = work.motion.vx;
            S32_AT(actor, 0x8) = work.motion.vz;
            if (func_800987DC(actor, actor, 0) == 0)
            {
                old_slot = (U8_AT(actor, 0x3A) * 0x23C) + (s32)D_80105AE0;
                S32_AT(old_slot, 0x174) = (s32)(S32_AT(old_slot, 0x174) & ~0x4000);
            }
        }
        else
        {
            sentinel_or_reaction = func_800987DC(actor, &work.motion.vx, 0) & 0x7FFF;
            if (sentinel_or_reaction < 3)
            {
                S32_AT(actor, 0x0) = work.motion.vx;
                S32_AT(actor, 0x8) = work.motion.vz;
            }
            else
            {
                if (func_800987DC(actor, actor, 0) != 0)
                {
                    new_slot = (U8_AT(actor, 0x3A) * 0x23C) + (s32)D_80105AE0;
                    S32_AT(new_slot, 0x174) = (s32)(S32_AT(new_slot, 0x174) | 0x4000);
                }
                ((MovementFlags *)actor)->flags = (u32)(((MovementFlags *)actor)->flags & ~0x600);
                func_80098C7C(actor, sentinel_or_reaction);
            }
        }
    }
    func_80091AC8(actor, pad_index);
    if (S16_AT(actor, 0x2A) != 0)
    {
        func_8008EF0C(actor);
    }
    return ((u32)((MovementFlags *)actor)->flags >> 9) & 3;
}
