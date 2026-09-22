#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_8009484C(void);
extern s16 D_800D9318[];
extern s32 D_801B2B3C;
extern s32 D_801B2B38;
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80094800(void)
{
    D_800D9318[17] = 0;
    D_800D9318[19] = 8;
    D_801B2B3C = 0x10;
    D_801B2B38 += 1;
    func_8009484C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009484C(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0xF, 0x2, 0);
    if (--D_801B2B3C == 0)
    {
        D_801B2B38 += 1;
    }
}
