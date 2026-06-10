#include "gname.h"

/**
 * @brief Reset the RGB fade state.
 *
 * Zeros the current color (@c g_fade_current) and the target color +
 * step count (@c g_fade_target). After this call the next
 * @ref render_fade_overlay tick will write a black tint (0,0,0) to the
 * primitive at @c arg0->prim_cursor.
 *
 * @see https://decomp.me/scratch/ld2aW (100%)
 */
void reset_fade_state(void)
{
    g_fade_current.r = 0;
    g_fade_current.g = 0;
    g_fade_current.b = 0;
    g_fade_target.r = 0;
    g_fade_target.g = 0;
    g_fade_target.b = 0;
    g_fade_target.steps = 0;
}

/**
 * @brief Per-frame RGB fade tick: lerp current toward target and emit a
 *        full-screen tint quad + draw-mode pair into the OT.
 *
 * If `g_fade_target.steps` is non-zero, advances `g_fade_current` by one step of
 * `(target - current) / steps` per channel and decrements `steps`.
 * Otherwise snaps current to target (RGB only - `steps` is left alone).
 *
 * If the current color is not the identity (@c FADE_CHAN_NEUTRAL on all
 * channels), emits two primitives at @c ctx->prim_cursor:
 *   1. A full-screen TILE (@c SCREEN_WIDTH x @c SCREEN_HEIGHT) with the
 *      tinted RGB. Channels >= @c FADE_CHAN_ADDITIVE are written as
 *      `value - 1` (additive bias); channels below as `~value`
 *      (subtractive bias).
 *   2. A @c DR_TPAGE packet: @c FADE_TPAGE_ADD (abr=1, Back+Front,
 *      brightening) when any channel >= @c FADE_CHAN_ADDITIVE, or
 *      @c FADE_TPAGE_SUB (abr=2, Back-Front, darkening) otherwise.
 *
 * Both packets are spliced into @c ctx->ot[0] and @c ctx->prim_cursor is
 * advanced past them. When the color is identity no primitives are emitted.
 *
 * @param ctx Render context whose @c ot[0] is the OT entry and
 *            @c prim_cursor is the primitive heap cursor.
 *
 * @note Equivalent to TITLE.BIN's RenderFadeOverlay.
 * @see https://decomp.me/scratch/NvocJ (100%)
 */
static void render_fade_overlay(RenderContext* ctx)
{
    void* prim = ctx->prim_cursor;
    RenderContext* p_ctx = ctx;
    s32 step_r;
    s32 step_g;
    s32 step_b;
    s32 tpage;

    /* Lerp current toward target, or snap if no steps remain. */
    if (g_fade_target.steps != 0)
    {
        step_r = (g_fade_target.r - g_fade_current.r) / g_fade_target.steps;
        step_g = (g_fade_target.g - g_fade_current.g) / g_fade_target.steps;
        step_b = (g_fade_target.b - g_fade_current.b) / g_fade_target.steps;
        g_fade_target.steps--;
        g_fade_current.r += step_r;
        g_fade_current.g += step_g;
        g_fade_current.b += step_b;
    }
    else
    {
        g_fade_current.r = g_fade_target.r;
        g_fade_current.g = g_fade_target.g;
        g_fade_current.b = g_fade_target.b;
    }

    /* Skip emit when all channels are neutral (identity tint). */
    if (!((g_fade_current.r != FADE_CHAN_NEUTRAL) || (g_fade_current.g != FADE_CHAN_NEUTRAL) || (g_fade_current.b != FADE_CHAN_NEUTRAL)))
    {
        ctx->prim_cursor = prim;
        return;
    }

    /* Write RGB into the flat-quad color bytes. */
    if (g_fade_current.r >= FADE_CHAN_ADDITIVE)
    {
        /* Additive bias: subtract 1 so FADE_CHAN_ADDITIVE maps to 0x00. */
        ((TILE*)prim)->r0 = (u8)g_fade_current.r - 1;
        ((TILE*)prim)->g0 = (u8)g_fade_current.g - 1;
        ((TILE*)prim)->b0 = (u8)g_fade_current.b - 1;
    }
    else
    {
        /* Subtractive bias: bitwise NOT so 0xFF->0x00, 0x00->0xFF.
         * FADE_CHAN_NEUTRAL (casts to 0 as u8) is clamped to 0 explicitly. */
        if (g_fade_current.r == FADE_CHAN_NEUTRAL)
        {
            ((TILE*)prim)->r0 = 0;
        }
        else
        {
            ((TILE*)prim)->r0 = ~g_fade_current.r;
        }

        if (g_fade_current.g == FADE_CHAN_NEUTRAL)
        {
            ((TILE*)prim)->g0 = 0;
        }
        else
        {
            ((TILE*)prim)->g0 = ~g_fade_current.g;
        }

        if (g_fade_current.b == FADE_CHAN_NEUTRAL)
        {
            ((TILE*)prim)->b0 = 0;
        }
        else
        {
            ((TILE*)prim)->b0 = ~g_fade_current.b;
        }
    }

    setTile(prim);
    setSemiTrans(prim, 1);
    SET_YX0((TILE*)prim, 0, 0);
    setWH((TILE*)prim, SCREEN_WIDTH, SCREEN_HEIGHT);
    addPrim(p_ctx->ot, prim);
    prim = (TILE*)prim + 1;

    /* Choose blend mode by direction of tint. */
    tpage = g_fade_current.r < FADE_CHAN_ADDITIVE ? FADE_TPAGE_SUB : FADE_TPAGE_ADD;

    setDrawTPage(prim, 0, 0, tpage);
    addPrim(p_ctx->ot, prim);
    prim = (DR_TPAGE*)prim + 1;

    ctx->prim_cursor = prim;
}

/**
 * @brief Set the RGB fade target and step count.
 *
 * Writes the four-field target struct in one call. The next
 * `steps` ticks of @ref render_fade_overlay will lerp the current color toward
 * `(r, g, b)` and then snap on the final tick.
 *
 * @param r     Target red   (0..0x100 normal, >0x100 = additive).
 * @param g     Target green (0..0x100 normal, >0x100 = additive).
 * @param b     Target blue  (0..0x100 normal, >0x100 = additive).
 * @param steps Frames over which to interpolate. 0 means "snap immediately".
 *
 * @see https://decomp.me/scratch/jq3uD (100%)
 */
static void set_fade_target(s32 r, s32 g, s32 b, s32 steps)
{
    g_fade_target.r = r;
    g_fade_target.g = g;
    g_fade_target.b = b;
    g_fade_target.steps = steps;
}

/**
 * @brief Overlay boot/reset entry: prep VRAM, load CLUT, init run state.
 *
 * Calls (in order):
 *  - @ref load_name_entry_tim  - uploads the overlay's glyph TIM to VRAM.
 *  - @ref func_800AA02C  - engine helper (audio/SFX init).
 *  - sets @c g_startup_delay = 0x28 (40 - likely a startup delay countdown).
 *  - @ref func_8006441C  - engine helper.
 *  - @ref reset_run_state  - zero/seed all of the overlay's run-state globals.
 *  - @ref func_80063194  - engine helper.
 *
 * @see https://decomp.me/scratch/pnzC1 (100%)
 */
void gname_init(void)
{
    volatile int dummy[2]; /* forces 0x20 stack frame, ra at 0x18(sp) */
    load_name_entry_tim();
    func_800AA02C();
    g_startup_delay = 0x28;
    func_8006441C();
    reset_run_state();
    func_80063194();
}

/**
 * @brief Upload the name-entry overlay's glyph TIM to its fixed VRAM slots.
 *
 * Builds the destination-coordinate block consumed by @ref load_tim_to_vram
 * and loads @c g_name_entry_tim. The four packed s16 coordinates are:
 *   - [0],[1] = pixel-data destination, VRAM (@c SCREEN_WIDTH, 0).
 *   - [2],[3] = CLUT destination, VRAM (0, @c VRAM_CLUT_Y).
 *
 * @see https://decomp.me/scratch/EWwJI (100%)
 */
void load_name_entry_tim(void)
{
    TimDstCoords dst_coords;
    dst_coords.pixel_x = SCREEN_WIDTH;
    dst_coords.pixel_y = 0;
    dst_coords.clut_x = 0;
    dst_coords.clut_y = VRAM_CLUT_Y;
    load_tim_to_vram(&dst_coords);
}

/**
 * @brief Upload a TIM image's pixel data and CLUT to VRAM.
 *
 * Parses @c g_name_entry_tim: the CLUT block starts at @c TIM_HEADER_SIZE,
 * its @c CLUT_ENTRY_COUNT palette entries at @c TIM_CLUT_DATA_OFFSET. Before uploading,
 * @c GPU_STP_BIT is OR'd into every non-zero CLUT entry. The TIM's own
 * embedded destination coordinates are ignored; @p dst_coords supplies them.
 *
 * @param dst_coords Pixel and CLUT VRAM destination coordinates.
 *
 * @note @c func_80019A34 is the engine's LoadImage-style VRAM upload
 *       (RECT, source data).
 * @see https://decomp.me/scratch/P3W9C (100%)
 */
void load_tim_to_vram(TimDstCoords* dst_coords)
{
    RECT rect;
    TimBlock* pixel_block;
    int i;
    Tim* tim = &g_name_entry_tim;
    s32 clut_len = tim->clut_block.bnum;
    u16* clut = tim->clut_data;

    rect.x = dst_coords->clut_x;
    rect.y = dst_coords->clut_y;
    rect.w = CLUT_ENTRY_COUNT;
    rect.h = 1;

    /* Mark every non-zero CLUT entry semi-transparent. */
    for (i = 0; i < CLUT_ENTRY_COUNT; i++)
    {
        if ((*clut) != 0)
        {
            *clut |= GPU_STP_BIT;
        }
        clut++;
    }

    func_80019A34(&rect, tim->clut_data);
    pixel_block = TIM_PIXEL_BLOCK(tim, clut_len);

    rect.x = dst_coords->pixel_x;
    rect.y = dst_coords->pixel_y;
    rect.w = pixel_block->w;
    rect.h = pixel_block->h;

    func_80019A34(&rect, pixel_block + 1);

    /* Trailing rect writes are dead but load-bearing for the match. */
    rect.x = dst_coords->clut_x;
    rect.y = dst_coords->clut_y + 1;
    rect.w = CLUT_ENTRY_COUNT;
    rect.h = 1;
}

/**
 * @brief Per-frame tick: reset/prep, render frame contents, advance frame
 *        counter, advance overlay state machine.
 *
 *  - @ref draw_name_cursor_row  - emits the name-entry cursor glyph row.
 *  - @ref gname_render  - main render pass for this overlay.
 *  - increments the global frame counter @c g_frame_counter.
 *  - @ref gname_update_state  - countdown / lerp / SFX trigger update.
 *
 * @param ctx Render context passed through to @ref gname_render.
 *
 * @see https://decomp.me/scratch/yYkTM (100%)
 */
void gname_tick(s32 ctx)
{
    draw_name_cursor_row();
    gname_render(ctx);
    g_frame_counter += 1;
    gname_update_state();
}

/**
 * @brief Per-frame state-machine update: countdown, scalar lerp, input SFX.
 *
 *  - When the startup countdown @c g_startup_delay hits zero, hands off to
 *    @ref gname_process_input (the next stage); otherwise decrements it.
 *  - Lerps the scalar @c g_strip_width toward @c g_strip_width_target over
 *    @c g_strip_width_steps frames using the same `(target - current)/steps` shape
 *    as the RGB fade.
 *  - When input mask @c g_pad_input == 0x800 (a specific button bit), plays
 *    one of two SFX via @ref play_menu_sfx (bank 0x80, sound 0x7E or 0x78)
 *    based on whether the cursor's current entry passes the
 *    @ref name_char_count / @ref name_is_blank validation pair, and on the
 *    "valid" path also kicks @c g_overlay_result = 5 to advance overlay state.
 *
 * @see https://decomp.me/scratch/g5Rx3 (100%)
 */
void gname_update_state(void)
{
    s32 steps;

    /* Startup delay countdown. */
    if (g_startup_delay == 0)
    {
        gname_process_input();
    }
    else
    {
        g_startup_delay--;
    }

    /* Lerp g_strip_width toward g_strip_width_target, snap when no steps remain. */
    steps = g_strip_width_steps;
    if (steps != 0)
    {
        g_strip_width_steps--;
        g_strip_width += (g_strip_width_target - g_strip_width) / steps;
    }
    else
    {
        g_strip_width = g_strip_width_target;
    }

    /* Confirm-button: play accept SFX on valid entry, reject SFX otherwise. */
    if (g_pad_input == 0x800)
    {
        if ((name_char_count(g_active_name) != 0) && (name_is_blank(g_active_name) == 0))
        {
            play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME); /* accept */
            g_overlay_result = 5;
            return;
        }
        play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME); /* reject */
    }
}

/**
 * @brief Reset the overlay's run-state globals to their per-session defaults.
 *
 * Called from the boot path @ref gname_init. Zeros most counters/indices,
 * primes the lerp scalar (@c g_strip_width_steps = 5), seeds the cursor state
 * (@c g_cursor_x / @c g_cursor_y from frozen defaults @c g_cursor_x_target /
 * @c g_cursor_y_target), kicks @ref handle_char_set_input to compute initial @c g_char_set_mode,
 * and registers the overlay's per-character buffer with
 * @ref name_copy (`g_active_name`, `&g_initial_name`).
 *
 * @see https://decomp.me/scratch/FboaU (100%)
 */
void reset_run_state(void)
{
    g_cursor_tab = 0xFF;
    g_char_set_mode = handle_char_set_input(0, 0);
    g_cursor_lerp_steps = 0;
    g_scroll_pos = 0;
    g_scroll_target = 0;
    g_scroll_steps = 0;
    g_char_cursor = 0;
    g_name_clipboard = 0;
    g_cursor_x = g_cursor_x_target;
    g_cursor_y = g_cursor_y_target;
    name_copy(g_active_name, &g_initial_name); /* matches 'la a1, g_initial_name' */
    g_strip_width = 0;
    recalc_name_width();
    g_strip_width_steps = 5;
    g_append_anim_frame = 0;
    g_append_anim_timer = 2;
    g_char_panel = 0;
}

inline int inline_fn(s32 arg0)
{
    return arg0 * 2;
}

/**
 * @brief Process one frame of name-entry UI input and return the new char-set mode.
 *
 * State machine for the name-entry character selection screen. @p mode encodes
 * which region of the UI is currently focused:
 *   - GNAME_MODE_ACTION_OK..DEFAULT (0-3): action tab bar
 *   - GNAME_MODE_PANEL_BASE..+3 (4-7): character-panel selector tabs (panel N-4)
 *   - GNAME_MODE_GRID (0x10): in-grid character cursor mode
 *
 * @p buttons is the caller-filtered pad bitmask (e.g. @c g_pad_input & @c GNAME_BTN_NAV_MASK).
 * As a side effect the function updates cursor position, panel, scroll state,
 * and name buffer contents before returning the next mode value.
 *
 * @param mode    Current char-set mode (see above).
 * @param buttons Filtered button bitmask from @c g_pad_input.
 * @return        New char-set mode after processing the input.
 * @see decomp.me (98.22%) https://decomp.me/scratch/PpqVd
 */
s32 handle_char_set_input(s32 arg0, s32 arg1)
{
    s32 var_s1 = arg0;
    s32 var_s2 = arg1;
    s32 var_s0 = 0xFF;
    int offset = 0x68;

    while (var_s0 == 0xFF)
    {
        switch (var_s1)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            /* tabs 0-3: control tabs (back, delete, random, history, etc.) */
            if (var_s2 & 0x220)
            {
                g_cursor_tab = var_s1;
                switch (var_s1)
                {
                case 0:
                    if (name_char_count(g_active_name) != 0 && !name_is_blank(g_active_name))
                    {
                        play_menu_sfx(0x7E, 0x80);
                        g_overlay_result = 5;
                    }
                    else
                    {
                        play_menu_sfx(0x78, 0x80);
                    }
                    var_s0 = 0;
                    continue;
                case 1:
                    play_menu_sfx(0x7E, 0x80);
                    name_pop_last_char(g_active_name);
                    break;
                case 2:
                    play_menu_sfx(0x7E, 0x80);
                    if (g_name_source_mode == 4)
                    {
                        g_name_clipboard = 0;
                        name_copy(g_active_name, (g_random_names - 0x10) + *(u32*)g_random_names +
                                                     *(u16*)((g_random_names - 0x10) + *(u32*)g_random_names + (rand() % 128) * 2));
                    }
                    else if (g_name_source_mode == 5)
                    {
                        g_name_clipboard = 0;
                        name_copy(g_active_name, (g_random_names - 0x10) + *(u32*)g_random_names +
                                                     *(u16*)((g_random_names - 0x10) + *(u32*)g_random_names + ((rand() % 128) + 128) * 2));
                    }
                    else if (g_name_source_mode == 3)
                    {
                        g_name_clipboard = 0;
                        if (g_history_name_idx >= 0x81)
                        {
                            name_copy(g_active_name, &g_initial_name);
                        }
                        else
                        {
                            name_copy(g_active_name, (g_random_names - 0x10) + *(u32*)g_history_names +
                                                         *(u16*)((g_random_names - 0x10) + *(u32*)g_history_names + g_history_name_idx * 2));
                            name_append(g_active_name, (g_random_names - 0x10) + *(u32*)g_history_names +
                                                           *(u16*)((g_history_names - 0x10) + *(u32*)g_history_names + ((rand() % 128) + 130) * 2));
                        }
                    }
                    else if (g_name_source_mode == 1)
                    {
                        g_name_clipboard = 0;
                        name_copy(g_active_name, &g_custom_name_buf);
                    }
                    else
                    {
                        play_menu_sfx(0x7E, 0x80);
                        g_name_clipboard = 0;
                        name_copy(g_active_name, &g_initial_name);
                    }
                    break;
                case 3:
                    play_menu_sfx(0x7E, 0x80);
                    g_name_clipboard = 0;
                    name_copy(g_active_name, &g_initial_name);
                    break;
                default:
                    var_s0 = 0;
                    continue;
                }
                recalc_name_width();
                var_s0 = 0;
                g_strip_width_steps = 5;
            }
            else
            {
                if (var_s2 != 0)
                {
                    if (var_s2 & PAD_BTN_DOWN)
                    { /* DOWN */
                        var_s1 = 0x10;
                        var_s2 = 0;
                        continue;
                    }
                    if (var_s2 & PAD_BTN_LEFT)
                    { /* LEFT */
                        var_s1 = (var_s1 == 0) ? 3 : (var_s1 - 1);
                    }
                    else if (var_s2 & PAD_BTN_RIGHT)
                    { /* RIGHT */
                        var_s1 = (var_s1 < 3) ? (var_s1 + 1) : 0;
                    }
                }

                play_menu_sfx(0x7D, 0x80);
                g_cursor_x_target = g_tab_cursor_pos[var_s1 + 2].x - 8;
                g_cursor_lerp_steps = 5;
                g_cursor_y_target = g_tab_cursor_pos[var_s1 + 2].y;
                var_s0 = 0;
            }
            break;

        case 4:
        case 5:
        case 6:
        case 7:

            if ((var_s2 & 0x220) && (g_cursor_tab = var_s1, g_char_panel != var_s1 - 4))
            {
                var_s2 = 0;

                g_scroll_target = 0;
                g_scroll_pos = 0;
                g_char_panel = g_cursor_tab - 4;
                g_scroll_steps = 0;
                g_char_cursor = 0;
                play_menu_sfx(0x7E, 0x80);

                continue;
            }
            else
            {
                if (var_s2 != 0)
                {
                    if (var_s2 & PAD_BTN_RIGHT)
                    { /* RIGHT */
                        var_s1 = 0x10;
                        var_s2 = 0;
                        continue;
                    }
                    if (var_s2 & PAD_BTN_UP)
                    { /* UP */
                        var_s1 = (var_s1 == 4) ? 6 : (var_s1 - 1);
                    }
                    else if (var_s2 & PAD_BTN_DOWN)
                    { /* DOWN */
                        var_s1 = (var_s1 < 6) ? (var_s1 + 1) : 4;
                    }
                }

                play_menu_sfx(0x7D, 0x80);
                g_cursor_x_target = g_tab_cursor_pos[var_s1 + 2].x - 8;
                g_cursor_lerp_steps = 5;
                g_cursor_y_target = g_tab_cursor_pos[var_s1 + 2].y;
                var_s0 = 0;
            }
            break;

        default:
            /* character selection panel */
            if ((var_s2 & 0x220) && ((g_char_last_row * 10) + g_char_last_col) >= g_char_cursor)
            {

                /* Replaced Switch with If-Else chain */
                if (g_char_panel < 3)
                {
                    if (name_char_count(g_active_name) < 10)
                    {
                        u8* argA;
                        g_append_anim_timer = 2;
                        argA =
                            ((g_random_names - 0x10) + (u32)g_char_panel_data) +
                            (*((u16*)((((g_random_names - 0x10) + (u32)g_char_panel_data) + (g_panel_char_offsets[g_char_panel] * 2)) + (g_char_cursor * 2))));
                        g_append_anim_frame = 0;
                        name_append(g_active_name, argA);
                        recalc_name_width();
                        g_strip_width_steps = 5;
                        play_menu_sfx(0x7D, 0x80);
                    }
                    else
                    {
                        play_menu_sfx(0x78, 0x80);
                    }
                }
                else if (g_char_panel == 3)
                {
                    if (g_kanji_cat_entries[g_char_cursor] == 0xFF)
                    {
                        var_s0 = 0;
                        continue;
                    }
                    g_kanji_cat = g_char_cursor;
                    g_char_panel = 4;
                    g_scroll_target = 0;
                    g_scroll_pos = 0;
                    g_scroll_steps = 0;
                    g_cursor_x_target = 0x54;
                    g_cursor_y_target = 0x68;
                    g_cursor_lerp_steps = 4;
                    g_char_cursor = 0;
                    g_kanji_cat_name = (g_random_names - 0x10) + (u32)g_char_panel_data +
                                       *(u16*)((g_random_names - 0x10) + (u32)g_char_panel_data + g_kanji_cat_names_offset * 2 + g_kanji_cat * 2);
                    play_menu_sfx(0x7E, 0x80);
                }
                else if (g_char_panel == 4)
                {
                    if (name_char_count(g_active_name) < 10)
                    {
                        u8* argA;
                        g_append_anim_timer = 2;
                        argA = (g_random_names - 0x10) + (u32)g_kanji_panel_data +
                               *(u16*)((g_random_names - 0x10) + (u32)g_kanji_panel_data + g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat]] * 2 +
                                       g_char_cursor * 2);
                        g_append_anim_frame = 0;
                        name_append(g_active_name, argA);
                        recalc_name_width();
                        g_strip_width_steps = 5;
                        play_menu_sfx(0x7D, 0x80);
                    }
                    else
                    {
                        play_menu_sfx(0x78, 0x80);
                    }
                }
                var_s0 = 0;
            }
            else
            {
                if (var_s2 != 0)
                {
                    /* Early-out panel jump chain */
                    if ((var_s2 & PAD_BTN_UP) && (g_char_cursor / 10 == 0))
                    {
                        var_s1 = 0;
                        var_s2 = 0;
                        continue;
                    }
                    if ((var_s2 & PAD_BTN_LEFT) && ((g_char_cursor % 10) == 0))
                    {
                        var_s1 = 4;
                        var_s2 = 0;
                        continue;
                    }

                    if ((var_s2 & PAD_BTN_UP) && (g_char_cursor / 10 != 0))
                    {
                        g_char_cursor -= 10;
                    }
                    else if ((var_s2 & PAD_BTN_DOWN) && (g_char_cursor / 10 != g_char_last_row))
                    {
                        g_char_cursor += 10;
                    }
                    else if ((var_s2 & PAD_BTN_LEFT) && ((g_char_cursor % 10) != 0))
                    {
                        g_char_cursor -= 1;
                    }
                    else if ((var_s2 & PAD_BTN_RIGHT) && ((g_char_cursor % 10) != 9))
                    {
                        g_char_cursor += 1;
                    }
                    else
                    {
                        var_s0 = 0;
                        continue;
                    }
                }

                play_menu_sfx(0x7D, 0x80);
                g_cursor_x_target = (g_char_cursor % 10) * 0x10 + 0x54;
                g_cursor_y_target = (g_char_cursor / 10) * 0x10 - (g_scroll_pos - offset);

                if (g_cursor_y_target < 0x68)
                {
                    g_cursor_y_target = 0x68;
                    g_scroll_target = (g_char_cursor / 10) * 0x10;
                    g_scroll_steps = 4;
                }
                if (g_cursor_y_target >= 0xA9)
                {
                    g_cursor_y_target = 0xA8;
                    g_scroll_target = (g_char_cursor / 10) * 0x10 - 0x40;
                    g_scroll_steps = 4;
                }

                g_cursor_lerp_steps = 4;
                var_s0 = 0;
            }
            break;
        }
    }

    return var_s1;
}

/**
 * decomp.me (96.22%) https://decomp.me/scratch/ctu1w
 */
/**
 * @brief Per-frame input handler for the active name-entry phase.
 *
 * Called every frame by @ref gname_update_state once @c g_startup_delay
 * reaches zero. Processes pad input, updates @c g_char_set_mode and the
 * kanji category state, then advances the cursor and scroll lerp animations.
 *
 * Input dispatch (evaluated in priority order):
 *  - @c GNAME_BTN_NAV_MASK (D-pad + confirm): delegated to
 *    @ref handle_char_set_input for grid/tab navigation and character
 *    selection.
 *  - @c GNAME_BTN_UNDO (L2): pop the last character from @c g_active_name,
 *    prepend it to the clipboard (@c g_name_clipboard), trimming the clipboard
 *    to 10 chars first. Plays GNAME_SFX_MOVE.
 *  - @c GNAME_BTN_REDO (R2): pop the first character from the clipboard and
 *    append it to @c g_active_name (if not already at NAME_MAX_CHARS).
 *    Plays GNAME_SFX_MOVE on success, GNAME_SFX_ERROR when full.
 *  - @c PAD_BTN_CIRCLE (cancel): if @c g_allow_empty_cancel and the name is
 *    empty, sets @c g_overlay_result = 2 and returns immediately; otherwise
 *    pops the last character. Plays GNAME_SFX_CANCEL.
 *
 * Kanji category navigation (only when @c g_char_set_mode == GNAME_MODE_GRID
 * and @c g_char_panel == 4):
 *  - @c GNAME_BTN_KANJI_PREV (L1): cycle @c g_kanji_cat back by 10 with wrap.
 *  - @c GNAME_BTN_KANJI_NEXT (R1): cycle @c g_kanji_cat forward by 10 with wrap.
 *  - On a valid category (@c g_kanji_cat_entries[cat] != 0xFF), resets scroll
 *    and cursor to the top-left of the new page, resolves and stores
 *    @c g_kanji_cat_name, and clears the nav bits from @c g_pad_input.
 *
 * Lerp updates (run unconditionally after input):
 *  - Cursor (@c g_cursor_x / @c g_cursor_y): step toward target by
 *    (target - current) / steps, decrement @c g_cursor_lerp_steps; snap on 0.
 *  - Scroll (@c g_scroll_pos): same shape toward @c g_scroll_target via
 *    @c g_scroll_steps; snap on 0.
 *
 * @note The dead code block after the empty-cancel @c return (lines following
 *       "if (!g_cursor_x_target)") is a codegen artifact and must be preserved.
 * @see decomp.me TODO
 */
void gname_process_input(void)
{
    s8 char_lo;
    s8 char_hi;
    s8 char_null;
    s32 sfx_id;
    s32 cat_prev;
    u8(*clipboard_ptr)[];
    s32 temp_a0_2;
    s32 nav_input;
    s32 cat_after_dec;
    s32 cursor_dx;
    s32 undo_char;
    s32 scroll_step;
    s32* scroll_ptr;
    s32 cat_after_inc;
    s32 cursor_dy;
    s32 panel3_off;
    s8 clipboard_char;
    u16 clipboard_char_u16;
    u8* base;
    u32 idx;
    u32 offset;
    u16 kanji_name_tbl_off;
    s32 cat_prev_inc;
    s32 kanji_panel_offset;
    int sfx_vol;
    u8* ptr;
    u16 val;
    nav_input = g_pad_input & GNAME_BTN_NAV_MASK;
    g_cursor_tab = 0xFF;
    if (nav_input != 0)
    {
        g_char_set_mode = handle_char_set_input(g_char_set_mode, nav_input);
    }
    else if (g_pad_input & GNAME_BTN_UNDO)
    {
        /* Undo: move last char of active name to front of clipboard. */
        undo_char = name_pop_last_char(g_active_name);
        while (name_char_count(&g_name_clipboard) >= 0xB)
        {
            name_pop_last_char(&g_name_clipboard);
        }

        name_prepend_char(&g_name_clipboard, undo_char & 0xFFFF);
        recalc_name_width();
        g_strip_width_steps = 5;
        sfx_id = GNAME_SFX_MOVE;
        sfx_vol = GNAME_SFX_VOLUME;
        play_menu_sfx(sfx_id, sfx_vol);
    }
    else if (g_pad_input & GNAME_BTN_REDO)
    {
        /* Redo: move first char of clipboard to end of active name. */
        if (name_char_count(g_active_name) < NAME_MAX_CHARS)
        {
            clipboard_ptr = &g_name_clipboard;
            clipboard_char = name_pop_first_char(clipboard_ptr);
            clipboard_char_u16 = (u16)clipboard_char;
            if (clipboard_char_u16 != 0)
            {
                /* Unpack the 2-byte glyph (or single byte) into a stack buffer for name_append. */
                char_lo = clipboard_char;
                (&char_lo)[1] = (s8)(clipboard_char_u16 >> 8);
                (&char_lo)[2] = 0;
                name_append(g_active_name, &char_lo);
                recalc_name_width();
                g_strip_width_steps = 5;
            }
            sfx_id = GNAME_SFX_MOVE;
            play_menu_sfx(sfx_id, GNAME_SFX_VOLUME);
        }
        else
        {
            play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
        }
    }
    else if (g_pad_input & PAD_BTN_CIRCLE)
    {
        if (g_allow_empty_cancel != 0)
        {
            if (name_char_count(g_active_name) == 0)
            {
                g_overlay_result = 2;
                play_menu_sfx(GNAME_SFX_CANCEL, GNAME_SFX_VOLUME);
                return;
                if (!g_cursor_x_target)
                {
                }
            }
        }
        play_menu_sfx(GNAME_SFX_CANCEL, GNAME_SFX_VOLUME);
        name_pop_last_char(g_active_name);
        recalc_name_width();
        g_strip_width_steps = 5;
    }
    if (((g_char_set_mode == GNAME_MODE_GRID) && (g_char_panel == 4)) && (g_pad_input & GNAME_BTN_KANJI_NAV))
    {
        play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
        if (g_pad_input & GNAME_BTN_KANJI_NAV)
        {
            do
            {
                if (g_pad_input & GNAME_BTN_KANJI_PREV)
                {
                    /* Decrement category by 10; wrap from -1 -> 0 or from below 0 -> +41. */
                    cat_prev = g_kanji_cat;
                    cat_after_dec = cat_prev - 0xA;
                    g_kanji_cat = cat_after_dec;
                    if (cat_after_dec == (-1))
                    {
                        g_kanji_cat = 0;
                    }
                    else if (cat_after_dec < 0)
                    {
                        g_kanji_cat = cat_prev + 0x29;
                    }
                }
                else
                {
                    /* Increment category by 10; wrap from 50 -> 9 or from above 50 -> -41. */
                    cat_prev_inc = g_kanji_cat;
                    cat_after_inc = cat_prev_inc + 10;
                    g_kanji_cat = cat_after_inc;
                    if (cat_after_inc == 0x32)
                    {
                        g_kanji_cat = 9;
                    }
                    else if (cat_after_inc >= 0x32)
                    {
                        g_kanji_cat = cat_prev_inc - 0x29;
                    }
                }
                offset = g_kanji_cat;
                if (g_kanji_cat_entries[offset] != 0xFF)
                {
                    g_scroll_target = (long)0;
                    g_scroll_pos = 0;
                    panel3_off = g_panel_char_offsets[3];
                    g_scroll_steps = 0;
                    g_char_cursor = 0;
                    g_cursor_x_target = NAME_GRID_X_BASE;
                    g_cursor_y_target = NAME_GRID_Y_TOP;
                    g_cursor_lerp_steps = 4;
                    kanji_panel_offset = panel3_off;
                    base = D_80142EF4;
                    idx = g_kanji_cat;
                    offset = (idx * 2) + ((kanji_panel_offset * 2) + g_char_panel_data);
                    kanji_name_tbl_off = *((u16*)(base + offset));
                    g_pad_input &= ~GNAME_BTN_KANJI_NAV;
                    g_kanji_cat_name = (void*)((g_char_panel_data + kanji_name_tbl_off) + ((unsigned long)base));
                }
            } while (g_pad_input & GNAME_BTN_KANJI_NAV);
        }
    }
    if (g_cursor_lerp_steps != 0)
    {
        cursor_dx = ((s32)(g_cursor_x_target - g_cursor_x)) / ((s32)g_cursor_lerp_steps);
        cursor_dy = ((s32)(g_cursor_y_target - g_cursor_y)) / ((s32)g_cursor_lerp_steps);
        g_cursor_lerp_steps -= 1;
        g_cursor_x += cursor_dx;
        g_cursor_y += cursor_dy;
    }
    else
    {
        g_cursor_x = g_cursor_x_target;
        g_cursor_y = g_cursor_y_target;
    }
    if (g_scroll_steps != 0)
    {
        scroll_ptr = &g_scroll_pos;
        scroll_step = ((s32)(g_scroll_target - (*scroll_ptr))) / ((s32)g_scroll_steps);
        g_scroll_steps -= 1;
        g_scroll_pos += scroll_step;
        return;
    }
    g_scroll_pos = g_scroll_target;
}

/**
 * @brief Emit the text cursor glyph (g_glyph_table[NAME_CURSOR_GLYPH_COUNT]) as a SPRT + DR_TPAGE pair.
 *
 * Writes a 20-byte SPRT primitive at @p prim using the UV, size, and CLUT
 * data from g_glyph_table[NAME_CURSOR_GLYPH_COUNT], then writes an 8-byte
 * DR_TPAGE packet (texture page 5) immediately after. Both primitives are
 * linked into @p ot via addPrim.
 *
 * @param prim  Write position in the primitive buffer; must have at least
 *              sizeof(SPRT) + sizeof(DR_TPAGE) (28 bytes) available.
 * @param ot    Pointer to the OT slot to chain both primitives into.
 * @param x     Screen X position for the cursor sprite.
 * @param y     Screen Y position for the cursor sprite.
 * @return Pointer past the last written primitive (prim advanced by 7 u_longs).
 * @see decomp.me (100%) https://decomp.me/scratch/oXGkF
 */
u_long* emit_cursor_glyph(u_long* prim, u_long* ot, s16 x, s16 y)
{
    u32 clut;
    SPRT* sprt = (SPRT*)prim;

    SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
    setSprt(sprt);
    setXY0(sprt, x, y);

    setUV0(sprt, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].u, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].v);
    setWH(sprt, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].w, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].h);

    clut = g_glyph_table[NAME_CURSOR_GLYPH_COUNT].clut & GLYPH_CLUT_X_MASK;
    sprt->clut = clut | GLYPH_CLUT_PAGE_BITS;
    addPrim(ot, sprt);

    prim += sizeof(SPRT) / sizeof(u_long);
    setDrawTPage(prim, 0, 0, 5);
    addPrim(ot, prim);

    return prim + sizeof(DR_TPAGE) / sizeof(u_long);
}

/**
 * @brief Main per-frame render pass for the name-entry overlay.
 *
 * Builds this frame's primitives into @p ctx and splices them onto the
 * render context's ordering table. Runs each tick from @ref gname_tick,
 * after @ref draw_name_cursor_row. The work, in order:
 *
 *  1. Character grid: walk grid-table entries 2..12 (skipping 9) and emit a
 *     drop-shadowed glyph SPRT for each via @ref func_80142274 into OT entry
 *     @c ot[0x0B] (offset 0x2C). The entry whose index equals @c g_cursor_tab
 *     is drawn highlighted (the @p arg6 flag of @ref func_80142274), marking
 *     the current selection.
 *  2. Append decoration: emit a static glyph plus the character-append
 *     animation (@ref draw_char_append_anim) into @c ot[0x0D] (offset 0x34),
 *     then run @ref func_80141C34.
 *  3. Text cursor: build a white textured SPRT (glyph @c g_glyph_table[0xA0])
 *     at the cursor position (@c g_cursor_x, @c g_cursor_y), followed by an
 *     additive DrawMode, and chain both into @c ot[0x08] (offset 0x20).
 *  4. Conditional glyphs: when @c g_scroll_pos is set, and again when
 *     @c g_char_last_row >= 5 passes a phase check, emit extra glyphs from the
 *     @c g_tab_cursor_pos table.
 *  5. Hand off to the remaining sub-passes @ref func_80141D64,
 *     @ref func_80141F9C and @ref func_80141E04.
 *
 * @param ctx Render context (@ref RenderContext). Uses OT entries at byte
 *            offsets 0x20/0x24/0x2C/0x34 and the heap cursor at 0x4040.
 *
 * @note Still WIP; locals/params renamed but control flow is left verbatim.
 * @see decomp.me (82.68%) https://decomp.me/scratch/rQBi6
 */
void gname_render(void* ctx)
{
    s32 cursor_x;
    s32 i;
    s32 f8b4;
    s32* entry;
    void* prim;
    char* ctx_bytes;
    void* cursor_sprite;
    void* prim2;
    void* ctx2;
    unsigned char* glyph_ptr;
    char* tmp_ptr;
    entry = (s32*)g_tab_cursor_entries;
    i = 2;
    glyph_ptr = (unsigned char*)g_tab_cursor_entries + 2;
    prim = *((void**)(((char*)ctx) + 0x4040));
    /* 1. Character grid: entries 2..12, skip 9; highlight the selection. */
    do
    {
        if (i != 9)
        {
            prim = func_80142274(prim, ((char*)ctx) + 0x2C, glyph_ptr[1], (*entry) & 0x1FF, ((s32)glyph_ptr[0]) - 8, 1, (i - 2) == g_cursor_tab, 0);
        }
        i += 1;
        glyph_ptr += 4;
        entry++;
    } while (i < 0xD);
    cursor_x = g_cursor_x;
    ctx2 = ctx;
    /* 2. Static glyph + append animation, then func_80141C34. */
    cursor_sprite = func_80141C34(
        emit_draw_mode_prim(
            draw_char_append_anim(func_80142274(emit_draw_mode_prim(prim, ((char*)ctx2) + 0x2C), ((char*)ctx2) + 0x34, (u8)3, 0xE8, 4, 0, 0, 0), ctx),
            ((char*)ctx2) + 0x34),
        ctx);
    /* 3. Text cursor SPRT at (g_cursor_x, g_cursor_y) + additive DrawMode. */
    tmp_ptr = ((char*)cursor_sprite) + 0x14;
    *((s32*)(((char*)cursor_sprite) + 4)) = 0x808080;
    *((u8*)(((char*)cursor_sprite) + 7)) = 0x64;
    prim2 = ((char*)cursor_sprite) + 0x1C;
    *((u8*)(((char*)cursor_sprite) + 3)) = 4;
    *((s16*)(((char*)cursor_sprite) + 8)) = cursor_x;
    {
        s32 tmp = g_cursor_y;
        *((s16*)(((char*)cursor_sprite) + 10)) = tmp;
    }
    *((u8*)(((char*)cursor_sprite) + 12)) = g_glyph_table[NAME_CURSOR_GLYPH_COUNT].u;
    *((u8*)(((char*)cursor_sprite) + 13)) = g_glyph_table[NAME_CURSOR_GLYPH_COUNT].v;
    *((s16*)(((char*)cursor_sprite) + 16)) = (s16)g_glyph_table[NAME_CURSOR_GLYPH_COUNT].w;
    *((s16*)(((char*)cursor_sprite) + 18)) = (s16)g_glyph_table[NAME_CURSOR_GLYPH_COUNT].h;
    {
        u32 tmp = g_glyph_table[NAME_CURSOR_GLYPH_COUNT].clut;
        *((s16*)(((char*)cursor_sprite) + 14)) = (s16)((tmp & GLYPH_CLUT_X_MASK) | GLYPH_CLUT_PAGE_BITS);
    }
    ctx_bytes = (char*)ctx2;
    {
        s32 old = *((s32*)cursor_sprite);
        *((s32*)cursor_sprite) = (old & 0xFF000000) | ((*((s32*)(ctx_bytes + 0x20))) & 0xFFFFFF);
    }
    *((s32*)(ctx_bytes + 0x20)) = ((*((s32*)(ctx_bytes + 0x20))) & 0xFF000000) | (((s32)cursor_sprite) & 0xFFFFFF);
    {
        void* drawmode = ((char*)cursor_sprite) + 0x14;
        *((u8*)(((char*)drawmode) + 3)) = 1;
        *((u32*)(((char*)drawmode) + 4)) = 0xE1000005;
        *((s32*)tmp_ptr) = ((*((s32*)tmp_ptr)) & 0xFF000000) | ((*((s32*)(ctx_bytes + 0x20))) & 0xFFFFFF);
        f8b4 = g_scroll_pos;
        *((s32*)(ctx_bytes + 0x20)) = ((*((s32*)(ctx_bytes + 0x20))) & 0xFF000000) | (((s32)drawmode) & 0xFFFFFF);
    }
    tmp_ptr = ctx_bytes + 0x4040;
    /* 4. Conditional extra glyphs from the g_tab_cursor_pos table. */
    if (g_scroll_pos != 0)
    {
        prim2 = func_80142274(prim2, ctx, g_tab_cursor_pos[0].glyph, (*(s32*)&g_tab_cursor_pos[0]) & 0x1FF, (s32)g_tab_cursor_pos[0].y, 0, 0, 0);
    }
    if (g_char_last_row >= 5)
    {
        f8b4 = g_scroll_pos;
        if (f8b4 < 0)
        {
            f8b4 += 0xF;
        }
        if ((((f8b4 >> 1) >> 1) >> 2) != (g_char_last_row - 4))
        {
            prim2 = func_80142274(prim2, ctx, g_tab_cursor_pos[1].glyph, (*(s32*)&g_tab_cursor_pos[1]) & 0x1FF, (s32)g_tab_cursor_pos[1].y, 0, 0, 0);
        }
    }
    /* 5. Remaining sub-passes. */
    *((void**)tmp_ptr) = func_80141D64(emit_draw_mode_prim(prim2, ctx), ctx_bytes + 0x24);
    func_80141F9C(ctx, g_char_panel);
    func_80141E04(ctx2, g_active_name, g_strip_width);
}

/**
 * decomp.me (62.11%) https://decomp.me/scratch/Yf7Ha
 */
s32 func_80141C34(s32 arg0, s32 arg1)
{
    s32 f8ac;
    s32 var_a0;
    u16 val16;
    u32* entry;
    u32 temp;
    u8* base;
    s32 var;
    s32 f848;
    void* ptr;

    f8ac = g_char_set_mode;
    var_a0 = arg0;

    if (f8ac < 8)
    {
        entry = (u32*)((u32)&g_char_panel_data + ((f8ac + 2) * 4));
        temp = *entry;
        base = ((u8*)&g_char_panel_data) - 4;
        var = g_char_panel_data; /* value, not address */
        val16 = *(u16*)(base + ((temp >> 8) & 0xFE) + var);
        ptr = base + val16 + var;
        var_a0 = func_800A88A0(var_a0, arg1, ptr, 1, 0xb0, 0xc8, 2);
    }
    else if (f8ac == 0x10)
    {
        f848 = g_char_panel;
        if (((u32)(f848 - 3)) < 2U)
        {
            base = ((u8*)&g_char_panel_data) - 4;
            var = g_char_panel_data;
            /* val16 from &g_char_panel_data + (f848*4) + 0x10 */
            val16 = *(u16*)((u8*)&g_char_panel_data + (f848 * 4) + 0x10);
            ptr = base + val16 + var;
            var_a0 = func_800A88A0(var_a0, arg1, ptr, 1, 0xb0, 0xc8, 2);
        }
        else
        {
            base = ((u8*)&g_char_panel_data) - 4;
            /* val16 from &g_char_panel_data + (arg0*4) + 0x50 */
            val16 = *(u16*)((u8*)&g_char_panel_data + (arg0 * 4) + 0x50);
            ptr = base + val16 + (arg0 * 4);
            var_a0 = func_800A88A0(var_a0, arg1, ptr, 1, 0xb0, 0xc8, 2);
        }
    }

    return var_a0;
}

/**
 * decomp.me (43.25%) https://decomp.me/scratch/0GHRZ
 */
s32 func_80141D64(void)
{
    u32 n;
    void* var_a2;

    n = g_char_panel;
    if (n < 4)
    {
        u32 t0;
        char* v1; /* pointer arithmetic, no constant folding */
        u16 h;

        t0 = g_char_panel_data;
        v1 = (char*)&g_char_panel_data - 4; /* two addiu instructions */
        h = *(u16*)(v1 + (2 * n + t0));     /* lhu from (v1 + 2n + t0) */
        var_a2 = (void*)(t0 + (h + (u32)v1));
    }
    else
    {
        var_a2 = g_kanji_cat_name;
    }

    return func_800A88A0(var_a2, 1, 0x23, 0x47, 2);
}

/**
 * @brief Append three GPU primitives to the render context's OT and reserve a
 *        right-edge VRAM strip for upload on the back page.
 *
 * Builds, in order:
 *   1. A 0x40-byte template packet copied from the inactive frame's reserve
 *      slot at `g_render_buf_base + (alt_buf * 0x40C0) + 0x4064`.
 *   2. A textured sprite (tag 0x64) emitted via @ref func_800A88A0 using
 *      `tex_src` as its source data, then a Draw-Mode (GP0 0xE1) packet
 *      emitted via @ref emit_draw_mode_prim / @ref func_80142274.
 *   3. A 0x60-byte image-load packet built on the stack by
 *      @ref func_8001C56C describing a `strip_width x 32` rectangle at VRAM
 *      `(240 - strip_width, 24 | 256)` - i.e. right-aligned on whichever
 *      VRAM page is currently the back buffer (Y=0x18 vs 0x100).
 *
 * Each packet is spliced into the 24-bit OT at @c ctx->ot[0x0E] with the
 * standard `(top_byte | next_addr & 0xFFFFFF)` link idiom, and the heap
 * cursor @c ctx->prim_cursor is advanced by 0x40 bytes past the last packet.
 *
 * @param ctx         Render context: OT head at ot[0x0E], primitive heap cursor
 *                    at prim_cursor, double-buffer parity at frame_parity.
 * @param tex_src     Source data pointer for the sprite primitive (passed
 *                    through to func_800A88A0).
 * @param strip_width Width in pixels of the back-page VRAM upload strip; also
 *                    sets the strip's X position as `240 - strip_width`.
 *
 * @see https://decomp.me/scratch/LxujJ (99.26%)
 */
void func_80141E04(RenderContext* ctx, s32 tex_src, s32 strip_width)
{
    s32* ot_head;   /* &ctx->ot[0x0E] - passed as the OT head pointer */
    s32* prim;      /* current primitive being emitted */
    s32* next_prim; /* heap cursor after the sprite/draw-mode pair */
    s32 vram_y;     /* VRAM Y of the back page (0x18 or 0x100) */
    u32 load_packet[0x19];
    s32 vram_x; /* VRAM X of the right-aligned strip */

    ot_head = (s32*)&ctx->ot[0x0E];
    prim = ctx->prim_cursor;
    next_prim = prim;

    /* 1. Copy template packet from the *other* frame's reserve slot, then
     *    splice it into the OT. */
    func_8001A5D4(prim, (void*)(g_render_buf_base + ((ctx->frame_parity ^ 1) * 0x40C0) + 0x4064));

    *prim = (*prim & 0xFF000000) | (ctx->ot[0x0E] & 0xFFFFFF);
    ctx->ot[0x0E] = (ctx->ot[0x0E] & 0xFF000000) | ((u32)prim & 0xFFFFFF);

    /* 2. Emit textured sprite (tag 0x64) wrapped by a Draw-Mode (0xE1) packet.
     *    Returns the heap cursor just past both packets. */
    next_prim = emit_draw_mode_prim(func_80142274(func_800A88A0(prim + 0x10, ot_head, tex_src, 1, 0x10, 8, 0), ot_head, 2, 0, 0, 0, 0, 0), ot_head);

    /* 3. Build a back-page VRAM upload RECT (W = strip_width, H = 32) at the
     *    right edge of whichever page is currently the back buffer. */
    vram_x = 0xF0 - strip_width;
    vram_y = 0x18;
    if (ctx->frame_parity != 0)
    {
        vram_y = 0x100;
    }

    func_8001C56C(load_packet, vram_x, vram_y, strip_width, 0x20);
    func_8001A5D4(next_prim, load_packet);

    *next_prim = (*next_prim & 0xFF000000) | (ctx->ot[0x0E] & 0xFFFFFF);

    /* Advance heap cursor 0x40 bytes past the load packet. */
    ctx->prim_cursor = next_prim + 0x10;

    ctx->ot[0x0E] = (ctx->ot[0x0E] & 0xFF000000) | ((u32)next_prim & 0xFFFFFF);
}

/**
 * decomp.me (77.64%) https://decomp.me/scratch/Glw7t
 */
void func_80141F9C(void* arg0, s32 arg1)
{
    u8 sp28[0x80];
    RenderContext* obj = (RenderContext*)arg0;
    u32* temp_s1 = obj->prim_cursor;
    u32* var_a2;
    u8* var_s4;
    u16* var_s3;
    s32 var_s0, var_s1, var_s2, var_s5;
    s32 temp_v1;

    /* First call and pointer setup */
    func_8001A5D4(temp_s1, (void*)(g_render_buf_base + ((obj->frame_parity ^ 1) * 0x40C0) + 0x4064));

    *temp_s1 = (*temp_s1 & 0xFF000000) | (obj->ot[0x0A] & 0x00FFFFFF);
    obj->ot[0x0A] = (obj->ot[0x0A] & 0xFF000000) | ((u32)temp_s1 & 0x00FFFFFF);

    /* var_a2 = temp_s1 + 0x40 (byte addition) */
    var_a2 = (u32*)((u8*)temp_s1 + 0x40);

    /* Determine var_s4, var_s1, var_s5, var_s2 based on g_char_panel */
    if (g_char_panel == 4)
    {
        var_s4 = (u8*)(g_kanji_panel_data + ((u32)&g_kanji_panel_data - 8));
        var_s1 = g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat]];
        var_s5 = g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat] + 1];
        var_s2 = 0;
    }
    else
    {
        var_s1 = g_panel_char_offsets[arg1][0];
        var_s5 = g_panel_char_offsets[arg1][1];
        var_s4 = (u8*)(g_char_panel_data + ((u32)&g_char_panel_data - 4));
        var_s2 = 0;
    }

    var_s0 = var_s2;
    var_s3 = (u16*)(var_s4 + (var_s1 * 2));

    /* Main loop */
    for (;;)
    {
        temp_v1 = (var_s2 * 0x10) - g_scroll_pos;
        if ((u32)(temp_v1 + 0x0B) < 0x5B)
        {
            var_a2 = func_800A88A0(var_a2, (void*)&obj->ot[0x0A], (void*)(var_s4 + *var_s3), 1, var_s0 * 0x10, temp_v1, 0);
        }
        var_s1++;
        var_s3++;
        if (var_s5 == var_s1)
            break;

        var_s0++;
        if (var_s0 == 10)
        {
            var_s0 = 0;
            var_s2++;
        }
    }

    /* After loop - final setup and second call */
    {
        u32 param_a2 = 0x68;
        g_char_last_row = var_s2;
        g_char_last_col = var_s0;
        if (obj->frame_parity != 0)
        {
            param_a2 = 0x150;
        }

        func_8001C56C(sp28, 0x60, param_a2, 0xA0, 0x50);
        func_8001A5D4(var_a2, sp28);

        *var_a2 = (*var_a2 & 0xFF000000) | (obj->ot[0x0A] & 0x00FFFFFF);
        obj->ot[0x0A] = (obj->ot[0x0A] & 0xFF000000) | ((u32)var_a2 & 0x00FFFFFF);
        /* Byte addition of 0x40, not element addition */
        obj->prim_cursor = (u32*)((u8*)var_a2 + 0x40);
    }
}

/**
 * @brief Emit a Draw-Mode (GP0 0xE1) primitive and link it to the OT.
 *
 * Writes an 8-byte packet at @p prim:
 *  - byte 3 = 1 (one-word payload).
 *  - bytes 4..7 = `0xE1000005` (GP0 Draw Mode: texpage default, abr=1,
 *    dither off, drawing-to-display-area enabled).
 * Then splices the packet into the 24-bit OT whose head is at @p ot_head
 * using the standard `(top_byte | next_addr & 0xFFFFFF)` chain idiom and
 * returns the heap cursor advanced by 8 bytes.
 *
 * @param prim    Destination address for the 8-byte packet (heap cursor).
 * @param ot_head Pointer to the 24-bit OT head pointer.
 * @return Heap cursor advanced past the packet (`prim + 8`).
 *
 * @see https://decomp.me/scratch/EyVeo (100%)
 */
void* emit_draw_mode_prim(void* arg0, s32* arg1)
{
    unsigned char* bytes = (unsigned char*)arg0;
    u32* words = (u32*)arg0;
    u32 temp1, temp2;

    setDrawTPage(bytes, 0, 0, 0x05);
    addPrim(arg1, arg0);

    return (void*)(bytes + 8);
}

/**
 * @brief Emit 1 or 2 glyph SPRT primitives and chain them onto an OT tag.
 *
 * Always writes a primary white-tinted (RGB=0x80) SPRT for glyph @p arg2
 * at @c (arg3 - arg5 + arg6, arg4 - arg5 + arg6). When @p arg5 != 0,
 * writes a second SPRT at @c (arg3 + tmp, arg4 + tmp) with
 * @c tmp = (arg5 - arg6) * 2. The second sprite's tint and code byte
 * depend on @p arg7:
 *   - @p arg7 != 0: opaque blue (RGB=(0,0,0xA0), code 0x64) - highlight.
 *   - @p arg7 == 0: semi-transparent black (RGB=0, code 0x66) - drop shadow.
 *
 * Both sprites pull u/v/w/h/clut from @c g_glyph_table[arg2] and are
 * appended to the linked list at @p arg1 via the standard addPrim sequence.
 *
 * @param arg0 Pointer to the next free byte in the primitive buffer.
 * @param arg1 Pointer to the OT head tag (addPrim "ot" arg).
 * @param arg2 Glyph ID (index into @c g_glyph_table).
 * @param arg3 Base X coordinate.
 * @param arg4 Base Y coordinate.
 * @param arg5 Shadow/highlight offset distance. When 0, only the primary
 *             sprite is emitted (the second SPRT block is skipped).
 * @param arg6 Origin compensation (subtracted from base for the primary
 *             sprite; combined into @c tmp for the secondary).
 * @param arg7 Secondary-sprite mode: non-zero = opaque blue highlight,
 *             zero = semi-transparent black drop shadow.
 * @return Pointer to the byte after the last emitted primitive.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Au2h5
 */
void* func_80142274(void* arg0, s32* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7)
{
    u8* ptr = (u8*)arg0;
    SPRT* sprt = (SPRT*)ptr;

    /* (offset) + (base) form so gcc emits `addu v1,v1,v0` (vs the reverse
       order from `&g_glyph_table[arg2]`). Also keeps arg2 live for the
       second SPRT's re-derivation below. */
    u8* entry = (u8*)((arg2 << 3) + (u32)g_glyph_table);
    u32 clut_word;
    s32 tmp2;

    /* Primary glyph SPRT - white tint, fully opaque. */
    *(u32*)&sprt->r0 = 0x808080; /* r=g=b=0x80, code byte = 0 */
    setSprt(sprt);
    setXY0(sprt, (s16)(arg3 - arg5 + arg6), (s16)(arg4 - arg5 + arg6));
    setUV0(sprt, entry[0], entry[1]);
    setWH(sprt, entry[2], entry[3]);
    setClut(sprt, *(u32*)(entry + 4) << 4, 498);
    addPrim(arg1, sprt);
    ptr += sizeof(SPRT);

    if (arg5 != 0)
    {
        /* Secondary SPRT - drop shadow (arg7==0) or highlight (arg7!=0). */
        u8* new_var2;
        u8* entry2;
        u32 clut_word2;

        *(u32*)&((SPRT*)ptr)->r0 = (arg7 != 0) ? 0xA00000 : 0;

        setSprt((SPRT*)ptr);

        if (arg7 == 0)
        {
            setSemiTrans((SPRT*)ptr, 1);
        }

        tmp2 = (arg5 - arg6) * 2;

        new_var2 = (u8*)g_glyph_table;

        entry2 = (u8*)((arg2 << 3) + (u32)new_var2);

        setXY0((SPRT*)ptr, (s16)(arg3 + tmp2), (s16)(arg4 + tmp2));
        setUV0((SPRT*)ptr, entry2[0], entry2[1]);
        setWH((SPRT*)ptr, (s16)entry2[2], (s16)entry2[3]);
        setClut((SPRT*)ptr, *(u32*)(entry2 + 4) << 4, 498);
        addPrim(arg1, ptr);

        ptr += sizeof(SPRT);
    }

    return (void*)ptr;
}

/**
 * @brief Build the name-entry cursor's per-frame sprite packet:
 *        a TexWindow bracket around @ref NAME_CURSOR_GLYPH_COUNT textured
 *        glyph sprites, terminated by a DrawMode.
 *
 * Walks @c g_name_cursor_glyphs (a @ref GlyphSeqEntry array) one entry per glyph
 * cell, looks each glyph up in @c g_glyph_table, and emits a white
 * (RGB=0x80) free-size textured SPRT primitive (code 0x64). The chain is
 * wrapped with @c setTexWindow at both ends (rect @c {0,0,0xFF,0xFF} -
 * a no-op full-page window) and closed with @c setDrawTPage(0,0,5).
 * The buffer cursor at @c obj->prim_cursor is advanced past the final primitive.
 *
 * @param ctx Render context (@ref RenderContext). Reads/writes:
 *             - @c ot[0x0F] at offset 0x3C - addPrim OT entry.
 *             - @c prim_cursor at offset 0x4040 - primitive scratch-pool cursor.
 *
 * @note Called by @ref gname_tick via the implicit-@c $a0 convention
 *       (the call site passes no args; the function reads whatever the
 *       caller left in @c $a0).
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Q6WL2
 */
void draw_name_cursor_row(RenderContext* ctx)
{
    RECT tw_rect;

    s32 i;

    u8* ptr_t1;
    u8* list_ptr;
    DR_TWIN* twin;
    SPRT* sprt;
    u8* drawmode;
    u_long* ptr;
    GlyphSeqEntry* seq;

    /* Two aliases of the same context pointer: gcc allocates them to t6/t2
       and uses t6 for the very first addPrim/buf access and t2 for every
       subsequent addPrim. This split is load-bearing for the asm match. */
    RenderContext* obj = ctx;
    RenderContext* obj2;
    u8* glyph_table_base;
    obj2 = obj;

    ptr_t1 = (u8*)obj->prim_cursor;

    /* First TexWindow init: source order is h, w, y, x. */
    tw_rect.h = 0xFF;
    tw_rect.w = 0xFF;
    tw_rect.y = 0;
    tw_rect.x = 0;

    /* Opening texture window. The first addPrim runs *before* seq/i/glyph_table_base
       are assigned so gcc materializes the 0x00FFFFFF mask at the very top
       of the prologue (the mask is the first non-arg constant used). */
    twin = (DR_TWIN*)ptr_t1;
    setTexWindow(twin, &tw_rect);
    addPrim(&obj->ot[0x0F], twin);

    seq = g_name_cursor_glyphs;
    i = 0;
    glyph_table_base = (u8*)g_glyph_table;

    ptr_t1 += sizeof(DR_TWIN);

    /* Glyph sprites. list_ptr is a separate variable aliased to ptr_t1 so
       gcc keeps both pointers live across the loop (target uses t1 + a2 in
       parallel). */
    list_ptr = ptr_t1;
    do
    {
        u32 idx = seq->id;
        u32 xy;
        u8* glyph;

        sprt = (SPRT*)list_ptr;
        /* RGB only (high byte lands in `code`, immediately overwritten). */
        *(u32*)&sprt->r0 = 0x808080;
        setlen(sprt, 4);
        setcode(sprt, 0x64);

        xy = seq->xy;
        /* (offset) + (base) order forces gcc to emit `addu v1,v1,s0` (vs.
           the reverse order `s0,v1` you'd get from `&glyph_table[idx]`). */
        glyph = (u8*)((idx << 3) + (u32)glyph_table_base);
        *(u32*)&sprt->x0 = xy;

        sprt->u0 = glyph[0];
        sprt->v0 = glyph[1];
        sprt->w = glyph[2];
        {
            /* `i++` between the load and store of `h` so gcc schedules the
               counter increment into the slot the target asm uses. */
            u8 hh = glyph[3];
            i++;
            sprt->h = hh;
        }
        {
            /* Read clut as a full word (gcc would otherwise optimize this
               to `lhu` since only the low 16 bits affect the result). The
               `seq++` advance sits between read and store to match the
               target's instruction scheduling. */
            u32 clut_word = *(u32*)(glyph + 4);
            seq++;
            sprt->clut = (u16)((clut_word & GLYPH_CLUT_X_MASK) | GLYPH_CLUT_PAGE_BITS);
        }

        addPrim(&obj2->ot[0x0F], sprt);
        list_ptr += sizeof(SPRT);
    } while (i < NAME_CURSOR_GLYPH_COUNT);
    ptr_t1 = list_ptr;

    /* Closing texture window. Field-assignment order differs from the
       opening call (w,h,x,y vs. h,w,y,x) - the original C wrote them in
       this exact order and gcc preserves it. */
    tw_rect.w = 0xFF;
    tw_rect.h = 0xFF;
    tw_rect.x = 0;
    tw_rect.y = 0;
    twin = (DR_TWIN*)ptr_t1;
    setTexWindow(twin, &tw_rect);
    addPrim(&obj2->ot[0x0F], twin);
    ptr_t1 += sizeof(DR_TWIN);

    /* DrawMode terminator: tpage 5, dfe=0, dtd=0. Writes only tag + 1 word.
       The buffer cursor is computed as `drawmode + 8` (not by mutating
       ptr_t1 first), which yields `addiu v0,t1,8; sw v0,0x4040(...)`. */
    drawmode = ptr_t1;
    setDrawTPage(drawmode, 0, 0, 5);
    addPrim(&obj2->ot[0x0F], drawmode);

    ctx->prim_cursor = (u32*)(drawmode + 8);
}

/**
 * @brief Number of bytes in a name buffer, excluding the null terminator.
 *
 * Walks the variable-width encoding: each byte in [0x19, 0x20) consumes
 * two buffer bytes (DBCS lead + trail), every other byte consumes one.
 *
 * @param name Null-terminated name buffer.
 * @return Byte length excluding the terminator.
 * @see https://decomp.me/scratch/2QgjW (100%)
 */
s32 name_byte_length(u8* name)
{
    s32 byte_len;
    u8 c;
    u8* p;

    p = name;
    c = *p;
    byte_len = 0;
    if (c != 0)
    {
        do
        {
            if ((u32)(c - 0x19) < 7U) /* DBCS lead byte: 2-byte glyph */
            {
                p += 2;
                byte_len += 2;
            }
            else
            {
                p += 1;
                byte_len += 1;
            }
            c = *p;
        } while (c != 0);
    }
    return byte_len;
}

/**
 * @brief Number of logical glyphs in a name buffer.
 *
 * Like @ref name_byte_length but counts each DBCS pair as one glyph.
 *
 * @param name Null-terminated name buffer.
 * @return Glyph (character) count.
 * @see https://decomp.me/scratch/c8fPe (100%)
 */
s32 name_char_count(const u8* name)
{
    s32 char_count = 0;

    while (*name)
    {
        name += IS_DBSC_LEAD_BYTE(*name) ? 2 : 1;
        char_count++;
    }

    return char_count;
}

/**
 * @brief Append @p src to the end of @p dst (in-place concatenation).
 *
 * Computes the byte lengths of both buffers (respecting the DBCS-style
 * encoding) and copies @p src's payload after @p dst's existing payload,
 * writing a fresh null terminator. Caller is responsible for ensuring
 * @p dst has room for both.
 *
 * @param dst Null-terminated name buffer; appended to in-place.
 * @param src Null-terminated source name to append.
 * @see https://decomp.me/scratch/1lsbD (100%)
 */
void name_append(u8* dst, const u8* src)
{
    const u8* p;
    s32 dst_len;
    s32 src_len;
    s32 offset;
    s32 i;

    p = dst;
    dst_len = 0;

    while (*p)
    {
        if (IS_DBSC_LEAD_BYTE(*p))
        {
            p += 2;
            dst_len += 2;
        }
        else
        {
            p += 1;
            dst_len += 1;
        }
    }

    p = src;
    src_len = 0;
    offset = dst_len;

    while (*p)
    {
        if (IS_DBSC_LEAD_BYTE(*p))
        {
            p += 2;
            src_len += 2;
        }
        else
        {
            p += 1;
            src_len += 1;
        }
    }

    for (i = 0; i < src_len; i++)
    {
        dst[offset + i] = src[i];
    }

    dst[offset + i] = 0;
}

/**
 * @brief Remove the last glyph from @p name and return it.
 *
 * Walks the buffer keeping a one-glyph-behind pointer; on exit @c prev_pos
 * points at the last glyph and @c scan_pos at the null. The returned
 * @c s32 packs the glyph as `lead | (trail << 8)` for a DBCS pair, or just
 * the byte value for a 1-byte glyph. The buffer is truncated by writing
 * 0 at @c prev_pos. Empty names return 0 unchanged.
 *
 * @param name Null-terminated name buffer (truncated in-place).
 * @return Removed glyph packed as `lead | (trail << 8)`, or 0 if empty.
 * @see https://decomp.me/scratch/agZ8y (100%)
 */
s32 name_pop_last_char(u8* name)
{
    u8* prev_pos;
    u8* scan_pos;
    s32 last_char;

    prev_pos = name;
    scan_pos = prev_pos;

    while (*scan_pos)
    {
        prev_pos = scan_pos;
        if (IS_DBSC_LEAD_BYTE(*scan_pos))
        {
            scan_pos += 2;
        }
        else
        {
            scan_pos++;
        }
    }

    last_char = MAKE_DBCS_GLYPH(prev_pos[0], prev_pos[1]);

    if (prev_pos != scan_pos)
    {
        *prev_pos = 0;
    }

    return last_char;
}

/**
 * @brief Copy @p src into @p dst, including the null terminator.
 *
 * Computes the byte length of @p src walking the DBCS-style encoding, then
 * copies that many bytes and writes a terminator. Caller must ensure
 * @p dst has room.
 *
 * @param dst Destination name buffer.
 * @param src Null-terminated source name.
 * @see https://decomp.me/scratch/UeYRe (100%)
 */
void name_copy(u8* dst, const u8* src)
{
    const u8* p;
    s32 i;
    s32 src_len;

    p = src;
    src_len = 0;

    while (*p)
    {
        if (IS_DBSC_LEAD_BYTE(*p))
        {
            p += 2;
            src_len += 2;
        }
        else
        {
            p += 1;
            src_len += 1;
        }
    }

    for (i = 0; i < src_len; i++)
    {
        dst[i] = src[i];
    }

    dst[i] = 0;
}

/**
 * @brief Recompute the active name's rendered pixel width and strip target.
 *
 * Measures every glyph of @c g_active_name via @c func_800644FC (which
 * fills one @ref GlyphMeasure per glyph and returns the glyph count), sums
 * the per-glyph widths into @c g_name_pixel_width, then sets
 * @c g_strip_width_target to that width plus a fixed 0x18 margin. Called
 * whenever the name buffer changes (append, delete, reset).
 *
 * @see https://decomp.me/scratch/y0CgJ (100%)
 */
void recalc_name_width(void)
{
    GlyphMeasure glyphs[16];
    GlyphMeasure* cursor;
    GlyphMeasure* entry;
    s16 width;
    s32 glyph_count;
    s32 i;

    glyph_count = func_800644FC(glyphs, g_active_name, 0);
    i = 0;
    width = entry->width;
    g_name_pixel_width = 0;

    if (i < glyph_count)
    {
        cursor = glyphs;
        while (i < glyph_count)
        {
            entry = cursor;
            width = entry->width;
            g_name_pixel_width += width;
            cursor++;
            i++;
        }
    }

    g_strip_width_target = g_name_pixel_width + 0x18;
}

/**
 * @brief Insert a glyph at the front of @p buffer (in-place).
 *
 * @p new_char packs the glyph as `lead | (trail << 8)`; the lead byte
 * decides whether it is a 1- or 2-byte glyph. The existing buffer
 * contents (including the null terminator) are shifted right by that
 * amount and the new glyph is written at offset 0. Caller must ensure
 * @p buffer has room.
 *
 * No-op if the lead byte is 0.
 *
 * @param buffer   Null-terminated name buffer.
 * @param new_char Glyph to prepend, packed `lead | (trail << 8)`.
 * @see https://decomp.me/scratch/VOLcD (100%)
 */
void name_prepend_char(u8* buffer, u16 header)
{
    u8* ptr;
    u32 len;
    u32 header_size;
    u32 move_count;
    u32 i;
    u16 h = header; // save header to match register usage

    if ((h & 0xFF) == 0)
        return;

    if (IS_DBSC_LEAD_BYTE(h & 0xFF))
    {
        header_size = 2;
    }
    else
    {
        header_size = 1;
    }

    ptr = buffer;
    len = 0;

    while (*ptr != 0)
    {

        if (IS_DBSC_LEAD_BYTE(*ptr))
        {
            ptr += 2;
            len += 2;
        }
        else
        {
            ptr += 1;
            len += 1;
        }
    }

    move_count = len + 1;
    for (i = move_count; i > 0; i--)
    {
        buffer[(header_size + i) - 1] = buffer[i - 1];
    }

    buffer[0] = (u8)(h & 0xFF);
    if (header_size == 2)
    {
        buffer[1] = (u8)(h >> 8);
    }
}

/**
 * @brief Remove the first glyph from @p name and return it.
 *
 * Reads one glyph at the head of the buffer (1 or 2 bytes per the
 * DBCS-style encoding), measures the rest of the buffer's byte length,
 * shifts the remaining bytes (plus null terminator) left by the glyph
 * size, and returns the removed glyph packed as `lead | (trail << 8)`.
 *
 * @param name Null-terminated name buffer (mutated in-place).
 * @return Removed glyph packed in low 16 bits, or 0 if @p name was empty.
 * @see https://decomp.me/scratch/ArXXq (100%)
 */
s32 name_pop_first_char(u8* name)
{
    u8 first;
    u32 width;
    u16 first_char;
    u8* p;
    s32 tail_len;
    s32 move_count;
    s32 i;
    u32 mask_u16;

    first = name[0];

    if (first == 0)
    {
        return 0;
    }

    if (IS_DBSC_LEAD_BYTE(first))
    {
        first_char = MAKE_DBCS_GLYPH(name[0], name[1]);
        width = 2;
    }
    else
    {
        first_char = name[0];
        width = 1;
    }

    /* Measure tail (everything after the first glyph) in bytes. */
    tail_len = 0;
    p = name + width;

    while (*p != 0)
    {
        if (IS_DBSC_LEAD_BYTE(*p))
        {
            p += 2;
            tail_len += 2;
        }
        else
        {
            p += 1;
            tail_len += 1;
        }
    }

    move_count = tail_len + 1; /* +1 to also shift the null terminator */
    mask_u16 = 0xFFFFU;
    for (i = 0; i < move_count; i++)
    {
        name[i] = name[i + width];
    }

    return (s32)(first_char & mask_u16);
}

/**
 * @brief Draw the current frame of the character-append animation and
 *        advance its frame timer.
 *
 * When the player commits a glyph into the name being entered, the input
 * handler seeds @c g_append_anim_frame to 0 and @c g_append_anim_timer to 2
 * (see the @ref name_append call sites). This routine then runs once per
 * render tick from @ref gname_render:
 *
 *  1. Draw: emit up to @ref APPEND_ANIM_SLOT_COUNT textured-glyph SPRTs for
 *     the current frame. The frame index @c g_append_anim_frame selects a
 *     record in @c g_char_append_anim (logically an @ref AppendAnimFrame).
 *     Each glyph slot gives an (x, y, glyph) triple; a slot whose glyph id
 *     is 0 is skipped. X is biased by 0xE8, Y by 4.
 *  2. Advance: decrement @c g_append_anim_timer; when it reaches 0, step to
 *     the next frame. Frame @ref APPEND_ANIM_FRAME_COUNT wraps back to 0 and
 *     stops the animation (timer left at 0); otherwise the new frame's
 *     duration is loaded from byte 3 of its record (@c slots[0].pad).
 *
 * @param prim Primitive-buffer cursor (next free byte).
 * @param ctx  Render-context base; the OT head tag for this layer is at
 *             offset 0x30.
 * @return Primitive-buffer cursor advanced past the emitted SPRTs.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/3TQG6
 */
s32 draw_char_append_anim(s32 prim, s32 ctx)
{
    u8 frame = g_append_anim_frame;
    s32 result = prim;
    u8* table;
    s32 ot_base = ctx;
    s32 i;
    /* px points at a slot's x byte; py = px + 1 reads y at [0] and glyph at
       [1]. The two incrementing pointers are load-bearing for the match. */
    u8* px = &g_char_append_anim[frame * APPEND_ANIM_FRAME_STRIDE];
    u8* py = px + 1;
    short glyph;
    for (i = 0; i < APPEND_ANIM_SLOT_COUNT; i++, py += 4, px += 4)
    {
        s32 glyph_byte = py[1];
        glyph = glyph_byte;
        if (glyph != 0)
        {
            result = func_80142274(result, ot_base + 0x30, (u8)glyph, px[0] + 0xE8, py[0] + 4, 0, 0, 0);
        }
    }

    if (g_append_anim_timer != 0)
    {
        g_append_anim_timer--;
        if (g_append_anim_timer == 0)
        {
            g_append_anim_frame++;
            if (g_append_anim_frame == APPEND_ANIM_FRAME_COUNT)
            {
                g_append_anim_frame = 0;
                g_append_anim_timer = 0;
                return result;
            }
            else
            {
                table = g_char_append_anim;
                g_append_anim_timer = table[(g_append_anim_frame * APPEND_ANIM_FRAME_STRIDE) + 3];
            }
        }
    }
    return result;
}

/**
 * @brief Test whether a name buffer is empty or contains only blanks.
 *
 * Walks @p name byte-by-byte (note: not glyph-by-glyph). The buffer is
 * blank if every byte is either ASCII space (@ref CHAR_SPACE) or the
 * wide-space sentinel (@ref CHAR_WIDE_SPACE). An empty (immediate-null)
 * buffer also counts as blank.
 *
 * @param name Null-terminated name buffer.
 * @return 1 if blank, 0 otherwise.
 * @see https://decomp.me/scratch/rdbBA (100%)
 */
s32 name_is_blank(u8* name)
{
    while (*name != 0)
    {
        if (*name != CHAR_SPACE && *name != CHAR_WIDE_SPACE)
        {
            return FALSE;
        }

        name++;
    }

    return TRUE;
}