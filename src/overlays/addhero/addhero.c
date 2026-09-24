#include "field_text.h"
#include "addhero_internal.h"

/**
 * @brief Address of the ADDHERO text whose table offset is @p offset.
 * @note Summed as integers, offset first, like the original list drawing code.
 */
#define ADDHERO_TEXT_BY_OFFSET(table, offset) ((u8*)((s32)(offset) + (s32)(table)))

/**
 * @brief Reset overlay state and build the initial UI elements.
 * @param work_base Work-RAM base (always 0x80170000); stored in g_addhero_work_ram_base, unused so far.
 * @param mode Mode selector, stored in g_addhero_mode.
 * @see decomp.me (100%)
 */
void addhero_init(s32 work_base, s32 mode)
{
    RECT rect;

    g_addhero_mode = mode;
    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
    g_addhero_card_slot = 0;

    addhero_reset_entry_ranks();
    g_addhero_result = 3;
    addhero_init_card_events();
    g_addhero_icon_phase = 0;
    field_set_default_fade_target();

    setRECT(&rect, OVERLAY_INIT_CLEAR_VRAM_X, OVERLAY_INIT_CLEAR_VRAM_Y, OVERLAY_INIT_CLEAR_VRAM_W, OVERLAY_INIT_CLEAR_VRAM_H);

    ClearImage(&rect, 0, 0, 0);
    addhero_reset_glyph_cache();

    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = 0;
    g_addhero_io_busy = 0;
    g_addhero_frame_parity = 0;
    g_addhero_exit_requested = 0;

    field_reset_input_repeat();
    addhero_build_ui_elements();

    g_addhero_work_ram_base = work_base;
}

/**
 * @brief Run one frame: tear down and exit if requested, else update and render.
 * @param draw_state Frame drawing context passed through to the element renderer.
 * @return Non-zero exit code when exiting, 0 while running.
 * @see decomp.me (100%)
 */
s32 addhero_state_step(AddheroDrawState* draw_state)
{
    if (g_addhero_exit_requested != 0)
    {
        addhero_shutdown_card_events();
        field_text_reset_windows();
        DrawSync(0);
        return g_addhero_exit_requested;
    }

    field_text_reset_scratch();
    addhero_begin_glyph_cache_frame();
    addhero_update_state(draw_state);
    addhero_evict_unused_glyphs();
    field_text_upload_immediate_cache();
    g_addhero_frame_parity ^= 1;
    return 0;
}

/**
 * @brief Reset scroll/selection state and populate the UI element pool for the current mode.
 * @note mode != 0: transfer layout (status + two card-slot labels). mode == 0: full browser
 *       (entry list, mode glyph, two slot labels, entry details).
 * @see decomp.me (100%)
 */
void addhero_build_ui_elements(void)
{
    AddheroElement* element;
    s32 unused[2]; /* never used, but the original stack frame reserves it */

    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_selected_row = 0;
    g_addhero_selection_status = 0;
    g_addhero_pad_work_ptr = g_pad_ctx + 0xCE0;
    addhero_clear_elements();
    g_addhero_load_flow_active = 0;
    if (g_addhero_mode != 0)
    {
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
        element = addhero_alloc_element();
        element->draw_handler = addhero_draw_transfer_status;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = 0x10;
        element->attr.bits.y = 0x61;
        element->size.bits.width_high = 1;
        element->size.bits.height = 0x2C;
        ADDHERO_SET_ELEMENT_WIDTH_LOW(element, 0x20);

        element = addhero_alloc_element();
        element->draw_handler = addhero_draw_card_slot0_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = 0x18;
        element->attr.bits.y = 0x4D;
        element->size.bits.width_high = 0;
        element->size.bits.height = 0x10;
        ADDHERO_SET_ELEMENT_WIDTH_LOW(element, 0x80);

        element = addhero_alloc_element();
        element->draw_handler = addhero_draw_card_slot1_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = 0xA0;
        element->attr.bits.y = 0x4D;
        element->size.bits.width_high = 0;
        element->size.bits.height = 0x10;
        ADDHERO_SET_ELEMENT_WIDTH_LOW(element, 0x80);
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        return;
    }

    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    element = addhero_alloc_element();
    element->draw_handler = addhero_draw_entry_list;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 0x1C;
    element->attr.bits.y = 0x32;
    element->size.bits.width_high = 1;
    element->size.bits.height = 0x58;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(element, 8);
    element->size.bits.scrollable = 1;

    element = addhero_alloc_element();
    element->draw_handler = addhero_draw_mode_glyph;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 0x24;
    element->attr.bits.y = 0x0A;
    element->size.bits.width_high = 0;
    element->size.bits.height = 0x10;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(element, 0xF0);

    element = addhero_alloc_element();
    element->draw_handler = addhero_draw_card_slot0_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 0x18;
    element->attr.bits.y = 0x1E;
    element->size.bits.width_high = 0;
    element->size.bits.height = 0x10;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(element, 0x80);

    element = addhero_alloc_element();
    element->draw_handler = addhero_draw_card_slot1_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 0xA0;
    element->attr.bits.y = 0x1E;
    element->size.bits.width_high = 0;
    element->size.bits.height = 0x10;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(element, 0x80);

    element = addhero_alloc_element();
    element->draw_handler = addhero_draw_selected_entry_details;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 0x1E;
    element->attr.bits.y = 0x8E;
    element->size.bits.width_high = 1;
    element->size.bits.height = 0x34;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(element, 4);
    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
}

/**
 * @brief Run one frame of overlay logic: update elements, advance the load
 *        sequence when armed, sample pad input, and step the scroll animation.
 * @param draw_state Frame drawing context passed to the element renderer.
 * @see decomp.me (100%)
 */
void addhero_update_state(AddheroDrawState* draw_state)
{
    addhero_update_elements(draw_state);
    g_addhero_icon_phase += 2;
    if (g_addhero_element1.attr.bits.state == ADDHERO_ELEMENT_STATE_ACTIVE && g_addhero_element1.attr.bits.transition_step == 0)
    {
        addhero_update_load_sequence();
    }
    if ((u16)g_pad_input == 0xFFFF)
    {
        g_pad_input = 0;
    }
    addhero_handle_input();
    if (g_addhero_scroll_frames != 0)
    {
        g_addhero_scroll_y += (g_addhero_scroll_target_y - g_addhero_scroll_y) / g_addhero_scroll_frames--;
    }
    else
    {
        g_addhero_scroll_y = g_addhero_scroll_target_y;
    }
}

/**
 * @brief Drive the card load/scan state machine one frame, mapping its result
 *        code onto the next load step and any error entry-state sentinel.
 * @return Unspecified; callers ignore the return value.
 * @see decomp.me (100%)
 */
s32 addhero_update_load_sequence(void)
{
    s32 result;

    if (g_addhero_entry_state >= 0x10)
    {
        if (g_addhero_load_step == NULL)
        {
            g_addhero_load_step = &g_addhero_loadseq_start;
        }
    }

    do
    {
        result = addhero_advance_load_sequence();
    } while (result == ADDHERO_LOAD_RESULT_CONTINUE);

    if ((g_addhero_load_flow_active != 0) && (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK))
    {
        if (g_addhero_mode == 0)
        {
            g_addhero_entry_state = 0xF9;
        }
        else
        {
            g_addhero_entry_state = 0xF8;
        }
        g_addhero_load_step = g_addhero_loadseq_abort;
    }
    else
    {
        switch (result)
        {
        case ADDHERO_LOAD_RESULT_NONE:
            break;
        case ADDHERO_LOAD_RESULT_COMPLETE:
            g_addhero_load_step = g_addhero_loadseq_done;
            g_addhero_load_flow_active = 0;
            break;
        case ADDHERO_LOAD_RESULT_CARD_ERROR:
            if (g_addhero_mode == 0)
            {
                g_addhero_entry_state = 0xF9;
            }
            else
            {
                g_addhero_entry_state = 0xF8;
            }
            /* fallthrough */
        case ADDHERO_LOAD_RESULT_ABORT:
            g_addhero_load_step = g_addhero_loadseq_abort;
            break;
        }
    }
}

/**
 * @brief Handle browser input, entry navigation, and load confirmation.
 * @return Unspecified; callers ignore the return value.
 */
s32 addhero_handle_input(void)
{
    s32 entry_count;
    s32 input;
    s32 move_count;
    AddheroElement* prompt;
    struct DIRENTRY* selected_entry;

    if (g_addhero_element_pool[1].attr.bits.state == ADDHERO_ELEMENT_STATE_INACTIVE)
    {
        g_addhero_exit_requested = g_addhero_result;
        return;
    }
    if (g_addhero_exit_requested != 0)
    {
        return;
    }
    if (g_addhero_element_pool[1].attr.bits.state >= ADDHERO_ELEMENT_STATE_CLOSING)
    {
        return;
    }
    if (g_addhero_element_pool[0].attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
    {
        return;
    }

    entry_count = g_addhero_entry_state;
    if (entry_count == ADDHERO_ENTRY_STATE_IDLE)
    {
        return;
    }
    if (g_addhero_entry_scan_active != 0)
    {
        return;
    }
    if (g_addhero_io_busy != 0)
    {
        return;
    }
    if (*g_addhero_load_step >= 6 && *g_addhero_load_step <= 7)
    {
        return;
    }
    if (g_addhero_mode != 0)
    {
        return;
    }

    input = g_pad_input;
    if (input & PAD_BTN_CIRCLE)
    {
        D_80122718 = 3;
        play_menu_sfx(0x78, 0x80);
        addhero_close_all_elements();
        return;
    }
    if (input & ADDHERO_CARD_SWITCH_BUTTON_MASK)
    {
        play_menu_sfx(0x7D, 0x80);
        addhero_reset_state();
        return;
    }
    if (entry_count >= ADDHERO_ENTRY_COUNT_LIMIT)
    {
        return;
    }

    move_count = 1;
    if (input & PAD_BTN_R1)
    {
        g_pad_input = PAD_BTN_DOWN;
        move_count = 1;
    }
    if (g_pad_input & PAD_BTN_L1)
    {
        g_pad_input = PAD_BTN_UP;
        move_count = 1;
    }

    while (move_count != 0)
    {
        if (g_pad_input & PAD_BTN_UP)
        {
            g_addhero_selected_row--;
            if (g_addhero_selected_row < 0)
            {
                g_addhero_selected_row = g_addhero_entry_state - 1;
            }
        }
        if (g_pad_input & PAD_BTN_DOWN)
        {
            g_addhero_selected_row++;
            if (g_addhero_selected_row >= g_addhero_entry_state)
            {
                g_addhero_selected_row = 0;
            }
        }
        move_count--;
    }

    if (g_pad_input & (PAD_BTN_UP | PAD_BTN_DOWN))
    {
        addhero_commit_selected_entry();
        play_menu_sfx(0x7D, 0x80);
        addhero_scroll_to_selection();
        return;
    }

    if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
    {
        selected_entry = &g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row];
        if (strncmp(g_lom_save_filename_prefix, selected_entry->name, 0xC) == 0)
        {
            if ((g_addhero_entry_metadata.hero_id != ((AddheroRecord*)g_pad_ctx)->hero_id) &&
                ((g_save_slot_index == 0xFF) || (g_addhero_entry_metadata.owner_id == g_save_slot_index)))
            {
                prompt = addhero_alloc_element();
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.x = 0x10;
                prompt->attr.bits.y = 0x61;
                prompt->size.bits.width_high = 1;
                prompt->size.bits.height = 0x1E;
                ADDHERO_SET_ELEMENT_WIDTH_LOW(prompt, 0x20);
                addhero_enable_choice_toggle();
                prompt->draw_handler = addhero_draw_load_prompt;
                addhero_restart_load_sequence();
                play_menu_sfx(0x7E, 0x80);
                return;
            }
        }
        play_menu_sfx(0x78, 0x80);
    }
}

/**
 * @brief Reset scroll/selection state and flip to the other card slot, then
 *        clear ranks and pad input to restart browsing.
 * @see decomp.me (100%)
 */
void addhero_reset_state(void)
{
    g_addhero_load_flow_active = 0;
    g_addhero_load_step = NULL;
    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_selected_row = 0;
    g_addhero_selection_status = 0;
    g_addhero_card_slot ^= 1;
    addhero_reset_entry_ranks();
    field_reset_input_repeat();
    g_pad_input = 0;
}

/**
 * @brief Put every active pool element into the closing transition (state 3,
 *        phase 0x40) so they animate out.
 * @see decomp.me (100%)
 */
void addhero_close_all_elements(void)
{
    AddheroElement* element;
    s32 slot;

    field_restore_fade_target();
    element = g_addhero_element_pool;
    for (slot = 0; slot < ADDHERO_ELEMENT_COUNT; slot++, element++)
    {
        if (element->attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
        {
            element->attr.bits.state = ADDHERO_ELEMENT_STATE_CLOSING;
            element->attr.bits.transition_step = 8;
        }
    }
}

/**
 * @brief Retarget the list scroll so the selected row stays on screen,
 *        animating over four frames when it falls above or below the window.
 * @see decomp.me (100%)
 */
void addhero_scroll_to_selection(void)
{
    s32 row_y;
    s32 relative_y;

    row_y = (g_addhero_selected_row * 7) << 1;
    relative_y = row_y - g_addhero_scroll_y;

    if (relative_y >= 0x4B)
    {
        g_addhero_scroll_target_y = row_y - 0x46;
        g_addhero_scroll_frames = 4;
    }
    if (relative_y < 0)
    {
        g_addhero_scroll_target_y = row_y;
        g_addhero_scroll_frames = 4;
    }
}

/**
 * @brief Thin wrapper that runs the element update/draw pass on the active
 *        draw state.
 * @param draw_state Frame drawing context to update.
 * @see decomp.me (100%)
 */
void addhero_update_elements(AddheroDrawState* draw_state)
{
    addhero_update_and_draw_elements(draw_state);
}

/**
 * @brief Draw the save-entry browser: status/error screens by entry-state
 *        sentinel, the per-row entry list with rank glyphs, and the selection
 *        highlight tile.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index within the ordering table.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset applied to each row.
 * @return The updated primitive pointer after linking this frame's glyphs.
 */
void* addhero_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 entry_state = g_addhero_entry_state;

    switch (entry_state)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_f8, 26), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_f8, 26), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case ADDHERO_ENTRY_STATE_CARD_FULL:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_fa, 1), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case ADDHERO_ENTRY_STATE_CARD_IO_ERROR:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_fd, 2), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_fb, 8), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_fc, 9), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFE:
        break;
    case ADDHERO_ENTRY_STATE_IDLE:
    {
        s32 message_x;
        u16* text_table;

        message_x = -x_offset + 0x84;
        text_table = &g_addhero_glyph_table;
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 0), 4, message_x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 15), 4, message_x, ADDHERO_ENTRY_ROW_HEIGHT - y_offset, 2);
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 89), 4, message_x, (ADDHERO_ENTRY_ROW_HEIGHT * 2) - y_offset, 2);
    }
    break;
    default:
    {
        s32 row_y;
        s32 entry_index;

        if (g_addhero_entry_scan_active != 0)
        {
            s32 message_x;
            u16* text_table;

            message_x = -x_offset + 0x84;
            text_table = &g_addhero_glyph_table;
            prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 0), 4, message_x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 15), 4, message_x, ADDHERO_ENTRY_ROW_HEIGHT - y_offset, 2);
            prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 89), 4, message_x, (ADDHERO_ENTRY_ROW_HEIGHT * 2) - y_offset, 2);
            break;
        }
        entry_index = 0;
        if (entry_state > 0)
        {
            s32 list_x;
            u16 marker_offset;
            DVECTOR value_pos;
            u16* text_table;

            text_table = &g_addhero_glyph_table;
            list_x = -x_offset;
            do
            {
                row_y = ((entry_index * ADDHERO_ENTRY_ROW_HEIGHT) - y_offset) - g_addhero_scroll_y + 1;
                if (row_y >= -13 && row_y <= 87)
                {
                    if (g_addhero_entry_ranks[entry_index] >= 0)
                    {
                        value_pos.vx = list_x + 0x86;
                        value_pos.vy = row_y;
                        prim = func_800A88A0(func_800A8A78(ot, prim, g_addhero_entry_suffix_values[entry_index], 4, &value_pos, 0), ot,
                                             ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_glyph_entry_value_label), 4, list_x + 0x70, row_y, 0);
                        if ((g_addhero_rank_count - 1) == g_addhero_entry_ranks[entry_index])
                        {
                            marker_offset = text_table[27];
                            prim = func_800A88A0(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, marker_offset), 4, list_x + 0xC0, row_y, 0);
                        }
                        else if (g_addhero_entry_ranks[entry_index] < 2)
                        {
                            marker_offset = text_table[28];
                            prim = func_800A88A0(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, marker_offset), 4, list_x + 0xC0, row_y, 0);
                        }
                        if (*addhero_skip_hex_digits(&g_addhero_entries[g_addhero_card_slot][entry_index].name[ADDHERO_SAVE_FILENAME_PREFIX_LENGTH]) == '+')
                        {
                            prim = func_800A88A0(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_glyph_plus_marker), 4, 0xF2 - x_offset, row_y, 1);
                        }
                    }
                    if (strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_glyph_save_entry_label), 4, 1 - x_offset, row_y, 0);
                    }
                    else if (strncmp(g_lom_alt_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name,
                                     ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_glyph_alt_save_entry_label), 4, 1 - x_offset, row_y, 0);
                    }
                    else if (strncmp(g_new_save_entry_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name,
                                     ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_glyph_new_entry_label), 4, 1 - x_offset, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_glyph_default_entry_label), 4, 1 - x_offset, row_y, 0);
                    }
                }
                entry_index++;
            } while (entry_index < g_addhero_entry_state);
        }
        row_y = ((g_addhero_selected_row * ADDHERO_ENTRY_ROW_HEIGHT) - y_offset) - g_addhero_scroll_y;

        if (g_addhero_entry_scan_active == 0)
        {
            TILE* tile = (TILE*)prim;

            *(u32*)&tile->r0 = 0xF080F0;
            setlen(tile, 3);
            setcode(tile, 0x62);
            tile->w = 0x108;
            setXY0(tile, 0, row_y);
            tile->h = ADDHERO_ENTRY_ROW_HEIGHT;
            addPrim(ot, tile);
            prim = tile + 1;
        }
    }
    break;
    }
    return prim;
}

/**
 * @brief Draw the header glyph that reflects the current mode (load vs save).
 * @param ot   Ordering table the glyph is linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
void* addhero_draw_mode_glyph(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */

    if (g_addhero_mode == 1)
    {
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_mode1, 35), 4, -x_offset + 0x78, -y_offset, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_mode0, 34), 4, -x_offset + 0x78, -y_offset, 2);
    }
    return prim;
}

/**
 * @brief Draw the slot-0 card label, dimming it with a backing tile when that
 *        slot is not the active one.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
void* addhero_draw_card_slot0_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */
    TILE* tile;

    if (g_addhero_card_slot != 0)
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
    return func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_card_slot0_label, 6), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the slot-1 card label, dimming it with a backing tile when that
 *        slot is not the active one.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
void* addhero_draw_card_slot1_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */
    TILE* tile;

    if (g_addhero_card_slot == 0)
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
    return func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_card_slot1_label, 7), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the detail panel for the selected entry: animated character
 *        icons, play-time, hero name, and either the cached name text or a
 *        fallback message depending on entry type.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
void* addhero_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    void* result;
    DVECTOR pos;
    u8 name[0x100];
    s32 slot[3];

    result = prim;
    if (g_addhero_selection_status == 0)
    {
        return result;
    }
    if (g_addhero_entry_scan_active != 0)
    {
        return result;
    }
    if (g_addhero_selection_status != 3 && g_addhero_entry_state < 0x10)
    {
        if (g_addhero_selection_status == 2)
        {
            s32 x = -x_offset;
            u16* text_table;

            result = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_details_status2_msg, 20), 4, x, -y_offset, 0);
            text_table = ADDHERO_TEXT_TABLE(g_addhero_glyph_details_status2_msg, 20);
            return func_800A88A0(result, ot, ADDHERO_TEXT(text_table, 21), 4, x, 0x10 - y_offset, 0);
        }
        else
        {
            if (strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) ==
                0)
            {
                if (g_save_slot_index == 0xFF || g_addhero_entry_owner_id == g_save_slot_index)
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

                    {
                        AddheroRecord* record = &g_addhero_entry_metadata;
                        slot[0] = record->first_icon;
                        slot[1] = record->second_icon;
                        slot[2] = record->third_icon;
                        g_addhero_icon_palette = record->icon_palette;
                    }

                    total = 0;
                    present_count = 0;
                    for (i = 0; i < 3; i++)
                    {
                        if (slot[i] != ADDHERO_NO_ICON)
                        {
                            present_count += 1;
                        }
                    }

                    switch (present_count)
                    {
                    case 2:
                        step = 0x20;
                        half_step = 0x10;
                        time_val = g_addhero_icon_phase;
                        if (g_addhero_icon_phase < 0)
                        {
                            time_val = g_addhero_icon_phase + 0x1F;
                        }
                        g_addhero_icon_phase -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        g_addhero_icon_phase %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        g_addhero_icon_phase = 0x1F;
                        break;
                    }

                    i = 0;
                    j = i;
                    for (; j < 3; j++)
                    {
                        base_y = i * half_step;
                        base_x = base_y + half_step;
                        if (slot[j] != ADDHERO_NO_ICON)
                        {
                            s32 adjust = step;
                            s32 rem;
                            s32 hi;
                            s32 delta;

                            if ((g_addhero_icon_phase >= base_y && g_addhero_icon_phase < base_x && (delta = g_addhero_icon_phase - base_y, 1)) ||
                                (rem = base_x % (half_step * present_count),
                                 g_addhero_icon_phase >= rem && g_addhero_icon_phase < (hi = rem + half_step) && (delta = hi - g_addhero_icon_phase, 1)))
                            {
                                adjust += delta;
                            }
                            result = addhero_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, slot[j], i, j);
                            i += 1;
                            total += adjust;
                        }
                    }

                    {
                        AddheroRecord* record = &g_addhero_entry_metadata;
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        base_y = record->play_time_frames;
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
                        result = func_800A8A78(ot, result, base_y, 4, &pos, 1);
                        result = func_800A88A0(result, ot, record->name, 4, x + 0x54, y + 0x10, 0);

                        if (record->hero_id == ((AddheroRecord*)g_pad_ctx)->hero_id)
                        {
                            do
                            {
                                result = func_800A88A0(result, ot, ADDHERO_TEXT_AT(g_addhero_glyph_current_hero_marker, 40), 4, x + 0x54, y + 0x20, 0);
                            } while (0);
                        }
                        else
                        {
                            result = func_800A88A0(result, ot, ADDHERO_TEXT(g_addhero_entry_glyph_table, record->entry_label), 4, x + 0x54, y + 0x20, 0);
                        }
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, ADDHERO_TEXT_AT(g_addhero_glyph_owner_mismatch_msg, 42), 4, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 j;
                u8* record;

                addhero_terminate_multibyte_text(&g_addhero_entry_record);
                record = &g_addhero_entry_record;
                record -= 4;
                if (record[0x24] == 0 || record[0x24] >= 0x80)
                {
                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = record[4 + j];
                    }
                    name[j] = 0;
                    result = addhero_draw_cached_text(result, ot, name, -x_offset, -y_offset, 4, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = ((AddheroFallbackText*)&g_addhero_entry_read_buffer)->text[j];
                    }
                    name[j] = 0;
                    result = addhero_draw_cached_text(result, ot, name, -x_offset, -y_offset + 0x10, 4, 0);
                }
            }
        }
    }
    return result;
}

/**
 * @brief Advance past a run of hex digit characters (0-9, a-f, A-F) and return
 *        the pointer to the first non-hex byte.
 * @param text Start of the text to scan.
 * @return Pointer to the first byte that is not a hex digit.
 * @see decomp.me (100%)
 */
u8* addhero_skip_hex_digits(u8* text)
{
    u8* cursor = text;

    while ((*cursor >= '0' && *cursor <= '9') || (*cursor >= 'a' && *cursor <= 'f') || (*cursor >= 'A' && *cursor <= 'F'))
    {
        cursor++;
    }
    return cursor;
}

/**
 * @brief Zero-fill a 0x40-byte text field from the first null byte onward,
 *        walking multibyte (>= 0x80 lead) characters two bytes at a time.
 * @param buffer Start of the 0x40-byte text buffer to terminate/clear.
 * @see decomp.me (100%)
 */
void addhero_terminate_multibyte_text(void* buffer)
{
    u8* p;
    s32 i;

    p = (u8*)buffer;
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
 * @brief Clear the eight-element pool: drop the ADDHERO flag and free (state 0)
 *        every element, and reset the shared draw scale to 0x20.
 * @see decomp.me (100%)
 */
void addhero_clear_elements(void)
{
    AddheroElement* p;
    s32 i;

    g_menu_element_counter = 0x20;
    p = g_addhero_element_pool;
    for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++)
    {
        p->size.bits.scrollable = 0;
        p->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        p++;
    }
}

/**
 * @brief Claim the first free pool element, marking it state 1 (opening).
 * @return The claimed element, or the pool base element when none are free.
 * @see decomp.me (100%)
 */
AddheroElement* addhero_alloc_element(void)
{
    AddheroElement* p;
    s32 i;

    p = g_addhero_element_pool;
    for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, p++)
    {
        if (p->attr.bits.state == ADDHERO_ELEMENT_STATE_INACTIVE)
        {
            p->attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
            return p;
        }
    }
    return g_addhero_element_pool;
}

/**
 * @brief Update and render the active ADDHERO UI elements.
 * @param draw_state Draw state holding the primitive cursor and frame flag.
 * @see decomp.me (100%)
 */
void addhero_update_and_draw_elements(AddheroDrawState* draw_state)
{
    void* prim;
    u_long* ot;
    AddheroElement* element;
    s32 i;
    DRAWENV draw_env;
    s32 scaled_width;
    s32 scaled_height;

    prim = draw_state->prim_cursor;
    ot = &draw_state->ot;

    if ((g_addhero_entry_state < 0x10) && (g_addhero_element_pool[1].attr.bits.state == ADDHERO_ELEMENT_STATE_ACTIVE) &&
        (g_addhero_element_pool[1].size.bits.scrollable != 0))
    {
        if ((g_addhero_entry_state * ADDHERO_ENTRY_ROW_HEIGHT) > (g_addhero_scroll_y + 0x58))
        {
            prim = func_800AE76C(prim, ot, 0x114, 0x82, 0);
        }
        if (g_addhero_scroll_y != 0)
        {
            prim = func_800AE76C(prim, ot, 0x114, 0x3A, 1);
        }
    }

    if (draw_state->display_buffer_index != 0)
    {
        SetDefDrawEnv(&draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    }
    else
    {
        SetDefDrawEnv(&draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    }

    element = g_addhero_element_pool;
    for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
        {
            SetDrawEnv((DR_ENV*)prim, &draw_env);
            addPrim(ot, prim);
            prim = (DR_ENV*)prim + 1;

            switch (element->attr.bits.state)
            {
            case ADDHERO_ELEMENT_STATE_OPENING:
                g_pad_input = 0;
                {
                    u32 width_low = ADDHERO_ELEMENT_WIDTH_LOW(element);
                    s32 width = ADDHERO_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.bits.transition_step) / 8;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / 8;
                    prim = element->draw_handler(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = ADDHERO_ELEMENT_WIDTH_LOW(element);

                    prim = func_800AD850(prim, ot, x + (ADDHERO_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                         element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                         draw_state->display_buffer_index, i == 0);
                }
                element->attr.bits.transition_step++;
                if (element->attr.bits.transition_step == 8)
                {
                    field_reset_input_repeat();
                    element->attr.bits.state = ADDHERO_ELEMENT_STATE_ACTIVE;
                }
                break;

            case ADDHERO_ELEMENT_STATE_ACTIVE:
                prim = element->draw_handler(ot, prim, 0, 0);
                {
                    u32 width_low = ADDHERO_ELEMENT_WIDTH_LOW(element);

                    prim = func_800AD850(prim, ot, element->attr.bits.x, element->attr.bits.y, ADDHERO_ELEMENT_WIDTH(element, width_low),
                                         element->size.bits.height, draw_state->display_buffer_index, i == 0);
                }
                if (element->attr.bits.transition_step != 0)
                {
                    element->attr.bits.transition_step--;
                }
                break;

            case ADDHERO_ELEMENT_STATE_CLOSING:
                g_pad_input = 0;
                {
                    u32 width_low = ADDHERO_ELEMENT_WIDTH_LOW(element);
                    s32 width = ADDHERO_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.bits.transition_step) / 8;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / 8;
                    prim = element->draw_handler(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = ADDHERO_ELEMENT_WIDTH_LOW(element);

                    prim = func_800AD850(prim, ot, x + (ADDHERO_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                         element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                         draw_state->display_buffer_index, i == 0);
                }
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.transition_step = 3;
                    element->attr.bits.state = ADDHERO_ELEMENT_STATE_FINISHING;
                }
                break;

            case ADDHERO_ELEMENT_STATE_FINISHING:
                g_pad_input = 0;
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
                }
                break;
            }
        }
    }

    draw_state->prim_cursor = prim;
}

/**
 * @brief Free the primary pool element by clearing its state bits.
 * @see decomp.me (100%)
 */
void addhero_deactivate_primary_element(void)
{
    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
}

/**
 * @brief Append the multibyte string @p src onto the end of @p dst and
 *        null-terminate the result.
 * @param dst Destination string; appended to in place.
 * @param src Source string copied onto the end of @p dst.
 * @see decomp.me (100%)
 */
void addhero_text_append(u8* dst, u8* src)
{
    s32 dst_len;
    s32 src_len;
    s32 i;

    dst_len = addhero_text_byte_length(dst);
    src_len = addhero_text_byte_length(src);
    for (i = 0; i < src_len; i++)
    {
        dst[dst_len + i] = src[i];
    }
    dst[dst_len + i] = 0;
}

/**
 * @brief Measure the byte length of a string, counting characters in the
 *        0x19-0x1F lead range as two bytes.
 * @param str Null-terminated string to measure.
 * @return Length in bytes, excluding the terminator.
 * @see decomp.me (100%)
 */
s32 addhero_text_byte_length(u8* str)
{
    u8* p;
    u8 c;
    s32 len;

    p = str;
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
 * @brief Copy a multibyte string, counting 0x19-0x1F lead bytes as two-byte
 *        characters when computing its length, and null-terminate the result.
 * @param dst Destination buffer.
 * @param src Source string to copy.
 * @see decomp.me (100%)
 */
void addhero_text_copy(u8* dst, u8* src)
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
        dst[i] = src[i];
    }
    dst[i] = 0;
}
