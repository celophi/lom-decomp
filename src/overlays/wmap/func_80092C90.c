#include "common.h"

extern s32 D_801B2B08;
extern void func_80091964(void);
extern s32 D_801B2B0C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80092C58(void)
{
    D_801B2B0C = 0x20;
    D_801B2B08 += 1;
    func_80091964();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80092C90(void)
{
    D_801B2B08 += 1;
}
