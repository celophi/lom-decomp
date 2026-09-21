#include "common.h"

extern s32 D_801B2AB8;
extern void func_8008F7D4(void);
extern s32 D_801B2ABC;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80091440(void)
{
    D_801B2ABC = 0x40;
    D_801B2AB8 += 1;
    func_8008F7D4();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80091478(void)
{
    D_801B2AB8 += 1;
}
