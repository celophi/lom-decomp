/**
 * @file field_record_position_queries.c
 * @brief Measure X/Z distance and compare stored object positions with live bounds.
 */

#include "common.h"
#include "field_calls.h"
#include "field_state_ops.h"
#include "vector.h"

FieldStatusState* field_find_object_state(s32 actor_id);
s32 field_get_actor_position(s32 key, Vec3i* position);

/**
 * @brief Manhattan distance between two positions on the X and Z axes.
 * @param first First position.
 * @param second Second position.
 * @return |dx| + |dz|.
 */
s32 field_distance_xz(Vec3i* first, Vec3i* second)
{
    s32 dx;
    s32 dz;

    dx = first->x - second->x;
    if (dx < 0)
    {
        dx = -dx;
    }

    dz = first->z - second->z;
    if (dz < 0)
    {
        dz = -dz;
    }

    return dx + dz;
}

/**
 * @brief Test whether an object's stored position lies within a box around its live actor position.
 * @param actor_id Object id.
 * @param half_width Half-width of the box on X.
 * @param half_depth Half-depth of the box on Z.
 * @return -1 when inside the box, 0 otherwise.
 */
s32 field_is_actor_near_stored_position(s32 actor_id, s32 half_width, s32 half_depth)
{
    FieldStatusState* object;
    Vec3i live;

    object = field_find_object_state(actor_id);
    field_get_actor_position(actor_id, &live);
    if (live.x - half_width < object->position_x && object->position_x < live.x + half_width)
    {
        if (live.z - half_depth < object->position_z && object->position_z < live.z + half_depth)
        {
            return -1;
        }
    }
    return 0;
}
