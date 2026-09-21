#include "common.h"

extern s32 D_801B2D10;
extern void func_8009DBB0(void);
extern s32 D_801B2D14;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009F734(void)
{
    D_801B2D14 = 0x20;
    D_801B2D10 += 1;
    func_8009DBB0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F76C(void)
{
    D_801B2D10 += 1;
}
