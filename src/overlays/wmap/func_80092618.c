#include "common.h"

extern void func_80092664(void);
extern s16 D_800D93C8[];
extern s32 D_801B2AEC;
extern s32 D_801B2AE8;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80092618(void)
{
    D_800D93C8[19] = 4;
    D_800D93C8[17] = 0;
    D_801B2AEC = 0x20;
    D_801B2AE8 += 1;
    func_80092664();
}
