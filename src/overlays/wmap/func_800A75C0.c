#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_80099754(s32);
extern u8 D_800DBE3C[];
extern s32 D_8011CF54;
extern u8 D_8013A180[];
extern s32 D_801B2E70;
extern s32 D_801B2E74;

/** @brief Draw the sprite, move its packed coordinate, and update the countdown. */
void func_800A75C0(void)
{
    s32 remaining_ticks;
    u8 *sprite = D_800DBE3C;

    func_80099754(0);
    func_8006CC4C(sprite, &D_8013A180);
    func_80066F9C(sprite, D_8011CF54, 0x28, 2, 2);
    remaining_ticks = D_801B2E74 - 1;
    *(s16 *)&D_8011CF54 -= 4;
    D_801B2E74 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2E70 += 1;
    }
}
