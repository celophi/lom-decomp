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
 * @param ctx Render context (@ref RenderContext) passed through to @ref gname_render.
 *
 * @see https://decomp.me/scratch/yYkTM (100%)
 */
void gname_tick(RenderContext* ctx)
{
    draw_name_cursor_row();
    gname_render(ctx);
    g_frame_counter++;
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
 *  - When @c g_pad_input == @c PAD_BTN_START (only START held), plays
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
    if (g_pad_input == PAD_BTN_START)
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
    name_copy(g_active_name, &g_initial_name);
    g_strip_width = 0;
    recalc_name_width();
    g_strip_width_steps = 5;
    g_append_anim_frame = 0;
    g_append_anim_timer = 2;
    g_char_panel = 0;
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
 * @see decomp.me (99.78%) https://decomp.me/scratch/PpqVd
 */
s32 handle_char_set_input(s32 arg0, s32 arg1)
{
    s32 var_s0 = 0xFF;

    while (var_s0 == 0xFF)
    {
        switch (arg0)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            if (arg1 & 0x220)
            {
                g_cursor_tab = arg0;
                switch (arg0)
                {
                case 0:
                    if ((name_char_count(g_active_name) != 0) && (!name_is_blank(g_active_name)))
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
                        name_copy(g_active_name, ((g_random_names_off - 0x10) + (*((u32*)g_random_names_off))) +
                                                     (*((u16*)(((g_random_names_off - 0x10) + (*((u32*)g_random_names_off))) + ((rand() % 128) * 2)))));
                    }
                    else if (g_name_source_mode == 5)
                    {
                        g_name_clipboard = 0;
                        name_copy(g_active_name, ((g_random_names_off - 0x10) + (*((u32*)g_random_names_off))) +
                                                     (*((u16*)(((g_random_names_off - 0x10) + (*((u32*)g_random_names_off))) + (((rand() % 128) + 128) * 2)))));
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
                            name_copy(g_active_name,
                                      ((g_random_names_off - 0x10) + (*((u32*)g_history_names_off))) +
                                          (*((u16*)(((g_random_names_off - 0x10) + (*((u32*)g_history_names_off))) + (g_history_name_idx * 2)))));
                            name_append(g_active_name,
                                        ((g_random_names_off - 0x10) + ((*((u32*)g_history_names_off)))) +
                                            (*((u16*)(((g_history_names_off - 0x10) + (*((u32*)g_history_names_off))) + (((rand() % 128) + 130) * 2)))));
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
                    recalc_name_width();
                    var_s0 = 0;
                    g_strip_width_steps = 5;
                    continue;

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
                if (arg1 != 0)
                {
                    if (arg1 & PAD_BTN_DOWN)
                    {
                        arg0 = 0x10;
                        arg1 = 0;
                        continue;
                    }
                    if (arg1 & PAD_BTN_LEFT)
                    {
                        arg0 = (arg0 == 0) ? (3) : (arg0 - 1);
                    }
                    else if (arg1 & PAD_BTN_RIGHT)
                    {
                        arg0 = (arg0 < 3) ? (arg0 + 1) : (0);
                    }
                }
                play_menu_sfx(0x7D, 0x80);
                g_cursor_x_target = g_tab_cursor_pos[arg0 + 2].x - 8;
                g_cursor_y_target = g_tab_cursor_pos[arg0 + 2].y;
                g_cursor_lerp_steps = 5;
                var_s0 = 0;
            }
            break;

        case 4:
        case 5:
        case 6:
        case 7:
            if (((arg1 & 0x220) && ((g_cursor_tab = arg0, g_char_panel != (arg0 - 4)))) != 0)
            {
                g_char_panel = g_cursor_tab - 4;
                arg0 = 0x10;
                arg1 = 0;
                g_scroll_target = 0;
                g_scroll_pos = 0;
                g_scroll_steps = 0;
                g_char_cursor = 0;
                play_menu_sfx(0x7E, 0x80);
                continue;
            }
            else
            {
                if (arg1 != 0)
                {
                    if (arg1 & PAD_BTN_RIGHT)
                    {
                        arg0 = 0x10;
                        arg1 = 0;
                        continue;
                    }
                    if (arg1 & PAD_BTN_UP)
                    {
                        arg0 = (arg0 == 4) ? (6) : (arg0 - 1);
                    }
                    else if (arg1 & PAD_BTN_DOWN)
                    {
                        arg0 = (arg0 < 6) ? (arg0 + 1) : (4);
                    }
                }
                play_menu_sfx(0x7D, 0x80);
                g_cursor_x_target = g_tab_cursor_pos[arg0 + 2].x - 8;
                g_cursor_y_target = g_tab_cursor_pos[arg0 + 2].y;
                g_cursor_lerp_steps = 5;
                var_s0 = 0;
            }
            break;

        default:
            if (((arg1 & 0x220) && (((g_char_last_row * 10) + g_char_last_col) >= g_char_cursor)) != 0U)
            {
                if (g_char_panel < 3)
                {
                    if (name_char_count(g_active_name) < 10)
                    {
                        u8* argA;
                        g_append_anim_timer = 2;
                        argA = ((g_random_names_off - 0x10) + g_panel_tbl_off) +
                               (*((u16*)((((g_random_names_off - 0x10) + g_panel_tbl_off) + (g_panel_char_offsets[g_char_panel] * 2)) + (g_char_cursor * 2))));
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
                    g_kanji_cat_name = ((g_random_names_off - 0x10) + g_panel_tbl_off) +
                                       (*((u16*)((((g_random_names_off - 0x10) + g_panel_tbl_off) + (g_kanji_cat_names_offset * 2)) + (g_kanji_cat * 2))));
                    play_menu_sfx(0x7E, 0x80);
                }
                else if (g_char_panel == 4)
                {
                    if (name_char_count(g_active_name) < 10)
                    {
                        u8* argA;
                        g_append_anim_timer = 2;
                        argA = ((g_random_names_off - 0x10) + ((u32)g_kanji_panel_off)) +
                               (*((u16*)((((g_random_names_off - 0x10) + ((u32)g_kanji_panel_off)) +
                                          (g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat]] * 2)) +
                                         (g_char_cursor * 2))));
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
                s32 scroll_off;

                if (arg1 != 0)
                {
                    if ((arg1 & PAD_BTN_UP) && ((g_char_cursor / 10) == 0))
                    {
                        arg0 = 0;
                        arg1 = 0;
                        continue;
                    }
                    if ((arg1 & PAD_BTN_LEFT) && ((g_char_cursor % 10) == 0))
                    {
                        arg0 = 4;
                        arg1 = 0;
                        continue;
                    }
                    if ((arg1 & PAD_BTN_UP) && ((g_char_cursor / 10) != 0))
                    {
                        g_char_cursor -= 10;
                    }
                    else if ((arg1 & PAD_BTN_DOWN) && ((g_char_cursor / 10) != g_char_last_row))
                    {
                        g_char_cursor += 10;
                    }
                    else if ((arg1 & PAD_BTN_LEFT) && ((g_char_cursor % 10) != 0))
                    {
                        g_char_cursor -= 1;
                    }
                    else
                    {

                        if ((arg1 & PAD_BTN_RIGHT) && ((g_char_cursor % 10) != 9))
                        {
                            g_char_cursor += 1;
                        }
                        else
                        {
                            var_s0 = 0;
                            continue;
                        }
                    }
                }

                play_menu_sfx(0x7D, 0x80);
                g_cursor_x_target = ((g_char_cursor % 10) * 0x10) + 0x54;
                g_cursor_y_target = ((g_char_cursor / 10) * 0x10) + 0x68 - g_scroll_pos;

                if (g_cursor_y_target < 0x68)
                {
                    g_cursor_y_target = 0x68;
                    g_scroll_target = (g_char_cursor / 10) * 0x10;
                    g_scroll_steps = 4;
                }

                if (g_cursor_y_target >= 0xA9)
                {
                    g_cursor_y_target = 0xA8;
                    g_scroll_target = ((g_char_cursor / 10) * 0x10) - 0x40;
                    g_scroll_steps = 4;
                }

                g_cursor_lerp_steps = 4;
                var_s0 = 0;
            }
            break;
        }
    }

    return arg0;
}

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
 * @see decomp.me (96.22%) https://decomp.me/scratch/ctu1w
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
                    base = g_panel_data_base;
                    idx = g_kanji_cat;
                    offset = (idx * 2) + ((kanji_panel_offset * 2) + g_panel_tbl_off);
                    kanji_name_tbl_off = *((u16*)(base + offset));
                    g_pad_input &= ~GNAME_BTN_KANJI_NAV;
                    g_kanji_cat_name = (void*)((g_panel_tbl_off + kanji_name_tbl_off) + ((unsigned long)base));
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
 *     drop-shadowed glyph SPRT for each via @ref emit_glyph_sprt into OT entry
 *     @c ot[0x0B] (offset 0x2C). The entry whose index equals @c g_cursor_tab
 *     is drawn highlighted (the @p primary_adj flag of @ref emit_glyph_sprt), marking
 *     the current selection.
 *  2. Append decoration: emit a static glyph plus the character-append
 *     animation (@ref draw_char_append_anim) into @c ot[0x0D] (offset 0x34),
 *     then run @ref emit_panel_tab_sprite.
 *  3. Text cursor: build a white textured SPRT (glyph @c g_glyph_table[0xA0])
 *     at the cursor position (@c g_cursor_x, @c g_cursor_y), followed by an
 *     additive DrawMode, and chain both into @c ot[0x08] (offset 0x20).
 *  4. Conditional glyphs: when @c g_scroll_pos is set, and again when
 *     @c g_char_last_row >= 5 passes a phase check, emit extra glyphs from the
 *     @c g_tab_cursor_pos table.
 *  5. Hand off to the remaining sub-passes @ref emit_panel_label,
 *     @ref render_char_panel and @ref render_name_strip.
 *
 * @param ctx Render context. Uses OT entries at byte offsets
 *            0x20/0x24/0x2C/0x34 and the heap cursor at 0x4040.
 *
 * @note Control flow is left verbatim; the manual OT-splice bit-masking is
 *       load-bearing for the 82.68% match and must not be replaced with addPrim.
 * @see decomp.me (82.68%) https://decomp.me/scratch/rQBi6
 */
void gname_render(RenderContext* ctx)
{
    s32 cursor_x;
    s32 i;
    s32 scroll_pos;
    s32* entry;
    void* prim;
    char* ctx_bytes;
    void* cursor_sprite;
    void* prim2;
    RenderContext* ctx2;
    unsigned char* glyph_ptr;
    char* tmp_ptr;
    entry = (s32*)g_tab_cursor_entries;
    i = 2;
    glyph_ptr = (unsigned char*)g_tab_cursor_entries + 2;
    prim = ctx->prim_cursor;
    /* 1. Character grid: entries 2..12, skip 9; highlight the selection. */
    do
    {
        if (i != 9)
        {
            prim = emit_glyph_sprt(prim, ((char*)ctx) + 0x2C, glyph_ptr[1], (*entry) & 0x1FF, ((s32)glyph_ptr[0]) - 8, 1, (i - 2) == g_cursor_tab, 0);
        }
        i += 1;
        glyph_ptr += 4;
        entry++;
    } while (i < 0xD);
    cursor_x = g_cursor_x;
    ctx2 = ctx;
    /* 2. Static glyph + append animation, then panel-tab sprite. */
    cursor_sprite = emit_panel_tab_sprite(
        emit_draw_mode_prim(
            draw_char_append_anim(emit_glyph_sprt(emit_draw_mode_prim(prim, ((char*)ctx2) + 0x2C), ((char*)ctx2) + 0x34, (u8)3, 0xE8, 4, 0, 0, 0), ctx),
            ((char*)ctx2) + 0x34),
        &ctx->ot[0]);
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
        scroll_pos = g_scroll_pos;
        *((s32*)(ctx_bytes + 0x20)) = ((*((s32*)(ctx_bytes + 0x20))) & 0xFF000000) | (((s32)drawmode) & 0xFFFFFF);
    }
    tmp_ptr = ctx_bytes + 0x4040;
    /* 4. Conditional extra glyphs from the g_tab_cursor_pos table. */
    if (g_scroll_pos != 0)
    {
        prim2 = emit_glyph_sprt(prim2, &ctx->ot[0], g_tab_cursor_pos[0].glyph, g_tab_cursor_pos[0].x, (s32)g_tab_cursor_pos[0].y, 0, 0, 0);
    }
    if (g_char_last_row >= 5)
    {
        scroll_pos = g_scroll_pos;
        if (scroll_pos < 0)
        {
            scroll_pos += 0xF;
        }
        if ((((scroll_pos >> 1) >> 1) >> 2) != (g_char_last_row - 4))
        {
            prim2 = emit_glyph_sprt(prim2, &ctx->ot[0], g_tab_cursor_pos[1].glyph, g_tab_cursor_pos[1].x, (s32)g_tab_cursor_pos[1].y, 0, 0, 0);
        }
    }
    /* 5. Remaining sub-passes. */
    *((void**)tmp_ptr) = emit_panel_label(emit_draw_mode_prim(prim2, &ctx->ot[0]), (u_long*)(ctx_bytes + 0x24));
    render_char_panel(ctx, g_char_panel);
    render_name_strip(ctx2, g_active_name, g_strip_width);
}

/**
 * @brief Emit the panel-tab indicator sprite for the current character-set mode.
 *
 * Resolves a sprite record from the character panel data blob (see
 * @ref PanelDataHeader / @ref PANEL_DATA_BLOB) and forwards it to
 * @ref func_800A88A0 at fixed screen position (0xB0, 0xC8). A record is
 * always @c blob + tbl_off + table[i], where @c tbl_off is
 * @ref g_panel_tbl_off and @c table is @ref g_panel_record_offsets.
 *
 * Table index per mode:
 *  - mode 0-7 (kana/alpha): i = sprite_idx of the @ref g_tab_cursor_pos
 *    entry for tab (mode + 2)
 *  - mode 0x10, panel 3-4:  i = panel + 10 (entries 13-14)
 *  - mode 0x10, other:      i = 12
 *
 * All three branches share one compiled call tail that adds @c tbl_off last
 * (in the jal delay slot), which is why every call passes
 * @c sprite_data + tbl_off instead of folding the add earlier.
 *
 * @param prim     Primitive write cursor (linked-list head).
 * @param ot_entry Pointer into the render context OT for chaining.
 * @return Updated primitive write cursor after appending the sprite.
 * @see decomp.me (100%) https://decomp.me/scratch/RnoNS
 */
void* emit_panel_tab_sprite(void* prim, u_long* ot_entry)
{
    s32 mode = g_char_set_mode;

    if (g_char_set_mode < 8)
    {
        prim = func_800A88A0(prim, ot_entry, PANEL_RECORD(g_tab_cursor_pos[mode + 2].sprite_idx), 1, 0xB0, 0xC8, 2);
    }
    else if (g_char_set_mode == 0x10)
    {
        s32 panel = g_char_panel;

        if ((u32)(panel - 3) < 2U)
        {
            prim = func_800A88A0(prim, ot_entry, PANEL_RECORD(panel + 10), 1, 0xB0, 0xC8, 2);
        }
        else
        {
            prim = func_800A88A0(prim, ot_entry, PANEL_RECORD(12), 1, 0xB0, 0xC8, 2);
        }
    }
    return prim;
}

/**
 * @brief Emit the category-label sprite for the current character panel.
 *
 * For panels 0-3 resolves the label data from @ref g_panel_tbl_off using a
 * packed u16 offset; for panel >= 4 (kanji) uses @ref g_kanji_cat_name
 * directly. Forwards the pointer to @ref func_800A88A0 at fixed screen
 * position (0x23, 0x47).
 *
 * @return Updated primitive write cursor after appending the label sprite.
 * @see decomp.me (100%) https://decomp.me/scratch/jK7bc
 */
void* emit_panel_label(void* prim, u_long* ot_entry)
{
    s32 panel = g_char_panel;

    if (panel < 4)
    {
        prim = func_800A88A0(prim, ot_entry, PANEL_RECORD(panel), 1, 0x23, 0x47, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot_entry, g_kanji_cat_name, 1, 0x23, 0x47, 2);
    }

    return prim;
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
 *      emitted via @ref emit_draw_mode_prim / @ref emit_glyph_sprt.
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
 * @param name_buf    Active name buffer passed as the source data for the
 *                    sprite primitive (forwarded to func_800A88A0).
 * @param strip_width Width in pixels of the back-page VRAM upload strip; also
 *                    sets the strip's X position as `240 - strip_width`.
 *
 * @see https://decomp.me/scratch/LxujJ (100%)
 */
void render_name_strip(RenderContext* ctx, s32 name_buf, s32 strip_width)
{
    s32* ot_head;   /* &ctx->ot[0x0E] - passed as the OT head pointer */
    s32* prim;      /* current primitive being emitted */
    s32* next_prim; /* heap cursor after the sprite/draw-mode pair */
    s32 vram_y;     /* VRAM Y of the back page (0x18 or 0x100) */
    s32 vram_x;     /* VRAM X of the right-aligned strip */
    u32* pkt;
    u32 vram_load_pkt[0x19];

    ot_head = (s32*)&ctx->ot[0x0E];
    prim = ctx->prim_cursor;
    next_prim = prim;

    /* 1. Copy template packet from the *other* frame's reserve slot, then
     * splice it into the OT. */
    func_8001A5D4(prim, (void*)(g_render_buf_base + ((ctx->frame_parity ^ 1) * 0x40C0) + 0x4064));

    addPrim(&ctx->ot[0x0E], prim);
    
    /* 2. Emit textured sprite (tag 0x64) wrapped by a Draw-Mode (0xE1) packet.
     * Returns the heap cursor just past both packets. */
    next_prim = emit_draw_mode_prim(emit_glyph_sprt(func_800A88A0(prim + 0x10, ot_head, name_buf, 1, 0x10, 8, 0), ot_head, 2, 0, 0, 0, 0, 0), ot_head);

    /* 3. Build a back-page VRAM upload RECT (W = strip_width, H = 32) at the
     * right edge of whichever page is currently the back buffer. */
    pkt = vram_load_pkt + 2;
    vram_x = 0xF0 - strip_width;
    vram_y = 0x18;
    if (ctx->frame_parity != 0)
    {
        vram_y = 0x100;
    }

    func_8001C56C(pkt, vram_x, vram_y, strip_width, 0x20);
    func_8001A5D4(next_prim, pkt);
    
    addPrim(&ctx->ot[0x0E], next_prim);
    /* Advance heap cursor 0x40 bytes past the load packet. */
    next_prim += 0x10;
    ctx->prim_cursor = next_prim;
}

/**
 * @brief Render all visible character glyphs for the active panel into the OT.
 *
 * Splices a template packet into @c OT[0x0A], then walks every glyph entry in
 * the current panel (or kanji category when @c g_char_panel == 4), emitting a
 * sprite for each one whose scroll-adjusted Y position falls within the visible
 * grid window. After the loop, records the final grid position in
 * @c g_char_last_row / @c g_char_last_col, then appends a VRAM upload RECT
 * covering the full @c NAME_GRID_VIS_HEIGHT x @c 0xA0 grid area.
 *
 * Panel data sources:
 *  - @c g_char_panel == 4 (kanji picker): glyph entries come from
 *    @c g_kanji_panel_off, bounded by
 *    @c g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat]] .. [..+1].
 *  - Otherwise: glyph entries come from @c g_panel_tbl_off, bounded by
 *    @c g_panel_char_offsets[panel_idx][0] .. [1].
 *
 * Visibility culling: a glyph at row @c r with @c g_scroll_pos is visible when
 * @c (u32)((r*16 - g_scroll_pos) + 11) < 91, i.e. its screen Y is in [-11, 79].
 * Glyphs are emitted at @c x = col*NAME_GRID_CELL_SIZE,
 * @c y = row*NAME_GRID_CELL_SIZE - g_scroll_pos.
 *
 * The final VRAM rect is at (@c NAME_GRID_VRAM_X, @c NAME_GRID_Y_TOP) on the
 * back buffer page (@c NAME_GRID_Y_TOP for @c frame_parity==0, @c 0x150 for
 * @c frame_parity==1), size @c 0xA0 x @c NAME_GRID_VIS_HEIGHT.
 *
 * @param ctx_ptr   Render context cast to void* for codegen; OT head at
 *                  @c ot[0x0A], prim heap at @c prim_cursor, parity at
 *                  @c frame_parity.
 * @param panel_idx Active character panel index (0-3 normal, 4 kanji).
 *
 * @see decomp.me (77.64%) https://decomp.me/scratch/Glw7t
 */
void render_char_panel(RenderContext* ctx_ptr, s32 panel_idx)
{
    u8 grid_load_pkt[0x80];
    RenderContext* ctx = ctx_ptr;
    u32* prim = ctx->prim_cursor;
    u32* write_cur;
    u8* glyph_base;
    u16* entry_ptr;
    s32 col, entry_idx, row, entry_end;
    s32 screen_y;

    /* 1. Copy template packet from the inactive frame's reserve slot and
     *    splice it into OT[0x0A]. */
    func_8001A5D4(prim, (void*)(g_render_buf_base + ((ctx->frame_parity ^ 1) * 0x40C0) + 0x4064));

    addPrim(&ctx->ot[0x0A], prim);

    /* Write cursor starts 0x40 bytes past the template packet. */
    write_cur = (u32*)((u8*)prim + 0x40);

    /* 2. Select glyph data source based on the active panel. */
    if (g_char_panel == 4)
    {
        /* Kanji picker: use the kanji panel data blob and the current category's entry range. */
        glyph_base = (u8*)(g_kanji_panel_off + ((u32)&g_kanji_panel_off - 8));
        entry_idx = g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat]];
        entry_end = g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat] + 1];
        row = 0;
    }
    else
    {
        /* Normal panel: use the char panel data blob and the panel's entry range. */
        entry_idx = ((u16*)g_panel_char_offsets)[panel_idx * 2];
        entry_end = ((u16*)g_panel_char_offsets)[panel_idx * 2 + 1];
        glyph_base = (u8*)PANEL_REC_TBL;
        row = 0;
    }

    col = row; /* row is 0 here; col starts at 0 too */
    entry_ptr = (u16*)(glyph_base + (entry_idx * 2));

    /* 3. Walk every glyph entry in the panel, emitting sprites for visible ones. */
    for (;;)
    {
        /* Unsigned range check: visible if screen_y in [-11, 79] (within the 80-px grid window). */
        screen_y = (row * NAME_GRID_CELL_SIZE) - g_scroll_pos;
        if ((u32)(screen_y + 0x0B) < 0x5B)
        {
            write_cur = func_800A88A0(write_cur, (void*)&ctx->ot[0x0A], (void*)(glyph_base + *entry_ptr), 1, col * NAME_GRID_CELL_SIZE, screen_y, 0);
        }
        entry_idx++;
        entry_ptr++;
        if (entry_end == entry_idx)
        {
            break;
        }

        col++;
        if (col == NAME_GRID_CHARS_PER_ROW)
        {
            col = 0;
            row++;
        }
    }

    /* 4. Record the final grid position and append the VRAM upload RECT for the
     *    grid area on the back buffer page. */
    {
        u32 grid_vram_y = NAME_GRID_Y_TOP; /* 0x68; on page 1 it becomes 0x150 */
        g_char_last_row = row;
        g_char_last_col = col;
        if (ctx->frame_parity != 0)
        {
            grid_vram_y = 0x150;
        }

        func_8001C56C(grid_load_pkt, NAME_GRID_VRAM_X, grid_vram_y, 0xA0, NAME_GRID_VIS_HEIGHT);
        func_8001A5D4(write_cur, grid_load_pkt);

        addPrim(&ctx->ot[0x0A], write_cur);
        /* Byte addition of 0x40, not element addition. */
        ctx->prim_cursor = (u32*)((u8*)write_cur + 0x40);
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
 * @param prim    Destination @ref DR_TPAGE packet (8 bytes on the primitive heap).
 * @param ot_head Pointer to the 24-bit OT head entry (@ref u_long).
 * @return Heap cursor advanced past the packet (`prim + 8`).
 *
 * @see https://decomp.me/scratch/EyVeo (100%)
 */
void* emit_draw_mode_prim(DR_TPAGE* prim, u_long* ot_head)
{
    unsigned char* bytes = (unsigned char*)prim;
    u32* words = (u32*)prim;
    u32 temp1, temp2;

    setDrawTPage(bytes, 0, 0, 0x05);
    addPrim(ot_head, prim);

    return (void*)(bytes + 8);
}

/**
 * @brief Emit 1 or 2 glyph SPRT primitives and chain them onto an OT tag.
 *
 * Always writes a primary white-tinted (RGB=0x80) SPRT for glyph @p glyph_id
 * at @c (x - shadow_dist + primary_adj, y - shadow_dist + primary_adj).
 * When @p shadow_dist != 0, writes a second SPRT at @c (x + tmp, y + tmp)
 * with @c tmp = (shadow_dist - primary_adj) * 2. The second sprite's tint
 * and code byte depend on @p highlight:
 *   - @p highlight != 0: opaque blue (RGB=(0,0,0xA0), code 0x64).
 *   - @p highlight == 0: semi-transparent black (RGB=0, code 0x66) - drop shadow.
 *
 * Both sprites pull u/v/w/h/clut from @c g_glyph_table[glyph_id] and are
 * appended to the linked list at @p ot_tag via the standard addPrim sequence.
 *
 * @param prim_buf  Pointer to the next free byte in the primitive buffer.
 * @param ot_tag    Pointer to the OT head tag (addPrim "ot" arg).
 * @param glyph_id  Glyph index into @c g_glyph_table.
 * @param x         Base X screen coordinate.
 * @param y         Base Y screen coordinate.
 * @param shadow_dist Shadow offset distance in pixels. When 0, only the
 *                    primary SPRT is emitted.
 * @param primary_adj Added to the primary sprite's position offset. When
 *                    equal to @p shadow_dist the primary renders at (x,y)
 *                    with no shift (used for the selected/highlighted state).
 * @param highlight   Secondary-sprite mode: 0 = semi-transparent black drop
 *                    shadow; non-zero = opaque blue overlay.
 * @return Pointer to the byte after the last emitted primitive.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Au2h5
 */
void* emit_glyph_sprt(void* prim_buf, u_long* ot_tag, s32 glyph_id, s32 x, s32 y, s32 shadow_dist, s32 primary_adj, s32 highlight)
{
    u8* ptr = (u8*)prim_buf;
    SPRT* sprt = (SPRT*)ptr;

    /* (offset) + (base) form so gcc emits `addu v1,v1,v0` (vs the reverse
       order from `&g_glyph_table[glyph_id]`). Also keeps glyph_id live for the
       second SPRT's re-derivation below. */
    u8* entry = (u8*)((glyph_id << 3) + (u32)g_glyph_table);
    u32 clut_word;
    s32 tmp2;

    /* Primary glyph SPRT - white tint, fully opaque. */
    *(u32*)&sprt->r0 = 0x808080; /* r=g=b=0x80, code byte = 0 */
    setSprt(sprt);
    setXY0(sprt, (s16)(x - shadow_dist + primary_adj), (s16)(y - shadow_dist + primary_adj));
    setUV0(sprt, entry[0], entry[1]);
    setWH(sprt, entry[2], entry[3]);
    setClut(sprt, *(u32*)(entry + 4) << 4, 498);
    addPrim(ot_tag, sprt);
    ptr += sizeof(SPRT);

    if (shadow_dist != 0)
    {
        /* Secondary SPRT - drop shadow (highlight==0) or blue overlay (highlight!=0). */
        u8* new_var2;
        u8* entry2;
        u32 clut_word2;

        *(u32*)&((SPRT*)ptr)->r0 = (highlight != 0) ? 0xA00000 : 0;

        setSprt((SPRT*)ptr);

        if (highlight == 0)
        {
            setSemiTrans((SPRT*)ptr, 1);
        }

        tmp2 = (shadow_dist - primary_adj) * 2;

        new_var2 = (u8*)g_glyph_table;

        entry2 = (u8*)((glyph_id << 3) + (u32)new_var2);

        setXY0((SPRT*)ptr, (s16)(x + tmp2), (s16)(y + tmp2));
        setUV0((SPRT*)ptr, entry2[0], entry2[1]);
        setWH((SPRT*)ptr, (s16)entry2[2], (s16)entry2[3]);
        setClut((SPRT*)ptr, *(u32*)(entry2 + 4) << 4, 498);
        addPrim(ot_tag, ptr);

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
void* draw_char_append_anim(void* prim, void* ctx)
{
    u8 frame = g_append_anim_frame;
    void* result = prim;
    u8* table;
    u8* ot_base = (u8*)ctx;
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
            result = emit_glyph_sprt(result, ot_base + 0x30, (u8)glyph, px[0] + 0xE8, py[0] + 4, 0, 0, 0);
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