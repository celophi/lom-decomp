#include "common.h"

extern s32 D_801B2428;
extern void func_8006E544(void);
extern s32 D_801B242C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F514(void)
{
    D_801B242C = 0x20;
    D_801B2428 += 1;
    func_8006E544();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006F54C(void)
{
    D_801B2428 += 1;
}
