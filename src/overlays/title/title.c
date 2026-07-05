#include "title.h"
#include "cd_resources.h"
#include "display.h"

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
 * input (set by HandleTitleMenuInput); dispatching it quits the title back to
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
 *        buffer (returned by func_80015C48 in main.c); forwarded as-is to
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
        scene_state->unk0 = 0;
        scene_state->unk2 = 0;
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
    func_800157DC();
    SetDispMask(1);

    while (1)
    {
        s1 = s0->otag_buffer;
        ClearOTagR(s1, 0x1000);
        s0->next_prim_ptr = s0->prim_buffer;
        rand();
        VSync(1);
        RenderFadeOverlay(s0);
        RenderTitleBackdrop(s0);
        render_title_menu_items(s0);
        HandleTitleMenuInput();

        if (g_titleMenuExitState == 0)
        {
            DrawSync(0);
            func_800157B0(2);
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
            func_800157DC();
            cdrom_process_state();
            if (g_titleMenuExitState == 0)
            {
                continue;
            }
        }
        break;
    }

    func_800158E0();
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
    SetFadeTarget(0x100, 0x100, 0x100, 0x14);
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
    func_800157DC();
    SetDispMask(1);
    do
    {
        ot = current->otag_buffer;
        ClearOTagR(ot, 0x1000);
        current->next_prim_ptr = current->prim_buffer;
        VSync(1);
        RenderFadeOverlay(current);
        RenderSaveSlotMenu(current);
        DrawSync(0);
        func_800157B0(2);
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
        func_800157DC();
        cdrom_process_state();
    } while (g_titleMenuExitState == 0);
    func_800158E0();
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
    SetFadeTarget(0x100, 0x100, 0x100, 0x14);
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
 * Counterpart of CHECKPS func_80050258: interpolates g_fadeCurrent toward
 * g_fadeTarget and emits the fade-overlay primitive into the active prim
 * buffer.
 *
 * decomp.me (100%) https://decomp.me/scratch/fBro2
 */
void RenderFadeOverlay(MenuContext* arg0)
{
    MenuContext* arg = arg0;
    u32* var_t4 = (u32*)arg->next_prim_ptr;
    u32* unk40_ptr = (u32*)(((u8*)arg) + 0x40);
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
            ((u8*)var_t4)[4] = ((u8)g_fadeCurrent.red) - 1;
            ((u8*)var_t4)[5] = ((u8)g_fadeCurrent.green) - 1;
            ((u8*)var_t4)[6] = ((u8)g_fadeCurrent.blue) - 1;
        }
        else
        {
            if (g_fadeCurrent.red == 0x100)
            {
                ((u8*)var_t4)[4] = 0;
            }
            else
            {
                ((u8*)var_t4)[4] = ~((u8)g_fadeCurrent.red);
            }
            if (g_fadeCurrent.green == 0x100)
            {
                ((u8*)var_t4)[5] = 0;
            }
            else
            {
                ((u8*)var_t4)[5] = ~((u8)g_fadeCurrent.green);
            }
            if (g_fadeCurrent.blue == 0x100)
            {
                ((u8*)var_t4)[6] = 0;
            }
            else
            {
                ((u8*)var_t4)[6] = ~((u8)g_fadeCurrent.blue);
            }
        }
        ((u8*)var_t4)[3] = 3;
        ((u8*)var_t4)[7] = 0x62;
        *((u16*)(((u8*)var_t4) + 12)) = 0x140;
        *((u16*)(((u8*)var_t4) + 10)) = 0;
        *((u16*)(((u8*)var_t4) + 8)) = 0;
        *((u16*)(((u8*)var_t4) + 14)) = 0xF0;
        *var_t4 = ((*var_t4) & 0xFF000000) | ((*unk40_ptr) & 0xFFFFFF);
        *unk40_ptr = ((*unk40_ptr) & 0xFF000000) | (((u32)var_t4) & 0xFFFFFF);
        ;
        var_a1 = 0x25;
        var_t4 = (u32*)(((u8*)var_t4) + 0x10);
        if (((s32)g_fadeCurrent.red) < 0x101)
        {
            var_a1 = 0x45;
        }
        ((u8*)var_t4)[3] = 1;
        *((u32*)(((u8*)var_t4) + 4)) = (s32)(var_a1 | 0xE1000000);
        *var_t4 = ((*var_t4) & 0xFF000000) | ((*unk40_ptr) & 0xFFFFFF);
        *unk40_ptr = ((*unk40_ptr) & 0xFF000000) | (((u32)var_t4) & 0xFFFFFF);
        var_t4 = (u32*)(((u8*)var_t4) + 8);
    }
    arg->next_prim_ptr = (u_long*)var_t4;
}

/**
 * Counterpart of CHECKPS SetFadeTarget.
 *
 * decomp.me (100%) https://decomp.me/scratch/zxqdP
 */
void SetFadeTarget(s32 red, s32 green, s32 blue, s32 steps)
{
    g_fadeTarget.red = red;
    g_fadeTarget.green = green;
    g_fadeTarget.blue = blue;
    g_fadeTarget.steps = steps;
}

/**
 * Emits the title screen's tiled backdrop strip (5 quads stepping 0x40 px).
 *
 * decomp.me (100%) https://decomp.me/scratch/aKAFU
 */
void RenderTitleBackdrop(void* arg0)
{
    u8* base = (u8*)arg0;
    u8* t3;
    u32* a2;
    s32 t0;
    s32 t1;
    s32 a3;
    s32 temp_v0;
    s32 temp_v1;

    a2 = *((u32**)(base + 0x80B8));
    t3 = base + 0x40;
    t0 = 0;

    while (t0 < 5)
    {
        a3 = 0x40 + (t0 << 6);
        *((short*)((((u8*)a2) + 0x0E) + 0x12)) = (short)a3;
        *((short*)((((u8*)a2) + 0x0E) + 2)) = (short)a3;
        t1 = 0x140 + (t0 << 6);
        temp_v1 = t1 & 0x3FF;
        temp_v0 = t0 << 6;
        t0++;
        *((short*)((((u8*)a2) + 0x0E) + 0x0A)) = (short)temp_v0;
        *((short*)((((u8*)a2) + 0x0E) - 6)) = (short)temp_v0;
        (((u8*)a2) + 0x0E)[-0x0B] = 9;
        (((u8*)a2) + 0x0E)[-7] = 0x2C;
        (((u8*)a2) + 0x0E)[-8] = 0x80;
        (((u8*)a2) + 0x0E)[-9] = 0x80;
        (((u8*)a2) + 0x0E)[-0x0A] = 0x80;
        *((short*)((((u8*)a2) + 0x0E) + 4)) = 0;
        *((short*)((((u8*)a2) + 0x0E) - 4)) = 0;
        *((short*)((((u8*)a2) + 0x0E) + 0x14)) = 0xE0;
        *((short*)((((u8*)a2) + 0x0E) + 0x0C)) = 0xE0;
        (((u8*)a2) + 0x0E)[0x0E] = 0;
        (((u8*)a2) + 0x0E)[-2] = 0;
        (((u8*)a2) + 0x0E)[0x16] = 0x40;
        (((u8*)a2) + 0x0E)[6] = 0x40;
        (((u8*)a2) + 0x0E)[7] = 8;
        (((u8*)a2) + 0x0E)[-1] = 8;
        (((u8*)a2) + 0x0E)[0x17] = 0xE8;
        (((u8*)a2) + 0x0E)[0x0F] = 0xE8;
        *((short*)((((u8*)a2) + 0x0E) + 8)) = (short)((temp_v1 >> 6) | 0x110);
        *((short*)((((u8*)a2) + 0x0E) + 0)) = 0x7840;
        /* addPrim((P_TAG *)(t3 + 0x3FFC), a2) */
        ((P_TAG*)a2)->addr = (u_long)(((P_TAG*)(t3 + 0x3FFC))->addr);
        ((P_TAG*)(t3 + 0x3FFC))->addr = (u_long)a2;
        a2 = (u32*)(((u8*)a2) + 0x28);
    }

    *((u32**)(base + 0x80B8)) = a2;
}

/**
 * Per-frame input dispatcher for the main title menu.
 *
 * decomp.me (100%) https://decomp.me/scratch/vmcmD
 */
void HandleTitleMenuInput(void)
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
    if (g_debouncedInput & 0xA20)
    {
        play_title_sfx(op, 0x80);
        g_titleMenuExitState = 1;
        return;
    }
    if (g_debouncedInput & 0x9000)
    {
        menu_cursor_up();
        play_title_sfx(0x7D, 0x80);
    }
    else if (g_debouncedInput & 0x6100)
    {
        MenuCursorDown();
        play_title_sfx(0x7D, 0x80);
    }
}

/**
 * Linear-search D_80102670 forward for the next enabled menu slot. If none
 * remain, the cursor is reset to slot 0 with rank 0.
 *
 * decomp.me (100%) https://decomp.me/scratch/6k8uV
 */
void MenuCursorDown(void)
{
    s32 a1;
    u8* var_v1;
    const s32 LIMIT = 16;
    a1 = g_titleSelectedItem + 1;
    if (a1 < LIMIT)
    {
        u8* base = g_titleMenuItemFlags; // forces lui/addiu first
        var_v1 = base + a1 * 2;          // sll comes after
        while (1)
        {
            if (*var_v1 != 0)
            {
                break;
            }
            a1++;
            if (a1 < LIMIT)
            {
                var_v1 += 2;
                continue;
            }
            break;
        }
    }
    if (a1 == LIMIT)
    {
        g_titleVisibleItemRank = 0;
        g_titleSelectedItem = 0;
    }
    else
    {
        g_titleSelectedItem = (u8)a1;
        g_titleVisibleItemRank++;
    }
}

/**
 * @brief Move the menu cursor to the previous enabled slot, wrapping to the
 *        last enabled slot if already at the top.
 *
 * @details Mirror of MenuCursorDown searching backward. Scans
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
 * slot (same forward scan as MenuCursorDown).
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
 *       buttonData load and a volatile axisY read) rather than the SCDRegs
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
        buttons = ((base->buttonData >> 8) & 0xFF) | (base->buttonData << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);
        if (base->deviceState != 0)
        {
            axis = base->axisX;
            if (axis < (-1))
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }
            axis = base->axisY;
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
    UploadSaveLayoutTextures();
}

/**
 * decomp.me (100%) https://decomp.me/scratch/so5cY
 */
void RenderSaveSlotMenu(MenuContext* arg0)
{
    arg0->next_prim_ptr = (u_long*)RenderSaveLayoutPrims(arg0->next_prim_ptr, (s32*)((char*)arg0 + 0x40));
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

inline u16 inline_fn(unsigned char* arg0)
{
    return *((u16*)arg0);
}

/**
 * Walks the 0x1B-entry × 0x18-byte layout table at g_saveLayoutTable+2 and emits
 * one of four primitive shapes per entry (text quad, simple sprite,
 * solid-coloured POLY_F4, or chunked glyph strip). Returns the new prim
 * head.
 *
 * decomp.me (87.28%) https://decomp.me/scratch/UVLSm
 * (bad definitely not functional scratch 91.76% https://decomp.me/scratch/P2gt5)
 * NOTE THAT THIS MAY NOT BE FUNCTIONALLY EQUIVALENT YET!
 */
void* RenderSaveLayoutPrims(void* arg0, s32* arg1)
{
    unsigned char* t0;
    unsigned char* new_var;
    unsigned int new_var8;
    s32* t6;
    s32 s2;
    s32 s5;
    s32 new_var6;
    unsigned char* new_var4;
    s32 s4;
    s32 s3;
    unsigned char* s0;
    u32 t4;
    u32 t8;
    s32 s1;
    unsigned char* t2;
    int new_var3;
    unsigned char* a3;
    u32* new_var10;
    unsigned char* a1;
    unsigned char* a2;
    unsigned char* new_var7;
    s32 t1;
    s32 t3;
    u16 t5;
    unsigned char new_var2;
    u16 a3_16;
    s32 a1_16;
    s32 t9_16;
    int new_var5;
    u8 v0_8;
    int new_var9;
    u16 a2_16;
    unsigned long temp_ul;
    s32 v0;
    s32 v1;
    t0 = arg0;
    t6 = arg1;
    t2 = g_saveLayoutTable + 2;
    s2 = 0;
    s5 = 3;
    s4 = &g_slotSlideXLerped;
    s3 = (s32)(&g_slotSlideYLerped);
    s0 = g_saveLayoutTexTable;
    t4 = 0x00FFFFFF;
    t8 = 0xFF000000;
    do
    {
        if (1)
        {
            s1 = 0x64;
        }
        v0 = t2[-1];
        if (v0 == s5)
        {
            if (g_slotSlideX > 0)
            {
                a3 = (unsigned char*)(g_slotSelectedIndex + 1);
            }
            else
            {
                a3 = 0;
            }
            a1 = (unsigned char*)(((unsigned long)D_800F98AC) + (((unsigned long)a3) * 6));
            *((u32*)(t0 + 4)) = 0x808080;
            new_var9 = 0;
            t0[3] = 9;
            a3 = t0;
            if (((new_var9, *((u32*)(t2 - 2)))) & 2)
            {
                a3[7] = 0x2E;
            }
            else
            {
                a3[7] = 0x2C;
            }
            v0 = 0x20;
            v0 = (*((u16*)(a3 + 8)) = (inline_fn(t2 + 2) + g_slotSlideXLerped) - ((a1[4] * 8) - v0));
            *((u16*)(a3 + 0x18)) = v0;
            v0 = (*((u16*)(a3 + 10)) = (inline_fn(t2 + 4) + g_slotSlideYLerped) - ((a1[5] * 8) - 0x28));
            *((u16*)(a3 + 0x12)) = v0;
            new_var5 = (inline_fn(a3 + 8) + (a1[2] * 8)) - 1;
            *((u16*)(a3 + 0x20)) = new_var5;
            *((u16*)(a3 + 0x10)) = new_var5;
            *((u16*)(a3 + 0x22)) = (v0 = (inline_fn(a3 + 10) + (a1[3] * 8)) - 1);
            *((u16*)(a3 + 0x1A)) = v0;
            v0_8 = a1[0] * 8;
            a3[0x14] = v0_8;
            a3[0x24] = v0_8;
            v0_8 = (new_var2 = a1[1]) * 8;
            a3[0x0D] = v0_8;
            a3[0x15] = v0_8;
            t3 = a3[0x14];
            v0 = (a3[0x1C] = (t3 + (a1[2] * 8)) - 1);
            a3[0x0C] = v0;
            v0 = (a3[0x25] = (a3[0x0D] + (a1[3] * 8)) - 1);
            a3[0x1D] = v0;
            t0 = a3 + 0x28;
            a3 = (unsigned char*)((t2[0] * 0x10) + ((unsigned long)s0));
            v0 = (inline_fn(a3 + 6) << 6) | ((inline_fn(a3 + 4) >> 4) & 0x3F);
            a2 = t2;
            new_var3 = 6;
            *((u16*)((t0 - 0x28) + 0x0E)) = v0;
            a1 = (unsigned char*)((a2[0] * 0x10) + ((unsigned long)s0));
            a2_16 = inline_fn(a1 + 2);
            v0 = ((((((*((u32*)(t2 - 2))) << 3) & 0x60) | ((a1[0x0C] & 3) << 7)) | ((a2_16 & 0x100) >> 4)) | ((inline_fn(a1) & 0x3FF) >> new_var3)) |
                 ((a2_16 & 0x200) * 4);
            *((u16*)((t0 - 0x28) + 0x16)) = v0;
            ;
            ;
            *((u32*)((t0 + (-0x28)) + 0)) = ((*((u32*)((t0 - 0x28) + 0))) & t8) | ((*t6) & t4);
            v1 = (*t6) & t8;
            v0 = ((unsigned long)(t0 - 0x28)) & t4;
            *t6 = v1 | v0;
        }
        else if (v0 == 4)
        {
            if (g_slotSlideX < 0)
            {
                a3 = (unsigned char*)(g_slotSelectedIndex + 1);
            }
            else
            {
                a3 = 0;
            }
            *((u32*)(t0 + 4)) = 0x808080;
            t0[3] = 4;
            t0[7] = s1;
            a1 = (unsigned char*)(((unsigned long)D_800F98F4) + (((unsigned long)a3) * 6));
            if ((*((u32*)(t2 - 2))) & 2)
            {
                t0[7] = 0x66;
            }
            v1 = ((unsigned long)t0) & t4;
            *((u16*)(t0 + 8)) = (inline_fn(t2 - (-2)) + g_slotSlideXLerped) - ((a1[4] * 8) - 0x20);
            *((u16*)(t0 + 10)) = (inline_fn(t2 + 4) + g_slotSlideYLerped) - ((a1[5] * 8) - 0x28);
            t0[12] = a1[0] * 8;
            t0[13] = a1[1] * 8;
            *((u16*)(t0 + 0x10)) = a1[2] * 8;
            *((u16*)(t0 + 0x12)) = a1[3] * 8;
            a3 = s0 + ((t2[0] * 8) * 2);
            v0 = (inline_fn(a3 + 6) << 6) | ((inline_fn(a3 - (-4)) >> 4) & 0x3F);
            *((u16*)(t0 + 0x0E)) = v0;
            v1 = (*((u32*)t0)) & t8;
            ;
            *((u32*)t0) = v1 | ((*t6) & t4);
            v1 = ((unsigned long)t0) & t4;
            t0 += 0x14;
            v0 = (*t6) & t8;
            *t6 = v0 | v1;
            t0[3] = 1;
            a1 = s0 + (t2[0] * 0x10);
            a2_16 = inline_fn(a1 + 2);
            new_var6 = (*((u32*)(t2 - 2))) << 3;
            ;
            *((u32*)(t0 + 4)) = (((((((*((u32*)(a1 + 0x0C))) & 3) << 7) | (new_var6 & 0x60)) | ((a2_16 & 0x100) >> 4)) | ((inline_fn(a1) & 0x3FF) >> 6)) |
                                 ((a2_16 & 0x200) * 4)) |
                                0xE1000000;
            v1 = (*((u32*)t0)) & t8;
            ;
            new_var8 = v1 | ((*t6) & t4);
            *((u32*)t0) = new_var8;
            t0 += 8;
            v0 = (*t6) & t8;
            // FIX: use the old address of t0 (before the +8) instead of the old value
            *t6 = v0 | ((unsigned long)(t0 - 8) & t4);
        }
        else if (v0 == 2)
        {
            new_var10 = (u32*)t0;
            *((u32*)(t0 + 4)) = 0x40;
            v1 = ((unsigned long)t0) & t4;
            t0[3] = s5;
            t0[7] = 0x62;
            *((u16*)(t0 + 8)) = inline_fn(t2 + 6) + g_slotSlideXLerped;
            do
            {
                *((u16*)(t0 + 10)) = inline_fn(t2 + 8) + g_slotSlideYLerped;
                *((u16*)(t0 + 12)) = inline_fn(t2 + 14);
                *((u16*)(t0 + 14)) = inline_fn(t2 + 16);
                v1 = t8;
                v1 = (*((u32*)t0)) & v1;
                v0 = (*t6) & t4;
                *new_var10 = v1 | v0;
                v1 = (*t6) & t8;
                v0 = ((unsigned long)t0) & t4;
                *t6 = v1 | v0;
                a2 = t0 + 0x10;
                a2[3] = 1;
                *((u32*)(a2 + 4)) = 0xE1000025;
                v1 = (*((u32*)(t0 + 0x10))) & t8;
                v0 = (*t6) & t4;
                *((u32*)(t0 + 0x10)) = v1 | v0;
                // FIX: Use the address of the second sprite, not the lower bits of *t6
                *t6 = ((*t6) & t8) | ((unsigned long)a2 & t4);
                t0 += 0x18;
            } while (0);
        }
        else
        {
            new_var = t0;
            if (v0 != 0)
            {
                t3 = inline_fn(t2 + 14);
                t5 = inline_fn(t2 + 10);
                a1 = s0 + (t2[0] * 0x10);
                a3_16 = inline_fn(a1);
                s2 = (*((u32*)(t2 - 2))) & 1;
                if (s2)
                {
                    do
                    {
                        a1_16 = (*((s16*)(t2 + 2))) + g_slotSlideXLerped;
                        t9_16 = (*((s16*)(t2 + 4))) + g_slotSlideYLerped;
                    } while (0);
                }
                else
                {
                    a1_16 = *((s16*)(t2 + 2));
                    t9_16 = *((s16*)(t2 + 4));
                }
                t1 = 0x80;
                if (((t3 + 1) - 1) < 0x81)
                {
                    t1 = t3;
                }
                a2 = new_var + 4;
                while (1)
                {
                    *((u32*)(a2 + 0)) = 0x808080;
                    a2[-1] = 4;
                    a2[3] = 0x64;
                    if ((*((u32*)(t2 - 2))) & 2)
                    {
                        a2[3] = 0x66;
                    }
                    *((s16*)(a2 + 4)) = a1_16;
                    *((s16*)(a2 + 6)) = t9_16;
                    a2[8] = t5;
                    new_var7 = a2 + 14;
                    *((u16*)(a2 + 12)) = t1;
                    a2[9] = t2[12];
                    *((u16*)new_var7) = inline_fn(t2 + 16);
                    t3 -= t1;
                    a1 = s0 + (t2[0] * 0x10);
                    v0 = (inline_fn(a1 + 6) << 6) | ((inline_fn(a1 + 4) >> 4) & 0x3F);
                    *((u16*)(a2 + 10)) = v0;

                    a2 += 0x14;

                    v1 = (*((u32*)t0)) & t8;
                    v0 = (*t6) & t4;
                    *((u32*)t0) = v1 | v0;
                    v1 = ((unsigned long)t0) & t4;
                    t0 += 0x14;
                    v0 = (*t6) & t8;
                    *t6 = v0 | v1;
                    a2[-1] = 1;
                    a1 = s0 + (t2[0] * 0x10);
                    a2_16 = inline_fn(a1 + 2);
                    t9_16 = ((*((u32*)(t2 - 2))) << 3) & 0x60;
                    *((u32*)a2) = (((((((*((u32*)(a1 + 0x0C))) & 3) << 7) | t9_16) | ((a2_16 & 0x100) >> 4)) | (((s32)(a3_16 & 0x3FF)) >> 6)) |
                                   ((0, (a2_16 & 0x200) * 4))) |
                                  0xE1000000;
                    a2 += 8;
                    v1 = (*((u32*)t0)) & t8;
                    v0 = ((0, *t6)) & t4;
                    *((u32*)t0) = v1 | v0;
                    v1 = ((unsigned long)t0) & t4;
                    t0 += 8;
                    t9_16 = (*t6) & t8;
                    v0 = t9_16;
                    *t6 = v0 | v1;
                    if (t3 == 0)
                    {
                        break;
                    }
                    t5 ^= 0x80;
                    a1 = (new_var4 = s0 + (t2[0] * 0x10));
                    if (!((*((u32*)(a1 + 0x0C))) & 7))
                    {
                        a3_16 += 0x20;
                    }
                    else
                    {
                        a3_16 += 0x40;
                        t5 = 0;
                    }
                    t1 = 0x80;
                    if (t3 < 0x81)
                    {
                        t1 = t3;
                    }
                    a1_16 += 0x80;
                }
            }
        }
        s2++;
        t2 += 0x18;
    } while (s2 < 0x1B);
    return t0;
}

/**
 * For each of the 11 entries in g_saveLayoutTexTable (stride 0x10), uploads the
 * CLUT and image data to VRAM and bit-packs the resulting tex-page info
 * back into the entry's control word.
 *
 * decomp.me (100%) https://decomp.me/scratch/lzJHa
 */
unsigned short UploadSaveLayoutTextures(void)
{
    unsigned char* entry_base;
    u32 new_var3;
    unsigned char* control_ptr;
    unsigned char* db;
    u32 offset8;
    unsigned char* secondary;
    int product;
    int new_var2;
    u32 new_var4;
    u32 control;
    RECT rect;
    int counter;
    unsigned char* new_var;
    entry_base = g_saveLayoutTexTable;

    for (counter = 0; counter < 11; counter++)
    {
        control_ptr = entry_base;
        secondary = *((unsigned char**)(control_ptr + 8));
        new_var3 = *((u32*)(control_ptr + 0xc));
        control = new_var3;
        db = secondary;
        new_var = db + 0x12;
        control = (control & ((u32)(-8))) | (db[4] & 7);
        *((u32*)(control_ptr + 0xc)) = control;
        product = (*((u16*)(db + 0x10))) * (*((u16*)new_var));
        offset8 = *((u32*)(db + 8));
        db += 8;
        rect.x = *((s16*)((entry_base + 0xc) - 8));
        product++;
        product--;
        rect.y = *((s16*)((entry_base + 0xc) - 6));
        rect.h = 1;
        rect.w = product;

        LoadImage(&rect, (u_long*)(db + 0xc), product);
        secondary = db + offset8;
        new_var2 = 3;
        control = (new_var4 = *((u32*)(control_ptr + 0xc)));
        control = (control & ((u32)(-0x1ff9))) | (((*((u16*)(secondary + 8))) & 0x3ff) << new_var2);
        *((u32*)(entry_base + 0xc)) = control;
        control = control & 0xFF801FFF;
        control = control | (((*((u16*)(secondary + 0xa))) & 0x3ff) << 13);
        *((u32*)(entry_base + 0xc)) = control;
        rect.x = *((s16*)entry_base);
        rect.y = *((s16*)((entry_base + 0xc) - 0xa));
        rect.w = ((*((u32*)(entry_base + 0xc))) >> 3) & 0x3ff;
        rect.h = ((*((u32*)(entry_base + 0xc))) >> 13) & 0x3ff;
        LoadImage(&rect, (u_long*)(secondary + 0xc));
        entry_base += 0x10;
        control_ptr += 0x10;
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