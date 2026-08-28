#include "common.h"
#include "sdk/rand.h"

/**
 * @brief Returns a random value bounded by an inclusive maximum.
 *
 * @param maximum Inclusive upper bound. A value of -1 returns the raw random
 *                 value.
 * @return The raw random value for -1, or the random value modulo
 *         `maximum + 1` otherwise.
 */
u32 func_800BE680(s32 maximum)
{
    u32 range = maximum + 1;

    if (range == 0)
    {
        return rand();
    }
    if (range != 0)
    {
        return rand() % range;
    }
}
