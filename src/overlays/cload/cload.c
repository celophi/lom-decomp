#include "cload_internal.h"

/**
 * @brief Address of the CLOAD text whose table offset is @p offset.
 * @note Summed as integers, offset first, like the original list drawing code.
 */
#define CLOAD_TEXT_BY_OFFSET(table, offset) ((u8 *)((s32)(offset) + (s32)(table)))

/**
 * @brief Initialize and run the CLOAD save/continue menu.
 * @return CLOAD result code set by the menu loop.
 * @see decomp.me (100.00%)
 */
s32 cload_main(void)
{
    RECT rect;

    g_cload_entry_state = 0xFF;
    g_cload_card_slot = 0;
    cload_reset_entry_ranks();
    cload_load_icon_resources();
    cload_init_display();
    g_cload_result = 0;
    cload_init_stream_handles();
    g_cload_icon_phase = 0;
    setRECT(&rect, 0x140, 0, 0x40, 0x100);
    ClearImage(&rect, 0, 0, 0);
    cload_reset_glyph_cache();
    D_80162370 = 0;
    g_cload_progress_active = 0;
    g_cload_selection_status = 0;
    g_cload_io_busy = 0;
    g_cload_frame_parity = 0;
    g_cload_exit_requested = 0;
    field_reset_input_repeat();
    cload_build_ui_elements();
    cload_run_menu_loop();
    return g_cload_result;
}

/**
 * @brief Run the double-buffered CLOAD menu loop until it exits.
 * @see decomp.me (100.00%)
 */
void cload_run_menu_loop(void)
{
    RECT rect;
    CloadRenderBuffer *frame;
    u_long *ordering_table;
    s32 buffer_index;
    s32 dpad_input;

    DrawSync(0);
    VSync(0);
    setRECT(&rect, 0, 0, 0x140, 0x1D8);
    ClearImage(&rect, 0, 0, 0);
    frame = &g_cload_render_buffers[0];
    buffer_index = 0;
    ClearOTagR(frame->ordering_table, 0x1000);
    ClearOTagR(g_cload_render_buffers[1].ordering_table, 0x1000);
    PutDispEnv(&frame->disp_env);
    update_controllers();
    SetDispMask(1);
    do
    {
        ordering_table = frame->ordering_table;
        ClearOTagR(ordering_table, 0x1000);
        frame->prim_cursor = (CloadGpuPacket *)g_cload_primitive_buffers[buffer_index];
        field_update_input_repeat();
        dpad_input = g_pad_input & 0xF000;
        if (dpad_input != 0)
        {
            g_pad_input = dpad_input;
        }
        field_update_and_render_fade(frame);
        if (cload_update_frame(frame) != 0)
        {
            break;
        }
        DrawSync(0);
        set_controller_vsync_interval(2);
        VSync(2);
        ClearImage(&frame->clear_rect, 0, 0, 0);
        buffer_index = 0;
        if (frame == &g_cload_render_buffers[0])
        {
            frame = &g_cload_render_buffers[1];
            buffer_index = 1;
        }
        else
        {
            frame = &g_cload_render_buffers[0];
        }
        PutDispEnv(&frame->disp_env);
        PutDrawEnv(&frame->draw_env);
        DrawOTag(ordering_table + 0xFFF);
        update_controllers();
        cdrom_process_state();
    } while (1);
    reset_controller_vsync_state();
    VSync(0);
}

/**
 * @brief Initialize the CLOAD display and draw buffers.
 * @see decomp.me (100.00%)
 */
void cload_init_display(void)
{
    u8 *display_rect;
    s16 *second_display_rect;

    /* Preserve GCC 2.7.2's original stack-frame bucket without a dead call. */
    s32 stack_frame_pad[2];
    SetGeomScreen(0x5DC);
    SetGeomOffset(0xA0, 0x78);
    D_8014EA38 = 0;
    display_rect = (u8 *)&D_8014EA38;
    second_display_rect = (s16 *)(display_rect + 0x7CC4);
    *(s16 *)(display_rect + 0x2) = 0;
    *(s16 *)(display_rect + 0x4) = 0x140;
    *(s16 *)(display_rect + 0x6) = 0xF0;
    *(s16 *)(display_rect + 0x7CC4) = 0;
    second_display_rect[1] = 0xE8;
    second_display_rect[2] = 0x140;
    second_display_rect[3] = 0xF0;
    DrawSync(0);
    VSync(0);
    SetDefDispEnv(display_rect - 0x70, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(display_rect + 0x7C54, 0, 0xE8, 0x140, 0xF0);
    SetDefDrawEnv(display_rect - 0x5C, 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv(display_rect + 0x7C68, 0, 0x8, 0x140, 0xE0);
    display_rect[0x7C7E] = 0;
    display_rect[-0x46] = 0;
    field_reset_fade_state();
    field_set_fade_target(0x100, 0x100, 0x100, 0x14);
}

/**
 * @brief Advance one CLOAD menu frame and report whether it should exit.
 * @param frame Render buffer being built this frame.
 * @return 1 when the overlay should exit, otherwise 0.
 * @see decomp.me (100.00%)
 */
s32 cload_update_frame(CloadRenderBuffer *frame)
{
    if (g_cload_exit_requested != 0)
    {
        cload_shutdown_stream_handles();
        field_text_reset_windows();
        DrawSync(0);
        return 1;
    }
    field_text_reset_scratch();
    cload_begin_glyph_cache_frame();
    cload_update_menu(frame);
    cload_evict_unused_glyphs();
    field_text_upload_immediate_cache();
    g_cload_frame_parity ^= 1;
    return 0;
}

/**
 * @brief Allocate and lay out the five fixed windows of the load screen.
 * @see decomp.me (100.00%)
 */
void cload_build_ui_elements(void)
{
    CloadElement *element;
    s32 unused[2]; /* never used, but the compiled code only matches the original with it */

    g_cload_scroll_frames = 0;
    g_cload_scroll_target_y = 0;
    g_cload_scroll_y = 0;
    g_cload_selected_row = 0;
    g_cload_selection_status = 0;
    cload_clear_elements();
    /* Hold slot 0 so the fixed elements below are allocated from slot 1 on. */
    g_cload_element_pool[0].attr.f.state = CLOAD_ELEMENT_OPENING;

    element = cload_alloc_element();
    element->draw = cload_draw_entry_list;
    element->attr.f.phase = 1;
    element->attr.f.x = 28;
    element->attr.f.y = 74;
    element->size.f.framed = 0;
    CLOAD_SET_ELEMENT_WIDTH_LOW(element, 8);
    element->size.f.width_high = 1;
    CLOAD_SET_ELEMENT_HEIGHT(element, 73);

    element = cload_alloc_element();
    element->draw = cload_draw_header_label;
    element->attr.f.state = CLOAD_ELEMENT_OPEN;
    element->attr.f.phase = 1;
    element->attr.f.x = 80;
    element->attr.f.y = 12;
    element->size.f.framed = 0;
    CLOAD_SET_ELEMENT_WIDTH_LOW(element, 0xA0);
    element->size.f.width_high = 0;
    CLOAD_SET_ELEMENT_HEIGHT(element, 15);

    element = cload_alloc_element();
    element->attr.f.state = CLOAD_ELEMENT_OPEN;
    element->attr.f.phase = 1;
    element->attr.f.x = 24;
    element->draw = cload_draw_card_slot0_label;
    element->attr.f.y = 44;
    element->size.f.framed = 0;
    CLOAD_SET_ELEMENT_WIDTH_LOW(element, 0x80);
    element->size.f.width_high = 0;
    CLOAD_SET_ELEMENT_HEIGHT(element, 15);

    element = cload_alloc_element();
    element->attr.f.state = CLOAD_ELEMENT_OPEN;
    element->attr.f.phase = 1;
    element->attr.f.x = 168;
    element->draw = cload_draw_card_slot1_label;
    element->attr.f.y = 44;
    element->size.f.framed = 0;
    CLOAD_SET_ELEMENT_WIDTH_LOW(element, 0x80);
    element->size.f.width_high = 0;
    CLOAD_SET_ELEMENT_HEIGHT(element, 15);

    element = cload_alloc_element();
    element->draw = cload_draw_selected_entry_details;
    element->attr.f.state = CLOAD_ELEMENT_OPEN;
    element->attr.f.phase = 1;
    element->attr.f.x = 30;
    element->attr.f.y = 160;
    element->size.f.framed = 0;
    CLOAD_SET_ELEMENT_WIDTH_LOW(element, 4);
    element->size.f.width_high = 1;
    CLOAD_SET_ELEMENT_HEIGHT(element, 51);

    g_cload_element_pool[0].attr.f.state = CLOAD_ELEMENT_FREE;
}

/**
 * @brief Update input, loading state, scrolling, and UI elements for one frame.
 * @param frame Render buffer being built this frame.
 * @see decomp.me (100.00%)
 */
void cload_update_menu(CloadRenderBuffer *frame)
{
    s32 delta;

    cload_update_elements(frame);
    g_cload_icon_phase += 2;
    if ((g_cload_element1_state & 0x7F) == 2)
    {
        cload_update_load_sequence();
    }
    if ((u16)g_pad_input == 0xFFFF)
    {
        g_pad_input = 0;
    }
    cload_handle_input();
    if (g_cload_scroll_frames != 0)
    {
        s32 base = g_cload_scroll_y;
        delta = (g_cload_scroll_target_y - g_cload_scroll_y) / g_cload_scroll_frames;
        g_cload_scroll_frames -= 1;
        g_cload_scroll_y += delta;
    }
    else
    {
        g_cload_scroll_y = g_cload_scroll_target_y;
    }
}

/**
 * @brief Advance the active load sequence and react to its phase result.
 * @see decomp.me (100.00%)
 */
void cload_update_load_sequence(void)
{
    s32 phase;

    if (g_cload_entry_state >= 0x10)
    {
        if (g_cload_load_step == NULL)
        {
            g_cload_load_step = g_cload_steps_initial_scan;
        }
    }
    do
    {
        phase = cload_advance_load_sequence();
    } while (phase == 3);
    if (phase == 2)
    {
        g_cload_load_step = g_cload_steps_card_reset;
    }
    if (phase == 4)
    {
        g_cload_load_step = g_cload_steps_refresh_entries;
    }
    if (phase == 5)
    {
        g_cload_entry_state = 0xF9;
        g_cload_load_step = g_cload_steps_card_reset;
    }
}

/**
 * @brief Handle CLOAD menu navigation, confirm, and cancel input.
 * @return Input-handler status used by the caller.
 * @note The up/down navigation reads g_pad_input directly inside the count
 *       loop, so the selected-row arithmetic materializes in the target's
 *       registers.
 * @see decomp.me (100.00%)
 */
s32 cload_handle_input(void)
{
    s32 pending;
    s32 status;
    s32 count;
    s32 sfx_id;
    CloadElement *prompt;

    if (g_cload_element_pool[1].attr.f.state == CLOAD_ELEMENT_FREE)
    {
        g_cload_exit_requested = 1;
        return;
    }
    if (g_cload_exit_requested != 0)
    {
        return;
    }
    if (g_cload_element_pool[1].attr.f.state >= CLOAD_ELEMENT_CLOSING)
    {
        return;
    }
    if (g_cload_element_pool[0].attr.f.state != CLOAD_ELEMENT_FREE)
    {
        return;
    }
    pending = g_cload_entry_state;
    if (pending == 0xFF)
    {
        return;
    }
    if (g_cload_entry_scan_active != 0)
    {
        return;
    }
    if (g_cload_io_busy != 0)
    {
        return;
    }
    if (*g_cload_load_step >= 6 && *g_cload_load_step <= 7)
    {
        return;
    }
    status = g_pad_input;
    if (status & 0x40)
    {
        g_cload_exit_requested = 1;
        g_cload_result = 1;
        play_menu_sfx(0x78, 0x80);
        return;
    }
    if (status & 0xA100)
    {
        play_menu_sfx(0x7D, 0x80);
        g_cload_scroll_frames = 0;
        g_cload_scroll_target_y = 0;
        g_cload_scroll_y = 0;
        g_cload_selected_row = 0;
        g_cload_load_step = NULL;
        g_cload_entry_state = 0xFF;
        g_cload_selection_status = 0;
        g_cload_card_slot ^= 1;
        cload_reset_entry_ranks();
        return;
    }
    if (pending >= 0x10)
    {
        return;
    }
    count = 1;
    if (status & 8)
    {
        g_pad_input = 0x4000;
        count = 1;
    }
    if (g_pad_input & 4)
    {
        g_pad_input = 0x1000;
        count = 1;
    }
    while (count != 0)
    {
        if (g_pad_input & 0x1000)
        {
            g_cload_selected_row -= 1;
            if (g_cload_selected_row < 0)
            {
                g_cload_selected_row = g_cload_entry_state - 1;
            }
        }
        if (g_pad_input & 0x4000)
        {
            g_cload_selected_row += 1;
            if (g_cload_selected_row >= g_cload_entry_state)
            {
                g_cload_selected_row = 0;
            }
        }
        count -= 1;
    }
    if (g_pad_input & 0x5000)
    {
        cload_commit_selected_entry();
        play_menu_sfx(0x7D, 0x80);
        cload_scroll_to_selection();
        return;
    }
    if (g_pad_input & 0x220)
    {
        if (strncmp(g_lom_save_filename_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, g_cload_selected_row).name, 0xC) != 0)
        {
            sfx_id = 0x78;
        }
        else
        {
            if ((g_cload_selected_save_slot_id == g_save_slot_index) || (g_cload_selected_save_slot_id == 0xFF))
            {
                prompt = cload_alloc_element();
                prompt->draw = cload_draw_load_prompt;
                prompt->attr.f.phase = 1;
                prompt->attr.f.x = 16;
                prompt->attr.f.y = 91;
                prompt->size.f.width_high = 1;
                prompt->size.f.height = 43;
                CLOAD_SET_ELEMENT_WIDTH_LOW(prompt, 0x20);
                cload_enable_choice_toggle();
                cload_restart_load_sequence();
                sfx_id = 0x7E;
            }
            else
            {
                sfx_id = 0x78;
            }
        }
        play_menu_sfx(sfx_id, 0x80);
    }
}

/**
 * @brief Put every live UI element into its closing state.
 * @see decomp.me (100.00%)
 */
void cload_close_all_elements(void)
{
    CloadElement *element;
    s32 i;

    element = g_cload_element_pool;
    for (i = 0; i < CLOAD_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.f.state != CLOAD_ELEMENT_FREE)
        {
            element->attr.f.state = CLOAD_ELEMENT_CLOSING;
            element->attr.f.phase = CLOAD_ELEMENT_PHASE_STEPS;
        }
    }
}


/**
 * @brief Move the list scroll target to keep the selected row visible.
 * @see decomp.me (100.00%)
 */
void cload_scroll_to_selection(void)
{
    s32 base;
    s32 delta;

    base = g_cload_selected_row * CLOAD_ENTRY_ROW_HEIGHT;
    delta = base - g_cload_scroll_y;
    if (delta >= 0x3C)
    {
        g_cload_scroll_target_y = base - 0x38;
        g_cload_scroll_frames = 4;
    }
    if (delta < 0)
    {
        g_cload_scroll_target_y = g_cload_selected_row * CLOAD_ENTRY_ROW_HEIGHT;
        g_cload_scroll_frames = 4;
    }
}

/**
 * @brief Run the UI element update/draw pass.
 * @param frame Render buffer being built this frame.
 * @see decomp.me (100.00%)
 */
void cload_update_elements(CloadRenderBuffer *frame)
{
    cload_update_and_draw_elements(frame);
}

/**
 * @brief Draw the visible save-entry list and selection cursor.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note Menu string/glyph-row drawing callback (state-dispatched TILE + text
 *       renderer). The row loop is a `do { } while (i < g_cload_entry_state)`
 *       guarded by `if (state > 0)` with `row_y`/`i` hoisted to the default
 *       block, the rank-marker glyph offsets are materialized through a `u16
 *       misc_glyph` intermediate, and entry comparisons use strncmp - the
 *       shapes the target's register assignment requires.
 * @see decomp.me (100.00%)
 */
void *cload_draw_entry_list(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    s32 state = g_cload_entry_state;

    switch (state)
    {
    case 0xF8:
        do
        {
            prim = func_800A88A0(prim, ot, CLOAD_TEXT_AT(g_cload_text_no_lom_save_data, 26), 1, -x_offset + 0x84, -y_offset, 2);
        } while (0);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, CLOAD_TEXT_AT(g_cload_text_no_lom_save_data, 26), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, CLOAD_TEXT_AT(g_cload_text_not_enough_blocks, 1), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, CLOAD_TEXT_AT(g_cload_text_no_memory_card, 2), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, CLOAD_TEXT_AT(g_cload_text_card_access_failed, 8), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, CLOAD_TEXT_AT(g_cload_text_no_save_data, 9), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFE:
        break;
    default:
        {
            s32 row_y;
            s32 i;

        if (g_cload_entry_scan_active != 0)
        {
            s32 x;
            u16 *text_table;
        case 0xFF:
            x = -x_offset + 0x84;
            text_table = &g_cload_text_check_memory_card;
            prim = func_800A88A0(prim, ot, CLOAD_TEXT(text_table, 0), 1, x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, CLOAD_TEXT(text_table, 15), 1, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, CLOAD_TEXT(text_table, 89), 1, x, 0x1C - y_offset, 2);
            break;
        }
        i = 0;
        if (state > 0)
        {
            s32 off;
            s32 base_x;
            s32 *flag_ptr;
            u16 marker_offset;
            char *entry;
            DVECTOR pos;
            u16 *text_table;

            off = i;
            base_x = -x_offset;
            text_table = &g_cload_text_check_memory_card;
            entry = g_cload_entries;
            off = i;
            do
            {
                row_y = ((i * CLOAD_ENTRY_ROW_HEIGHT) - y_offset) - g_cload_scroll_y;
                if (row_y >= -13 && row_y <= 72)
                {
                    flag_ptr = (s32 *)((u8 *)g_cload_entry_ranks + off);
                    if (*flag_ptr >= 0)
                    {
                        pos.vx = base_x + 0x86;
                        pos.vy = row_y;
                        prim = func_800A88A0(func_800A8A78(ot, prim, *(s32 *)((u8 *)g_cload_entry_suffix_values + off), 1, &pos, 0), ot, CLOAD_TEXT_BY_OFFSET(text_table, g_cload_text_number_prefix), 1, base_x + 0x70, row_y, 0);
                        if ((g_cload_rank_count - 1) == *flag_ptr)
                        {
                            marker_offset = text_table[27];
                            prim = func_800A88A0(prim, ot, CLOAD_TEXT_BY_OFFSET(text_table, marker_offset), 1, base_x + 0xC2, row_y, 0);
                        }
                        else if (*flag_ptr < 2)
                        {
                            marker_offset = text_table[28];
                            prim = func_800A88A0(prim, ot, CLOAD_TEXT_BY_OFFSET(text_table, marker_offset), 1, base_x + 0xC2, row_y, 0);
                        }
                        if (*cload_skip_hex_digits((u8 *)((g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES) + (s32)entry + 0xC)) == 0x2B)
                        {
                            prim = func_800A88A0(prim, ot, CLOAD_TEXT_BY_OFFSET(text_table, g_cload_text_plus_marker), 1, 0xF8 - x_offset, row_y, 1);
                        }
                    }
                    if (strncmp(g_lom_save_filename_prefix, (char *)((g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES) + (s32)entry), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, CLOAD_TEXT_BY_OFFSET(text_table, g_cload_text_mana), 1, base_x, row_y, 0);
                    }
                    else if (strncmp(g_lom_alt_save_filename_prefix, (char *)((g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES) + (s32)entry), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, CLOAD_TEXT_BY_OFFSET(text_table, g_cload_text_alt_save), 1, base_x, row_y, 0);
                    }
                    else if (strncmp(g_new_save_entry_prefix, (char *)((g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES) + (s32)entry), 8) == 0)
                    {
                        prim = func_800A88A0(prim, ot, CLOAD_TEXT_BY_OFFSET(text_table, g_cload_text_new_save), 1, base_x, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, CLOAD_TEXT_BY_OFFSET(text_table, g_cload_text_other_game), 1, base_x, row_y, 0);
                    }
                }
                entry += CLOAD_DIRECTORY_ENTRY_BYTES;
                off += 4;
                i++;
            } while (i < g_cload_entry_state);
        }
            row_y = ((g_cload_selected_row * CLOAD_ENTRY_ROW_HEIGHT) - y_offset) - g_cload_scroll_y;

            if (g_cload_entry_scan_active == 0)
            {
                TILE *tile = (TILE *)prim;

                *(u32 *)&tile->r0 = 0xF080F0;
                setlen(tile, 3);
                setcode(tile, 0x62);
                tile->y0 = (s16)(row_y - 1);
                tile->w = 0x108;
                tile->x0 = 0;
                tile->h = 0xE;
                addPrim(ot, tile);
                prim = tile + 1;
            }
        }
        break;
    }
    return prim;
}

/**
 * @brief Advance a pointer past a run of hex-digit characters ('0'-'9',
 *        'a'-'f', 'A'-'F').
 * @param text Pointer to the first character to test.
 * @return Pointer to the first character that is not a hex digit.
 * @see decomp.me (100%)
 */
u8 *cload_skip_hex_digits(u8 *text)
{
    while ((*text >= '0' && *text <= '9') || (*text >= 'a' && *text <= 'f') || (*text >= 'A' && *text <= 'F'))
    {
        text++;
    }
    return text;
}

/**
 * @brief Draw the fixed CLOAD header label.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100.00%)
 */
void *cload_draw_header_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */

    return func_800A88A0(prim, ot, CLOAD_TEXT_AT(g_cload_text_load, 22), 1, -x_offset + 0x50, -y_offset, 2);
}

/**
 * @brief Draw the first memory-card slot label.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100.00%)
 */
void *cload_draw_card_slot0_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    s32 color;
    u8 *text;
    RECT unused; /* never used, but the original stack frame reserves it */

    color = 1;
    text = CLOAD_TEXT_AT(g_cload_text_card_slot_1, 6);
    if (g_cload_card_slot != 0)
    {
        color = 3;
    }
    return func_800A88A0(prim, ot, text, color, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the second memory-card slot label.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100.00%)
 */
void *cload_draw_card_slot1_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    s32 color;
    u8 *text;
    RECT unused; /* never used, but the original stack frame reserves it */

    color = 1;
    text = CLOAD_TEXT_AT(g_cload_text_card_slot_2, 7);
    if (g_cload_card_slot == 0)
    {
        color = 3;
    }
    return func_800A88A0(prim, ot, text, color, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw metadata for the selected save entry.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note Save-slot HUD callback: draws either the elapsed-play-time display
 *       (hours:minutes plus a 3-memcard-icon highlight strip) when the slot
 *       name matches the empty-slot marker, or the slot's save-file name
 *       otherwise. The icon-highlight loop keeps the "entries seen so far"
 *       count (i) and the raw slot index (j) as two separate locals, and the
 *       fallback-text branch wraps its two copy loops in the target's nested
 *       do/while(0) cross-jump shells.
 * @see decomp.me (100.00%)
 */
void *cload_draw_selected_entry_details(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    void *result;
    DVECTOR pos;
    u8 name[0x100];
    s32 party_icon[3];

    result = prim;
    if (g_cload_selection_status == 0)
    {
        return result;
    }
    if (g_cload_entry_scan_active != 0)
    {
        return result;
    }
    if (g_cload_selection_status != 3 && g_cload_entry_state < 0x10)
    {
        if (g_cload_selection_status == 2)
        {
            s32 x = -x_offset;
            u16 *text_table;

            result = func_800A88A0(prim, ot, CLOAD_TEXT_AT(g_cload_text_new_save_prompt, 20), 1, x, -y_offset, 0);
            text_table = CLOAD_TEXT_TABLE(g_cload_text_new_save_prompt, 20);
            return func_800A88A0(result, ot, CLOAD_TEXT(text_table, 21), 1, x, 0x10 - y_offset, 0);
        }
        else
        {
            if (strncmp(g_lom_save_filename_prefix, CLOAD_DIR_ENTRY(g_cload_card_slot, g_cload_selected_row).name, 0xC) == 0)
            {
                CloadSaveMetadata *save = &g_cload_selected_save_metadata;

                if (save->save_slot_id == 0xFF || save->save_slot_id == g_save_slot_index)
                {
                    s32 present_count;
                    s32 i;
                    s32 j;
                    s32 step;
                    s32 half_step;
                    s32 base_x;
                    s32 base_y;
                    s32 total;
                    s32 hours;
                    s32 time_val;

                    total = 0;
                    party_icon[0] = save->party_icon_0;
                    party_icon[1] = save->party_icon_1;
                    party_icon[2] = save->party_icon_2;
                    g_cload_icon_palette = save->icon_palette;

                    present_count = 0;
                    for (i = 0; i < 3; i++)
                    {
                        if (party_icon[i] != CLOAD_NO_ICON)
                        {
                            present_count += 1;
                        }
                    }

                    switch (present_count)
                    {
                    case 2:
                        step = 0x20;
                        half_step = 0x10;
                        time_val = g_cload_icon_phase;
                        if (g_cload_icon_phase < 0)
                        {
                            time_val = g_cload_icon_phase + 0x1F;
                        }
                        g_cload_icon_phase -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        g_cload_icon_phase %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        g_cload_icon_phase = 0x1F;
                        break;
                    }

                    i = 0;
                    j = i;
                    for (; j < 3; j++)
                    {
                        base_y = i * half_step;
                        base_x = base_y + half_step;
                        if (party_icon[j] != CLOAD_NO_ICON)
                        {
                            s32 adjust = step;
                            s32 rem;
                            s32 hi;
                            s32 delta;

                            if ((g_cload_icon_phase >= base_y && g_cload_icon_phase < base_x && (delta = g_cload_icon_phase - base_y, 1))
                                || (rem = base_x % (half_step * present_count), g_cload_icon_phase >= rem && g_cload_icon_phase < (hi = rem + half_step) && (delta = hi - g_cload_icon_phase, 1)))
                            {
                                adjust += delta;
                            }
                            result = cload_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, party_icon[j], i, j);
                            total += adjust;
                            i += 1;
                        }
                    }

                    {
                        CloadSaveMetadata *shown_save = &g_cload_selected_save_metadata;
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        base_y = shown_save->playtime;

                        pos.vx = (s16)(x + 0x70);
                        pos.vy = (s16)y;
                        hours = base_y / 216000;
                        result = func_800A8A78(ot, result, hours, 1, &pos, 1);
                        result = func_800A88A0(result, ot, FIELD_UI_TEXT_AT(g_text_time_separator_offset_bytes, 25), 1, x + 0x6F, y, 0);
                        base_y = (base_y / 3600) - (hours * 0x3C);
                        if (base_y < 0xA)
                        {
                            pos.vx = (s16)(x + 0x7D);
                            pos.vy = (s16)y;
                            result = func_800A8A78(ot, result, 0, 1, &pos, 1);
                        }
                        pos.vx = (s16)(x + 0x85);
                        pos.vy = (s16)y;
                        result = func_800A88A0(func_800A88A0(func_800A8A78(ot, result, base_y, 1, &pos, 1), ot, shown_save->title, 1, x + 0x54, y + 0x10, 0), ot, CLOAD_TEXT(g_cload_location_names, shown_save->location), 1, x + 0x54, y + 0x20, 0);
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, CLOAD_TEXT_AT(g_cload_text_version_error, 42), 1, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 j;

                {
                    u8 *text_base;
                    cload_terminate_multibyte_text(g_cload_selected_file_title);
                    text_base = (u8 *)g_cload_selected_file_title;
                    text_base -= 4;
                    if (text_base[0x24] == 0 || text_base[0x24] >= 0x80)
                    {
                        do {
                        do {
                        do {

                        for (j = 0; j < 0x20; j++)
                        {
                            name[j] = text_base[j + 4];
                        }
                    name[j] = 0;
                    result = cload_draw_cached_text(result, ot, name, -x_offset, -y_offset, 1, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = ((CloadCardHeaderText *)g_cload_selected_file_header)->title_line_2[j];
                    }
                    name[j] = 0;
                        result = cload_draw_cached_text(result, ot, name, -x_offset, -y_offset + 0x10, 1, 0);
                        } while (0);
                        } while (0);
                        } while (0);
                    }
                }
            }
        }
    }
    return result;
}

/**
 * @brief Zero-fill a 64-byte text buffer after its encoded terminator.
 * @param text Encoded text buffer.
 * @see decomp.me (100.00%)
 */
void cload_terminate_multibyte_text(void *text)
{
    u8 *p;
    s32 i;

    p = (u8 *)text;
    i = 0;
    for (;;)
    {
        if (i >= 0x40)
        {
            return;
        }
        if (*p == 0)
        {
            while (i < 0x40)
            {
                *p = 0;
                i++;
                p++;
            }
            return;
        }
        if (*p >= 0x80)
        {
            p += 2;
            i += 2;
        }
        else
        {
            p += 1;
            i += 1;
        }
    }
}

/**
 * @brief Mark all eight UI elements as inactive.
 * @see decomp.me (100.00%)
 */
void cload_clear_elements(void)
{
    CloadElement *p;
    s32 i;

    p = g_cload_element_pool;
    for (i = 0; i < CLOAD_ELEMENT_COUNT; i++)
    {
        p->attr.f.state = CLOAD_ELEMENT_FREE;
        p++;
    }
}

/**
 * @brief Activate and return the first free UI element.
 * @return First free element, or the pool head if all slots are busy.
 * @see decomp.me (100.00%)
 */
CloadElement *cload_alloc_element(void)
{
    CloadElement *p;
    s32 i;

    p = g_cload_element_pool;
    for (i = 0; i < CLOAD_ELEMENT_COUNT; i++, p++)
    {
        if (p->attr.f.state == CLOAD_ELEMENT_FREE)
        {
            p->size.f.framed = 1;
            p->attr.f.state = CLOAD_ELEMENT_OPENING;
            return p;
        }
    }
    return g_cload_element_pool;
}


/**
 * @brief Advance and draw the eight pool elements for one frame.
 *
 * Links a draw-environment packet for each live element into the ordering
 * table, then animates it by state: an opening window grows, an open window
 * holds, a closing window shrinks, and a closed window counts down to free.
 *
 * @param frame Render buffer being built; prim_cursor is read on entry and written back on exit.
 * @see decomp.me (100.00%)
 */
void cload_update_and_draw_elements(CloadRenderBuffer *frame)
{
    void *arrow_prim; /* separate cursor for the arrows; one shared cursor does not match the original */
    void *prim;
    u_long *ot;
    CloadElement *element;
    s32 i;
    DRAWENV draw_env;
    s32 scaled_width;
    s32 scaled_height;

    arrow_prim = frame->prim_cursor;
    ot = frame->ordering_table;

    if ((g_cload_entry_state < 0x10) && ((g_cload_element1_state & CLOAD_ELEMENT_STATE_MASK) == 2))
    {
        if ((g_cload_entry_state * CLOAD_ENTRY_ROW_HEIGHT) > (g_cload_scroll_y + 0x49))
        {
            arrow_prim = cload_emit_scroll_arrow(arrow_prim, ot, 0x114, 0x87, 0);
        }
        if (g_cload_scroll_y != 0)
        {
            arrow_prim = cload_emit_scroll_arrow(arrow_prim, ot, 0x114, 0x4A, 1);
        }
    }

    if (frame->clear_rect.y != 0)
    {
        SetDefDrawEnv(&draw_env, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        SetDefDrawEnv(&draw_env, 0, 8, 0x140, 0xE0);
    }

    prim = arrow_prim;
    element = g_cload_element_pool;
    for (i = 0; i < CLOAD_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.f.state != 0)
        {
            SetDrawEnv((DR_ENV *)prim, &draw_env);
            addPrim(ot, prim);
            prim = (DR_ENV *)prim + 1;

            switch (element->attr.f.state)
            {
            case CLOAD_ELEMENT_OPENING:
                g_pad_input = 0;
                {
                    u32 width_low = CLOAD_ELEMENT_WIDTH_LOW(element);
                    s32 width = CLOAD_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.f.phase) / 8;
                    scaled_height = (element->size.f.height * element->attr.f.phase) / 8;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.f.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.f.x;
                    u32 width_low = CLOAD_ELEMENT_WIDTH_LOW(element);

                    prim = cload_emit_window_frame(prim, ot, x + (CLOAD_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                   element->attr.f.y + (element->size.f.height - scaled_height) / 2, scaled_width, scaled_height,
                                                   frame->clear_rect.y, element->size.f.framed);
                }
                element->attr.f.phase++;
                if (element->attr.f.phase == CLOAD_ELEMENT_PHASE_STEPS)
                {
                    field_reset_input_repeat();
                    element->attr.f.state = CLOAD_ELEMENT_OPEN;
                }
                break;

            case CLOAD_ELEMENT_OPEN:
                prim = element->draw(ot, prim, 0, 0);
                {
                    u32 width_low = CLOAD_ELEMENT_WIDTH_LOW(element);

                    prim = cload_emit_window_frame(prim, ot, element->attr.f.x, element->attr.f.y, CLOAD_ELEMENT_WIDTH(element, width_low),
                                                   element->size.f.height, frame->clear_rect.y, element->size.f.framed);
                }
                if (element->attr.f.phase != 0)
                {
                    element->attr.f.phase--;
                }
                break;

            case CLOAD_ELEMENT_CLOSING:
                g_pad_input = 0;
                {
                    u32 width_low = CLOAD_ELEMENT_WIDTH_LOW(element);
                    s32 width = CLOAD_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.f.phase) / 8;
                    scaled_height = (element->size.f.height * element->attr.f.phase) / 8;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.f.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.f.x;
                    u32 width_low = CLOAD_ELEMENT_WIDTH_LOW(element);

                    prim = cload_emit_window_frame(prim, ot, x + (CLOAD_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                   element->attr.f.y + (element->size.f.height - scaled_height) / 2, scaled_width, scaled_height,
                                                   frame->clear_rect.y, element->size.f.framed);
                }
                element->attr.f.phase--;
                if (element->attr.f.phase == 0)
                {
                    element->attr.f.phase = 3;
                    element->attr.f.state = CLOAD_ELEMENT_CLOSED;
                }
                break;

            case CLOAD_ELEMENT_CLOSED:
                g_pad_input = 0;
                element->attr.f.phase--;
                if (element->attr.f.phase == 0)
                {
                    element->attr.f.state = CLOAD_ELEMENT_FREE;
                }
                break;
            }
        }
    }

    frame->prim_cursor = cload_emit_icon_highlight_strip(prim, ot);
}

/**
 * @brief Append one encoded CLOAD string to another.
 * @param dest Destination text buffer.
 * @param src Source text buffer.
 * @see decomp.me (100.00%)
 */
void cload_text_append(u8 *dest, u8 *src)
{
    s32 dst_len;
    s32 src_len;
    s32 i;

    dst_len = cload_text_byte_length(dest);
    src_len = cload_text_byte_length(src);
    for (i = 0; i < src_len; i++)
    {
        dest[dst_len + i] = src[i];
    }
    dest[dst_len + i] = 0;
}

/**
 * @brief Measure an encoded CLOAD string in bytes.
 * @param text Encoded text buffer.
 * @return Encoded byte length excluding the terminator.
 * @see decomp.me (100.00%)
 */
s32 cload_text_byte_length(u8 *text)
{
    u8 *p;
    u8 c;
    s32 len;

    p = text;
    c = *p;
    len = 0;
    while (c != 0)
    {
        if (c >= 0x19 && c <= 0x1F)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
        c = *p;
    }
    return len;
}

/**
 * @brief Copy one encoded CLOAD string including its terminator.
 * @param dest Destination text buffer.
 * @param src Source text buffer.
 * @see decomp.me (100.00%)
 */
void cload_text_copy(u8 *dest, u8 *src)
{
    u8 *p;
    s32 len;
    s32 i;

    p = src;
    len = 0;
    while (*p != 0)
    {
        if (*p >= 0x19 && *p <= 0x1F)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
    }
    for (i = 0; i < len; i++)
    {
        dest[i] = src[i];
    }
    dest[i] = 0;
}

