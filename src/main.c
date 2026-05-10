#include "main.h"

u32 g_overlayLoadAddress;

/**
 * decomp.me link (91.90%) https://decomp.me/scratch/No2jL
 */
void Main(void)
{
    u32* new_var3;
    RECT rect;

    u32* streamDst = &g_overlayLoadAddress;
    tempU* temp_s2 = (tempU*)(((u8*)(&g_gameDataBasePtr)) - 0x5F0);
    u32 gameState;
    u32* ptrA = &g_gameDataBasePtr;
    long cd_stop_ret;

    __main();
    SetMem(2);
    SetConf(0x10, 4, 0x801FFF00);
    ResetGraph(0);
    SetGraphDebug(0);
    SetDispMask(0);
    _96_remove();
    ResetCallback();
    SetVideoMode(0);
    SpuInit();
    FUN_80015c18();
    cdrom_init();
    InitGeom();
    InitCARD(0);
    StartCARD();
    _bu_init();
    McxStartCom();
    ChangeClearPAD(0);
    InitializeControllers(0);
    cdrom_load_resource_table(0x18, 0xB598);
    FUN_80021fbc();

    // MAP/FDATA/FONT.PRS
    cdrom_stream(0xB2, 0x801E1200);
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
    FUN_80015c28();
    // BIN/CHECKPS.BIN
    cdrom_stream(15, *streamDst);
    cdrom_wait_queue_empty();
    RunCheckPS(0x80100000);
    DrawSync(0);
    VSync(0);
    g_previousGameState = 0xFF;
    while (1)
    {
        gameState = g_gameState;
        new_var3 = streamDst;
        do
        {
            switch (gameState)
            {
            case 0:

            case 9:

            case 10:
                SetDispMask(0);
                VSync(0);
                DrawSync(0);
                FUN_80015c28();
                // BIN/FIELD.BIN
                cdrom_stream(2, *new_var3);
                if (g_gameState != 0)
                {
                    // BIN/MOVIE.BIN
                    cdrom_stream(11, 0x80140000);
                    cdrom_wait_queue_empty();
                    if (g_gameState == 9)
                    {
                        movie_play(1);
                    }
                    else
                    {
                        movie_play(2);
                        movie_play(3);
                        movie_play(4);
                    }
                }
                else
                {
                    cdrom_wait_queue_empty();
                }
                D_80042FCC = 0;
                g_gameState = FUN_80015c58();
                D_8003EC88 = 0;
                akao_cmd_f0();
                akao_cmd_f1();
                akao_cmd_c0(0, 0x7F);
                g_previousGameState = 0;
                break;

            case 1:
                FUN_80015c38();
                // BIN/WMAP.BIN
                cdrom_stream(3, *(new_var3 + 171));
                GFX_Transition(0);
                rect.x = 0;
                rect.y = 0;
                rect.w = 320;
                rect.h = 464;
                ClearImage(&rect, 0, 0, 0);
                DrawSync(0);
                VSync(0);
                akao_cmd_f0();
                akao_cmd_f1();
                cdrom_wait_queue_empty();
                *((u32*)(((u8*)(&g_gameState)) + 0x522C)) = FUN_80060814();
                akao_cmd_f0();
                akao_cmd_f1();
                if ((((*((u32*)(((u8*)(&g_gameState)) + 0x522C))) != 2) &&
                     ((*((u32*)(((u8*)(&g_gameState)) + 0x522C))) != 9)) &&
                    ((*((u32*)(((u8*)(&g_gameState)) + 0x522C))) != 10))
                {
                    FUN_80011638(*((u8*)(D_80046FDE + ((u32)(&D_800351A0)))));
                }
                D_80046FD8 = -1;
                *((u32*)(((u8*)(&g_previousGameState)) + 0x2FB0)) = 1;
                break;

            case 2:
            {
                func_80015C48();
                cd_stop_ret = cdrom_stop();
                // BIN/TITLE.BIN
                cdrom_stream(4, *(new_var3 + 171));
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                *((u32*)(((u8*)(&g_gameState)) + 0x522C)) = func_8004FC74(cd_stop_ret);
                DrawSync(0);
                VSync(0);
                *((u32*)(((u8*)(&g_previousGameState)) + 0x2FB0)) = 2;
                break;
            }

            case 3:
                FUN_80015c28();
                // BIN/FIELD.BIN
                cdrom_stream(2, *(new_var3 + 171));
                // BIN/GNAME.BIN
                cdrom_stream(5, 0x80140000);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                func_800A3534();
                func_80051FBC(0);
                D_8003EC98 = 0;
                *((u32*)(((u8*)(&g_gameState)) + 0x522C)) =
                    run_overlay(0x80160000, (u32)ptrA, (u32)ptrA, (temp_s2->u_608 & 0x7F) + 4, 0, (u32)ptrA, 1);
                DrawSync(0);
                VSync(0);
                *((u32*)(((u8*)(&g_previousGameState)) + 0x2FB0)) = 3;
                break;

            case 5:
                FUN_80015c28();
                // BIN/WSEL.BIN
                cdrom_stream(14, *(new_var3 + 171));
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                *((u32*)(((u8*)(&g_gameState)) + 0x522C)) = func_8004FC8C(0x80170000);
                DrawSync(0);
                VSync(0);
                *((u32*)(((u8*)(&g_previousGameState)) + 0x2FB0)) = 5;
                break;

            case 7:
                FUN_80015c28();
                // BIN/FIELD.BIN
                cdrom_stream(2, *(new_var3 + 171));
                // BIN/CLOAD.BIN
                cdrom_stream(16, 0x80140000);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                func_80051FBC(0);
                D_8003EC9C = 7;
                if (func_801400C4() != 0)
                {
                    *((u32*)(((u8*)(&g_gameState)) + 0x522C)) = 2;
                }
                else
                {
                    D_8003EC88 = 6;
                    D_8003EC90 = temp_s2->u_0x24;
                    temp_s2->u_0x18 = (temp_s2->u_0x18 & 0xFE000000) | 6;
                    D_80042FCC = temp_s2->u_0x26;
                    D_80042FC4 = temp_s2->u_0x27;
                    D_8003EC94 = temp_s2->u_0x1C;
                    D_80046FD8 = temp_s2->u_0x1E;
                    D_80046FDE = temp_s2->u_0x20;
                    {
                        s32 result = 5;
                        if ((temp_s2->u_0x28 & 0xC) != 0xC)
                        {
                            GFX_Transition(0);
                            result = FUN_80015c58();
                        }
                        *((u32*)(((u8*)(&g_gameState)) + 0x522C)) = result;
                    }
                }
                DrawSync(0);
                VSync(0);
                *((u32*)(((u8*)(&g_previousGameState)) + 0x2FB0)) = 0;
                break;

            case 8:
                func_80015C48();
                
                // BIN/MOVIE.BIN
                cdrom_stream(11, 0x80140000);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                movie_play(0);
                *((u32*)(((u8*)(&g_gameState)) + 0x522C)) = 2;
                DrawSync(0);
                VSync(0);
                *((u32*)(((u8*)(&g_previousGameState)) + 0x2FB0)) = 8;
                break;
            }

        } while (g_gameState != 4);
        g_gameState = 2;
    }
}