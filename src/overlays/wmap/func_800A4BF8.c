#include "common.h"

extern s32 D_801B2E00;
extern void func_800A3624(void);
extern s32 D_801B2E04;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A4BC0(void)
{
    D_801B2E04 = 0x40;
    D_801B2E00 += 1;
    func_800A3624();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A4BF8(void)
{
    D_801B2E00 += 1;
}
