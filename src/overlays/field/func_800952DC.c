#include "common.h"

/** @brief Field entry identifying the actor slot to update. */
typedef struct
{
    u8 pad[0x3A];
    u8 unk3A;
} Entry;
/** @brief Actor slot with activation and animation completion bytes. */
typedef struct
{
    u8 pad[0x24];
    u8 unk24;
    u8 pad25[5];
    u8 unk2A;
    u8 pad2B[0x244 - 0x2B];
} Actor;
extern u8 D_80105880[];
extern Actor g_field_actor_slots[];
extern s32 field_is_actor_animation_active(s32);
extern void func_80084424(u8);
/**
 * @brief Mark an active animation or release its associated actor slot.
 * @param arg0 Field entry whose actor slot is checked.
 * @param arg1 Whether to clear the slot activation byte before releasing it.
 */
void func_800952DC(Entry *arg0, s32 arg1)
{
    u8 *base;
    s32 slot;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;

    base = D_80105880;
    if ((u8)arg0->unk3A < 2U)
    {
        var_v0 = arg0->unk3A * 0x1C;
    }
    else
    {
        var_v0 = 0x38;
    }
    if (*(s32 *)(base + var_v0) != 0)
    {
        base = D_80105880;
        if ((u8)arg0->unk3A < 2U)
        {
            var_v0_2 = arg0->unk3A * 0x1C;
        }
        else
        {
            var_v0_2 = 0x38;
        }
        if (field_is_actor_animation_active(*(s32 *)(base + var_v0_2 + 0x18)) != 0)
        {
            base = D_80105880;
            if ((u8)arg0->unk3A < 2U)
            {
                var_v0_3 = arg0->unk3A * 0x1C;
            }
            else
            {
                var_v0_3 = 0x38;
            }
            if (*(s32 *)(base + var_v0_3 + 0xC) == arg0->unk3A)
            {
                Actor *actors;
                u8 *lookup;

                temp_a0 = *(s32 *)(base + var_v0_3 + 0xC);
                actors = g_field_actor_slots;
                lookup = D_80105880;
                if ((u32)(temp_a0 & 0xFF) < 2U)
                {
                    var_v0_4 = temp_a0 * 0x1C;
                }
                else
                {
                    var_v0_4 = 0x38;
                }
                {
                    u32 actor_address;
                    actor_address = (u32)actors;
                    actor_address += *(s32 *)(lookup + var_v0_4 + 0x18) * 0x244;
                    ((Actor *)actor_address)->unk2A = 1;
                }
            }
        }
        else
        {
            base = D_80105880;
            if ((u8)arg0->unk3A < 2U)
            {
                var_v0_5 = arg0->unk3A * 0x1C;
            }
            else
            {
                var_v0_5 = 0x38;
            }
            slot = arg0->unk3A;
            temp_a0_2 = *(s32 *)(base + var_v0_5 + 0xC);
            if (temp_a0_2 == slot)
            {
                if (arg1 != 0)
                {
                    Actor *actors;
                    u8 *lookup;

                    actors = g_field_actor_slots;
                    lookup = D_80105880;
                    if ((u32)(temp_a0_2 & 0xFF) < 2U)
                    {
                        var_v0_6 = temp_a0_2 * 0x1C;
                    }
                    else
                    {
                        var_v0_6 = 0x38;
                    }
                    {
                        u32 actor_address;
                        actor_address = (u32)actors;
                        actor_address += *(s32 *)(lookup + var_v0_6 + 0x18) * 0x244;
                        ((Actor *)actor_address)->unk24 = 0;
                    }
                }
                func_80084424(arg0->unk3A);
            }
        }
    }
}
