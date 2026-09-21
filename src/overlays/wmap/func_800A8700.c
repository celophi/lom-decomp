#include "common.h"

extern void func_800A8770(void);
extern s32 D_801398D0;
extern s32 D_800DCEC8[];
extern s32 D_80139950[];
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2E58;

/**
 * @brief World-map step handler: cache the relative scroll delta, bump the frame
 *        counter, and run the sub-step handler.
 */
void func_800A8700(void)
{
    D_801398D0 = 2;
    D_80182D68 = D_800DCEC8[0] - D_80139950[0];
    D_80182D78 = D_800DCEC8[1] - D_80139950[1];
    D_801B2E58 += 1;
    func_800A8770();
}
