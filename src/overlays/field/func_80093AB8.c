#include "common.h"

/** @brief Actor fields used to track and reset pending action state. */
typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x1C - 8];
    s32 unk1C;
    u8 unk20, unk21;
    u8 pad22[8];
    u16 unk2A;
    u8 pad2C[4];
    u16 unk30;
    u8 pad32[8];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} Actor;
/** @brief Action flags and mode bytes within a 0x23C-byte object slot. */
typedef struct
{
    u8 pad0[12];
    s32 unkC;
    u8 pad10[0x16F - 0x10];
    u8 unk16F;
    u8 pad170[0x18D - 0x170];
    u8 unk18D;
    u8 pad18E[0x23C - 0x18E];
} Slot;
/** @brief Party record containing the action-type byte at offset one. */
typedef struct
{
    u8 unk0, unk1;
    u8 pad2[0x268 - 2];
} Party;
extern Slot D_80105AE0[];
extern Party D_800FD818[];
void func_8006C5FC(Actor *);
s32 func_8006C7D8(Actor *);
s32 func_800A29F8(s32, s32, s32);
void func_800A2DD8(s32);
/**
 * @brief Update pending actor actions and clear stale action state.
 * @param input Actor whose object slot and pending-action counter are checked.
 * @return One when the actor enters state 0x95; zero otherwise.
 */
s32 func_80093AB8(Actor *input)
{
    Actor *actor = input;
    s32 selection;
    s32 clear_mask;
    s32 flags;
    u16 retry_count;
    u16 count;
    u8 object_index;
    s32 mode;
    Slot *slot;
    Slot *base;
    Slot *reset_slot;

    if (actor->unk1C & 0x1FF)
    {
        D_80105AE0[actor->unk3A].unkC &= 0xFFFF7FFF;
        actor->unk30 = (u16)(actor->unk30 + 1);
        return 0;
    }
    selection = func_800A29F8(actor->unk3A, ((u8)actor->unk21 >> 7) ^ 1, 1);
    base = D_80105AE0;
    slot = &base[actor->unk3A];
    if (((slot->unk16F == 2) || (actor->unk30 != 0)) && (actor->unk4 == 0))
    {
        if (!(actor->unk1C & 0x1FF))
        {
            flags = slot->unkC;
            if (!(flags & 0x400))
            {
                slot->unkC = (s32)(flags | 0x8000);
            }
        }
        if (selection != 3)
        {
            if (selection < 4)
            {
                if (selection == 2)
                {
                    count = actor->unk30;
                    if (count < 5U)
                    {
                        actor->unk30 = (u16)(count + 1);
                    }
                    if (func_8006C7D8(actor) != 0)
                    {
                        retry_count = actor->unk30;
                        if (retry_count < 5U)
                        {
                            if ((u8)actor->unk3A < 2U)
                            {
                                if (D_800FD818[actor->unk3A].unk1 == 0xA)
                                {
                                    if (retry_count >= 3U)
                                    {
                                        goto cancel_pending;
                                    }
                                    goto check_mode;
                                }
                                goto check_mode;
                            }
                            goto check_mode;
                        }
                    }
                cancel_pending:
                    func_800A2DD8(actor->unk3A);
                    clear_mask = 0xFFFF7FFF;
                    actor->unk30 = 0U;
                    D_80105AE0[actor->unk3A].unk18D = 0;
                    reset_slot = &D_80105AE0[actor->unk3A];
                    goto reset_actor;
                }
                goto clear_pending_counter;
            }
        }
    clear_pending_counter:
        D_80105AE0[actor->unk3A].unk18D = 0;
        actor->unk30 = 0U;
    }
check_mode:
    object_index = actor->unk3A;
    mode = D_80105AE0[object_index].unk16F;
    if (mode == 3)
    {
        if (selection != 4 && selection != 6 && selection != 5 && selection != 7 &&
            selection != 8 && selection != 9 && selection != 10)
        {
            func_800A2DD8(object_index);
            clear_mask = 0xFFFF7FFF;
            D_80105AE0[actor->unk3A].unk18D = 0;
            actor->unk30 = 0;
            reset_slot = &D_80105AE0[actor->unk3A];
        reset_actor:
            flags = reset_slot->unkC;
            flags &= clear_mask;
            reset_slot->unkC = flags;
            func_8006C5FC(actor);
            actor->unk2A = 0x95;
            actor->unk20 = 0x14;
            return 1;
        }
    }
    else
    {
        if ((s32)mode >= 3)
        {
            if ((s32)mode < 11)
            {
                if ((s32)mode >= 8)
                {
                    if (selection != 4 && selection != 6 && selection != 5 && selection != 7)
                    {
                        func_800A2DD8(object_index);

                        D_80105AE0[actor->unk3A].unk18D = 0;
                        actor->unk30 = 0;
                        D_80105AE0[actor->unk3A].unkC &= 0xFFFF7FFF;
                    }
                }
            }
        }
    }
    return 0;
}
