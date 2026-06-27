#include "main.h"
#include "cd_resources.h"
#include "cd.h"
#include "movie.h"

/* Prototypes not yet in any project header, or whose existing header declarations
 * conflict (e.g. decomp1.h has stale/conflicting signatures for several of these).
 * Keep them local until the headers are unified. */
void InitializeControllers(undefined1 controllerMode);
extern void SpuInit(void);
void McxStartCom();
void func_8004FD14(s32);
void func_80051FBC(u32);
void InitVSyncController(void);
undefined* FUN_80015c18(void);
u32* FUN_80015c28(void);
undefined4 FUN_80021fbc(void);
void FUN_80011638(int param_1);
s32 akao_cmd_f0(void);

/**
 * decomp.me (100%) https://decomp.me/scratch/Tc7j3
 */
void Main(void)
{
    RECT rect;
    MenuLayout* menu_layout;
    u8* scratch_base;
    u32* overlay_arg;
    int field_config;
    s32 raw_config;
    long cd_stop_ret;
    u32 state;
    int prev_state;
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
    g_layout_flag = 0;
    g_layout_sub_mode = -1;
    g_layout_option = -1;
    g_scene_mode = 0;
    g_save_slot_index = 7;
    D_800473E0 = 0;
    D_8003EC8C = 0xB;
    D_80042FD0 = 0x13;
    g_gameState = 8U;
    FUN_80015c28();
    cdrom_stream(15, g_overlayLoadAddress);
    cdrom_wait_queue_empty();
    RunCheckPS(0x80100000);
    DrawSync(0);
    VSync(0);
    g_previousGameState = 0xFF;
    while (1)
    {
        {
            state = g_gameState;
            scratch_base = (u8*)0x80040000;
            switch (state)
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
                g_field_entry_flag = 0;
                *((u32*)(scratch_base - 0x1378)) = 0;
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
                g_gameState = FUN_80060814();
                akao_cmd_f0();
                akao_cmd_f1();
                if (((g_gameState != 2) && (g_gameState != 9)) && (g_gameState != 10))
                {
                    FUN_80011638(g_music_track_table[g_music_track_index]);
                }
                g_layout_sub_mode = -1;
                g_previousGameState = 1;
                break;

            case 2:
            {
                cd_stop_ret = (long)func_80015C48();
                cdrom_stop();
                cdrom_stream(4, g_overlayLoadAddress);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                prev_state = 2;
                g_gameState = func_8004FC74(cd_stop_ret);
                DrawSync(0);
                VSync(0);
                g_previousGameState = prev_state;
                break;
            }

            case 3:
                FUN_80015c28();
                cdrom_stream(CD_RES_FIELD_BIN, g_overlayLoadAddress);
                cdrom_stream(CD_RES_GNAME_BIN, 0x80140000);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                func_800A3534();
                func_80051FBC(0);
                g_field_audio_timer = 0;
                overlay_arg = &g_gameDataBasePtr;
                menu_layout = (MenuLayout*)(((u8*)overlay_arg) - 0x5F0);
                g_gameState = run_overlay(0x80160000, (u32)overlay_arg, (u32)overlay_arg, ((*((u8*)(&menu_layout->slot_flags))) & 0x7F) + 4, 0, (u32)overlay_arg, 1);
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
                func_80051FBC(0);
                g_save_slot_index = 7;
                if (func_801400C4() != 0)
                {
                    g_gameState = 2;
                }
                else
                {
                    field_config = (u32)(raw_config = ((MenuLayout*)(((u8*)(&g_gameDataBasePtr)) - 0x5F0))->unk018);
                    field_config = field_config & 0xFE000000U;
                    field_config = field_config | 6;
                    *((u32*)(scratch_base - 0x1378)) = 6;
                    ((MenuLayout*)(((u8*)(&g_gameDataBasePtr)) - 0x5F0))->unk018 = field_config;
                    g_scene_mode = ((MenuLayout*)(((u8*)(&g_gameDataBasePtr)) - 0x5F0))->scene_mode;
                    g_field_entry_flag = ((MenuLayout*)(((u8*)(&g_gameDataBasePtr)) - 0x5F0))->field_flags;
                    g_layout_flag = ((MenuLayout*)(((u8*)(&g_gameDataBasePtr)) - 0x5F0))->layout_flags;
                    g_layout_option = ((MenuLayout*)(((u8*)(&g_gameDataBasePtr)) - 0x5F0))->option_id;
                    g_layout_sub_mode = ((MenuLayout*)(((u8*)(&g_gameDataBasePtr)) - 0x5F0))->sub_mode;
                    ((MenuLayout*)(((u8*)(&g_gameDataBasePtr)) - 0x5F0))->unk018 = field_config;
                    {
                        u32 tmp_u20 = ((MenuLayout*)(((u8*)(&g_gameDataBasePtr)) - 0x5F0))->music_track;
                        g_music_track_index = (u16)tmp_u20;
                    }
                    if ((((MenuLayout*)(((u8*)(&g_gameDataBasePtr)) - 0x5F0))->unk028 & 0xC) == 0xC)
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
                movie_play(0);
                g_gameState = 2;
                DrawSync(0);
                VSync(0);
                g_previousGameState = 8;
                break;
            }
        }
        if (g_gameState == 4)
        {
            g_gameState = 2;
        }
    }
}