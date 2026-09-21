#include "common.h"

extern s32 D_801B2F28;
extern void func_800ADEB0(void);
extern s32 D_801B2F2C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800AFED0(void)
{
    D_801B2F2C = 0x40;
    D_801B2F28 += 1;
    func_800ADEB0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800AFF08(void)
{
    D_801B2F28 += 1;
}
