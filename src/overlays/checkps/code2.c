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

/**
 * decomp.me link (100%) https://decomp.me/scratch/2UOve
 */
void func_8005239C(void) 
{
    s32 i;
    s32 *p;
    RECT rect;

    // First loop: zero 0x100 words from D_800890C0 to D_800890C0+0x3FC
    i = 0xFF;
    // Force two‑step address calculation: base address + 0x3FC
    p = &D_800890C0;
    p = (s32*)((u_long)p + 0x3FC);
    
    while (i >= 0) {
        *p = 0;
        p--;
        i--;
    }

    // Second loop: zero 0x8000 bytes from D_800810C0
    for (i = 0; i <= 0x7FFF; i++) {
        D_800810C0[i] = 0;
    }

    // Assign RECT fields in the exact order required by the target assembly:
    // y, w, x, then h (h goes into the delay slot after the x store)
    rect.y = 0x1FF;
    rect.w = 0x10;
    rect.x = 0;
    rect.h = 1;

    LoadImage(&rect, (u_long*)&D_8005D054);
}