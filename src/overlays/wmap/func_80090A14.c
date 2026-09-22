#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_80090A60(void);
extern s16 D_800D9344[];
extern s32 D_801B2A8C;
extern s32 D_801B2A88;
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80090A14(void)
{
    D_800D9344[19] = 8;
    D_800D9344[17] = 0;
    D_801B2A8C = 0x10;
    D_801B2A88 += 1;
    func_80090A60();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80090A60(void)
{
    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x17, 0xA, 0);
    if (--D_801B2A8C == 0)
    {
        D_801B2A88 += 1;
    }
}
