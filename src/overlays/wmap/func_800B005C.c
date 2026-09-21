#include "common.h"

extern s32 D_801B2F30;
extern void func_800AE05C(void);
extern s32 D_801B2F34;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800B0024(void)
{
    D_801B2F34 = 0x40;
    D_801B2F30 += 1;
    func_800AE05C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B005C(void)
{
    D_801B2F30 += 1;
}
