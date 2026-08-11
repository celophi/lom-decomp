#include "title.h"
#include "cd_resources.h"
#include "display.h"
#include "gpu_packet.h"

/* Width in pixels of a single save-slot panel; one horizontal slide moves the
 * stage by exactly this much. */
#define SLOT_PANEL_WIDTH 160

/* Number of frames the slide-lerper takes to animate a full panel scroll. */
#define SLOT_SLIDE_FRAMES 8

/* CD resource index of the first title SEQ. load_title_seq adds the variant
 * index to this base to obtain the actual resource passed to cdrom_queue_read. */
#define TITLE_SEQ_RESOURCE_BASE 0x17

/* Length, in 32-bit words, of each sub-menu layout table copied by
 * load_sub_menu_layout (0x94 words == 0x250 bytes). */
#define SUB_MENU_LAYOUT_WORDS 0x94U

/* Length, in 32-bit words, of a full menu-layout template copied by
 * load_menu_layout (0xC9A words). */
#define MENU_LAYOUT_WORDS 0xC9AU

/* Value g_titleSelectedItem holds when the idle countdown expires with no
 * input (set by handle_title_menu_input); dispatching it quits the title back to
 * the attract loop. The same 0xFF is also written into g_save_slot_index ("no save
 * slot selected") on the load-a-saved-game path. */
#define TITLE_ITEM_IDLE_QUIT 0xFF

/* Number of selectable slots in the title menu item-flag table
 * (g_titleMenuItemFlags), each occupying 2 bytes. */
#define TITLE_MENU_SLOT_COUNT 16

/* Idle countdown loaded by init_title_menu_state: 0xE10 == 3600 frames (~60 s at
 * 60 Hz) before the title times out to the attract loop. */
#define TITLE_IDLE_COUNTDOWN_FRAMES 0xE10

static void scroll_slots_right(void);
static void scroll_slots_left(void);

/**
 * @brief Top-level entry point and main loop of the TITLE.BIN overlay.
 *
 * @details Boots the title audio (instrument bank, SEQ, then music), then
 * repeatedly initialises the title display, runs the menu render/input loop
 * until the player makes a selection, and dispatches on @c g_titleSelectedItem
 * to choose which outer game state to return into. Mirror of @ref RunCheckPS
 * in the CHECKPS overlay. Invoked from the main state machine (g_gameState
 * case 2) as @c g_gameState = run_title(...). The previously-decompiled symbol
 * was @c title_func_8004FC74.
 *
 * @param base_address Base address of the double-buffered MenuContext render
 *        buffer (returned by get_title_menu_buffers in main.c); forwarded as-is to
 *        init_title_display, render_menu and run_save_slot_menu.
 * @return Next game-state code consumed by the main state machine:
 *         - 3: "New Game" confirmed in the save-slot menu (start a new field).
 *         - 7: menu item 1 selected (continue / load a saved game).
 *         - 8: idle timeout (@ref TITLE_ITEM_IDLE_QUIT); music is stopped and
 *              control returns to the attract loop.
 *         - 0: any other item; arms the load path (g_save_slot_index, layout template,
 *              RNG seed) and drops into the field/demo state.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/mEAXF
 */
s32 run_title(s32 base_address)
{
    s32 ctx_base;
    S_801ED480* scene_state = (S_801ED480*)0x801ED480;
    s32* exit_state_base;
    MenuLayout* layout;
    u32 idle_quit;
    s32 rand_lo, rand_hi;
    u8 selected_item;
    ctx_base = base_address;

    load_title_audio_bank();
    load_title_seq(0);
    start_title_music();
    /* 0x80100000 is the global-RAM base; g_titleMenuExitState lives at
     * 0x80102640 (word index 0x990). The exit-state flag is read/written
     * through this base pointer rather than via the symbol directly so the
     * build suppresses its relocations (see config/relocations/
     * title_reloc_addrs.txt). This indirection is load-bearing for the
     * byte-for-byte match - do not collapse it to g_titleMenuExitState. */
    exit_state_base = (s32*)0x80100000;
    idle_quit = TITLE_ITEM_IDLE_QUIT;
    layout = (MenuLayout*)g_menuLayoutBuffer;
    while (1)
    {
        init_title_display(ctx_base);
        scene_state->map_id = 0;
        scene_state->object_index = 0;
        scene_state->unk4 = 0;
        scene_state->unk8 = 0;
        scene_state->unkC = 0;
        do
        {
            render_menu(ctx_base);
        } while (exit_state_base[0x990] == 0); /* wait until g_titleMenuExitState != 0 */

        D_80042FB4 = VSync(-1);
        selected_item = g_titleSelectedItem;

        if (selected_item == 0)
        {
            load_menu_layout(0);
            exit_state_base[0x990] = 0; /* g_titleMenuExitState = 0 */
            if (run_save_slot_menu(ctx_base) == 2)
            {
                GFX_Transition(0);
                continue;
            }
            return GAME_STATE_GNAME;
        }
        else if (selected_item == 1)
        {
            return GAME_STATE_MENU_LOAD;
        }
        else if (selected_item == idle_quit)
        {
            stop_title_music();
            return GAME_STATE_INTRO_MOVIE;
        }
        else
        {
            akao_cmd_c1(0, 0x3C, 0);
            load_menu_layout(-1);
            g_save_slot_index = idle_quit;
            rand_lo = rand();
            rand_hi = rand();
            layout->rng_seed = (s16)(rand_lo | (rand_hi << 0xF));
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
 * @return The final value of g_titleMenuExitState: 2 if the player
 *         confirmed a save slot, otherwise the cancel/back code.
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
 * @details Counterpart of InitCheckPSDisplay in the CHECKPS overlay. Clears
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
 * registers it as the active AKAO bank, then submits the trailing AKAO
 * sequence sub-block for blocking playback.
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

        akao_register_bank((AkaoSeqHeader*)g_titleAudioBankBase);
        akao_play_sequence_blocking((AkaoSeqHeader*)(base + off[1]), 1);
    }
}

/**
 * @brief Load and play a title-screen SEQ resource from CD-ROM.
 *
 * @details Counterpart of CHECKPS func_80050138. Reads CD resource
 * (TITLE_SEQ_RESOURCE_BASE + seq_variant) into the 0x80180000 scratch
 * buffer, splits it via its self-referential offset table, copies the
 * sequence sub-block to D_8003ECA0, then submits it for blocking playback.
 *
 * @param seq_variant Offset added to TITLE_SEQ_RESOURCE_BASE to select
 *        which title SEQ variant to load.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/mBQ6i
 */
void load_title_seq(s32 seq_variant)
{
    u32* off;
    u8* base;

    cdrom_queue_read((seq_variant + TITLE_SEQ_RESOURCE_BASE) & 0xFFFF, (void*)0x80180000);
    cdrom_wait_queue_empty();

    off = (u32*)0x80180004;
    base = (u8*)0x80180000;

    bcopy(base + off[0], (u8*)&D_8003ECA0, (int)(off[1] - off[0]));
    akao_play_sequence_blocking((AkaoSeqHeader*)(base + off[1]), 1);
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
 * akao_cmd_c0.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/xYPkq
 */
void start_title_music(void)
{
    akao_play_song(&D_8003ECA0);
    akao_cmd_c0(0, 0x7F);
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
 * @details Counterpart of CHECKPS ResetFadeState.
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
    u32* var_t4 = (u32*)base->next_prim_ptr;
    u32* unk40_ptr = (u32*)base->otag_buffer;
    s32 temp_a2;
    s32 temp_a0;
    s32 temp_v1;
    s32 var_a1;
    if (g_fadeTarget.steps != 0)
    {
        temp_a2 = ((s32)(g_fadeTarget.red - g_fadeCurrent.red)) / ((s32)g_fadeTarget.steps);
        temp_a0 = ((s32)(g_fadeTarget.green - g_fadeCurrent.green)) / ((s32)g_fadeTarget.steps);
        temp_v1 = ((s32)(g_fadeTarget.blue - g_fadeCurrent.blue)) / ((s32)g_fadeTarget.steps);
        g_fadeTarget.steps = g_fadeTarget.steps - 1;
        g_fadeCurrent.red = g_fadeCurrent.red + temp_a2;
        g_fadeCurrent.green = g_fadeCurrent.green + temp_a0;
        g_fadeCurrent.blue = g_fadeCurrent.blue + temp_v1;
    }
    else
    {
        g_fadeCurrent.red = g_fadeTarget.red;
        g_fadeCurrent.green = g_fadeTarget.green;
        g_fadeCurrent.blue = g_fadeTarget.blue;
    }
    if (!(((g_fadeCurrent.red == 0x100) && (g_fadeCurrent.green == 0x100)) && (g_fadeCurrent.blue == 0x100)))
    {
        if (((s32)g_fadeCurrent.red) >= 0x101)
        {
            ((TILE*)var_t4)->r0 = ((u8)g_fadeCurrent.red) - 1;
            ((TILE*)var_t4)->g0 = ((u8)g_fadeCurrent.green) - 1;
            ((TILE*)var_t4)->b0 = ((u8)g_fadeCurrent.blue) - 1;
        }
        else
        {
            if (g_fadeCurrent.red == 0x100)
            {
                ((TILE*)var_t4)->r0 = 0;
            }
            else
            {
                ((TILE*)var_t4)->r0 = ~((u8)g_fadeCurrent.red);
            }
            if (g_fadeCurrent.green == 0x100)
            {
                ((TILE*)var_t4)->g0 = 0;
            }
            else
            {
                ((TILE*)var_t4)->g0 = ~((u8)g_fadeCurrent.green);
            }
            if (g_fadeCurrent.blue == 0x100)
            {
                ((TILE*)var_t4)->b0 = 0;
            }
            else
            {
                ((TILE*)var_t4)->b0 = ~((u8)g_fadeCurrent.blue);
            }
        }
        setlen(var_t4, 3);
        setcode(var_t4, 0x62);
        ((TILE*)var_t4)->w = SCREEN_WIDTH;
        ((TILE*)var_t4)->y0 = 0;
        ((TILE*)var_t4)->x0 = 0;
        ((TILE*)var_t4)->h = SCREEN_HEIGHT;
        *var_t4 = ((*var_t4) & 0xFF000000) | ((*unk40_ptr) & 0xFFFFFF);
        *unk40_ptr = ((*unk40_ptr) & 0xFF000000) | (((u32)var_t4) & 0xFFFFFF);
        ;
        var_a1 = 0x25;
        var_t4 = (u32*)(((u8*)var_t4) + 0x10);
        if (((s32)g_fadeCurrent.red) < 0x101)
        {
            var_a1 = 0x45;
        }
        setlen(var_t4, 1);
        ((DR_TPAGE*)var_t4)->code[0] = (s32)(var_a1 | 0xE1000000);
        *var_t4 = ((*var_t4) & 0xFF000000) | ((*unk40_ptr) & 0xFFFFFF);
        *unk40_ptr = ((*unk40_ptr) & 0xFF000000) | (((u32)var_t4) & 0xFFFFFF);
        var_t4 = (u32*)(((u8*)var_t4) + 8);
    }
    base->next_prim_ptr = (u_long*)var_t4;
}

/**
 * @brief Set the target fade color and step count for the title-screen fade.
 *
 * @details Counterpart of CHECKPS SetFadeTarget. render_fade_overlay
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
        idx = 0;
        do
        {
            scan_ptr = &g_titleMenuItemFlags[0];
            while (idx < TITLE_MENU_SLOT_COUNT)
            {
                if ((*scan_ptr) != 0)
                {
                    enabled_count++;
                    last_enabled = idx;
                }
                idx++;
                scan_ptr += 2;
            };
        } while (0);

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
 * g_cursorBlinkPalette[(g_titleAnimFrame >> 2) & 3] for a 4-frame blink, and
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
                                      (s32)g_cursorBlinkPalette[(g_titleAnimFrame >> 2) & 3], 0x10, 0);
    
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
    y1_ptr = ptr + 0x12;
    *((u16*)y1_ptr) = (u16)y;       /* y1 */
    *((u16*)(ptr + 0x0A)) = (u16)y; /* y0 */
    y_bottom = (u16)(y + 0x10);
    do
    {
    } while (0);
    ptr[0x1C] = (u8)u0_base; /* u2 */
    ptr[0x0C] = (u8)u0_base; /* u0 */
    u_right = (u8)(u0_base + width);
    clut_word = (u16)((clut_index & 0x3F) | 0x7800);
    *((u16*)(ptr + 0x22)) = y_bottom; /* y3 */
    *((u16*)(ptr + 0x1A)) = y_bottom; /* y2 */
    old_word = *((u32*)ptr);
    *((u16*)(ptr + 0x20)) = x_right;   /* x3 */
    *((u16*)(ptr + 0x10)) = x_right;   /* x1 */
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
 *       struct used by read_pad_input; those reads are load-bearing artifacts
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

/**
 * Counterpart of CHECKPS UpdateControllerInput.
 *
 * decomp.me (100%) https://decomp.me/scratch/1dQbp
 */
static void read_pad_input(void)
{
    SCDRegs* base = SCD_REGS;
    s32 state;
    u32 buttons;
    s16 axis;

    g_debouncedInput = 0;
    if (D_801ED600[0] >= 254)
    {
        state = 0;
    }
    else
    {
        buttons = ((base->held_buttons >> 8) & 0xFF) | (base->held_buttons << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);
        if (base->device_type != 0)
        {
            axis = base->axis_x;
            if (axis < (-1))
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }
            axis = base->axis_y;
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
    g_lastInputState = state;
    g_inputRepeatTimer = 15;
}

/**
 * Initialises the save-slot sub-menu state and uploads its sprite atlases.
 *
 * decomp.me (100%) https://decomp.me/scratch/t2lHt
 */
void InitSaveSlotMenu(void)
{
    read_pad_input();
    g_slotSlideFrames = 0;
    g_slotSlideYLerped = 0;
    g_slotSlideY = 0;
    g_slotSlideXLerped = 0;
    g_slotSlideX = 0;
    g_slotSelectedIndex = 0;
    g_slotHighlightX = 0;
    g_slotHighlightTargetX = 0;
    g_slotHighlightFrames = 0;
    upload_save_layout_textures();
}

/**
 * decomp.me (100%) https://decomp.me/scratch/so5cY
 */
void RenderSaveSlotMenu(MenuContext* arg0)
{
    arg0->next_prim_ptr = (u_long*)RenderSaveLayoutPrims(arg0->next_prim_ptr, (u_long*)((char*)arg0 + 0x40));
    handle_save_slot_input();
}

/**
 * @brief Per-frame input dispatcher for the save-slot sub-menu.
 *
 * @details While a slide is in flight (g_slotSlideFrames != 0) it only steps
 * the X/Y slide lerpers and returns. Once settled it snaps the lerpers to
 * their targets, reads input, and branches on whether the stage is at its
 * home column (g_slotSlideX == 0) or scrolled to a side panel:
 *  - Home column: confirm toggles the new-game expand entries (18/19), a
 *    left/right press scrolls to a side panel, and cancel quits the sub-menu
 *    (g_titleMenuExitState = 2).
 *  - Side panel: confirm loads the matching sub-menu layout, seeds its RNG,
 *    copies the selected save record into D_80043618, clears the per-slot
 *    field of every other menu-layout slot, and confirms (exit state 1);
 *    cancel scrolls back home; up/down move the slot cursor (wrapping over
 *    the 11 slots). Always re-runs the highlight-panel animation.
 *
 * @see decomp.me (99.41%) https://decomp.me/scratch/Xl8gF
 */
void handle_save_slot_input(void)
{
    s32 slide_x_step;
    s32 rng_lo;
    u8* unused_ptr;
    s32* slide_x_lerped_ptr;
    s32 prev_index;
    int rng_hi;
    s32 next_index;
    s32 selected_slot;
    s32 slide_y_step;
    s32 slot_idx;
    s32 new_flags;
    u32 copy_count;
    u8 byte;
    MenuLayout* layout;
    u8* src_ptr;
    u8* dest_ptr;
    s32 flag_mask;
    if (g_slotSlideFrames != 0)
    {
        slide_x_lerped_ptr = &g_slotSlideXLerped;
        slide_x_step = ((s32)(g_slotSlideX - (*slide_x_lerped_ptr))) / ((s32)g_slotSlideFrames);
        slide_y_step = ((s32)(g_slotSlideY - g_slotSlideYLerped)) / ((s32)g_slotSlideFrames);
        g_slotSlideFrames -= 1;
        g_slotSlideXLerped += slide_x_step;
        g_slotSlideYLerped += slide_y_step;
        return;
    }
    g_slotSlideXLerped = g_slotSlideX;
    g_slotSlideYLerped = g_slotSlideY;
    update_menu_input();
    if (g_slotSlideX == 0)
    {
        if (g_debouncedInput & (PAD_BTN_LEFT | PAD_BTN_RIGHT))
        {
            SaveLayoutEntry* entry;
            play_title_sfx(0x7D, 0x80);
            entry = ((SaveLayoutEntry*)g_saveLayoutTable);
            if (entry[18].type != 0)
            {
                entry[18].type = 0;
                entry[19].type = 1;
                return;
            }
            entry[18].type = 1;
            entry[19].type = g_slotSlideYLerped * 0;
            return;
        }
        if (g_debouncedInput & (PAD_BTN_START | PAD_BTN_L3 | PAD_BTN_CROSS))
        {
            play_title_sfx(0x7E, 0x80);
            if (D_800F9AED != 0)
            {
                scroll_slots_right();
                reset_save_slot_panel();
                return;
            }
            scroll_slots_left();
            reset_save_slot_panel();
            return;
        }
        if (g_debouncedInput & PAD_BTN_CIRCLE)
        {
            play_title_sfx(0x7F, 0x80);
            g_titleMenuExitState = 2;
        }
    }
    else
    {
        if (g_debouncedInput & (PAD_BTN_START | PAD_BTN_L3 | PAD_BTN_CROSS))
        {
            if (g_slotSlideX > 0)
            {
                load_sub_menu_layout(0);
                flag_mask = ~0x7F;
                layout = (MenuLayout*)g_menuLayoutBuffer;
                new_flags = (layout->slot_flags) & flag_mask;
            }
            else
            {
                load_sub_menu_layout(1);
                flag_mask = ~0x7F;
                layout = (MenuLayout*)g_menuLayoutBuffer;
                new_flags = ((layout->slot_flags) & flag_mask) | 1;
            }
            layout->slot_flags = new_flags;
            rng_lo = rand();
            rng_hi = rand();
            rng_lo |= rng_hi << 0xF;
            /* Cast store (not layout->rng_seed): a struct-member store is
             * marked MEM_IN_STRUCT, which lets the gcc 2.7.2 scheduler hoist
             * the copy-loop setup loads above it; the cast form keeps the
             * store in place, matching the original order. */
            *(s16*)((u8*)layout + 0xD4) = (s16)rng_lo;
            /* The do/while(0) wrapper is required to match: its loop notes
             * end the scheduling region after the store above, preventing
             * the address setup below from being scheduled before it. */
            do
            {
                dest_ptr = D_80043618;
                src_ptr = g_saveSlotData + (g_slotSelectedIndex << 6);
                copy_count = 0;
                while (copy_count < 0x40U)
                {
                    copy_count += 1;
                    byte = *src_ptr;
                    src_ptr += 1;
                    *dest_ptr = byte;
                    dest_ptr += 1;
                }

                slot_idx = 0;
                selected_slot = g_slotSelectedIndex;
                src_ptr = g_menuLayoutBuffer;
                slot_idx = 0;
                do
                {
                    if (selected_slot != slot_idx)
                    {
                        *((s32*)(src_ptr + 0x34)) = 0;
                    }
                    slot_idx += 1;
                    src_ptr += 4;
                } while (slot_idx < 0xB);
                play_title_sfx(0x7E, 0x80);
            } while (0);
            g_titleMenuExitState = 1;
        }
        else if (g_debouncedInput & PAD_BTN_CIRCLE)
        {
            play_title_sfx(0x7F, 0x80);
            if (g_slotSlideX > 0)
            {
                scroll_slots_left();
                reset_save_slot_panel();
            }
            else
            {
                scroll_slots_right();
                reset_save_slot_panel();
            }
        }
        else if (g_slotHighlightFrames == 0)
        {
            if ((g_debouncedInput & PAD_BTN_UP) != 0U)
            {
                play_title_sfx(0x7D, 0x80);
                prev_index = g_slotSelectedIndex - 1;
                g_slotSelectedIndex = prev_index;
                if (prev_index < 0)
                {
                    g_slotSelectedIndex = 0xA;
                }
            }
            if (g_debouncedInput & PAD_BTN_DOWN)
            {
                play_title_sfx(0x7D, 0x80);
                next_index = g_slotSelectedIndex + 1;
                g_slotSelectedIndex = next_index;
                if (next_index >= 0xB)
                {
                    g_slotSelectedIndex = 0;
                }
            }
        }
        AnimateSaveSlotPanel();
    }
}

/**
 * Lerps g_slotHighlightX toward g_slotHighlightTargetX over
 * g_slotHighlightFrames frames, pans the scroll window so the selected
 * slot is always visible, then writes the updated V-coordinate and
 * visibility flags for the highlight-bar layout entries in g_saveLayoutTable.
 *
 * decomp.me (100%) https://decomp.me/scratch/d3s3Q
 */
void AnimateSaveSlotPanel(void)
{
    u8* layout;
    s16 scroll_width;
    s32* new_var2;
    s32 target_adjusted;
    s32 scroll_offset;
    s32 new_var;
    SaveLayoutEntry* ptr;
    if (g_slotHighlightFrames != 0)
    {
        g_slotHighlightX += (g_slotHighlightTargetX - g_slotHighlightX) / g_slotHighlightFrames;
        g_slotHighlightFrames -= 1;
    }
    else
    {
        g_slotHighlightX = g_slotHighlightTargetX;
    }
    new_var = g_slotHighlightTargetX;
    target_adjusted = new_var;
    if (new_var < 0)
    {
        target_adjusted = new_var + 0xF;
    }
    target_adjusted >>= 4;
    scroll_offset = *(new_var2 = &g_slotSelectedIndex);
    if (g_slotSelectedIndex < target_adjusted)
    {
        g_slotHighlightTargetX = scroll_offset * 0x10;
        g_slotHighlightFrames = 4;
    }
    else if ((target_adjusted + 6) < (*new_var2))
    {
        g_slotHighlightTargetX = (g_slotSelectedIndex - 6) * 0x10;
        g_slotHighlightFrames = 4;
    }
    ptr = (SaveLayoutEntry*)g_saveLayoutTable;
    (ptr + 2)->v0 = (u16)g_slotHighlightX;
    ptr[3].v0 = ((u16)g_slotHighlightX) + 0x20;
    ptr[9].v0 = (u16)g_slotHighlightX;
    ptr[10].v0 = ((u16)g_slotHighlightX) + 0x20;
    if (g_slotHighlightX != 0)
    {
        SaveLayoutEntry* ptr4 = (SaveLayoutEntry*)g_saveLayoutTable;
        ptr4[7].type = 1;
        ptr4[8].type = 1;
        ptr4[14].type = 1;
        ptr4[15].type = 1;
    }
    else
    {
        SaveLayoutEntry* ptr5 = (SaveLayoutEntry*)g_saveLayoutTable;
        ptr5[7].type = 0;
        ptr5[8].type = 0;
        ptr5[14].type = 0;
        ptr5[15].type = 0;
    }
    if (g_slotHighlightX != 0x40)
    {
        SaveLayoutEntry* ptr3 = (SaveLayoutEntry*)g_saveLayoutTable;
        ptr3[4].type = 1;
        ptr3[5].type = 1;
        ptr3[11].type = 1;
        ptr3[12].type = 1;
    }
    else
    {
        SaveLayoutEntry* ptr2 = (SaveLayoutEntry*)g_saveLayoutTable;
        ptr2[4].type = 0;
        ptr2[5].type = 0;
        ptr2[11].type = 0;
        ptr2[12].type = 0;
    }
    scroll_offset = (g_slotSelectedIndex * 0x10) - (new_var = g_slotHighlightX);
    if (scroll_offset < 0)
    {
        scroll_offset = 0;
    }
    if (scroll_offset > 0x60)
    {
        scroll_offset = 0x60;
    }
    layout = g_saveLayoutTable;
    scroll_width = scroll_offset + 0x40;
    *((u16*)(layout + 0x96)) = scroll_width;
    *((u16*)(layout + 0x9A)) = scroll_width;
    *((u16*)(layout + 0x13E)) = scroll_width;
    *((u16*)(layout + 0x142)) = scroll_width;
}

/**
 * @brief Snap the save-slot panel back to its home position and clear its
 *        highlight/selection state.
 *
 * @details When a horizontal slide is in progress (g_slotSlideX != 0) this
 * rebuilds the panel's layout entries in g_saveLayoutTable: it re-homes the
 * scroll window (entries 6 and 13 reset to SAVE_SCROLL_WIDTH_HOME), shows the
 * right highlight halves (entries 4/5/11/12) while hiding the left halves
 * (entries 7/8/14/15), and rewrites the highlight-bar V coordinates (entries
 * 2/3/9/10) from the now-zeroed g_slotHighlightX. The selection/highlight
 * globals are all cleared. When no slide is active it only resets entry 0's
 * U/V to their home values.
 *
 * @note When a slide is active the entries are reached through a
 *       @ref SaveLayoutEntry pointer, matching AnimateSaveSlotPanel. The
 *       inactive-slide branch indexes the table by g_slotSlideX (always 0
 *       here) added to its base, so it stays raw pointer arithmetic.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/0YgmZ
 */
void reset_save_slot_panel(void)
{
    s16 highlight_bottom_v;
    if (g_slotSlideX != 0)
    {
        SaveLayoutEntry* entry = (SaveLayoutEntry*)g_saveLayoutTable;
        entry[0].v0 = SAVE_SLOT_HOME_V;
        g_slotSelectedIndex = 0;
        g_slotHighlightX = 0;
        g_slotHighlightTargetX = 0;
        g_slotHighlightFrames = 0;
        entry[7].type = 0;
        entry[8].type = 0;
        entry[14].type = 0;
        entry[15].type = 0;
        highlight_bottom_v = ((u16)g_slotHighlightX) + SAVE_HIGHLIGHT_SPAN;
        entry[6].y = SAVE_SCROLL_WIDTH_HOME;
        entry[6].tile_y = SAVE_SCROLL_WIDTH_HOME;
        entry[13].y = SAVE_SCROLL_WIDTH_HOME;
        entry[13].tile_y = SAVE_SCROLL_WIDTH_HOME;
        entry[0].u0 = 0;
        entry[4].type = 1;
        entry[5].type = 1;
        entry[11].type = 1;
        entry[12].type = 1;

        entry[2].v0 = (u16)g_slotHighlightX;
        entry[3].v0 = highlight_bottom_v;
        entry[9].v0 = (u16)g_slotHighlightX;
        entry[10].v0 = highlight_bottom_v;
        return;
    }
    {
        u32 low_addr = (u32)(&g_saveLayoutTable);
        u32 ptr = g_slotSlideX + low_addr;
        *((u16*)(ptr + 0xC)) = SAVE_SLOT_HOME_V;
        *((u16*)(ptr + 0xE)) = 0;
    }
}

/**
 * @brief Begins a slide of the save-slot stage one panel to the right.
 *
 * @details Sets the slide target to +SLOT_PANEL_WIDTH and seeds the lerper
 * with SLOT_SLIDE_FRAMES frames of remaining travel. If the lerper is
 * already showing the right-hand panel (g_slotSlideXLerped == SLOT_PANEL_WIDTH),
 * the call is a no-op so we don't accumulate further offset off the edge.
 *
 * @param void No parameters.
 * @return void No return value.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/SRP9z
 */
static void scroll_slots_right(void)
{
    if (g_slotSlideXLerped != SLOT_PANEL_WIDTH)
    {
        g_slotSlideX += SLOT_PANEL_WIDTH;
        g_slotSlideFrames = SLOT_SLIDE_FRAMES;
    }
}

/**
 * @brief Begins a slide of the save-slot stage one panel to the left.
 *
 * @details Mirror of scroll_slots_right: nudges the slide target by
 * -SLOT_PANEL_WIDTH and re-arms the lerper with SLOT_SLIDE_FRAMES of
 * travel. No-ops when the lerper is already at the left-hand limit so
 * the offset cannot run away off-stage.
 *
 * @param void No parameters.
 * @return void No return value.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/W1iA5
 */
static void scroll_slots_left(void)
{
    if (g_slotSlideXLerped != -SLOT_PANEL_WIDTH)
    {
        g_slotSlideX -= SLOT_PANEL_WIDTH;
        g_slotSlideFrames = SLOT_SLIDE_FRAMES;
    }
}

/**
 * @brief One UV/size descriptor in g_slotPolyUvTable / g_slotSprtUvTable.
 *
 * @note Every field is in 8-pixel units; the renderer multiplies by 8 on use.
 */
typedef struct
{
    u8 u;  /**< +0x00: source U, in 8-pixel units */
    u8 v;  /**< +0x01: source V, in 8-pixel units */
    u8 w;  /**< +0x02: width, in 8-pixel units */
    u8 h;  /**< +0x03: height, in 8-pixel units */
    u8 ox; /**< +0x04: X origin offset, in 8-pixel units */
    u8 oy; /**< +0x05: Y origin offset, in 8-pixel units */
} SlotUvRect;                  /* sizeof == 6 */

/*
 * getTPage() in Psy-Q's operand order (tpage mode first, then abr, then the
 * VRAM origin), with the abr term taken straight from the entry's flags word:
 * bits 2-3 of the flags are the abr, so `(flags << 3) & 0x60` places them
 * without the separate shift-and-mask getTPage's `(abr & 3) << 5` would need.
 *   _flags: entry flags word (bits 2-3 are abr)
 *   _tp:    texture-page mode (already masked to 2 bits)
 *   _x,_y:  texture page origin in VRAM
 */
#define TPAGE_WORD(_flags, _tp, _x, _y)                                            ((((_tp) & 3) << 7) | ((((u32)(_flags)) << 3) & 0x60) |                         (((_y) & 0x100) >> 4) | (((_x) & 0x3FF) >> 6) | (((_y) & 0x200) << 2))

/** Number of entries in g_saveLayoutTable. */
#define SAVE_LAYOUT_ENTRIES 0x1B

/** A glyph strip wider than this is split into chunks of this many pixels. */
#define GLYPH_CHUNK_WIDTH 0x80

/**
 * @brief Write the SPRT tag word: length 4 plus the primitive code.
 *
 * @param p    Sprite primitive being filled in.
 * @param code GPU primitive code byte (0x64, or 0x66 for semi-transparent).
 *
 * @note Required to match, in the type-4 arm. Passing @p code as a PARAMETER
 *       gives the constant its own pseudo that is set before the inlined body,
 *       stretching its movable lifetime from 1 to 3; loop.c's move_movables
 *       only hoists when threshold * savings * lifetime >= insn_count
 *       (50 * savings * lifetime >= 479 here). The glyph chunk loop must use
 *       plain @c setSprt instead: at lifetime 1 its constant is too small to
 *       be hoisted out of the INNER loop (50 < 108) and so survives to be
 *       merged with this one by combine_movables, which sums both lifetimes
 *       and savings (2 * 7 = 14) and clears the outer gate.
 */
static inline void set_sprt_tag(SPRT* p, s32 code)
{
    setlen(p, 4);
    setcode(p, code);
}

/**
 * @brief Emit the GPU primitives for every entry of the save-slot layout table.
 *
 * @details Walks the SAVE_LAYOUT_ENTRIES entries of g_saveLayoutTable and emits
 * one primitive shape per entry, selected by SaveLayoutEntry::type:
 *   - 3: a POLY_FT4 quad (the slot panel background), UV rect from
 *        D_800F98AC, indexed by g_slotSelectedIndex while sliding right.
 *   - 4: a SPRT plus its DR_TPAGE, UV rect from D_800F98F4, indexed by
 *        g_slotSelectedIndex while sliding left.
 *   - 2: a solid TILE plus its DR_TPAGE (the dimmed backdrop).
 *   - any other non-zero value: a glyph strip, emitted as one SPRT plus
 *        DR_TPAGE per GLYPH_CHUNK_WIDTH pixels of SaveLayoutEntry::width.
 *   - 0: nothing.
 *
 * SaveLayoutEntry::flags bit0 selects whether the horizontal/vertical slide
 * lerp is applied to the glyph strip's position, bit1 selects the
 * semi-transparent code byte, and bits2-3 are the tpage abr (blend) mode.
 * CLUT and tpage words come from g_saveLayoutTexTable, indexed by
 * SaveLayoutEntry::tex_slot.
 *
 * @param ptr Pointer to the next free byte in the primitive buffer.
 * @param ot  Pointer to the OT head tag that each primitive is linked onto
 *            via addPrim.
 * @return Pointer to the byte just past the last primitive emitted.
 *
 * @note WORK IN PROGRESS - 99.77% (454/474 exact rows), 474/474 instructions,
 *       NO structural rows: every instruction is in the right place and the
 *       frame, prologue and sp-slot traffic all match. The whole residue is
 *       one 4-way saved-register rotation - @c sprt_code takes s4 where the
 *       target has s1, pushing @c i, the YLerped base and the XLerped base
 *       each down one slot.
 *
 * @note TODO - what that rotation needs is +4 weighted REG_N_REFS on the
 *       hoisted 0x64 pseudo, at zero instruction cost. gcc 2.7.2 global.c
 *       `allocno_compare` sorts by floor_log2(refs)*refs/live_length, and
 *       `reg_n_refs` is weighted by loop depth (flow.c:2067). Hoisting the
 *       constant moves its SET from depth 2 to depth 1 and stretches
 *       live_length 284 -> 852, dropping its priority 492 -> 140 so it loses
 *       s1 to @c i (7 refs / 435 live, pri 321). Measured: at 10 weighted refs
 *       the target's saved-register set falls out exactly - s0=texTable,
 *       s1=0x64, s2=i, s3=vy, s4=vx, s5=tile_len. Two genuine store sites give
 *       6 (1 + 2 + 3), and the extra references cannot be faked: a duplicate
 *       store to the same address and a `sprt_code | 2` for the 0x66 case are
 *       both folded away before the .flow pass. @c vx and @c vy cannot be
 *       demoted below the hoisted constant instead - at 9 refs they would need
 *       a live_length longer than the whole function. So this is a
 *       source-model question about where the original references that byte,
 *       not a tuning knob. Reverting to a shared `s32 sprt_code = 0x64;` in
 *       the else arm gives up the hoist but scores 471/474 exact; that variant
 *       is kept as working/RenderSaveLayoutPrims/code.c.
 *
 * @note Shapes below are measured-required; the number is what reverting costs.
 *       @c idx is ONE variable serving the type-3 and type-4 arms' table index
 *       AND the glyph strip's texture X (+8) - the target colours all three
 *       sites a3, so they are three uses of one scratch, not three variables.
 *       @c tint is likewise ONE variable for the 0x808080 colour word shared by
 *       the type-3 and type-4 arms (+3, and it must be a variable: written as a
 *       literal, loop.c combines the two arms' constants and hoists them into a
 *       SEVENTH saved register, growing the frame to -0x20). Do not extend
 *       @c tint into the glyph loop as well (-101).
 *
 * @note @p e is the ONLY table cursor; the target's entry+2 pointer is a giv
 *       gcc derives from it. In the glyph arm the apply-slide flag must be an
 *       unnamed temp (`if (*(u32*)e & 1)`) and the preheader's texture pointer
 *       its own local (@c tex0), separate from the in-loop @c tex (+7 together):
 *       as a single variable @c tex is a global allocno and takes v0 ahead of
 *       the flags temp, which local alloc otherwise orders as the target does.
 *       @c setSemiTrans is correct ONLY in the type-3 arm; the type-4 and glyph
 *       arms are a bare `if (flags & 2) setcode(..., 0x66)`.
 *
 * @note The origin offset must be its own statement (@c offx / @c offy) and the
 *       sum (@c vx / @c vy) must be assigned BEFORE it. Inline, fold-const's
 *       split_tree (fold-const.c:3759) rewrites `A - (B - C)` into `(A + C) - B`
 *       and no spelling avoids it. @c vx and @c vy are u16 so gcc narrows the
 *       s32 lerp globals to `lhu`; @c tpw is u32 so it does NOT narrow the
 *       type-3 flags load.
 */
void* RenderSaveLayoutPrims(u8* ptr, u_long* ot)
{
    u8* e = (u8*)g_saveLayoutTable;
    s32 i = 0;
    s32 tile_len = 3;
    s32 idx;
    u32 tint;

    do
    {
        s32 type = e[1];

        if (type == tile_len)
        {
            /* Slot panel background: a single textured quad. */
            POLY_FT4* poly;
            u16 vx;
            s32 offx;
            u16 vy;
            s32 offy;
            SlotUvRect* uv;
            u8* tex;
            u8* tex2;
            u32 tpw;

            if (g_slotSlideX > 0)
            {
                idx = g_slotSelectedIndex + 1;
            }
            else
            {
                idx = 0;
            }

            /* (offset) + (base) so gcc emits `addu v1,v1,v0`, not the reverse. */
            uv = (SlotUvRect*)((idx * 6) + (u32)D_800F98AC);

            /* Scheduling barrier, required to match: gcc 2.7.2 gives an insn
               carrying loop notes a dependence on every prior register use and
               set (sched.c:2058), which pins the uv address computation ahead
               of the colour-word stores. Reads like a debug macro that expands
               to nothing in the retail build. */
            do { } while (0);

            poly = (POLY_FT4*)ptr;
            tint = GPU_TINT_NEUTRAL;
            *(u32*)(ptr + 4) = tint;
            setPolyFT4(poly);

            setSemiTrans(poly, *(u32*)e & 2);

            /* Chained assignment: the value propagated to x2/y1 is the *short*
               result of the store, so gcc sinks the 16-bit truncation into the
               add and loads the lerp global with lhu rather than lw. */
            vx = *(u16*)(e + 4) + g_slotSlideXLerped;
            offx = uv->ox * 8 - 0x20;
            poly->x2 = poly->x0 = vx - offx;
            vy = *(u16*)(e + 6) + g_slotSlideYLerped;
            offy = uv->oy * 8 - 0x28;
            poly->y1 = poly->y0 = vy - offy;

            poly->x1 = poly->x3 = (poly->x0 + (uv->w * 8)) - 1;
            poly->y2 = poly->y3 = (poly->y0 + (uv->h * 8)) - 1;

            poly->u3 = poly->u1 = uv->u * 8;
            poly->v1 = poly->v0 = uv->v * 8;

            poly->u0 = poly->u2 = (poly->u1 + (uv->w * 8)) - 1;
            poly->v2 = poly->v3 = (poly->v0 + (uv->h * 8)) - 1;

            ptr = (u8*)poly + sizeof(POLY_FT4);

            tex = (u8*)((e[2] * 0x10) + (u32)g_saveLayoutTexTable);
            poly->clut = getClut(*(u16*)(tex + 4), *(u16*)(tex + 6));

            tex2 = (u8*)((e[2] * 0x10) + (u32)g_saveLayoutTexTable);
            /* u32 temp: assigning the word straight into the 16-bit tpage field
               lets gcc narrow the flags load to lhu; the target keeps lw. */
            tpw = TPAGE_WORD(*(u32*)e, tex2[0x0C] & 3, *(u16*)tex2, *(u16*)(tex2 + 2));
            poly->tpage = tpw;

            addPrim(ot, poly);
        }
        else
        {
            if (type == 4)
            {
                /* Slot cursor / decoration: one free-size sprite. */
                SlotUvRect* uv;
                u16 vx;
                s32 offx;
                u16 vy;
                s32 offy;
                u8* tex;
                u8* tex2;
                SPRT* sprt;
                DR_TPAGE* tp;
                s32 sprt_code;

                if (g_slotSlideX < 0)
                {
                    idx = g_slotSelectedIndex + 1;
                }
                else
                {
                    idx = 0;
                }

                sprt = (SPRT*)ptr;
                sprt_code = 0x64;
                tint = GPU_TINT_NEUTRAL;
                *(u32*)(ptr + 4) = tint;
                set_sprt_tag(sprt, sprt_code);

                uv = (SlotUvRect*)((idx * 6) + (u32)D_800F98F4);

                if (*(u32*)e & 2)
                {
                    setcode(sprt, 0x66);
                }

                vx = *(u16*)(e + 4) + g_slotSlideXLerped;
                offx = uv->ox * 8 - 0x20;
                sprt->x0 = vx - offx;
                vy = *(u16*)(e + 6) + g_slotSlideYLerped;
                offy = uv->oy * 8 - 0x28;
                sprt->y0 = vy - offy;
                sprt->u0 = uv->u * 8;
                sprt->v0 = uv->v * 8;
                sprt->w = uv->w * 8;
                sprt->h = uv->h * 8;

                tex = (u8*)((e[2] * 0x10) + (u32)g_saveLayoutTexTable);
                sprt->clut = getClut(*(u16*)(tex + 4), *(u16*)(tex + 6));

                addPrim(ot, sprt);
                ptr += sizeof(SPRT);

                tp = (DR_TPAGE*)ptr;
                setlen(tp, 1);

                tex2 = (u8*)((e[2] * 0x10) + (u32)g_saveLayoutTexTable);
                tp->code[0] = TPAGE_WORD(*(u32*)e, *(u32*)(tex2 + 0x0C) & 3, *(u16*)tex2, *(u16*)(tex2 + 2)) | 0xE1000000;

                addPrim(ot, tp);
                ptr += sizeof(DR_TPAGE);
            }
            else
            {
                if (type == 2)
                {
                    /* Dimmed backdrop behind the slot list: one solid tile. */
                    TILE* tile = (TILE*)ptr;
                    DR_TPAGE* tp;

                    *(u32*)(ptr + 4) = 0x40; /* solid dark-blue fill; code byte set below */
                    setlen(tile, tile_len);
                    setcode(tile, 0x62);

                    tile->x0 = *(u16*)(e + 8) + g_slotSlideXLerped;
                    tile->y0 = *(u16*)(e + 10) + g_slotSlideYLerped;
                    tile->w = *(u16*)(e + 16);
                    tile->h = *(u16*)(e + 18);

                    addPrim(ot, tile);

                    tp = (DR_TPAGE*)(ptr + sizeof(TILE));
                    setlen(tp, 1);
                    tp->code[0] = 0xE1000025;
                    addPrim(ot, tp);

                    ptr += sizeof(TILE) + sizeof(DR_TPAGE);
                }
                else if (type != 0)
                {
                    /* Glyph strip: one SPRT + DR_TPAGE per GLYPH_CHUNK_WIDTH pixels. */
                    s32 remaining = *(u16*)(e + 16);
                    u16 u0 = *(u16*)(e + 12);
                    u8* tex;
                    u8* tex0 = (u8*)((e[2] * 0x10) + (u32)g_saveLayoutTexTable);
                    s32 x;
                    s32 y;
                    s32 chunk;
                    u8* tex2;

                    idx = *(u16*)tex0;

                    if (*(u32*)e & 1)
                    {
                        x = *(s16*)(e + 4) + g_slotSlideXLerped;
                        y = *(s16*)(e + 6) + g_slotSlideYLerped;
                    }
                    else
                    {
                        x = *(s16*)(e + 4);
                        y = *(s16*)(e + 6);
                    }

                    chunk = GLYPH_CHUNK_WIDTH;
                    if (remaining < GLYPH_CHUNK_WIDTH + 1)
                    {
                        chunk = remaining;
                    }

                    while (1)
                    {
                        SPRT* sprt = (SPRT*)ptr;
                        DR_TPAGE* tp;

                        SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
                        setSprt(sprt);

                        if (*(u32*)e & 2)
                        {
                            setcode(sprt, 0x66);
                        }

                        sprt->x0 = x;
                        sprt->y0 = y;
                        sprt->u0 = u0;
                        sprt->v0 = e[14];
                        sprt->w = chunk;
                        sprt->h = *(u16*)(e + 18);

                        remaining -= chunk;

                        tex = (u8*)((e[2] * 0x10) + (u32)g_saveLayoutTexTable);
                        sprt->clut = getClut(*(u16*)(tex + 4), *(u16*)(tex + 6));

                        addPrim(ot, ptr);
                        ptr += sizeof(SPRT);

                        tp = (DR_TPAGE*)ptr;
                        setlen(tp, 1);

                        tex2 = (u8*)((e[2] * 0x10) + (u32)g_saveLayoutTexTable);
                        tp->code[0] = TPAGE_WORD(*(u32*)e, *(u32*)(tex2 + 0x0C) & 3, idx, *(u16*)(tex2 + 2)) | 0xE1000000;

                        addPrim(ot, ptr);
                        ptr += sizeof(DR_TPAGE);

                        if (remaining == 0)
                        {
                            break;
                        }

                        /* Next chunk: flip to the other half of the texture page, or
                           step the page origin on when the mode bits say the strip
                           spans pages. */
                        u0 ^= 0x80;
                        tex = (u8*)((e[2] * 0x10) + (u32)g_saveLayoutTexTable);

                        if (!(*(u32*)(tex + 0x0C) & 7))
                        {
                            idx += 0x20;
                        }
                        else
                        {
                            idx += 0x40;
                            u0 = 0;
                        }

                        chunk = GLYPH_CHUNK_WIDTH;
                        if (remaining < GLYPH_CHUNK_WIDTH + 1)
                        {
                            chunk = remaining;
                        }

                        x += GLYPH_CHUNK_WIDTH;
                    }
                }
            }
        }

        i++;
        e += sizeof(SaveLayoutEntry);
    } while (i < SAVE_LAYOUT_ENTRIES);

    return ptr;
}

/**
 * @brief Upload every g_saveLayoutTexTable entry's CLUT and pixel data to VRAM.
 *
 * @details For each of the 11 entries in g_saveLayoutTexTable (stride 0x10),
 * reads an on-disk-TIM-style source blob via the entry's data pointer
 * (+0x8), uploads its CLUT block and then its pixel block with LoadImage,
 * and bit-packs the uploaded image's dimensions back into the entry's control
 * word (SaveLayoutTex::control). The destination entry is typed as
 * SaveLayoutTex; the source blob's internal TIM-style block layout is only
 * partially understood, so it is still walked with raw byte offsets.
 *
 * @return Not explicitly set on any path (matches original codegen); callers
 *         should not rely on the return value.
 *
 * @note @p data_ptr and @p block_ptr are kept as two distinct pointers that
 *       both start out holding the source blob: @p data_ptr is the working
 *       cursor (advanced past the header, used for the CLUT upload) while
 *       @p block_ptr is reused to point at the pixel block. Merging them into a
 *       single variable changes gcc 2.7's register allocation and drops the
 *       match, so the pair is required to match.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/lzJHa
 */
/* Bit layout of SaveLayoutTex::control: bits 0-2 = TIM pixel mode, bits 3-12 =
 * texture width, bits 13-22 = texture height (widths/heights are 10-bit). */
#define SAVE_TEX_MODE_MASK    0x7
#define SAVE_TEX_WIDTH_SHIFT  3
#define SAVE_TEX_HEIGHT_SHIFT 13
#define SAVE_TEX_DIM_MASK     0x3ff
unsigned short upload_save_layout_textures(void)
{
    SaveLayoutTex* tex_entry;
    u32 orig_control;
    SaveLayoutTex* entry_ctrl_ptr;
    u8* data_ptr;
    u32 pixel_block_offset;
    u8* block_ptr;
    int clut_pixels;
    int shift;
    u32 reload_control;
    u32 control;
    RECT rect;
    int counter;
    u8* clut_h_ptr;
    tex_entry = (SaveLayoutTex*)g_saveLayoutTexTable;

    for (counter = 0; counter < 11; counter++)
    {
        entry_ctrl_ptr = tex_entry;
        block_ptr = entry_ctrl_ptr->src;
        orig_control = entry_ctrl_ptr->control;
        control = orig_control;
        data_ptr = block_ptr;
        clut_h_ptr = data_ptr + 0x12;
        control = (control & ~SAVE_TEX_MODE_MASK) | (data_ptr[4] & SAVE_TEX_MODE_MASK);
        entry_ctrl_ptr->control = control;
        clut_pixels = (*((u16*)(data_ptr + 0x10))) * (*((u16*)clut_h_ptr));
        pixel_block_offset = *((u32*)(data_ptr + 8));
        data_ptr += 8;
        rect.x = tex_entry->clut_x;
        clut_pixels++;
        clut_pixels--;
        rect.y = tex_entry->clut_y;
        rect.h = 1;
        rect.w = clut_pixels;

        LoadImage(&rect, (u_long*)(data_ptr + 0xc), clut_pixels);
        block_ptr = data_ptr + pixel_block_offset;
        shift = SAVE_TEX_WIDTH_SHIFT;
        control = (reload_control = entry_ctrl_ptr->control);
        control = (control & ~(SAVE_TEX_DIM_MASK << SAVE_TEX_WIDTH_SHIFT)) |
                  (((*((u16*)(block_ptr + 8))) & SAVE_TEX_DIM_MASK) << shift);
        tex_entry->control = control;
        control = control & ~(SAVE_TEX_DIM_MASK << SAVE_TEX_HEIGHT_SHIFT);
        control = control | (((*((u16*)(block_ptr + 0xa))) & SAVE_TEX_DIM_MASK) << SAVE_TEX_HEIGHT_SHIFT);
        tex_entry->control = control;
        setRECT(&rect,
                tex_entry->tex_x,
                tex_entry->tex_y,
                (tex_entry->control >> SAVE_TEX_WIDTH_SHIFT) & SAVE_TEX_DIM_MASK,
                (tex_entry->control >> SAVE_TEX_HEIGHT_SHIFT) & SAVE_TEX_DIM_MASK);
        LoadImage(&rect, (u_long*)(block_ptr + 0xc));
        tex_entry++;
        entry_ctrl_ptr++;
    }
}

/**
 * @brief Load one of the two full menu-layout templates into g_menuLayoutBuffer.
 *
 * Copies a MENU_LAYOUT_WORDS-word (~13 KB) MenuLayout template over the working
 * g_menuLayoutBuffer and sets the companion mode field g_scene_mode.
 *
 * @param use_alt Zero selects the default template (g_menuLayoutTemplateDefault,
 *                g_scene_mode = 0xD); non-zero selects the alternate template
 *                (g_menuLayoutTemplateAlt, g_scene_mode = 0).
 *
 * @note The copy is an explicit word loop, not a struct assignment, so it
 *       reproduces the original codegen; MenuLayout is only partially mapped.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/aPcbW
 */
void load_menu_layout(s32 use_alt)
{
    s32* src;
    s32* dst;
    u32 i;
    if (use_alt == 0)
    {
        src = (s32*)&g_menuLayoutTemplateDefault;
        g_scene_mode = 0xD;
    }
    else
    {
        src = (s32*)&g_menuLayoutTemplateAlt;
        g_scene_mode = 0;
    }
    g_music_track_index = 0;
    g_layout_flag = 0;

    do
    {
    } while (0);

    i = 0;
    dst = (s32*)g_menuLayoutBuffer;

    while (i < MENU_LAYOUT_WORDS)
    {
        *dst++ = *src++;
        i++;
    }
}

/**
 * @brief Load one of the two sub-menu layout tables for the save-slot screen.
 *
 * Copies a SUB_MENU_LAYOUT_WORDS-word (0x250-byte) layout table into the
 * game-data buffer at g_gameDataBasePtr. The default table is used when
 * starting a new game; the continue table is used when resuming a saved game,
 * in which case bit 0 of MenuLayout::mode_flags is also set to flag the slot
 * as "continue mode".
 *
 * @param is_continue Zero selects the default layout (g_subMenuLayoutDefault);
 *                     non-zero selects the continue layout
 *                     (g_subMenuLayoutContinue) and sets the continue-mode bit.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/CU7Ml
 */
void load_sub_menu_layout(s32 is_continue)
{
    s32* src;
    s32* dst;
    u32 i;

    if (is_continue != 0)
    {
        src = g_subMenuLayoutContinue;
        ((MenuLayout*)g_menuLayoutBuffer)->mode_flags |= 1;
    }
    else
    {
        src = g_subMenuLayoutDefault;
    }

    dst = (s32*)&g_gameDataBasePtr;

    for (i = 0; i < SUB_MENU_LAYOUT_WORDS; i++)
    {
        dst[i] = src[i];
    }
}
