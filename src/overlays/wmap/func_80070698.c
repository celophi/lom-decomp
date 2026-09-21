#include "common.h"

extern s32 D_801B2460;
extern void func_8006EDF4(void);
extern s32 D_801B2464;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80070660(void)
{
    D_801B2464 = 0x28;
    D_801B2460 += 1;
    func_8006EDF4();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80070698(void)
{
    D_801B2460 += 1;
}
