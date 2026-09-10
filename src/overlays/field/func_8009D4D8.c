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
/** @brief Movement mode and signed destination coordinates in the runtime slot. */
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
 * @param actor Actor whose runtime movement fields are changed.
 * @param mode Movement mode selector; mode four generates three separated random points.
 * @note Random points are accepted only when at least 0x40 units from earlier points.
 * @note Signed fixed-point positions are divided by 256 with truncation toward zero.
 */
void func_8009D4D8(Actor *actor, u32 mode)
{
    s32 *delta = (s32 *)0x1F800000;
    s32 *squares = (s32 *)0x1F800010;
    Slot *base;
    s32 previous_offset;
    s32 point_offset;
    s32 temp_v0_5;
    s32 temp_v0_6;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1;
    s32 var_a1_2;
    s32 previous_index;
    s32 point_index;
    s32 retry;
    s32 flags;
    u8 temp_a0;
    Slot *temp_a2;
    Slot *temp_a2_2;
    Slot *temp_v0;
    Slot *temp_v0_2;
    Slot *temp_v0_3;
    Slot *temp_v0_4;
    Slot *clear_slot;

    D_80105AE0[actor->unk3A].unk5C = 0;
    switch (mode)
    {
    case 1:
        base = D_80105AE0;
        temp_v0 = &base[actor->unk3A];
        temp_v0->unk174 = (s32)((temp_v0->unk174 & ~0x3FF) | 0x40);
        return;
    case 2:
        base = D_80105AE0;
        temp_v0_2 = &base[actor->unk3A];
        temp_v0_2->unk174 = (s32)((temp_v0_2->unk174 & ~0x3FF) | 0x10);
        return;
    case 0:
    case 3:
        base = D_80105AE0;
        temp_v0_3 = &base[actor->unk3A];
        temp_v0_3->unk174 = (s32)((temp_v0_3->unk174 & ~0x3FF) | 0x1E);
        return;
    case 4:
        point_index = 0;
        base = D_80105AE0;
        temp_v0_4 = &base[actor->unk3A];
        temp_v0_4->unk174 = (s32)((temp_v0_4->unk174 & ~0x3FF) | 0x1E);
        retry = 1;
        do
        {
            point_offset = point_index * 4;
        loop_7:
            ((Slot *)(point_offset + (actor->unk3A * 0x23C) + (u8 *)base))->unk190 = (s16)((rand() >> 7) - 0x80);
            ((Slot *)(point_offset + (actor->unk3A * 0x23C) + (u8 *)base))->unk192 = (s16)((rand() >> 7) - 0x80);
            var_a1 = D_800F22A0;
            temp_a2 = (Slot *)(point_offset + (u8 *)&base[actor->unk3A]);

            temp_a2->unk190 = (u16)(((u16)temp_a2->unk190 - (var_a1 / 256)) - (actor->unk0 / 256));
            var_a1_2 = D_800F22A8;
            temp_a2_2 = (Slot *)(point_offset + (u8 *)&base[actor->unk3A]);

            temp_a2_2->unk192 = (u16)(((u16)temp_a2_2->unk192 - (var_a1_2 / 256)) - (actor->unk8 / 256));
            previous_index = 0;
            if (point_index > 0)
            {
            loop_16:
                previous_offset = previous_index * 4;
                temp_v0_5 = actor->unk3A * 0x23C;
                delta[0] = ((Slot *)(previous_offset + temp_v0_5 + (u8 *)base))->unk190 -
                           ((Slot *)(point_offset + temp_v0_5 + (u8 *)base))->unk190;
                temp_v0_6 = actor->unk3A * 0x23C;
                delta[1] = ((Slot *)(previous_offset + temp_v0_6 + (u8 *)base))->unk192 -
                           ((Slot *)(point_offset + temp_v0_6 + (u8 *)base))->unk192;
                delta[2] = 0;
                gte_ldlvl(delta);
                gte_sqr0();
                gte_stlvnl(squares);
                if (SquareRoot0(squares[0] + squares[1]) >= 0x40)
                {
                    previous_index += 1;
                    if (previous_index < point_index)
                    {
                        goto loop_16;
                    }
                }
            }
            if (previous_index == point_index)
            {
                retry = 0;
            }
            if (retry != 0)
            {
                goto loop_7;
            }
            point_index += 1;
            retry = 1;
        } while (point_index < 3);
        return;
    case 5:
        base = D_80105AE0;
        clear_slot = &base[actor->unk3A];
        flags = clear_slot->unk174 & ~0x3FF;
        goto block_25;
    case 6:
        base = D_80105AE0;
        clear_slot = &base[actor->unk3A];
        flags = (clear_slot->unk174 & ~0x3FF) | 0x1E;
    block_25:
        clear_slot->unk174 = flags;
        base[actor->unk3A].unk190 = 0;
        base[actor->unk3A].unk192 = 0;
        return;
    default:
        return;
    }
}
