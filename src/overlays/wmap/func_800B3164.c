#include "common.h"

extern s32 D_801B2FC8;
extern void func_800B1B78(void);
extern s32 D_801B2FCC;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800B312C(void)
{
    D_801B2FCC = 0x8;
    D_801B2FC8 += 1;
    func_800B1B78();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B3164(void)
{
    D_801B2FC8 += 1;
}
