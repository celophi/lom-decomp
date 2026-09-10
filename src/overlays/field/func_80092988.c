#include "field_types.h"

/** @brief Proposed field position and its signed screen projection. */
typedef struct
{
    FieldVector position;
    Vec2s screen;
} FieldBoundsProjection;

extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;

/**
 * @brief Test whether a proposed displacement crosses a screen boundary.
 * @param position Current fixed-point field position.
 * @param delta Proposed displacement; only the X and Z components are tested.
 * @return One when the displacement crosses its corresponding screen edge.
 * @note Preserve the intermediate short casts and padded local vector for matching.
 */
s32 func_80092988(Vec3i *position, Vec3i *delta)
{
    FieldBoundsProjection local;

    local.position.vx = position->x + delta->x;
    local.position.vy = position->y;
    local.position.vz = position->z + delta->z;
    local.screen.x = D_800F22A0 / 256 + (s16)(local.position.vx / 256 + 160);
    local.screen.y = D_800F22A4 / 256 + (s16)(local.position.vy / 256 + 112) - local.position.vz / 512 - D_800F22A8 / 512;
    if (delta->x < 0)
    {
        if (local.screen.x < 6)
        {
            return 1;
        }
    }
    else if (delta->x > 0)
    {
        if (local.screen.x >= 314)
        {
            return 1;
        }
    }
    if (delta->z > 0)
    {
        if (local.screen.y < 9)
        {
            return 1;
        }
    }
    else if (delta->z < 0)
    {
        if (local.screen.y >= 224)
        {
            return 1;
        }
    }
    return 0;
}
