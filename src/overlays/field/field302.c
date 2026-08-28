#include "common.h"

extern s32 D_80117EE0;
extern void *D_80117EEC;
extern s32 D_8011F308;
extern void *D_8011F324;
extern s32 D_8011F328;

/**
 * @see decomp.me (100%)
 */
void *func_800A4348(s32 arg0, void *arg1)
{
    void *new_var;
    void *ptr;

    if (D_80117EE0 == 0)
    {
        if (arg0 & 0x800)
        {
            new_var = (void *)0x801DC800;
            ptr = new_var;
        }
        else
        {
            ptr = (void *)0x801DC000;
        }
        D_80117EEC = ptr;
        if (arg0 == 0)
        {
            D_8011F308 = 2;
        }
        else
        {
            D_8011F308 = 1;
        }
        D_8011F328 = arg0;
        new_var = arg1;
        D_8011F324 = new_var;
        D_80117EE0 = 1;
        return ptr;
    }
    return (void *)0;
}
