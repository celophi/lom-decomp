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

void upload_tim(void* tim, s16 x, s16 y, s16 clut_x, s32 clut_y);
void stop_title_music(void);
void start_title_music(void);
void set_fade_target(s32 red, s32 green, s32 blue, s32 steps);
void reset_fade_state(void);
void render_title_menu_items(TitleMenuContext* ctx);
void render_title_backdrop(TitleMenuContext* ctx);
void render_fade_overlay(TitleMenuContext* ctx);
void menu_cursor_up(void);
void menu_cursor_down(void);
void load_title_seq(s32 seq_variant);
void load_title_audio_bank(void);
void init_title_menu_state(void);
void handle_title_menu_input(void);

/* Title-menu selection values dispatched by run_title. */
#define TITLE_MENU_ITEM_NEW_GAME 0
#define TITLE_MENU_ITEM_CONTINUE 1

/* Value g_title_selected_item holds when the idle countdown expires. */
#define TITLE_SELECTION_SENTINEL 0xFF

/* Title backdrop: five 64-pixel strips textured from VRAM x 0x140 onwards. */
#define TITLE_BACKDROP_STRIPS 5
#define TITLE_BACKDROP_STRIP_WIDTH 0x40
#define TITLE_BACKDROP_TEXTURE_X 0x140

/**
 * @brief Screen X of the title menu's header quad (texture row 0).
 * @note JP moves it to 0x70.
 */
#if defined(VERSION_JP)
#define TITLE_MENU_HEADER_X 0x70
#else
#define TITLE_MENU_HEADER_X 0x64
#endif

/** @brief Fixed RAM buffer that CD resources are staged into before being unpacked. */
#define TITLE_LOAD_BUFFER ((u8*)LOAD_BUFFER_ADDRESS)
#define TITLE_AUDIO_BANK ((u8*)SOUND_BANK_ADDRESS)
/** @brief Offset table at the head of a staged file: [0] first block, [1] instrument bank. */
#define TITLE_LOAD_BUFFER_OFFSETS ((u32*)LOAD_BUFFER_AT(0x4))

/* run_save_slot_menu result that returns from the picker to the title menu. */
#define SAVE_SLOT_MENU_EXIT_CANCEL 2

/* AKAO sound command used before the fallback field-entry path. */
#define TITLE_SELECTION_SFX_ID 0x3C

/* Number of selectable slots in the title menu item-flag table
 * (g_title_menu_item_flags), each occupying 2 bytes. */
#define TITLE_MENU_SLOT_COUNT 16

/* Idle countdown loaded by init_title_menu_state: 0xE10 == 3600 frames (~60 s at
 * 60 Hz) before the title times out to the attract loop. */
#define TITLE_IDLE_COUNTDOWN_FRAMES 0xE10

/* Full-screen fade encoding and blend modes. */

/** @brief Packet view for a fade TILE or draw-mode command. */
typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} TitleFadePrimitive;

/* The complete .data payload is one databin (title_data), linked unchanged
 * and exported by tools/data/overlays/title.py. Symbols used by the code are
 * declared in title_internal.h and assigned fixed overlay addresses by
 * config/<version>/symbols/title_symbol_addrs.txt. */

/** Advance a fade packet cursor by the concrete packet just emitted. */
#define TITLE_NEXT_FADE_PRIMITIVE(primitive, type) ((TitleFadePrimitive*)((u8*)(primitive) + sizeof(type)))

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
            stop_title_music();
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
    ClearOTagR(current->otag_buffer, TITLE_OT_LENGTH);
    ClearOTagR(current->otag_buffer2, TITLE_OT_LENGTH);
    PutDispEnv(&current->disp_env);
    update_controllers();
    SetDispMask(1);

    while (1)
    {
        ot_head = current->otag_buffer;
        ClearOTagR(ot_head, TITLE_OT_LENGTH);
        current->next_prim_ptr = current->prim_buffer;
        rand();
        VSync(1);
        render_fade_overlay(current);
        render_title_backdrop(current);
        render_title_menu_items(current);
        handle_title_menu_input();

        if (g_title_menu_exit_state == 0)
        {
            DrawSync(0);
            set_controller_vsync_interval(2);
            VSync(2);

            next_context = context;
            if (current == context)
            {
                next_context = current->second_buffer_header;
            }
            current = next_context;
            PutDispEnv(&current->disp_env);
            PutDrawEnv(&current->draw_env);
            DrawOTag(ot_head + TITLE_OT_LENGTH - 1);
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
 * @brief Save-slot picker sub-screen shown after selecting "New Game".
 *
 * @details Same double-buffered render loop shape as @ref render_menu, but
 * drives the save-slot layout/highlight instead of the main title menu.
 * Loops rendering and swapping the two display buffers until
 * g_title_menu_exit_state becomes non-zero (set by handle_save_slot_input),
 * then resets the frame-queue state and returns.
 *
 * @param ctx_base Base address of the double-buffered TitleMenuContext render
 *        buffer, forwarded as-is from run_title.
 * @return The final value of g_title_menu_exit_state: 1 after confirmation or
 *         SAVE_SLOT_MENU_EXIT_CANCEL when returning to the title menu.
 *
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
    set_fade_target(FADE_NEUTRAL, FADE_NEUTRAL, FADE_NEUTRAL, TITLE_FADE_FRAMES);
    DrawSync(0);
    VSync(0);
    setRECT(&rect, 0, 0, SCREEN_WIDTH, VRAM_BACK_DISP_Y + SCREEN_HEIGHT);
    ClearImage(&rect, 0, 0, 0);
    current = ctx_base;
    ClearOTagR(current->otag_buffer, TITLE_OT_LENGTH);
    ClearOTagR(current->otag_buffer2, TITLE_OT_LENGTH);
    VSync(0);
    PutDispEnv(&current->disp_env);
    update_controllers();
    SetDispMask(1);
    do
    {
        ot = current->otag_buffer;
        ClearOTagR(ot, TITLE_OT_LENGTH);
        current->next_prim_ptr = current->prim_buffer;
        VSync(1);
        render_fade_overlay(current);
        render_save_slot_menu(current);
        DrawSync(0);
        set_controller_vsync_interval(2);
        VSync(2);
        next_context = ctx_base;
        if (current == ctx_base)
        {
            next_context = current->second_buffer_header;
        }
        current = next_context;
        PutDispEnv(&current->disp_env);
        PutDrawEnv(&current->draw_env);
        DrawOTag(ot + TITLE_OT_LENGTH - 1);
        update_controllers();
        cdrom_process_state();
    } while (g_title_menu_exit_state == 0);
    reset_controller_vsync_state();
    VSync(0);
    return g_title_menu_exit_state;
}

/**
 * @brief Set up the title overlay's double-buffered display/draw environments.
 *
 * @details Counterpart of init_checkps_display in the CHECKPS overlay. Clears
 * the hardware display registers, configures the geometry screen/offset,
 * clears all of VRAM, and sets up the front and back DISPENV/DRAWENV pairs
 * (front at ctx_base->disp_env/draw_env, back at ctx_base->disp_env2/draw_env2).
 * Called once per title-menu iteration from run_title.
 *
 * @param ctx_base Base address of the double-buffered TitleMenuContext render
 *        buffer, forwarded as-is from run_title.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/evJur
 */
void init_title_display(TitleMenuContext* ctx_base)
{
    RECT rect;
    u8* hw = (u8*)SCD_REGS; /* bytes of the controller block past the SCDRegs fields */

    /* Clear hardware register bytes */
    hw[0x13F] = 0;
    hw[0x91] = 0;
    hw[0x140] = 0;
    hw[0x92] = 0;

    akao_set_mono_output(0);
    SetGeomScreen(SCREEN_PROJECTION_DISTANCE);
    SetGeomOffset(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    ctx_base->front_screen.x = 0;
    ctx_base->front_screen.y = 0;
    ctx_base->front_screen.w = SCREEN_WIDTH;
    ctx_base->front_screen.h = SCREEN_HEIGHT;

    ctx_base->back_screen.x = 0;
    ctx_base->back_screen.y = VRAM_BACK_DISP_Y;
    ctx_base->back_screen.w = SCREEN_WIDTH;
    ctx_base->back_screen.h = SCREEN_HEIGHT;

    DrawSync(0);
    VSync(0);

    /* Clear the full VRAM extent */
    setRECT(&rect, 0, 0, VRAM_WIDTH, VRAM_HEIGHT);
    ClearImage(&rect, 0, 0, 0);

    /* Set display environments */
    SetDefDispEnv(&ctx_base->disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&ctx_base->disp_env2, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);

    /* Set drawing environments */
    SetDefDrawEnv(&ctx_base->draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&ctx_base->draw_env2, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    /* Clear two more bytes */
    ctx_base->draw_env2.dtd = 0;
    ctx_base->draw_env.dtd = 0;

    reset_fade_state();
    set_fade_target(FADE_NEUTRAL, FADE_NEUTRAL, FADE_NEUTRAL, TITLE_FADE_FRAMES);
    init_title_menu_state();

    g_title_menu_exit_state = 0;
}

/**
 * @brief Load and register the title overlay's AKAO instrument/sample bank.
 *
 * @details Counterpart of CHECKPS load_embedded_checkps_audio. Skipped if g_previous_game_state
 * indicates the bank is already resident (values 2, 3, 5, 6, 7). Otherwise
 * loads SOUND/EFFECT.SET from CD-ROM into the 0x80180000 scratch buffer,
 * splits the blob via its self-referential offset table, copies the
 * instrument/sample sub-block to g_title_audio_bank_base (0x8013C000) and
 * registers it as the active AKAO bank, then uploads the trailing instrument
 * bank to the SPU.
 *
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
        cdrom_queue_read(CD_RES_SOUND_EFFECT_SET, TITLE_LOAD_BUFFER);
        cdrom_wait_queue_empty();

        base = TITLE_LOAD_BUFFER;
        off = TITLE_LOAD_BUFFER_OFFSETS;

        bcopy(base + off[0], (u8*)g_title_audio_bank_base, (int)(off[1] - off[0]));

        akao_register_bank((AkaoHeader*)g_title_audio_bank_base);
        akao_upload_bank_blocking((AkaoBankHeader*)(base + off[1]), 1);
    }
}

/**
 * @brief Load a title-screen sequence and its instrument bank from CD-ROM.
 *
 * @details Counterpart of CHECKPS load_checkps_song_from_disc. Reads CD resource
 * @c CD_RES_MUSIC_FILE(seq_variant) into the 0x80180000 scratch
 * buffer, splits it via its self-referential offset table, copies the
 * sequence sub-block to g_resident_song_buffer, then uploads the trailing instrument bank.
 *
 * @param seq_variant Music-file index; 0 selects MSC_DATA.DAT.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/mBQ6i
 */
void load_title_seq(s32 seq_variant)
{
    u32* off;
    u8* base;

    cdrom_queue_read(CD_RES_MUSIC_FILE(seq_variant), TITLE_LOAD_BUFFER);
    cdrom_wait_queue_empty();

    off = TITLE_LOAD_BUFFER_OFFSETS;
    base = TITLE_LOAD_BUFFER;

    bcopy(base + off[0], g_resident_song_buffer, (int)(off[1] - off[0]));
    akao_upload_bank_blocking((AkaoBankHeader*)(base + off[1]), 1);
}

/**
 * @brief Stop the title-screen background music.
 *
 * @details Counterpart of CHECKPS stop_checkps_song.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/1cta3
 */
void stop_title_music(void)
{
    akao_stop_song(0);
}

/**
 * @brief Start playback of the title-screen background music.
 *
 * @details Counterpart of CHECKPS play_loaded_checkps_song. Plays the SEQ loaded into
 * g_resident_song_buffer (by load_title_seq) and sets the song volume to maximum via
 * akao_set_song_volume.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/xYPkq
 */
void start_title_music(void)
{
    akao_play_song((AkaoHeader*)g_resident_song_buffer);
    akao_set_song_volume(0, AKAO_VOLUME_MAX);
}

/**
 * @brief Play a title-screen UI sound effect at maximum volume.
 *
 * @details Counterpart of CHECKPS play_checkps_sfx (with the first parameter
 * folded away to a constant 0). sound_id values observed: 0x3C (selection
 * chime), 0x7C..0x7F (cursor / cancel / confirm beeps).
 *
 * @param sound_id Sound id forwarded to akao_play_sfx's sound_id (lower 10 bits used).
 * @param pan Forwarded to akao_play_sfx's pan (8-bit, possibly pan); every
 *        call site in this file passes the constant 0x80.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/ZuKeL
 */
void play_title_sfx(s32 sound_id, s32 pan)
{
    akao_play_sfx(sound_id, 0, pan, 0x7F);
}

/**
 * @brief Reset the title-screen fade state to opaque black with no fade in progress.
 *
 * @details Counterpart of CHECKPS reset_fade_state.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/m80gj
 */
void reset_fade_state(void)
{
    g_fade_current.red = 0;
    g_fade_current.green = 0;
    g_fade_current.blue = 0;

    g_fade_target.red = 0;
    g_fade_target.green = 0;
    g_fade_target.blue = 0;
    g_fade_target.steps = 0;
}

/**
 * @brief Interpolate the screen fade and emit its overlay primitive.
 *
 * @details Counterpart of CHECKPS update_and_draw_fade: interpolates g_fade_current
 * toward g_fade_target and emits the fade-overlay primitive into the active
 * prim buffer.
 *
 * @param ctx Active TitleMenuContext render buffer.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/fBro2
 */
void render_fade_overlay(TitleMenuContext* ctx)
{
    TitleMenuContext* base = ctx;
    TitleFadePrimitive* primitive = (TitleFadePrimitive*)base->next_prim_ptr;
    u_long* ordering_table_tag = base->otag_buffer;
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;

    if (g_fade_target.steps != 0)
    {
        red_step = (g_fade_target.red - g_fade_current.red) / g_fade_target.steps;
        green_step = (g_fade_target.green - g_fade_current.green) / g_fade_target.steps;
        blue_step = (g_fade_target.blue - g_fade_current.blue) / g_fade_target.steps;
        g_fade_target.steps = g_fade_target.steps - 1;
        g_fade_current.red = g_fade_current.red + red_step;
        g_fade_current.green = g_fade_current.green + green_step;
        g_fade_current.blue = g_fade_current.blue + blue_step;
    }
    else
    {
        g_fade_current.red = g_fade_target.red;
        g_fade_current.green = g_fade_target.green;
        g_fade_current.blue = g_fade_target.blue;
    }
    if (!(((g_fade_current.red == FADE_NEUTRAL) && (g_fade_current.green == FADE_NEUTRAL)) && (g_fade_current.blue == FADE_NEUTRAL)))
    {
        if (g_fade_current.red >= FADE_ADDITIVE_THRESHOLD)
        {
            setRGB0(&primitive->tile, g_fade_current.red - 1, g_fade_current.green - 1, g_fade_current.blue - 1);
        }
        else
        {
            if (g_fade_current.red == FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~g_fade_current.red;
            }
            if (g_fade_current.green == FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~g_fade_current.green;
            }
            if (g_fade_current.blue == FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~g_fade_current.blue;
            }
        }

        setTile(&primitive->tile);
        setSemiTrans(&primitive->tile, 1);
        SET_YX0(&primitive->tile, 0, 0);
        setWH(&primitive->tile, SCREEN_WIDTH, SCREEN_HEIGHT);
        addPrim(ordering_table_tag, &primitive->tile);

        draw_mode = FADE_ADDITIVE_TPAGE;
        primitive = TITLE_NEXT_FADE_PRIMITIVE(primitive, TILE);
        if (g_fade_current.red < FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = FADE_SUBTRACTIVE_TPAGE;
        }
        setDrawTPage(&primitive->draw_mode, 0, 0, draw_mode);
        addPrim(ordering_table_tag, &primitive->draw_mode);

        primitive = TITLE_NEXT_FADE_PRIMITIVE(primitive, DR_TPAGE);
    }
    base->next_prim_ptr = (u_long*)primitive;
}

/**
 * @brief Set the target fade color and step count for the title-screen fade.
 *
 * @details Counterpart of CHECKPS set_fade_target. render_fade_overlay
 * interpolates g_fade_current toward this target over the given number of steps.
 *
 * @param red Target red channel value.
 * @param green Target green channel value.
 * @param blue Target blue channel value.
 * @param steps Number of frames over which to interpolate toward the target.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/zxqdP
 */
void set_fade_target(s32 red, s32 green, s32 blue, s32 steps)
{
    g_fade_target.red = red;
    g_fade_target.green = green;
    g_fade_target.blue = blue;
    g_fade_target.steps = steps;
}

/**
 * @brief Emit the title screen's tiled backdrop strip.
 *
 * @details Emits TITLE_BACKDROP_STRIPS POLY_FT4 quads, each linked into the
 * active OT's last entry.
 *
 * @param ctx Active TitleMenuContext render buffer.
 *
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

    prim = (POLY_FT4*)ctx->next_prim_ptr;
    ot = ctx->otag_buffer;
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
        addPrim(ot + TITLE_OT_LENGTH - 1, prim);
        prim++;
    }

    ctx->next_prim_ptr = (u_long*)prim;
}

/**
 * @brief Per-frame input dispatcher for the main title menu.
 *
 * @details Ticks the idle countdown (dispatching the idle-quit item once it
 * expires), then debounces the confirm/cancel and cursor up/down button
 * combos and moves the cursor or arms g_title_menu_exit_state accordingly.
 *
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
    g_title_idle_countdown -= 1;
    if (g_debounced_input & (PADh | PADi | PADRright))
    {
        play_title_sfx(0x7C, AKAO_PAN_CENTER);
        g_title_menu_exit_state = 1;
        return;
    }
    if (g_debounced_input & (PADLup | PADLleft))
    {
        menu_cursor_up();
        play_title_sfx(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
    }
    else if (g_debounced_input & (PADLdown | PADLright | PADselect))
    {
        menu_cursor_down();
        play_title_sfx(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
    }
}

/**
 * @brief Move the menu cursor to the next enabled slot, wrapping to slot 0
 *        if none remain.
 *
 * @details Linear-search g_title_menu_item_flags forward for the next enabled
 * menu slot. If none remain, the cursor is reset to slot 0 with rank 0.
 *
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
        g_title_selected_item = (u8)item;
        g_title_visible_item_rank++;
    }
}

/**
 * @brief Move the menu cursor to the previous enabled slot, wrapping to the
 *        last enabled slot if already at the top.
 *
 * @details Mirror of menu_cursor_down searching backward. Scans
 * g_title_menu_item_flags from the slot before the current selection toward 0
 * for the next enabled slot, decrementing the visible rank. If the search
 * runs past slot 0, it wraps: a forward pass over all TITLE_MENU_SLOT_COUNT
 * slots counts the enabled ones and records the last enabled index, then the
 * cursor is parked on that slot with rank = count - 1.
 *
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
        /* Wrap to the last enabled slot; its rank is the number of enabled slots minus one. */
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
        g_title_selected_item = (u8)last_enabled;
        return;
    }
    g_title_selected_item = item;
    g_title_visible_item_rank--;
}

/**
 * @brief Emit the title-menu header, each enabled item, and the cursor.
 *
 * @details Walks the 16-slot g_title_menu_item_flags table and emits one
 * emit_menu_item_quad per enabled slot, stacking them vertically (item_y += 0xC
 * each). The currently selected item (visible_index == g_title_visible_item_rank)
 * uses CLUT 1, the rest CLUT 2. A fixed header quad is emitted first, and the
 * cursor quad last; the cursor's U coordinate cycles through
 * g_cursor_blink_u_offsets[(g_title_anim_frame >> 2) & 3] for a 4-frame blink, and
 * its Y tracks the selected rank. The advanced prim cursor is written back to
 * TitleMenuContext::next_prim_ptr.
 *
 * @param ctx Active TitleMenuContext render buffer.
 *
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

    ot_head = ctx->otag_buffer;
    first_prim = ctx->next_prim_ptr;
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

    ctx->next_prim_ptr = result;
    g_title_anim_frame++;
}

/**
 * @brief Build one menu-item POLY_FT4 (textured quad) and link it into the OT.
 *
 * @details Builds a POLY_FT4 at screen (x, y) with size @p width, samples a
 * 16-pixel-tall texture row selected by @p tex_row (V = tex_row * 16 .. +16),
 * and links it at the head of @p ot_head. The quad is neutral-grey (0x80) flat
 * shaded, textured from page (0x140, 0) with a CLUT from row 480.
 *
 * @param ot_head    OT entry to link this primitive in front of.
 * @param prim       Destination primitive buffer (>= 0x28 bytes).
 * @param tex_row    Texture row index; selects V = tex_row*16 (top) .. +16.
 * @param x          Screen X of the quad's left edge.
 * @param y          Screen Y of the quad's top edge.
 * @param u0_base    Base U texture coordinate (left edge).
 * @param width      Quad width in pixels, added to x and u0_base for the
 *                   right edge.
 * @param clut_index CLUT x index (low 6 bits) packed into the CLUT word.
 * @return Pointer just past the emitted primitive (prim + 0x28).
 *
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
    v_top = (u8)(tex_row << 4);
    quad->b0 = 0x80;
    quad->v1 = v_top;
    quad->v0 = v_top;
    v_bottom = (u8)((tex_row << 4) + 0x10);
    quad->g0 = 0x80;
    quad->r0 = 0x80;
    quad->v3 = v_bottom;
    quad->v2 = v_bottom;
    quad->x2 = (u16)x;
    quad->x0 = (u16)x;
    quad->tpage = getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_HALF, 0x140, 0);
    x_right = (u16)(x + width);
    quad->x3 = x_right;
    quad->x1 = x_right;
    quad->y1 = (u16)y;
    quad->y0 = (u16)y;
    y_bottom = (u16)(y + 0x10);
    quad->u2 = (u8)u0_base;
    quad->u0 = (u8)u0_base;
    u_right = (u8)(u0_base + width);
    clut_word = (u16)((clut_index & 0x3F) | getClut(0, 480));
    quad->y3 = y_bottom;
    quad->y2 = y_bottom;
    quad->u3 = u_right;
    quad->u1 = u_right;
    quad->clut = clut_word;
    addPrim(ot_head, quad);
    return quad + 1;
}

/**
 * @brief Initialise the title-menu state globals and upload its TIMs.
 *
 * @details Zeros the per-slot flag table (TITLE_MENU_SLOT_COUNT entries),
 * enables the first two slots (two flag bytes each), resets cursor/input/animation globals, arms the
 * idle countdown to TITLE_IDLE_COUNTDOWN_FRAMES, and uploads the two menu TIMs
 * from g_title_menu_tim_table[1..2] to VRAM. When re-entering from the attract
 * loop (g_previous_game_state == 0) it advances the cursor to the first enabled
 * slot (same forward scan as menu_cursor_down).
 *
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
            g_title_selected_item = (u8)next_item;
            g_title_visible_item_rank++;
        }
    }
}

/**
 * @brief Upload a standard PSX TIM image (optional CLUT + pixel block) to VRAM.
 *
 * @details Reads the TIM flag word at +4: if the CLUT-present bit is set,
 * loads the CLUT (clut_width x clut_height entries as a single VRAM row) at
 * (clut_x, clut_y), then advances @c p past the CLUT block by its byte length.
 * The pixel block that follows is then loaded at (x, y) using its own
 * width/height header.
 *
 * The TIM layout used here (offsets from @c p):
 *  - +0x04 flag word (bit 3 = has CLUT; happens to equal TIM_HEADER_SIZE)
 *  - CLUT block: +0x08 block byte length, +0x0C/0x0E VRAM x/y,
 *    +0x10/0x12 width/height, +0x14 entries
 *  - pixel block: +0x08 VRAM x/y, +0x10/0x12 width/height (here read as
 *    +0x08/0x0A after @c p is advanced), +0x0C pixels
 *
 * @param tim   Pointer to the TIM image header.
 * @param x     Destination VRAM X for the pixel block.
 * @param y     Destination VRAM Y for the pixel block.
 * @param clut_x Destination VRAM X for the CLUT.
 * @param clut_y Destination VRAM Y for the CLUT.
 *
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
        setRECT(&rect, clut_x, (s16)clut_y, clut_width * clut_height, clut_rows);
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

/**
 * @brief Reads the SCD pad state and returns the remapped button bitmap.
 *
 * Same byte-swap and button-remap as @p read_pad_input, but returns the
 * computed bitmap directly instead of writing it into @p g_last_input_state
 * and resetting @p g_input_repeat_timer.
 *
 *
 * @return Remapped button bitmap, or 0 if the pad is not present
 *         (g_controller_device_type >= TITLE_PAD_UNAVAILABLE).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/Z5swg
 * @note JP reports the face buttons as read (no PAD_REMAP_FACE_BITS swap).
 */
s32 read_pad_state(void)
{
    SCDRegs* regs = SCD_REGS;
    u32 buttons;
    s16 axis_x;
    s16 axis_y;
    u16 hi_read;
    u16 lo_read;

    if (regs->device_type >= TITLE_PAD_UNAVAILABLE)
    {
        return 0;
    }

    /* Read twice because the controller register may change asynchronously. */
    hi_read = regs->held_buttons;
    lo_read = regs->held_buttons;
    buttons = (hi_read >> 8) | (lo_read << 8);
#if !defined(VERSION_JP)
    buttons = PAD_REMAP_FACE_BITS(buttons);
#endif
    if (regs->device_type != 0)
    {
        /* Convert signed analog-axis thresholds to digital directions. */
        axis_x = regs->axis_x.signed_value;
        if (axis_x < TITLE_ANALOG_LOW_THRESHOLD)
        {
            buttons |= PAD_BTN_LEFT;
        }
        else if (axis_x >= TITLE_ANALOG_HIGH_THRESHOLD)
        {
            buttons |= PAD_BTN_RIGHT;
        }

        axis_y = regs->axis_y.signed_value;
        if (axis_y < TITLE_ANALOG_LOW_THRESHOLD)
        {
            buttons |= PAD_BTN_UP;
        }
        else if (axis_y >= TITLE_ANALOG_HIGH_THRESHOLD)
        {
            buttons |= PAD_BTN_DOWN;
        }
    }
    return buttons;
}

/**
 * @brief Read the SCD pad, debounce it, and publish the result in g_debounced_input.
 *
 * @details Counterpart of CHECKPS update_controller_input. Builds the remapped
 * button bitmap (byte-swap + face-bit remap + analog-stick to d-pad
 * thresholding) exactly like read_pad_input, then runs an auto-repeat state
 * machine over g_last_input_state / g_input_repeat_timer:
 *  - If this frame matches the previous state (or shares any repeat-eligible
 *    bit with it), only the d-pad directions auto-repeat: g_debounced_input
 *    fires every (2 + 1) frames while held, otherwise it is suppressed.
 *  - A fresh, different press is published immediately and arms the longer
 *    initial-repeat delay (15 frames).
 *  - No input clears all three globals.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/geg1v
 * @note JP reads the face buttons as they are (no PAD_REMAP_FACE_BITS swap).
 */
void update_menu_input(void)
{
    SCDRegs* regs = SCD_REGS;
    u32 buttons;
    s32 axis;
    s32 input_state;

    if (g_controller_device_type >= TITLE_PAD_UNAVAILABLE)
    {
        input_state = 0;
    }
    else
    {
        buttons = (regs->held_buttons >> 8) | (regs->held_buttons << 8);
#if !defined(VERSION_JP)
        buttons = PAD_REMAP_FACE_BITS(buttons);
#endif
        if (regs->device_type != 0)
        {
            axis = regs->axis_x.signed_value;
            if (axis < TITLE_ANALOG_LOW_THRESHOLD)
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis >= TITLE_ANALOG_HIGH_THRESHOLD)
            {
                buttons |= PAD_BTN_RIGHT;
            }

            axis = regs->axis_y.signed_value;
            if (axis < TITLE_ANALOG_LOW_THRESHOLD)
            {
                buttons |= PAD_BTN_UP;
            }
            else if (axis >= TITLE_ANALOG_HIGH_THRESHOLD)
            {
                buttons |= PAD_BTN_DOWN;
            }
        }
        input_state = buttons;
    }
    g_debounced_input = 0;
    if (((input_state == g_last_input_state) || ((g_last_input_state != 0) && (input_state & (g_last_input_state | TITLE_NON_REPEAT_BUTTON_MASK)))) &&
        (input_state != 0))
    {
        /* Held input repeats directional buttons only. */
        if ((input_state & TITLE_DPAD_BUTTONS) != 0)
        {
            input_state &= TITLE_DPAD_BUTTONS;
        }
        if (g_input_repeat_timer == 0)
        {
            g_debounced_input = input_state;
            g_input_repeat_timer = TITLE_REPEAT_DELAY;
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
        g_input_repeat_timer = TITLE_INITIAL_REPEAT_DELAY;
    }
}
