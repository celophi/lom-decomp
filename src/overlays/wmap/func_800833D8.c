#include "common.h"

extern void func_80083424(void);
extern s16 D_800D9420[];
extern s32 D_801B283C;
extern s32 D_801B2838;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800833D8(void)
{
    D_800D9420[19] = 8;
    D_800D9420[17] = 0;
    D_801B283C = 0x8;
    D_801B2838 += 1;
    func_80083424();
}
