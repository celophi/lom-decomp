#include "overlays/field/field_text.h"
#include "internal/addhero_internal.h"
#include "common/gpu_packet.h"
#include "main/audio/akao.h"

/**
 * @brief Reset overlay state and build the initial UI elements.
 * @param work_base Work-RAM base (always 0x80170000); stored in g_addhero_work_ram_base, unused so far.
 * @param mode Mode selector, stored in g_addhero_mode.
 */
void addhero_init(s32 work_base, s32 mode)
{
    RECT rect;

    g_addhero_mode = mode;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    g_card_slot = 0;

    card_reset_entry_ranks();
    g_addhero_result = ADDHERO_RESULT_CANCELLED;
    addhero_init_card_events();
    g_addhero_icon_phase = 0;
    field_set_default_fade_target();

    setRECT(&rect, OVERLAY_INIT_CLEAR_VRAM_X, OVERLAY_INIT_CLEAR_VRAM_Y, OVERLAY_INIT_CLEAR_VRAM_W, OVERLAY_INIT_CLEAR_VRAM_H);

    ClearImage(&rect, 0, 0, 0);
    reset_glyph_cache();

    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = CARD_MENU_SELECTION_NONE;
    g_addhero_io_busy = 0;
    g_card_menu_frame_parity = 0;
    g_card_menu_exit_requested = 0;

    field_reset_input_repeat();
    addhero_build_ui_elements();

    g_addhero_work_ram_base = work_base;
}

/**
 * @brief Run one frame: tear down and exit if requested, else update and render.
 * @param draw_state Frame drawing context passed through to the element renderer.
 * @return Non-zero exit code when exiting, 0 while running.
 */
s32 addhero_state_step(FieldRenderHalf* draw_state)
{
    if (g_card_menu_exit_requested != 0)
    {
        shutdown_card_events();
        field_text_reset_windows();
        DrawSync(0);
        return g_card_menu_exit_requested;
    }

    field_text_reset_scratch();
    begin_glyph_cache_frame();
    card_menu_update_state(draw_state);
    evict_unused_glyphs();
    field_text_upload_immediate_cache();
    g_card_menu_frame_parity ^= 1;
    return 0;
}

/**
 * @brief Reset scroll/selection state and populate the UI element pool for the current mode.
 * @note mode != 0: transfer layout (status + two card-slot labels). mode == 0: full browser
 *       (entry list, mode glyph, two slot labels, entry details).
 */
void addhero_build_ui_elements(void)
{
    CardMenuElement* element;
    s32 unused[2];

    g_card_menu_scroll_frames = 0;
    g_card_menu_scroll_target_y = 0;
    g_card_menu_scroll_y = 0;
    g_card_menu_selected_row = 0;
    g_addhero_selection_status = CARD_MENU_SELECTION_NONE;
    g_addhero_items = g_saved_game_ctx->items;
    addhero_clear_elements();
    g_addhero_load_flow_active = 0;
    if (g_addhero_mode != 0)
    {
        g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_OPENING;
        element = card_menu_alloc_element();
        element->draw = addhero_draw_transfer_status;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARD_MENU_MESSAGE_X;
        element->attr.bits.y = CARD_MENU_EXCHANGE_MESSAGE_Y;
        element->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
        element->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_MESSAGE_WIDTH);

        element = card_menu_alloc_element();
        element->draw = card_menu_draw_card_slot0_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT0_LABEL_X;
        element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_TRANSFER_Y;
        element->size.bits.width_high = 0;
        element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

        element = card_menu_alloc_element();
        element->draw = card_menu_draw_card_slot1_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT1_LABEL_X;
        element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_TRANSFER_Y;
        element->size.bits.width_high = 0;
        element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);
        g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
        return;
    }

    g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_OPENING;
    element = card_menu_alloc_element();
    element->draw = addhero_draw_entry_list;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_LIST_X;
    element->attr.bits.y = CARD_MENU_LIST_Y;
    element->size.bits.width_high = CARD_MENU_EXCHANGE_LIST_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_LIST_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_EXCHANGE_LIST_WIDTH);
    element->size.bits.flag = 1;

    element = card_menu_alloc_element();
    element->draw = addhero_draw_mode_glyph;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_TITLE_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_TITLE_Y;
    element->size.bits.width_high = CARD_MENU_EXCHANGE_TITLE_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_EXCHANGE_TITLE_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_EXCHANGE_TITLE_WIDTH);

    element = card_menu_alloc_element();
    element->draw = card_menu_draw_card_slot0_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT0_LABEL_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_BROWSER_Y;
    element->size.bits.width_high = 0;
    element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

    element = card_menu_alloc_element();
    element->draw = card_menu_draw_card_slot1_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT1_LABEL_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_BROWSER_Y;
    element->size.bits.width_high = 0;
    element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

    element = card_menu_alloc_element();
    element->draw = addhero_draw_selected_entry_details;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_DETAILS_X;
    element->attr.bits.y = CARD_MENU_DETAILS_Y;
    element->size.bits.width_high = CARD_MENU_DETAILS_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_DETAILS_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_DETAILS_WIDTH);
    g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
}

/**
 * @brief Run one frame of overlay logic: update elements, advance the load
 *        sequence when armed, sample pad input, and step the scroll animation.
 * @param draw_state Frame drawing context passed to the element renderer.
 */
void card_menu_update_state(FieldRenderHalf* draw_state)
{
    card_menu_update_elements(draw_state);
    g_addhero_icon_phase += 2;
    if (g_card_menu_element_pool[CARD_MENU_ELEMENT_MAIN].attr.bits.state == CARD_MENU_ELEMENT_OPEN && g_card_menu_element_pool[CARD_MENU_ELEMENT_MAIN].attr.bits.transition_step == 0)
    {
        addhero_update_load_sequence();
    }
    if ((u16)g_pad_input == PAD_ALL_BUTTONS)
    {
        g_pad_input = 0;
    }
    addhero_handle_input();
    if (g_card_menu_scroll_frames != 0)
    {
        g_card_menu_scroll_y += (g_card_menu_scroll_target_y - g_card_menu_scroll_y) / g_card_menu_scroll_frames--;
    }
    else
    {
        g_card_menu_scroll_y = g_card_menu_scroll_target_y;
    }
}

/**
 * @brief Drive the card load/scan state machine one frame, mapping its result
 *        code onto the next load step and any error entry-state sentinel.
 * @return Nothing: the original is declared int but returns no value, and its caller ignores it.
 */
s32 addhero_update_load_sequence(void)
{
    s32 result;

    if (g_card_entry_state >= CARD_MENU_ENTRY_COUNT_LIMIT)
    {
        if (g_card_step == NULL)
        {
            g_card_step = &g_addhero_loadseq_start;
        }
    }

    do
    {
        result = addhero_advance_load_sequence();
    } while (result == CARD_MENU_SEQUENCE_RUN_AGAIN);

    if ((g_addhero_load_flow_active != 0) && (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK))
    {
        if (g_addhero_mode == 0)
        {
            g_card_entry_state = CARD_MENU_ENTRY_STATE_UNFORMATTED;
        }
        else
        {
            g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
        }
        g_card_step = g_addhero_loadseq_abort;
    }
    else
    {
        switch (result)
        {
        case CARD_MENU_SEQUENCE_NONE:
            break;
        case CARD_MENU_SEQUENCE_NO_CARD:
            g_card_step = g_addhero_loadseq_done;
            g_addhero_load_flow_active = 0;
            break;
        case CARD_MENU_SEQUENCE_UNFORMATTED:
            if (g_addhero_mode == 0)
            {
                g_card_entry_state = CARD_MENU_ENTRY_STATE_UNFORMATTED;
            }
            else
            {
                g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
            }
            /* fallthrough */
        case CARD_MENU_SEQUENCE_FINISHED:
            g_card_step = g_addhero_loadseq_abort;
            break;
        }
    }
}

/**
 * @brief Handle browser input, entry navigation, and load confirmation.
 * @return Nothing: the original is declared int but returns no value, and its caller ignores it.
 */
s32 addhero_handle_input(void)
{
    s32 entry_count;
    s32 input;
    s32 move_count;
    CardMenuElement* prompt;
    struct DIRENTRY* selected_entry;
    SavedGameLayout* entry;

    if (g_card_menu_element_pool[1].attr.bits.state == CARD_MENU_ELEMENT_FREE)
    {
        g_card_menu_exit_requested = g_addhero_result;
        return;
    }
    if (g_card_menu_exit_requested != 0)
    {
        return;
    }
    if (g_card_menu_element_pool[1].attr.bits.state >= CARD_MENU_ELEMENT_CLOSING)
    {
        return;
    }
    if (g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state != CARD_MENU_ELEMENT_FREE)
    {
        return;
    }

    entry_count = g_card_entry_state;
    if (entry_count == CARD_MENU_ENTRY_STATE_CHECKING_CARD)
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
    if (*g_card_step >= CARD_MENU_STEP_SCAN_ENTRIES && *g_card_step <= CARD_MENU_STEP_SCAN_DONE)
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
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        card_menu_close_all_elements();
        return;
    }
    if (input & CARD_MENU_CARD_SWITCH_BUTTON_MASK)
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        addhero_reset_state();
        return;
    }
    if (entry_count >= CARD_MENU_ENTRY_COUNT_LIMIT)
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

    for (; move_count != 0; move_count--)
    {
        if (g_pad_input & PAD_BTN_UP)
        {
            g_card_menu_selected_row--;
            if (g_card_menu_selected_row < 0)
            {
                g_card_menu_selected_row = g_card_entry_state - 1;
            }
        }
        if (g_pad_input & PAD_BTN_DOWN)
        {
            g_card_menu_selected_row++;
            if (g_card_menu_selected_row >= g_card_entry_state)
            {
                g_card_menu_selected_row = 0;
            }
        }
    }

    if (g_pad_input & (PAD_BTN_UP | PAD_BTN_DOWN))
    {
        addhero_commit_selected_entry();
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        card_menu_scroll_to_selection();
        return;
    }

    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        selected_entry = &g_card_entries[g_card_slot][g_card_menu_selected_row];
        if (strncmp(g_lom_save_filename_prefix, selected_entry->name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            entry = &g_addhero_entry_file.saved_game;
            if ((entry->identity.ids.game_id != g_saved_game_ctx->identity.ids.game_id) &&
                ((g_save_compatibility_tag == SAVE_TAG_ANY) || (entry->compatibility_tag == g_save_compatibility_tag)))
            {
                prompt = card_menu_alloc_element();
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.x = CARD_MENU_MESSAGE_X;
                prompt->attr.bits.y = CARD_MENU_EXCHANGE_MESSAGE_Y;
                prompt->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
                prompt->size.bits.height = CARD_MENU_EXCHANGE_PROMPT_HEIGHT;
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(prompt, CARD_MENU_MESSAGE_WIDTH);
                card_menu_enable_choice_toggle();
                prompt->draw = addhero_draw_load_prompt;
                restart_card_sequence();
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                return;
            }
        }
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    }
}

/**
 * @brief Reset scroll/selection state and flip to the other card slot, then
 *        clear ranks and pad input to restart browsing.
 */
inline void addhero_reset_state(void)
{
    g_addhero_load_flow_active = 0;
    g_card_step = NULL;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    g_card_menu_scroll_frames = 0;
    g_card_menu_scroll_target_y = 0;
    g_card_menu_scroll_y = 0;
    g_card_menu_selected_row = 0;
    g_addhero_selection_status = CARD_MENU_SELECTION_NONE;
    g_card_slot ^= 1;
    card_reset_entry_ranks();
    field_reset_input_repeat();
    g_pad_input = 0;
}

#include "../../common/card_menu/card_menu_close_all_elements.inc.c"

#include "../../common/card_menu/card_menu_scroll_to_selection.inc.c"

#include "../../common/card_menu/card_menu_update_elements.inc.c"

/**
 * @brief Draw the save-entry browser: status/error screens by entry-state
 *        sentinel, the per-row entry list with rank glyphs, and the selection
 *        highlight tile.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset applied to each row.
 * @return The updated primitive pointer after linking this frame's glyphs.
 */
void* addhero_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 entry_state = g_card_entry_state;

    switch (entry_state)
    {
    case CARD_MENU_ENTRY_STATE_NO_GAME_DATA:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_UNFORMATTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_CARD_FULL:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_no_card, CARD_MENU_TEXT_NO_CARD), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_card_access_failed, CARD_MENU_TEXT_CARD_ACCESS_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_NO_SAVE_DATA:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_no_save_data, CARD_MENU_TEXT_NO_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_BLANK:
        break;
    case CARD_MENU_ENTRY_STATE_CHECKING_CARD:
    {
        s32 message_x;
        u16* text_table;

        message_x = -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2;
        text_table = &g_addhero_text_table;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               (CARD_MENU_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
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

            message_x = -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2;
            text_table = &g_addhero_text_table;
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                                   CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                                   (CARD_MENU_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
            break;
        }
        entry_index = 0;
        if (entry_state > 0)
        {
            s32 list_x;
            u16 marker_offset;
            Vec2s value_pos;
            u16* text_table;

            text_table = &g_addhero_text_table;
            list_x = -x_offset;
            do
            {
                row_y = ((entry_index * CARD_MENU_ENTRY_ROW_HEIGHT) - y_offset) - g_card_menu_scroll_y + 1;
                if (row_y > -CARD_MENU_ENTRY_ROW_HEIGHT && row_y < CARD_MENU_LIST_HEIGHT)
                {
                    if (g_card_entry_ranks[entry_index] >= 0)
                    {
                        value_pos.x = list_x + CARD_MENU_ENTRY_VALUE_X;
                        value_pos.y = row_y;
                        prim = field_draw_text(
                            field_draw_number(ot, prim, g_card_entry_suffix_values[entry_index], FIELD_TEXT_COLOR_NORMAL, &value_pos, FIELD_TEXT_ALIGN_LEFT),
                            ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_addhero_text_number_label), FIELD_TEXT_COLOR_NORMAL, list_x + CARD_MENU_ENTRY_NUMBER_LABEL_X,
                            row_y, FIELD_TEXT_ALIGN_LEFT);
                        if ((g_card_rank_count - 1) == g_card_entry_ranks[entry_index])
                        {
                            marker_offset = text_table[CARD_MENU_TEXT_NEWEST];
                            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, marker_offset), FIELD_TEXT_COLOR_NORMAL,
                                                   list_x + CARD_MENU_EXCHANGE_ENTRY_MARKER_X, row_y, FIELD_TEXT_ALIGN_LEFT);
                        }
                        else if (g_card_entry_ranks[entry_index] < 2)
                        {
                            marker_offset = text_table[CARD_MENU_TEXT_OLDEST];
                            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, marker_offset), FIELD_TEXT_COLOR_NORMAL,
                                                   list_x + CARD_MENU_EXCHANGE_ENTRY_MARKER_X, row_y, FIELD_TEXT_ALIGN_LEFT);
                        }
                        if (*skip_hex_digits(&g_card_entries[g_card_slot][entry_index].name[CARD_SAVE_FILENAME_PREFIX_LENGTH]) == '+')
                        {
                            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_addhero_text_plus_marker), FIELD_TEXT_COLOR_NORMAL,
                                                   CARD_MENU_EXCHANGE_ENTRY_PLUS_RIGHT_X - x_offset, row_y, FIELD_TEXT_ALIGN_RIGHT);
                        }
                    }
                    if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_addhero_text_mana_label), FIELD_TEXT_COLOR_NORMAL,
                                               CARD_MENU_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_addhero_text_ring_ring_land_label), FIELD_TEXT_COLOR_NORMAL,
                                               CARD_MENU_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_addhero_text_new_save_label), FIELD_TEXT_COLOR_NORMAL,
                                               CARD_MENU_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_addhero_text_other_game_label), FIELD_TEXT_COLOR_NORMAL,
                                               CARD_MENU_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                }
                entry_index++;
            } while (entry_index < g_card_entry_state);
        }
        row_y = ((g_card_menu_selected_row * CARD_MENU_ENTRY_ROW_HEIGHT) - y_offset) - g_card_menu_scroll_y;

        if (g_addhero_entry_scan_active == 0)
        {
            TILE* tile = (TILE*)prim;

            *(u32*)&tile->r0 = CARD_MENU_HIGHLIGHT_COLOR;
            setlen(tile, 3);
            setcode(tile, GPU_CODE_TILE | GPU_CODE_SEMI_TRANS);
            tile->w = CARD_MENU_EXCHANGE_LIST_WIDTH;
            setXY0(tile, 0, row_y);
            tile->h = CARD_MENU_ENTRY_ROW_HEIGHT;
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
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 */
void* addhero_draw_mode_glyph(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;

    if (g_addhero_mode == 1)
    {
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_select_item, CARD_MENU_TEXT_SELECT_ITEM), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    else
    {
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_select_save_data, CARD_MENU_TEXT_SELECT_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    return prim;
}

#include "../../common/card_menu/card_menu_draw_card_slot0_label.inc.c"

#include "../../common/card_menu/card_menu_draw_card_slot1_label.inc.c"

/**
 * @brief Draw the detail panel for the selected entry: animated character
 *        icons, play-time, hero name, and either the cached name text or a
 *        fallback message depending on entry type.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 */
void* addhero_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    void* result;
    Vec2s pos;
    u8 name[0x100];
    s32 slot[FIELD_PARTY_SIZE];

    result = prim;
    if (g_addhero_selection_status == CARD_MENU_SELECTION_NONE)
    {
        return result;
    }
    if (g_addhero_entry_scan_active != 0)
    {
        return result;
    }
    if (g_addhero_selection_status == CARD_MENU_SELECTION_EMPTY_CARD || g_card_entry_state >= CARD_MENU_ENTRY_COUNT_LIMIT)
    {
        return result;
    }
    if (g_addhero_selection_status == CARD_MENU_SELECTION_NEW_SAVE)
    {
        s32 x = -x_offset;
        u16* text_table;

        result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_new_save_title, CARD_MENU_TEXT_NEW_SAVE_TITLE), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                 FIELD_TEXT_ALIGN_LEFT);
        text_table = CARD_MENU_TEXT_TABLE(g_addhero_text_new_save_title, CARD_MENU_TEXT_NEW_SAVE_TITLE);
        return field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_USES_TWO_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x,
                               CARD_MENU_DETAILS_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_LEFT);
    }
    else
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            if (g_save_compatibility_tag == SAVE_TAG_ANY || g_addhero_entry_file.saved_game.compatibility_tag == g_save_compatibility_tag)
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

                {
                    SavedGameLayout* entry = &g_addhero_entry_file.saved_game;
                    slot[0] = entry->spawn.bits.party_icon_0;
                    slot[1] = entry->track.bits.party_icon_1;
                    slot[2] = entry->track.bits.party_icon_2;
                    g_card_menu_icon_palette = entry->icon_palette;
                }

                total = 0;
                present_count = 0;
                for (i = 0; i < FIELD_PARTY_SIZE; i++)
                {
                    if (slot[i] != SAVE_NO_ICON)
                    {
                        present_count += 1;
                    }
                }

                switch (present_count)
                {
                case 2:
                    step = 32;
                    half_step = 16;
                    g_addhero_icon_phase %= 32;
                    break;
                case 3:
                    step = 16;
                    half_step = 32;
                    g_addhero_icon_phase %= 96;
                    break;
                default:
                    step = 16;
                    half_step = 32;
                    g_addhero_icon_phase = 31;
                    break;
                }

                i = 0;
                j = i;
                for (; j < FIELD_PARTY_SIZE; j++)
                {
                    base_y = i * half_step;
                    base_x = base_y + half_step;
                    if (slot[j] != SAVE_NO_ICON)
                    {
                        s32 adjust = step;
                        s32 rem;
                        s32 hi;

                        if (g_addhero_icon_phase >= base_y && g_addhero_icon_phase < base_x)
                        {
                            adjust += g_addhero_icon_phase - base_y;
                        }
                        else
                        {
                            rem = base_x % (half_step * present_count);
                            if (g_addhero_icon_phase >= rem)
                            {
                                hi = rem + half_step;
                                if (g_addhero_icon_phase < hi)
                                {
                                    adjust += hi - g_addhero_icon_phase;
                                }
                            }
                        }
                        result = card_menu_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, slot[j], i, j);
                        i += 1;
                        total += adjust;
                    }
                }

                {
                    SavedGameLayout* entry = &g_addhero_entry_file.saved_game;
                    s32 x = -x_offset;
                    s32 y = -y_offset;

                    base_y = entry->play_time;
                    pos.x = (s16)(x + CARD_MENU_DETAILS_HOURS_RIGHT_X);
                    pos.y = (s16)y;
                    hours = base_y / SAVED_PLAY_TIME_TICKS_PER_HOUR;
                    result = field_draw_number(ot, result, hours, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                    result = field_draw_text(result, ot, FIELD_UI_TEXT_AT(g_text_time_separator_offset_bytes, FIELD_UI_TEXT_TIME_SEPARATOR),
                                             FIELD_TEXT_COLOR_NORMAL, x + CARD_MENU_DETAILS_TIME_SEPARATOR_X, y, FIELD_TEXT_ALIGN_LEFT);
                    base_y = (base_y / SAVED_PLAY_TIME_TICKS_PER_MINUTE) - (hours * 60);
                    if (base_y < 10)
                    {
                        pos.x = (s16)(x + CARD_MENU_DETAILS_MINUTES_TENS_RIGHT_X);
                        pos.y = (s16)y;
                        result = field_draw_number(ot, result, 0, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                    }
                    pos.x = (s16)(x + CARD_MENU_DETAILS_MINUTES_RIGHT_X);
                    pos.y = (s16)y;
                    result = field_draw_number(ot, result, base_y, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                    result = field_draw_text(result, ot, entry->summary_name, FIELD_TEXT_COLOR_NORMAL, x + CARD_MENU_DETAILS_TEXT_X,
                                             y + CARD_MENU_DETAILS_LINE_HEIGHT, FIELD_TEXT_ALIGN_LEFT);

                    if (entry->identity.ids.game_id == g_saved_game_ctx->identity.ids.game_id)
                    {
                        result =
                            field_draw_text(result, ot, CARD_MENU_TEXT_AT(g_addhero_text_same_hero_data, CARD_MENU_TEXT_SAME_HERO_DATA), FIELD_TEXT_COLOR_NORMAL,
                                            x + CARD_MENU_DETAILS_TEXT_X, y + CARD_MENU_DETAILS_LINE_HEIGHT * 2, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else
                    {
                        result =
                            field_draw_text(result, ot, CARD_MENU_TEXT(g_addhero_location_text_table, entry->track.bits.music_track), FIELD_TEXT_COLOR_NORMAL,
                                            x + CARD_MENU_DETAILS_TEXT_X, y + CARD_MENU_DETAILS_LINE_HEIGHT * 2, FIELD_TEXT_ALIGN_LEFT);
                    }
                }
            }
            else
            {
                result = field_draw_text(result, ot, CARD_MENU_TEXT_AT(g_addhero_text_wrong_version, CARD_MENU_TEXT_WRONG_VERSION), FIELD_TEXT_COLOR_NORMAL,
                                         -x_offset, -y_offset, FIELD_TEXT_ALIGN_LEFT);
            }
        }
        else
        {
            s32 j;
            SaveFileHeader* header;

            terminate_multibyte_text(g_addhero_entry_file.header.title);
            header = &g_addhero_entry_file.header;
            if (header->title[1][0] == 0 || header->title[1][0] >= SJIS_LEAD_MIN)
            {
                for (j = 0; j < SAVE_FILE_TITLE_LINE_BYTES; j++)
                {
                    name[j] = *(header->title[0] + j);
                }
                name[j] = 0;
                result = draw_cached_text(result, ot, name, -x_offset, -y_offset, FIELD_TEXT_COLOR_NORMAL, FIELD_TEXT_ALIGN_LEFT);

                for (j = 0; j < SAVE_FILE_TITLE_LINE_BYTES; j++)
                {
                    name[j] = g_addhero_entry_file.header.title[1][j];
                }
                name[j] = 0;
                result = draw_cached_text(result, ot, name, -x_offset, -y_offset + CARD_MENU_DETAILS_LINE_HEIGHT, FIELD_TEXT_COLOR_NORMAL, FIELD_TEXT_ALIGN_LEFT);
            }
        }
    }
    return result;
}

#include "../../common/save_file/skip_hex_digits.inc.c"
#include "../../common/save_file/terminate_multibyte_text.inc.c"

/**
 * @brief Clear the element pool: drop each element's scroll flag and free it,
 *        and select the sub-overlay frame style.
 */
inline void addhero_clear_elements(void)
{
    CardMenuElement* p;
    s32 i;

    g_field_menu_frame_style = FIELD_MENU_FRAME_STYLE_SUBSCREEN;
    p = g_card_menu_element_pool;
    for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++)
    {
        p->size.bits.flag = 0;
        p->attr.bits.state = CARD_MENU_ELEMENT_FREE;
        p++;
    }
}

#include "../../common/card_menu/card_menu_alloc_element.inc.c"

/**
 * @brief Update and render the active ADDHERO UI elements.
 * @param draw_state Draw state holding the primitive cursor and frame flag.
 */
void card_menu_update_and_draw_elements(FieldRenderHalf* draw_state)
{
    void* prim;
    u_long* ot;
    CardMenuElement* element;
    s32 i;
    DRAWENV draw_env;
    s32 scaled_width;
    s32 scaled_height;

    prim = draw_state->primitive_cursor;
    ot = draw_state->ordering_table;

    if ((g_card_entry_state < CARD_MENU_ENTRY_COUNT_LIMIT) && (g_card_menu_element_pool[CARD_MENU_ELEMENT_MAIN].attr.bits.state == CARD_MENU_ELEMENT_OPEN) &&
        (g_card_menu_element_pool[CARD_MENU_ELEMENT_MAIN].size.bits.flag != 0))
    {
        if ((g_card_entry_state * CARD_MENU_ENTRY_ROW_HEIGHT) > (g_card_menu_scroll_y + CARD_MENU_LIST_HEIGHT))
        {
            prim = field_draw_menu_scroll_arrow(prim, ot, CARD_MENU_EXCHANGE_SCROLL_ARROW_X, CARD_MENU_SCROLL_ARROW_DOWN_Y, FIELD_MENU_ARROW_DOWN);
        }
        if (g_card_menu_scroll_y != 0)
        {
            prim = field_draw_menu_scroll_arrow(prim, ot, CARD_MENU_EXCHANGE_SCROLL_ARROW_X, CARD_MENU_SCROLL_ARROW_UP_Y, FIELD_MENU_ARROW_UP);
        }
    }

    if (draw_state->display_rect.y != 0)
    {
        SetDefDrawEnv(&draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    }
    else
    {
        SetDefDrawEnv(&draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    }

    element = g_card_menu_element_pool;
    for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.bits.state != CARD_MENU_ELEMENT_FREE)
        {
            SetDrawEnv((DR_ENV*)prim, &draw_env);
            addPrim(ot, prim);
            prim = (DR_ENV*)prim + 1;

            switch (element->attr.bits.state)
            {
            case CARD_MENU_ELEMENT_OPENING:
                g_pad_input = 0;
                {
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                    s32 width = CARD_MENU_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.bits.transition_step) / CARD_MENU_ELEMENT_TRANSITION_STEPS;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / CARD_MENU_ELEMENT_TRANSITION_STEPS;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);

                    prim = field_draw_menu_frame(prim, ot, x + (CARD_MENU_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                 element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                                 draw_state->display_rect.y, i == CARD_MENU_ELEMENT_MODAL);
                }
                element->attr.bits.transition_step++;
                if (element->attr.bits.transition_step == CARD_MENU_ELEMENT_TRANSITION_STEPS)
                {
                    field_reset_input_repeat();
                    element->attr.bits.state = CARD_MENU_ELEMENT_OPEN;
                }
                break;

            case CARD_MENU_ELEMENT_OPEN:
                prim = element->draw(ot, prim, 0, 0);
                {
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);

                    prim = field_draw_menu_frame(prim, ot, element->attr.bits.x, element->attr.bits.y, CARD_MENU_ELEMENT_WIDTH(element, width_low),
                                                 element->size.bits.height, draw_state->display_rect.y, i == CARD_MENU_ELEMENT_MODAL);
                }
                if (element->attr.bits.transition_step != 0)
                {
                    element->attr.bits.transition_step--;
                }
                break;

            case CARD_MENU_ELEMENT_CLOSING:
                g_pad_input = 0;
                {
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                    s32 width = CARD_MENU_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.bits.transition_step) / CARD_MENU_ELEMENT_TRANSITION_STEPS;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / CARD_MENU_ELEMENT_TRANSITION_STEPS;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);

                    prim = field_draw_menu_frame(prim, ot, x + (CARD_MENU_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                 element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                                 draw_state->display_rect.y, i == CARD_MENU_ELEMENT_MODAL);
                }
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.transition_step = CARD_MENU_ELEMENT_CLOSED_FRAMES;
                    element->attr.bits.state = CARD_MENU_ELEMENT_CLOSED;
                }
                break;

            case CARD_MENU_ELEMENT_CLOSED:
                g_pad_input = 0;
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.state = CARD_MENU_ELEMENT_FREE;
                }
                break;
            }
        }
    }

    draw_state->primitive_cursor = prim;
}

#include "../../common/card_menu/card_menu_deactivate_primary_element.inc.c"

#include "../../common/encoded_text/encoded_text_append.inc.c"
#include "../../common/encoded_text/encoded_text_byte_length.inc.c"
#include "../../common/encoded_text/encoded_text_copy.inc.c"

/**
 * @brief Draw the load prompt in the modal window and act on its answer.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset; the text is centred in the message window.
 * @param y_offset Vertical transition offset.
 * @return Primitive-buffer cursor after the prompt.
 * @note A card error closes the prompt and rescans the card; Circle or "no" closes
 *       it and aborts the load; "yes" turns the window into the load progress screen.
 */
void* addhero_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;
    void* result;
    s32 x;
    s32 status;
    CardMenuElement* element;

    x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
    result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_load_prompt, CARD_MENU_TEXT_LOAD_PROMPT), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                             FIELD_TEXT_ALIGN_CENTER);
    result = card_menu_draw_choice_prompt(result, ot, x, CARD_MENU_LINE_HEIGHT - y_offset);

    status = poll_and_retry_card_info();
    if (status == CARD_EVENT_ERROR || status == CARD_EVENT_TIMEOUT)
    {
        card_menu_deactivate_primary_element();
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        card_reset_entry_ranks();
        g_card_step = NULL;
    }
    else if ((g_pad_input & PAD_BTN_CIRCLE) || ((g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK) && (g_card_menu_choice_toggle != CARD_MENU_CHOICE_YES)))
    {
        card_menu_deactivate_primary_element();
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_step = g_addhero_loadseq_abort;
    }
    else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
        g_addhero_progress_active = 1;
        g_card_step = g_addhero_loadseq_load_begin;
        element = &g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL];
        element->draw = addhero_draw_load_progress;
        element->attr.bits.transition_step = 1;
        element->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
        element->attr.bits.x = CARD_MENU_MESSAGE_X;
        element->attr.bits.y = CARD_MENU_EXCHANGE_MESSAGE_Y;
        element->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
        element->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_MESSAGE_WIDTH);
    }
    return result;
}

/**
 * @brief Draw the loading message and progress bar, and add the hero once the
 *        selected save has been read.
 * @param ot Ordering-table entry the text and bar are linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset; the text is centred in the message window.
 * @param y_offset Vertical transition offset.
 * @return Primitive-buffer cursor after the window contents.
 * @note The save's hero replaces the guest slot, which keeps its own
 *       pad_controlled bit. A save that fails validation opens the load-failed dialog.
 */
void* addhero_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;
    u16* text_table;
    SaveFile* file;
    void* result;
    s32 x;
    u32 pad_controlled;

    x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
    result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_loading, CARD_MENU_TEXT_LOADING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                             FIELD_TEXT_ALIGN_CENTER);
    text_table = CARD_MENU_TEXT_TABLE(g_addhero_text_loading, CARD_MENU_TEXT_LOADING);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x,
                             CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                             (CARD_MENU_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = addhero_draw_progress_bar(result, ot);

    if (g_addhero_progress_active == 0)
    {
        file = &g_addhero_save_file;
        card_menu_deactivate_primary_element();
        if (validate_save_file(file) == 0)
        {
            addhero_open_status_dialog(ADDHERO_DIALOG_INVALID_SAVE);
            return result;
        }

        field_play_sound(FIELD_SOUND_LOAD_DONE, AKAO_PAN_CENTER);
        pad_controlled = g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.bits.pad_controlled;
        bcopy((u8*)&file->saved_game.characters[FIELD_PARTY_HERO], (u8*)&g_saved_game_ctx->characters[FIELD_PARTY_GUEST], sizeof(FieldCharacterRecord));
        g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.bits.pad_controlled = pad_controlled;
        g_saved_game_ctx->guest_origin.ids.game_id = file->saved_game.identity.ids.game_id;
        g_saved_game_ctx->guest_origin.ids.save_id = file->saved_game.identity.ids.save_id;
        g_saved_game_ctx->guest_loaded = 1;
        card_menu_close_all_elements();
        field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
        g_addhero_result = ADDHERO_RESULT_LOADED;
    }

    return result;
}

/**
 * @brief Draw the transfer progress bar: a gradient across the top of the message
 *        window that fills over CARD_MENU_PROGRESS_FULL_TICKS VSyncs.
 * @param quad Primitive-buffer cursor the bar is written to.
 * @param ot Ordering-table entry the bar is linked into.
 * @return Primitive-buffer cursor after the bar, or @p quad while no transfer is running.
 */
inline void* addhero_draw_progress_bar(POLY_G4* quad, u_long* ot)
{
    s32 elapsed;
    s32 width;

    if (g_addhero_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_addhero_progress_start_tick;
        if (elapsed > CARD_MENU_PROGRESS_FULL_TICKS)
        {
            elapsed = CARD_MENU_PROGRESS_FULL_TICKS;
        }
        width = elapsed * CARD_MENU_MESSAGE_WIDTH;
        SET_BGR0_PACKED(quad, CARD_MENU_PROGRESS_TOP_LEFT_COLOR);
        SET_POLY_G4_BGR1_PACKED(quad, CARD_MENU_PROGRESS_TOP_RIGHT_COLOR);
        SET_POLY_G4_BGR3_PACKED(quad, CARD_MENU_PROGRESS_BOTTOM_RIGHT_COLOR);
        SET_POLY_G4_BGR2_PACKED(quad, CARD_MENU_PROGRESS_BOTTOM_LEFT_COLOR);
        setPolyG4(quad);
        quad->x0 = quad->x2 = 0;
        quad->x1 = quad->x3 = width / CARD_MENU_PROGRESS_FULL_TICKS;
        quad->y0 = quad->y1 = 0;
        quad->y2 = quad->y3 = CARD_MENU_MESSAGE_HEIGHT;
        addPrim(ot, quad);
        quad++;
    }
    return quad;
}

/**
 * @brief Turn the modal window into a dialog showing @p message_id and abandon
 *        any card transfer; acknowledging the dialog returns to the browser.
 * @param message_id CARD_MENU_DIALOG_* or ADDHERO_DIALOG_INVALID_SAVE message to show.
 */
inline void addhero_open_status_dialog(s32 message_id)
{
    CardMenuElement* element;

    field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    element = &g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL];
    element->draw = addhero_draw_status_dialog;
    element->attr.bits.transition_step = 1;
    element->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
    element->attr.bits.x = CARD_MENU_DIALOG_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_DIALOG_Y;
    element->size.bits.width_high = CARD_MENU_DIALOG_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_EXCHANGE_DIALOG_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_DIALOG_WIDTH);
    field_reset_input_repeat();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = CARD_MENU_SELECTION_NONE;
    g_addhero_io_busy = 0;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    card_reset_entry_ranks();
    g_card_step = NULL;
    g_addhero_dialog_state = message_id;
}

/**
 * @brief Turn the main window into a dialog showing @p message_id and abandon
 *        any card transfer; acknowledging the dialog leaves the overlay.
 * @param message_id CARD_MENU_DIALOG_* or ADDHERO_DIALOG_INVALID_SAVE message to show.
 */
void addhero_open_exit_dialog(s32 message_id)
{
    CardMenuElement* element;

    field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    element = &g_card_menu_element_pool[CARD_MENU_ELEMENT_MAIN];
    element->draw = addhero_draw_exit_dialog;
    element->attr.bits.transition_step = 1;
    element->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
    element->attr.bits.x = CARD_MENU_DIALOG_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_DIALOG_Y;
    element->size.bits.width_high = CARD_MENU_DIALOG_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_EXCHANGE_DIALOG_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_DIALOG_WIDTH);
    field_reset_input_repeat();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = CARD_MENU_SELECTION_NONE;
    g_addhero_io_busy = 0;
    card_reset_entry_ranks();
    g_card_step = NULL;
    g_addhero_dialog_state = message_id;
}

/**
 * @brief Draw the status dialog's message; confirming closes the dialog.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset; the text is centred in the dialog window.
 * @param y_offset Vertical transition offset.
 * @return Primitive-buffer cursor after the message.
 */
void* addhero_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;

    switch (g_addhero_dialog_state)
    {
    case CARD_MENU_DIALOG_SAVE_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_save_failed, CARD_MENU_TEXT_SAVE_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_CARD_NOT_INSERTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_card_not_inserted, CARD_MENU_TEXT_CARD_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_not_pocketstation, CARD_MENU_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_LOAD_FAILED:
    case ADDHERO_DIALOG_INVALID_SAVE:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        card_menu_deactivate_primary_element();
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Draw the exit dialog's message; confirming closes every window and
 *        leaves the overlay with ADDHERO_RESULT_CANCELLED.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset; the text is centred in the dialog window.
 * @param y_offset Vertical transition offset.
 * @return Primitive-buffer cursor after the message.
 */
void* addhero_draw_exit_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;

    switch (g_addhero_dialog_state)
    {
    case CARD_MENU_DIALOG_SAVE_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_save_failed, CARD_MENU_TEXT_SAVE_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_CARD_NOT_INSERTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_card_not_inserted, CARD_MENU_TEXT_CARD_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_not_pocketstation, CARD_MENU_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_LOAD_FAILED:
    case ADDHERO_DIALOG_INVALID_SAVE:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        g_addhero_result = ADDHERO_RESULT_CANCELLED;
        addhero_clear_elements();
        field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Draw the main window of mode 1 and run the save-back flow for the
 *        current g_card_entry_state.
 * @param ot Ordering-table entry the text and bar are linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset; the text is centred in the message window.
 * @param y_offset Vertical transition offset.
 * @return Primitive-buffer cursor after the window contents.
 * @note While entries are being checked, the window steps through them until one
 *       is the guest's load file (a Legend of Mana save whose identity matches
 *       guest_origin), then reads it, asks to overwrite the 2P data and writes it.
 *       Circle closes the overlay and the card-switch buttons rescan the other slot.
 */
void* addhero_draw_transfer_status(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;
    SaveFile* file;

    switch (g_card_entry_state)
    {
    case CARD_MENU_ENTRY_STATE_NO_GAME_DATA:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_UNFORMATTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_CHECKING_CARD:
    {
        s32 message_x;
        u16* text_table;

        message_x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
        text_table = &g_addhero_text_table;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               (CARD_MENU_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    break;
    case CARD_MENU_ENTRY_STATE_CARD_FULL:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_no_card, CARD_MENU_TEXT_NO_CARD), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_card_access_failed, CARD_MENU_TEXT_CARD_ACCESS_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_NO_SAVE_DATA:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_no_save_data, CARD_MENU_TEXT_NO_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_NO_LOAD_FILE:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_no_load_file, CARD_MENU_TEXT_NO_LOAD_FILE), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_LOAD_PROGRESS:
    {
        s32 message_x;
        u16* text_table;

        message_x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_loading, CARD_MENU_TEXT_LOADING), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_addhero_text_loading, CARD_MENU_TEXT_LOADING);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               (CARD_MENU_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = addhero_draw_progress_bar(prim, ot);
        if (g_addhero_progress_active == 0)
        {
            file = &g_addhero_save_file;
            if (validate_save_file(file) == 0)
            {
                addhero_open_status_dialog(ADDHERO_DIALOG_INVALID_SAVE);
                return prim;
            }
            field_play_sound(FIELD_SOUND_LOAD_DONE, AKAO_PAN_CENTER);
            g_card_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
            card_menu_enable_choice_toggle();
            field_reset_input_repeat();
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_CONFIRM_NO_SAVE:
    {
        s32 message_x;

        message_x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_2p_data_not_saved, CARD_MENU_TEXT_2P_DATA_NOT_SAVED), FIELD_TEXT_COLOR_NORMAL, message_x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = card_menu_draw_choice_prompt(prim, ot, message_x, CARD_MENU_LINE_HEIGHT - y_offset);
        if ((g_pad_input & PAD_BTN_CIRCLE) || ((g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK) && (g_card_menu_choice_toggle != CARD_MENU_CHOICE_YES)))
        {
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            card_menu_enable_choice_toggle();
            g_card_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
            field_reset_input_repeat();
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
            g_addhero_result = ADDHERO_RESULT_CANCELLED;
            addhero_clear_elements();
            field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
            field_reset_input_repeat();
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_SAVE_CONFIRM:
    {
        s32 message_x;
        u16* text_table;
        s32 checksum;

        message_x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_found_load_file, CARD_MENU_TEXT_FOUND_LOAD_FILE), FIELD_TEXT_COLOR_NORMAL, message_x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_addhero_text_found_load_file, CARD_MENU_TEXT_FOUND_LOAD_FILE);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_OVERWRITE_2P_DATA), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = card_menu_draw_choice_prompt(prim, ot, message_x, (CARD_MENU_LINE_HEIGHT * 2) - y_offset);
        if ((g_pad_input & PAD_BTN_CIRCLE) || ((g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK) && (g_card_menu_choice_toggle != CARD_MENU_CHOICE_YES)))
        {
            card_menu_enable_choice_toggle();
            g_card_entry_state = ADDHERO_ENTRY_STATE_CONFIRM_NO_SAVE;
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            field_reset_input_repeat();
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
            file = &g_addhero_save_file;
            bcopy((u8*)&g_saved_game_ctx->characters[FIELD_PARTY_GUEST], (u8*)&file->saved_game.characters[FIELD_PARTY_HERO], sizeof(FieldCharacterRecord));
            file->saved_game.characters[FIELD_PARTY_HERO].info.bits.pad_controlled = 1;
            checksum = compute_save_checksum(file);
            file->magic = SAVE_FILE_MAGIC;
            file->checksum = checksum;
            g_addhero_write_in_progress = 1;
            g_card_step = g_addhero_loadseq_save_begin;
            g_card_entry_state = ADDHERO_ENTRY_STATE_SAVE_PROGRESS;
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_SAVE_PROGRESS:
    {
        s32 message_x;
        u16* text_table;

        message_x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_addhero_text_saving, CARD_MENU_TEXT_SAVING), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_addhero_text_saving, CARD_MENU_TEXT_SAVING);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               (CARD_MENU_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = addhero_draw_progress_bar(prim, ot);
        if (g_addhero_write_in_progress == 0)
        {
            g_saved_game_ctx->characters[FIELD_PARTY_GUEST].name[0] = 0;
            field_play_sound(FIELD_SOUND_SAVE_DONE, AKAO_PAN_CENTER);
            addhero_clear_elements();
            field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
            g_addhero_result = ADDHERO_RESULT_SAVED;
        }
    }
    break;
    default:
    {
        s32 message_x;
        u16* text_table;

        message_x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
        text_table = &g_addhero_text_table;
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               (CARD_MENU_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
        if (g_addhero_entry_scan_active == 0)
        {
            if (g_addhero_io_busy != 0)
            {
                return prim;
            }
            if (*g_card_step >= CARD_MENU_STEP_SCAN_ENTRIES && *g_card_step <= CARD_MENU_STEP_SCAN_DONE)
            {
                return prim;
            }
            if ((strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) != 0) ||
                (g_addhero_entry_file.saved_game.identity.word != g_saved_game_ctx->guest_origin.word))
            {
                g_card_menu_selected_row++;
                if (g_card_menu_selected_row >= g_card_entry_state)
                {
                    if (g_card_entry_state != 0)
                    {
                        g_card_entry_state = ADDHERO_ENTRY_STATE_NO_LOAD_FILE;
                    }
                    else
                    {
                        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
                    }
                }
                else
                {
                    addhero_commit_selected_entry();
                    card_menu_scroll_to_selection();
                }
            }
            else
            {
                g_addhero_progress_start_tick = VSync(-1);
                g_addhero_progress_active = 1;
                g_card_step = g_addhero_loadseq_load_progress;
                g_card_entry_state = ADDHERO_ENTRY_STATE_LOAD_PROGRESS;
            }
        }
    }
    break;
    case CARD_MENU_ENTRY_STATE_BLANK:
        break;
    }

    if (g_addhero_io_busy != 0 || g_card_entry_state == ADDHERO_ENTRY_STATE_LOAD_PROGRESS || g_card_entry_state == ADDHERO_ENTRY_STATE_SAVE_PROGRESS ||
        g_card_entry_state == ADDHERO_ENTRY_STATE_SAVE_CONFIRM || g_card_entry_state == ADDHERO_ENTRY_STATE_CONFIRM_NO_SAVE)
    {
        return prim;
    }

    if (g_pad_input & PAD_BTN_CIRCLE)
    {
        D_80122718 = 3;
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        card_menu_close_all_elements();
        return prim;
    }

    if ((g_pad_input & CARD_MENU_CARD_SWITCH_BUTTON_MASK) && (g_card_entry_state != CARD_MENU_ENTRY_STATE_CHECKING_CARD))
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        addhero_reset_state();
        g_addhero_progress_bar_active = 0;
        g_card_step = &g_addhero_loadseq_start;
    }
    return prim;
}

#include "../../common/card_menu/card_menu_draw_icon_highlight.inc.c"

#include "../../common/card_menu/card_menu_enable_choice_toggle.inc.c"

#include "../../common/card_menu/card_menu_draw_choice_prompt.inc.c"

#include "../../common/save_file/validate_save_file.inc.c"
#include "../../common/save_file/compute_save_checksum.inc.c"
