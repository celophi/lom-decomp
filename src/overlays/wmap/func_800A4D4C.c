#include "common.h"

extern s32 D_801B2E08;
extern void func_800A37D0(void);
extern s32 D_801B2E0C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A4D14(void)
{
    D_801B2E0C = 0x40;
    D_801B2E08 += 1;
    func_800A37D0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A4D4C(void)
{
    D_801B2E08 += 1;
}
