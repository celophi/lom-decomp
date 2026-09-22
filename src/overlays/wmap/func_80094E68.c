#include "common.h"

extern s32 D_801B2B58;
extern void func_80093784(void);
extern s32 D_801B2B5C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80094E30(void)
{
    D_801B2B5C = 0x20;
    D_801B2B58 += 1;
    func_80093784();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80094E68(void)
{
    D_801B2B58 += 1;
}
