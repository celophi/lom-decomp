#include "main.h"
#include "cd_resources.h"
#include "cdrom.h"
#include "movie.h"
#include "screen_transition.h"
#include "game_audio.h"
#include "controller.h"
#include "overlay_memory.h"
#include "field_runtime.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libapi.h"
#include "psyq/libetc.h"

extern u32 g_overlayLoadAddress;
extern u8 g_music_track_table[];
extern s32 D_8003EC8C;
extern s32 D_80042FD0;
extern s32 D_800473E0;

void __main(void);
void _bu_init(void);
extern void SpuInit(void);
void McxStartCom();
void func_8004FD14(s32);
undefined4 FUN_80021fbc(void);
s32 akao_cmd_f0(void);
u32 func_8004FC8C(u32);
u32 run_overlay(u32, u32, u32, s32, s32, u32, s32);
s32 func_801400C4(void);
void srand(u_int param_1);
s32 func_8004FC74(s32);

/** @brief Fixed base address for scratch/system register access.
 *  @note g_field_scene_config lives at (SCRATCH_BASE - 0x1378). */
#define SCRATCH_BASE 0x80040000

/**
 * @brief Main game loop — initializes hardware, then runs the top-level
 *        state machine forever.
 *
 * State transitions:
 *   - 0/9/10 (FIELD): Load FIELD.BIN overlay. States 9/10 play attract
 *     movies first. Transitions to the field scene loop via run_field_scene.
 *   - 1 (WORLD_MAP): Load WMAP.BIN, run world map, may play map music.
 *   - 2 (TITLE): Load TITLE.BIN, run title screen (new game / continue).
 *   - 3 (GNAME): Load FIELD.BIN + GNAME.BIN, run name-entry overlay.
 *   - 5 (WORLD_SELECT): Load WSEL.BIN, run world select overlay.
 *   - 7 (MENU_LOAD): Load FIELD.BIN + CLOAD.BIN, run save/continue menu.
 *     Copies MenuLayout companion fields from the loaded save layout into
 *     the globals consumed by the field overlay and run_field_scene.
 *   - 8 (INTRO_MOVIE): Play intro movie, then jump to title.
 *   - 4 is a transient dead state immediately redirected to TITLE.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Tc7j3
 */
void Main(void)
{
    RECT rect;
    MenuLayout* menu_layout;
    u8* scratch_base;
    u32* overlay_arg;
    int field_config;
    s32 raw_config;
    long title_menu_buffers;
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
    get_overlay_load_base();
    cdrom_init();
    InitGeom();
    InitCARD(0);
    StartCARD();
    _bu_init();
    McxStartCom();
    ChangeClearPAD(0);
    initialize_controllers(0);
    cdrom_load_resource_table(0x18, 0xB598);
    FUN_80021fbc();
    cdrom_stream(0xB2, 0x801E1200);
    initialize_controller_vsync();
    srand(1);
    g_layout_flag = 0;
    g_layout_sub_mode = -1;
    g_layout_option = -1;
    g_scene_mode = 0;
    g_save_slot_index = 7;
    D_800473E0 = 0;
    D_8003EC8C = 0xB;
    D_80042FD0 = 0x13;
    g_gameState = GAME_STATE_INTRO_MOVIE;
    get_field_render_buffers();
    cdrom_stream(CD_RES_CHECKPS_BIN, g_overlayLoadAddress);
    cdrom_wait_queue_empty();
    run_checkps(0x80100000);
    DrawSync(0);
    VSync(0);
    g_previousGameState = GAME_STATE_NONE;
    while (1)
    {
        {
            state = g_gameState;
            scratch_base = (u8*)SCRATCH_BASE;
            switch (state)
            {
            /* ---- Field entry (attract / demo / normal) ---- */
            case GAME_STATE_FIELD:

            case GAME_STATE_ATTRACT_1:

            case GAME_STATE_ATTRACT_2:
                SetDispMask(0);
                VSync(0);
                DrawSync(0);
                get_field_render_buffers();
                cdrom_stream(CD_RES_FIELD_BIN, g_overlayLoadAddress);
                if (g_gameState != GAME_STATE_FIELD)
                {
                    cdrom_stream(CD_RES_MOVIE_BIN, 0x80140000);
                    cdrom_wait_queue_empty();
                    if ((unsigned long)(g_gameState == GAME_STATE_ATTRACT_1))
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
                g_gameState = run_field_scene();
                akao_cmd_f0();
                akao_cmd_f1();
                akao_cmd_c0(0, 0x7F);
                g_previousGameState = GAME_STATE_FIELD;
                break;

            /* ---- World map ---- */
            case GAME_STATE_WORLD_MAP:
                get_world_map_overlay_end();
                cdrom_stream(CD_RES_WMAP_BIN, g_overlayLoadAddress);
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
                if (((g_gameState != GAME_STATE_TITLE) && (g_gameState != GAME_STATE_ATTRACT_1)) && (g_gameState != GAME_STATE_ATTRACT_2))
                {
                    load_and_play_song(g_music_track_table[g_music_track_index]);
                }
                g_layout_sub_mode = -1;
                g_previousGameState = GAME_STATE_WORLD_MAP;
                break;

            /* ---- Title screen ---- */
            case GAME_STATE_TITLE:
            {
                title_menu_buffers = (long)get_title_menu_buffers();
                cdrom_stop();
                cdrom_stream(CD_RES_TITLE_BIN, g_overlayLoadAddress);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                prev_state = GAME_STATE_TITLE;
                g_gameState = func_8004FC74(title_menu_buffers);
                DrawSync(0);
                VSync(0);
                g_previousGameState = prev_state;
                break;
            }

            /* ---- Name entry (GNAME overlay) ---- */
            case GAME_STATE_GNAME:
                get_field_render_buffers();
                cdrom_stream(CD_RES_FIELD_BIN, g_overlayLoadAddress);
                cdrom_stream(CD_RES_GNAME_BIN, 0x80140000);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                field_restore_entry_music();
                field_scene_reset(0);
                g_field_audio_timer = 0;
                overlay_arg = &g_gameDataBasePtr;
                menu_layout = (MenuLayout*)(((u8*)overlay_arg) - 0x5F0);
                g_gameState = run_overlay(0x80160000, (u32)overlay_arg, (u32)overlay_arg, ((*((u8*)(&menu_layout->slot_flags))) & 0x7F) + 4, 0, (u32)overlay_arg, 1);
                DrawSync(0);
                VSync(0);
                g_previousGameState = GAME_STATE_GNAME;
                break;

            /* ---- World select ---- */
            case GAME_STATE_WORLD_SELECT:
                get_field_render_buffers();
                cdrom_stream(CD_RES_WSEL_BIN, g_overlayLoadAddress);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                g_gameState = func_8004FC8C(0x80170000);
                DrawSync(0);
                VSync(0);
                g_previousGameState = GAME_STATE_WORLD_SELECT;
                break;

            /* ---- Menu / load save ---- */
            case GAME_STATE_MENU_LOAD:
                get_field_render_buffers();
                /* (float)2 is a codegen artifact: the compiler uses an FP
                 * register to hold the CD_RES_FIELD_BIN value. The cast
                 * forces the same register allocation as the original. */
                cdrom_stream((float)CD_RES_FIELD_BIN, g_overlayLoadAddress);
                cdrom_stream(CD_RES_CLOAD_BIN, 0x80140000);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                field_scene_reset(0);
                g_save_slot_index = 7;
                if (func_801400C4() != 0)
                {
                    g_gameState = GAME_STATE_TITLE;
                }
                else
                {
                    /* Mask 0xFE000000 preserves the upper 25 bits of unk018
                     * (likely a base address or segment), OR-ing in 6 as the
                     * field overlay sub-mode. Written both to the scratch
                     * register at (SCRATCH_BASE - 0x1378) = g_field_scene_config and
                     * back into the MenuLayout struct. */
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
                        g_gameState = GAME_STATE_WORLD_SELECT;
                    }
                    else
                    {
                        GFX_Transition(0);
                        g_gameState = run_field_scene();
                    }
                }
                DrawSync(0);
                VSync(0);
                g_previousGameState = GAME_STATE_FIELD;
                break;

            /* ---- Intro movie, then title ---- */
            case GAME_STATE_INTRO_MOVIE:
                get_title_menu_buffers();
                cdrom_stream(CD_RES_MOVIE_BIN, 0x80140000);
                GFX_Transition(0);
                cdrom_wait_queue_empty();
                movie_play(0);
                g_gameState = GAME_STATE_TITLE;
                DrawSync(0);
                VSync(0);
                g_previousGameState = GAME_STATE_INTRO_MOVIE;
                break;
            }
        }
        /* State 4 is a transient dead state — immediately redirect to title. */
        if (g_gameState == 4)
        {
            g_gameState = GAME_STATE_TITLE;
        }
    }
}
