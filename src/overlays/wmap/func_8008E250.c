#include "common.h"

extern s32 D_801B2A30;
extern void func_8008CA70(void);
extern s32 D_801B2A34;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008E218(void)
{
    D_801B2A34 = 0x20;
    D_801B2A30 += 1;
    func_8008CA70();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E250(void)
{
    D_801B2A30 += 1;
}
