#include "common.h"

#define WMAP_ROUTE_NODE_COUNT 64
#define WMAP_ROUTE_UNVISITED_DISTANCE 100
#define WMAP_INVALID_NODE 0xFF
#define WMAP_NODE_RECORD_SIZE 12
#define WMAP_NODE_X(index) ((u8)(D_800432C9[(index) * WMAP_NODE_RECORD_SIZE] & 0xF))
#define WMAP_NODE_Y(index) ((u8)(D_800432C9[(index) * WMAP_NODE_RECORD_SIZE] >> 4))

extern u8 D_800432C9[];

s32 func_8005D670(s32 x, s32 y);
extern int abs(int value);

/**
 * @brief Build a world-map route between two grid coordinates.
 * @param start_x Starting world-map grid X coordinate.
 * @param start_y Starting world-map grid Y coordinate.
 * @param end_x Destination world-map grid X coordinate.
 * @param end_y Destination world-map grid Y coordinate.
 * @param out_x Output array receiving route X coordinates.
 * @param out_y Output array receiving route Y coordinates.
 */
void func_8005EB68(s32 start_x, s32 start_y, s32 end_x, s32 end_y, s32 *out_x, s32 *out_y)
{
    s32 i;
    s32 path_found;
    s32 current_x;
    s32 current_y;
    s32 distance;
    s32 node_index;
    s32 distances[WMAP_ROUTE_NODE_COUNT];
    s32 route[WMAP_ROUTE_NODE_COUNT];

    if (func_8005D670(start_x, start_y) == WMAP_INVALID_NODE)
    {
        current_x = start_x;
        current_y = start_y;
        i = 0;
        while (current_x != end_x || current_y != end_y)
        {
            out_x[i] = current_x;
            out_y[i] = current_y;

            if (abs(end_x - current_x) > abs(end_y - current_y))
            {
                current_x += (end_x - current_x) / abs(end_x - current_x);
            }
            else
            {
                current_y += (end_y - current_y) / abs(end_y - current_y);
            }

            i++;
        }

        out_x[i] = end_x;
        out_y[i] = end_y;
        return;
    }

    i = 0;
    while (i < WMAP_ROUTE_NODE_COUNT)
    {
        distances[i] = WMAP_ROUTE_UNVISITED_DISTANCE;
        i++;
    }

    distances[func_8005D670(start_x, start_y)] = 0;
    distance = 0;
    path_found = 0;

    while (path_found == 0)
    {
        i = 0;
        while (i < WMAP_ROUTE_NODE_COUNT)
        {
            if (distances[i] == distance)
            {
                node_index = func_8005D670(WMAP_NODE_X(i) - 1, WMAP_NODE_Y(i));
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == func_8005D670(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }

                node_index = func_8005D670(WMAP_NODE_X(i) + 1, WMAP_NODE_Y(i));
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == func_8005D670(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }

                node_index = func_8005D670(WMAP_NODE_X(i), WMAP_NODE_Y(i) - 1);
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == func_8005D670(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }

                node_index = func_8005D670(WMAP_NODE_X(i), WMAP_NODE_Y(i) + 1);
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == func_8005D670(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }
            }
            i++;
        }
        distance++;
    }

    distance = distances[func_8005D670(end_x, end_y)];
    route[distance] = func_8005D670(end_x, end_y);
    node_index = func_8005D670(end_x, end_y);

    while (distance >= 0)
    {
        i = 0;
        while (i < WMAP_ROUTE_NODE_COUNT)
        {
            if (distances[i] == distance - 1)
            {
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index) - 1) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index)))
                {
                    route[distance - 1] = i;
                    break;
                }
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index) + 1) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index)))
                {
                    route[distance - 1] = i;
                    break;
                }
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index)) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index) - 1))
                {
                    route[distance - 1] = i;
                    break;
                }
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index)) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index) + 1))
                {
                    route[distance - 1] = i;
                    break;
                }
            }
            i++;
        }
        distance--;
        node_index = route[distance];
    }

    i = 0;
    while (i <= distances[func_8005D670(end_x, end_y)])
    {
        out_x[i] = WMAP_NODE_X(route[i]);
        out_y[i] = WMAP_NODE_Y(route[i]);
        i++;
    }
}
