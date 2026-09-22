#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_8009CBD0(void);
extern s16 D_800D9370[];
extern s32 D_801B2CA4;
extern s32 D_801B2CA0;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009CB84(void)
{
    D_800D9370[19] = 4;
    D_800D9370[17] = 0;
    D_801B2CA4 = 0x80;
    D_801B2CA0 += 1;
    func_8009CBD0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009CBD0(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x15, 0x5, 0);
    if (--D_801B2CA4 == 0)
    {
        D_801B2CA0 += 1;
    }
}
