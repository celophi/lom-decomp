#include "common.h"

extern s32 D_801B2FD0;
extern void func_800B1D7C(void);
extern s32 D_801B2FD4;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800B3288(void)
{
    D_801B2FD4 = 0x8;
    D_801B2FD0 += 1;
    func_800B1D7C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B32C0(void)
{
    D_801B2FD0 += 1;
}
