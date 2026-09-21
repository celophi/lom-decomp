#include "common.h"

extern s32 D_801B2888;
extern void func_80083EA0(void);
extern s32 D_801B288C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80084CE0(void)
{
    D_801B288C = 0x80;
    D_801B2888 += 1;
    func_80083EA0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80084D18(void)
{
    D_801B2888 += 1;
}
