#include "field_text.h"
#include "addhero_internal.h"

/**
 * @brief Address of the ADDHERO text whose table offset is @p offset.
 */
#define ADDHERO_TEXT_BY_OFFSET(table, offset) ((u8*)((uintptr_t)(offset) + (uintptr_t)(table)))

/**
 * @brief Card slot label layout: window and dimming-tile width, label text X,
 *        and the slot-0 window X.
 * @note JP narrows both labels to 0x70 and moves slot 0 right.
 */
#if defined(VERSION_JP)
#define ADDHERO_CARD_LABEL_WIDTH 0x70
#define ADDHERO_CARD_LABEL_TEXT_X 0x38
#define ADDHERO_CARD_SLOT0_LABEL_X 0x28
#else
#define ADDHERO_CARD_LABEL_WIDTH 0x80
#define ADDHERO_CARD_LABEL_TEXT_X 0x40
#define ADDHERO_CARD_SLOT0_LABEL_X 0x18
#endif

/**
 * @brief Entry list columns: the suffix value and the rank marker.
 * @note JP moves both right.
 */
#if defined(VERSION_JP)
#define ADDHERO_ENTRY_VALUE_X 0x94
#define ADDHERO_ENTRY_MARKER_X 0xCC
#else
#define ADDHERO_ENTRY_VALUE_X 0x86
#define ADDHERO_ENTRY_MARKER_X 0xC0
#endif

/** @brief Entry list columns shared by both versions: the file label, number label and "+" marker (right edge). */
#define ADDHERO_ENTRY_LABEL_X 1
#define ADDHERO_ENTRY_NUMBER_LABEL_X 112
#define ADDHERO_ENTRY_PLUS_RIGHT_X 242

/** @brief Colour of the semi-transparent selected-row highlight. */
#define ADDHERO_HIGHLIGHT_COLOR GPU_COLOR_WORD(0xF0, 0x80, 0xF0)

/** @brief Colour of the semi-transparent tile that dims the inactive card slot's label. */
#define ADDHERO_INACTIVE_LABEL_COLOR GPU_COLOR_WORD(0x10, 0x10, 0x10)

/** @brief Details window text: second column, line spacing and the play-time digits (right edges). */
#define ADDHERO_DETAILS_LINE_HEIGHT 16
#if defined(VERSION_JP)
#define ADDHERO_DETAILS_TEXT_X 80
#define ADDHERO_DETAILS_HOURS_RIGHT_X 100
#define ADDHERO_DETAILS_TIME_SEPARATOR_X 100
#define ADDHERO_DETAILS_MINUTES_TENS_RIGHT_X 120
#define ADDHERO_DETAILS_MINUTES_RIGHT_X 130
#else
#define ADDHERO_DETAILS_TEXT_X 84
#define ADDHERO_DETAILS_HOURS_RIGHT_X 112
#define ADDHERO_DETAILS_TIME_SEPARATOR_X 111
#define ADDHERO_DETAILS_MINUTES_TENS_RIGHT_X 125
#define ADDHERO_DETAILS_MINUTES_RIGHT_X 133
#endif

/**
 * @brief Reset overlay state and build the initial UI elements.
 * @param work_base Work-RAM base (always 0x80170000); stored in g_addhero_work_ram_base, unused so far.
 * @param mode Mode selector, stored in g_addhero_mode.
 */
void addhero_init(s32 work_base, s32 mode)
{
    RECT rect;

    g_addhero_mode = mode;
    g_card_entry_state = ADDHERO_ENTRY_STATE_CHECKING_CARD;
    g_card_slot = 0;

    addhero_reset_entry_ranks();
    g_addhero_result = ADDHERO_RESULT_CANCELLED;
    addhero_init_card_events();
    g_addhero_icon_phase = 0;
    field_set_default_fade_target();

    setRECT(&rect, OVERLAY_INIT_CLEAR_VRAM_X, OVERLAY_INIT_CLEAR_VRAM_Y, OVERLAY_INIT_CLEAR_VRAM_W, OVERLAY_INIT_CLEAR_VRAM_H);

    ClearImage(&rect, 0, 0, 0);
    reset_glyph_cache();

    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = ADDHERO_SELECTION_NONE;
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
 */
s32 addhero_state_step(AddheroDrawState* draw_state)
{
    if (g_addhero_exit_requested != 0)
    {
        shutdown_card_events();
        field_text_reset_windows();
        DrawSync(0);
        return g_addhero_exit_requested;
    }

    field_text_reset_scratch();
    begin_glyph_cache_frame();
    addhero_update_state(draw_state);
    evict_unused_glyphs();
    field_text_upload_immediate_cache();
    g_addhero_frame_parity ^= 1;
    return 0;
}

/**
 * @brief Reset scroll/selection state and populate the UI element pool for the current mode.
 * @note mode != 0: transfer layout (status + two card-slot labels). mode == 0: full browser
 *       (entry list, mode glyph, two slot labels, entry details).
 */
void addhero_build_ui_elements(void)
{
    AddheroElement* element;
    s32 unused[2];

    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_selected_row = 0;
    g_addhero_selection_status = ADDHERO_SELECTION_NONE;
    g_addhero_items = g_saved_game_ctx->items;
    addhero_clear_elements();
    g_addhero_load_flow_active = 0;
    if (g_addhero_mode != 0)
    {
        g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
        element = addhero_alloc_element();
        element->draw_handler = addhero_draw_transfer_status;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = ADDHERO_MESSAGE_X;
        element->attr.bits.y = ADDHERO_MESSAGE_Y;
        element->size.bits.width_high = ADDHERO_MESSAGE_WIDTH >> 8;
        element->size.bits.height = ADDHERO_MESSAGE_HEIGHT;
        ADDHERO_SET_ELEMENT_WIDTH_LOW(element, ADDHERO_MESSAGE_WIDTH);

        element = addhero_alloc_element();
        element->draw_handler = addhero_draw_card_slot0_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = ADDHERO_CARD_SLOT0_LABEL_X;
        element->attr.bits.y = ADDHERO_CARD_LABEL_TRANSFER_Y;
        element->size.bits.width_high = 0;
        element->size.bits.height = ADDHERO_CARD_LABEL_HEIGHT;
        ADDHERO_SET_ELEMENT_WIDTH_LOW(element, ADDHERO_CARD_LABEL_WIDTH);

        element = addhero_alloc_element();
        element->draw_handler = addhero_draw_card_slot1_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = ADDHERO_CARD_SLOT1_LABEL_X;
        element->attr.bits.y = ADDHERO_CARD_LABEL_TRANSFER_Y;
        element->size.bits.width_high = 0;
        element->size.bits.height = ADDHERO_CARD_LABEL_HEIGHT;
        ADDHERO_SET_ELEMENT_WIDTH_LOW(element, ADDHERO_CARD_LABEL_WIDTH);
        g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        return;
    }

    g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    element = addhero_alloc_element();
    element->draw_handler = addhero_draw_entry_list;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = ADDHERO_LIST_X;
    element->attr.bits.y = ADDHERO_LIST_Y;
    element->size.bits.width_high = ADDHERO_LIST_WIDTH >> 8;
    element->size.bits.height = ADDHERO_LIST_HEIGHT;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(element, ADDHERO_LIST_WIDTH);
    element->size.bits.scrollable = 1;

    element = addhero_alloc_element();
    element->draw_handler = addhero_draw_mode_glyph;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = ADDHERO_TITLE_X;
    element->attr.bits.y = ADDHERO_TITLE_Y;
    element->size.bits.width_high = ADDHERO_TITLE_WIDTH >> 8;
    element->size.bits.height = ADDHERO_TITLE_HEIGHT;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(element, ADDHERO_TITLE_WIDTH);

    element = addhero_alloc_element();
    element->draw_handler = addhero_draw_card_slot0_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = ADDHERO_CARD_SLOT0_LABEL_X;
    element->attr.bits.y = ADDHERO_CARD_LABEL_BROWSER_Y;
    element->size.bits.width_high = 0;
    element->size.bits.height = ADDHERO_CARD_LABEL_HEIGHT;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(element, ADDHERO_CARD_LABEL_WIDTH);

    element = addhero_alloc_element();
    element->draw_handler = addhero_draw_card_slot1_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = ADDHERO_CARD_SLOT1_LABEL_X;
    element->attr.bits.y = ADDHERO_CARD_LABEL_BROWSER_Y;
    element->size.bits.width_high = 0;
    element->size.bits.height = ADDHERO_CARD_LABEL_HEIGHT;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(element, ADDHERO_CARD_LABEL_WIDTH);

    element = addhero_alloc_element();
    element->draw_handler = addhero_draw_selected_entry_details;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = ADDHERO_DETAILS_X;
    element->attr.bits.y = ADDHERO_DETAILS_Y;
    element->size.bits.width_high = ADDHERO_DETAILS_WIDTH >> 8;
    element->size.bits.height = ADDHERO_DETAILS_HEIGHT;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(element, ADDHERO_DETAILS_WIDTH);
    g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
}

/**
 * @brief Run one frame of overlay logic: update elements, advance the load
 *        sequence when armed, sample pad input, and step the scroll animation.
 * @param draw_state Frame drawing context passed to the element renderer.
 */
void addhero_update_state(AddheroDrawState* draw_state)
{
    addhero_update_elements(draw_state);
    g_addhero_icon_phase += 2;
    if (g_addhero_element1.attr.bits.state == ADDHERO_ELEMENT_STATE_ACTIVE && g_addhero_element1.attr.bits.transition_step == 0)
    {
        addhero_update_load_sequence();
    }
    if ((u16)g_pad_input == PAD_ALL_BUTTONS)
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
 * @return Nothing: the original is declared int but returns no value, and its caller ignores it.
 */
s32 addhero_update_load_sequence(void)
{
    s32 result;

    if (g_card_entry_state >= ADDHERO_ENTRY_COUNT_LIMIT)
    {
        if (g_card_step == NULL)
        {
            g_card_step = &g_addhero_loadseq_start;
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
            g_card_entry_state = ADDHERO_ENTRY_STATE_BROWSER_READ_ERROR;
        }
        else
        {
            g_card_entry_state = ADDHERO_ENTRY_STATE_NO_GAME_DATA;
        }
        g_card_step = g_addhero_loadseq_abort;
    }
    else
    {
        switch (result)
        {
        case ADDHERO_LOAD_RESULT_NONE:
            break;
        case ADDHERO_LOAD_RESULT_COMPLETE:
            g_card_step = g_addhero_loadseq_done;
            g_addhero_load_flow_active = 0;
            break;
        case ADDHERO_LOAD_RESULT_CARD_ERROR:
            if (g_addhero_mode == 0)
            {
                g_card_entry_state = ADDHERO_ENTRY_STATE_BROWSER_READ_ERROR;
            }
            else
            {
                g_card_entry_state = ADDHERO_ENTRY_STATE_NO_GAME_DATA;
            }
            /* fallthrough */
        case ADDHERO_LOAD_RESULT_ABORT:
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
    AddheroElement* prompt;
    struct DIRENTRY* selected_entry;
    SavedGameLayout* entry;

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
    if (g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
    {
        return;
    }

    entry_count = g_card_entry_state;
    if (entry_count == ADDHERO_ENTRY_STATE_CHECKING_CARD)
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
    if (*g_card_step >= ADDHERO_STEP_SCAN_ENTRIES && *g_card_step <= ADDHERO_STEP_SCAN_DONE)
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
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
        addhero_close_all_elements();
        return;
    }
    if (input & ADDHERO_CARD_SWITCH_BUTTON_MASK)
    {
        field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
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

    for (; move_count != 0; move_count--)
    {
        if (g_pad_input & PAD_BTN_UP)
        {
            g_addhero_selected_row--;
            if (g_addhero_selected_row < 0)
            {
                g_addhero_selected_row = g_card_entry_state - 1;
            }
        }
        if (g_pad_input & PAD_BTN_DOWN)
        {
            g_addhero_selected_row++;
            if (g_addhero_selected_row >= g_card_entry_state)
            {
                g_addhero_selected_row = 0;
            }
        }
    }

    if (g_pad_input & (PAD_BTN_UP | PAD_BTN_DOWN))
    {
        addhero_commit_selected_entry();
        field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
        addhero_scroll_to_selection();
        return;
    }

    if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
    {
        selected_entry = &g_card_entries[g_card_slot][g_addhero_selected_row];
        if (strncmp(g_lom_save_filename_prefix, selected_entry->name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            entry = &g_addhero_entry_file.saved_game;
            if ((entry->identity.ids.game_id != g_saved_game_ctx->identity.ids.game_id) &&
                ((g_save_compatibility_tag == SAVE_TAG_ANY) || (entry->compatibility_tag == g_save_compatibility_tag)))
            {
                prompt = addhero_alloc_element();
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.x = ADDHERO_MESSAGE_X;
                prompt->attr.bits.y = ADDHERO_MESSAGE_Y;
                prompt->size.bits.width_high = ADDHERO_MESSAGE_WIDTH >> 8;
                prompt->size.bits.height = ADDHERO_PROMPT_HEIGHT;
                ADDHERO_SET_ELEMENT_WIDTH_LOW(prompt, ADDHERO_MESSAGE_WIDTH);
                addhero_enable_choice_toggle();
                prompt->draw_handler = addhero_draw_load_prompt;
                restart_card_sequence();
                field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
                return;
            }
        }
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
    }
}

/**
 * @brief Reset scroll/selection state and flip to the other card slot, then
 *        clear ranks and pad input to restart browsing.
 */
void addhero_reset_state(void)
{
    g_addhero_load_flow_active = 0;
    g_card_step = NULL;
    g_card_entry_state = ADDHERO_ENTRY_STATE_CHECKING_CARD;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_selected_row = 0;
    g_addhero_selection_status = ADDHERO_SELECTION_NONE;
    g_card_slot ^= 1;
    addhero_reset_entry_ranks();
    field_reset_input_repeat();
    g_pad_input = 0;
}

/**
 * @brief Start the closing transition of every active pool element so they animate out.
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
            element->attr.bits.transition_step = ADDHERO_ELEMENT_TRANSITION_STEPS;
        }
    }
}

/**
 * @brief Retarget the list scroll so the selected row stays on screen,
 *        animating over four frames when it falls above or below the window.
 */
void addhero_scroll_to_selection(void)
{
    s32 row_y;
    s32 relative_y;

    row_y = (g_addhero_selected_row * 7) << 1;
    relative_y = row_y - g_addhero_scroll_y;

    if (relative_y > ADDHERO_LIST_HEIGHT - ADDHERO_ENTRY_ROW_HEIGHT)
    {
        g_addhero_scroll_target_y = row_y - ADDHERO_LIST_LAST_ROW_Y;
        g_addhero_scroll_frames = ADDHERO_SCROLL_FRAMES;
    }
    if (relative_y < 0)
    {
        g_addhero_scroll_target_y = row_y;
        g_addhero_scroll_frames = ADDHERO_SCROLL_FRAMES;
    }
}

/**
 * @brief Thin wrapper that runs the element update/draw pass on the active
 *        draw state.
 * @param draw_state Frame drawing context to update.
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
    s32 entry_state = g_card_entry_state;

    switch (entry_state)
    {
    case ADDHERO_ENTRY_STATE_NO_GAME_DATA:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_no_game_save_data, ADDHERO_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_BROWSER_READ_ERROR:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_no_game_save_data, ADDHERO_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_CARD_FULL:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_not_enough_blocks, ADDHERO_TEXT_NOT_ENOUGH_BLOCKS), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_no_card, ADDHERO_TEXT_NO_CARD), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_card_access_failed, ADDHERO_TEXT_CARD_ACCESS_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_NO_SAVE_DATA:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_no_save_data, ADDHERO_TEXT_NO_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_BLANK:
        break;
    case ADDHERO_ENTRY_STATE_CHECKING_CARD:
    {
        s32 message_x;
        u16* text_table;

        message_x = -x_offset + ADDHERO_LIST_WIDTH / 2;
        text_table = &g_addhero_text_table;
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               ADDHERO_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               (ADDHERO_TEXT_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
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

            message_x = -x_offset + ADDHERO_LIST_WIDTH / 2;
            text_table = &g_addhero_text_table;
            prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                                   ADDHERO_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                                   (ADDHERO_TEXT_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
            break;
        }
        entry_index = 0;
        if (entry_state > 0)
        {
            s32 list_x;
            u16 marker_offset;
            DVECTOR value_pos;
            u16* text_table;

            text_table = &g_addhero_text_table;
            list_x = -x_offset;
            do
            {
                row_y = ((entry_index * ADDHERO_ENTRY_ROW_HEIGHT) - y_offset) - g_addhero_scroll_y + 1;
                if (row_y > -ADDHERO_ENTRY_ROW_HEIGHT && row_y < ADDHERO_LIST_HEIGHT)
                {
                    if (g_addhero_entry_ranks[entry_index] >= 0)
                    {
                        value_pos.vx = list_x + ADDHERO_ENTRY_VALUE_X;
                        value_pos.vy = row_y;
                        prim = field_draw_text(
                            field_draw_number(ot, prim, g_card_entry_suffix_values[entry_index], FIELD_TEXT_COLOR_NORMAL, &value_pos, FIELD_TEXT_ALIGN_LEFT),
                            ot, ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_text_number_label), FIELD_TEXT_COLOR_NORMAL, list_x + ADDHERO_ENTRY_NUMBER_LABEL_X,
                            row_y, FIELD_TEXT_ALIGN_LEFT);
                        if ((g_addhero_rank_count - 1) == g_addhero_entry_ranks[entry_index])
                        {
                            marker_offset = text_table[ADDHERO_TEXT_NEWEST];
                            prim = field_draw_text(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, marker_offset), FIELD_TEXT_COLOR_NORMAL,
                                                   list_x + ADDHERO_ENTRY_MARKER_X, row_y, FIELD_TEXT_ALIGN_LEFT);
                        }
                        else if (g_addhero_entry_ranks[entry_index] < 2)
                        {
                            marker_offset = text_table[ADDHERO_TEXT_OLDEST];
                            prim = field_draw_text(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, marker_offset), FIELD_TEXT_COLOR_NORMAL,
                                                   list_x + ADDHERO_ENTRY_MARKER_X, row_y, FIELD_TEXT_ALIGN_LEFT);
                        }
                        if (*skip_hex_digits(&g_card_entries[g_card_slot][entry_index].name[CARD_SAVE_FILENAME_PREFIX_LENGTH]) == '+')
                        {
                            prim = field_draw_text(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_text_plus_marker), FIELD_TEXT_COLOR_NORMAL,
                                                   ADDHERO_ENTRY_PLUS_RIGHT_X - x_offset, row_y, FIELD_TEXT_ALIGN_RIGHT);
                        }
                    }
                    if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_text_mana_label), FIELD_TEXT_COLOR_NORMAL,
                                               ADDHERO_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_text_ring_ring_land_label), FIELD_TEXT_COLOR_NORMAL,
                                               ADDHERO_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_text_new_save_label), FIELD_TEXT_COLOR_NORMAL,
                                               ADDHERO_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else
                    {
                        prim = field_draw_text(prim, ot, ADDHERO_TEXT_BY_OFFSET(text_table, g_addhero_text_other_game_label), FIELD_TEXT_COLOR_NORMAL,
                                               ADDHERO_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                }
                entry_index++;
            } while (entry_index < g_card_entry_state);
        }
        row_y = ((g_addhero_selected_row * ADDHERO_ENTRY_ROW_HEIGHT) - y_offset) - g_addhero_scroll_y;

        if (g_addhero_entry_scan_active == 0)
        {
            TILE* tile = (TILE*)prim;

            *(u32*)&tile->r0 = ADDHERO_HIGHLIGHT_COLOR;
            setlen(tile, 3);
            setcode(tile, GPU_CODE_TILE | GPU_CODE_SEMI_TRANS);
            tile->w = ADDHERO_LIST_WIDTH;
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
 */
void* addhero_draw_mode_glyph(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;

    if (g_addhero_mode == 1)
    {
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_select_item, ADDHERO_TEXT_SELECT_ITEM), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    else
    {
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_select_save_data, ADDHERO_TEXT_SELECT_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
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
 */
void* addhero_draw_card_slot0_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;
    TILE* tile;

    if (g_card_slot != 0)
    {
        tile = (TILE*)prim;
        *(u32*)&tile->r0 = ADDHERO_INACTIVE_LABEL_COLOR;
        setlen(tile, 3);
        setcode(tile, GPU_CODE_TILE | GPU_CODE_SEMI_TRANS);
        setXY0(tile, 0, 0);
        setWH(tile, ADDHERO_CARD_LABEL_WIDTH, ADDHERO_CARD_LABEL_HEIGHT);
        addPrim(ot, tile);
        prim = tile + 1;
    }
    return field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_card_slot0_label, ADDHERO_TEXT_CARD_SLOT0_LABEL), FIELD_TEXT_COLOR_NORMAL,
                           -x_offset + ADDHERO_CARD_LABEL_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Draw the slot-1 card label, dimming it with a backing tile when that
 *        slot is not the active one.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 */
void* addhero_draw_card_slot1_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;
    TILE* tile;

    if (g_card_slot == 0)
    {
        tile = (TILE*)prim;
        *(u32*)&tile->r0 = ADDHERO_INACTIVE_LABEL_COLOR;
        setlen(tile, 3);
        setcode(tile, GPU_CODE_TILE | GPU_CODE_SEMI_TRANS);
        setXY0(tile, 0, 0);
        setWH(tile, ADDHERO_CARD_LABEL_WIDTH, ADDHERO_CARD_LABEL_HEIGHT);
        addPrim(ot, tile);
        prim = tile + 1;
    }
    return field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_card_slot1_label, ADDHERO_TEXT_CARD_SLOT1_LABEL), FIELD_TEXT_COLOR_NORMAL,
                           -x_offset + ADDHERO_CARD_LABEL_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
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
 */
void* addhero_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    void* result;
    DVECTOR pos;
    u8 name[0x100];
    s32 slot[FIELD_PARTY_SIZE];

    result = prim;
    if (g_addhero_selection_status == ADDHERO_SELECTION_NONE)
    {
        return result;
    }
    if (g_addhero_entry_scan_active != 0)
    {
        return result;
    }
    if (g_addhero_selection_status == ADDHERO_SELECTION_EMPTY_CARD || g_card_entry_state >= ADDHERO_ENTRY_COUNT_LIMIT)
    {
        return result;
    }
    if (g_addhero_selection_status == ADDHERO_SELECTION_NEW_SAVE)
    {
        s32 x = -x_offset;
        u16* text_table;

        result = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_new_save_title, ADDHERO_TEXT_NEW_SAVE_TITLE), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                 FIELD_TEXT_ALIGN_LEFT);
        text_table = ADDHERO_TEXT_TABLE(g_addhero_text_new_save_title, ADDHERO_TEXT_NEW_SAVE_TITLE);
        return field_draw_text(result, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_USES_TWO_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x,
                               ADDHERO_DETAILS_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_LEFT);
    }
    else
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_addhero_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
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
                    g_addhero_icon_palette = entry->icon_palette;
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
                        result = addhero_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, slot[j], i, j);
                        i += 1;
                        total += adjust;
                    }
                }

                {
                    SavedGameLayout* entry = &g_addhero_entry_file.saved_game;
                    s32 x = -x_offset;
                    s32 y = -y_offset;

                    base_y = entry->play_time;
                    pos.vx = (s16)(x + ADDHERO_DETAILS_HOURS_RIGHT_X);
                    pos.vy = (s16)y;
                    hours = base_y / SAVED_PLAY_TIME_TICKS_PER_HOUR;
                    result = field_draw_number(ot, result, hours, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                    result = field_draw_text(result, ot, FIELD_UI_TEXT_AT(g_text_time_separator_offset_bytes, FIELD_UI_TEXT_TIME_SEPARATOR),
                                             FIELD_TEXT_COLOR_NORMAL, x + ADDHERO_DETAILS_TIME_SEPARATOR_X, y, FIELD_TEXT_ALIGN_LEFT);
                    base_y = (base_y / SAVED_PLAY_TIME_TICKS_PER_MINUTE) - (hours * 60);
                    if (base_y < 10)
                    {
                        pos.vx = (s16)(x + ADDHERO_DETAILS_MINUTES_TENS_RIGHT_X);
                        pos.vy = (s16)y;
                        result = field_draw_number(ot, result, 0, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                    }
                    pos.vx = (s16)(x + ADDHERO_DETAILS_MINUTES_RIGHT_X);
                    pos.vy = (s16)y;
                    result = field_draw_number(ot, result, base_y, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                    result = field_draw_text(result, ot, entry->summary_name, FIELD_TEXT_COLOR_NORMAL, x + ADDHERO_DETAILS_TEXT_X,
                                             y + ADDHERO_DETAILS_LINE_HEIGHT, FIELD_TEXT_ALIGN_LEFT);

                    if (entry->identity.ids.game_id == g_saved_game_ctx->identity.ids.game_id)
                    {
                        result =
                            field_draw_text(result, ot, ADDHERO_TEXT_AT(g_addhero_text_same_hero_data, ADDHERO_TEXT_SAME_HERO_DATA), FIELD_TEXT_COLOR_NORMAL,
                                            x + ADDHERO_DETAILS_TEXT_X, y + ADDHERO_DETAILS_LINE_HEIGHT * 2, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else
                    {
                        result =
                            field_draw_text(result, ot, ADDHERO_TEXT(g_addhero_location_text_table, entry->track.bits.music_track), FIELD_TEXT_COLOR_NORMAL,
                                            x + ADDHERO_DETAILS_TEXT_X, y + ADDHERO_DETAILS_LINE_HEIGHT * 2, FIELD_TEXT_ALIGN_LEFT);
                    }
                }
            }
            else
            {
                result = field_draw_text(result, ot, ADDHERO_TEXT_AT(g_addhero_text_wrong_version, ADDHERO_TEXT_WRONG_VERSION), FIELD_TEXT_COLOR_NORMAL,
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
                result = draw_cached_text(result, ot, name, -x_offset, -y_offset + ADDHERO_DETAILS_LINE_HEIGHT, FIELD_TEXT_COLOR_NORMAL, FIELD_TEXT_ALIGN_LEFT);
            }
        }
    }
    return result;
}

#include "../../common/save_file/skip_hex_digits.inc.c"
#include "../../common/save_file/terminate_multibyte_text.inc.c"

/**
 * @brief Clear the element pool: drop each element's scroll flag and free it,
 *        and reset g_menu_element_counter to 0x20.
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
 * @brief Claim the first free pool element and start its opening transition.
 * @return The claimed element, or the pool base element when none are free.
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

    if ((g_card_entry_state < ADDHERO_ENTRY_COUNT_LIMIT) && (g_addhero_element_pool[ADDHERO_ELEMENT_MAIN].attr.bits.state == ADDHERO_ELEMENT_STATE_ACTIVE) &&
        (g_addhero_element_pool[ADDHERO_ELEMENT_MAIN].size.bits.scrollable != 0))
    {
        if ((g_card_entry_state * ADDHERO_ENTRY_ROW_HEIGHT) > (g_addhero_scroll_y + ADDHERO_LIST_HEIGHT))
        {
            prim = field_draw_menu_scroll_arrow(prim, ot, ADDHERO_SCROLL_ARROW_X, ADDHERO_SCROLL_ARROW_DOWN_Y, FIELD_MENU_ARROW_DOWN);
        }
        if (g_addhero_scroll_y != 0)
        {
            prim = field_draw_menu_scroll_arrow(prim, ot, ADDHERO_SCROLL_ARROW_X, ADDHERO_SCROLL_ARROW_UP_Y, FIELD_MENU_ARROW_UP);
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

                    scaled_width = (width * element->attr.bits.transition_step) / ADDHERO_ELEMENT_TRANSITION_STEPS;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / ADDHERO_ELEMENT_TRANSITION_STEPS;
                    prim = element->draw_handler(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = ADDHERO_ELEMENT_WIDTH_LOW(element);

                    prim = field_draw_menu_frame(prim, ot, x + (ADDHERO_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                 element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                                 draw_state->display_buffer_index, i == ADDHERO_ELEMENT_MODAL);
                }
                element->attr.bits.transition_step++;
                if (element->attr.bits.transition_step == ADDHERO_ELEMENT_TRANSITION_STEPS)
                {
                    field_reset_input_repeat();
                    element->attr.bits.state = ADDHERO_ELEMENT_STATE_ACTIVE;
                }
                break;

            case ADDHERO_ELEMENT_STATE_ACTIVE:
                prim = element->draw_handler(ot, prim, 0, 0);
                {
                    u32 width_low = ADDHERO_ELEMENT_WIDTH_LOW(element);

                    prim = field_draw_menu_frame(prim, ot, element->attr.bits.x, element->attr.bits.y, ADDHERO_ELEMENT_WIDTH(element, width_low),
                                                 element->size.bits.height, draw_state->display_buffer_index, i == ADDHERO_ELEMENT_MODAL);
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

                    scaled_width = (width * element->attr.bits.transition_step) / ADDHERO_ELEMENT_TRANSITION_STEPS;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / ADDHERO_ELEMENT_TRANSITION_STEPS;
                    prim = element->draw_handler(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = ADDHERO_ELEMENT_WIDTH_LOW(element);

                    prim = field_draw_menu_frame(prim, ot, x + (ADDHERO_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                 element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                                 draw_state->display_buffer_index, i == ADDHERO_ELEMENT_MODAL);
                }
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.transition_step = ADDHERO_ELEMENT_FINISH_FRAMES;
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
 */
void addhero_deactivate_primary_element(void)
{
    g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
}

#include "../../common/encoded_text/encoded_text_append.inc.c"
#include "../../common/encoded_text/encoded_text_byte_length.inc.c"
#include "../../common/encoded_text/encoded_text_copy.inc.c"
