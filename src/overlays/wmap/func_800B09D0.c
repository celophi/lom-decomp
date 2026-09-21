#include "common.h"

extern s32 D_801B2F60;
extern s32 D_801B2F64;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B09D0(void)
{
    D_801B2F60 = 1;
    D_801B2F64 = 1;
}
