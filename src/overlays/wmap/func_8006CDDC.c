#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    s32 x, y, scale, pad;
} WmapTransform;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern WmapTransform D_80139950;

/** @brief Project the current map position through the active GTE matrix. */
void func_8006CDDC(void)
{
    SVECTOR position;

    position.vz = 0;
    position.vx = (((D_8011D510 - 1) * 160 -
                   D_80139950.x * 0x14000 / D_80139950.scale) * 0x6000) /
                  D_80139950.scale;
    position.vy = (((D_8011D530 - 1) * 160 -
                   D_80139950.y * 0x14000 / D_80139950.scale) * 0x6000) /
                  D_80139950.scale;
    gte_ldv0(&position);
    gte_rtps();
}
