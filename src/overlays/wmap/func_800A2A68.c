#include "common.h"

extern s32 D_801B2DA0;
extern void func_800A0E84(void);
extern s32 D_801B2DA4;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A2A30(void)
{
    D_801B2DA4 = 0x40;
    D_801B2DA0 += 1;
    func_800A0E84();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2A68(void)
{
    D_801B2DA0 += 1;
}
