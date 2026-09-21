#include "common.h"

extern s32 D_801B2410;
extern void func_8006DA24(void);
extern s32 D_801B2414;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F01C(void)
{
    D_801B2414 = 0x40;
    D_801B2410 += 1;
    func_8006DA24();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006F054(void)
{
    D_801B2410 += 1;
}
