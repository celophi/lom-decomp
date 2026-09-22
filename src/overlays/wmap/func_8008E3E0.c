#include "common.h"

extern s32 D_801B2A38;
extern void func_8008CD10(void);
extern s32 D_801B2A3C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008E3A8(void)
{
    D_801B2A3C = 0x40;
    D_801B2A38 += 1;
    func_8008CD10();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E3E0(void)
{
    D_801B2A38 += 1;
}
