#include "title_internal.h"

/* Title-menu selection values dispatched by run_title. */
#define TITLE_MENU_ITEM_NEW_GAME 0
#define TITLE_MENU_ITEM_CONTINUE 1

/* Value g_titleSelectedItem holds when the idle countdown expires. The same
 * value marks g_save_slot_index as having no selected slot on the fallback
 * field-entry path. */
#define TITLE_SELECTION_SENTINEL 0xFF

/* run_save_slot_menu result that returns from the picker to the title menu. */
#define SAVE_SLOT_MENU_EXIT_CANCEL 2

/* Fixed-address accesses used by run_title. */
#define TITLE_GLOBAL_RAM_BASE 0x80100000
#define TITLE_MENU_EXIT_STATE_WORD_INDEX 0x990
#define TITLE_SCENE_STATE_ADDRESS 0x801ED480

/* High rand() value placement in MenuLayout::rng_seed. */
#define TITLE_RNG_HIGH_SHIFT 15

/* AKAO sound command used before the fallback field-entry path. */
#define TITLE_SELECTION_SFX_ID 0x3C

/* Number of selectable slots in the title menu item-flag table
 * (g_titleMenuItemFlags), each occupying 2 bytes. */
#define TITLE_MENU_SLOT_COUNT 16

/* Idle countdown loaded by init_title_menu_state: 0xE10 == 3600 frames (~60 s at
 * 60 Hz) before the title times out to the attract loop. */
#define TITLE_IDLE_COUNTDOWN_FRAMES 0xE10

/* Full-screen fade encoding and blend modes. */
#define TITLE_FADE_NEUTRAL 0x100
#define TITLE_FADE_ADDITIVE_THRESHOLD (TITLE_FADE_NEUTRAL + 1)
#define TITLE_FADE_ADDITIVE_DRAW_MODE 0x25
#define TITLE_FADE_SUBTRACTIVE_DRAW_MODE 0x45

/** @brief Packet view for a fade TILE or draw-mode command. */
typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} TitleFadePrimitive;

/* The complete .data payload is extracted by Splat as named databins.
 * Symbols used by the code are declared in title_internal.h and assigned
 * fixed overlay addresses by config/symbols/title_symbol_addrs.txt. */

/** Advance a fade packet cursor by the concrete packet just emitted. */
#define TITLE_NEXT_FADE_PRIMITIVE(primitive, type) \
    ((TitleFadePrimitive*)((u8*)(primitive) + sizeof(type)))

/**
 * @brief Top-level entry point and main loop of the TITLE.BIN overlay.
 *
 * @details Boots the title audio (instrument bank, SEQ, then music), then
 * repeatedly initializes the title display, runs the menu render/input loop,
 * and dispatches the selected item. New Game opens the save-slot picker;
 * canceling that picker restarts the title menu, while confirming continues to
 * name entry. The main state machine calls this fixed address through its
 * temporary @c func_8004FC74 declaration.
 *
 * @param menu_context_address Address of the double-buffered MenuContext
 *        returned by get_title_menu_buffers; forwarded unchanged to the title
 *        display and menu routines.
 * @return Next game-state code consumed by the main state machine:
 *         - GAME_STATE_GNAME after New Game is confirmed.
 *         - GAME_STATE_MENU_LOAD when Continue is selected.
 *         - GAME_STATE_INTRO_MOVIE after the title idle timeout.
 *         - GAME_STATE_FIELD for the fallback field-entry path.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/mEAXF
 */
s32 run_title(s32 menu_context_address)
{
    s32 context_address;
    S_801ED480* persistent_scene_state = (S_801ED480*)TITLE_SCENE_STATE_ADDRESS;
    s32* global_ram_base;
    MenuLayout* menu_layout;
    u32 selection_sentinel;
    s32 random_low;
    s32 random_high;
    u8 selection;

    context_address = menu_context_address;

    load_title_audio_bank();
    load_title_seq(0);
    start_title_music();

    /* Access g_titleMenuExitState at 0x80102640 through the global-RAM base so
     * the configured MIPS_NONE relocation sites remain unchanged. */
    global_ram_base = (s32*)TITLE_GLOBAL_RAM_BASE;
    selection_sentinel = TITLE_SELECTION_SENTINEL;
    menu_layout = (MenuLayout*)g_menuLayoutBuffer;

    while (1)
    {
        init_title_display(context_address);
        persistent_scene_state->map_id = 0;
        persistent_scene_state->object_index = 0;
        persistent_scene_state->unk4 = 0;
        persistent_scene_state->unk8 = 0;
        persistent_scene_state->unkC = 0;

        do
        {
            render_menu(context_address);
        } while (global_ram_base[TITLE_MENU_EXIT_STATE_WORD_INDEX] == 0);

        D_80042FB4 = VSync(-1);
        selection = g_titleSelectedItem;

        if (selection == TITLE_MENU_ITEM_NEW_GAME)
        {
            load_menu_layout(0);
            global_ram_base[TITLE_MENU_EXIT_STATE_WORD_INDEX] = 0;
            if (run_save_slot_menu(context_address) == SAVE_SLOT_MENU_EXIT_CANCEL)
            {
                GFX_Transition(0);
                continue;
            }
            return GAME_STATE_GNAME;
        }
        else if (selection == TITLE_MENU_ITEM_CONTINUE)
        {
            return GAME_STATE_MENU_LOAD;
        }
        else if (selection == selection_sentinel)
        {
            stop_title_music();
            return GAME_STATE_INTRO_MOVIE;
        }
        else
        {
            akao_cmd_c1(0, TITLE_SELECTION_SFX_ID, 0);
            load_menu_layout(-1);
            g_save_slot_index = selection_sentinel;
            random_low = rand();
            random_high = rand();
            menu_layout->rng_seed = (s16)(random_low | (random_high << TITLE_RNG_HIGH_SHIFT));
            return GAME_STATE_FIELD;
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/bMLDn
 */
void render_menu(MenuContext* context)
{
    RECT rect;
    MenuContext* base = context;
    MenuContext* s0;
    u_long* s1;
    void* tmp;

    DrawSync(0);
    VSync(0);
    rect.x = 0;
    rect.y = 0;
    rect.w = 320;
    rect.h = 472;
    ClearImage(&rect, 0, 0, 0);

    s0 = base;
    ClearOTagR(s0->otag_buffer, 0x1000);
    ClearOTagR(s0->otag_buffer2, 0x1000);
    PutDispEnv(&s0->disp_env);
    update_controllers();
    SetDispMask(1);

    while (1)
    {
        s1 = s0->otag_buffer;
        ClearOTagR(s1, 0x1000);
        s0->next_prim_ptr = s0->prim_buffer;
        rand();
        VSync(1);
        render_fade_overlay(s0);
        render_title_backdrop(s0);
        render_title_menu_items(s0);
        handle_title_menu_input();

        if (g_titleMenuExitState == 0)
        {
            DrawSync(0);
            set_controller_vsync_interval(2);
            VSync(2);

            tmp = base;
            if (s0 == base)
            {
                tmp = s0->_pad4;
            }
            s0 = tmp;
            PutDispEnv(&s0->disp_env);
            PutDrawEnv(&s0->draw_env);
            DrawOTag((u_long*)(s1 + 4095));
            update_controllers();
            cdrom_process_state();
            if (g_titleMenuExitState == 0)
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
 * g_titleMenuExitState becomes non-zero (set by handle_save_slot_input),
 * then resets the frame-queue state and returns.
 *
 * @param ctx_base Base address of the double-buffered MenuContext render
 *        buffer, forwarded as-is from run_title.
 * @return The final value of g_titleMenuExitState: 1 after confirmation or
 *         SAVE_SLOT_MENU_EXIT_CANCEL when returning to the title menu.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/AKk7x
 */
s32 run_save_slot_menu(MenuContext* ctx_base)
{
    RECT rect;
    MenuContext* base;
    MenuContext* current;
    void* tmp;
    u_long* ot;

    base = ctx_base;

    InitSaveSlotMenu();
    GFX_Transition(0);
    set_fade_target(0x100, 0x100, 0x100, 0x14);
    DrawSync(0);
    VSync(0);
    rect.x = 0;
    rect.y = 0;
    rect.w = SCREEN_WIDTH;
    rect.h = 0x1D8;
    ClearImage(&rect, 0, 0, 0);
    current = base;
    ClearOTagR(current->otag_buffer, 0x1000);
    ClearOTagR(current->otag_buffer2, 0x1000);
    VSync(0);
    PutDispEnv(&current->disp_env);
    update_controllers();
    SetDispMask(1);
    do
    {
        ot = current->otag_buffer;
        ClearOTagR(ot, 0x1000);
        current->next_prim_ptr = current->prim_buffer;
        VSync(1);
        render_fade_overlay(current);
        RenderSaveSlotMenu(current);
        DrawSync(0);
        set_controller_vsync_interval(2);
        VSync(2);
        tmp = base;
        if (current == base)
        {
            tmp = current->_pad4;
        }
        current = tmp;
        PutDispEnv(&current->disp_env);
        PutDrawEnv(&current->draw_env);
        DrawOTag(ot + 4095); /* last entry of the 4096-word otag_buffer */
        update_controllers();
        cdrom_process_state();
    } while (g_titleMenuExitState == 0);
    reset_controller_vsync_state();
    VSync(0);
    return g_titleMenuExitState;
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
 * @param ctx_base Base address of the double-buffered MenuContext render
 *        buffer, forwarded as-is from run_title.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/evJur
 */
void init_title_display(MenuContext* ctx_base)
{
    RECT rect;
    u8* base = (u8*)ctx_base;
    u8* hw = (u8*)0x801ED600; /* hardware registers */

    /* Clear hardware register bytes */
    hw[0x13F] = 0;
    hw[0x91] = 0;
    hw[0x140] = 0;
    hw[0x92] = 0;

    akao_set_paused(0);
    SetGeomScreen(0x5DC);
    SetGeomOffset(0xA0, 0x78);

    /* Write shorts at offsets 0x40B0..0x40B6 */
    *(short*)(base + 0x40B0) = 0;
    *(short*)(base + 0x40B2) = 0;
    *(short*)(base + 0x40B4) = SCREEN_WIDTH;
    *(short*)(base + 0x40B6) = SCREEN_HEIGHT;

    /* _pad5 (mirrors _pad2, but past the back-buffer disp/draw env pair) */
    *(short*)&ctx_base->_pad5[0] = 0;
    *(short*)&ctx_base->_pad5[2] = VRAM_BACK_DISP_Y;
    *(short*)&ctx_base->_pad5[4] = SCREEN_WIDTH;
    *(short*)&ctx_base->_pad5[6] = SCREEN_HEIGHT;

    DrawSync(0);
    VSync(0);

    /* Clear the full VRAM extent */
    rect.x = 0;
    rect.y = 0;
    rect.w = VRAM_WIDTH;
    rect.h = VRAM_HEIGHT;
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
    set_fade_target(0x100, 0x100, 0x100, 0x14);
    init_title_menu_state();

    g_titleMenuExitState = 0;
}

/**
 * @brief Load and register the title overlay's AKAO instrument/sample bank.
 *
 * @details Counterpart of CHECKPS func_800500FC. Skipped if g_previousGameState
 * indicates the bank is already resident (values 2, 3, 5, 6, 7). Otherwise
 * loads SOUND/EFFECT.SET from CD-ROM into the 0x80180000 scratch buffer,
 * splits the blob via its self-referential offset table, copies the
 * instrument/sample sub-block to g_titleAudioBankBase (0x8013C000) and
 * registers it as the active AKAO bank, then uploads the trailing instrument
 * bank to the SPU.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/6zUZp
 */
void load_title_audio_bank(void)
{
    u8* base;
    u32* off;

    if (((u32)(g_previousGameState - 2) >= 2U) && (g_previousGameState != 6) && (g_previousGameState != 7) && (g_previousGameState != 5))
    {

        g_titleAudioBankBase = 0x8013C000;
        cdrom_queue_read(CD_RES_SOUND_EFFECT_SET, (void*)0x80180000);
        cdrom_wait_queue_empty();

        base = (u8*)0x80180000;
        off = (u32*)0x80180004;

        bcopy(base + off[0], (u8*)g_titleAudioBankBase, (int)(off[1] - off[0]));

        akao_register_bank((AkaoHeader*)g_titleAudioBankBase);
        akao_upload_bank_blocking((AkaoBankHeader*)(base + off[1]), 1);
    }
}

/**
 * @brief Load a title-screen sequence and its instrument bank from CD-ROM.
 *
 * @details Counterpart of CHECKPS func_80050138. Reads CD resource
 * @c CD_RES_MUSIC_FILE(seq_variant) into the 0x80180000 scratch
 * buffer, splits it via its self-referential offset table, copies the
 * sequence sub-block to D_8003ECA0, then uploads the trailing instrument bank.
 *
 * @param seq_variant Music-file index; 0 selects MSC_DATA.DAT.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/mBQ6i
 */
void load_title_seq(s32 seq_variant)
{
    u32* off;
    u8* base;

    cdrom_queue_read(CD_RES_MUSIC_FILE(seq_variant), (void*)0x80180000);
    cdrom_wait_queue_empty();

    off = (u32*)0x80180004;
    base = (u8*)0x80180000;

    bcopy(base + off[0], (u8*)&D_8003ECA0, (int)(off[1] - off[0]));
    akao_upload_bank_blocking((AkaoBankHeader*)(base + off[1]), 1);
}

/**
 * @brief Stop the title-screen background music.
 *
 * @details Counterpart of CHECKPS func_800501AC.
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
 * @details Counterpart of CHECKPS func_800501CC. Plays the SEQ loaded into
 * D_8003ECA0 (by load_title_seq) and sets the song volume to maximum via
 * akao_set_song_volume.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/xYPkq
 */
void start_title_music(void)
{
    akao_play_song((AkaoHeader*)&D_8003ECA0);
    akao_set_song_volume(0, AKAO_VOLUME_MAX);
}

/**
 * @brief Play a title-screen UI sound effect at maximum volume.
 *
 * @details Counterpart of CHECKPS func_800501FC (with the first parameter
 * folded away to a constant 0). sound_id values observed: 0x3C (selection
 * chime), 0x7C..0x7F (cursor / cancel / confirm beeps).
 *
 * @param sound_id Sound id forwarded to akao_play_sfx's arg0 (lower 10 bits used).
 * @param pan Forwarded to akao_play_sfx's arg2 (8-bit, possibly pan); every
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
    g_fadeCurrent.red = 0;
    g_fadeCurrent.green = 0;
    g_fadeCurrent.blue = 0;

    g_fadeTarget.red = 0;
    g_fadeTarget.green = 0;
    g_fadeTarget.blue = 0;
    g_fadeTarget.steps = 0;
}

/**
 * @brief Interpolate the screen fade and emit its overlay primitive.
 *
 * @details Counterpart of CHECKPS func_80050258: interpolates g_fadeCurrent
 * toward g_fadeTarget and emits the fade-overlay primitive into the active
 * prim buffer.
 *
 * @param ctx Active MenuContext render buffer.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/fBro2
 */
void render_fade_overlay(MenuContext* ctx)
{
    MenuContext* base = ctx;
    TitleFadePrimitive* primitive = (TitleFadePrimitive*)base->next_prim_ptr;
    u_long* ordering_table_tag = base->otag_buffer;
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;

    if (g_fadeTarget.steps != 0)
    {
        red_step = (g_fadeTarget.red - g_fadeCurrent.red) / g_fadeTarget.steps;
        green_step = (g_fadeTarget.green - g_fadeCurrent.green) / g_fadeTarget.steps;
        blue_step = (g_fadeTarget.blue - g_fadeCurrent.blue) / g_fadeTarget.steps;
        g_fadeTarget.steps = g_fadeTarget.steps - 1;
        g_fadeCurrent.red = g_fadeCurrent.red + red_step;
        g_fadeCurrent.green = g_fadeCurrent.green + green_step;
        g_fadeCurrent.blue = g_fadeCurrent.blue + blue_step;
    }
    else
    {
        g_fadeCurrent.red = g_fadeTarget.red;
        g_fadeCurrent.green = g_fadeTarget.green;
        g_fadeCurrent.blue = g_fadeTarget.blue;
    }
    if (!(((g_fadeCurrent.red == TITLE_FADE_NEUTRAL) && (g_fadeCurrent.green == TITLE_FADE_NEUTRAL)) &&
          (g_fadeCurrent.blue == TITLE_FADE_NEUTRAL)))
    {
        if (g_fadeCurrent.red >= TITLE_FADE_ADDITIVE_THRESHOLD)
        {
            primitive->tile.r0 = g_fadeCurrent.red - 1;
            primitive->tile.g0 = g_fadeCurrent.green - 1;
            primitive->tile.b0 = g_fadeCurrent.blue - 1;
        }
        else
        {
            if (g_fadeCurrent.red == TITLE_FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~g_fadeCurrent.red;
            }
            if (g_fadeCurrent.green == TITLE_FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~g_fadeCurrent.green;
            }
            if (g_fadeCurrent.blue == TITLE_FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~g_fadeCurrent.blue;
            }
        }

        setTile(&primitive->tile);
        setSemiTrans(&primitive->tile, 1);
        SET_YX0(&primitive->tile, 0, 0);
        setWH(&primitive->tile, SCREEN_WIDTH, SCREEN_HEIGHT);
        addPrim(ordering_table_tag, &primitive->tile);

        draw_mode = TITLE_FADE_ADDITIVE_DRAW_MODE;
        primitive = TITLE_NEXT_FADE_PRIMITIVE(primitive, TILE);
        if (g_fadeCurrent.red < TITLE_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = TITLE_FADE_SUBTRACTIVE_DRAW_MODE;
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
 * interpolates g_fadeCurrent toward this target over the given number of steps.
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
    g_fadeTarget.red = red;
    g_fadeTarget.green = green;
    g_fadeTarget.blue = blue;
    g_fadeTarget.steps = steps;
}

/**
 * @brief Emit the title screen's tiled backdrop strip.
 *
 * @details Emits 5 POLY_FT4 quads stepping 0x40 px, each linked into the
 * active OT's tail entry.
 *
 * @param ctx Active MenuContext render buffer.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/aKAFU
 */
void render_title_backdrop(MenuContext* ctx)
{
    u_long* ot;
    POLY_FT4* prim;
    s32 t0;
    s32 t1;
    s32 a3;
    s32 temp_v0;
    s32 temp_v1;

    prim = (POLY_FT4*)ctx->next_prim_ptr;
    ot = ctx->otag_buffer;
    t0 = 0;

    while (t0 < 5)
    {
        a3 = 0x40 + (t0 << 6);
        prim->x3 = (short)a3;
        prim->x1 = (short)a3;
        t1 = 0x140 + (t0 << 6);
        temp_v1 = t1 & 0x3FF;
        temp_v0 = t0 << 6;
        t0++;
        prim->x2 = (short)temp_v0;
        prim->x0 = (short)temp_v0;
        setlen(prim, 9);
        setcode(prim, 0x2C);
        prim->b0 = 0x80;
        prim->g0 = 0x80;
        prim->r0 = 0x80;
        prim->y1 = 0;
        prim->y0 = 0;
        prim->y3 = 0xE0;
        prim->y2 = 0xE0;
        prim->u2 = 0;
        prim->u0 = 0;
        prim->u3 = 0x40;
        prim->u1 = 0x40;
        prim->v1 = 8;
        prim->v0 = 8;
        prim->v3 = 0xE8;
        prim->v2 = 0xE8;
        prim->tpage = (u_short)((temp_v1 >> 6) | 0x110);
        prim->clut = 0x7840;
        /* addPrim((P_TAG *)(ot + 4095), prim) */
        ((P_TAG*)prim)->addr = (u_long)(((P_TAG*)(ot + 4095))->addr);
        ((P_TAG*)(ot + 4095))->addr = (u_long)prim;
        prim = (POLY_FT4*)(((u8*)prim) + 0x28);
    }

    ctx->next_prim_ptr = (u_long*)prim;
}

/**
 * @brief Per-frame input dispatcher for the main title menu.
 *
 * @details Ticks the idle countdown (dispatching the idle-quit item once it
 * expires), then debounces the confirm/cancel and cursor up/down button
 * combos and moves the cursor or arms g_titleMenuExitState accordingly.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/vmcmD
 */
void handle_title_menu_input(void)
{
    s32 op = 0x7C;
    update_menu_input();
    if (g_titleIdleCountdown == 0)
    {
        g_titleMenuExitState = 1;
        g_titleSelectedItem = 0xFF;
        return;
    }
    g_titleIdleCountdown -= 1;
    if (g_debouncedInput & (PADh | PADi | PADRright))
    {
        play_title_sfx(op, 0x80);
        g_titleMenuExitState = 1;
        return;
    }
    if (g_debouncedInput & (PADLup | PADLleft))
    {
        menu_cursor_up();
        play_title_sfx(0x7D, 0x80);
    }
    else if (g_debouncedInput & (PADLdown | PADLright | PADselect))
    {
        menu_cursor_down();
        play_title_sfx(0x7D, 0x80);
    }
}

/**
 * @brief Move the menu cursor to the next enabled slot, wrapping to slot 0
 *        if none remain.
 *
 * @details Linear-search g_titleMenuItemFlags forward for the next enabled
 * menu slot. If none remain, the cursor is reset to slot 0 with rank 0.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/6k8uV
 */
void menu_cursor_down(void)
{
    s32 item;
    u8* item_ptr;
    item = g_titleSelectedItem + 1;
    if (item < TITLE_MENU_SLOT_COUNT)
    {
        u8* flags_base = g_titleMenuItemFlags; // forces lui/addiu first
        item_ptr = flags_base + item * 2;      // sll comes after
        while (1)
        {
            if (*item_ptr != 0)
            {
                break;
            }
            item++;
            if (item < TITLE_MENU_SLOT_COUNT)
            {
                item_ptr += 2;
                continue;
            }
            break;
        }
    }
    if (item == TITLE_MENU_SLOT_COUNT)
    {
        g_titleVisibleItemRank = 0;
        g_titleSelectedItem = 0;
    }
    else
    {
        g_titleSelectedItem = (u8)item;
        g_titleVisibleItemRank++;
    }
}

/**
 * @brief Move the menu cursor to the previous enabled slot, wrapping to the
 *        last enabled slot if already at the top.
 *
 * @details Mirror of menu_cursor_down searching backward. Scans
 * g_titleMenuItemFlags from the slot before the current selection toward 0
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
    s32 idx;
    s32 last_enabled;
    s32 enabled_count;
    u8* scan_ptr;
    u8* item_ptr;
    u8* flags_base;
    u8 rank;
    item = g_titleSelectedItem - 1;
    if (item >= 0)
    {
        flags_base = &g_titleMenuItemFlags[0];
        item_ptr = flags_base + (item * 2);
        while (item >= 0)
        {
            if ((*item_ptr) != 0)
            {
                break;
            }
            item--;
            item_ptr -= 2;
        }
    }
    if (item < 0)
    {
        enabled_count = 0;
        item = 0;
        scan_ptr = &g_titleMenuItemFlags[0];

        while (item < TITLE_MENU_SLOT_COUNT)
        {
            if (*scan_ptr != 0)
            {
                enabled_count++;
                last_enabled = item;
            }

            item++;
            scan_ptr += 2;
        }

        g_titleVisibleItemRank = enabled_count - 1;
        g_titleSelectedItem = (u8)last_enabled;
        return;
    }
    rank = g_titleVisibleItemRank;
    g_titleSelectedItem = (u8)item;
    g_titleVisibleItemRank = rank - 1;
    return;
}

/**
 * @brief Emit the title-menu header, each enabled item, and the cursor.
 *
 * @details Walks the 16-slot g_titleMenuItemFlags table and emits one
 * emit_menu_item_quad per enabled slot, stacking them vertically (item_y += 0xC
 * each). The currently selected item (visible_index == g_titleVisibleItemRank)
 * uses CLUT 1, the rest CLUT 2. A fixed header quad is emitted first, and the
 * cursor quad last; the cursor's U coordinate cycles through
 * g_cursorBlinkUOffsets[(g_titleAnimFrame >> 2) & 3] for a 4-frame blink, and
 * its Y tracks the selected rank. The advanced prim cursor is written back to
 * MenuContext::next_prim_ptr.
 *
 * @param ctx Active MenuContext render buffer.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/qegw7
 */
void render_title_menu_items(void* ctx)
{
    s32 ot_head;
    s32 prim;
    s32 slot;
    s32 visible_index;
    s32 item_y;
    s32 item_x;
    u8* flag_ptr;
    s32 first_prim;
    s32 item_visible;
    s32 result;
    u8 anim;

    ot_head = (s32)(((u8*)ctx) + 0x40);
    first_prim = *((s32*)(((u8*)ctx) + 0x80B8));
    prim = emit_menu_item_quad(ot_head, first_prim, 0, 0x64, 0xC8, 0, 0x80, 1);
    item_x = 0x88;
    item_y = 0xA0;
    visible_index = 0;
    slot = 0;
    flag_ptr = &g_titleMenuItemFlags[0];
    do
    {
        item_visible = (*flag_ptr) != 0;
        if (item_visible)
        {
            prim = (s32)emit_menu_item_quad(ot_head, prim, slot + 1, item_x, item_y, 0, 0x80, (g_titleVisibleItemRank == visible_index) ? 1 : 2);
            item_y += 0xC;
            visible_index++;
            prim += 0x28;
        }
        slot++;
        flag_ptr += 2;
    } while (slot < 0x10);
    result = (s32)emit_menu_item_quad(ot_head, prim, 7, 0x78, (6 * (2 * ((s32)g_titleVisibleItemRank))) + 0x9D,
                                      (s32)g_cursorBlinkUOffsets[(g_titleAnimFrame >> 2) & 3], 0x10, 0);

    anim = g_titleAnimFrame;
    *((s32*)(((u8*)ctx) + 0x80B8)) = result;
    g_titleAnimFrame = anim + 1;
}

/**
 * @brief Build one menu-item POLY_FT4 (textured quad) and link it into the OT.
 *
 * @details Hand-writes a libgpu POLY_FT4 (code 0x2C, len 9, equivalent to
 * setPolyFT4) at screen (x, y) with size @p width, samples a 16-px-tall
 * texture row selected by @p tex_row (V = tex_row*16 .. +0x10), and links it
 * at the head of @p ot_head. The four corners are neutral-grey (0x80) flat
 * shaded; the texture page is fixed at 5 and the CLUT y is fixed at 480
 * (the packed 0x7800), with @p clut_index choosing the CLUT x.
 *
 * Uses setPolyFT4 for the tag; the remaining fields are written as raw
 * byte/halfword stores rather than setUV4/setXY4/setRGB0/setClut/addPrim,
 * because the original interleaves them in a non-canonical order (and hoists
 * the OT-link load) that those bulk macros would reorder, breaking the match.
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
void* emit_menu_item_quad(s32* ot_head, void* prim, s32 tex_row, s16 x, s32 y, s32 u0_base, s32 width, s32 clut_index)
{
    u8* ptr;
    u8 v_top;
    u8 v_bottom;
    u16 x_right;
    u16 y_bottom;
    u8 u_right;
    u8* y1_ptr;
    u16 clut_word;
    u32 old_word;
    u32 new_word;
    u32 addr_mask;
    u32 tag_mask;
    ptr = (u8*)prim;
    addr_mask = 0x00FFFFFF;
    setPolyFT4(ptr); /* len = 9, code = 0x2C */
    v_top = (u8)(tex_row << 4);
    ptr[0x06] = 0x80;  /* b0 */
    ptr[0x15] = v_top; /* v1 */
    ptr[0x0D] = v_top; /* v0 */
    v_bottom = (u8)((tex_row << 4) + 0x10);
    ptr[0x05] = 0x80;               /* g0 */
    ptr[0x04] = 0x80;               /* r0 */
    ptr[0x25] = v_bottom;           /* v3 */
    ptr[0x1D] = v_bottom;           /* v2 */
    *((u16*)(ptr + 0x18)) = (u16)x; /* x2 */
    *((u16*)(ptr + 0x08)) = (u16)x; /* x0 */
    *((u16*)(ptr + 0x16)) = 5;      /* tpage */
    tag_mask = 0xFF000000;
    x_right = (u16)(x + width);
    *((u16*)(ptr + 0x20)) = x_right; /* x3 */
    *((u16*)(ptr + 0x10)) = x_right; /* x1 */
    y1_ptr = ptr + 0x12;
    *((u16*)y1_ptr) = (u16)y;       /* y1 */
    *((u16*)(ptr + 0x0A)) = (u16)y; /* y0 */
    y_bottom = (u16)(y + 0x10);
    ptr[0x1C] = (u8)u0_base; /* u2 */
    ptr[0x0C] = (u8)u0_base; /* u0 */
    u_right = (u8)(u0_base + width);
    clut_word = (u16)((clut_index & 0x3F) | 0x7800);
    *((u16*)(ptr + 0x22)) = y_bottom; /* y3 */
    *((u16*)(ptr + 0x1A)) = y_bottom; /* y2 */
    old_word = *((u32*)ptr);
    ptr[0x24] = u_right;               /* u3 */
    ptr[0x14] = u_right;               /* u1 */
    *((u16*)(ptr + 0x0E)) = clut_word; /* clut */
    new_word = (old_word & tag_mask) | (((u32)(*ot_head)) & addr_mask);
    *((u32*)ptr) = new_word;
    *ot_head = (s32)((((u32)(*ot_head)) & tag_mask) | (((u32)ptr) & addr_mask));
    return (void*)(ptr + 0x28);
}

/**
 * @brief Initialise the title-menu state globals and upload its TIMs.
 *
 * @details Zeros the per-slot flag table (TITLE_MENU_SLOT_COUNT entries),
 * enables the first 4 slots, resets cursor/input/animation globals, arms the
 * idle countdown to TITLE_IDLE_COUNTDOWN_FRAMES, and uploads the two menu TIMs
 * from g_titleMenuTimTable[1..2] to VRAM. When re-entering from the attract
 * loop (g_previousGameState == 0) it advances the cursor to the first enabled
 * slot (same forward scan as menu_cursor_down).
 *
 * @see decomp.me (100%) https://decomp.me/scratch/HW23j
 */
void init_title_menu_state(void)
{
    u8* flag_ptr;
    s32 i;
    s32 next_item;
    u8* item_ptr;
    i = 0;
    flag_ptr = g_titleMenuItemFlags;
    for (i = 0; i < TITLE_MENU_SLOT_COUNT; i++)
    {
        flag_ptr[0] = 0;
        flag_ptr[1] = 0;
        flag_ptr += 2;
    }

    g_titleMenuItemFlags[0] = 1;
    g_titleMenuItemFlags[1] = 1;
    g_titleMenuItemFlags[2] = 1;
    g_titleMenuItemFlags[3] = 1;

    g_titleVisibleItemRank = 0;

    g_titleSelectedItem = 0;
    g_titleAnimFrame = 0;
    g_inputRepeatTimer = 0;
    g_lastInputState = 0;
    g_debouncedInput = 0;
    g_titleIdleCountdown = TITLE_IDLE_COUNTDOWN_FRAMES;
    upload_tim((void*)(((u8*)&g_titleMenuTimTable) + g_titleMenuTimTable[1]), 0x140, 0, 0, 0x1E0);
    upload_tim((void*)(((u8*)&g_titleMenuTimTable) + g_titleMenuTimTable[2]), 0x140, 0x100, 0, 0x1E1);
    if (g_previousGameState == 0)
    {
        next_item = g_titleSelectedItem + 1;
        if (next_item < TITLE_MENU_SLOT_COUNT)
        {
            item_ptr = g_titleMenuItemFlags + (next_item << 1);
            do
            {
                if ((g_titleMenuTimTable && g_titleMenuTimTable) && g_titleMenuTimTable)
                {
                }
                if ((*item_ptr) != 0)
                {
                    break;
                }
                next_item++;
                item_ptr += 2;
            } while (next_item < TITLE_MENU_SLOT_COUNT);
        }
        if (next_item == TITLE_MENU_SLOT_COUNT)
        {
            g_titleVisibleItemRank = 0;
            g_titleSelectedItem = 0;
        }
        else
        {
            g_titleSelectedItem = (u8)next_item;
            g_titleVisibleItemRank++;
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
        rect.x = clut_x;
        rect.y = (s16)clut_y;
        rect.w = clut_width * clut_height;
        rect.h = clut_rows;
        LoadImage(&rect, (u_long*)(p + 0x14));
        p = (p + clut_skip_base) + clut_block_len;
    }
    else
    {
        p = p + 8;
    }
    rect.x = x;
    rect.y = y;
    rect.w = *((u16*)(p + 8));
    rect.h = *((u16*)(p + 0xA));
    LoadImage(&rect, (u_long*)(p + 0xC));
}

/**
 * @brief Reads the SCD pad state and returns the remapped button bitmap.
 *
 * Same byte-swap and button-remap as @p read_pad_input, but returns the
 * computed bitmap directly instead of writing it into @p g_lastInputState
 * and resetting @p g_inputRepeatTimer. The body type style (loose unsigned
 * locals, no SCDRegs alias) suggests this is a pre-refactor fossil that
 * @p read_pad_input later superseded.
 *
 * @note No callers exist in the linked binary — dead code preserved by
 *       the original build. Kept here so the address-stable layout of
 *       the TITLE overlay is reproduced byte-for-byte.
 *
 * @return Remapped button bitmap, or 0 if the pad is not present
 *         (raw status byte at @p 0x801ED600 ≥ 0xFE).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/Z5swg
 */
s32 read_pad_state(void)
{
    signed short axis_x_dup;
    unsigned char* ptr;
    unsigned char device_status;
    unsigned short raw_buttons;
    unsigned short raw_buttons_hi;
    unsigned long buttons;
    unsigned int raw_buttons_reread;
    signed short axis;
    ptr = (unsigned char*)0x801ED600;
    device_status = ptr[0];
    if (device_status >= 0xFE)
    {
        return 0;
    }
    raw_buttons = *((unsigned short*)(ptr + 2));
    raw_buttons_reread = *((unsigned short*)(ptr + 2));
    raw_buttons_hi = raw_buttons_reread;
    buttons = (raw_buttons >> 8) | (raw_buttons_hi << 8);
    buttons = PAD_REMAP_FACE_BITS(buttons);
    if (device_status)
    {
        axis = *((signed short*)(ptr + 0x2C));
        axis_x_dup = axis;
        if (axis < (-1))
        {
            buttons |= PAD_BTN_LEFT;
        }
        else if (axis_x_dup >= 2)
        {
            buttons |= PAD_BTN_RIGHT;
        }
        axis = *((signed short*)(ptr + 0x2E));
        if (axis < (-1))
        {
            buttons |= PAD_BTN_UP;
        }
        else if (axis >= 2)
        {
            buttons |= PAD_BTN_DOWN;
        }
    }
    return buttons;
}

/**
 * @brief Read the SCD pad, debounce it, and publish the result in g_debouncedInput.
 *
 * @details Counterpart of CHECKPS UpdateInputDebounced. Builds the remapped
 * button bitmap (byte-swap + face-bit remap + analog-stick to d-pad
 * thresholding) exactly like read_pad_input, then runs an auto-repeat state
 * machine over g_lastInputState / g_inputRepeatTimer:
 *  - If this frame matches the previous state (or shares any repeat-eligible
 *    bit with it), only the d-pad directions auto-repeat: g_debouncedInput
 *    fires every (2 + 1) frames while held, otherwise it is suppressed.
 *  - A fresh, different press is published immediately and arms the longer
 *    initial-repeat delay (15 frames).
 *  - No input clears all three globals.
 *
 * @note The pad registers are read through a raw pointer (with a duplicate
 *       held_buttons load and a volatile axis_y read) rather than the SCDRegs
 *       struct used by read_pad_input; those reads are required artifacts
 *       of the matched codegen, so they are left as-is.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/geg1v
 */
void update_menu_input(void)
{
    u8* ptr = (u8*)0x801ED600;
    u8 device_status = D_801ED600[0];
    u16 raw_buttons;
    u16 unused;
    u32 buttons;
    s16 axis;
    s32 state;
    if (device_status >= 0xFE)
    {
        state = 0;
    }
    else
    {
        raw_buttons = *((u16*)(ptr + 2));

        buttons = (raw_buttons >> 8) | (*((u16*)(2 + ptr)) << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);
        if ((*ptr) != 0)
        {
            axis = *((s16*)(ptr + 0x2C));
            if (axis < (-1))
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }
            axis = *((volatile s16*)(ptr + 0x2E));
            if (axis < (-1))
            {
                buttons |= PAD_BTN_UP;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_DOWN;
            }
        }
        state = buttons;
    }
    g_debouncedInput = 0;

    if (((state == g_lastInputState) || ((g_lastInputState != 0) && (state & (g_lastInputState | 0xB6F)))) && state != 0)
    {
        u32 dpad = state & (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT);
        if (dpad != 0)
        {
            state = dpad;
        }
        if (g_inputRepeatTimer == 0)
        {
            g_debouncedInput = state;
            g_inputRepeatTimer = 2;
        }
        else
        {
            g_inputRepeatTimer--;
            g_debouncedInput = 0;
        }
        return;
    }
    else if (state == 0)
    {
        (void)(&g_debouncedInput);
        *((s32*)(&g_inputRepeatTimer)) = 0;
        *((s32*)(&g_lastInputState)) = 0;
    }
    else
    {
        g_debouncedInput = state;
        g_lastInputState = state;
        g_inputRepeatTimer = 15;
    }
}
