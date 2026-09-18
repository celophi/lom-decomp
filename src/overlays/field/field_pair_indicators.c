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
void func_800A32A8(s32, u8 *);     /* forward */

extern s32 D_800EB04C[];
extern u8 D_800EC33C[];
extern u8 D_800EC37C[];
extern u8 D_800EC388[];
extern Actor D_800FDF58[];
extern s32 D_800FE754;
extern Slot D_80105AE0[];
extern s32 D_801158A0;
extern s32 D_80117EC0;
extern s32 D_80117EC4;
extern u8 D_80117EC8[];
extern s32 D_801227C8;

/**
 * @brief Select nearby active actor pairs and draw their animated indicators.
 * @param buffer Render buffer forwarded to the indicator drawing function.
 * @note Two active actors use indicator zero; three actors use the external pair order.
 * @note Distance checks exclude slots with flags 0x23E4 and use a threshold of 0x20.
 */
void func_800A2E40(u8 *buffer)
{
    extern s32 D_80117ED0[];

    s32 entries[3];
    Actor *actor;
    s32 *counter;
    s32 *second_entry;
    s32 *first_entry;
    s32 *entry_cursor;
    s32 first_actor;

    s32 active_count;
    s32 pair_index;
    s32 index;
    s32 absent;
    s32 first_absent;
    Slot *slot_base;
    s32 *order;
    Actor *actor_base;
    s32 *counter_base;
    s32 index_or_distance;
    s32 pair_offset;
    s32 next_offset;

    D_80117EC0 = 0;
    D_80117EC8[0] = 0xFFU;
    if ((D_80117EC4 == 0) && (D_801158A0 != 0))
    {
        index = 0;
        if (D_800FE754 != 0)
        {
            active_count = index;
            first_absent = 0xFF;
            actor = D_800FDF58;
            entry_cursor = entries;
            do
            {
                if (actor->unk25 != first_absent)
                {
                    *entry_cursor = index;
                    entry_cursor++;
                    active_count += 1;
                }
                index += 1;
                actor++;
            } while (index < 3);
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
                            entry_cursor = (s32 *)((((pair_index + 1) % 3) * sizeof(*order)) + (u32)order);
                            index_or_distance = *entry_cursor;
                            if (slot_base[index_or_distance].flags & 0x23E4)
                            {
                                index_or_distance = 0x20;
                            }
                            else
                            {
                                Actor *first_actor_ptr;
                                Actor *second_actor_ptr;

                                first_actor_ptr = (Actor *)(first_actor * sizeof(*actor_base) + (u32)actor_base);
                                second_actor_ptr = (Actor *)(index_or_distance * sizeof(*actor_base) + (u32)actor_base);
                                index_or_distance = func_8009A204(first_actor_ptr, second_actor_ptr);
                            }
                        }
                        if ((index_or_distance < 0x20) &&
                            (pair_offset = pair_index * sizeof(*order), first_entry = (s32 *)(pair_offset + (u32)order),
                             (actor_base[*first_entry].unk25 != absent)) &&
                            (next_offset = ((pair_index + 1) % 3) * sizeof(*order), second_entry = (s32 *)(next_offset + (u32)order),
                             counter = (s32 *)(pair_offset + (u32)counter_base), (actor_base[*second_entry].unk25 != absent)))
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
                index = D_80117EC0;
                D_80117EC8[index * 2] = 0xFF;
            }
        }
    }
}

/**
 * @brief Emit a fading animated textured quad for an active slot.
 * @param slot Quad layout index; zero and one share the first frame counter.
 * @param buffer Render buffer with an ordering table and cursor at offset 0x40B8.
 * @note Use the frame value read before incrementing its counter for this quad.
 */
void func_800A32A8(s32 slot, u8 *buffer)
{
    extern s32 D_80117ED0;

    s32 *write_counter;
    s32 *read_counter;
    s32 *reset_counter;
    s32 *increment_counter;
    s32 uv_offset;
    s32 frame;
    s32 brightness;
    u8 swap_value;
    u8 uv_flags;
    u8 *primitive;
    u32 *ordering_table;
    s32 *counters;

    primitive = *(u8 **)(buffer + 0x40B8);
    ordering_table = (u32 *)buffer;
    counters = &D_80117ED0;
    read_counter = counters;
    if (slot != 0)
    {
        read_counter = (s32 *)((slot - 1) * 4 + (u32)counters);
    }
    frame = *read_counter;
    if ((u32)(frame + 2) >= 2U)
    {
        if (frame >= 0x180)
        {
            reset_counter = counters;
            if (slot != 0)
            {
                reset_counter = (s32 *)((slot - 1) * 4 + (u32)counters);
            }
            *reset_counter = -1;
            return;
        }
        if (D_801227C8 == 0)
        {
            write_counter = counters;
            if (slot != 0)
            {
                write_counter = (s32 *)((slot - 1) * 4 + (u32)counters);
            }
            increment_counter = counters;
            if (slot != 0)
            {
                increment_counter = (s32 *)((slot - 1) * 4 + (u32)counters);
            }
            *write_counter = *increment_counter + 1;
        }
        if (frame > 0x100)
        {
            brightness = 0xFF - ((frame - 0x100) * 2);
        }
        else
        {
            brightness = 255;
        }
        if (brightness < 0)
        {
            brightness = 0;
        }
        if (brightness > 0xFF)
        {
            brightness = 0x100;
        }
        /* POLY_FT4 packet length, monochrome color and command. */
        *(u8 *)(primitive + 0x3) = 9;
        *(u8 *)(primitive + 0x6) = brightness;
        *(u8 *)(primitive + 0x5) = brightness;
        *(u8 *)(primitive + 0x4) = brightness;
        *(u8 *)(primitive + 0x7) = 0x2E;
        /* Preserve packed XY word accesses and their address grouping. */
        *(u32 *)(primitive + 0x8) = *(s32 *)(slot * 0x10 + D_800EC33C);
        *(u32 *)(primitive + 0x10) = *(s32 *)(D_800EC33C + slot * 0x10 + 0x4);
        *(u32 *)(primitive + 0x18) = *(s32 *)(D_800EC33C + slot * 0x10 + 0x8);
        *(u32 *)(primitive + 0x20) = *(s32 *)(D_800EC33C + slot * 0x10 + 0xC);
        /* Each animation frame selects UV coordinates and optional flips. */
        uv_flags = *((frame % 12) + D_800EC37C);
        uv_offset = (uv_flags & 1) * 8;
        *(u16 *)(primitive + 0xC) = *(u16 *)(uv_offset + D_800EC388);
        *(u16 *)(primitive + 0x14) = *(u16 *)(uv_offset + (D_800EC388 + 2));
        *(u16 *)(primitive + 0x1C) = *(u16 *)(uv_offset + (D_800EC388 + 4));
        *(u16 *)(primitive + 0x24) = *(u16 *)(uv_offset + (D_800EC388 + 6));
        if (uv_flags & 0x80)
        {
            swap_value = *(u8 *)(primitive + 0xC);
            *(u8 *)(primitive + 0xC) = *(u8 *)(primitive + 0x14);
            *(u8 *)(primitive + 0x14) = swap_value;
            swap_value = *(u8 *)(primitive + 0x1C);
            *(u8 *)(primitive + 0x1C) = *(u8 *)(primitive + 0x24);
            *(u8 *)(primitive + 0x24) = swap_value;
        }
        if (uv_flags & 0x40)
        {
            swap_value = *(u8 *)(primitive + 0xD);
            *(u8 *)(primitive + 0xD) = *(u8 *)(primitive + 0x1D);
            *(u8 *)(primitive + 0x1D) = swap_value;
            swap_value = *(u8 *)(primitive + 0x15);
            *(u8 *)(primitive + 0x15) = *(u8 *)(primitive + 0x25);
            *(u8 *)(primitive + 0x25) = swap_value;
        }
        /* Texture page, CLUT and ordering-table linkage. */
        *(u16 *)(primitive + 0x16) = 0x27;
        *(u16 *)(primitive + 0xE) = 0x7B05;
        *(u32 *)(primitive + 0x0) =
            (s32)((*(u32 *)(primitive + 0x0) & 0xFF000000) | (ordering_table[3] & 0xFFFFFF));
        ordering_table[3] = (s32)((ordering_table[3] & 0xFF000000) | ((s32)primitive & 0xFFFFFF));
        primitive += 0x28;
        *(u8 **)(buffer + 0x40B8) = primitive;
    }
}
