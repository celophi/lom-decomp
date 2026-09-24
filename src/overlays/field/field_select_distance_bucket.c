#include "saved_game.h"
#include "common.h"
#include "field_calls.h"
#include "sdk/rand.h"

extern int abs(int);
s32 func_80087F44(s32 arg0, void* arg1);

/**
 * @brief Select a distance-weighted index within the active layout bound.
 * @param actor_id Actor whose first coordinate is compared with actor 2.
 * @return Selected weighted index, or the center index when no interval is selected.
 */
s32 func_800C9ED4(s32 actor_id)
{
    s32 reference_position[4];
    s32 actor_position[4];
    s32 weight_values[6];
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
    u8* layout;

    /* BUG: slot_count is read uninitialized when the byte at 0x29D7 is 3. */
    selected_index = -1;
    layout = g_saved_game.bytes;
    if (((s8*)layout)[0x29D7] != 3)
    {
        slot_count = layout[((s8*)layout)[0x29D7] * 0x14C + 0x2B50] >> 4;
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
    func_80087F44(actor_id, actor_position);
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
    for (radius = 0; radius < (s32)slot_count; radius++)
    {
        table_index = center_index + radius;
        if (table_index < (s32)slot_count)
        {
            weight_values[table_index] = 0x400 / weight_divisor;
        }
        table_index = center_index - radius;
        if (table_index >= 0)
        {
            weight_values[table_index] = 0x400 / weight_divisor;
        }
        weight_divisor += weight_divisor * 2;
    }
    /* BUG: weight_total is never initialized before this sum. */
    for (radius = 0; radius < (s32)slot_count; radius++)
    {
        weight_total += weight_values[radius];
    }
    random_value = (rand() * weight_total) / 32767;
    cumulative_weight = 0;
    for (selection_index = 0; selection_index < (s32)slot_count; selection_index++)
    {
        if ((random_value >= cumulative_weight) && (random_value < (cumulative_weight + weight_values[selection_index])))
        {
            selected_index = selection_index;
        }
        cumulative_weight += weight_values[selection_index];
    }
    if (selected_index == -1)
    {
        selected_index = center_index;
    }
    return selected_index;
}
