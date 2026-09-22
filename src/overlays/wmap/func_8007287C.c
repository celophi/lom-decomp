#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
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

extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_801B2530;
extern s32 D_801B2534;

/** @brief Project and draw the map effect, advancing when its timer expires. */
void func_8007287C(void)
{
    SVECTOR position;
    s32 screen_position;
    s32 remaining;
    u8 *actor = D_800D9344;

    position.vz = 0;
    position.vx = (((D_8011D510 - 1) * 160 -
                   D_80139950.x * 0x14000 / D_80139950.scale) * 0x6000) /
                  D_80139950.scale;
    position.vy = (((D_8011D530 - 1) * 160 -
                   D_80139950.y * 0x14000 / D_80139950.scale) * 0x6000) /
                  D_80139950.scale;
    gte_ldv0(&position);
    gte_rtps();
    func_8006CC4C(actor, D_801399B0);
    gte_stsxy(&screen_position);
    func_80066F9C(actor, screen_position, 12, 10, 0);
    remaining = D_801B2534 - 1;
    D_801B2534 = remaining;
    if (remaining == 0)
    {
        D_801B2530++;
    }
}
