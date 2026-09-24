#include "carda_internal.h"

/**
 * @brief Address of the CARDA text whose table offset is @p offset.
 * @note Summed as integers, offset first, like the original list drawing code.
 */
#define CARDA_TEXT_BY_OFFSET(table, offset) ((u8*)((s32)(offset) + (s32)(table)))

/**
 * @brief Advance one CARDA frame and report whether the overlay should exit.
 * @param frame Render buffer being built this frame.
 * @return 1 when the overlay should exit, otherwise 0.
 */
s32 carda_update_frame(CardaRenderBuffer* frame)
{
    if (g_carda_exit_requested != 0)
    {
        carda_shutdown_stream_handles();
        field_text_reset_windows();
        DrawSync(0);
        return 1;
    }
    field_text_reset_scratch();
    carda_begin_glyph_cache_frame();
    carda_update_menu(frame);
    carda_evict_unused_glyphs();
    field_text_upload_immediate_cache();
    g_carda_frame_parity ^= 1;
    return 0;
}

/**
 * @brief Allocate and lay out the fixed windows of the save screen.
 *
 * Modes 2 and 3 get a four-window layout, every other mode the five-window
 * save-file browser.
 */
void carda_build_ui_elements(void)
{
    CardaElement* element;
    s32 unused[2]; /* never used, but the compiled code only matches the original with it */

    g_carda_scroll_frames = 0;
    g_carda_scroll_target_y = 0;
    g_carda_scroll_y = 0;
    g_carda_selected_row = 0;
    g_carda_selection_status = 0;
    D_80166100 = g_pad_ctx + 0xCE0;
    carda_clear_elements();
    D_801660F8 = 0;

    /* Hold slot 0 so the fixed elements below are allocated from slot 1 on. */
    g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_OPENING;
    if (g_carda_mode >= 2 && g_carda_mode <= 3)
    {
        element = carda_alloc_element();
        element->draw = carda_draw_save_flow;
        element->attr.f.phase = 1;
        element->attr.f.x = 16;
        element->attr.f.y = 76;
        element->size.f.width_high = 1;
        element->size.f.height = 72;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x20);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot0_label;
        element->attr.f.phase = 1;
        element->attr.f.x = 28;
        element->attr.f.y = 58;
        element->size.f.width_high = 0;
        element->size.f.height = 16;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x80);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot1_label;
        element->attr.f.phase = 1;
        element->attr.f.x = 164;
        element->attr.f.y = 58;
        element->size.f.width_high = 0;
        element->size.f.height = 16;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x80);

        element = carda_alloc_element();
        element->draw = carda_draw_header_label;
        element->attr.f.phase = 1;
        element->attr.f.x = 32;
        element->attr.f.y = 34;
        element->size.f.width_high = 1;
        element->size.f.height = 16;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, 0);
    }
    else
    {
        element = carda_alloc_element();
        element->draw = carda_draw_entry_list;
        element->attr.f.phase = 1;
        element->attr.f.x = 10;
        element->attr.f.y = 50;
        element->size.f.width_high = 1;
        element->size.f.height = 88;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x2C);

        element = carda_alloc_element();
        element->draw = carda_draw_header_label;
        element->attr.f.phase = 1;
        element->attr.f.x = 104;
        element->attr.f.y = 10;
        element->size.f.width_high = 0;
        element->size.f.height = 16;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x70);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot0_label;
        element->attr.f.phase = 1;
        element->attr.f.x = 28;
        element->attr.f.y = 30;
        element->size.f.width_high = 0;
        element->size.f.height = 16;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x80);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot1_label;
        element->attr.f.phase = 1;
        element->attr.f.x = 164;
        element->attr.f.y = 30;
        element->size.f.width_high = 0;
        element->size.f.height = 16;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x80);

        element = carda_alloc_element();
        element->draw = carda_draw_selected_entry_details;
        element->attr.f.phase = 1;
        element->attr.f.x = 30;
        element->attr.f.y = 142;
        element->size.f.width_high = 1;
        element->size.f.height = 52;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, 4);
    }
    g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
}

/**
 * @brief Update elements, the card sequence, input and scrolling for one frame.
 * @param frame Render buffer being built this frame.
 */
void carda_update_menu(CardaRenderBuffer* frame)
{
    s32 delta;

    carda_update_elements(frame);
    g_carda_icon_phase += 2;
    if ((g_carda_element1_state.attr.word & 0x7F) == 2)
    {
        carda_update_save_sequence();
    }
    if ((u16)g_pad_input == 0xFFFF)
    {
        g_pad_input = 0;
    }
    carda_handle_input();
    if (g_carda_scroll_frames != 0)
    {
        s32 base = g_carda_scroll_y; /* never used, but the compiled code only matches the original with it */
        delta = (g_carda_scroll_target_y - g_carda_scroll_y) / g_carda_scroll_frames;
        g_carda_scroll_frames -= 1;
        g_carda_scroll_y += delta;
    }
    else
    {
        g_carda_scroll_y = g_carda_scroll_target_y;
    }
}

/**
 * @brief Advance the active card sequence and react to its phase result.
 *
 * Starts the initial scan when no sequence is running, runs the sequence until
 * it stops asking to repeat, then handles a confirm press while a save is
 * pending and the sequence's result code.
 *
 * @return No value is returned; the s32 return type is historical.
 */
s32 carda_update_save_sequence(void)
{
    s32 phase;
    CardaElement* element;

    if (g_carda_mode >= 2 && g_carda_mode <= 3)
    {
        if (g_carda_save_step == NULL)
        {
            switch (g_carda_entry_state)
            {
            case 0xE9:
            case 0xEB:
            case 0xEC:
            case 0xED:
            case 0xEE:
            case 0xEF:
            case 0xF0:
            case 0xF1:
            case 0xF6:
            case 0xF8:
            case 0xF9:
            case 0xFA:
            case 0xFB:
            case 0xFC:
            case 0xFD:
            case 0xFF:
                return;
            default:
                g_carda_save_step = g_carda_steps_initial_scan;
                break;
            }
        }
    }
    else if (g_carda_entry_state >= 0x12 && g_carda_save_step == NULL && g_carda_entry_state != 0xF1)
    {
        g_carda_save_step = g_carda_steps_initial_scan;
    }

    do
    {
        phase = carda_advance_save_sequence();
    } while (phase == 3);

    if (D_801660F8 != 0 && (g_pad_input & 0x220))
    {
        g_carda_entry_state = 0xF9;
        D_801660F8 = 0;

        element = g_carda_element_pool;
        element->attr.f.state = CARDA_ELEMENT_OPENING;
        element->attr.f.phase = 1;
        element->attr.f.x = 16;
        element->attr.f.y = 76;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x20);
        element->size.f.width_high = 1;
        element->size.f.height = 72;
        carda_enable_choice_toggle();
        element->draw = carda_draw_format_prompt;
        carda_restart_card_sequence();
        return;
    }

    switch (phase)
    {
    case 0:
        break;
    case 2:
        g_carda_save_step = g_carda_steps_card_reset;
        break;
    case 4:
        if (g_carda_mode >= 2 && g_carda_mode <= 3)
        {
            g_carda_save_step = NULL;
        }
        else
        {
            g_carda_save_step = g_carda_steps_refresh_entries;
        }
        D_801660F8 = 0;
        break;
    case 5:
        if (g_carda_mode == 1 || g_carda_mode == 3)
        {
            g_carda_entry_state = 0xF9;
            if (g_carda_mode == 1)
            {
                g_carda_save_step = g_carda_steps_initial_scan;
            }
        }
        else
        {
            g_carda_entry_state = 0xF9;
            D_801660F8 = 0;

            element = g_carda_element_pool;
            element->attr.f.state = CARDA_ELEMENT_OPENING;
            element->attr.f.phase = 1;
            element->attr.f.x = 16;
            element->attr.f.y = 76;
            CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x20);
            element->size.f.width_high = 1;
            element->size.f.height = 72;
            carda_enable_choice_toggle();
            element->draw = carda_draw_format_prompt;
            carda_restart_card_sequence();
        }
        break;
    }
}

/**
 * @brief Handle CARDA browser navigation, confirm, and cancel input.
 * @return No value is returned; the s32 return type is historical.
 */
s32 carda_handle_input(void)
{
    s32 pending;
    s32 status;
    s32 count;
    CardaElement* element;

    if (g_carda_element_pool[1].attr.f.state == CARDA_ELEMENT_FREE)
    {
        g_carda_exit_requested = 1;
        return;
    }
    if (g_carda_exit_requested != 0)
    {
        return;
    }
    if (g_carda_element_pool[1].attr.f.state >= CARDA_ELEMENT_CLOSING)
    {
        return;
    }
    if (g_carda_element_pool[0].attr.f.state != CARDA_ELEMENT_FREE)
    {
        return;
    }
    if (g_carda_mode >= 2 && g_carda_mode <= 3)
    {
        return;
    }
    pending = g_carda_entry_state;
    if (pending == 0xFF)
    {
        return;
    }
    if (g_carda_entry_scan_active != 0)
    {
        return;
    }
    if (g_carda_io_busy != 0)
    {
        return;
    }
    if (*g_carda_save_step >= 6 && *g_carda_save_step <= 7)
    {
        return;
    }

    status = g_pad_input;
    if (status & 0x40)
    {
        g_field_card_overlay_mode = 3;
        play_menu_sfx(0x78, 0x80);
        carda_close_all_elements();
        return;
    }
    if (status & 0xA100)
    {
        play_menu_sfx(0x7D, 0x80);
        carda_switch_card();
        return;
    }
    if (pending >= 0x12)
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
            g_carda_selected_row -= 1;
            if (g_carda_selected_row < 0)
            {
                g_carda_selected_row = g_carda_entry_state - 1;
            }
        }
        if (g_pad_input & 0x4000)
        {
            g_carda_selected_row += 1;
            if (g_carda_selected_row >= g_carda_entry_state)
            {
                g_carda_selected_row = 0;
            }
        }
        count -= 1;
    }

    if (g_pad_input & 0x5000)
    {
        carda_commit_selected_entry();
        play_menu_sfx(0x7D, 0x80);
        carda_scroll_to_selection();
        return;
    }

    if (g_pad_input & 0x220)
    {
        if (g_carda_mode == 1)
        {
            if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, 0xC) == 0)
            {
                if (g_save_slot_index == 0xFF || g_carda_selected_save_slot_id == g_save_slot_index)
                {
                    element = carda_alloc_element();
                    element->attr.f.phase = 1;
                    element->attr.f.x = 16;
                    element->attr.f.y = 90;
                    element->size.f.width_high = 1;
                    element->size.f.height = 44;
                    CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x20);
                    carda_build_save_file();
                    carda_enable_choice_toggle();
                    element->draw = carda_draw_load_prompt;
                    carda_restart_card_sequence();
                    play_menu_sfx(0x7E, 0x80);
                    return;
                }
            }
        }
        else
        {
            if (strncmp(g_new_save_entry_prefix, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, 8) == 0)
            {
                element = carda_alloc_element();
                element->attr.f.phase = 1;
                element->attr.f.x = 16;
                element->attr.f.y = 90;
                element->size.f.width_high = 1;
                element->size.f.height = 44;
                CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x20);
                carda_build_save_file();
                carda_enable_choice_toggle();
                element->draw = carda_draw_save_prompt;
                carda_restart_card_sequence();
                play_menu_sfx(0x7E, 0x80);
                return;
            }
            if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, 0xC) == 0)
            {
                element = carda_alloc_element();
                element->attr.f.phase = 1;
                element->attr.f.x = 16;
                element->attr.f.y = 90;
                element->size.f.width_high = 1;
                element->size.f.height = 44;
                CARDA_SET_ELEMENT_WIDTH_LOW(element, 0x20);
                carda_build_save_file();
                carda_enable_choice_toggle();
                element->draw = carda_draw_overwrite_prompt;
                carda_restart_card_sequence();
                play_menu_sfx(0x7E, 0x80);
                return;
            }
        }
        play_menu_sfx(0x78, 0x80);
    }
}

/**
 * @brief Switch to the other memory card and restart its directory scan.
 */
void carda_switch_card(void)
{
    D_801660F8 = 0;
    g_carda_save_step = NULL;
    g_carda_entry_state = 0xFF;
    g_carda_scroll_frames = 0;
    g_carda_scroll_target_y = 0;
    g_carda_scroll_y = 0;
    g_carda_selected_row = 0;
    g_carda_selection_status = 0;
    g_carda_card_slot ^= 1;
    carda_reset_entry_ranks();
    carda_release_secondary_handles();
    carda_release_primary_handles();
}

/**
 * @brief Restore the field fade target and put every live UI element into its closing state.
 */
void carda_close_all_elements(void)
{
    CardaElement* element;
    s32 i;

    field_restore_fade_target();
    element = g_carda_element_pool;
    for (i = 0; i < CARDA_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.f.state != CARDA_ELEMENT_FREE)
        {
            element->attr.f.state = CARDA_ELEMENT_CLOSING;
            element->attr.f.phase = CARDA_ELEMENT_PHASE_STEPS;
        }
    }
}

/**
 * @brief Move the list scroll target to keep the selected row visible.
 */
void carda_scroll_to_selection(void)
{
    s32 base;
    s32 delta;

    base = g_carda_selected_row * CARDA_ENTRY_ROW_HEIGHT;
    delta = base - g_carda_scroll_y;
    if (delta >= 0x4B)
    {
        g_carda_scroll_target_y = base - 0x46;
        g_carda_scroll_frames = 4;
    }
    if (delta < 0)
    {
        g_carda_scroll_target_y = g_carda_selected_row * CARDA_ENTRY_ROW_HEIGHT;
        g_carda_scroll_frames = 4;
    }
}

/**
 * @brief Run the UI element update/draw pass.
 * @param frame Render buffer being built this frame.
 */
void carda_update_elements(CardaRenderBuffer* frame)
{
    carda_update_and_draw_elements(frame);
}

/**
 * @brief Build the primitive list for the memory-card entry browser body.
 *
 * Dispatches on the current status code @c g_carda_entry_state to draw a status/prompt
 * glyph, or, in the default case, renders one row per card entry (rank digits,
 * icons, protect/copy state) plus the highlight bar for the selected row.
 *
 * @param ot Ordering table the primitives are linked into.
 * @param prim GPU packet cursor to emit primitives into.
 * @param x_offset Horizontal scroll offset subtracted from each glyph x.
 * @param y_offset Vertical scroll offset subtracted from each row y.
 * @return The advanced packet cursor past the last emitted primitive.
 */
void* carda_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    if (g_carda_element_pool[0].attr.f.state != CARDA_ELEMENT_FREE)
    {
        if (g_carda_entry_state >= 0xF8)
        {
            if (g_carda_entry_state < 0xFE)
            {
                return prim;
            }
            if (g_carda_entry_state == 0xFF)
            {
                return prim;
            }
        }
    }

    switch (g_carda_entry_state)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_no_lom_save_data, 26), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_card_unformatted, 90), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xF6:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B07A, 33), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFF:
    {
        s32 x = -x_offset + 0x96;
        u16* text_table = &g_carda_text_check_memory_card;

        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0), 4, x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 15), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 89), 4, x, 0x1C - y_offset, 2);
        break;
    }
    case 0xFA:
    {
        s32 x = -x_offset + 0x96;
        u16* text_table;

        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_not_enough_blocks, 1), 4, x, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(g_carda_text_not_enough_blocks, 1);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 45), 4, x, 0x10 - y_offset, 2);
        break;
    }
    case 0xF7:
    {
        s32 x = -x_offset + 0x96;
        u16* text_table;

        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_not_enough_blocks, 1), 4, x, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(g_carda_text_not_enough_blocks, 1);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 48), 4, x, 0x10 - y_offset, 2);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 49), 4, x, 0x20 - y_offset, 2);
        break;
    }
    case 0xFD:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_no_memory_card, 2), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_card_access_failed, 8), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_no_save_data, 9), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFE:
        break;
    default:
    {
        s32 row_y;
        s32 i;

        if (g_carda_entry_scan_active != 0)
        {
            s32 x = -x_offset + 0x96;
            u16* text_table = &g_carda_text_check_memory_card;

            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0), 4, x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 15), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 89), 4, x, 0x1C - y_offset, 2);
            break;
        }

        i = 0;
        if (i < g_carda_entry_state)
        {
            s32 base_x;
            u16 marker_offset;
            DVECTOR pos;
            u16* text_table;
            s32 marker_x;
            s32 marker_x_bits;
            s32 color;
            s32 label_x;

            text_table = &g_carda_text_check_memory_card;
            base_x = -x_offset;
            do
            {
                color = 4;
                marker_x = base_x + 0xD6;
                label_x = 1 - x_offset;
                row_y = ((i * CARDA_ENTRY_ROW_HEIGHT) - y_offset) - g_carda_scroll_y + 1;
                if (row_y >= -13 && row_y <= 87)
                {
                    if (g_carda_entry_ranks[i] >= 0)
                    {
                        pos.vx = base_x + 0x86;
                        pos.vy = row_y;
                        prim = func_800A8A78(ot, prim, g_carda_entry_suffix_values[i], color, &pos, 0);
                        prim = func_800A88A0(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_number_prefix), color, base_x + 0x70, row_y, 0);
                        pos.vy = row_y;
                        pos.vx = marker_x;
                        if ((g_carda_rank_count - 1) == g_carda_entry_ranks[i])
                        {
                            marker_offset = text_table[27];
                            marker_x_bits = marker_x << 16;
                            prim = func_800A88A0(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, marker_offset), color, marker_x_bits >> 16, row_y, 0);
                        }
                        else if (g_carda_entry_ranks[i] < 2)
                        {
                            marker_offset = text_table[28];
                            marker_x_bits = marker_x << 16;
                            prim = func_800A88A0(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, marker_offset), color, marker_x_bits >> 16, row_y, 0);
                        }
                        if (*carda_skip_hex_digits(g_carda_entries[g_carda_card_slot][i].name + 12) == 0x2B)
                        {
                            prim = func_800A88A0(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_plus_marker), color, 0x10C - x_offset, row_y, 1);
                        }
                    }
                    if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][i].name, 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_mana), color, label_x, row_y, 0);
                    }
                    else if (strncmp(g_lom_alt_save_filename_prefix, g_carda_entries[g_carda_card_slot][i].name, 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_alt_save), color, label_x, row_y, 0);
                    }
                    else if (strncmp(g_new_save_entry_prefix, g_carda_entries[g_carda_card_slot][i].name, 8) == 0)
                    {
                        prim = func_800A88A0(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_new_save), color, label_x, row_y, 0);
                    }
                    else if (strncmp(D_800ECFD0, g_carda_entries[g_carda_card_slot][i].name, 9) == 0)
                    {
                        prim = func_800A88A0(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, D_8014B09E), color, label_x, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_other_game), color, label_x, row_y, 0);
                    }
                }
                i++;
            } while (i < g_carda_entry_state);
        }

        row_y = ((g_carda_selected_row * CARDA_ENTRY_ROW_HEIGHT) - y_offset) - g_carda_scroll_y;
        if (g_carda_entry_scan_active == 0)
        {
            TILE* tile = (TILE*)prim;

            *(u32*)&tile->r0 = 0xF080F0;
            setlen(tile, 3);
            setcode(tile, 0x62);
            tile->w = 0x12C;
            setXY0(tile, 0, row_y);
            tile->h = 0xE;
            addPrim(ot, tile);
            prim = tile + 1;
        }
        break;
    }
    }
    return prim;
}

/**
 * @brief Draw the mode-dependent CARDA header label.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see matching: 100.00%
 */
void* carda_draw_header_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */

    if (g_carda_mode >= 2 && g_carda_mode <= 3)
    {
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0D4, 78), 4, -x_offset + 0x80, -y_offset, 2);
    }
    else if (g_carda_mode == 1)
    {
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_save, 22), 4, -x_offset + 0x38, -y_offset, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B042, 5), 4, -x_offset + 0x38, -y_offset, 2);
    }

    return prim;
}

/**
 * @brief Draw the first memory-card slot label, highlighted while slot 1 is selected.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_card_slot0_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */
    TILE* tile;

    if (g_carda_card_slot != 0)
    {
        tile = (TILE*)prim;
        *(u32*)&tile->r0 = 0x101010;
        setlen(tile, 3);
        setcode(tile, 0x62);
        setXY0(tile, 0, 0);
        setWH(tile, 0x80, 0x10);
        addPrim(ot, tile);
        prim = tile + 1;
    }
    return func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_card_slot_1, 6), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the second memory-card slot label, highlighted while slot 0 is selected.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_card_slot1_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */
    TILE* tile;

    if (g_carda_card_slot == 0)
    {
        tile = (TILE*)prim;
        *(u32*)&tile->r0 = 0x101010;
        setlen(tile, 3);
        setcode(tile, 0x62);
        setXY0(tile, 0, 0);
        setWH(tile, 0x80, 0x10);
        addPrim(ot, tile);
        prim = tile + 1;
    }
    return func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_card_slot_2, 7), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the selected memory-card entry's details.
 *
 * Shows the new-save prompt, a notice, the selected LOM save's party icons,
 * play time, title and location, or the raw two-line file title of any other
 * game's save.
 *
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see matching: 100.00%
 */
void* carda_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    void* result;
    DVECTOR pos;
    u8 name[0x100];
    s32 party_icon[3];
    DVECTOR unused; /* never used, but the original stack frame reserves it */

    result = prim;
    if (g_carda_selection_status == 0)
    {
        return result;
    }
    if (g_carda_entry_scan_active != 0)
    {
        return result;
    }
    if (g_carda_selection_status != 3 && g_carda_entry_state != 0xFA && g_carda_entry_state < 0x10)
    {
        if (g_carda_selection_status == 2)
        {
            s32 x = -x_offset;
            u16* text_table;

            result = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_new_save_prompt, 20), 4, x, -y_offset, 0);
            text_table = CARDA_TEXT_TABLE(g_carda_text_new_save_prompt, 20);
            return func_800A88A0(result, ot, CARDA_TEXT(text_table, 21), 4, x, 0x10 - y_offset, 0);
        }
        else if (g_carda_selection_status == 4)
        {
            return func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B092, 45), 4, -x_offset, -y_offset, 0);
        }
        else
        {
            if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, 0xC) == 0)
            {
                if (g_save_slot_index == 0xFF || g_carda_selected_save_slot_id == g_save_slot_index || g_carda_selected_save_slot_id == 0xFF)
                {
                    CardaSaveMetadata* save = &g_carda_selected_save_metadata;
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
                    g_carda_icon_palette = save->icon_palette;

                    present_count = 0;
                    for (i = 0; i < 3; i++)
                    {
                        if (party_icon[i] != CARDA_NO_ICON)
                        {
                            present_count += 1;
                        }
                    }

                    switch (present_count)
                    {
                    case 2:
                        step = 0x20;
                        half_step = 0x10;
                        time_val = g_carda_icon_phase;
                        if (g_carda_icon_phase < 0)
                        {
                            time_val = g_carda_icon_phase + 0x1F;
                        }
                        g_carda_icon_phase -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        g_carda_icon_phase %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        g_carda_icon_phase = 0x1F;
                        break;
                    }

                    i = 0;
                    j = i;
                    for (; j < 3; j++)
                    {
                        base_y = i * half_step;
                        base_x = base_y + half_step;
                        if (party_icon[j] != CARDA_NO_ICON)
                        {
                            s32 adjust = step;
                            s32 rem;
                            s32 hi;
                            s32 delta;

                            if ((g_carda_icon_phase >= base_y && g_carda_icon_phase < base_x && (delta = g_carda_icon_phase - base_y, 1)) ||
                                (rem = base_x % (half_step * present_count),
                                 g_carda_icon_phase >= rem && g_carda_icon_phase < (hi = rem + half_step) && (delta = hi - g_carda_icon_phase, 1)))
                            {
                                adjust += delta;
                            }
                            result = carda_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, party_icon[j], i, j);
                            total += adjust;
                            i += 1;
                        }
                    }

                    {
                        CardaSaveMetadata* shown_save = &g_carda_selected_save_metadata;
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        base_y = shown_save->playtime;

                        pos.vx = (s16)(x + 0x70);
                        pos.vy = (s16)y;
                        hours = base_y / 216000;
                        result = func_800A8A78(ot, result, hours, 4, &pos, 1);
                        result = func_800A88A0(result, ot, FIELD_UI_TEXT_AT(g_text_time_separator_offset_bytes, 25), 4, x + 0x6F, y, 0);
                        base_y = (base_y / 3600) - (hours * 0x3C);
                        if (base_y < 0xA)
                        {
                            pos.vx = (s16)(x + 0x7D);
                            pos.vy = (s16)y;
                            result = func_800A8A78(ot, result, 0, 4, &pos, 1);
                        }
                        pos.vx = (s16)(x + 0x85);
                        pos.vy = (s16)y;
                        result = func_800A88A0(func_800A88A0(func_800A8A78(ot, result, base_y, 4, &pos, 1), ot, shown_save->title, 4, x + 0x54, y + 0x10, 0),
                                               ot, CARDA_TEXT(g_carda_location_names, shown_save->location), 4, x + 0x54, y + 0x20, 0);
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, CARDA_TEXT_AT(g_carda_text_version_error, 42), 4, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 j;

                {
                    u8* text_base;
                    carda_terminate_multibyte_text(D_80166124);
                    text_base = D_80166124;
                    text_base -= 4;
                    if (text_base[0x24] == 0 || text_base[0x24] >= 0x80)
                    {
                        do
                        {
                            do
                            {
                                for (j = 0; j < 0x20; j++)
                                {
                                    name[j] = text_base[j + 4];
                                }
                                name[j] = 0;
                                result = carda_draw_cached_text(result, ot, name, -x_offset, -y_offset, 4, 0);

                                for (j = 0; j < 0x20; j++)
                                {
                                    name[j] = g_carda_selected_file_header.title_line_2[j];
                                }
                                name[j] = 0;
                                result = carda_draw_cached_text(result, ot, name, -x_offset, -y_offset + 0x10, 4, 0);
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
 */
void carda_terminate_multibyte_text(void* text)
{
    u8* p;
    s32 i;

    p = (u8*)text;
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
 * @brief Draw a centred FIELD UI string.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_field_notice(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */

    return func_800A88A0(prim, ot, FIELD_UI_TEXT_AT(D_800EC3D0, 6), 5, 0x80 - x_offset, -y_offset, 2);
}

/**
 * @brief Mark all eight UI elements as inactive and reset the element counter.
 */
void carda_clear_elements(void)
{
    CardaElement* p;
    s32 i;

    g_menu_element_counter = 0x20;
    p = g_carda_element_pool;
    for (i = 0; i < CARDA_ELEMENT_COUNT; i++)
    {
        p->attr.f.state = CARDA_ELEMENT_FREE;
        p++;
    }
}

/**
 * @brief Activate and return the first free UI element.
 * @return First free element, or the pool head if all slots are busy.
 */
CardaElement* carda_alloc_element(void)
{
    CardaElement* p;
    s32 i;

    p = g_carda_element_pool;
    for (i = 0; i < CARDA_ELEMENT_COUNT; i++, p++)
    {
        if (p->attr.f.state == CARDA_ELEMENT_FREE)
        {
            p->attr.f.state = CARDA_ELEMENT_OPENING;
            return p;
        }
    }
    return g_carda_element_pool;
}

/**
 * @brief Advance and draw the eight pool elements for one frame.
 *
 * Emits the list scroll arrows, then links a draw-environment packet for each
 * live element into the ordering table and animates it by state: an opening
 * window grows, an open window holds, a closing window shrinks, and a closed
 * window counts down to free.
 *
 * @param frame Render buffer being built; prim_cursor is read on entry and written back on exit.
 */
void carda_update_and_draw_elements(CardaRenderBuffer* frame)
{
    void* prim;
    u_long* ot;
    CardaElement* element;
    s32 i;
    DRAWENV draw_env;
    s32 scaled_width;
    s32 scaled_height;

    prim = frame->prim_cursor;
    ot = frame->overlay_ot;

    if (g_carda_element_pool[1].draw == carda_draw_entry_list)
    {
        if ((g_carda_entry_state < 0x10) && (g_carda_element_pool[1].attr.f.state == CARDA_ELEMENT_OPEN))
        {
            if ((g_carda_entry_state * CARDA_ENTRY_ROW_HEIGHT) > (g_carda_scroll_y + 0x58))
            {
                prim = func_800AE76C(prim, ot, 0x12E, 0x82, 0);
            }
            if (g_carda_scroll_y != 0)
            {
                prim = func_800AE76C(prim, ot, 0x12E, 0x3A, 1);
            }
        }
    }
    else if ((g_carda_element_pool[1].draw == carda_draw_item_list) && (g_carda_element_pool[1].attr.f.state == CARDA_ELEMENT_OPEN))
    {
        if (((g_carda_received_item_count * CARDA_ENTRY_ROW_HEIGHT) - g_carda_scroll_y) >= 0x8D)
        {
            prim = func_800AE76C(prim, ot, 0x118, 0xBE, 0);
        }
        if (g_carda_scroll_y != 0)
        {
            prim = func_800AE76C(prim, ot, 0x118, 0x3E, 1);
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

    element = g_carda_element_pool;
    for (i = 0; i < CARDA_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.f.state != CARDA_ELEMENT_FREE)
        {
            SetDrawEnv((DR_ENV*)prim, &draw_env);
            addPrim(ot, prim);
            prim = (DR_ENV*)prim + 1;

            switch (element->attr.f.state)
            {
            case CARDA_ELEMENT_OPENING:
                g_pad_input = 0;
                {
                    u32 width_low = CARDA_ELEMENT_WIDTH_LOW(element);
                    s32 width = CARDA_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.f.phase) / 8;
                    scaled_height = (element->size.f.height * element->attr.f.phase) / 8;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.f.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.f.x;
                    u32 width_low = CARDA_ELEMENT_WIDTH_LOW(element);

                    prim = func_800AD850(prim, ot, x + (CARDA_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                         element->attr.f.y + (element->size.f.height - scaled_height) / 2, scaled_width, scaled_height, frame->clear_rect.y,
                                         i == 0);
                }
                element->attr.f.phase++;
                if (element->attr.f.phase == CARDA_ELEMENT_PHASE_STEPS)
                {
                    field_reset_input_repeat();
                    element->attr.f.state = CARDA_ELEMENT_OPEN;
                }
                break;

            case CARDA_ELEMENT_OPEN:
                prim = element->draw(ot, prim, 0, 0);
                {
                    u32 width_low = CARDA_ELEMENT_WIDTH_LOW(element);

                    prim = func_800AD850(prim, ot, element->attr.f.x, element->attr.f.y, CARDA_ELEMENT_WIDTH(element, width_low), element->size.f.height,
                                         frame->clear_rect.y, i == 0);
                }
                if (element->attr.f.phase != 0)
                {
                    element->attr.f.phase--;
                }
                break;

            case CARDA_ELEMENT_CLOSING:
                g_pad_input = 0;
                {
                    u32 width_low = CARDA_ELEMENT_WIDTH_LOW(element);
                    s32 width = CARDA_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.f.phase) / 8;
                    scaled_height = (element->size.f.height * element->attr.f.phase) / 8;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.f.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.f.x;
                    u32 width_low = CARDA_ELEMENT_WIDTH_LOW(element);

                    prim = func_800AD850(prim, ot, x + (CARDA_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                         element->attr.f.y + (element->size.f.height - scaled_height) / 2, scaled_width, scaled_height, frame->clear_rect.y,
                                         i == 0);
                }
                element->attr.f.phase--;
                if (element->attr.f.phase == 0)
                {
                    element->attr.f.phase = 3;
                    element->attr.f.state = CARDA_ELEMENT_CLOSED;
                }
                break;

            case CARDA_ELEMENT_CLOSED:
                g_pad_input = 0;
                element->attr.f.phase--;
                if (element->attr.f.phase == 0)
                {
                    element->attr.f.state = CARDA_ELEMENT_FREE;
                }
                break;
            }
        }
    }

    frame->prim_cursor = prim;
}

/**
 * @brief Free the element in pool slot 0.
 */
void carda_deactivate_primary_element(void)
{
    g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
}

/**
 * @brief Append one encoded CARDA string to another.
 * @param dest Destination text buffer.
 * @param src Source text buffer.
 */
void carda_text_append(u8* dest, u8* src)
{
    s32 dst_len;
    s32 src_len;
    s32 i;

    dst_len = carda_text_byte_length(dest);
    src_len = carda_text_byte_length(src);
    for (i = 0; i < src_len; i++)
    {
        dest[dst_len + i] = src[i];
    }
    dest[dst_len + i] = 0;
}

/**
 * @brief Measure an encoded CARDA string in bytes.
 * @param text Encoded text buffer.
 * @return Encoded byte length excluding the terminator.
 */
s32 carda_text_byte_length(u8* text)
{
    u8* p;
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
 * @brief Copy one encoded CARDA string including its terminator.
 * @param dest Destination text buffer.
 * @param src Source text buffer.
 */
void carda_text_copy(u8* dest, u8* src)
{
    u8* p;
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
