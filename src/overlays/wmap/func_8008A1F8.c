#include "common.h"

extern s32 D_801B2988;
extern void func_80088E24(void);
extern s32 D_801B298C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008A1C0(void)
{
    D_801B298C = 0x40;
    D_801B2988 += 1;
    func_80088E24();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008A1F8(void)
{
    D_801B2988 += 1;
}
