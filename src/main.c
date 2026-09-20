#include "main.h"
#include "akao.h"
#include "akao_cmd.h"
#include "checkps.h"
#include "cd_resources.h"
#include "cdrom.h"
#include "movie.h"
#include "title.h"
#include "gname.h"
#include "cload.h"
#include "wsel.h"
#include "world_map.h"
#include "screen_transition.h"
#include "game_audio.h"
#include "controller.h"
#include "overlay_memory.h"
#include "field_runtime.h"
#include "display.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/libapi.h"
#include "sdk/libetc.h"
#include "sdk/libspu.h"
#include "sdk/rand.h"

#define SECONDARY_OVERLAY_LOAD_ADDR 0x80140000

#define FIELD_ENTRY_MODE_RESUME 6
#define FIELD_CONFIG_FLAGS_MASK 0xFE000000U
#define NAME_SOURCE_MASK 0x7F
#define CD_RESOURCE_TABLE_LBA 24
#define CD_RESOURCE_TABLE_SIZE 46488
#define FIELD_FONT_LOAD_ADDRESS 0x801E1200

#define CHECKPS_RENDER_ADDRESS 0x80100000
#define GNAME_RENDER_ADDRESS 0x80160000
#define WSEL_RENDER_ADDRESS 0x80170000
#define BIOS_STACK_ADDRESS 0x801FFF00

extern s32 D_8003EC8C;
extern s32 D_80042FD0;

void __main(void);
void McxStartCom(void);

SavedGame g_saved_game;

/**
 * @brief Initialize the hardware and dispatch the game's overlays forever.
 * @see decomp.me (100%) https://decomp.me/scratch/Tc7j3
 */
void main_game_loop(void)
{
    RECT rect;
    SavedGameLayout* saved_layout;
    u32* entry_config;
    u8* player_name;
    u32 field_config;
    u32 music_track;
    TitleMenuContext* title_menu_buffers;
    u32 state;

    __main();
    SetMem(2);
    SetConf(16, 4, BIOS_STACK_ADDRESS);
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
    cdrom_load_resource_table(CD_RESOURCE_TABLE_LBA, CD_RESOURCE_TABLE_SIZE);
    akao_init();
    cdrom_stream(CD_RES_FIELD_FONT, FIELD_FONT_LOAD_ADDRESS);
    initialize_controller_vsync();
    srand(1);
    g_layout_flag = 0;
    g_layout_sub_mode = -1;
    g_layout_option = -1;
    g_scene_mode = 0;
    g_save_slot_index = 7;
    g_script_pair_value_49 = 0;
    D_8003EC8C = 11;
    D_80042FD0 = 19;
    g_game_state = GAME_STATE_INTRO_MOVIE;
    get_field_render_buffers();
    cdrom_stream(CD_RES_CHECKPS_BIN, g_overlay_load_address);
    cdrom_wait_queue_empty();
    run_checkps((CheckPSRenderState*)CHECKPS_RENDER_ADDRESS);
    DrawSync(0);
    VSync(0);
    g_previous_game_state = GAME_STATE_NONE;
    while (1)
    {
        state = g_game_state;
        switch (state)
        {
        case GAME_STATE_FIELD:
        case GAME_STATE_ATTRACT_1:
        case GAME_STATE_ATTRACT_2:
            SetDispMask(0);
            VSync(0);
            DrawSync(0);
            get_field_render_buffers();
            cdrom_stream(CD_RES_FIELD_BIN, g_overlay_load_address);
            if (g_game_state != GAME_STATE_FIELD)
            {
                cdrom_stream(CD_RES_MOVIE_BIN, SECONDARY_OVERLAY_LOAD_ADDR);
                cdrom_wait_queue_empty();
                if (g_game_state == GAME_STATE_ATTRACT_1)
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
            g_field_scene_config = 0;
            g_game_state = run_field_scene();
            akao_cmd_f0();
            akao_cmd_f1();
            akao_set_song_volume(0, AKAO_VOLUME_MAX);
            g_previous_game_state = GAME_STATE_FIELD;
            break;

        case GAME_STATE_WORLD_MAP:
            get_world_map_overlay_end();
            cdrom_stream(CD_RES_WMAP_BIN, g_overlay_load_address);
            screen_transition(0);
            rect.x = 0;
            rect.y = 0;
            rect.w = SCREEN_WIDTH;
            rect.h = SCREEN_HEIGHT + VRAM_DRAW_HEIGHT;
            ClearImage(&rect, 0, 0, 0);
            DrawSync(0);
            VSync(0);
            akao_cmd_f0();
            akao_cmd_f1();
            cdrom_wait_queue_empty();
            g_game_state = run_world_map();
            akao_cmd_f0();
            akao_cmd_f1();
            if (((g_game_state != GAME_STATE_TITLE) && (g_game_state != GAME_STATE_ATTRACT_1)) && (g_game_state != GAME_STATE_ATTRACT_2))
            {
                load_and_play_song(g_music_track_table[g_music_track_index]);
            }
            g_layout_sub_mode = -1;
            g_previous_game_state = GAME_STATE_WORLD_MAP;
            break;

        case GAME_STATE_TITLE:
        {
            title_menu_buffers = get_title_menu_buffers();
            cdrom_stop();
            cdrom_stream(CD_RES_TITLE_BIN, g_overlay_load_address);
            screen_transition(0);
            cdrom_wait_queue_empty();
            g_game_state = run_title(title_menu_buffers);
            DrawSync(0);
            VSync(0);
            g_previous_game_state = GAME_STATE_TITLE;
            break;
        }

        case GAME_STATE_GNAME:
            get_field_render_buffers();
            cdrom_stream(CD_RES_FIELD_BIN, g_overlay_load_address);
            cdrom_stream(CD_RES_GNAME_BIN, SECONDARY_OVERLAY_LOAD_ADDR);
            screen_transition(0);
            cdrom_wait_queue_empty();
            field_restore_entry_music();
            field_scene_reset(0);
            g_field_audio_timer = 0;
            player_name = g_saved_game.layout.player.name;
            saved_layout = &g_saved_game.layout;
            g_game_state = gname_run((RenderContext*)GNAME_RENDER_ADDRESS, player_name, player_name,
                                     (saved_layout->player.name_source_flags & NAME_SOURCE_MASK) + 4, 0, player_name, 1);
            DrawSync(0);
            VSync(0);
            g_previous_game_state = GAME_STATE_GNAME;
            break;

        case GAME_STATE_WORLD_SELECT:
            get_field_render_buffers();
            cdrom_stream(CD_RES_WSEL_BIN, g_overlay_load_address);
            screen_transition(0);
            cdrom_wait_queue_empty();
            g_game_state = wsel_main((void*)WSEL_RENDER_ADDRESS);
            DrawSync(0);
            VSync(0);
            g_previous_game_state = GAME_STATE_WORLD_SELECT;
            break;

        case GAME_STATE_MENU_LOAD:
            get_field_render_buffers();
            cdrom_stream(CD_RES_FIELD_BIN, g_overlay_load_address);
            cdrom_stream(CD_RES_CLOAD_BIN, SECONDARY_OVERLAY_LOAD_ADDR);
            screen_transition(0);
            cdrom_wait_queue_empty();
            field_scene_reset(0);
            entry_config = &g_field_scene_config;
            g_save_slot_index = 7;
            if (cload_main() != 0)
            {
                g_game_state = GAME_STATE_TITLE;
            }
            else
            {
                field_config = (g_saved_game.layout.field_entry_config & FIELD_CONFIG_FLAGS_MASK) | FIELD_ENTRY_MODE_RESUME;
                *entry_config = FIELD_ENTRY_MODE_RESUME;
                g_saved_game.layout.field_entry_config = field_config;
                g_scene_mode = g_saved_game.layout.scene_mode;
                g_field_entry_flag = g_saved_game.layout.field_flags;
                g_layout_flag = g_saved_game.layout.layout_flags;
                g_layout_option = g_saved_game.layout.option_id;
                g_layout_sub_mode = g_saved_game.layout.sub_mode;
                g_saved_game.layout.field_entry_config = field_config;
                music_track = g_saved_game.layout.music_track;
                g_music_track_index = music_track;
                if ((g_saved_game.layout.option_flags & (SAVED_OPTION_FLAG_2 | SAVED_OPTION_FLAG_3)) == (SAVED_OPTION_FLAG_2 | SAVED_OPTION_FLAG_3))
                {
                    g_game_state = GAME_STATE_WORLD_SELECT;
                }
                else
                {
                    screen_transition(0);
                    g_game_state = run_field_scene();
                }
            }
            DrawSync(0);
            VSync(0);
            g_previous_game_state = GAME_STATE_FIELD;
            break;

        case GAME_STATE_INTRO_MOVIE:
            get_title_menu_buffers();
            cdrom_stream(CD_RES_MOVIE_BIN, SECONDARY_OVERLAY_LOAD_ADDR);
            screen_transition(0);
            cdrom_wait_queue_empty();
            movie_play(0);
            g_game_state = GAME_STATE_TITLE;
            DrawSync(0);
            VSync(0);
            g_previous_game_state = GAME_STATE_INTRO_MOVIE;
            break;
        }
        if (g_game_state == GAME_STATE_RETURN_TO_TITLE)
        {
            g_game_state = GAME_STATE_TITLE;
        }
    }
}
