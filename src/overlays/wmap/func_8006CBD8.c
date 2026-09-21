#include "common.h"

extern s32 D_801B1058[];
extern s32 (*D_801B1078[])(s32);

/**
 * @brief Install and initialize a callback in the first inactive slot.
 * @param callback Callback receiving one for initialization and zero for updates.
 */
void func_8006CBD8(s32 (*callback)(s32))
{
    s32 (**slot)(s32);
    s32 *active;
    s32 i;

    i = 0;
    active = D_801B1058;
    slot = D_801B1078;
next_slot:
    i++;
    if (*active == 0)
    {
        *slot = callback;
        *active = 1;
        *active = (*slot)(1);
        return;
    }
    active++;
    slot++;
    if (i >= 8)
    {
        return;
    }
    goto next_slot;
}
