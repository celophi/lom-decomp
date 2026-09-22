#include "common.h"

extern s32 D_801B24F0;
extern void func_80070B28(void);
extern s32 D_801B24F4;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007223C(void)
{
    D_801B24F4 = 0x20;
    D_801B24F0 += 1;
    func_80070B28();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80072274(void)
{
    D_801B24F0 += 1;
}
