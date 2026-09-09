#include "common.h"

/** @brief Resource binding with owner, resource ID, and actor-slot index. */
typedef struct
{
    s32 unk0, unk4, unk8, unkC;
    u8 pad10[8];
    s32 unk18;
} Binding;
/** @brief Actor slot with availability and initialization state. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25;
    u8 unk26;
    u8 pad27[0x224 - 0x27];
    union
    {
        s32 flags;
        u8 bytes[4];
    } state;
    u8 pad228[10];
    u8 unk232;
    u8 pad233[7];
    u8 unk23A, unk23B;
    u8 pad23C[8];
} Slot;
/** @brief Actor record containing the byte copied into a newly reserved slot. */
typedef struct
{
    u8 pad0[0x16F];
    u8 unk16F;
    u8 pad170[0x23C - 0x170];
} Actor;
extern Binding D_80105880[];
extern Slot g_field_actor_slots[];
extern Actor D_80105AE0[];
extern s32 func_800B0850(void);
extern s32 func_8009A364(s32);
/**
 * @brief Reserve an unused actor slot and initialize its resource binding.
 * @param actor_id Owner actor identifier; binding indices above two use binding two.
 * @param resource_id Resource identifier to load.
 * @return One on success, or zero if unavailable or loading fails.
 */
s32 func_8008404C(s32 actor_id, s32 resource_id)
{
    Binding *binding_base;
    Binding *search_base;
    Binding *binding_cursor;
    s32 binding_index;
    Slot *slot_cursor;
    s32 temp_a0;
    s32 load_id;
    s32 free_slot;
    s32 first_index;
    s32 second_index;
    s32 slot_index;
    s32 owner_index;
    Binding *binding;
    Slot *slot;
    Slot *slot_base;

    if (func_800B0850() == 0)
    {
        first_index = actor_id;
        binding_base = D_80105880;

        if (actor_id >= 3)
        {
            first_index = 2;
        }
        if (binding_base[first_index].unk0 == 0)
        {
            second_index = actor_id;
            if (actor_id >= 3)
            {
                second_index = 2;
            }
            free_slot = -1;
            if (binding_base[second_index].unk0 == 0)
            {
                search_base = binding_base;
                slot_index = 0;
                slot_cursor = g_field_actor_slots;
            loop_8:
                binding_index = 0;
                if (slot_cursor->unk24 == 0)
                {
                    binding_cursor = search_base;
                loop_10:
                    if ((binding_cursor->unk0 == 0) || (binding_cursor->unk18 != slot_index))
                    {
                        binding_index += 1;
                        binding_cursor++;
                        if ((s32)binding_index >= 3)
                        {
                        }
                        else
                        {
                            goto loop_10;
                        }
                    }
                    free_slot = slot_index;
                    if (binding_index != 3)
                    {
                        goto block_15;
                    }
                }
                else
                {
                block_15:
                    slot_index += 1;
                    slot_cursor++;
                    if (slot_index >= 0x30)
                    {
                        free_slot = -1;
                    }
                    else
                    {
                        goto loop_8;
                    }
                }
            }
            if (free_slot != -1)
            {
                owner_index = actor_id;
                if (actor_id >= 3)
                {
                    owner_index = 2;
                }
                binding = &D_80105880[owner_index];
                load_id = resource_id + 0x2DC;
                binding->unk4 = load_id;
                if (func_8009A364(load_id) == 0)
                {
                    slot_base = g_field_actor_slots;
                    slot = &slot_base[free_slot];
                    slot->unk24 = 1;
                    slot->state.bytes[1] = 0;
                    slot->unk232 = 0;
                    slot->unk23A = 0;
                    slot->unk23B = 0;
                    slot->state.flags = (s32)(slot->state.flags | 0x1E);
                    slot->unk26 = (u8)D_80105AE0[actor_id].unk16F;
                    binding->unk18 = free_slot;
                    binding->unk0 = 1;
                    binding->unk8 = resource_id;
                    binding->unkC = actor_id;
                    return 1;
                }
                return 0;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}
