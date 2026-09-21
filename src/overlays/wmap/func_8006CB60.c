#include "common.h"

extern s32 D_801B1058[];
extern s32 (*D_801B1078[])(s32);

/** @brief Run each active callback and retain its returned active state. */
void func_8006CB60(void)
{
    s32 (**callback)(s32);
    s32 *active;
    s32 i;

    i = 0;
    active = D_801B1058;
    callback = D_801B1078;
    do
    {
        if (*active != 0)
        {
            *active = (*callback)(0);
        }
        active++;
        i++;
        callback++;
    } while (i < 8);
}
