#include "common.h"

typedef struct
{
    s32 x;
    u8 pad4[4];
    s32 z;
    u8 padC[0x21 - 0xC];
    u8 flags;
} FieldAngleRecord;

/**
 * @brief Test whether one field record faces another within an angular limit.
 * @param a Record whose facing direction is tested.
 * @param b Record used as the facing target.
 * @param distance Distance value used to derive the angular limit.
 * @return 1 when the target is within the angular limit, otherwise 0.
 */
s32 func_8009CE10(FieldAngleRecord *a, FieldAngleRecord *b, s32 distance)
{
    s32 limit;
    s32 angle;

    limit = ratan2(distance, 100);
    if (!(a->flags & 0x80))
    {
        angle = ratan2((a->z - b->z) >> 8, (a->x - b->x) >> 8);
        if (angle >= 0x801)
        {
            angle = 0x1000 - angle;
        }
        if (angle < limit && -limit < angle)
        {
            return 1;
        }
    }
    else
    {
        angle = ratan2((b->z - a->z) >> 8, (b->x - a->x) >> 8);
        if (angle >= 0x801)
        {
            angle = 0x1000 - angle;
        }
        if (angle < limit && -limit < angle)
        {
            return 1;
        }
    }
    return 0;
}
