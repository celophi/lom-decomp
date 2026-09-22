#include "common.h"

extern s32 D_801B25B8;
extern void func_80074B64(void);
extern s32 D_801B25BC;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076554(void)
{
    D_801B25BC = 0x28;
    D_801B25B8 += 1;
    func_80074B64();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007658C(void)
{
    D_801B25B8 += 1;
}
