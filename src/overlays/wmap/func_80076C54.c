#include "common.h"

extern s32 D_801B25D0;
extern void func_80074D28(void);
extern s32 D_801B25D4;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076C1C(void)
{
    D_801B25D4 = 0x60;
    D_801B25D0 += 1;
    func_80074D28();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80076C54(void)
{
    D_801B25D0 += 1;
}
