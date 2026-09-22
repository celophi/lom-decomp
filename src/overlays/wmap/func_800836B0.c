#include "common.h"

extern s32 D_801B2840;
extern void func_800811C8(void);
extern s32 D_801B2844;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80083678(void)
{
    D_801B2844 = 0x20;
    D_801B2840 += 1;
    func_800811C8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800836B0(void)
{
    D_801B2840 += 1;
}
