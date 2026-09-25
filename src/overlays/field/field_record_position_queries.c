/**
 * @file field_record_position_queries.c
 * @brief Measure X/Z distance and compare stored object positions with live bounds.
 */

#include "common.h"
#include "field_calls.h"
#include "field_state_ops.h"

/** @brief Three-dimensional field position. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

/** @brief Live actor position as field_get_actor_position writes it. */
typedef struct
{
    FieldPosition position;
    s32 unkC;
} FieldLivePosition;

FieldStatusState* field_find_object_state(s32 actor_id);
s32 field_get_actor_position(s32 actor_id, FieldLivePosition* out);

/**
 * @brief Manhattan distance between two positions on the X and Z axes.
 * @param first First position.
 * @param second Second position.
 * @return |dx| + |dz|.
 */
s32 func_800C1FBC(FieldPosition* first, FieldPosition* second)
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
 * @brief Test whether an object's stored position lies within a box around its live position.
 * @param actor_id Object id.
 * @param half_width Half-width of the box on X.
 * @param half_depth Half-depth of the box on Z.
 * @return -1 when inside the box, 0 otherwise.
 */
s32 func_800C1FFC(s32 actor_id, s32 half_width, s32 half_depth)
{
    FieldStatusState* object;
    FieldLivePosition live;
    s32 center_x;
    s32 center_z;

    object = field_find_object_state(actor_id);
    field_get_actor_position(actor_id, &live);
    center_x = live.position.x;
    if ((center_x - half_width) < object->position_x && object->position_x < (center_x + half_width))
    {
        center_z = live.position.z;
        if ((center_z - half_depth) < object->position_z && object->position_z < (center_z + half_depth))
        {
            return -1;
        }
    }
    return 0;
}
