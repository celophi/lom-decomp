#include "checkps.h"

/**
 * decomp.me link (91.21%) https://decomp.me/scratch/FyrJc
 */
void* func_80052218(void* arg0, s32* arg1, s32 arg2)
{
    s32 var_a0;
    SomeStruct* s = (SomeStruct*)arg0;
    s32 arg0_masked;

    /* Block 1: array indexing forces base→v0, index→v1, giving addu v1,v1,v0 */
    D_800890C0[arg2] |= 0x10000;

    /* Block 2: init simple structure members */
    s->u.byte.unk3 = 3;
    s->unk7 = 0x7C;
    s->unk5 = 0x80;
    s->unk6 = 0x80;
    s->unk4 = 0x80;

    /* Block 3: init unk8, unkA and compute var_a0 */
    var_a0 = arg2;
    s->unk8 = (u16)D_800894C0;
    s->unkA = (u16)D_800894C4;
    if (arg2 < 0)
        var_a0 = arg2 + 0xF;

    /* Block 4: compute unkC and unkD */
    s->unkC = (s8)((arg2 - ((var_a0 >> 4) * 0x10)) * 0x10);
    s->unkD = (s8)(arg2 & 0xF0);

    /*
     * Write unkE then update unk0 inline (no pre-loaded temp).
     * With v0 occupied by 0x7FC0, the compiler hoists lw v1,0(a3)
     * before the sh, then loads *arg1 into v0 — exactly matching
     * the target register allocation and instruction order.
     */
    s->unkE = 0x7FC0;
    s->u.unk0 = (s->u.unk0 & 0xFF000000) | (*arg1 & 0xFFFFFF);

    /* Compute masked arg0 (a3 still un-incremented) then advance pointer */
    arg0_masked = (s32)arg0 & 0xFFFFFF;
    arg0 = (char*)arg0 + 0x14;

    /* Global updates */
    {
        s32 old_c0 = D_800894C0;
        s32 new_c0 = old_c0 + 0x10;
        s32 cond   = (old_c0 + 0x20) < 0x280;
        D_800894C0 = new_c0;
        *arg1 = (*arg1 & 0xFF000000) | arg0_masked;
        if (!cond) {
            D_800894C0 = D_800894CC;
            D_800894C4 += 0x10;
        }
    }
    return arg0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/PpMnG
 */
void func_80052320(void) 
{
    s32 var_a0;
    u32 *var_v1;

    D_800894C8 = (s32)&D_800810C0;
    var_a0 = 0;
    var_v1 = (u32*)&D_800890C0;

    while (var_a0 < 0x100) {
        var_a0++;
        *var_v1 = (u32)(*(u16*)var_v1);
        var_v1++;
    }
}


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