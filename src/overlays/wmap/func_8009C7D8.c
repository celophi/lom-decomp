#include "common.h"

extern s32 D_801B2C90;
extern void func_8009B2CC(void);
extern s32 D_801B2C94;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009C7A0(void)
{
    D_801B2C94 = 0x20;
    D_801B2C90 += 1;
    func_8009B2CC();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C7D8(void)
{
    D_801B2C90 += 1;
}
