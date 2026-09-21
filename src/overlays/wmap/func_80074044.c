#include "common.h"

extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800DBE3C[];
extern u8 D_8013A180[];
extern s32 D_8011CF4C;
extern s32 D_801B2554;
extern s32 D_801B2550;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80074044(void)
{
    func_8006CC4C(D_800DBE3C, D_8013A180);
    func_80066F9C(D_800DBE3C, D_8011CF4C, 0xD, 0xA, 0);
    if (--D_801B2554 == 0)
    {
        D_801B2550 += 1;
    }
}
