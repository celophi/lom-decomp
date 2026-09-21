#include "common.h"

extern s32 D_801B2420;
extern void func_8006E2B0(void);
extern s32 D_801B2424;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F398(void)
{
    D_801B2424 = 0x80;
    D_801B2420 += 1;
    func_8006E2B0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006F3D0(void)
{
    D_801B2420 += 1;
}
