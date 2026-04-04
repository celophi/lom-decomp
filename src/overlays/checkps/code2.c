#include "checkps.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/PuSGD
 */
void func_8005235C(void)
{
    int new_var2;
    s32 var_a0;
    s32 *var_v1;

    var_a0 = 0;
    new_var2 = 0x10000;
    var_v1 = &D_800890C0;

    do {
        if (!((*var_v1) & new_var2)) {
            *var_v1 = 0;
        }

        var_a0 += 1;
        var_v1 += 1;
    } while (var_a0 < 0x100);
}
