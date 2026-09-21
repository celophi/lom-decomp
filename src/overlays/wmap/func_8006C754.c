#include "common.h"

extern s32 D_8011CF44;
extern s32 D_801B0FD8[];
extern s32 (*D_801B1018[])(s32);
extern void func_80064F14(void);
extern s32 func_8006C81C(s32);
extern void func_8006D2D4(void);
extern s32 D_8013B20C;
extern s32 D_801B10A0;

/**
 * @brief Register and initialize a callback in the first free slot.
 */
void func_8006C754(void)
{
    s32 i;
    s32 (*callback)(s32);

    callback = func_8006C81C;
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
    D_8013B20C = 1;
    D_801B10A0++;
    func_8006D2D4();
}
