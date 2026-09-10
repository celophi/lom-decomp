#include "common.h"
#include "field_types.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
/** @brief Byte access in a partially recovered actor layout. */
#define U8_AT(p, o) (*(u8 *)((s32)(p) + (o)))
/** @brief Unsigned halfword access in an actor or part record. */
#define U16_AT(p, o) (*(u16 *)((s32)(p) + (o)))
/** @brief Signed halfword access in an actor state record. */
#define S16_AT(p, o) (*(s16 *)((s32)(p) + (o)))
/** @brief Word access in actor flags, positions, and scratch vectors. */
#define S32_AT(p, o) (*(s32 *)((s32)(p) + (o)))
s32 func_8007E754(void *, void *);
void func_8007ECEC(void *, void *, void *, s32);
s32 func_8008A840(s32, s32);
s32 func_8008A9D8(s32, s32, s32);
void func_8008BC5C(void *);
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

/**
 * @brief Find eligible actors intersecting an attack's collision spheres and apply reactions.
 * @param actor Attacking actor state, including its target list and attack mode.
 * @param part Actor part used to obtain the attack radius and sphere centers.
 * @note Eligibility checks retain repeated reads of actor flags and target counts.
 * @note Distances use the GTE square operation followed by SquareRoot0.
 */
void func_80099A48(void *actor, void *part)
{
    s32 *delta = (s32 *)0x1F800080;
    s32 *squares = (s32 *)0x1F800090;
    s32 target_end;
    s32 sphere_count;
    s32 attack_radius;
    s32 sphere_base;
    s32 checked_position_offset;
    s32 *checked_position;
    s32 *position_cursor;
    s32 position_offset;
    s16 target_state;
    s32 *target_position;
    s32 target_flags;
    s32 controlled_actor;
    s32 initial_position_offset;
    s32 eligible;
    s32 sphere_address;
    s32 sphere_index;
    s32 range_end;
    s32 controller_offset;
    s32 active_controller_offset;
    s32 existing_index;
    s32 hit_index;
    s32 target_index;
    u8 source_index;
    u8 existing_count;
    u8 hit_count;
    void *source_slot;
    void *append_slot;
    void *count_slot;
    void *target_flags_address;
    void *target_z_address;

    sphere_base = 0x1F800000;
    attack_radius = func_8007E754(actor, part);
    func_8007ECEC(actor, part, (void *)0x1F8000A0, 0);
    sphere_base |= 0xA0;
    if ((S32_AT(actor, 0x224) & 0x1E) == 8)
    {
        func_8007ECEC(actor, part, (void *)0x1F8000B0, 1);
        func_8007ECEC(actor, part, (void *)0x1F8000C0, 2);
        sphere_count = 3;
    }
    else
    {
        sphere_count = 1;
    }
    target_index = 0;
    if (D_8010D020 != 0)
    {
        range_end = 0xD;
        goto set_target_end;
    }
    if (U16_AT(S32_AT(actor, 0xC), 0x18) & 1)
    {
        target_index = 0;
        if ((u8)U8_AT(actor, 0x228) < 3U)
        {
            target_end = 3;
        }
        else
        {
            target_index = 3;
            range_end = 0xD;
            goto set_target_end;
        }
    }
    else
    {
        target_index = 3;
        if ((u8)U8_AT(actor, 0x228) < 3U)
        {
            target_end = 0xD;
        }
        else
        {
            target_index = 0;
            range_end = 3;
        set_target_end:
            target_end = range_end;
        }
    }
    initial_position_offset = target_index * 0x54;
    target_position = (s32 *)(initial_position_offset + (s32)D_800FDF58);
    target_flags_address = (void *)((target_index * 0x23C) + (s32)D_80105AE0);
    if (target_index < target_end)
    {
        target_flags_address += 0xC;
        target_z_address = (u8 *)target_position + 8;
        position_cursor = target_position;
        position_offset = initial_position_offset;
    next_target:
        if (S32_AT(target_flags_address, 0x16C) & 0x80)
        {
            source_index = U8_AT(actor, 0x228);
            eligible = 0;
            if (((FieldTargetState *)(D_800FDF58 + source_index * 0x54))->state == 0x91)
            {
                source_slot = (void *)((source_index * 0x23C) + (s32)D_80105AE0);
                existing_count = U8_AT(source_slot, 0x17B);
                existing_index = 0;
                if (existing_count != 0)
                {
                scan_existing_targets:
                    if (U8_AT(source_slot + existing_index, 0x180) != target_index)
                    {
                        existing_index += 1;
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
                ((target_index >= 2) || ((U8_AT(target_z_address, 0x19) & 0x7F) != 0x3C)) &&
                (U8_AT(target_z_address, 0x1D) != 0xFF) && (U8_AT(actor, 0x228) != target_index) &&
                (S32_AT(target_flags_address, -0x8) != 0))
            {
                target_flags = S32_AT(target_flags_address, 0x16C);
                if (!(target_flags & 1) &&
                    ((((target_index < 3) != 0)) ||
                     (((S32_AT(target_flags_address, 0x4) & 0xF) == D_800FE754))) &&
                    ((target_flags & 0x20) == 0) && (eligible != 0) &&
                    !(S32_AT(target_flags_address, 0x168) & 0x8000))
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
                        controlled_actor =
                            ((FieldTargetController *)(D_80105880 + controller_offset))->object;
                        if (controlled_actor == U8_AT(target_z_address, 0x32))
                        {
                            if ((u32)(controlled_actor & 0xFF) < 2U)
                            {
                                active_controller_offset = controlled_actor * 0x1C;
                            }
                            else
                            {
                                active_controller_offset = 0x38;
                            }
                            if (S32_AT(D_80105880, active_controller_offset) == 0)
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
                                        delta[0] = (s32)((s32)(*target_position -
                                                               S32_AT(sphere_address, 0x0)) >>
                                                         8);
                                        delta[1] = (s32)((s32)(S32_AT(target_z_address, -0x4) -
                                                               S32_AT(sphere_address, 0x4)) >>
                                                         8);
                                        delta[2] = (s32)((s32)(S32_AT(target_z_address, 0x0) -
                                                               S32_AT(sphere_address, 0x8)) >>
                                                         8);
                                        gte_ldlvl(delta);
                                        gte_sqr0();
                                        gte_stlvnl(squares);
                                        if ((SquareRoot0(squares[0] + squares[1] + squares[2]) <
                                             (attack_radius +
                                              ((s16)U16_AT(target_flags_address, 0x122) >> 1))) &&
                                            ((u8)((FieldActorTargetCount *)(D_80105AE0 +
                                                                            U8_AT(actor, 0x228) *
                                                                                0x23C))
                                                 ->count < 9U))
                                        {
                                            S32_AT(target_flags_address, 0x16C) =
                                                (s32)(S32_AT(target_flags_address, 0x16C) | 0x80);
                                            S32_AT(target_flags_address, 0x0) =
                                                (s32)(S32_AT(target_flags_address, 0x0) & ~0x400);
                                            append_slot = (void *)((U8_AT(actor, 0x228) * 0x23C) +
                                                                   (s32)D_80105AE0);
                                            U8_AT(append_slot, U8_AT(append_slot, 0x17B) + 0x180) =
                                                target_index;
                                            count_slot = (void *)((U8_AT(actor, 0x228) * 0x23C) +
                                                                  (s32)D_80105AE0);
                                            U8_AT(count_slot, 0x17B) =
                                                (u8)(U8_AT(count_slot, 0x17B) + 1);
                                            U8_AT(actor, 0x23A) = (u8)(U8_AT(actor, 0x23A) |
                                                                       (1 << U8_AT(actor, 0x232)));
                                            U8_AT(actor, U8_AT(actor, 0x232) + 0x229) =
                                                target_index;
                                            U8_AT(actor, 0x232) = (u8)(U8_AT(actor, 0x232) + 1);
                                            if ((target_index < 2) &&
                                                !(U16_AT(checked_position, 0x1C) & 0x1FF))
                                            {
                                                func_800A2DD8(target_index);
                                            }
                                            func_8008BC5C((void *)(checked_position_offset +
                                                                   (s32)D_800FDF58));
                                            if (((u8)U8_AT(actor, 0x26) < 0xCU) ||
                                                (((FieldTargetState *)(D_800FDF58 +
                                                                       U8_AT(actor, 0x228) * 0x54))
                                                     ->state == 0xBC))
                                            {
                                                do
                                                {
                                                    func_8008A9D8(U8_AT(actor, 0x228), target_index,
                                                                  U8_AT(actor, 0x26));
                                                } while (0);
                                                goto sphere_done;
                                            }
                                            else
                                            {
                                                switch (U8_AT(actor, 0x26))
                                                {
                                                case 0x50:
                                                    do
                                                    {
                                                        func_8008A9D8(U8_AT(actor, 0x228),
                                                                      target_index, 0x12U);
                                                    } while (0);
                                                    sphere_index += 1;
                                                    break;
                                                case 0x51:
                                                    do
                                                    {
                                                        func_8008A9D8(U8_AT(actor, 0x228),
                                                                      target_index, 0x13U);
                                                    } while (0);
                                                    sphere_index += 1;
                                                    break;
                                                case 0x4E:
                                                    do
                                                    {
                                                        func_8008A9D8(U8_AT(actor, 0x228),
                                                                      target_index, 0x14U);
                                                    } while (0);
                                                    sphere_index += 1;
                                                    break;
                                                case 0x4F:
                                                    do
                                                    {
                                                        func_8008A9D8(U8_AT(actor, 0x228),
                                                                      target_index, 0x15U);
                                                    } while (0);
                                                    sphere_index += 1;
                                                    break;
                                                case 0x3E:
                                                    do
                                                    {
                                                        func_8008A9D8(U8_AT(actor, 0x228),
                                                                      target_index, 0x19U);
                                                    } while (0);
                                                    sphere_index += 1;
                                                    break;
                                                case 0x45:
                                                    do
                                                    {
                                                        func_8008A9D8(U8_AT(actor, 0x228),
                                                                      target_index, 0x1AU);
                                                    } while (0);
                                                    sphere_index += 1;
                                                    break;
                                                default:
                                                    func_8008A840(U8_AT(actor, 0x228),
                                                                  target_index);
                                                    goto advance_sphere;
                                                }
                                            }
                                        }
                                        else
                                        {
                                        advance_sphere:
                                        sphere_done:
                                            sphere_index += 1;
                                        }
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
        target_position = (s32 *)((u8 *)target_position + 0x54);
        target_flags_address += 0x23C;
        position_cursor = (s32 *)((u8 *)position_cursor + 0x54);
        position_offset += 0x54;
        if (target_index < target_end)
        {
            goto next_target;
        }
    }
}
