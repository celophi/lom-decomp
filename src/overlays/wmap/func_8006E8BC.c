#include "common.h"

/** @brief World-map actor configuration. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;


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

extern WmapConfigA D_800D9268[];
extern s32 D_8011CF4C;
extern u8 D_8011D538[];
extern void *D_801399AC;
extern s32 D_80182DF0;
extern s32 D_801B2438;
extern s32 D_801B243C;
extern void func_8006F864(void);

/** @brief Initialize and project the map effect before its first draw. */
void func_8006E8BC(void)
{
    SVECTOR position;

    D_801399AC = D_8011D538;
    D_800D9268[4].field_06 = 15;
    D_800D9268[4].field_10 = -1;
    D_800D9268[4].field_02 = 0;
    D_800D9268[4].field_0E = 0;
    D_800D9268[4].field_22 = 128;
    D_800D9268[4].field_24 = 128;
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
    D_80182DF0 = 128;
    D_801B243C = 64;
    D_801B2438++;
    func_8006F864();
}
