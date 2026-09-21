#include "common.h"

extern s32 D_801B25A0;
extern void func_80074680(void);
extern s32 D_801B25A4;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076170(void)
{
    D_801B25A4 = 0x28;
    D_801B25A0 += 1;
    func_80074680();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800761A8(void)
{
    D_801B25A0 += 1;
}
