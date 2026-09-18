/** @file field_actor_movement_modes.c
 * @brief Set an actor's movement mode and initialize its per-mode slot state.
 */

#include "common.h"
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
} Actor;

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
} Slot;

s32 rand(void); /* extern */
extern s32 D_800F22A0;
extern s32 D_800F22A8;
extern Slot D_80105AE0[];

/**
 * @brief Set an actor's movement mode and initialize its destination offsets.
 * @param actor Actor whose runtime movement fields are changed (index in unk3A).
 * @param mode Movement mode selector; mode 4 generates three separated random points.
 * @note Mode 4 accepts a new random point only when it is at least 0x40 units from
 *       every earlier point, using the GTE square/square-root path for the distance.
 * @note Signed fixed-point positions are divided by 256 with truncation toward zero.
 */
void func_8009D4D8(Actor *actor, u32 mode)
{
    s32 *delta = (s32 *)0x1F800000;
    s32 *squares = (s32 *)0x1F800010;
    Slot *base;
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
    Slot *point;
    Slot *point_y;
    Slot *slot;
    Slot *slot_2;
    Slot *slot_3;
    Slot *slot_4;
    Slot *slot_5;

    D_80105AE0[actor->unk3A].unk5C = 0;
    switch (mode)
    {
    case 1:
    {
        Slot *base;
        base = D_80105AE0;
        slot = &base[actor->unk3A];
        slot->unk174 = (s32)((slot->unk174 & ~0x3FF) | 0x40);
        return;
    }
    case 2:
    {
        Slot *base;
        base = D_80105AE0;
        slot_2 = &base[actor->unk3A];
        slot_2->unk174 = (s32)((slot_2->unk174 & ~0x3FF) | 0x10);
        return;
    }
    case 0:
    case 3:
    {
        Slot *base;
        base = D_80105AE0;
        slot_3 = &base[actor->unk3A];
        slot_3->unk174 = (s32)((slot_3->unk174 & ~0x3FF) | 0x1E);
        return;
    }
    case 4:
    {
        Slot *base;
        point_index = 0;
        slot_5 = D_80105AE0;
        base = slot_5;
        slot_4 = &base[actor->unk3A];
        slot_4->unk174 = (s32)((slot_4->unk174 & ~0x3FF) | 0x1E);
        do
        {
            needs_retry = 1;
            point_offset = point_index * 4;
        loop_7:
            {
                s32 random_value;
                s32 offset;
                Slot *point;
                random_value = (rand() >> 7) - 0x80;
                offset = point_offset + (actor->unk3A * 0x23C);
                point = (Slot *)(offset + (s32)base);
                point->unk190 = (s16)random_value;
            }
            {
                s32 random_value;
                s32 offset;
                Slot *point;
                random_value = (rand() >> 7) - 0x80;
                offset = point_offset + (actor->unk3A * 0x23C);
                point = (Slot *)(offset + (s32)base);
                point->unk192 = (s16)random_value;
            }
            base_dx = D_800F22A0;
            {
                s32 offset;
                offset = point_offset + (actor->unk3A * 0x23C);
                point = (Slot *)(offset + (s32)base);
            }

            point->unk190 = (u16)(((u16)point->unk190 - (base_dx / 256)) - (actor->unk0 / 256));
            base_dy = D_800F22A8;
            {
                s32 offset;
                offset = point_offset + (actor->unk3A * 0x23C);
                point_y = (Slot *)(offset + (s32)base);
            }

            point_y->unk192 = (u16)(((u16)point_y->unk192 - (base_dy / 256)) - (actor->unk8 / 256));
            compare_index = 0;
            if (point_index > 0)
            {
            loop_16:
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
                    delta[0] = ((Slot *)compare_addr)->unk190 - ((Slot *)current_offset)->unk190;
                }
                slot_stride_y = actor->unk3A * 0x23C;
                compare_offset += slot_stride_y;
                compare_offset += (s32)base;
                {
                    s32 current_offset;
                    current_offset = point_offset + (actor->unk3A * 0x23C);
                    current_offset += (s32)base;
                    delta[1] = ((Slot *)compare_offset)->unk192 - ((Slot *)current_offset)->unk192;
                }
                delta[2] = 0;
                do
                {
                    do
                    {
                        gte_ldlvl(delta);
                    } while (0);
                    gte_sqr0();
                } while (0);
                do
                {
                    gte_stlvnl(squares);
                } while (0);
                if (SquareRoot0(squares[0] + squares[1]) >= 0x40)
                {
                    compare_index += 1;
                    if (compare_index < point_index)
                    {
                        goto loop_16;
                    }
                }
            }
            if (compare_index == point_index)
            {
                needs_retry = 0;
            }
            if (needs_retry != 0)
            {
                goto loop_7;
            }
            point_index += 1;
        } while (point_index < 3);
        return;
    }
    case 5:
    {
        Slot *clear_base = D_80105AE0;
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
        Slot *clear_base = D_80105AE0;
        Slot *clear_slot = &clear_base[actor->unk3A];
        s32 clear_flags = clear_slot->unk174;
        s32 clear_mask = ~0x3FF;
        clear_flags &= clear_mask;
        clear_flags |= 0x1E;
        clear_slot->unk174 = clear_flags;
        clear_base[actor->unk3A].unk190 = 0;
        clear_base[actor->unk3A].unk192 = 0;
        return;
    }
    default:
        return;
    }
}
