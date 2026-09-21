#include "common.h"

extern s32 D_8011CF44;
extern s32 D_801B0FD8[];
extern s32 (*D_801B1018[])(s32);
extern void func_80064F14(void);

/**
 * @brief Register and initialize a callback in the first free slot.
 * @param callback Callback receiving one for initialization and zero for updates.
 */
void func_8006CAC0(s32 (*callback)(s32))
{
    s32 i;

    for (i = 0; i < 14; i++)
    {
        if (D_801B0FD8[i] == 0)
        {
            D_801B1018[i] = callback;
            D_801B0FD8[i] = 1;
            (D_801B1018[i])(1);
            D_8011CF44++;
            break;
        }
    }
    if (i == 14)
    {
        func_80064F14();
    }
}
