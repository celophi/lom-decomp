#include "common.h"

extern s32 D_801B26B0;
extern void func_80079480(void);
extern s32 D_801B26B4;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007A8C0(void)
{
    D_801B26B4 = 0x40;
    D_801B26B0 += 1;
    func_80079480();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007A8F8(void)
{
    D_801B26B0 += 1;
}
