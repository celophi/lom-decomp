#include "decomp1.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/mXgky
 */
void FUN_8001160c(void) 
{
    func_800227D0(D_80042FB8, 0x12C, 0);
}

/**
 * decomp.me link (98.25%) https://decomp.me/scratch/Oy5Dh
 */
void FUN_80011638(s32 arg0)
{
    u32 base;
    s32 *info;
    u8 *ptr;
    s32 temp;
    
    if (arg0 == 0xFF)
    {
        return;
    }
    
    base = (arg0 + 0x93) & 0xFFFF;
    FUN_800141ec(base, 0x80180000);
    CD_WaitForQueueEmpty();

    info = (s32 *) 0x80180004;
    ptr = &D_80046FE0; 
    base = 0x80180000; 
   
    func_80016E7C((u_char *) ((*info) + base), ptr, info[1] - (*info)); 
    func_80022AE8(info[1] + base, 1); 
    
    temp = func_80022040(ptr);
    D_80042FB8 = temp;
    FUN_8002279c(temp, 0x7f);
}

/**
 * decomp.me link: (100%) https://decomp.me/scratch/DRBPP
 */
void FUN_800116d8(int arg0, s16 arg1, s16 arg2, s16 arg3) 
{
    D_80042FBC = arg0;
    
    if ((arg0 << 0x10) < 0) {
        D_80046FDC = arg0;
    }
    
    D_80042FBE = arg1;
    D_80042FC0 = arg2;
    D_80042FC2 = arg3;
}
