#include "common.h"

/**
 * @brief Wrap a value into an inclusive range.
 * @param value Value to wrap.
 * @param minimum Lower endpoint.
 * @param maximum Upper endpoint; must not be less than minimum.
 * @return Wrapped value.
 */
s32 func_8005D948(s32 value, s32 minimum, s32 maximum)
{
    s32 adjusted;
    if (value < minimum)
    {
        adjusted = value - minimum + 1;
        return func_8005D948(adjusted + maximum, minimum, maximum);
    }
    if (maximum < value)
    {
        adjusted = value - 1;
        adjusted -= maximum;
        return func_8005D948(adjusted + minimum, minimum, maximum);
    }
    return value;
}
