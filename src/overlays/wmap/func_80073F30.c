#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2548;
extern u8 D_800DBE10[];
extern u8 D_8013A178[];
extern s32 D_8011CF4C;
extern s32 D_801B254C;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80073EB4(void)
{
    func_8006CC4C(D_800DBE10, D_8013A178);
    func_80066F9C(D_800DBE10, D_8011CF4C, 0xE, 0xA, 0);
    if (--D_801B254C == 0)
    {
        D_801B2548 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80073F30(void)
{
    D_801B2548 += 1;
}
