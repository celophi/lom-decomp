#include "common.h"

extern s32 D_8011CF44;
extern s32 D_801B0FD8[];
extern s32 (*D_801B1018[])(s32);

/** @brief Run active callbacks and decrement the active count when a callback finishes. */
void func_8006CA28(void)
{
    s32 i;
    s32 result;

    i = 0;
    for (; i < 14; i++)
    {
        if (D_801B0FD8[i] != 0)
        {
            result = D_801B1018[i](0);
            D_801B0FD8[i] = result;
            if (result == 0)
            {
                D_8011CF44--;
            }
        }
    }
}
