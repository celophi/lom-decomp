#include "common.h"
#include "vector.h"

/**
 * @brief Find an integer intersection of two screen-space line segments.
 * @param first_start First segment's starting point.
 * @param first_end First segment's ending point.
 * @param second_start Second segment's starting point.
 * @param second_end Second segment's ending point.
 * @return X in the low 16 bits and Y in the high 16 bits, or 0x80008000.
 * @note Collinear overlapping segments return the first segment's start.
 * @note Preserve full-width candidates and the unsigned weighted bounds checks.
 */
s32 func_800978AC(Vec2s *first_start, Vec2s *first_end, Vec2s *second_start, Vec2s *second_end)
{
    s32 endpoint_a, endpoint_b, distance_a;
    s32 endpoint_a0, endpoint_b0;
    s32 endpoint_a1, endpoint_b1;
    s32 endpoint_a2, endpoint_b2;
    s32 endpoint_a3, endpoint_b3;
    s32 load_end;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_a0_3;
    s32 temp_a0_4;
    s32 temp_a0_5;
    s32 temp_a0_6;
    s32 temp_a0_7;
    s32 temp_a0_8;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_a1_3;
    s32 temp_a1_4;
    s32 temp_a1_5;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_a2_3;
    s32 temp_a2_4;
    s32 temp_a3;
    s32 temp_a3_2;
    s32 temp_a3_3;
    s32 temp_a3_4;
    s32 temp_t0;
    s32 temp_t1;
    s32 temp_t1_3;
    s32 temp_t1_4;
    s32 first_start_x;
    s32 intersection_x;
    s32 intersection_y;
    s32 first_start_y;
    s32 first_end_x;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 temp_v1_5;
    s32 temp_v1_6;
    s32 temp_v1_9;
    s32 cross_yx;
    s32 cross_xy;
    s32 second_dx;
    s32 temp_t0_3;
    s32 first_dx;
    s32 first_dy;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v1;
    s32 temp_v1_7;
    s32 second_dy;
    s32 var_t0;
    s32 var_t0_2;
    s32 var_t0_3;
    s32 var_t0_4;
    s32 value;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    u32 packed_x_or_sum;
    u32 temp_v1_10;
    u32 temp_v1_11;
    u32 temp_v1_12;
    u32 temp_v1_13;

    /* Coordinate locals also retain endpoint values before interpolation. */
    intersection_y = first_end->y;
    first_start_y = first_start->y;
    first_dy = intersection_y - first_start_y;
    if (first_dy == 0)
    {
        load_end = second_end->y;
        temp_t1 = second_start->y;
        temp_v1 = load_end - temp_t1;
        intersection_y = first_start_y;
        if (temp_v1 == 0)
        {
            if ((intersection_y == temp_t1) && ((temp_t0 = second_start->x, temp_v1_2 = first_start->x, ((temp_t0 < temp_v1_2) == 0)) || (temp_a0 = second_end->x, ((temp_a0 < temp_v1_2) == 0)) || (temp_v1_3 = first_end->x, ((temp_t0 < temp_v1_3) == 0)) || (temp_a0 >= temp_v1_3)) && ((temp_a2 = second_start->x, temp_v1_4 = first_start->x, ((temp_v1_4 < temp_a2) == 0)) || (temp_a3 = second_end->x, ((temp_v1_4 < temp_a3) == 0)) || (temp_a1 = first_end->x, ((temp_a1 < temp_a2) == 0)) || (temp_a1 >= temp_a3)))
            {
                packed_x_or_sum = (u16) first_start->x;
                value = intersection_y << 0x10;
                goto pack_result;
            }
            goto no_intersection;
        }
        load_end = second_end->x;
        temp_a0_2 = second_start->x;
        second_dx = load_end - temp_a0_2;
        if (second_dx == 0)
        {
            intersection_x = temp_a0_2;
        }
        else
        {
            intersection_x = ((s32) ((intersection_y - temp_t1) * second_dx) / temp_v1) + temp_a0_2;
        }
        goto check_bounds;
    }
    first_end_x = first_end->x;
    first_start_x = first_start->x;
    first_dx = first_end_x - first_start_x;
    if (first_dx == 0)
    {
        load_end = second_end->x;
        temp_t1_3 = second_start->x;
        temp_t0_3 = load_end - temp_t1_3;
        intersection_x = first_start_x;
        if (temp_t0_3 == 0)
        {
            if ((intersection_x == temp_t1_3) && ((temp_a0_3 = second_start->y, ((temp_a0_3 < first_start_y) == 0)) || (temp_v1_5 = second_end->y, ((temp_v1_5 < first_start_y) == 0)) || (temp_a0_3 >= intersection_y) || (temp_v1_5 >= intersection_y)))
            {
                temp_a2_2 = second_start->y;
                temp_v1_6 = first_start->y;
                if ((temp_v1_6 >= temp_a2_2) || (temp_a3_2 = second_end->y, ((temp_v1_6 < temp_a3_2) == 0)) || (temp_a1_2 = first_end->y, ((temp_a1_2 < temp_a2_2) == 0)) || (temp_a1_2 >= temp_a3_2))
                {
                    packed_x_or_sum = intersection_x & 0xFFFF;
                    value = first_start->y << 0x10;
                    goto pack_result;
                }
                goto reject_overlap;
            }
            goto no_intersection;
        }
        load_end = second_end->y;
        temp_a0_4 = second_start->y;
        temp_v1_7 = load_end - temp_a0_4;
        if (temp_v1_7 == 0)
        {
            intersection_y = temp_a0_4;
        }
        else
        {
            intersection_y = ((s32) ((intersection_x - temp_t1_3) * temp_v1_7) / temp_t0_3) + temp_a0_4;
        }
        goto check_bounds;
    }
    load_end = second_end->y;
    intersection_x = second_start->y;
    second_dy = load_end - intersection_x;
    intersection_y = intersection_x;
    if (second_dy == 0)
    {
        intersection_x = ((s32) ((intersection_y - first_start_y) * first_dx) / first_dy) + first_start_x;
        goto check_bounds;
    }
    intersection_y = second_end->x;
    temp_a0_5 = second_start->x;
    second_dx = intersection_y - temp_a0_5;
    if (second_dx == 0)
    {
        intersection_x = temp_a0_5;
        goto calculate_y;
    }
    cross_yx = first_dy * second_dx;
    cross_xy = first_dx * second_dy;
    if (cross_yx == cross_xy)
    {
        if ((intersection_x == (((s32) ((temp_a0_5 - first_start_x) * first_dy) / first_dx) + first_start_y)) && ((temp_a0_5 >= first_start_x) || (intersection_y >= first_start_x) || (temp_a0_5 >= first_end_x) || (intersection_y >= first_end_x)))
        {
            temp_a2_3 = second_start->x;
            temp_v1_9 = first_start->x;
            if ((temp_v1_9 >= temp_a2_3) || (temp_a3_3 = second_end->x, ((temp_v1_9 < temp_a3_3) == 0)) || (temp_a1_3 = first_end->x, ((temp_a1_3 < temp_a2_3) == 0)) || (temp_a1_3 >= temp_a3_3))
            {
                packed_x_or_sum = (u16) first_start->x;
                value = first_start->y << 0x10;
                goto pack_result;
            }
            goto reject_overlap;
        }
        goto no_intersection;
    }
    intersection_x = ((s32) ((((intersection_x - ((s32) (second_dy * temp_a0_5) / second_dx)) - first_start_y) + ((s32) (first_dy * first_start_x) / first_dx)) * (first_dx * second_dx)) / (s32) (cross_yx - cross_xy));
calculate_y:
    intersection_y = ((s32) ((intersection_x - first_start_x) * first_dy) / first_dx) + first_start_y;
check_bounds:
    /* Each weighted quotient must reproduce its candidate coordinate. */
    endpoint_a0 = first_start->x;
    endpoint_b0 = first_end->x;
    value = intersection_x - endpoint_a0;
    distance_a = value;
    if (value < 0)
    {
        distance_a = -distance_a;
    }
    value = intersection_x - endpoint_b0;
    if (value < 0)
    {
        value = -value;
    }
    packed_x_or_sum = distance_a + value;
    if (packed_x_or_sum != 0)
    {
        value *= endpoint_a0;
        distance_a *= endpoint_b0;
        value += distance_a;
        if ((u32)value / packed_x_or_sum != intersection_x)
        {
            goto no_intersection;
        }
    }
    endpoint_a1 = first_start->y;
    endpoint_b1 = first_end->y;
    value = intersection_y - endpoint_a1;
    distance_a = value;
    if (value < 0)
    {
        distance_a = -distance_a;
    }
    value = intersection_y - endpoint_b1;
    if (value < 0)
    {
        value = -value;
    }
    packed_x_or_sum = distance_a + value;
    if (packed_x_or_sum != 0)
    {
        value *= endpoint_a1;
        distance_a *= endpoint_b1;
        value += distance_a;
        if ((u32)value / packed_x_or_sum != intersection_y)
        {
            goto no_intersection;
        }
    }
    endpoint_a2 = second_start->x;
    endpoint_b2 = second_end->x;
    value = intersection_x - endpoint_a2;
    distance_a = value;
    if (value < 0)
    {
        distance_a = -distance_a;
    }
    value = intersection_x - endpoint_b2;
    if (value < 0)
    {
        value = -value;
    }
    packed_x_or_sum = distance_a + value;
    if (packed_x_or_sum != 0)
    {
        value *= endpoint_a2;
        distance_a *= endpoint_b2;
        value += distance_a;
        if ((u32)value / packed_x_or_sum != intersection_x)
        {
            goto no_intersection;
        }
    }
    endpoint_a3 = second_start->y;
    endpoint_b3 = second_end->y;
    value = intersection_y - endpoint_a3;
    distance_a = value;
    if (value < 0)
    {
        distance_a = -distance_a;
    }
    value = intersection_y - endpoint_b3;
    if (value < 0)
    {
        value = -value;
    }
    packed_x_or_sum = distance_a + value;
    if (packed_x_or_sum != 0)
    {
        value *= endpoint_a3;
        distance_a *= endpoint_b3;
        value += distance_a;
        if ((u32)value / packed_x_or_sum != intersection_y)
        {
            goto no_intersection;
        }
    }
    goto valid;
reject_overlap:
no_intersection:
    return 0x80008000;
valid:
    packed_x_or_sum = intersection_x & 0xFFFF;
    value = intersection_y << 16;
pack_result:
    return packed_x_or_sum | value;
}
