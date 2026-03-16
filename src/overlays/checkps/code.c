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
