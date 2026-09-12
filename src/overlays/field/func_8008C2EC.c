#include "common.h"
/** @brief Actor depth, presence, and slot index in a 0x54-byte record. */
typedef struct
{
    u8 pad[8];
    s32 z;
    u8 pad_c[0x19];
    u8 presence;
    u8 pad26[0x14];
    u8 slot_index;
    u8 pad3B[0x19];
} Actor;
/** @brief Actor-slot identity, eligibility flags, and depth extent. */
typedef struct
{
    s32 pad0, active;
    u8 pad8[8];
    s32 group_flags, id;
    u8 pad18[0x116];
    u16 extent;
    u8 pad130[0x10C];
} Slot;
extern Actor D_800FDF58[];
extern s32 D_800FE754;
extern Slot D_80105AE0[];
extern s32 D_8010D020;

/**
 * @brief Check eligibility and depth overlap for two actor identifiers.
 * @param first_id Identifier whose position supplies the center of the depth test.
 * @param second_id Identifier whose eligibility and position are checked.
 * @return -1 for a missing actor, 1 for an eligible overlap, or 0 otherwise.
 */
s32 func_8008C2EC(s32 first_id, s32 second_id)
{
    Slot *second_slot_scan;
    Slot *first_slot_scan;
    Actor *second_scan;
    Actor *first_scan;
    Actor *second;
    Actor *first;
    s32 second_z;
    s32 first_z;
    s32 first_extent;
    s16 second_extent;
    s32 result;
    s32 index;
    u8 second_slot;
    u8 first_index;
    u8 second_index;
    Slot *slot;

    first_scan = D_800FDF58;
    first_slot_scan = D_80105AE0;
    index = 0;
loop_1:
    index++;
    if (first_slot_scan->id == first_id)
    {
        goto found_first;
    }
    first_slot_scan++;
    first_scan++;
    if (index < 13)
    {
        goto loop_1;
    }
    first = (Actor *)-1;
check_first:
    if (first != (Actor *)-1)
    {
        goto search_second;
    }
    return -1;
found_first:
    first = first_scan;
    goto check_first;
found_second:
    second = second_scan;
    goto check_second;
search_second:
    index = 0;
    second_scan = D_800FDF58;
    second_slot_scan = D_80105AE0;
loop_9:
    index++;
    if (second_slot_scan->id == second_id)
    {
        goto found_second;
    }
    second_slot_scan++;
    second_scan++;
    if (index < 13)
    {
        goto loop_9;
    }
    second = (Actor *)-1;
check_second:
    if (second == (Actor *)-1)
    {
        return -1;
    }
    second_slot = second->slot_index;
    slot = &D_80105AE0[second_slot];
    if (second->presence == 0xFF)
    {
        return -1;
    }
    if (second_slot >= 3U && (slot->group_flags & 15) != D_800FE754)
    {
        return 0;
    }
    if (slot->active == 0)
    {
        return 0;
    }
    second_index = second->slot_index;
    first_index = first->slot_index;
    if (second_index == first_index)
    {
        return 0;
    }
    if (D_8010D020 == 0)
    {
        if (first_index < 3U)
        {
            if (second_index < 3U)
            {
                return 0;
            }
        }
        else if (second_index >= 3U)
        {
            return 0;
        }
    }
    first_z = first->z;
    second_z = second->z;
    first_extent = ((s32)(D_80105AE0[first->slot_index].extent << 16) >> 17) << 8;
    second_extent = (s16)slot->extent;
    if (second_z < first_z - first_extent - (second_extent << 7))
    {
        goto no_overlap;
    }
    result = first_z + first_extent + (second_extent << 7) >= second_z;
    goto done;
no_overlap:
    result = 0;
done:
    return result;
}
