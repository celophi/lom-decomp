#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800B16B0(void);
extern s16 D_800D93C8[];
extern s32 D_801B2F8C;
extern s32 D_801B2F88;
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800B1664(void)
{
    D_800D93C8[19] = 4;
    D_800D93C8[17] = 0;
    D_801B2F8C = 0x20;
    D_801B2F88 += 1;
    func_800B16B0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B16B0(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x11, 0x4, 0);
    if (--D_801B2F8C == 0)
    {
        D_801B2F88 += 1;
    }
}
