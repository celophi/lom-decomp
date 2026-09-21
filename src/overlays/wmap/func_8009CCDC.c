#include "common.h"

extern s32 D_801B2CA8;
extern s32 D_801B2CAC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009CCDC(void)
{
    D_801B2CA8 = 1;
    D_801B2CAC = 1;
}
