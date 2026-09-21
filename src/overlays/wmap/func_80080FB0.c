#include "common.h"

extern s32 D_801B27E0;
extern void func_8007F2D0(void);
extern s32 D_801B27E4;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80080F78(void)
{
    D_801B27E4 = 0x40;
    D_801B27E0 += 1;
    func_8007F2D0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80080FB0(void)
{
    D_801B27E0 += 1;
}
