#include "common.h"

extern s32 D_801B2928;
extern void func_8008701C(void);
extern s32 D_801B292C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80087FB8(void)
{
    D_801B292C = 0x10;
    D_801B2928 += 1;
    func_8008701C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80087FF0(void)
{
    D_801B2928 += 1;
}
