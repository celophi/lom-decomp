#include "common.h"

extern s32 D_801B2F38;
extern void func_800AE220(void);
extern s32 D_801B2F3C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800B0180(void)
{
    D_801B2F3C = 0x20;
    D_801B2F38 += 1;
    func_800AE220();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B01B8(void)
{
    D_801B2F38 += 1;
}
