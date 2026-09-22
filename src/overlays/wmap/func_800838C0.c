#include "common.h"

extern s32 D_801B2848;
extern void func_80081294(void);
extern s32 D_801B284C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80083888(void)
{
    D_801B284C = 0x18;
    D_801B2848 += 1;
    func_80081294();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800838C0(void)
{
    D_801B2848 += 1;
}
