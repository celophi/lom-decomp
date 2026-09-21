#include "common.h"

extern s32 D_801B2A48;
extern void func_8008D0F8(void);
extern s32 D_801B2A4C;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008E6C8(void)
{
    D_801B2A4C = 0x40;
    D_801B2A48 += 1;
    func_8008D0F8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E700(void)
{
    D_801B2A48 += 1;
}
