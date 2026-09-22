#include "common.h"

extern s32 D_801B2DB0;
extern void func_800A12B0(void);
extern s32 D_801B2DB4;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A2CF0(void)
{
    D_801B2DB4 = 0x20;
    D_801B2DB0 += 1;
    func_800A12B0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2D28(void)
{
    D_801B2DB0 += 1;
}
