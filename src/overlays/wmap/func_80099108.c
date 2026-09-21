#include "common.h"

extern s32 D_801B2C28;
extern void func_80097A94(void);
extern s32 D_801B2C2C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800990D0(void)
{
    D_801B2C2C = 0x40;
    D_801B2C28 += 1;
    func_80097A94();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80099108(void)
{
    D_801B2C28 += 1;
}
