#include "common/saved_game.h"
#include "main/display.h"
#include "internal/title_internal.h"
#include "internal/title_save.h"
#include "main/screen_transition.h"
#include "main/audio/akao_cmd.h"
#include "main/audio/game_audio.h"
#include <memory.h>
#include "main/cdrom.h"
#include <rand.h>
#include "main/controller.h"
#include "overlays/field/field_sound.h"
#include "common/gpu_packet.h"
#include "main/audio/akao.h"

#define TITLE_MENU_ITEM_NEW_GAME 0
#define TITLE_MENU_ITEM_CONTINUE 1

/* Selection set when the idle countdown runs out; starts the intro movie. */
#define TITLE_SELECTION_SENTINEL 0xFF

#define TITLE_BACKDROP_STRIPS 5
#define TITLE_BACKDROP_STRIP_WIDTH 0x40
#define TITLE_BACKDROP_TEXTURE_X 0x140

#if defined(VERSION_JP)
#define TITLE_MENU_HEADER_X 0x70
#else
#define TITLE_MENU_HEADER_X 0x64
#endif

#define TITLE_AUDIO_BANK ((u8*)SOUND_BANK_ADDRESS)

#define SAVE_SLOT_MENU_EXIT_CANCEL 2

#define TITLE_SELECTION_SFX_ID 0x3C

/* g_title_menu_item_flags has two bytes per slot; only the first is tested. */
#define TITLE_MENU_SLOT_COUNT 16

/* 60 seconds at 60 Hz. */
#define TITLE_IDLE_COUNTDOWN_FRAMES 0xE10

void upload_tim(void* tim, s16 x, s16 y, s16 clut_x, s32 clut_y);
void start_title_music(void);
void render_title_menu_items(TitleMenuContext* ctx);
void render_title_backdrop(TitleMenuContext* ctx);
void menu_cursor_up(void);
void menu_cursor_down(void);
void load_title_seq(s32 seq_variant);
void load_title_audio_bank(void);
void init_title_menu_state(void);
void handle_title_menu_input(void);
void init_title_display(TitleMenuContext* context);
void render_menu(TitleMenuContext* context);
s32 run_save_slot_menu(TitleMenuContext* context);
void* emit_menu_item_quad(u_long* ot_head, void* prim, s32 tex_row, s32 x, s32 y, s32 u0_base, s32 width, s32 clut_index);

/**
 * @brief Run the title menu and choose the next game state.
 * @param menu_context Title display buffers.
 * @return Next game-state code selected by the title screen.
 * @see decomp.me (100%) https://decomp.me/scratch/mEAXF
 */
s32 run_title(TitleMenuContext* menu_context)
{
    TitleMenuContext* context;
    SceneState* persistent_scene_state = SCENE_STATE;
    s32 random_low;
    s32 random_high;
    u8 selection;

    context = menu_context;

    load_title_audio_bank();
    load_title_seq(0);
    start_title_music();

    while (1)
    {
        init_title_display(context);
        persistent_scene_state->map_id = 0;
        persistent_scene_state->object_index = 0;
        persistent_scene_state->camera_x = 0;
        persistent_scene_state->camera_y = 0;
        persistent_scene_state->camera_z = 0;

        do
        {
            render_menu(context);
        } while (g_title_menu_exit_state == 0);

        g_playtime_vsync_origin = VSync(-1);
        selection = g_title_selected_item;

        if (selection == TITLE_MENU_ITEM_NEW_GAME)
        {
            load_saved_game_template(0);
            g_title_menu_exit_state = 0;

            if (run_save_slot_menu(context) == SAVE_SLOT_MENU_EXIT_CANCEL)
            {
                screen_transition(0);
                continue;
            }

            return GAME_STATE_GNAME;
        }
        else if (selection == TITLE_MENU_ITEM_CONTINUE)
        {
            return GAME_STATE_MENU_LOAD;
        }
        else if (selection == TITLE_SELECTION_SENTINEL)
        {
            stop_song();
            return GAME_STATE_INTRO_MOVIE;
        }
        else
        {
            akao_fade_song_volume(0, TITLE_SELECTION_SFX_ID, 0);
            load_saved_game_template(-1);
            g_save_compatibility_tag = SAVE_TAG_ANY;
            random_low = rand();
            random_high = rand();
            g_saved_game.layout.identity.ids.game_id = random_low | (random_high << TITLE_RNG_HIGH_SHIFT);
            return GAME_STATE_FIELD;
        }
    }
}

/**
 * @brief Render the main title menu until a selection or idle timeout occurs.
 * @param context Display and primitive buffers for both title frames.
 * @see decomp.me (100%) https://decomp.me/scratch/bMLDn
 */
void render_menu(TitleMenuContext* context)
{
    RECT rect;
    TitleMenuContext* current;
    u_long* ot_head;
    void* next_context;

    DrawSync(0);
    VSync(0);
    setRECT(&rect, 0, 0, SCREEN_WIDTH, VRAM_BACK_DISP_Y + SCREEN_HEIGHT);
    ClearImage(&rect, 0, 0, 0);

    current = context;
    ClearOTagR(current->screen.ot, FRAME_ORDERING_TABLE_LENGTH);
    ClearOTagR(current->otag_buffer2, FRAME_ORDERING_TABLE_LENGTH);
    PutDispEnv(&current->screen.disp_env);
    update_controllers();
    SetDispMask(1);

    while (1)
    {
        ot_head = current->screen.ot;
        ClearOTagR(ot_head, FRAME_ORDERING_TABLE_LENGTH);
        current->screen.prim_cursor = current->screen.packets;
        rand();
        VSync(1);
        update_and_draw_fade(&current->screen);
        render_title_backdrop(current);
        render_title_menu_items(current);
        handle_title_menu_input();

        if (g_title_menu_exit_state == 0)
        {
            DrawSync(0);
            set_controller_vsync_interval(FRAME_VSYNC_INTERVAL);
            VSync(FRAME_VSYNC_INTERVAL);

            next_context = context;

            if (current == context)
            {
                next_context = current->second_buffer_header;
            }

            current = next_context;
            PutDispEnv(&current->screen.disp_env);
            PutDrawEnv(&current->screen.draw_env);
            DrawOTag(ot_head + FRAME_ORDERING_TABLE_LENGTH - 1);
            update_controllers();
            cdrom_process_state();

            if (g_title_menu_exit_state == 0)
            {
                continue;
            }
        }

        break;
    }

    reset_controller_vsync_state();
    VSync(0);
    DrawSync(0);
}

/**
 * @brief Run the new-game picker until the player confirms or backs out.
 * @param ctx_base Title display buffers.
 * @return 1 on confirm, SAVE_SLOT_MENU_EXIT_CANCEL to go back to the title menu.
 * @see decomp.me (100%) https://decomp.me/scratch/AKk7x
 */
s32 run_save_slot_menu(TitleMenuContext* ctx_base)
{
    RECT rect;
    TitleMenuContext* current;
    void* next_context;
    u_long* ot;

    init_save_slot_menu();
    screen_transition(0);
    set_fade_target(FADE_NEUTRAL, FADE_NEUTRAL, FADE_NEUTRAL, SCREEN_FADE_FRAMES);
    DrawSync(0);
    VSync(0);
    setRECT(&rect, 0, 0, SCREEN_WIDTH, VRAM_BACK_DISP_Y + SCREEN_HEIGHT);
    ClearImage(&rect, 0, 0, 0);
    current = ctx_base;
    ClearOTagR(current->screen.ot, FRAME_ORDERING_TABLE_LENGTH);
    ClearOTagR(current->otag_buffer2, FRAME_ORDERING_TABLE_LENGTH);
    VSync(0);
    PutDispEnv(&current->screen.disp_env);
    update_controllers();
    SetDispMask(1);

    do
    {
        ot = current->screen.ot;
        ClearOTagR(ot, FRAME_ORDERING_TABLE_LENGTH);
        current->screen.prim_cursor = current->screen.packets;
        VSync(1);
        update_and_draw_fade(&current->screen);
        render_save_slot_menu(current);
        DrawSync(0);
        set_controller_vsync_interval(FRAME_VSYNC_INTERVAL);
        VSync(FRAME_VSYNC_INTERVAL);
        next_context = ctx_base;

        if (current == ctx_base)
        {
            next_context = current->second_buffer_header;
        }

        current = next_context;
        PutDispEnv(&current->screen.disp_env);
        PutDrawEnv(&current->screen.draw_env);
        DrawOTag(ot + FRAME_ORDERING_TABLE_LENGTH - 1);
        update_controllers();
        cdrom_process_state();
    } while (g_title_menu_exit_state == 0);
    reset_controller_vsync_state();
    VSync(0);
    return g_title_menu_exit_state;
}

/**
 * @brief Clear VRAM, set up both display buffers, and reset the title menu.
 * @param ctx_base Title display buffers.
 * @see decomp.me (100%) https://decomp.me/scratch/evJur
 */
void init_title_display(TitleMenuContext* ctx_base)
{
    RECT rect;
    u8* hw = (u8*)SCD_REGS; /* bytes of the controller block past the SCDRegs fields */

    hw[0x13F] = 0;
    hw[0x91] = 0;
    hw[0x140] = 0;
    hw[0x92] = 0;

    akao_set_mono_output(0);
    SetGeomScreen(SCREEN_PROJECTION_DISTANCE);
    SetGeomOffset(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    ctx_base->screen.clear_rect.x = 0;
    ctx_base->screen.clear_rect.y = 0;
    ctx_base->screen.clear_rect.w = SCREEN_WIDTH;
    ctx_base->screen.clear_rect.h = SCREEN_HEIGHT;

    ctx_base->back_screen.x = 0;
    ctx_base->back_screen.y = VRAM_BACK_DISP_Y;
    ctx_base->back_screen.w = SCREEN_WIDTH;
    ctx_base->back_screen.h = SCREEN_HEIGHT;

    DrawSync(0);
    VSync(0);

    setRECT(&rect, 0, 0, VRAM_WIDTH, VRAM_HEIGHT);
    ClearImage(&rect, 0, 0, 0);

    SetDefDispEnv(&ctx_base->screen.disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&ctx_base->disp_env2, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);

    SetDefDrawEnv(&ctx_base->screen.draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&ctx_base->draw_env2, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    ctx_base->draw_env2.dtd = 0;
    ctx_base->screen.draw_env.dtd = 0;

    reset_fade_state();
    set_fade_target(FADE_NEUTRAL, FADE_NEUTRAL, FADE_NEUTRAL, SCREEN_FADE_FRAMES);
    init_title_menu_state();

    g_title_menu_exit_state = 0;
}

/**
 * @brief Load the sound-effect bank from disc and upload it to the SPU.
 * @note Skipped when arriving from a screen that leaves the bank resident.
 * @see decomp.me (100%) https://decomp.me/scratch/6zUZp
 */
void load_title_audio_bank(void)
{
    u8* base;
    u32* off;

    if ((g_previous_game_state != GAME_STATE_TITLE) && (g_previous_game_state != GAME_STATE_GNAME) && (g_previous_game_state != GAME_STATE_CHECKPS) &&
        (g_previous_game_state != GAME_STATE_MENU_LOAD) && (g_previous_game_state != GAME_STATE_WORLD_SELECT))
    {

        g_title_audio_bank_base = TITLE_AUDIO_BANK;
        cdrom_queue_read(CD_RES_SOUND_EFFECT_SET, LOAD_BUFFER_AKAO_CONTAINER);
        cdrom_wait_queue_empty();

        base = (u8*)LOAD_BUFFER_AKAO_CONTAINER;
        off = LOAD_BUFFER_AKAO_CONTAINER->section_offsets;

        bcopy(base + off[AKAO_CONTAINER_RESIDENT_SECTION], (u8*)g_title_audio_bank_base,
              (int)(off[AKAO_CONTAINER_BANK_SECTION] - off[AKAO_CONTAINER_RESIDENT_SECTION]));

        akao_register_bank((AkaoHeader*)g_title_audio_bank_base);
        akao_upload_bank_blocking((AkaoBankHeader*)(base + off[AKAO_CONTAINER_BANK_SECTION]), 1);
    }
}

/**
 * @brief Load a song into g_resident_song_buffer and upload its instruments.
 * @param seq_variant Music-file index; 0 selects MSC_DATA.DAT.
 * @see decomp.me (100%) https://decomp.me/scratch/mBQ6i
 */
void load_title_seq(s32 seq_variant)
{
    u32* off;
    u8* base;

    cdrom_queue_read(CD_RES_MUSIC_FILE(seq_variant), LOAD_BUFFER_AKAO_CONTAINER);
    cdrom_wait_queue_empty();

    off = LOAD_BUFFER_AKAO_CONTAINER->section_offsets;
    base = (u8*)LOAD_BUFFER_AKAO_CONTAINER;

    bcopy(base + off[AKAO_CONTAINER_RESIDENT_SECTION], g_resident_song_buffer, (int)(off[AKAO_CONTAINER_BANK_SECTION] - off[AKAO_CONTAINER_RESIDENT_SECTION]));
    akao_upload_bank_blocking((AkaoBankHeader*)(base + off[AKAO_CONTAINER_BANK_SECTION]), 1);
}

#include "../../common/overlay_audio/stop_song.inc.c"

/**
 * @brief Play the song loaded by load_title_seq at full volume.
 * @see decomp.me (100%) https://decomp.me/scratch/xYPkq
 */
void start_title_music(void)
{
    akao_play_song((AkaoHeader*)g_resident_song_buffer);
    akao_set_song_volume(0, AKAO_VOLUME_MAX);
}

#include "../../common/overlay_audio/play_sfx.inc.c"

#include "../../common/screen_fade_words/reset_fade_state.inc.c"

#include "../../common/screen_fade_words/update_and_draw_fade.inc.c"

#include "../../common/screen_fade_words/set_fade_target.inc.c"

/**
 * @brief Draw the title backdrop as a row of 16-bit texture strips.
 * @param ctx Active title display buffer.
 * @see decomp.me (100%) https://decomp.me/scratch/aKAFU
 */
void render_title_backdrop(TitleMenuContext* ctx)
{
    u_long* ot;
    POLY_FT4* prim;
    s32 strip_index;
    s32 texture_x;
    s32 right_x;
    s32 left_x;
    s32 page_x;

    prim = (POLY_FT4*)ctx->screen.prim_cursor;
    ot = ctx->screen.ot;
    strip_index = 0;

    while (strip_index < TITLE_BACKDROP_STRIPS)
    {
        right_x = TITLE_BACKDROP_STRIP_WIDTH + (strip_index << 6);
        prim->x1 = prim->x3 = right_x;
        texture_x = TITLE_BACKDROP_TEXTURE_X + (strip_index << 6);
        page_x = texture_x & 0x3FF;
        left_x = strip_index << 6;
        strip_index++;
        prim->x0 = prim->x2 = left_x;
        setPolyFT4(prim);
        prim->r0 = prim->g0 = prim->b0 = 0x80;
        prim->y0 = prim->y1 = 0;
        prim->y2 = prim->y3 = VRAM_DRAW_HEIGHT;
        prim->u0 = prim->u2 = 0;
        prim->u1 = prim->u3 = TITLE_BACKDROP_STRIP_WIDTH;
        prim->v0 = prim->v1 = 8;
        prim->v2 = prim->v3 = 8 + VRAM_DRAW_HEIGHT;
        prim->tpage = getTPage(GPU_TEXTURE_16BIT, GPU_BLEND_HALF, page_x, 0x100);
        prim->clut = getClut(0, 481);
        addPrim(ot + FRAME_ORDERING_TABLE_LENGTH - 1, prim);
        prim++;
    }

    ctx->screen.prim_cursor = (u_long*)prim;
}

/**
 * @brief Handle one frame of title-menu input and the idle timeout.
 * @see decomp.me (100%) https://decomp.me/scratch/vmcmD
 */
void handle_title_menu_input(void)
{
    update_menu_input();

    if (g_title_idle_countdown == 0)
    {
        g_title_menu_exit_state = 1;
        g_title_selected_item = TITLE_SELECTION_SENTINEL;
        return;
    }

    g_title_idle_countdown--;

    if (g_debounced_input & (PADh | PADi | PADRright))
    {
        play_sfx(0x7C, AKAO_PAN_CENTER);
        g_title_menu_exit_state = 1;
        return;
    }

    if (g_debounced_input & (PADLup | PADLleft))
    {
        menu_cursor_up();
        play_sfx(SFX_CURSOR, AKAO_PAN_CENTER);
    }
    else if (g_debounced_input & (PADLdown | PADLright | PADselect))
    {
        menu_cursor_down();
        play_sfx(SFX_CURSOR, AKAO_PAN_CENTER);
    }
}

/**
 * @brief Move the cursor to the next enabled item, wrapping to the first.
 * @see decomp.me (100%) https://decomp.me/scratch/6k8uV
 */
void menu_cursor_down(void)
{
    s32 item;

    for (item = g_title_selected_item + 1; item < TITLE_MENU_SLOT_COUNT; item++)
    {
        if (g_title_menu_item_flags[item * 2] != 0)
        {
            break;
        }
    }

    if (item == TITLE_MENU_SLOT_COUNT)
    {
        g_title_visible_item_rank = 0;
        g_title_selected_item = 0;
    }
    else
    {
        g_title_selected_item = item;
        g_title_visible_item_rank++;
    }
}

/**
 * @brief Move the cursor to the previous enabled item, wrapping to the last.
 * @see decomp.me (100%) https://decomp.me/scratch/mmzjI
 */
void menu_cursor_up(void)
{
    s32 item;
    s32 last_enabled;
    s32 enabled_count;

    for (item = g_title_selected_item - 1; item >= 0; item--)
    {
        if (g_title_menu_item_flags[item * 2] != 0)
        {
            break;
        }
    }

    if (item < 0)
    {
        enabled_count = 0;

        for (item = 0; item < TITLE_MENU_SLOT_COUNT; item++)
        {
            if (g_title_menu_item_flags[item * 2] != 0)
            {
                enabled_count++;
                last_enabled = item;
            }
        }

        g_title_visible_item_rank = enabled_count - 1;
        g_title_selected_item = last_enabled;
        return;
    }

    g_title_selected_item = item;
    g_title_visible_item_rank--;
}

/**
 * @brief Draw the menu header, the enabled items, and the animated cursor.
 * @param ctx Active title display buffer.
 * @note Item quads use texture row slot + 1; the selected item uses CLUT 1, the others CLUT 2.
 * @see decomp.me (100%) https://decomp.me/scratch/qegw7
 */
void render_title_menu_items(TitleMenuContext* ctx)
{
    u_long* ot_head;
    u8* prim;
    s32 slot;
    s32 visible_index;
    s32 item_y;
    s32 item_x;
    u8* flag_ptr;
    void* first_prim;
    s32 item_visible;
    void* result;

    ot_head = ctx->screen.ot;
    first_prim = ctx->screen.prim_cursor;
    prim = emit_menu_item_quad(ot_head, first_prim, 0, TITLE_MENU_HEADER_X, 0xC8, 0, 0x80, 1);
    item_x = 0x88;
    item_y = 0xA0;
    visible_index = 0;
    flag_ptr = &g_title_menu_item_flags[0];

    for (slot = 0; slot < TITLE_MENU_SLOT_COUNT; slot++)
    {
        item_visible = (*flag_ptr) != 0;

        if (item_visible)
        {
            prim = (u8*)emit_menu_item_quad(ot_head, prim, slot + 1, item_x, item_y, 0, 0x80, (g_title_visible_item_rank == visible_index) ? 1 : 2);
            item_y += 0xC;
            visible_index++;
            prim += 0x28;
        }

        flag_ptr += 2;
    }

    result =
        emit_menu_item_quad(ot_head, prim, 7, 0x78, g_title_visible_item_rank * 12 + 0x9D, g_cursor_blink_u_offsets[(g_title_anim_frame >> 2) & 3], 0x10, 0);

    ctx->screen.prim_cursor = result;
    g_title_anim_frame++;
}

/**
 * @brief Add one 16-pixel-tall row of the menu texture as a quad.
 * @param ot_head OT entry to link the quad into.
 * @param prim Primitive buffer to build the quad in.
 * @param tex_row Texture row, in 16-pixel steps.
 * @param x Screen X.
 * @param y Screen Y.
 * @param u0_base Left texture U.
 * @param width Width in pixels, on screen and in the texture.
 * @param clut_index CLUT within VRAM row 480.
 * @return Pointer just past the quad.
 * @see decomp.me (100%) https://decomp.me/scratch/FcuOZ
 */
void* emit_menu_item_quad(u_long* ot_head, void* prim, s32 tex_row, s32 x, s32 y, s32 u0_base, s32 width, s32 clut_index)
{
    POLY_FT4* quad;
    u8 v_top;
    u8 v_bottom;
    u16 x_right;
    u16 y_bottom;
    u8 u_right;
    u16 clut_word;

    quad = prim;
    setPolyFT4(quad);
    v_top = (tex_row << 4);
    quad->b0 = 0x80;
    quad->v1 = v_top;
    quad->v0 = v_top;
    v_bottom = ((tex_row << 4) + 0x10);
    quad->g0 = 0x80;
    quad->r0 = 0x80;
    quad->v3 = v_bottom;
    quad->v2 = v_bottom;
    quad->x2 = x;
    quad->x0 = x;
    quad->tpage = getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_HALF, 0x140, 0);
    x_right = (x + width);
    quad->x3 = x_right;
    quad->x1 = x_right;
    quad->y1 = y;
    quad->y0 = y;
    y_bottom = (y + 0x10);
    quad->u2 = u0_base;
    quad->u0 = u0_base;
    u_right = (u0_base + width);
    clut_word = ((clut_index & 0x3F) | getClut(0, 480));
    quad->y3 = y_bottom;
    quad->y2 = y_bottom;
    quad->u3 = u_right;
    quad->u1 = u_right;
    quad->clut = clut_word;
    addPrim(ot_head, quad);
    return quad + 1;
}

/**
 * @brief Enable New Game and Continue, reset the cursor, and upload the menu TIMs.
 * @note Coming back from FIELD starts the cursor on Continue.
 * @see decomp.me (100%) https://decomp.me/scratch/HW23j
 */
void init_title_menu_state(void)
{
    u8* flag_ptr;
    s32 i;
    s32 next_item;

    i = 0;
    flag_ptr = g_title_menu_item_flags;

    for (; i < TITLE_MENU_SLOT_COUNT; i++)
    {
        flag_ptr[0] = 0;
        flag_ptr[1] = 0;
        flag_ptr += 2;
    }

    g_title_menu_item_flags[0] = 1;
    g_title_menu_item_flags[1] = 1;
    g_title_menu_item_flags[2] = 1;
    g_title_menu_item_flags[3] = 1;

    g_title_visible_item_rank = 0;

    g_title_selected_item = 0;
    g_title_anim_frame = 0;
    g_input_repeat_timer = 0;
    g_last_input_state = 0;
    g_debounced_input = 0;
    g_title_idle_countdown = TITLE_IDLE_COUNTDOWN_FRAMES;
    upload_tim((void*)(((u8*)&g_title_menu_tim_table) + g_title_menu_tim_table[1]), 0x140, 0, 0, 0x1E0);
    upload_tim((void*)(((u8*)&g_title_menu_tim_table) + g_title_menu_tim_table[2]), 0x140, 0x100, 0, 0x1E1);

    if (g_previous_game_state == GAME_STATE_FIELD)
    {
        for (next_item = g_title_selected_item + 1; next_item < TITLE_MENU_SLOT_COUNT; next_item++)
        {
            if (g_title_menu_item_flags[next_item * 2] != 0)
            {
                break;
            }
        }

        if (next_item == TITLE_MENU_SLOT_COUNT)
        {
            g_title_visible_item_rank = 0;
            g_title_selected_item = 0;
        }
        else
        {
            g_title_selected_item = next_item;
            g_title_visible_item_rank++;
        }
    }
}

/**
 * @brief Upload a TIM to VRAM at the given positions instead of its own.
 * @param tim TIM image.
 * @param x Pixel-block VRAM X.
 * @param y Pixel-block VRAM Y.
 * @param clut_x CLUT VRAM X.
 * @param clut_y CLUT VRAM Y.
 * @note The CLUT is loaded as one VRAM row, however many rows the TIM gives it.
 * @see decomp.me (100%) https://decomp.me/scratch/fzh5x
 */
void upload_tim(void* tim, s16 x, s16 y, s16 clut_x, s32 clut_y)
{
    u8* p = (u8*)tim;
    int tim_header_size;
    RECT rect;
    s32 clut_block_len;
    u16 clut_width;
    int clut_rows;
    int clut_skip_base;
    u16 clut_height;
    tim_header_size = 8;
    clut_rows = 1;

    if (p[4] & tim_header_size)
    {
        clut_width = *((u16*)(p + 0x10));
        clut_height = *((u16*)(p + 0x12));
        clut_block_len = *((s32*)(p + tim_header_size));
        clut_skip_base = 8;
        setRECT(&rect, clut_x, clut_y, clut_width * clut_height, clut_rows);
        LoadImage(&rect, (u_long*)(p + 0x14));
        p = (p + clut_skip_base) + clut_block_len;
    }
    else
    {
        p = p + 8;
    }

    setRECT(&rect, x, y, *((u16*)(p + 8)), *((u16*)(p + 0xA)));
    LoadImage(&rect, (u_long*)(p + 0xC));
}

#include "../../common/pad_repeat/read_pad_buttons.inc.c"

/**
 * @brief Read the pad into g_debounced_input with menu auto-repeat.
 * @note A new press fires at once; held, only the d-pad repeats, after PAD_REPEAT_INITIAL_DELAY frames.
 * @see decomp.me (100%) https://decomp.me/scratch/geg1v
 */
void update_menu_input(void)
{
    SCDRegs* regs = SCD_REGS;
    u32 buttons;
    s32 axis;
    s32 input_state;

    if (g_controller_device_type >= CONTROLLER_DEVICE_CONFIGURING)
    {
        input_state = 0;
    }
    else
    {
        buttons = (regs->held_buttons >> 8) | (regs->held_buttons << 8);
#if !defined(VERSION_JP)
        buttons = PAD_REMAP_FACE_BITS(buttons);
#endif
        if (regs->device_type != CONTROLLER_DEVICE_DIGITAL)
        {
            axis = regs->axis_x.signed_value;

            if (axis < -PAD_STICK_THRESHOLD)
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis > PAD_STICK_THRESHOLD)
            {
                buttons |= PAD_BTN_RIGHT;
            }

            axis = regs->axis_y.signed_value;

            if (axis < -PAD_STICK_THRESHOLD)
            {
                buttons |= PAD_BTN_UP;
            }
            else if (axis > PAD_STICK_THRESHOLD)
            {
                buttons |= PAD_BTN_DOWN;
            }
        }

        input_state = buttons;
    }

    g_debounced_input = 0;

    if (((input_state == g_last_input_state) || ((g_last_input_state != 0) && (input_state & (g_last_input_state | PAD_REPEAT_HOLD_BUTTONS)))) &&
        (input_state != 0))
    {
        if ((input_state & PAD_BTN_DPAD) != 0)
        {
            input_state &= PAD_BTN_DPAD;
        }

        if (g_input_repeat_timer == 0)
        {
            g_debounced_input = input_state;
            g_input_repeat_timer = PAD_REPEAT_INTERVAL;
        }
        else
        {
            g_input_repeat_timer--;
            g_debounced_input = 0;
        }
    }
    else if (input_state == 0)
    {
        g_input_repeat_timer = 0;
        g_last_input_state = 0;
    }
    else
    {
        g_debounced_input = input_state;
        g_last_input_state = input_state;
        g_input_repeat_timer = PAD_REPEAT_INITIAL_DELAY;
    }
}
