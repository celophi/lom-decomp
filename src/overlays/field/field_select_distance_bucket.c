#include "common.h"
#include "sdk/rand.h"

extern int abs(int);

extern u8 g_menuLayoutBuffer[];
s32 func_80087F44(s32 arg0, void *arg1);

/**
 * @brief Select a distance-weighted index within the active layout bound.
 * @param arg0 Actor identifier whose first coordinate is compared with actor 2.
 * @return Selected weighted index, or the center index when no interval is selected.
 */
s32 func_800C9ED4(s32 arg0)
{
    s32 reference_position[4];
    s32 actor_position[4];
    s32 weight_values[6];
    s32 *selection_weight;
    s32 *sum_weight;
    s32 *weight_table;
    s32 random_value;
    s32 table_index;
    s32 radius;
    s32 selection_index;
    s32 weight_divisor;
    s32 cumulative_weight;
    s32 center_index;
    s32 weight_total;
    s32 selected_index;
    s32 distance;
    u32 slot_count;
    u32 bounded_count;
    u8 *layout;

    selected_index = -1;
    layout = g_menuLayoutBuffer;
    if (((s8 *)layout)[0x29D7] != 3)
    {
        slot_count = layout[((s8 *)layout)[0x29D7] * 0x14C + 0x2B50] >> 4;
    }
    if ((s32)slot_count >= 4)
    {
        bounded_count = 6;
        if ((s32)slot_count < 7)
        {
            bounded_count = slot_count;
        }
    }
    else
    {
        bounded_count = 4;
    }
    slot_count = bounded_count;
    func_80087F44(2, reference_position);
    func_80087F44(arg0, actor_position);
    distance = abs(reference_position[0] - actor_position[0]);
    center_index = distance / 0x100;
    if (center_index >= 0)
    {
        table_index = 0x95;
        if (center_index < 0x96)
        {
            table_index = center_index;
        }
    }
    else
    {
        table_index = 0;
    }
    weight_divisor = 1;
    center_index = table_index / (s32)(0x96 / (s32)slot_count);
    radius = 0;
    if ((s32)slot_count > 0)
    {
        weight_table = weight_values;
        do
        {
            table_index = center_index + radius;
            if (table_index < (s32)slot_count)
            {
                *(s32 *)((u8 *)weight_table + (table_index << 2)) = 0x400 / weight_divisor;
            }
            table_index = center_index - radius;
            if (table_index >= 0)
            {
                *(s32 *)((u8 *)weight_table + (table_index << 2)) = 0x400 / weight_divisor;
            }
            weight_divisor += weight_divisor * 2;
            radius += 1;
        } while (radius < (s32)slot_count);
    }
    slot_count++;
    slot_count--;
    radius = 0;
    if ((s32)slot_count > 0)
    {
        sum_weight = &weight_values[0];
        do
        {
            weight_total += *sum_weight++;
            radius += 1;
        } while (radius < (s32)slot_count);
    }
    random_value = (rand() * weight_total) / 32767;
    cumulative_weight = 0;
    selection_index = 0;
    if ((s32)slot_count > 0)
    {
        selection_weight = &weight_values[0];
        do
        {
            if ((random_value >= cumulative_weight) && (random_value < (cumulative_weight + *selection_weight)))
            {
                selected_index = selection_index;
            }
            cumulative_weight += *selection_weight++;
            selection_index += 1;
        } while (selection_index < (s32)slot_count);
    }
    if (selected_index == -1)
    {
        selected_index = center_index;
    }
    return selected_index;
}
