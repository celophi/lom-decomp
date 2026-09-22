#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_80082F48(void);
extern s16 D_800D93C8[];
extern s32 D_801B282C;
extern s32 D_801B2828;
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80082EFC(void)
{
    D_800D93C8[19] = 8;
    D_800D93C8[17] = 0;
    D_801B282C = 0x8;
    D_801B2828 += 1;
    func_80082F48();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082F48(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x8, 0xA, 0);
    if (--D_801B282C == 0)
    {
        D_801B2828 += 1;
    }
}
