#include "common.h"

typedef struct
{
    u32 cap;
    u32 value;
} SaturatingCounter;

/**
 * @brief Add to a counter and clamp it to its cap on overflow.
 * @param counter Counter to update.
 * @param delta Amount to add to the counter's value.
 */
void func_800B3114(SaturatingCounter *counter, s32 delta)
{
    u32 cap;
    u32 sum;

    cap = counter->cap;
    sum = counter->value + delta;
    counter->value = sum;
    if (cap < sum)
    {
        counter->value = cap;
    }
}
