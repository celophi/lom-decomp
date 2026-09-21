#include "common.h"

extern s32 D_801B2418;
extern void func_8006DEFC(void);
extern s32 D_801B241C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F1DC(void)
{
    D_801B241C = 0x40;
    D_801B2418 += 1;
    func_8006DEFC();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006F214(void)
{
    D_801B2418 += 1;
}
