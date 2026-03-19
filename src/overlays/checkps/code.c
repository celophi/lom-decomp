#include "checkps.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/bzlSh
 */
s32 FUN_8004fd14(s32 arg0) 
{
    func_80050080();
    func_8004FEE8(arg0);
    
    do {
        func_8004FD68(arg0);
    } while (D_8005D060 == 0);
    
    return 8;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/lBhMG
 */
void func_8004FD68(int arg0) 
{
    RECT rect;
    u_long* temp_s1;
    void* var_s0;
    void* var_v0;

    DrawSync(0);
    VSync(0);
    
    var_s0 = arg0;
    func_800158E0();
    
    rect.w = 320;
    rect.x = 0;
    rect.y = 0;
    rect.h = 472;
    
    ClearImage(&rect, 0, 0, 0);
    ClearOTagR(var_s0 + 0x40, 0x1000);
    ClearOTagR(var_s0 + 0xBD0C, 0x1000);
    PutDispEnv(var_s0 + 0x4040);
    func_800157DC();
    SetDispMask(1);
    
    do {
        temp_s1 = (u_long*)(var_s0 + 0x40);
        ClearOTagR(temp_s1, 0x1000);
        *(u32*)(var_s0 + 0x80B8) = (s32) (var_s0 + 0x40B8);
        func_80052320();
        func_80050258(var_s0);
        func_800505B4(var_s0);
        func_80050570();
        func_8005235C();
        DrawSync(0);
        func_800157B0(2);
        VSync(2);
        ClearImage(var_s0 + 0x40B0, 0, 0, 0);
        var_v0 = arg0;
        if (var_s0 == var_v0) {
            var_v0 = var_s0 + 0xBCCC;
        }
        var_s0 = var_v0;
        PutDispEnv(var_s0 + 0x4040);
        PutDrawEnv(var_s0 + 0x4054);

        temp_s1 = (u8*)temp_s1 + 0x3FFC;
        DrawOTag(temp_s1);
        
        func_800157DC();
        CD_UpdateAndProcessQueue();
    } while (D_8005D060 == 0);
    
    func_800158E0();
    VSync(0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/tlBGm
 */
void func_8004FEE8(int arg0)
{
    RECT rect;

    func_8001D5AC(0x5DC);
    func_8001D58C(0xA0, 0x78);
    *(u16*)(arg0 + 0x40B0) = 0;
    *(u16*)(arg0 + 0x40B2) = 0;
    *(u16*)(arg0 + 0x40B4) = 0x140;
    *(u16*)(arg0 + 0x40B6) = 0xF0;
    *(u16*)(arg0 + 0xFD7C) = 0;
    *(u16*)(arg0 + 0xFD7E) = 0xE8;
    *(u16*)(arg0 + 0xFD80) = 0x140;
    *(u16*)(arg0 + 0xFD82) = 0xF0;
    DrawSync(0);
    VSync(0);
    
    rect.w = 0x400;
    rect.x = 0;
    rect.y = 0;
    rect.h = 0x200;
    
    ClearImage(&rect, 0, 0, 0);
    SetDefDispEnv(arg0 + 0x4040, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(arg0 + 0xFD0C, 0, 0xE8, 0x140, 0xF0);
    SetDefDrawEnv(arg0 + 0x4054, 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv(arg0 + 0xFD20, 0, 8, 0x140, 0xE0);
    *(u8*)(arg0 + 0xFD36) = 0;
    *(u8*)(arg0 + 0x406A) = 0;
    
    rect.x = 0x3C0;
    rect.w = 0x40;
    rect.y = 0;
    rect.h = 0x100;
    
    ClearImage(&rect, 0, 0, 0);
    func_8005239C();
    func_80050228();
    func_80050554(0x100, 0x100, 0x100, 0x14);
    func_800506D0();
    D_8005D060 = 0;
    func_80050A0C();
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/4wbZH
 */
void func_80050080(void)
{
    u8* src;
    u8* dst;
    u32 count;
    s32* ref;
    u32* offs;
    
    if (((((D_80042FB0 == 2) || (D_80042FB0 == 3)) || (D_80042FB0 == 0)) || (D_80042FB0 == 6)) || (D_80042FB0 == 7) || (D_80042FB0 == 5))
    {
        return;
    }
    
    ref = &D_80061088;
    *ref = 0x8013C000;

    offs = &D_80052428;
    offs++;
    
    src = (u8 *) &D_80052428 + offs[0];
    dst = (u8*) 0x8013C000;
    count = offs[1] - offs[0];
    
    bcopy(src, dst, count);
    func_80021FFC(*ref);
    func_80022AE8((u32)&D_80052428 + offs[1], 1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/Ptv27
 */
void func_80050138(s32 arg0) 
{
    u32* offs;
    u8* ref;

    FUN_800141ec((arg0 + 0x17) & 0xFFFF, 0x80180000);
    CD_WaitForQueueEmpty();

    offs = (u32*)0x80180004;
    ref = (u8*)0x80180000;

    bcopy(ref + offs[0], &D_8005D088, offs[1] - offs[0]);
    func_80022AE8(offs[1] + (u32)ref, 1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/K0uKO
 */
void func_800501AC(void) 
{
    func_80022068(0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/2R9zp
 */
void func_800501CC(void) 
{
    func_80022040(&D_8005D088);
    FUN_8002279c(0, 0x7F);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/Fklyd
 */
void func_800501FC(u32 arg1, u32 arg2, u32 arg3) 
{
    func_8002216C(arg1, 0, arg2, arg3);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/i9Kyk
 */
void func_80050228(void) 
{
    u32* addrA = &D_8005D078;
    u32* addrB = &D_8005D068;
    
    *addrA = 0;
    *(addrA + 1) = 0;
    *(addrA + 2) = 0;
    
    *addrB = 0;
    *(addrB + 1) = 0;
    *(addrB + 2) = 0;
    *(addrB + 3) = 0;
}

/**
 * decomp.me link (83.77%) https://decomp.me/scratch/o9VAE
 */
void func_80050258(s32* arg0) {
    s32 temp_a0, temp_a2, temp_v1;
    s32 var_a1;
    u8 var_v0;
    u32* temp_t4;
    u32* ref;
    s32* addrA = &D_8005D068;
    s32* addrB = &D_8005D078;

    // Load pointer stored at arg0 + 0x80b8 (8238 * 4)
    ref = *(u32**)(arg0 + 8238);

    if (*(addrA + 3) != 0) {
        temp_a2 = (*(addrA) - *(addrB)) / *(addrA + 3);
        temp_a0 = (*(addrA + 1) - *(addrB + 1)) / *(addrA + 3);
        temp_v1 = (*(addrA + 2) - *(addrB + 2)) / *(addrA + 3);
        *(addrA + 3) = *(addrA + 3) - 1;
        *(addrB) = *(addrB) + temp_a2;
        *(addrB + 1) = *(addrB + 1) + temp_a0;
        *(addrB + 2) = *(addrB + 2) + temp_v1;
    } else {
        *(addrB) = *(addrA);
        *(addrB + 1) = *(addrA + 1);
        *(addrB + 2) = *(addrA + 2);
    }

    // If all three are 0x100, skip the rest
    if (*(addrB) == 0x100 && *(addrB + 1) == *(addrB) && *(addrB + 2) == *(addrB + 1)) {
        goto end;
    }

    // Store the three bytes at ref+4, ref+5, ref+6
    if (*(addrB) >= 0x101) {
        ((u8*)ref)[4] = (u8)(*(addrB) - 1);
        ((u8*)ref)[5] = (u8)(*(addrB + 1) - 1);
        var_v0 = (u8)(*(addrB + 2) - 1);
        goto block_18;
    }

    if (*(addrB) == 0x100) {
        ((u8*)ref)[4] = 0;
    } else {
        ((u8*)ref)[4] = ~(u8)(*(addrB));
    }

    if (*(addrB + 1) == 0x100) {
        ((u8*)ref)[5] = 0;
    } else {
        ((u8*)ref)[5] = ~(u8)(*(addrB + 1));
    }

    if (*(addrB + 2) == 0x100) {
        ((u8*)ref)[6] = 0;
    } else {
        var_v0 = ~(u8)(*(addrB + 2));
block_18:
        ((u8*)ref)[6] = var_v0;
    }

    // Common stores after the three bytes
    ((u8*)ref)[3] = 3;
    ((u8*)ref)[7] = 0x62;
    *(u16*)((u8*)ref + 12) = 0x140;  // halfword at offset 12
    *(u16*)((u8*)ref + 8)  = 0;      // halfword at offset 8
    *(u16*)((u8*)ref + 10) = 0;      // halfword at offset 10
    *(u16*)((u8*)ref + 14) = 0xF0;   // halfword at offset 14

    // First word operation: update *ref using arg0+16
    *ref = (*ref & 0xFF000000) | (*(u32*)(arg0 + 16) & 0x00FFFFFF);
    *(u32*)(arg0 + 16) = (*(u32*)(arg0 + 16) & 0xFF000000) | ((u32)ref & 0x00FFFFFF);

    // Prepare for next block
    temp_t4 = (u32*)((u8*)ref + 0x10);  // byte offset 16

    var_a1 = 0x25;
    if (*(addrB) < 0x101) {
        var_a1 = 0x45;
    }

    ((u8*)temp_t4)[3] = 1;
    *(u32*)((u8*)temp_t4 + 4) = var_a1 | 0xE1000000;

    // Update word at temp_t4
    *temp_t4 = (*temp_t4 & 0xFF000000) | (*(u32*)(arg0 + 16) & 0x00FFFFFF);
    *(u32*)(arg0 + 16) = (*(u32*)(arg0 + 16) & 0xFF000000) | ((u32)temp_t4 & 0x00FFFFFF);

    // Advance ref to temp_t4 + 8 (byte offset 24)
    ref = (u32*)((u8*)temp_t4 + 8);

end:
    *(u32*)(arg0 + 8238) = ref;
}