#include "common.h"

extern s32 D_801B26A0;
extern void func_80079088(void);
extern s32 D_801B26A4;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007A5B0(void)
{
    D_801B26A4 = 0x40;
    D_801B26A0 += 1;
    func_80079088();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007A5E8(void)
{
    D_801B26A0 += 1;
}
