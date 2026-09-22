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

extern SVECTOR D_80139278;
extern VECTOR D_80182DC0;
extern s32 D_8011CF4C;
extern s32 D_8013B20C;
extern s32 D_801B1098;

/** @brief Set the map transform, project its position, and advance the effect. */
void func_80066DD8(void)
{
    MATRIX matrix;
    SVECTOR position;

    RotMatrix(&D_80139278, &matrix);
    TransMatrix(&matrix, &D_80182DC0);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    position.vz = 0;
    position.vx = (((D_8011D510 - 1) * 160 -
                   D_80139950.x * 0x14000 / D_80139950.scale) * 0x6000) /
                  D_80139950.scale;
    position.vy = (((D_8011D530 - 1) * 160 -
                   D_80139950.y * 0x14000 / D_80139950.scale) * 0x6000) /
                  D_80139950.scale;
    gte_ldv0(&position);
    gte_rtps();
    gte_stsxy(&D_8011CF4C);
    D_8013B20C = 0;
    D_801B1098++;
}
