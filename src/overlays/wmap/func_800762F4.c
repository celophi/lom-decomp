#include "common.h"

extern s32 D_801B25A8;
extern void func_80074820(void);
extern s32 D_801B25AC;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800762BC(void)
{
    D_801B25AC = 0x20;
    D_801B25A8 += 1;
    func_80074820();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800762F4(void)
{
    D_801B25A8 += 1;
}
