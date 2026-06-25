#include "common.h"

extern unsigned int D_801ED02C;

/**
 * @brief Return non-zero if D_801ED02C is set.
 * @return 1 if D_801ED02C != 0, 0 otherwise.
 * @see decomp.me TODO
 */
s32 func_8005B218(void) {
    return D_801ED02C != 0;
}
