#include "main.h"
#include "cd_resources.h"

u32 g_overlayLoadAddress;

/**
 * decomp.me link (98.23%) https://decomp.me/scratch/iAIgl
 */
volatile void Main(void)
{
    RECT rect;
    int new_var2;
    MenuLayout* temp_s2;
    u32* ptrA;
    int new_var;
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
    cdrom_stream(15, g_overlayLoadAddress);
    cdrom_wait_queue_empty();
    func_8004FD14(0x80100000);
    DrawSync(0);
    VSync(0);
    g_previousGameState = 0xFF;
    ptrA = &g_gameDataBasePtr;
    temp_s2 = (MenuLayout*)(((u8*)ptrA) - 0x5F0);
    while (1)
    {
        new_var2 = 0;
        do
        {
            switch (g_gameState)
            {
            case 0:

            case 9:

            case 10:
                SetDispMask(0);
                VSync(0);
                DrawSync(0);
                FUN_80015c28();
                cdrom_stream(CD_RES_FIELD_BIN, g_overlayLoadAddress);
                if (g_gameState != 0)
                {
                    cdrom_stream(11, 0x80140000);
                    cdrom_wait_queue_empty();
                    if ((unsigned long)(g_gameState == 9))
                    {
                        func_80140018(1);
                    }
                    else
                    {
                        func_80140018(2);
                        func_80140018(3);
                        func_80140018(4);
                    }
                }
                else
                {
                    cdrom_wait_queue_empty();
                }
                D_80042FCC = 0;
                D_8003EC88 = 0;
                g_gameState = FUN_80015c58();
                akao_cmd_f0();
                akao_cmd_f1();
                akao_cmd_c0(0, 0x7F);
                g_previousGameState = 0;
                break;

            case 1:
                FUN_80015c38();
                cdrom_stream(3, g_overlayLoadAddress);
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
                g_gameState = func_80060814();
                akao_cmd_f0();
                akao_cmd_f1();
                if (((g_gameState != 2) && (g_gameState != 9)) && (g_gameState != 10))
                {
                    FUN_80011638(D_800351A0[D_80046FDE]);
                }
                D_80046FD8 = -1;
                g_previousGameState = 1;
                break;

            case 2:
            {
                cd_stop_ret = (long)func_80015C48();
                if (1)
                {
                    cdrom_stop();
                }
                cdrom_stream(4, g_overlayLoadAddress);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                g_gameState = func_8004FC74(cd_stop_ret);
                DrawSync(0);
                VSync(0);
                g_previousGameState = 2;
                break;
            }

            case 3:
                FUN_80015c28();
                cdrom_stream(CD_RES_FIELD_BIN, g_overlayLoadAddress);
                cdrom_stream(CD_RES_GNAME_BIN, 0x80140000);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                func_800A3534();
                field_scene_reset(0);
                D_8003EC98 = 0;
                g_gameState =
                    func_80140004(0x80160000, (u32)ptrA, (u32)ptrA, (*(u8*)&temp_s2->slot_flags & 0x7F) + 4, 0, (u32)ptrA, 1);
                DrawSync(0);
                VSync(0);
                g_previousGameState = 3;
                break;

            case 5:
                FUN_80015c28();
                cdrom_stream(14, g_overlayLoadAddress);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                g_gameState = func_8004FC8C(0x80170000);
                DrawSync(0);
                VSync(0);
                g_previousGameState = 5;
                break;

            case 7:
                FUN_80015c28();
                cdrom_stream((float)2, g_overlayLoadAddress);
                cdrom_stream(16, 0x80140000);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                field_scene_reset(new_var2);
                D_8003EC9C = 7;
                if (func_801400C4() != 0)
                {
                    g_gameState = 2;
                }
                else
                {
                    new_var = (u32)temp_s2->unk018;
                    new_var = new_var & 0xFE000000U;
                    new_var = new_var | 6;
                    D_8003EC88 = 6;
                    temp_s2->unk018 = new_var;
                    D_8003EC90 = temp_s2->unk024;
                    D_80042FCC = temp_s2->unk026;
                    D_80042FC4 = temp_s2->unk027;
                    D_8003EC94 = temp_s2->unk01C;
                    D_80046FD8 = temp_s2->unk01E;
                    temp_s2->unk018 = new_var;
                    {
                        u32 tmp_u20 = temp_s2->unk020;
                        D_80046FDE = (u16)tmp_u20;
                    }
                    if ((temp_s2->unk028 & 0xC) == 0xC)
                    {
                        g_gameState = 5;
                    }
                    else
                    {
                        GFX_Transition(0);
                        g_gameState = FUN_80015c58();
                    }
                }
                DrawSync(0);
                VSync(0);
                g_previousGameState = 0;
                break;

            case 8:
                func_80015C48();
                cdrom_stream(11, 0x80140000);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                func_80140018(0);
                g_gameState = 2;
                DrawSync(0);
                VSync(0);
                g_previousGameState = 8;
                break;
            }

        } while (g_gameState != 4);
        g_gameState = 2;
    }
}