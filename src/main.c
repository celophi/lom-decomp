#include "main.h"

u32 D_800102AC;

/**
 * decomp.me link (91.90%) https://decomp.me/scratch/No2jL
 */
void Main(void) 
{
    RECT rect;
    s32 result;
    u32 gameState;
    
    u32 var_v0_3;
    tempU* temp_s2;
    u32* streamDst;
    u32* ptrA;
    u32* ptrB;
    u32* ptrC;
    u32 val;
     s32 new_var;
    s16 *new_var3;
    u32 *new_var4;
    
    __main();
    SetMem(2);
    SetConf(0x10, 4, 0x801FFF00);
    ResetGraph(0);
    SetGraphDebug(0);
    SetDispMask(0);
    FUN_80016784();
    ResetCallback();
    SetVideoMode(0);
    SsUtReverbOff();

    ptrB = &D_8003EC88;
    
    FUN_80015c18();
    CD_Initialize();
    InitGeom();
    InitCARD(0);
    StartCARD();
    _bu_init();
    McxStartCom();
    ChangeClearPAD(0);
    InitializeControllers(0);
    CD_InitResources(0x18, 0xB598);
    FUN_80021fbc();

    // MAP/FDATA/FONT.PRS
    CD_StreamData(0xB2, 0x801E1200);
    InitVSyncController();
    srand(1);
    
    D_80042FC4 = 0;
    D_80046FD8 = -1;
    
    D_8003EC94 = -1;
    D_8003EC90 = 0;
    D_8003EC9C = 7;
    D_800473E0 = 0;
    D_8003EC8C = 0xB;
    D_80042FD0 = 0x13;
    g_gameState = 8U;
    new_var4 = &D_800102AC;
    FUN_80015c28();

    
    streamDst = new_var4;
    
    // BIN/CHECKPS.BIN
    CD_StreamData(15, *streamDst);
    CD_WaitForQueueEmpty();
    FUN_8004fd14(0x80100000);
    DrawSync(0);
    
    VSync(0);
    
    ptrC = &g_previousGameState;
    *ptrC = 0xFF;
    ptrA = &D_800435C8;
    temp_s2 = (tempU*)(*ptrA);
    temp_s2 = (tempU*)((u8*)temp_s2  - 0x5F0);
    
    
    
    while (TRUE) 
    {
        
    gameState = g_gameState;
    do {
        
        switch (gameState) 
            {
            case 0:                                 /* switch 1 */
            case 9:                                 /* switch 1 */
            case 10:                                /* switch 1 */
                SetDispMask(0);
                VSync(0);
                DrawSync(0);
                FUN_80015c28();

                // BIN/FIELD.BIN
                CD_StreamData(2, *streamDst);
                
                if (g_gameState != 0) 
                {
                    // BIN/MOVIE.BIN
                    CD_StreamData(11, 0x80140000);
                    CD_WaitForQueueEmpty();
                    
                    if (g_gameState == 9) 
                    {
                        FUN_80140018(1);
                    } 
                    else 
                    {
                        FUN_80140018(2);
                        FUN_80140018(3);
                        FUN_80140018(4);
                    }
                    
                } 
                else
                {
                    CD_WaitForQueueEmpty();
                }
                
                D_80042FCC = 0;
                *ptrB = 0; // extra lui?
                g_gameState = FUN_80015c58();
                
                FUN_80022aa8();
                FUN_80022ac8();
                FUN_8002279c(0, 0x7F);
                g_previousGameState = 0;
                break;
            case 1:                                 /* switch 1 */
                FUN_80015c38();
                CD_StreamData(3, *(streamDst + 171));
                GFX_Transition(0);
                
                rect.x = 0;
                rect.y = 0;
                rect.w = 320;
                rect.h = 464;
                
                ClearImage(&rect, 0, 0, 0);
                DrawSync(0);
                VSync(0);
                FUN_80022aa8();
                FUN_80022ac8();
                CD_WaitForQueueEmpty();
                
                *(u32*)((u8*)gameState + 0x522C) = FUN_80060814();
                
                FUN_80022aa8();
                FUN_80022ac8();

                if (*(u32*)((u8*)gameState + 0x522C) != 2 && *(u32*)((u8*)gameState + 0x522C) != 9 && *(u32*)((u8*)gameState + 0x522C) != 10) {
                    FUN_80011638(*(u8*)(D_80046FDE + *D_800351A0));
                }
                
                D_80046FD8 = -1;
                *(u32*)(ptrC + 3052) = 1;
                break;
            case 2:                                 /* switch 1 */
                func_80015C48();
                CD_Stop();
                CD_StreamData(4, *(streamDst + 171));
                GFX_Transition(0);
                CD_WaitForQueueEmpty();
                *(u32*)(*(u8*)gameState + 0x522C) = func_8004FC74();
                DrawSync(0);
                VSync(0);
                *(u32*)(ptrC + 3052) = 2;
                break;
            case 3:                                 /* switch 1 */
                FUN_80015c28();
                CD_StreamData(2, *(streamDst + 171));
                CD_StreamData(5, 0x80140000);
                GFX_Transition(0);
                CD_WaitForQueueEmpty();
                func_800A3534();
                func_80051FBC(0);
                val = temp_s2->u_608;
                D_8003EC98 = 0;
                *(u32*)(*(u8*)gameState + 0x522C) = func_80140004(0x80160000, (u32)ptrA, (u32)ptrA, (val & 0x7F) + 4, 0, (u32)ptrA, 1);
                DrawSync(0);
                VSync(0);
                *(u32*)(ptrC + 3052) = 3;
                break;
            case 5:                                 /* switch 1 */
                FUN_80015c28();
                CD_StreamData(0xE, *(streamDst + 171));
                GFX_Transition(0);
                CD_WaitForQueueEmpty();
                *(u32*)((*(u8*)gameState + 0x522C)) = func_8004FC8C(0x80170000);
                DrawSync(0);
                VSync(0);
                *(u32*)(ptrC + 3052) = 5;
                break;
            case 7:                                 /* switch 1 */
                FUN_80015c28();
                CD_StreamData(2, *(streamDst + 171));
                CD_StreamData(0x10, 0x80140000);
                GFX_Transition(0);
                CD_WaitForQueueEmpty();
                func_80051FBC(0);
                D_8003EC9C = 7;
                if (func_801400C4() != 0) 
                {
                    *(u32*)((u8*)gameState + 0x522C) = 2;
                } 
                else 
                {
                    *(u32*)(ptrB - 1246) = 6;
                    
                    
                    
                    D_8003EC90 = temp_s2->u_0x24;
                    new_var = 0xFE000000;
                    temp_s2->u_0x18 = (s32) ((temp_s2->u_0x18 & new_var) | 6);

                    val = temp_s2->u_0x20;
                    D_80042FCC = (s32) temp_s2->u_0x26;
                    new_var3 = &temp_s2->u_0x1C;
                    D_80042FC4 = (s32) temp_s2->u_0x27;
                    
                     D_8003EC94 = (s32) (*new_var3);
                    new_var = (s32) temp_s2->u_0x1E; //cast is correct
                    D_80046FD8 = new_var;
                    
                    D_80046FDE = (u16)val;
                    result = 5;
                    new_var = temp_s2->u_0x28;
                    if (((new_var) & 0xC) != 0xC)
                    {
                        GFX_Transition(0);
                        
                        result = FUN_80015c58();
                    }
                    *(u32*)((u8*)gameState + 0x522C) = result;
                }
                DrawSync(0);
                VSync(0);
                *(u32*)(ptrC + 3052) = 0;
                break;
            case 8:                                 /* switch 1 */
                func_80015C48();
                CD_StreamData(0xB, 0x80140000);
                GFX_Transition(0);
                CD_WaitForQueueEmpty();
                FUN_80140018(0);
                *(u32*)((u8*)gameState + 0x522C) = 2U;
                DrawSync(0);
                VSync(0);
                *(u32*)(ptrC + 3052) = 8;
                break;
            }
            
        
    } while (g_gameState != 4);
        g_gameState = 2U;
    }
}
