#include "common.h"

/** @brief Actor position and packed facing direction used by the query. */
typedef struct
{
    s32 position_x;
    u8 pad4[4];
    s32 position_z;
    u8 pad12[0x21 - 0xC];
    u8 facing_flags;
} FacingActorRecord;

FacingActorRecord *func_80087C9C(s32);
long ratan2(long, long);

extern int abs(int);

/**
 * @brief Test whether one actor faces toward another within one direction step.
 * @param first_handle Identifier of the actor whose facing direction is tested.
 * @param second_handle Identifier of the target actor.
 * @return -1 if either lookup fails, otherwise 1 when facing the target or 0.
 */
s32 func_80087770(s32 first_handle, s32 second_handle)
{
    FacingActorRecord *first;
    FacingActorRecord *second;
    s32 direction;
    s32 facing;
    s32 flags;

    first = func_80087C9C(first_handle);
    if (first == (FacingActorRecord *)-1)
    {
        return -1;
    }
    second = func_80087C9C(second_handle);
    if (second == (FacingActorRecord *)-1)
    {
        return -1;
    }
    flags = first->facing_flags;
    facing = (flags & 0x7F) % 5;
    if (flags & 0x80)
    {
        facing = 8 - facing;
    }
    direction = ratan2(first->position_z - second->position_z, second->position_x - first->position_x);
    direction = (direction + 0x800) / 0x200;
    direction += 2;
    direction %= 8;
    if (abs(facing - direction) < 2)
    {
        return 1;
    }
    if (abs(facing - direction + 8) < 2)
    {
        return 1;
    }
    return abs(direction - facing + 8) < 2;
}
