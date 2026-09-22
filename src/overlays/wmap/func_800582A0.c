#include "common.h"

extern s32 func_80058400(void);
extern s32 D_800DBE70;

/**
 * @brief Limit the current result by the world-map state unless that state is two.
 * @return The current result or its lower state limit.
 */
s32 func_800582A0(void)
{
    s32 current = func_80058400();
    if (D_800DBE70 != 2)
    {
        s32 limit = D_800DBE70;
        if (current >= limit)
        {
            return limit;
        }
    }
    return current;
}
