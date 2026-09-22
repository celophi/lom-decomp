#include "common.h"

extern s32 D_801B25B0;
extern void func_800749C0(void);
extern s32 D_801B25B4;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076408(void)
{
    D_801B25B4 = 0x20;
    D_801B25B0 += 1;
    func_800749C0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80076440(void)
{
    D_801B25B0 += 1;
}
