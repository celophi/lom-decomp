#include "common.h"
#include "field_types.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/*
 * Consolidated field actor-behavior translation unit.
 *
 * Members (ascending address order):
 *   func_8008DC54  (from func_8008DC54.c)
 *   func_8008E690  (from func_8008E690.c)
 *   func_8008EBA4  (from func_8008EBA4.c)
 *   func_8008EF0C  (from func_8008EF0C.c)
 *   func_80090B38  (from func_80090B38.c)
 *   func_80090D48  (from func_80090D48.c)
 *   func_80090F50  (from field38.c)
 *   func_80090F80  (from func_80090F80.c)
 *   func_8009104C  (from func_8009104C.c)
 *
 * Each function keeps its original declaration environment (types, extern
 * declarations, and prototypes) at BLOCK scope so the incompatible views of
 * the shared records (D_80105AE0, g_field_resource_entries, D_800FD818, ...)
 * and the implicit callee declarations in func_8008EF0C are reproduced exactly.
 */

/** @brief Byte access within partially recovered actor and controller records. */
#define U8_AT(p, o) (*(u8 *)((s32)(p) + (o)))
#define S8_AT(p, o) (*(s8 *)((s32)(p) + (o)))
#define U16_AT(p, o) (*(u16 *)((s32)(p) + (o)))
#define S16_AT(p, o) (*(s16 *)((s32)(p) + (o)))
#define S32_AT(p, o) (*(s32 *)((s32)(p) + (o)))

#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8*)(expr) + (offset)))

#define SLOT23C(i) (((Func8008Slot*)&D_80105AE0)[(i)])
#define FD268(i) (((Func8008FDEntry*)&D_800FD818)[(i)])
#define RESOURCE14(i) (((Func8008ResourceEntry*)&g_field_resource_entries)[(i)])

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

    extern s32 D_800F22A0, D_800F22A4, D_800F22A8, D_800FE754, D_8010AE64, D_80122B20;
    extern u8 D_8010AE84;
    extern MovementScale D_800FE3A0[];
    extern MovementSlot D_80105AE0[];
    extern MovementResource g_field_resource_entries[];

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

/** @brief Partial 0x54-byte actor layout used by the func_8008E690 action dispatcher. */
typedef struct
{
    u8 pad0[0x21];
    u8 unk21;
    u8 pad22[2];
    u8 unk24;
    u8 pad25[2];
    u8 unk27;
    u8 pad28[2];
    union
    {
        u16 word;
        u8 bytes[2];
    } state;
    u8 pad2c[2];
    u16 unk2E, unk30;
    u8 pad32[8];
    u8 unk3A, unk3B;
    u8 pad3c[0x54 - 0x3C];
} E690Actor;

/**
 * @brief Validate and dispatch the pending resource action for an actor.
 *
 * The high command byte selects a resource descriptor. Failed availability or
 * slot checks clear the pending command; accepted actions update slot flags,
 * optionally start an animation, then apply the descriptor's action flags.
 *
 * @param actor Actor whose pending command and action state are updated.
 */
void func_8008E690(E690Actor *actor)
{
    /** @brief Partial 0x23C-byte actor-slot layout and overlapping status bytes. */
    typedef struct
    {
        u8 pad0[0x3C];
        s32 unk3C;
        u8 pad40[8];
        u16 unk48;
        u8 pad4a[2];
        s32 unk4C;
        u8 pad50[0x16F - 0x50];
        u8 unk16F;
        u8 pad170[4];
        s32 unk174;
        union
        {
            s32 word;
            u8 bytes[4];
        } flags;
        u8 pad17c[0x23C - 0x17C];
    } Slot;
    /** @brief Eight-byte resource action descriptor. */
    typedef struct
    {
        u16 unk0, unk2, unk4, unk6;
    } Action;
    /** @brief Resource directory entry containing the availability flags. */
    typedef struct
    {
        u8 pad0[0x10];
        s32 flags;
    } Resource;
    /** @brief Party entry with the action-table bank selector at offset one. */
    typedef struct
    {
        u8 unk0, unk1;
        u8 pad2[0x268 - 2];
    } Party;

    extern Slot D_80105AE0[];
    extern Action D_8010A038[];
    extern Party D_800FD818[];
    extern Resource g_field_resource_entries[];
    extern s32 D_8010AE58;

    s32 field_object_has_active_actor_tracks(s32);
    s32 field_count_free_actor_slots(s32);
    s32 func_8008404C(s32, s32);
    void func_800A3938(s32, s32);
    void func_8006C3FC(E690Actor *);
    s32 func_800839F8(s32, s32);
    s32 func_80083EEC(s32, s32, s32);
    void field_start_actor_animation(s32, s32, u8 *);
    void func_8009D4D8(E690Actor *, s32);

    s32 animation_slot;
    s32 resource_offset;
    s32 party_offset;
    s32 action_index;
    u16 requirement;
    u16 animation;
    u8 actor_index;
    u8 mode;
    Slot *slot;
    Action *action;

    if (actor->state.bytes[0] == 0x85)
    {
        D_80105AE0[actor->unk3A].flags.word = (s32)(D_80105AE0[actor->unk3A].flags.word & ~0x1C);
        if (g_field_resource_entries[actor->unk3B].flags & 1)
        {
            D_80105AE0[actor->unk3A].unk16F = (s8)(actor->state.word >> 8);
            /* Each resource owns 50 eight-byte descriptors. */
            resource_offset = actor->unk3B * 0x190;
            actor->state.word = actor->state.bytes[0];
            slot = &D_80105AE0[actor->unk3A];
            mode = slot->unk16F;
            action = (Action *)(resource_offset + (s32)&D_8010A038[mode]);
            if (mode == 0xB)
            {
                slot->unk16F = *(u8 *)&action->unk0;
            }
            if (!(action->unk2 & 0x400))
            {
                if (action->unk0 == 0 && action->unk4 == 0)
                {
                    goto play_failure;
                }
                /* Preserve the descriptor re-read present in the original path. */
                if (!(((volatile Action *)action)->unk2 & 0x400))
                {
                    goto check_action;
                }
            }
            if ((field_object_has_active_actor_tracks(actor->unk3A) == 0) &&
                (field_count_free_actor_slots(actor->unk3A) >= 3) && (actor->unk30 == 0))
            {
            check_action:
                D_80105AE0[actor->unk3A].flags.word =
                    (s32)(D_80105AE0[actor->unk3A].flags.word & ~2);
                if (((u16)action->unk0 & 0x8000) && !(action->unk2 & 0x400))
                {
                    if ((field_object_has_active_actor_tracks(actor->unk3A) == 0) &&
                        (D_8010AE58 == 0) && (field_count_free_actor_slots(actor->unk3A) >= 3))
                    {
                        actor_index = actor->unk3A;
                        if (D_80105AE0[actor_index].unk48 != 0xFF)
                        {
                        play_failure:
                            func_800A3938(0x78, 0x80);
                        cancel_action:
                            actor->state.word = 0;
                            return;
                        }
                        action_index = action->unk0 & 0x7fff;
                        party_offset = (D_800FD818[actor_index].unk1 * 0x18) + 0x88;
                        if (func_8008404C(actor_index, action_index + party_offset) != 0)
                        {
                            D_80105AE0[actor->unk3A].unk174 =
                                (s32)(D_80105AE0[actor->unk3A].unk174 | 0x8000);
                            goto start_action;
                        }
                        goto cancel_action;
                    }
                    goto cancel_action;
                }
                requirement = action->unk6;
                if (!(requirement & 0x8000) ||
                    (func_8008404C(actor->unk3A, requirement & 0x3FF) != 0))
                {
                start_action:
                    if (action->unk2 & 0x400)
                    {
                        D_80105AE0[actor->unk3A].unk4C = (s32)(D_80105AE0[actor->unk3A].unk4C & ~1);
                        D_80105AE0[actor->unk3A].flags.word =
                            (s32)(D_80105AE0[actor->unk3A].flags.word | 0x40);
                        actor->unk2E = 1;
                        actor->unk24 = 1;
                        actor->unk27 = 0;
                        actor->unk21 = (u8)((actor->unk21 & 0x80) + 0x10);
                        D_80105AE0[actor->unk3A].unk174 =
                            (s32)(D_80105AE0[actor->unk3A].unk174 & ~0x1800);
                        func_8006C3FC(actor);
                        if (action->unk4 != 0)
                        {
                            D_80105AE0[actor->unk3A].unk3C = (s32)action->unk4;
                        }
                        D_80105AE0[actor->unk3A].unk174 =
                            (s32)(D_80105AE0[actor->unk3A].unk174 & ~0x400);
                    }
                    else if (!((u16)action->unk0 & 0x8000))
                    {
                        animation = action->unk4;
                        if ((animation != 0xFFFF) && (animation != 0))
                        {
                            animation_slot = func_800839F8(actor->unk3A, 0);
                            if ((animation_slot != -1) &&
                                (func_80083EEC(actor->unk3A, animation_slot, action->unk4) != 0))
                            {
                                field_start_actor_animation(animation_slot, 0, 0);
                                D_80105AE0[actor->unk3A].flags.bytes[1] = animation_slot;
                            }
                        }
                    }
                    func_8009D4D8(actor, *(u8 *)&action->unk2);
                }
                else
                {
                    goto cancel_action;
                }
            }
            else
            {
                goto cancel_action;
            }
        }
        else
        {
            goto cancel_action;
        }
    }
}

/** @brief Partial field record containing direction and animation state (func_8008EBA4). */
typedef struct FieldMotionRecord
{
    u8 pad00[0x1B];
    u8 direction;
    u8 pad1C[5];
    u8 animation;
    u8 pad22[2];
    u8 active;
    u8 pad25[2];
    u8 animation_frame;
    u8 pad28[6];
    u16 idle_mode;
    u8 pad30[3];
    u8 movement_flags;
    u8 pad34[5];
    u8 stop_delay;
    u8 pad3A;
    u8 resource_index;
} FieldMotionRecord;

/**
 * @brief Select a movement animation from the requested displacement, or settle to idle.
 * @param record Field record whose direction and animation state are updated.
 * @param delta_x Horizontal displacement used to select a direction.
 * @param delta_z Depth displacement used to select a direction.
 * @note Resource flag 1 selects the alternate directional animation tables.
 */
void func_8008EBA4(FieldMotionRecord *record, s32 delta_x, s32 delta_z)
{
    /** @brief Resource metadata entry containing animation-layout flags. */
    typedef struct FieldMotionResource
    {
        u8 pad00[0x10];
        u32 flags;
    } FieldMotionResource;

    extern FieldMotionResource g_field_resource_entries[];
    extern s32 D_800EB0A4[];
    extern s32 D_800EB0C4[];
    extern s32 D_800EB0E4[];
    extern s32 rand(void);
    extern void func_8006C3FC(FieldMotionRecord *record);

    s32 *vertical_entry;
    s32 *direction_entry;
    s32 sector_index;
    s32 direction_or_animation;
    s32 movement_direction;
    s32 old_animation;
    s32 direction_table_address;
    s32 sector_or_flags;
    s32 direction_offset;
    u8 resource_index;
    u8 current_animation;
    s32 previous_animation;
    s32 low_state;
    s32 idle_state;
    s32 state_mask;
    FieldMotionRecord *refresh_record;
    u8 idle_delay;
    u8 movement_delay;

    resource_index = record->resource_index;
    if (g_field_resource_entries[resource_index].flags & 1)
    {
        if ((delta_x | delta_z) != 0)
        {
            direction_or_animation = ratan2(-delta_z, delta_x);
            direction_or_animation >>= 4;
            direction_or_animation += 0x10;
            if (direction_or_animation < 0)
            {
                direction_or_animation += 0x100;
            }
            if (direction_or_animation >= 0x100)
            {
                direction_or_animation -= 0x100;
            }
            sector_or_flags = direction_or_animation >> 5;
            if ((sector_or_flags == 2) || (sector_or_flags == 6))
            {
                s32 *vertical_base;

                state_mask = ~0x80;
                vertical_base = D_800EB0C4;
                vertical_entry = vertical_base + sector_or_flags;
                if ((record->animation & state_mask) != (*vertical_entry & state_mask))
                {
                    record->direction = (s8)(direction_or_animation & 0xE0);
                    direction_or_animation = (u8)*vertical_entry;
                    old_animation = record->animation;
                    record->animation_frame = 0;
                    record->active = 1;
                    direction_or_animation |= old_animation & 0x80;
                    record->animation = direction_or_animation;
                    func_8006C3FC(record);
                }
            }
            else
            {
                sector_index = sector_or_flags;
                current_animation = record->animation;
                if (((current_animation != D_800EB0C4[sector_index]) && (delta_z == 0)) ||
                    ((current_animation != D_800EB0E4[sector_index]) && (delta_z != 0)))
                {
                    record->direction = (s8)(direction_or_animation & 0xE0);
                    if (delta_z != 0)
                    {
                        record->animation = D_800EB0E4[direction_or_animation >> 5];
                    }
                    else
                    {
                        record->animation = D_800EB0C4[direction_or_animation >> 5];
                    }
                    record->animation_frame = 0;
                    record->active = 1;
                    func_8006C3FC(record);
                }
            }
            goto keep_moving;
        }
        idle_delay = record->stop_delay;
        if (idle_delay != 0)
        {
            record->stop_delay = (u8)(idle_delay - 1);
        }
        idle_state = record->animation;
        idle_state &= 0x7F;
        if (((idle_state >= 2) && (record->stop_delay == 0)) ||
            ((idle_state < 2) && (record->idle_mode == 0)))
        {
            s32 random_value;

            delta_z = record->animation;
            delta_z &= 0x80;
            random_value = rand();
            refresh_record = record;
            delta_z += random_value >= 0x6001;
            refresh_record->animation = delta_z;
            refresh_record->animation_frame = 0;
            refresh_record->idle_mode = 1U;
            refresh_record->active = 1;
            func_8006C3FC(refresh_record);
        }
    }
    else
    {
        if ((delta_x | delta_z) != 0)
        {
            movement_direction = ratan2(-delta_z, delta_x);
            movement_direction >>= 4;
            movement_direction += 0x10;
            if (movement_direction < 0)
            {
                movement_direction += 0x100;
            }
            if (movement_direction >= 0x100)
            {
                movement_direction -= 0x100;
            }
            direction_offset = movement_direction >> 5;
            direction_table_address = (s32)D_800EB0A4;
            direction_offset *= 4;
            direction_entry = (s32 *)(direction_table_address + direction_offset);
            if (record->animation != (*direction_entry + ((record->movement_flags & 1) * 5) + 5))
            {
                record->direction = (s8)(movement_direction & 0xE0);
                direction_or_animation = (u8)*direction_entry;
                sector_or_flags = record->movement_flags;
                record->animation_frame = 0;
                record->active = 1;
                direction_or_animation += (sector_or_flags & 1) * 5;
                direction_or_animation += 5;
                record->animation = (u8)direction_or_animation;
                func_8006C3FC(record);
            }
        keep_moving:
            record->stop_delay = 3U;
            record->active = 1;
            return;
        }
        movement_delay = record->stop_delay;
        if (movement_delay != 0)
        {
            record->stop_delay = (u8)(movement_delay - 1);
        }
        previous_animation = record->animation;
        low_state = previous_animation;
        low_state &= 0x7F;
        if ((low_state >= 5) && (record->stop_delay == 0))
        {
            refresh_record = record;
            refresh_record->active = 1;
            refresh_record->animation_frame = 0;
            refresh_record->animation = (u8)((low_state % 5) | (previous_animation & 0x80));
            func_8006C3FC(refresh_record);
        }
    }
}

/**
 * @brief Field actor per-frame state machine dispatch (opcodes 0x81..).
 *
 * Reads the actor's current opcode at offset 0x2A (biased by 0x81) and dispatches
 * to the matching movement, animation, path-following, resource-load, or transition
 * handler. Covers the large secondary opcode block for a field actor's active state.
 *
 * @param arg0 Pointer to the field actor state record (0x54-byte layout).
 * @return The actor's updated dispatch result (unused by most callers).
 * @see decomp.me (100%)
 */
s32 func_8008EF0C(void* arg0)
{
    typedef struct Func8008EF0CScratch
    {
        s32 sp18;
        u8 pad1C[0x2C];
        u16 sp48;
        u8 pad4A[2];
        u16 sp4C;
        s16 sp4E;
        s32 sp50;
        s32 sp54;
        s32 sp58;
        u8 pad5C[4];
        s32 sp60;
        s32 sp64;
        u8 tail68[8];
    } Func8008EF0CScratch;

    typedef struct Func8008Slot
    {
        u8 pad000[0x48];
        s16 unk48;
        u8 pad04A[0x16F - 0x4A];
        u8 unk16F;
        u8 pad170[0x172 - 0x170];
        u16 unk172;
        u32 unk174;
        union
        {
            s32 word;
            struct
            {
                u8 b178, b179, b17A, b17B;
            } b;
        } u178;
        u8 pad17C[4];
        u8 unk180[0x23C - 0x180];
    } Func8008Slot;

    typedef struct Func8008FDEntry
    {
        u8 unk0, unk1, unk2, unk3;
        u8 pad004[0x257 - 4];
        u8 unk257;
        u8 pad258[0x268 - 0x258];
    } Func8008FDEntry;

    typedef struct Func8008ResourceEntry
    {
        u8* start;
        u8* end;
        u8 unk8;
        u8 slot_index;
        u8 padA[4];
        s16 unkE;
        u32 flags;
    } Func8008ResourceEntry;

    extern u8 D_800EB074;
    extern u8 D_800EB114;
    extern u8 D_800FD818;
    extern u8 D_800FDF58;
    extern u8 D_80105880;
    extern u8 D_80105AE0;
    extern u8 D_8010A038;
    extern u8 D_8010A090;
    extern s32 D_8010AE54;
    extern u8 g_field_actor_slots;
    extern u8 g_field_resource_entries;

    void field_stop_actor_animations_for_object(void* record, s32 force);

    Func8008EF0CScratch scratch;
    s16 temp_v1;
    s32* var_a1_3;
    s32 temp_a0_3;
    s32 temp_a0_6;
    s32 temp_a1_2;
    s32 temp_a1_3;
    s32 temp_s0;
    s32 temp_s0_2;
    s32 temp_s0_3;
    s32 temp_s0_4;
    s32 temp_s0_5;
    s32 temp_s0_6;
    s32 temp_s0_7;
    s32 temp_s0_8;
    s32 temp_s0_9;
    s32 var_a1_2;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_s0;
    s32 var_s0_2;
    s32 var_v0_10;
    s32 var_v0_12;
    s32 var_v0_13;
    s32 var_v0_14;
    s32 var_v0_15;
    s32 var_v0_16;
    s32 var_v0_17;
    s32 var_v0_18;
    s32 var_v0_19;
    s32 var_v0_20;
    s32 var_v0_21;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v0_9;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    u16* var_s3;
    s32 temp_v0_16;
    s32 temp_v0_18;
    s32 temp_v0_20;
    u16 temp_v1_12;
    u32 temp_a1_5;
    u8 temp_a0_10;
    u8 temp_a0_12;
    s32 temp_a0_13;
    s32 temp_a0_14;
    u8 temp_a0_5;
    u8 temp_a1_4;
    u8 temp_v1_13;
    s32 temp_v1_4;
    u8 temp_v1_6;
    u8 var_v0_6;
    void* temp_a0;
    void* base37;
    void* base5fd;
    void* base058801;
    void* base058802;
    void* base058803;
    void* actors_base;
    void* stop_actors;
    void* slot_base1;
    void* slot_base2;
    void* slot_base3;
    void* fd_base;
    void* temp_a0_11;
    void* fd_counter_base;
    void* failure_base;
    void* temp_a0_4;
    void* temp_a0_9;
    void* temp_s2;
    void* temp_v0_11;
    void* temp_v0_12;
    void* temp_v0_13;
    void* temp_v0_14;
    void* temp_v0_17;
    void* temp_v0_19;
    void* temp_v0_21;
    void* temp_v0_26;
    void* temp_v0_28;
    void* temp_v0_29;
    void* temp_v0_31;
    void* temp_v0_32;
    void* temp_v0_33;
    void* temp_v0_34;
    void* temp_v0_35;
    void* temp_v0_36;
    void* temp_v0_37;
    void* temp_v0_38;
    void* temp_v0_7;
    void* temp_v1_11;
    void* temp_v1_3;
    void* temp_v1_8;

    var_s3 = (u16*)0x801ED480;
    temp_s2 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + &D_80105AE0;
    temp_v1 = M2C_FIELD(arg0, u16*, 0x2A) - 0x81;
    switch (temp_v1)
    {
    case 0x27:
        if (M2C_FIELD(arg0, u16*, 0x2E) == 0)
        {
            M2C_FIELD(arg0, u16*, 0x2A) = 0U;
        }
        scratch.sp50 = 0;
        scratch.sp54 = 0;
        scratch.sp58 = 0;
        func_80097FA0(arg0, &scratch.sp50, 0);
        return;
    case 0x36:
        if (M2C_FIELD(arg0, u16*, 0x2E) == 0)
        {
            if ((M2C_FIELD(arg0, u8*, 0x20) == 0) || (--M2C_FIELD(arg0, u8*, 0x20) == 0))
            {
                M2C_FIELD(arg0, u16*, 0x2A) = 0U;
            }
        }
        return;
    case 0x32:
    block_229:
        M2C_FIELD(arg0, u16*, 0x2A) = 0U;
        return;
    case 0x3A:
        if ((M2C_FIELD(arg0, u8*, 0x3D) != 0) && (--M2C_FIELD(arg0, u8*, 0x3D) != 0))
        {
            return;
        }
        if (field_object_has_active_actor_tracks(M2C_FIELD(arg0, u8*, 0x3A)) != 0)
        {
            return;
        }
        goto block_set_ff;
    case 0x37:
        if ((M2C_FIELD(arg0, u8*, 0x3D) != 0) && (--M2C_FIELD(arg0, u8*, 0x3D) != 0))
        {
            goto block_37_record;
        }
        if (field_object_has_active_actor_tracks(M2C_FIELD(arg0, u8*, 0x3A)) == 0)
        {
            goto block_37_cleanup;
        }
    block_37_record:
        base37 = &D_800FDF58;
        temp_a0 = (M2C_FIELD(arg0, u8*, 0x20) * 0x54) + base37;
        if (M2C_FIELD(temp_a0, u8*, 0x25) == 0xFF)
        {
        block_37_cleanup:
            field_stop_actor_animations_for_object(arg0, 0);
        block_set_ff:
            M2C_FIELD(arg0, u8*, 0x25) = 0xFF;
            return;
        }
        M2C_FIELD(arg0, s32*, 0) = M2C_FIELD(temp_a0, s32*, 0);
        M2C_FIELD(arg0, s32*, 4) = M2C_FIELD(((M2C_FIELD(arg0, u8*, 0x20) * 0x54) + base37), s32*, 4);
        M2C_FIELD(arg0, s32*, 8) = M2C_FIELD(((M2C_FIELD(arg0, u8*, 0x20) * 0x54) + base37), s32*, 8);
        return;
    case 0x5:
        if (M2C_FIELD(arg0, u8*, 0x3A) < 2U)
        {
            base5fd = &D_800FD818;
            temp_v1_3 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x268) + base5fd;
            if ((M2C_FIELD(temp_v1_3, u8*, 3) == 0) && (M2C_FIELD(temp_v1_3, u8*, 0x257) != 0))
            {
                if ((func_80091728(M2C_FIELD(arg0, u8*, 0x3A), 0U, arg0) != 0) && (func_80091728(M2C_FIELD(arg0, u8*, 0x3A), 1U, arg0) != 0))
                {
                    temp_a0_4 = &D_8010A038;
                    temp_v0_7 = (M2C_FIELD(arg0, u8*, 0x3B) * 0x190) + temp_a0_4;
                    var_s0 = func_800AD7DC(M2C_FIELD(temp_v0_7, u16*, 0), M2C_FIELD(temp_v0_7, u16*, 8));
                    temp_a0_3 = var_s0 * 2;
                    if (var_s0 != 0xFF)
                    {
                        scratch.sp48 = var_s0;
                        scratch.sp4C = (u16) * (temp_a0_3 + &D_800EB114);
                        scratch.sp4E = (s16)M2C_FIELD((&D_800EB114 + temp_a0_3), u8*, 1);
                        switch (var_s0)
                        {
                        case 0x34:
                        case 0x3E:
                        case 0x45:
                        case 0x4E:
                        case 0x4F:
                        case 0x50:
                        case 0x51:
                            SLOT23C(M2C_FIELD(arg0, u8*, 0x3A)).unk16F = (u8)var_s0;
                            break;
                        }
                        if (scratch.sp4C != 0xFFFF)
                        {
                            if (scratch.sp4C != 0)
                            {
                                var_s0 = func_800839F8(M2C_FIELD(arg0, u8*, 0x3A), 0);
                                if (var_s0 != -1)
                                {
                                    if (func_80083EEC(M2C_FIELD(arg0, u8*, 0x3A), var_s0, scratch.sp4C) != 0)
                                    {
                                        field_start_actor_animation(var_s0, 0, 0);
                                        SLOT23C(M2C_FIELD(arg0, u8*, 0x3A)).u178.b.b179 = (u8)var_s0;
                                    }
                                }
                            }
                        }
                        func_80090D48(arg0, temp_s2, &scratch.sp48);
                        FD268(M2C_FIELD(arg0, u8*, 0x3A)).unk257 = 0;
                        var_s0 = 0;
                        func_800A2DD8(M2C_FIELD(arg0, u8*, 0x3A));
                        func_8008BC5C(arg0);
                        temp_v1_4 = M2C_FIELD(arg0, u8*, 0x3A);
                        if (SLOT23C(temp_v1_4).u178.b.b17B != 0)
                        {
                            do
                            {
                                temp_v1_4 = SLOT23C(temp_v1_4).unk180[var_s0];
                                SLOT23C(temp_v1_4).u178.word &= ~0x80;
                                temp_v1_4 = M2C_FIELD(arg0, u8*, 0x3A);
                                var_s0 += 1;
                            } while (var_s0 < SLOT23C(temp_v1_4).u178.b.b17B);
                        }
                        SLOT23C(M2C_FIELD(arg0, u8*, 0x3A)).u178.b.b17B = 0;
                        return;
                    }
                }
                fd_counter_base = &D_800FD818;
                temp_v0_11 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x268) + fd_counter_base;
                M2C_FIELD(temp_v0_11, u8*, 0x257) = (u8)(M2C_FIELD(temp_v0_11, u8*, 0x257) - 1);
                goto block_42;
            }
        }
        goto block_42;
    case 0x15:
        func_800925EC(arg0, 0);
        return;
    case 0x17:
    case 0x1A:
    block_42:
        func_800925EC(arg0, 1);
        return;
    case 0x6:
        func_80093EB4(arg0);
        return;
    case 0x10:
        func_8009403C(arg0, M2C_FIELD(temp_s2, u16*, 0x172));
        return;
    case 0x31:
        if (!(RESOURCE14(M2C_FIELD(arg0, u8*, 0x3B)).flags & 1))
        {
            temp_v1_6 = *(M2C_FIELD(arg0, volatile u8*, 0x20) + &D_800EB074);
            M2C_FIELD(arg0, u8*, 0x20) = (u8)(M2C_FIELD(arg0, volatile u8*, 0x20) + 1);
            M2C_FIELD(arg0, u8*, 0x21) = temp_v1_6;
            if (*(M2C_FIELD(arg0, u8*, 0x20) + &D_800EB074) == 0xFF)
            {
                M2C_FIELD(arg0, u16*, 0x2A) = 0U;
                M2C_FIELD(arg0, u8*, 0x20) = 0U;
            }
        }
        else
        {
            M2C_FIELD(arg0, u16*, 0x2A) = 0U;
            M2C_FIELD(arg0, u8*, 0x20) = 0U;
            M2C_FIELD(arg0, u8*, 0x21) = (u8)(M2C_FIELD(arg0, u8*, 0x21) ^ 0x80);
        }
        M2C_FIELD(arg0, s16*, 0x2E) = 1;
        M2C_FIELD(arg0, s8*, 0x27) = 0;
        M2C_FIELD(arg0, s8*, 0x24) = 1;
        func_8006C3FC(arg0);
        return;
    case 0x19:
        func_80092550(arg0);
        return;
    case 0x18:
        func_800924D8(arg0);
        return;
    case 0x1:
        func_800923F0(arg0);
        return;
    case 0xD:
    {
        u8 index_d;
        void* entry_d;
        s16 limit_d;
        void* base_d;
        base_d = &D_800FD818;
        index_d = M2C_FIELD(arg0, u8*, 0x3A);
        entry_d = (index_d * 0x268) + base_d;
        limit_d = M2C_FIELD(entry_d, s16*, 0x260);
        if (limit_d != 0)
        {
            if (index_d < 3U)
            {
                if (((M2C_FIELD(entry_d, s16*, 0x25E) >= limit_d) || (M2C_FIELD(entry_d, s16*, 0x25E) = (s16)((u16)M2C_FIELD(entry_d, s16*, 0x25E) + 1),
                                                                      temp_v0_12 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x268) + &D_800FD818,
                                                                      ((M2C_FIELD(temp_v0_12, s16*, 0x25E) < M2C_FIELD(temp_v0_12, s16*, 0x260)) == 0))))
                {
                    temp_v0_13 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x268) + &D_800FD818;
                    M2C_FIELD(temp_v0_13, s16*, 0x260) = 0;
                    M2C_FIELD(temp_v0_13, s16*, 0x25E) = 0;
                    temp_a0_5 = M2C_FIELD(arg0, u8*, 0x3A);
                    temp_v0_14 = (temp_a0_5 * 0x268) + &D_800FD818;
                    func_80089D44(temp_a0_5, M2C_FIELD(temp_v0_14, s16*, 0x262), M2C_FIELD(temp_v0_14, s16*, 0x264), M2C_FIELD(temp_v0_14, s16*, 0x266));
                    return;
                }
            }
        }
        break;
    }
    case 0xF:
        func_80095168(arg0);
        return;
    case 0x11:
        temp_a0 = &D_80105880;
        if ((u8)M2C_FIELD(arg0, u8*, 0x3A) < 2U)
        {
            var_v0_2 = M2C_FIELD(arg0, u8*, 0x3A) * 0x1C;
        }
        else
        {
            var_v0_2 = 0x38;
        }
        if (M2C_FIELD((temp_a0 + var_v0_2), s32*, 0) != 0)
        {
            temp_a0 = &D_80105880;
            if ((u8)M2C_FIELD(arg0, u8*, 0x3A) < 2U)
            {
                var_v0_3 = M2C_FIELD(arg0, u8*, 0x3A) * 0x1C;
            }
            else
            {
                var_v0_3 = 0x38;
            }
            if (M2C_FIELD((temp_a0 + var_v0_3), s32*, 0xC) == M2C_FIELD(arg0, u8*, 0x3A))
            {
                func_80094EA4(arg0);
                return;
            }
        }
        goto block_78;
    case 0x12:
        temp_a0 = &D_80105880;
        if ((u8)M2C_FIELD(arg0, u8*, 0x3A) < 2U)
        {
            var_v0_4 = M2C_FIELD(arg0, u8*, 0x3A) * 0x1C;
        }
        else
        {
            var_v0_4 = 0x38;
        }
        if (M2C_FIELD((temp_a0 + var_v0_4), s32*, 0) != 0)
        {
            temp_a0 = &D_80105880;
            if ((u8)M2C_FIELD(arg0, u8*, 0x3A) < 2U)
            {
                var_v0_5 = M2C_FIELD(arg0, u8*, 0x3A) * 0x1C;
            }
            else
            {
                var_v0_5 = 0x38;
            }
            if (M2C_FIELD((temp_a0 + var_v0_5), s32*, 0xC) == M2C_FIELD(arg0, u8*, 0x3A))
            {
                func_80094F40(arg0);
                return;
            }
        }
    block_78:
        func_8008404C(M2C_FIELD(arg0, u8*, 0x3A), RESOURCE14(M2C_FIELD(arg0, u8*, 0x3B)).unkE & 0x3FF);
        return;
    case 0x13:
        func_80094FDC(arg0);
        return;
    case 0x2D:
        var_v0_6 = M2C_FIELD(arg0, u8*, 0x20);
        if (var_v0_6 != 0)
        {
            var_v0_6--;
            goto block_165;
        }
        if (!(M2C_FIELD(temp_s2, s32*, 0x178) & 1))
        {
            func_8008C104(arg0);
            return;
        }
        break;
    case 0x0:
        var_a2 = 0;
        func_80094508(arg0, 0, var_a2, 0);
        return;
    case 0xC:
        temp_s0 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
        func_800951CC(arg0, temp_s0, 0, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
        return;
    case 0xE:
        temp_s0_2 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
        func_800949CC(arg0, temp_s0_2, 0, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
        return;
    case 0x2E:
        func_800946FC(arg0);
        return;
    case 0x2F:
        var_v0_7 = M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3)), s32*, 0x1AC) - M2C_FIELD(arg0, s32*, 0);
        if (var_v0_7 < 0)
        {
            var_v0_7 += 0xFF;
        }
        scratch.sp50 = var_v0_7 >> 8;
        var_v0_8 = M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3)), s32*, 0x1B0) - M2C_FIELD(arg0, s32*, 8);
        if (var_v0_8 < 0)
        {
            var_v0_8 += 0xFF;
        }
        scratch.sp54 = var_v0_8 >> 8;
        gte_ldlvl((VECTOR*)&scratch.sp50);
        gte_sqr0();
        gte_stlvnl((VECTOR*)&scratch.sp60);
        if ((scratch.sp60 + scratch.sp64) < 0x32)
        {
            M2C_FIELD(arg0, s32*, 0) = (s32)M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3)), s32*, 0x1AC);
            M2C_FIELD(arg0, s32*, 8) = (s32)M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3)), s32*, 0x1B0);
            temp_v0_16 = M2C_FIELD(temp_s2, u16*, 0x1A6) + 1;
            if (M2C_FIELD(temp_s2, u16*, 0x1A4) != temp_v0_16)
            {
                M2C_FIELD(temp_s2, u16*, 0x1A6) = temp_v0_16;
                return;
            }
            goto block_229;
        }
        temp_v0_17 = temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3);
        var_v0_9 = ratan2(-M2C_FIELD(temp_v0_17, s32*, 0x1B0) + M2C_FIELD(arg0, s32*, 8), M2C_FIELD(temp_v0_17, s32*, 0x1AC) - M2C_FIELD(arg0, s32*, 0)) >> 4;
        M2C_FIELD(arg0, u8*, 0x1B) = (u8)var_v0_9;
        temp_s0_8 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
        func_80094508(arg0, temp_s0_8, 0, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
        return;
    case 0x7:
        temp_s0_3 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
        func_80094508(arg0, temp_s0_3, 0, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
        return;
    case 0x30:
        temp_v1_8 = temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3);
        var_v0_10 = M2C_FIELD(temp_v1_8, s32*, 0x1B0) - M2C_FIELD(arg0, s32*, 8);
        temp_a0_6 = M2C_FIELD(temp_v1_8, s32*, 0x1AC);
        var_v0_10 = abs(var_v0_10);
        var_v1 = temp_a0_6 - M2C_FIELD(arg0, s32*, 0);
        var_v1 = abs(var_v1);
        if ((var_v0_10 + var_v1) < 0x2000)
        {
            M2C_FIELD(arg0, s32*, 0) = temp_a0_6;
            M2C_FIELD(arg0, s32*, 8) = (s32)M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3)), s32*, 0x1B0);
            temp_v0_18 = M2C_FIELD(temp_s2, u16*, 0x1A6) + 1;
            if (M2C_FIELD(temp_s2, u16*, 0x1A4) != temp_v0_18)
            {
                M2C_FIELD(temp_s2, u16*, 0x1A6) = temp_v0_18;
                return;
            }
            goto block_128;
        }
        M2C_FIELD(arg0, s8*, 0x33) = 1;
        temp_v0_19 = temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3);
        M2C_FIELD(arg0, u8*, 0x1B) =
            (u8)(ratan2(-M2C_FIELD(temp_v0_19, s32*, 0x1B0) + M2C_FIELD(arg0, s32*, 8), M2C_FIELD(temp_v0_19, s32*, 0x1AC) - M2C_FIELD(arg0, s32*, 0)) >> 4);
        temp_s0_8 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
        func_80094508(arg0, temp_s0_8, 0, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
        if (M2C_FIELD(arg0, s16*, 0x2A) == 0)
        {
            M2C_FIELD(arg0, s8*, 0x33) = 0;
            return;
        }
        break;
    case 0x2C:
        temp_s0_4 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
        func_80094508(arg0, temp_s0_4, 0, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
        if (M2C_FIELD(arg0, s16*, 0x2A) == 0)
        {
            M2C_FIELD(arg0, s8*, 0x33) = 0;
            return;
        }
        break;
    case 0x34:
        var_v0_12 = M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3)), s32*, 0x1AC) - M2C_FIELD(arg0, s32*, 0);
        if (var_v0_12 < 0)
        {
            var_v0_12 += 0xFF;
        }
        scratch.sp50 = var_v0_12 >> 8;
        var_v0_13 = M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3)), s32*, 0x1B0) - M2C_FIELD(arg0, s32*, 8);
        if (var_v0_13 < 0)
        {
            var_v0_13 += 0xFF;
        }
        scratch.sp54 = var_v0_13 >> 8;
        gte_ldlvl((VECTOR*)&scratch.sp50);
        gte_sqr0();
        gte_stlvnl((VECTOR*)&scratch.sp60);
        if ((scratch.sp60 + scratch.sp64) < 0x32)
        {
            M2C_FIELD(arg0, s32*, 0) = (s32)M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3)), s32*, 0x1AC);
            M2C_FIELD(arg0, s32*, 8) = (s32)M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3)), s32*, 0x1B0);
            temp_v0_20 = M2C_FIELD(temp_s2, u16*, 0x1A6) + 1;
            if (M2C_FIELD(temp_s2, u16*, 0x1A4) == temp_v0_20)
            {
                M2C_FIELD(arg0, u16*, 0x2A) = 0U;
            }
            else
            {
                M2C_FIELD(temp_s2, u16*, 0x1A6) = temp_v0_20;
            }
        }
        else
        {
            temp_v0_21 = temp_s2 + (M2C_FIELD(temp_s2, u16*, 0x1A6) << 3);
            M2C_FIELD(arg0, u8*, 0x1B) =
                (u8)(ratan2(-M2C_FIELD(temp_v0_21, s32*, 0x1B0) + M2C_FIELD(arg0, s32*, 8), M2C_FIELD(temp_v0_21, s32*, 0x1AC) - M2C_FIELD(arg0, s32*, 0)) >>
                     4);
            temp_s0_5 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
            func_80094508(arg0, temp_s0_5, 0, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
        }
        {
            s32 negative_b;
            s32 current_b;
            s32 raw_b;
            raw_b = M2C_FIELD(var_s3, s32*, 4);
            current_b = M2C_FIELD(arg0, s32*, 0);
            negative_b = -raw_b;
            if ((negative_b + 0xA00) < current_b)
            {
                if (current_b < (negative_b + 0x13600))
                {
                    raw_b = M2C_FIELD(var_s3, s32*, 0xC);
                    current_b = M2C_FIELD(arg0, s32*, 8);
                    negative_b = -raw_b;
                    if ((negative_b + 0xA00) < current_b)
                    {
                        if (current_b < (negative_b + 0x1B600))
                        {
                            M2C_FIELD(arg0, u16*, 0x2A) = 0U;
                            return;
                        }
                    }
                }
            }
        }
        break;
    case 0xA:
        var_v0_14 = M2C_FIELD(temp_s2, s32*, 0x58) - M2C_FIELD(arg0, s32*, 8);
        var_v0_14 = abs(var_v0_14);
        temp_a1_2 = M2C_FIELD(temp_s2, s32*, 0x50) - M2C_FIELD(arg0, s32*, 0);
        var_v1_2 = abs(temp_a1_2);
        if ((var_v0_14 + var_v1_2) >= 0x1000)
        {
            var_v0_9 = ratan2(M2C_FIELD(arg0, s32*, 8) - M2C_FIELD(temp_s2, s32*, 0x58), temp_a1_2) >> 4;
            M2C_FIELD(arg0, u8*, 0x1B) = (u8)var_v0_9;
            temp_s0_8 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
            func_80094508(arg0, temp_s0_8, 0, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
            return;
        }
        goto block_229;
    case 0x2B:
        var_v0_15 = M2C_FIELD(temp_s2, s32*, 0x58) - M2C_FIELD(arg0, s32*, 8);
        var_v0_15 = abs(var_v0_15);
        var_v1_3 = M2C_FIELD(temp_s2, s32*, 0x50) - M2C_FIELD(arg0, s32*, 0);
        var_v1_3 = abs(var_v1_3);
        if ((var_v0_15 + var_v1_3) < 0x1000)
        {
        block_128:
            M2C_FIELD(arg0, u16*, 0x2A) = 0U;
            M2C_FIELD(arg0, s8*, 0x33) = 0;
            return;
        }
        M2C_FIELD(arg0, s8*, 0x33) = 1;
        {
            s32 target_z = M2C_FIELD(temp_s2, s32*, 0x58);
            s32 current_z = M2C_FIELD(arg0, s32*, 8);
            s32 target_x = M2C_FIELD(temp_s2, s32*, 0x50);
            s32 current_x = M2C_FIELD(arg0, s32*, 0);
            M2C_FIELD(arg0, u8*, 0x1B) = (u8)(ratan2(current_z - target_z, target_x - current_x) >> 4);
        }
        temp_s0_8 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
        func_80094508(arg0, temp_s0_8, 0, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
        if (M2C_FIELD(arg0, s16*, 0x2A) == 0)
        {
            M2C_FIELD(arg0, s8*, 0x33) = 0;
            return;
        }
        break;
    case 0xB:
    {
        s32 target_z;
        s32 current_z;
        s32 target_x;
        s32 current_x;
        target_z = M2C_FIELD(temp_s2, s32*, 0x58);
        current_z = M2C_FIELD(arg0, s32*, 8);
        target_x = M2C_FIELD(temp_s2, s32*, 0x50);
        current_x = M2C_FIELD(arg0, s32*, 0);
        var_v0_9 = (ratan2(current_z - target_z, target_x - current_x) >> 4) - 0x80;
        M2C_FIELD(arg0, u8*, 0x1B) = (u8)var_v0_9;
        temp_s0_8 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
        func_80094508(arg0, temp_s0_8, 0, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
        return;
    }
    case 0x8:
        func_80094508(arg0, 0, -0x100, 0);
        return;
    case 0x9:
        func_80094508(arg0, 0, 0x100, 0);
        return;
    case 0x1B:
        func_80094B5C(arg0, 1);
        return;
    case 0x1C:
        func_80094B5C(arg0, 0);
        return;
    case 0x1F:
        var_v0_16 = M2C_FIELD(temp_s2, s32*, 0x58) - M2C_FIELD(arg0, s32*, 8);
        var_v0_16 = abs(var_v0_16);
        temp_a1_3 = M2C_FIELD(temp_s2, s32*, 0x50) - M2C_FIELD(arg0, s32*, 0);
        var_v1_4 = abs(temp_a1_3);
        if ((var_v0_16 + var_v1_4) >= 0x2000)
        {
            M2C_FIELD(arg0, u8*, 0x1B) = (u8)(ratan2(M2C_FIELD(arg0, s32*, 8) - M2C_FIELD(temp_s2, s32*, 0x58), temp_a1_3) >> 4);
            temp_s0_6 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
            func_80094BC4(arg0, temp_s0_6, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
            return;
        }
        goto block_229;
    case 0x26:
        temp_s0_7 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
        func_80094C00(arg0, temp_s0_7, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
        if ((M2C_FIELD(temp_s2, u8*, 0x171) == 0) || (--M2C_FIELD(temp_s2, u8*, 0x171) == 0))
        {
            M2C_FIELD(arg0, u16*, 0x2A) = 0U;
        }
        return;
    case 0x35:
        temp_s0_8 = rcos(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4;
        func_80094690(arg0, temp_s0_8, (s32)-rsin(M2C_FIELD(arg0, u8*, 0x1B) * 0x10) >> 4);
        if ((M2C_FIELD(temp_s2, u8*, 0x171) == 0) || (--M2C_FIELD(temp_s2, u8*, 0x171) == 0))
        {
            M2C_FIELD(arg0, u16*, 0x2A) = 0U;
        }
        return;
    case 0x3B:
        actors_base = &g_field_actor_slots;
        base058801 = &D_80105880;
        if ((u8)M2C_FIELD(arg0, u8*, 0x3A) < 2U)
        {
            var_v0_17 = M2C_FIELD(arg0, u8*, 0x3A) * 0x1C;
        }
        else
        {
            var_v0_17 = 0x38;
        }
        if (M2C_FIELD((actors_base - (-(M2C_FIELD((base058801 + var_v0_17), s32*, 0x18) * 0x244))), u8*, 0x24) == 0)
        {
            M2C_FIELD(arg0, u16*, 0x2A) = 0U;
            return;
        }
        break;
    case 0x3C:
        if (M2C_FIELD(arg0, u16*, 0x2E) == 0)
        {
            temp_a1_4 = M2C_FIELD(arg0, u8*, 0x21);
            if ((temp_a1_4 & 0x7F) == 0xF)
            {
                slot_base1 = &D_80105AE0;
                M2C_FIELD(arg0, u8*, 0x21) = (u8)((temp_a1_4 & 0x80) + 0x31);
                M2C_FIELD(arg0, u16*, 0x2E) = 1U;
                M2C_FIELD(arg0, s8*, 0x27) = 0;
                M2C_FIELD(arg0, s8*, 0x24) = 1;
                temp_v0_26 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + slot_base1;
                M2C_FIELD(temp_v0_26, s32*, 0x174) = (s32)(M2C_FIELD(temp_v0_26, s32*, 0x174) & ~0x1800);
                func_8006C3FC(arg0);
                return;
            }
            goto block_229;
        }
        break;
    case 0x23:
        if (M2C_FIELD(arg0, u16*, 0x2E) == 0)
        {
            M2C_FIELD(arg0, u16*, 0x2A) = 0U;
            M2C_FIELD(arg0, u16*, 0x2E) = 1U;
            M2C_FIELD(arg0, s8*, 0x24) = 1;
            M2C_FIELD(arg0, u8*, 0x21) = (u8)(M2C_FIELD(arg0, u8*, 0x21) & 0x80);
            func_8006C3FC(arg0);
            return;
        }
        break;
    case 0x14:
        var_v0_6 = M2C_FIELD(arg0, u8*, 0x20);
        if (var_v0_6 != 0)
        {
            var_v0_6--;
            goto block_165;
        }
        func_80096334(arg0);
        M2C_FIELD(arg0, u16*, 0x2A) = 0U;
        func_800A2DD8(M2C_FIELD(arg0, u8*, 0x3A));
        return;
    block_165:
        M2C_FIELD(arg0, u8*, 0x20) = var_v0_6;
        return;
    case 0x4:
    {
        void* base4;
        base4 = &D_80105AE0;
        M2C_FIELD(((M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + base4), s8*, 0x17B) = 0;
        if ((u8)M2C_FIELD(arg0, u8*, 0x3A) < 3U)
        {
            fd_base = &D_800FD818;
            M2C_FIELD(((M2C_FIELD(arg0, u8*, 0x3A) * 0x268) + fd_base), s8*, 0x257) = 0;
        }
        if (M2C_FIELD(arg0, s32*, 0x1C) & 0x1FF)
        {
            s32 row4 = M2C_FIELD(arg0, u8*, 0x3B) * 0x190;
            var_s3 = (void*)(row4 - -(s32)((M2C_FIELD(((M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + base4), u8*, 0x16F) * 8) + &D_8010A038));
        }
        else
        {
            temp_a0_9 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + base4;
            if ((u8)M2C_FIELD(temp_a0_9, u8*, 0x16F) < 0xCU)
            {
                {
                    s32 row4 = M2C_FIELD(arg0, u8*, 0x3B) * 0x190;
                    var_s3 = (void*)(row4 - -(s32)((M2C_FIELD(temp_a0_9, u8*, 0x16F) * 8) + &D_8010A038));
                }
            }
            else
            {
                var_s3 = (M2C_FIELD(arg0, u8*, 0x3B) * 0x190) + &D_8010A090;
            }
        }
    }
        if (M2C_FIELD(var_s3, u8*, 2) != 0xFF)
        {
            s32 result4;
            temp_s0_9 = func_8009D1E4(M2C_FIELD(arg0, u8*, 0x3A), var_s3, (M2C_FIELD(var_s3, u16*, 2) >> 8) & 3, M2C_FIELD(temp_s2, u16*, 0x174) & 0x3FF,
                                      &scratch.sp18);
            result4 = 0;
            if (D_8010AE54 == 0)
            {
                if (!(M2C_FIELD(arg0, s32*, 0x1C) & 0x1FF))
                {
                    void* check_base = &D_80105AE0;
                    temp_a0_10 = M2C_FIELD(arg0, u8*, 0x3A);
                    temp_v1_11 = (temp_a0_10 * 0x23C) + check_base;
                    if ((u8)M2C_FIELD(temp_v1_11, u8*, 0x16F) < 0xCU)
                    {
                        result4 = func_80091728(temp_a0_10, M2C_FIELD(temp_v1_11, u8*, 0x16F), arg0);
                    }
                }
                else
                {
                    result4 = func_80090F50(arg0);
                }
            }
            if (result4 != 0)
            {
                temp_s0_9 = (s32)&D_80105AE0;
                temp_v0_28 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + (void*)temp_s0_9;
                M2C_FIELD(temp_v0_28, s32*, 0x178) = (s32)(M2C_FIELD(temp_v0_28, s32*, 0x178) | 0x40);
                func_8009D9E0(arg0, M2C_FIELD(var_s3, u8*, 2));
                if ((M2C_FIELD(var_s3, u16*, 2) & 0x400) && (M2C_FIELD(arg0, u16*, 0x2E) == 0))
                {
                    M2C_FIELD(arg0, u16*, 0x2E) = 1U;
                    M2C_FIELD(arg0, s8*, 0x24) = 1;
                    M2C_FIELD(arg0, s8*, 0x27) = 0;
                    M2C_FIELD(arg0, u8*, 0x21) = (u8)((M2C_FIELD(arg0, u8*, 0x21) & 0x80) + 0xE);
                    temp_v0_29 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + (void*)temp_s0_9;
                    M2C_FIELD(temp_v0_29, s32*, 0x174) = (s32)(M2C_FIELD(temp_v0_29, s32*, 0x174) & ~0x1800);
                    func_8006C3FC(arg0);
                }
                temp_s2 = &D_80105AE0;
                temp_a0_11 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + temp_s2;
                temp_a1_5 = M2C_FIELD(temp_a0_11, u32*, 0x174);
                if (!((temp_a1_5 >> 0xA) & 1) && ((u16)M2C_FIELD(temp_a0_11, u16*, 0x4A) >= 0x41U))
                {
                    temp_v1_12 = M2C_FIELD(var_s3, u16*, 4);
                    if ((temp_v1_12 != 0xFFFF) && (temp_v1_12 != 0))
                    {
                        M2C_FIELD(temp_a0_11, u32*, 0x174) = (u32)(temp_a1_5 | 0x400);
                        temp_s0_9 = func_800839F8(M2C_FIELD(arg0, u8*, 0x3A), 0);
                        if ((temp_s0_9 != -1) && (func_80083EEC(M2C_FIELD(arg0, u8*, 0x3A), temp_s0_9, M2C_FIELD(var_s3, u16*, 4)) != 0))
                        {
                            field_start_actor_animation(temp_s0_9, 0, 0);
                            M2C_FIELD(((M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + temp_s2), s8*, 0x179) = temp_s0_9;
                            temp_a0_12 = M2C_FIELD(arg0, u8*, 0x3A);
                            func_800A623C(temp_a0_12, (0x80000000 | (temp_a0_12 << 0x10)) | (M2C_FIELD(((temp_a0_12 * 0x23C) + temp_s2), u8*, 0x16F) - 4));
                            return;
                        }
                    }
                }
            }
            else
            {
                temp_v0_31 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + &D_80105AE0;
                M2C_FIELD(temp_v0_31, s32*, 0x178) = (s32)(M2C_FIELD(temp_v0_31, s32*, 0x178) & ~0x40);
                temp_v1_13 = M2C_FIELD(arg0, u8*, 0x3A);
                if (!(((u32)SLOT23C(temp_v1_13).unk174 >> 0xA) & 1))
                {
                    stop_actors = &g_field_actor_slots;
                    base058802 = &D_80105880;
                    if (temp_v1_13 < 2U)
                    {
                        var_v0_18 = temp_v1_13 * 0x1C;
                    }
                    else
                    {
                        var_v0_18 = 0x38;
                    }
                    M2C_FIELD((stop_actors - (-(M2C_FIELD((base058802 + var_v0_18), s32*, 0x18) * 0x244))), s8*, 0x24) = 0;
                    func_80084424(M2C_FIELD(arg0, u8*, 0x3A));
                    slot_base2 = &D_80105AE0;
                    M2C_FIELD(arg0, u16*, 0x2E) = 1U;
                    M2C_FIELD(arg0, s8*, 0x24) = 1;
                    M2C_FIELD(arg0, s8*, 0x27) = 0;
                    M2C_FIELD(arg0, u8*, 0x21) = (u8)((M2C_FIELD(arg0, u8*, 0x21) & 0x80) + 0xF);
                    temp_v0_32 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + slot_base2;
                    M2C_FIELD(temp_v0_32, s32*, 0x174) = (s32)(M2C_FIELD(temp_v0_32, s32*, 0x174) & ~0x1800);
                    func_8006C3FC(arg0);
                    M2C_FIELD(arg0, u16*, 0x2A) = 0xBDU;
                    return;
                }
                base058803 = &D_80105880;
                if (temp_v1_13 < 2U)
                {
                    var_v0_19 = temp_v1_13 * 0x1C;
                }
                else
                {
                    var_v0_19 = 0x38;
                }
                var_s0_2 = func_80090B38(temp_s0_9, &scratch.sp18, M2C_FIELD((base058803 + var_v0_19), s32*, 0x18));
                actors_base = &g_field_actor_slots;
                base058801 = &D_80105880;
                if ((u8)M2C_FIELD(arg0, u8*, 0x3A) < 2U)
                {
                    var_v0_20 = M2C_FIELD(arg0, u8*, 0x3A) * 0x1C;
                }
                else
                {
                    var_v0_20 = 0x38;
                }
                temp_v0_33 = actors_base + (M2C_FIELD((base058801 + var_v0_20), s32*, 0x18) * 0x244);
                M2C_FIELD(temp_v0_33, s32*, 0x224) = (s32)((M2C_FIELD(temp_v0_33, s32*, 0x224) & ~0x1E) | ((M2C_FIELD(var_s3, u8*, 2) & 0xF) * 2));
                if (!(M2C_FIELD(var_s3, u16*, 6) & 0x800))
                {
                    if (func_80090F80(M2C_FIELD(arg0, u8*, 0x3A), 0, NULL, M2C_FIELD(var_s3, u16*, 6)) != 0)
                    {
                        goto block_217;
                    }
                    else
                    {
                        goto block_212;
                    }
                }
                if (var_s0_2 != 0)
                {
                    var_a1_2 = var_s0_2;
                    if (var_s0_2 >= 0xA)
                    {
                        var_s0_2 = 9;
                        var_a1_2 = 9;
                    }
                    if (func_80090F80(M2C_FIELD(arg0, u8*, 0x3A), var_a1_2, &scratch.sp18, M2C_FIELD(var_s3, u16*, 6)) != 0)
                    {
                        var_a2_2 = 0;
                        if (var_s0_2 > 0)
                        {
                            do
                            {
                                var_a1_3 = &scratch.sp18 + var_a2_2;
                                temp_v0_34 = (*var_a1_3 * 0x23C) + &D_80105AE0;
                                M2C_FIELD(temp_v0_34, s32*, 0x178) = (s32)(M2C_FIELD(temp_v0_34, s32*, 0x178) | 0x80);
                                temp_v0_35 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + &D_80105AE0;
                                M2C_FIELD((temp_v0_35 + M2C_FIELD(temp_v0_35, u8*, 0x17B)), u8*, 0x180) = (u8)*var_a1_3;
                                var_a2_2 += 1;
                                temp_v0_36 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + &D_80105AE0;
                                M2C_FIELD(temp_v0_36, u8*, 0x17B) = (u8)(M2C_FIELD(temp_v0_36, u8*, 0x17B) + 1);
                            } while (var_a2_2 < var_s0_2);
                        }
                        goto block_217;
                    }
                block_212:
                    failure_base = &D_80105AE0;
                    temp_v0_37 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + failure_base;
                    M2C_FIELD(temp_v0_37, s32*, 0x178) = (s32)(M2C_FIELD(temp_v0_37, s32*, 0x178) | 0x40);
                    return;
                }
                actors_base = &g_field_actor_slots;
                base058801 = &D_80105880;
                if ((u8)M2C_FIELD(arg0, u8*, 0x3A) < 2U)
                {
                    var_v0_21 = M2C_FIELD(arg0, u8*, 0x3A) * 0x1C;
                }
                else
                {
                    var_v0_21 = 0x38;
                }
                M2C_FIELD((actors_base - (-(M2C_FIELD((base058801 + var_v0_21), s32*, 0x18) * 0x244))), s8*, 0x24) = 0;
                func_80084424(M2C_FIELD(arg0, u8*, 0x3A));
                SLOT23C(M2C_FIELD(arg0, u8*, 0x3A)).u178.b.b179 = 0xFF;
                goto block_217;
            }
        }
        else
        {
        block_217:
            if (M2C_FIELD(var_s3, u16*, 2) & 0x400)
            {
                slot_base3 = &D_80105AE0;
                M2C_FIELD(arg0, u16*, 0x2A) = 0x87U;
                M2C_FIELD(arg0, u16*, 0x2E) = 1U;
                M2C_FIELD(arg0, s8*, 0x24) = 1;
                M2C_FIELD(arg0, s8*, 0x27) = 0;
                M2C_FIELD(arg0, u8*, 0x21) = (u8)((M2C_FIELD(arg0, u8*, 0x21) & 0x80) + 0xF);
                temp_v0_38 = (M2C_FIELD(arg0, u8*, 0x3A) * 0x23C) + slot_base3;
                M2C_FIELD(temp_v0_38, s32*, 0x174) = (s32)(M2C_FIELD(temp_v0_38, s32*, 0x174) & ~0x1800);
                func_8006C3FC(arg0);
                M2C_FIELD(arg0, s32*, 0x1C) |= 0x800;
                return;
            }
            if (M2C_FIELD(var_s3, u16*, 0) & 0x8000)
            {
                SLOT23C(M2C_FIELD(arg0, u8*, 0x3A)).unk48 = 0;
                M2C_FIELD(temp_s2, s32*, 0x44) = 0;
                M2C_FIELD(temp_s2, s32*, 0x40) = (s32)M2C_FIELD(var_s3, u16*, 6);
                M2C_FIELD(temp_s2, s32*, 0x3C) = (s32)M2C_FIELD(var_s3, u16*, 4);
                M2C_FIELD(temp_s2, u32*, 0x174) = (s32)((s32)M2C_FIELD(temp_s2, u32*, 0x174) & ~0x1800);
                M2C_FIELD(temp_s2, s32*, 0x40) =
                    (s32)((((M2C_FIELD(var_s3, u16*, 0) & 0x7FFF) + 0x88) | 0x8000) + (FD268(M2C_FIELD(arg0, u8*, 0x3A)).unk1 * 0x18));
                if ((u8)M2C_FIELD(arg0, u8*, 0x3A) < 3U)
                {
                    temp_a0_13 = M2C_FIELD(arg0, u8*, 0x3A);
                    func_800A623C(temp_a0_13, (M2C_FIELD(var_s3, u16*, 0) & 0x7FFF) + (FD268(temp_a0_13).unk1 * 0x18));
                }
                func_800954F0(arg0, M2C_FIELD(var_s3, u16*, 0) & 0x7FFF);
                M2C_FIELD(temp_s2, u16*, 0x172) = (u16)(M2C_FIELD(var_s3, u16*, 0) & 0x7FFF);
                M2C_FIELD(arg0, u16*, 0x2A) = 0x91U;
                func_8006C3FC(arg0);
                M2C_FIELD(arg0, s32*, 0x1C) = (s32)(M2C_FIELD(arg0, s32*, 0x1C) | 0x800);
                return;
            }
            if ((u8)M2C_FIELD(arg0, u8*, 0x3A) < 3U)
            {
                temp_a0_14 = M2C_FIELD(arg0, u8*, 0x3A);
                if ((u8)SLOT23C(temp_a0_14).unk16F < 2U)
                {
                    FD268(temp_a0_14).unk257 = 0xA;
                }
            }
            func_80090D48(arg0, temp_s2, var_s3);
            return;
        }
        break;
    default:
        scratch.sp50 = 0;
        scratch.sp54 = 0;
        scratch.sp58 = 0;
        func_80097FA0(arg0, &scratch.sp50, 0);
        M2C_FIELD(arg0, u16*, 0x2A) = 0U;
        return;
    }
}

/**
 * @brief Compact eligible actor indices into the front of the supplied list.
 * @param arg0 Number of indices to inspect.
 * @param arg1 Input indices and destination for the eligible indices.
 * @return Number of eligible indices copied.
 * @note The original routine reserves scratch space for 16 eligible entries.
 */
s32 func_80090B38(s32 arg0, s32 *arg1)
{
    /** @brief Animation entry containing eligibility state and actor slot. */
    typedef struct
    {
        u8 pad0[0x25];
        u8 unk25;
        u8 pad26[4];
        s16 unk2A;
        u8 pad2C[14];
        u8 unk3A;
        u8 pad3B[0x54 - 0x3B];
    } Entry;
    /** @brief Actor record containing resources, group, and eligibility flags. */
    typedef struct
    {
        s32 unk0;
        s32 unk4;
        s32 unk8;
        s32 unkC;
        s32 unk10;
        u8 pad14[0x12C - 0x14];
        s32 unk12C;
        u8 pad130[0x174 - 0x130];
        s32 unk174;
        s32 unk178;
        u8 pad17C[0x23C - 0x17C];
    } Actor;

    extern Entry D_800FDF58[];
    extern Actor D_80105AE0[];
    extern s32 D_800FE754;
    extern u8 D_80105880[];

    Entry *entry_base;
    Actor *actor_base;
    u8 *slot_base;
    s32 group;
    s32 sentinel;
    s32 scratch[16];
    s16 temp_v1_2;
    s32 *var_a1;
    s32 *var_t2;
    s32 *var_t4;
    s32 *var_v1;
    s32 temp_a2_2;
    s32 slot;
    s32 temp_t0;
    s32 temp_v0;
    s32 temp_v1;
    s32 temp_v1_3;
    s32 var_t1;
    s32 var_t1_2;
    s32 var_t3;
    s32 var_v0;
    s32 var_v0_2;
    Entry *temp_a2;
    Actor *temp_a3;

    var_a1 = arg1;
    var_t1 = 0;
    var_t3 = var_t1;
    if (arg0 > 0)
    {
        sentinel = 0xFF;
        entry_base = D_800FDF58;
        actor_base = D_80105AE0;
        group = D_800FE754;
        slot_base = D_80105880;
        var_t2 = var_a1;
        var_t4 = scratch;
        do
        {
            temp_v1 = *var_t2;
            if (temp_v1 != sentinel)
            {
                temp_a2 = (Entry *)(temp_v1 * 0x54 + (s32)entry_base);
                temp_a3 = (Actor *)(temp_v1 * 0x23C + (s32)actor_base);
                if ((temp_a2->unk25 != sentinel) && (temp_a3->unk4 != 0))
                {
                    temp_t0 = temp_a3->unk178;
                    if (!(temp_t0 & 1) && ((var_t1 < 3) || ((temp_a3->unk10 & 0xF) == group)))
                    {
                        temp_v1_2 = temp_a2->unk2A;
                        if ((temp_v1_2 != 0x91) && (temp_v1_2 != 0xAE) && (temp_v1_2 != 0x87))
                        {
                            if (!(temp_t0 & 0x40))
                            {
                                if ((u8)temp_a2->unk3A < 2U)
                                {
                                    var_v0 = temp_a2->unk3A * 0x1C;
                                }
                                else
                                {
                                    var_v0 = 0x38;
                                }
                                slot = temp_a2->unk3A;
                                temp_a2 = (Entry *)*(s32 *)(slot_base + var_v0 + 0xC);
                                temp_a2_2 = (s32)temp_a2;
                                if (temp_a2_2 == slot)
                                {
                                    if ((u32)(temp_a2_2 & 0xFF) < 2U)
                                    {
                                        var_v0_2 = temp_a2_2 * 0x1C;
                                    }
                                    else
                                    {
                                        var_v0_2 = 0x38;
                                    }
                                    if (*(s32 *)(slot_base + var_v0_2) == 0)
                                    {
                                        goto block_20;
                                    }
                                }
                                else
                                {
                                    goto block_20;
                                }
                            }
                            else
                            {
                            block_20:
                                if ((temp_a3->unk12C != 0) && !(temp_a3->unkC & 0x2280))
                                {
                                    temp_v1_3 = temp_a3->unk178;
                                    if (!(temp_v1_3 & 0x20) && !((u8)temp_v1_3 & 0x80) &&
                                        !(temp_a3->unk174 & 0x8000))
                                    {
                                        var_t3 += 1;
                                        *var_t4 = *var_t2;
                                        var_t4++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            var_t1 += 1;
            var_t2++;
        } while (var_t1 < arg0);
    }
    var_t1 = 0;
    if (var_t3 > 0)
    {
        var_v1 = scratch;
        do
        {
            temp_v0 = *var_v1;
            var_v1++;
            var_t1 += 1;
            *var_a1 = temp_v0;
            var_a1++;
        } while (var_t1 < var_t3);
    }
    return var_t3;
}

/** @brief Animation entry with state, frame counters, variant, and actor slot (func_80090D48). */
typedef struct
{
    u8 pad0[0x1C];
    s32 unk1C;
    u8 pad20;
    u8 unk21;
    u8 pad22[2];
    u8 unk24;
    u8 pad25[2];
    u8 unk27;
    u8 pad28[2];
    u16 unk2A;
    u8 pad2C[2];
    u16 unk2E;
    union
    {
        u16 half;
        u8 byte[2];
    } variant;
    u8 pad32[8];
    u8 unk3A;
} D48Entry;
/** @brief Actor fields affected by animation commands (func_80090D48). */
typedef struct
{
    u8 pad0[12];
    s32 unkC;
    u8 pad10[4];
    s32 unk14;
    u8 pad18[0x3C - 0x18];
    s32 unk3C;
    u8 pad40[8];
    u16 unk48;
    u8 pad4A[0x174 - 0x4A];
    s32 unk174;
} D48Actor;
/** @brief Animation command code and argument (func_80090D48). */
typedef struct
{
    union
    {
        u16 half;
        u8 byte[2];
    } code;
    u8 pad2[4];
    u16 unk6;
} D48Command;

/**
 * @brief Apply an animation command and its actor and sound side effects.
 * @param arg0 Animation entry to update.
 * @param arg1 Actor receiving the command.
 * @param arg2 Animation command and argument.
 * @return Unspecified.
 */
s32 func_80090D48(D48Entry *arg0, D48Actor *arg1, D48Command *arg2)
{
    /** @brief Actor slot record containing the type byte. */
    typedef struct
    {
        u8 pad0;
        u8 unk1;
        u8 pad2[0x268 - 2];
    } ActorData;

    extern ActorData D_800FD818[];
    extern u8 D_800EB1B8[];
    extern void func_8006C3FC(D48Entry *);
    extern void func_800A3938(s32, s32);
    extern void func_800B61C4(s32);

    s32 animation_id;
    s32 masked_command_code;
    s32 command_code;
    u16 unk48_value;
    u16 next_unk48_value;

    arg1->unk3C = (s32)arg2->unk6;
    command_code = arg2->code.half;
    if (((u32)(command_code - 0x2F) < 2U) || (masked_command_code = command_code & 0xFFFF, (masked_command_code == 0x44)) ||
        (masked_command_code == 0x45))
    {
        arg1->unkC = (s32)(arg1->unkC | 0x4000);
    }
    if ((arg2->code.half == 0x1F) && (arg0->variant.half != 0) &&
        ((arg0->unk3A >= 2U) || (D_800FD818[arg0->unk3A].unk1 != 0xA)))
    {
        arg0->unk21 = (u8)(arg0->variant.byte[0] + (arg2->code.byte[0] + (arg0->unk21 & 0x80)));
    }
    else
    {
        arg0->unk21 = (u8)(arg2->code.byte[0] + (arg0->unk21 & 0x80));
    }
    arg0->unk2E = 1;
    arg0->unk27 = 0;
    arg0->unk24 = 1;
    arg1->unk174 = (s32)(arg1->unk174 & ~0x1800);
    func_8006C3FC(arg0);
    animation_id = arg0->unk21 & 0x7F;
    switch (animation_id)
    {
    case 0x41:
        func_800B61C4(arg1->unk14);
        break;
    case 0x33:
        unk48_value = arg1->unk48;
        next_unk48_value = unk48_value < 0xE0U ? unk48_value + 0x20 : 0xFF;
        arg1->unk48 = next_unk48_value;
        break;
    case 0x8:
    case 0xA:
    case 0x31:
    case 0x3D:
        arg0->unk2A = 0x86;
        arg0->unk1C = (s32)(arg0->unk1C | 0x800);
        goto play_sound;
    default:
        break;
    }
    arg0->unk2A = 0x86;
play_sound:
    if (((u8)arg0->unk3A < 2U) && (D_800EB1B8[arg0->unk21 & 0x7F] != 0xFF))
    {
        func_800A3938(D_800EB1B8[arg0->unk21 & 0x7F], 0x80);
    }
}

/** @brief Actor record whose slot index selects a slot (func_80090F50). */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A; /* 0x3A */
} Struct_D800FDF58;

/**
 * @brief Report whether an actor slot's height field is still below the sentinel.
 * @param rec Actor record whose slot index selects the slot to inspect.
 * @return Non-zero when the slot height halfword is below 0xFF.
 */
s32 func_80090F50(Struct_D800FDF58 *rec)
{
    typedef struct
    {
        u8 pad0[0x4A];
        u16 unk4A; /* 0x4A */
        u8 pad4C[0x23C - 0x4C];
    } Struct_D80105AE0;

    extern Struct_D80105AE0 D_80105AE0[];

    return D_80105AE0[rec->unk3A].unk4A < 0xFF;
}

/**
 * @brief Start an actor animation, or defer to the layered variant for banked flags.
 * @param arg0 Actor owning the pending slot request.
 * @param arg1 Number of animation targets.
 * @param arg2 Animation target identifiers.
 * @param arg3 Layer-selection flags; other bit meanings are unknown.
 * @return One when the request was applied.
 */
s32 func_80090F80(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    typedef struct {
        u8 pad0[0x179];
        s8 unk179;
        u8 pad17A[0x23C - 0x17A];
    } Rec23C;

    extern Rec23C D_80105AE0[];

    s32 func_800839F8(s32 arg0, s32 arg1);
    s32 func_80083EEC(s32 arg0, s32 arg1, s32 arg2);
    void field_start_actor_animation(s32 arg0, s32 arg1, s32 arg2);
    s32 func_8009104C(void);

    s32 v;

    if (arg3 & 0x8000)
        return func_8009104C();

    v = func_800839F8(arg0, 0);
    if ((v != -1) && (func_80083EEC(arg0, v, arg3 & 0x3FF) != 0))
    {
        field_start_actor_animation(v, arg1, arg2);
        D_80105AE0[arg0].unk179 = v;
    }
    return 1;
}

/**
 * @brief Start the requested actor animation and initialize any extra layers.
 * @param actor_id Actor owning the pending slot request.
 * @param target_count Number of animation targets.
 * @param targets Animation target identifiers.
 * @param flags Layer-selection flags; other bit meanings are unknown.
 * @return One when the request was applied, or zero when it was not ready.
 * @note GCC 2.7.2 CDK currently matches 98.983406 percent of the target.
 */
s32 func_8009104C(s32 actor_id, s32 target_count, u8 *targets, s32 flags)
{
    /** @brief Animation definition view with the original 0x1C-byte stride. */
    typedef struct
    {
        u8 pad0[0x12];
        u16 unk12;
        u8 pad14[2];
        u8 unk16;
        u8 pad17[5];
    } AnimationDef;

    /** @brief Actor slot view with the original 0x244-byte stride. */
    typedef struct
    {
        u8 pad0[0xC];
        AnimationDef *unkc;
        AnimationDef *unk10;
        u8 pad14[0x10];
        u8 unk24;
        u8 pad25[4];
        u8 unk29;
        u8 unk2a;
        u8 pad2b[0x222 - 0x2B];
        u16 unk222;
        u8 pad224[0xF];
        u8 unk233;
        u8 pad234[4];
        s16 unk238;
        u8 pad23a[0xA];
    } ActorSlot;

    /** @brief Pending actor-slot request, indexed with the original 0x1C-byte stride. */
    typedef struct
    {
        s32 unk0;
        u8 pad4[8];
        s32 unkc;
        u8 pad10[8];
        s32 unk18;
    } SlotRequest;

    /** @brief Actor state view containing the active slot identifier. */
    typedef struct
    {
        u8 pad0[0x179];
        u8 unk179;
        u8 pad17a[0x23C - 0x17A];
    } ActorState;

    void bcopy(void *, void *, s32);
    void field_start_actor_animation(s32, s32, u8 *);
    s32 func_800839F8(s32, s32);
    extern SlotRequest D_80105880[];
    extern ActorState D_80105AE0[];
    extern ActorSlot g_field_actor_slots[];

    u16 frame_value;
    ActorSlot *slot_base;
    s32 remaining_layers;
    s32 layer_offset;
    s32 result;
    s32 request_index;
    s32 new_slot_id;
    s32 layer_index;
    ActorSlot *slot;
    SlotRequest *request;
    ActorSlot *base_slot;
    AnimationDef *animation;
    ActorSlot *default_slot;
    ActorSlot *selected_slot;
    ActorSlot *layer_slot;
    ActorSlot *initialized_slot;

    request_index = actor_id;
    if (actor_id >= 3)
    {
        request_index = 2;
    }
    request = &D_80105880[request_index];
    if (request->unkc != actor_id)
    {
        return 0;
    }
    if (request->unk0 != 2)
    {
        return 0;
    }

    {
        if (flags & 0x4000)
        {
            if (!(flags & 0x400))
            {
                g_field_actor_slots[request->unk18].unk29 = (s8) ((flags >> 0xC) & 3);
                layer_slot = &g_field_actor_slots[request->unk18];
                layer_slot->unk222 = layer_slot->unk10[layer_slot->unk29].unk12;
            }
            else
            {
                g_field_actor_slots[request->unk18].unk29 = 0;
                base_slot = &g_field_actor_slots[request->unk18];
                remaining_layers = (flags >> 0xC) & 3;
                base_slot->unk222 = (u16) base_slot->unk10->unk12;
                layer_index = 1;
                g_field_actor_slots[request->unk18].unk2a = 1;
                if (remaining_layers != 0)
                {
                    slot_base = g_field_actor_slots;
                    layer_offset = 0x1C;
                    do
                    {
                        new_slot_id = func_800839F8(actor_id, 0);
                        if (new_slot_id != -1)
                        {
                            slot = &slot_base[new_slot_id];
                            bcopy(&slot_base[request->unk18], slot, 0x244);
                            animation = slot->unk10;
                            slot->unk233 = new_slot_id;
                            slot->unk29 = layer_index;
                            frame_value = animation->unk12;
                            slot->unkc = (AnimationDef *)((u8 *)animation + layer_offset);
                            slot->unk238 = 0;
                            result = 1;
                            slot->unk2a = result;
                            slot->unk24 = result;
                            slot->unk222 = frame_value;
                            field_start_actor_animation(new_slot_id, target_count, targets);
                        }
                        remaining_layers -= 1;
                        layer_offset += 0x1C;
                        layer_index += 1;
                    } while (remaining_layers != 0);
                }
            }
        }
        else
        {
            default_slot = &g_field_actor_slots[request->unk18];
            default_slot->unk222 = (u16) default_slot->unk10->unk12;
            g_field_actor_slots[request->unk18].unk29 = 0;
        }
        initialized_slot = &g_field_actor_slots[request->unk18];
        initialized_slot->unk238 = initialized_slot->unk10[initialized_slot->unk29].unk16;
        selected_slot = &g_field_actor_slots[request->unk18];
        selected_slot->unkc = selected_slot->unk10 + selected_slot->unk29;
        g_field_actor_slots[request->unk18].unk24 = 1;
        field_start_actor_animation(request->unk18, target_count, targets);
        D_80105AE0[actor_id].unk179 = (u8) request->unk18;
    }
    result = 1;
    return result;
}
