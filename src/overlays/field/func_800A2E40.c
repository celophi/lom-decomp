#include "common.h"

/** @brief Actor presence byte within the original 0x54-byte record. */
typedef struct
{
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[0x54 - 0x26];
} Actor;
/** @brief Action flags within the original 0x23C-byte runtime slot. */
typedef struct
{
    u8 pad0[12];
    s32 flags;
    u8 pad10[0x23C - 0x10];
} Slot;
s32 func_8009A204(void *, void *); /* extern */
void func_800A32A8(s32, u8 *);     /* external */
extern s32 D_800EB04C[];
extern Actor D_800FDF58[];
extern s32 D_800FE754;
extern Slot D_80105AE0[];
extern s32 D_801158A0;
extern s32 D_80117EC0;
extern s32 D_80117EC4;
extern u8 D_80117EC8[];
extern s32 D_80117ED0[];

/**
 * @brief Select nearby active actor pairs and draw their animated indicators.
 * @param buffer Render buffer forwarded to the indicator drawing function.
 * @note Two active actors use indicator zero; three actors use the external pair order.
 * @note Distance checks exclude slots with flags 0x23E4 and use a threshold of 0x20.
 */
void func_800A2E40(u8 *buffer)
{
    s32 entries[3];
    Actor *actor;
    s32 *counter;
    s32 *second_entry;
    s32 *first_entry;
    s32 *next_entry;
    s32 first_actor;

    s32 active_count;
    s32 pair_index;
    s32 var_v0;
    s32 actor_index;
    s32 absent;
    s32 first_absent;
    Slot *slot_base;
    s32 *order;
    Actor *actor_base;
    s32 *counter_base;
    s32 index_or_distance;

    D_80117EC0 = 0;
    D_80117EC8[0] = 0xFFU;
    if ((D_80117EC4 == 0) && (D_801158A0 != 0))
    {
        actor_index = 0;
        if (D_800FE754 != 0)
        {
            active_count = actor_index;
            first_absent = 0xFF;
            actor = D_800FDF58;
            next_entry = entries;
            do
            {
                if (actor->unk25 != first_absent)
                {
                    *next_entry = actor_index;
                    next_entry++;
                    active_count += 1;
                }
                actor_index += 1;
                actor++;
            } while (actor_index < 3);
            if (active_count >= 2)
            {
                pair_index = 0;
                if (active_count == 2)
                {
                    index_or_distance = 0x20;
                    if (!(D_80105AE0[entries[0]].flags & 0x23E4))
                    {
                        index_or_distance = entries[1];
                        if (D_80105AE0[index_or_distance].flags & 0x23E4)
                        {
                            index_or_distance = 0x20;
                        }
                        else
                        {
                            index_or_distance = func_8009A204(&D_800FDF58[entries[0]], &D_800FDF58[index_or_distance]);
                        }
                    }
                    if (index_or_distance < 0x20)
                    {
                        if (D_80117ED0[0] == -2)
                        {
                            D_80117ED0[0] = 0;
                        }
                        if (D_80117ED0[0] != -1)
                        {
                            D_80117EC8[2] = 0xFF;
                            D_80117EC0 = 2;
                            D_80117EC8[0] = (u8)entries[0];
                            D_80117EC8[1] = (u8)entries[1];
                        }
                    }
                    else
                    {
                        D_80117ED0[0] = -2;
                    }
                    func_800A32A8(0, buffer);
                }
                else
                {
                    slot_base = D_80105AE0;
                    order = D_800EB04C;
                    actor_base = D_800FDF58;
                    absent = 0xFF;
                    counter_base = D_80117ED0;
                pair_loop:
                {
                    first_actor = order[pair_index];
                    index_or_distance = 0x20;
                    if (!(slot_base[first_actor].flags & 0x23E4))
                    {
                        index_or_distance = order[(pair_index + 1) % 3];
                        if (slot_base[index_or_distance].flags & 0x23E4)
                        {
                            index_or_distance = 0x20;
                        }
                        else
                        {
                            index_or_distance = func_8009A204(&actor_base[first_actor], &actor_base[index_or_distance]);
                        }
                    }
                    if ((index_or_distance < 0x20) &&
                        (first_entry = &order[pair_index], (actor_base[*first_entry].unk25 != absent)) &&
                        (second_entry = &order[(pair_index + 1) % 3], counter = &counter_base[pair_index],
                         (actor_base[*second_entry].unk25 != absent)))
                    {
                        if (*counter == -2)
                        {
                            *counter = 0;
                        }
                        if (*counter != -1)
                        {
                            D_80117EC8[D_80117EC0 * 2] = (u8)*first_entry;
                            D_80117EC8[D_80117EC0 * 2 + 1] = (u8)*second_entry;
                            D_80117EC0 += 1;
                        }
                    }
                    else
                    {
                        counter_base[pair_index] = -2;
                    }
                    pair_index += 1;
                    func_800A32A8(pair_index, buffer);
                }
                    if (pair_index < 3)
                    {
                        goto pair_loop;
                    }
                }
                D_80117EC8[D_80117EC0 * 2] = 0xFF;
            }
        }
    }
}
