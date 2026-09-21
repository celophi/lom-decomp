#include "common.h"

extern void func_80064F14(void);
extern s32 D_8011CF58;
extern s32 D_8011CF70;
extern s32 D_80139960;
extern s32 D_80182D70;

/**
 * @brief Change the current effect selection and retain the previous selection.
 * @param selection New effect selection.
 */
void func_8006D870(s32 selection)
{
    s32 previous_selection;

    if (selection != D_8011CF58)
    {
        if (D_80139960 != 0)
        {
            func_80064F14();
        }
        if (selection == 0)
        {
            D_80139960 = -1;
        }
        else
        {
            D_80139960 = 1;
        }
        D_8011CF70 = 0x10;
        previous_selection = D_8011CF58;
        D_8011CF58 = selection;
        D_80182D70 = previous_selection;
    }
}
