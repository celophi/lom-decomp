#include "common.h"

extern s32 D_801B2D18;
extern void func_8009DD90(void);
extern s32 D_801B2D1C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009F890(void)
{
    D_801B2D1C = 0x10;
    D_801B2D18 += 1;
    func_8009DD90();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F8C8(void)
{
    D_801B2D18 += 1;
}
