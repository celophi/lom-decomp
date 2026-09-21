#include "common.h"

extern void func_80063BE0(void);
extern s32 D_8011CF18;
extern s32 D_8011CF44;
extern s32 D_8011D4FC;
extern s32 D_80139230;
extern s32 D_80182DB4;
extern s32 D_80182DE0;

/** @brief Clear completed wait states or continue the pending world-map action. */
void func_80061F18(void)
{
    switch (D_80182DB4)
    {
    case 3:
        if (D_8011D4FC != -1)
        {
            D_80182DB4 = 0;
        }
        else
        {
            func_80063BE0();
        }
        break;
    case 4:
        if (D_80182DE0 != 0)
        {
            D_80182DB4 = 0;
        }
        else
        {
            func_80063BE0();
        }
        break;
    case 5:
        if (D_8011CF44 == 0)
        {
            D_80182DB4 = 0;
        }
        else
        {
            func_80063BE0();
        }
        break;
    case 6:
        if (D_80139230 != 0)
        {
            D_80182DB4 = 0;
        }
        else
        {
            func_80063BE0();
        }
        break;
    case 7:
        if (D_8011CF18 == 3)
        {
            D_80182DB4 = 0;
        }
        else
        {
            func_80063BE0();
        }
        break;
    }
}
