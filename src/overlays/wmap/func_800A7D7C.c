#include "common.h"

extern s32 D_801398D0;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_80139950[];
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2E48;
extern void func_800A7E0C(void);

/**
 * @brief World-map step: recompute the scroll offsets from the camera position and
 *        advance to the next handler.
 */
void func_800A7D7C(void)
{
    D_801398D0 = 2;
    D_80182D68 = (D_800DCEF8 - 1) * 0x30 - D_80139950[0];
    D_80182D78 = (D_800DCF00 - 1) * 0x30 - D_80139950[1];
    D_801B2E48 += 1;
    func_800A7E0C();
}
