#include "common.h"

extern void func_80089A84(void);
extern s16 D_800D9268[];
extern s32 D_801B296C;
extern s32 D_801B2968;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80089A38(void)
{
    D_800D9268[107] = 8;
    D_800D9268[105] = 0;
    D_801B296C = 0x10;
    D_801B2968 += 1;
    func_80089A84();
}
