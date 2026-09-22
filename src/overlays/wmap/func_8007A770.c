#include "common.h"

extern s32 D_801B26A8;
extern void func_80079284(void);
extern s32 D_801B26AC;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007A738(void)
{
    D_801B26AC = 0x40;
    D_801B26A8 += 1;
    func_80079284();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007A770(void)
{
    D_801B26A8 += 1;
}
