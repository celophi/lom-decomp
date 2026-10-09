#include "main/main.h"
#include "main/audio/akao.h"
#include "main/audio/akao_cmd.h"
#include "overlays/checkps/checkps.h"
#include "main/cd_resources.h"
#include "main/cdrom.h"
#include "overlays/movie/movie.h"
#include "overlays/title/title.h"
#include "overlays/gname/gname.h"
#include "overlays/cload/cload.h"
#include "overlays/wsel/wsel.h"
#include "overlays/wmap/world_map.h"
#include "main/screen_transition.h"
#include "main/audio/game_audio.h"
#include "main/controller.h"
#include "main/overlay_memory.h"
#include "main/field_runtime.h"
#include "main/display.h"
#include <libgte.h>
#include <libgpu.h>
#include <libapi.h>
#include <libetc.h>
#include <libmcx.h>
#include <libspu.h>
#include <rand.h>

#define SECONDARY_OVERLAY_LOAD_ADDR ((void*)SECONDARY_OVERLAY_ADDRESS)

#define SPAWN_ID_FLAGS_MASK 0xFE000000U
#define NAME_SOURCE_MASK 0x7F
#define CD_RESOURCE_TABLE_LBA 24
#define CD_RESOURCE_TABLE_SIZE 46488
#define FIELD_FONT_LOAD_ADDRESS ((u8*)FONT_ADDRESS)

#define CHECKPS_RENDER_ADDRESS PRIMARY_OVERLAY_UPPER_ADDRESS
#define GNAME_RENDER_ADDRESS SECONDARY_OVERLAY_AT(0x20000)
#define WSEL_RENDER_ADDRESS SECONDARY_OVERLAY_AT(0x30000)

#if !defined(VERSION_JP)
extern s32 D_8003EC8C;
extern s32 D_80042FD0;
#endif

void __main(void);

SavedGame g_saved_game;

/**
 * @brief Initialize the hardware and dispatch the game's overlays forever.
 * @see decomp.me (100%) https://decomp.me/scratch/Tc7j3
 */
void main_game_loop(void)
{
    RECT rect;
    u32* spawn_id;
    u8* player_name;
    u32 saved_spawn_id;
    u32 music_track;
    TitleMenuContext* title_menu_buffers;

    __main();
    SetMem(2);
    SetConf(16, 4, BIOS_STACK_ADDRESS);
#if !defined(VERSION_JP)
    ResetGraph(0);
    SetGraphDebug(0);
    SetDispMask(0);
#endif
    _96_remove();
    ResetCallback();
    SetVideoMode(0);
    SpuInit();
#if defined(VERSION_JP)
    ResetGraph(0);
    SetGraphDebug(0);
    SetDispMask(0);
#endif
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
#if !defined(VERSION_JP)
    akao_init();
#endif
    cdrom_stream(CD_RES_FIELD_FONT, FIELD_FONT_LOAD_ADDRESS);
    initialize_controller_vsync();
    srand(1);
    g_field_music_id = 0;
    g_field_secondary_music_id = -1;
    g_field_sound_bank_id = -1;
    g_field_scene_id = 0;
    g_save_compatibility_tag = SAVE_TAG_STARTUP;
    g_script_pair_value_49 = 0;
#if defined(VERSION_JP)
    g_game_state = GAME_STATE_CHECKPS;
#else
    D_8003EC8C = 11;
    D_80042FD0 = 19;
    g_game_state = GAME_STATE_INTRO_MOVIE;
    get_field_render_buffers();
    cdrom_stream(CD_RES_CHECKPS_BIN, g_overlay_load_address);
    cdrom_wait_queue_empty();
    run_checkps((CheckPSRenderState*)CHECKPS_RENDER_ADDRESS);
    DrawSync(0);
    VSync(0);
#endif
    g_previous_game_state = GAME_STATE_NONE;
    while (1)
    {
        switch (g_game_state)
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
                    movie_play(MOVIE_INDEX_ATTRACT_1);
                }
                else
                {
                    movie_play(MOVIE_INDEX_ATTRACT_2_PART_1);
                    movie_play(MOVIE_INDEX_ATTRACT_2_PART_2);
                    movie_play(MOVIE_INDEX_ATTRACT_2_PART_3);
                }
            }
            else
            {
                cdrom_wait_queue_empty();
            }
            g_field_object_id = 0;
            g_field_spawn_id = 0;
            g_game_state = run_field_scene();
            akao_stop_all_songs();
            akao_release_all_sfx();
            akao_set_song_volume(0, AKAO_VOLUME_MAX);
            g_previous_game_state = GAME_STATE_FIELD;
            break;

        case GAME_STATE_WORLD_MAP:
            get_world_map_overlay_end();
            cdrom_stream(CD_RES_WMAP_BIN, g_overlay_load_address);
            screen_transition(0);
            setRECT(&rect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT + VRAM_DRAW_HEIGHT);
            ClearImage(&rect, 0, 0, 0);
            DrawSync(0);
            VSync(0);
            akao_stop_all_songs();
            akao_release_all_sfx();
            cdrom_wait_queue_empty();
            g_game_state = run_world_map();
            akao_stop_all_songs();
            akao_release_all_sfx();
            if ((g_game_state != GAME_STATE_TITLE) && (g_game_state != GAME_STATE_ATTRACT_1) && (g_game_state != GAME_STATE_ATTRACT_2))
            {
                load_and_play_song(g_music_track_table[g_music_track_index]);
            }
            g_field_secondary_music_id = -1;
            g_previous_game_state = GAME_STATE_WORLD_MAP;
            break;

        case GAME_STATE_TITLE:
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

        case GAME_STATE_GNAME:
            get_field_render_buffers();
            cdrom_stream(CD_RES_FIELD_BIN, g_overlay_load_address);
            cdrom_stream(CD_RES_GNAME_BIN, SECONDARY_OVERLAY_LOAD_ADDR);
            screen_transition(0);
            cdrom_wait_queue_empty();
            field_restore_entry_music();
            field_scene_reset(NULL);
            g_field_audio_timer = 0;
            player_name = g_saved_game.layout.characters[0].name;
            g_game_state = gname_run((RenderContext*)GNAME_RENDER_ADDRESS, player_name, player_name,
                                     (g_saved_game.layout.characters[0].info.bytes[0] & NAME_SOURCE_MASK) + 4, 0, player_name, 1);
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

#if defined(VERSION_JP)
        case GAME_STATE_CHECKPS:
            akao_init();
            get_field_render_buffers();
            cdrom_stream(CD_RES_CHECKPS_BIN, g_overlay_load_address);
            screen_transition(0);
            cdrom_wait_queue_empty();
            g_game_state = run_checkps((CheckPSRenderState*)CHECKPS_RENDER_ADDRESS);
            DrawSync(0);
            VSync(0);
            g_previous_game_state = GAME_STATE_CHECKPS;
            break;

#endif
        case GAME_STATE_MENU_LOAD:
            get_field_render_buffers();
            cdrom_stream(CD_RES_FIELD_BIN, g_overlay_load_address);
            cdrom_stream(CD_RES_CLOAD_BIN, SECONDARY_OVERLAY_LOAD_ADDR);
            screen_transition(0);
            cdrom_wait_queue_empty();
            field_scene_reset(NULL);
            spawn_id = &g_field_spawn_id;
            g_save_compatibility_tag = SAVE_TAG_STARTUP;
            if (cload_main() != 0)
            {
                g_game_state = GAME_STATE_TITLE;
            }
            else
            {
                saved_spawn_id = (g_saved_game.layout.spawn.word & SPAWN_ID_FLAGS_MASK) | FIELD_SPAWN_LOAD_GAME;
                *spawn_id = FIELD_SPAWN_LOAD_GAME;
                g_saved_game.layout.spawn.word = saved_spawn_id;
                g_field_scene_id = g_saved_game.layout.scene_id;
                g_field_object_id = g_saved_game.layout.object_id;
                g_field_music_id = g_saved_game.layout.music_id;
                g_field_sound_bank_id = g_saved_game.layout.sound_bank_id;
                g_field_secondary_music_id = g_saved_game.layout.secondary_music_id;
                g_saved_game.layout.spawn.word = saved_spawn_id;
                music_track = g_saved_game.layout.track.word;
                g_music_track_index = music_track;
                if ((g_saved_game.layout.options.word & (SAVED_OPTION_FLAG_2 | SAVED_OPTION_FLAG_3)) == (SAVED_OPTION_FLAG_2 | SAVED_OPTION_FLAG_3))
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
            movie_play(MOVIE_INDEX_INTRO);
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
