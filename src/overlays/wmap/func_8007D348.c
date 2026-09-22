#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_8007D394(void);
extern s16 D_800D9BB0[];
extern s32 D_801B273C;
extern s32 D_801B2738;
extern u8 D_80139B38[];
extern s32 D_8011CF4C;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007D2CC(void)
{
    func_8006CC4C(D_800D9BB0, D_80139B38);
    func_80066F9C(D_800D9BB0, D_8011CF4C, 0x15, 0x28, 0);
    if (--D_801B273C == 0)
    {
        D_801B2738 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8007D348(void)
{
    D_800D9BB0[19] = 8;
    D_800D9BB0[17] = 0;
    D_801B273C = 0x10;
    D_801B2738 += 1;
    func_8007D394();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007D394(void)
{
    func_8006CC4C(D_800D9BB0, D_80139B38);
    func_80066F9C(D_800D9BB0, D_8011CF4C, 0x15, 0x28, 0);
    if (--D_801B273C == 0)
    {
        D_801B2738 += 1;
    }
}
