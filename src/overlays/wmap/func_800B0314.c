#include "common.h"

extern s32 D_801B2F40;
extern void func_800AE400(void);
extern s32 D_801B2F44;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800B02DC(void)
{
    D_801B2F44 = 0x40;
    D_801B2F40 += 1;
    func_800AE400();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B0314(void)
{
    D_801B2F40 += 1;
}
