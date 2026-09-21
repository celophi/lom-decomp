#include "common.h"

extern s32 D_801B27E8;
extern s32 D_801B27EC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80081C18(void)
{
    D_801B27E8 = 1;
    D_801B27EC = 1;
}
