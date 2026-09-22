#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_8007A150(void);
extern s32 D_801B2694;
extern s32 D_801B2690;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007A09C(void)
{
    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x13, 0xB, 0);
    if (--D_801B2694 == 0)
    {
        D_801B2690 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007A118(void)
{
    D_801B2694 = 0x8;
    D_801B2690 += 1;
    func_8007A150();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007A150(void)
{
    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x13, 0xB, 0);
    if (--D_801B2694 == 0)
    {
        D_801B2690 += 1;
    }
}
