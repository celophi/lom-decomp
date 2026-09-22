#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800B06A8(void);
extern s16 D_800D9370[];
extern s32 D_801B2F54;
extern s32 D_801B2F50;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B05E0(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x10, 0x7, 0);
    if (--D_801B2F54 == 0)
    {
        D_801B2F50 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800B065C(void)
{
    D_800D9370[19] = 2;
    D_800D9370[17] = 0;
    D_801B2F54 = 0x40;
    D_801B2F50 += 1;
    func_800B06A8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B06A8(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x10, 0x7, 0);
    if (--D_801B2F54 == 0)
    {
        D_801B2F50 += 1;
    }
}
