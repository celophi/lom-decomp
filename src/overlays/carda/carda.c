#include "carda_internal.h"

/**
 * @brief Card-slot label windows: width, which is also the dimming tile's, and height.
 * @note JP narrows both labels.
 */
#if defined(VERSION_JP)
#define CARDA_CARD_LABEL_WIDTH 112
#else
#define CARDA_CARD_LABEL_WIDTH 128
#endif
#define CARDA_CARD_LABEL_HEIGHT 16

/** @brief Card-slot label window positions: slot 0 and slot 1 X, and Y by layout. */
#define CARDA_CARD_SLOT0_LABEL_X 28
#define CARDA_CARD_SLOT1_LABEL_X 164
#define CARDA_CARD_LABEL_BROWSER_Y 30
#define CARDA_CARD_LABEL_POCKETSTATION_Y 58

/** @brief Title window of the browser layout. */
#define CARDA_TITLE_X 104
#define CARDA_TITLE_Y 10
#define CARDA_TITLE_WIDTH 112
#define CARDA_TITLE_HEIGHT 16

/** @brief Title window of the PocketStation layout. */
#define CARDA_POCKETSTATION_TITLE_X 32
#define CARDA_POCKETSTATION_TITLE_Y 34
#define CARDA_POCKETSTATION_TITLE_WIDTH 256

/** @brief Details window of the browser layout. */
#define CARDA_DETAILS_X 30
#define CARDA_DETAILS_Y 142
#define CARDA_DETAILS_WIDTH 260
#define CARDA_DETAILS_HEIGHT 52

/**
 * @brief X of the entry list's suffix value column.
 * @note JP moves it right.
 */
#if defined(VERSION_JP)
#define CARDA_ENTRY_VALUE_X 0x94
#else
#define CARDA_ENTRY_VALUE_X 0x86
#endif

/** @brief Entry list columns shared by both versions: the file label, number label, rank marker and "+" marker (right edge). */
#define CARDA_ENTRY_LABEL_X 1
#define CARDA_ENTRY_NUMBER_LABEL_X 112
#define CARDA_ENTRY_MARKER_X 214
#define CARDA_ENTRY_PLUS_RIGHT_X 268

/** @brief Colour of the semi-transparent selected-row highlight. */
#define CARDA_HIGHLIGHT_COLOR GPU_COLOR_WORD(0xF0, 0x80, 0xF0)

/** @brief Colour of the semi-transparent tile that dims the inactive card slot's label. */
#define CARDA_INACTIVE_LABEL_COLOR GPU_COLOR_WORD(0x10, 0x10, 0x10)

/** @brief Details window text: second column, line spacing and the play-time digits (right edges). */
#define CARDA_DETAILS_TEXT_X 84
#define CARDA_DETAILS_LINE_HEIGHT 16
#define CARDA_DETAILS_HOURS_RIGHT_X 112
#define CARDA_DETAILS_TIME_SEPARATOR_X 111
#define CARDA_DETAILS_MINUTES_TENS_RIGHT_X 125
#define CARDA_DETAILS_MINUTES_RIGHT_X 133

/**
 * @brief Address of the CARDA text whose table offset is @p offset.
 * @note Summed as integers, offset first, like the original list drawing code.
 */
#define CARDA_TEXT_BY_OFFSET(table, offset) ((u8*)((s32)(offset) + (s32)(table)))

/**
 * @brief Reset the overlay state, clear the icon and glyph VRAM and build the windows.
 * @param work Work buffer from FIELD; the save file is built and read in it.
 * @param mode Card screen mode (CARDA_MODE_SAVE and so on), stored in g_carda_mode.
 */
void carda_init(void* work, s32 mode)
{
    RECT rect;

    g_carda_mode = mode;
    g_carda_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
    g_carda_card_slot = 0;
    carda_reset_entry_ranks();
    carda_init_card_events();
    g_carda_icon_phase = 0;
    field_set_default_fade_target();

    setRECT(&rect, OVERLAY_INIT_CLEAR_VRAM_X, OVERLAY_INIT_CLEAR_VRAM_Y, OVERLAY_INIT_CLEAR_VRAM_W, OVERLAY_INIT_CLEAR_VRAM_H);
    ClearImage(&rect, 0, 0, 0);
    carda_reset_glyph_cache();

    g_carda_save_in_progress = 0;
    g_carda_progress_active = 0;
    g_carda_selection_status = CARDA_SELECTION_NONE;
    g_carda_io_busy = 0;
    g_carda_frame_parity = 0;
    g_carda_exit_requested = 0;
    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        g_carda_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
    }
    g_carda_card_step = NULL;
    g_carda_choice_toggle = g_carda_card_slot;
    field_reset_input_repeat();
    carda_build_ui_elements();

    g_carda_save_blob = (u8*)(((u32)work + 3) & ~3);
}

/**
 * @brief Run one frame: tear down and exit if requested, else update and render.
 * @param render FIELD render buffer being built this frame.
 * @return 1 when the overlay has finished, otherwise 0.
 */
s32 carda_update_frame(FieldRenderHalf* render)
{
    if (g_carda_exit_requested != 0)
    {
        carda_shutdown_card_events();
        field_text_reset_windows();
        DrawSync(0);
        return 1;
    }
    field_text_reset_scratch();
    carda_begin_glyph_cache_frame();
    carda_update_menu(render);
    carda_evict_unused_glyphs();
    field_text_upload_immediate_cache();
    g_carda_frame_parity ^= 1;
    return 0;
}

/**
 * @brief Reset scroll and selection state and open the windows of the current mode.
 *
 * The PocketStation modes get the transfer window, both card-slot labels and
 * the title; the save and load modes get the save-file browser: entry list,
 * title, both card-slot labels and the details window.
 * @note JP changes this function; the JP build takes it from assembly.
 */
#if defined(VERSION_JP)
INCLUDE_ASM("overlays/carda/nonmatchings/carda", carda_build_ui_elements);
#else
void carda_build_ui_elements(void)
{
    CardaElement* element;
    s32 unused[2]; /* never used, but the original stack frame reserves it */

    g_carda_scroll_frames = 0;
    g_carda_scroll_target_y = 0;
    g_carda_scroll_y = 0;
    g_carda_selected_row = 0;
    g_carda_selection_status = CARDA_SELECTION_NONE;
    g_carda_items = g_saved_game_ctx->items;
    carda_clear_elements();
    g_carda_format_declined = 0;

    g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_OPENING;
    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        element = carda_alloc_element();
        element->draw = carda_draw_save_flow;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_MESSAGE_X;
        element->attr.bits.y = CARDA_TRANSFER_Y;
        element->size.bits.width_high = CARDA_MESSAGE_WIDTH >> 8;
        element->size.bits.height = CARDA_TRANSFER_HEIGHT;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, CARDA_MESSAGE_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot0_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_CARD_SLOT0_LABEL_X;
        element->attr.bits.y = CARDA_CARD_LABEL_POCKETSTATION_Y;
        element->size.bits.width_high = CARDA_CARD_LABEL_WIDTH >> 8;
        element->size.bits.height = CARDA_CARD_LABEL_HEIGHT;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, CARDA_CARD_LABEL_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot1_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_CARD_SLOT1_LABEL_X;
        element->attr.bits.y = CARDA_CARD_LABEL_POCKETSTATION_Y;
        element->size.bits.width_high = CARDA_CARD_LABEL_WIDTH >> 8;
        element->size.bits.height = CARDA_CARD_LABEL_HEIGHT;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, CARDA_CARD_LABEL_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_title;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_POCKETSTATION_TITLE_X;
        element->attr.bits.y = CARDA_POCKETSTATION_TITLE_Y;
        element->size.bits.width_high = CARDA_POCKETSTATION_TITLE_WIDTH >> 8;
        element->size.bits.height = CARDA_TITLE_HEIGHT;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, CARDA_POCKETSTATION_TITLE_WIDTH);
    }
    else
    {
        element = carda_alloc_element();
        element->draw = carda_draw_entry_list;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_LIST_X;
        element->attr.bits.y = CARDA_LIST_Y;
        element->size.bits.width_high = CARDA_LIST_WIDTH >> 8;
        element->size.bits.height = CARDA_LIST_HEIGHT;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, CARDA_LIST_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_title;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_TITLE_X;
        element->attr.bits.y = CARDA_TITLE_Y;
        element->size.bits.width_high = CARDA_TITLE_WIDTH >> 8;
        element->size.bits.height = CARDA_TITLE_HEIGHT;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, CARDA_TITLE_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot0_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_CARD_SLOT0_LABEL_X;
        element->attr.bits.y = CARDA_CARD_LABEL_BROWSER_Y;
        element->size.bits.width_high = CARDA_CARD_LABEL_WIDTH >> 8;
        element->size.bits.height = CARDA_CARD_LABEL_HEIGHT;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, CARDA_CARD_LABEL_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_card_slot1_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_CARD_SLOT1_LABEL_X;
        element->attr.bits.y = CARDA_CARD_LABEL_BROWSER_Y;
        element->size.bits.width_high = CARDA_CARD_LABEL_WIDTH >> 8;
        element->size.bits.height = CARDA_CARD_LABEL_HEIGHT;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, CARDA_CARD_LABEL_WIDTH);

        element = carda_alloc_element();
        element->draw = carda_draw_selected_entry_details;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARDA_DETAILS_X;
        element->attr.bits.y = CARDA_DETAILS_Y;
        element->size.bits.width_high = CARDA_DETAILS_WIDTH >> 8;
        element->size.bits.height = CARDA_DETAILS_HEIGHT;
        CARDA_SET_ELEMENT_WIDTH_LOW(element, CARDA_DETAILS_WIDTH);
    }
    g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
}
#endif

/**
 * @brief Run one frame of overlay logic: update the windows, advance the card
 *        sequence once the main window is open, handle input and step the scroll.
 * @param render FIELD render buffer being built this frame.
 */
void carda_update_menu(FieldRenderHalf* render)
{
    carda_update_elements(render);
    g_carda_icon_phase += 2;
    if (g_carda_element1_state.attr.bits.state == CARDA_ELEMENT_OPEN && g_carda_element1_state.attr.bits.transition_step == 0)
    {
        carda_update_card_sequence();
    }
    if ((u16)g_pad_input == PAD_ALL_BUTTONS)
    {
        g_pad_input = 0;
    }
    carda_handle_input();
    if (g_carda_scroll_frames != 0)
    {
        g_carda_scroll_y += (g_carda_scroll_target_y - g_carda_scroll_y) / g_carda_scroll_frames--;
    }
    else
    {
        g_carda_scroll_y = g_carda_scroll_target_y;
    }
}

/**
 * @brief Drive the card sequence one frame and act on its result.
 *
 * Starts the initial scan when no sequence is running and no status message
 * is up, runs the sequence until it stops asking to run again, reopens the
 * format prompt when the player confirms after declining it, and maps the
 * sequence result onto the next step table or the unformatted-card prompt.
 *
 * @return Nothing: the original is declared int but returns no value, and its caller ignores it.
 */
s32 carda_update_card_sequence(void)
{
    s32 result;
    CardaElement* prompt;

    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        if (g_carda_card_step == NULL)
        {
            switch (g_carda_entry_state)
            {
            case CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH:
            case CARDA_ENTRY_STATE_FORMAT_FAILED:
            case CARDA_ENTRY_STATE_SAVE_CORRUPT:
            case CARDA_ENTRY_STATE_NO_POCKETSTATION:
            case CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED:
            case CARDA_ENTRY_STATE_UPLOAD_FAILED:
            case CARDA_ENTRY_STATE_DOWNLOAD_FAILED:
            case CARDA_ENTRY_STATE_SELECT_SLOT:
            case CARDA_ENTRY_STATE_NOT_POCKETSTATION:
            case CARDA_ENTRY_STATE_NO_GAME_DATA:
            case CARDA_ENTRY_STATE_UNFORMATTED:
            case CARDA_ENTRY_STATE_CARD_FULL:
            case CARDA_ENTRY_STATE_ACCESS_FAILED:
            case CARDA_ENTRY_STATE_NO_SAVE_DATA:
            case CARDA_ENTRY_STATE_NO_CARD:
            case CARDA_ENTRY_STATE_CHECKING_CARD:
                return;
            default:
                g_carda_card_step = g_carda_steps_initial_scan;
                break;
            }
        }
    }
    else if (g_carda_entry_state >= CARDA_ENTRY_COUNT_INPUT_LIMIT && g_carda_card_step == NULL && g_carda_entry_state != CARDA_ENTRY_STATE_SELECT_SLOT)
    {
        g_carda_card_step = g_carda_steps_initial_scan;
    }

    do
    {
        result = carda_advance_card_sequence();
    } while (result == CARDA_SEQUENCE_RUN_AGAIN);

    if (g_carda_format_declined != 0 && (g_pad_input & CARDA_CONFIRM_BUTTON_MASK))
    {
        g_carda_entry_state = CARDA_ENTRY_STATE_UNFORMATTED;
        g_carda_format_declined = 0;

        prompt = g_carda_element_pool;
        prompt->attr.bits.state = CARDA_ELEMENT_OPENING;
        prompt->attr.bits.transition_step = 1;
        prompt->attr.bits.x = CARDA_MESSAGE_X;
        prompt->attr.bits.y = CARDA_TRANSFER_Y;
        CARDA_SET_ELEMENT_WIDTH_LOW(prompt, CARDA_MESSAGE_WIDTH);
        prompt->size.bits.width_high = CARDA_MESSAGE_WIDTH >> 8;
        prompt->size.bits.height = CARDA_TRANSFER_HEIGHT;
        carda_enable_choice_toggle();
        prompt->draw = carda_draw_format_prompt;
        carda_restart_card_sequence();
        return;
    }

    switch (result)
    {
    case CARDA_SEQUENCE_NONE:
        break;
    case CARDA_SEQUENCE_FINISHED:
        g_carda_card_step = g_carda_steps_card_reset;
        break;
    case CARDA_SEQUENCE_NO_CARD:
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            g_carda_card_step = NULL;
        }
        else
        {
            g_carda_card_step = g_carda_steps_refresh_entries;
        }
        g_carda_format_declined = 0;
        break;
    case CARDA_SEQUENCE_UNFORMATTED:
        if (g_carda_mode == CARDA_MODE_LOAD || g_carda_mode == CARDA_MODE_RETURN_PET)
        {
            g_carda_entry_state = CARDA_ENTRY_STATE_UNFORMATTED;
            if (g_carda_mode == CARDA_MODE_LOAD)
            {
                g_carda_card_step = g_carda_steps_initial_scan;
            }
        }
        else
        {
            g_carda_entry_state = CARDA_ENTRY_STATE_UNFORMATTED;
            g_carda_format_declined = 0;

            prompt = g_carda_element_pool;
            prompt->attr.bits.state = CARDA_ELEMENT_OPENING;
            prompt->attr.bits.transition_step = 1;
            prompt->attr.bits.x = CARDA_MESSAGE_X;
            prompt->attr.bits.y = CARDA_TRANSFER_Y;
            CARDA_SET_ELEMENT_WIDTH_LOW(prompt, CARDA_MESSAGE_WIDTH);
            prompt->size.bits.width_high = CARDA_MESSAGE_WIDTH >> 8;
            prompt->size.bits.height = CARDA_TRANSFER_HEIGHT;
            carda_enable_choice_toggle();
            prompt->draw = carda_draw_format_prompt;
            carda_restart_card_sequence();
        }
        break;
    }
}

/**
 * @brief Handle browser input: cancel, card switch, entry navigation and confirm.
 * @return Nothing: the original is declared int but returns no value, and its caller ignores it.
 */
s32 carda_handle_input(void)
{
    s32 entry_count;
    s32 input;
    s32 move_count;
    CardaElement* prompt;

    if (g_carda_element_pool[CARDA_ELEMENT_MAIN].attr.bits.state == CARDA_ELEMENT_FREE)
    {
        g_carda_exit_requested = 1;
        return;
    }
    if (g_carda_exit_requested != 0)
    {
        return;
    }
    if (g_carda_element_pool[CARDA_ELEMENT_MAIN].attr.bits.state >= CARDA_ELEMENT_CLOSING)
    {
        return;
    }
    if (g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state != CARDA_ELEMENT_FREE)
    {
        return;
    }
    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        return;
    }
    entry_count = g_carda_entry_state;
    if (entry_count == CARDA_ENTRY_STATE_CHECKING_CARD)
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
    if (*g_carda_card_step >= CARDA_STEP_SCAN_ENTRIES && *g_carda_card_step <= CARDA_STEP_SCAN_DONE)
    {
        return;
    }

    input = g_pad_input;
    if (input & PAD_BTN_CIRCLE)
    {
        g_field_card_overlay_mode = CARDA_RESULT_CANCELLED;
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
        carda_close_all_elements();
        return;
    }
    if (input & CARDA_CARD_SWITCH_BUTTON_MASK)
    {
        field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
        carda_switch_card();
        return;
    }
    if (entry_count >= CARDA_ENTRY_COUNT_INPUT_LIMIT)
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
            g_carda_selected_row--;
            if (g_carda_selected_row < 0)
            {
                g_carda_selected_row = g_carda_entry_state - 1;
            }
        }
        if (g_pad_input & PAD_BTN_DOWN)
        {
            g_carda_selected_row++;
            if (g_carda_selected_row >= g_carda_entry_state)
            {
                g_carda_selected_row = 0;
            }
        }
    }

    if (g_pad_input & (PAD_BTN_UP | PAD_BTN_DOWN))
    {
        carda_commit_selected_entry();
        field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
        carda_scroll_to_selection();
        return;
    }

    if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
    {
        if (g_carda_mode == CARDA_MODE_LOAD)
        {
            if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, CARDA_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                if (g_save_compatibility_tag == SAVE_TAG_ANY || g_carda_selected_file.saved_game.compatibility_tag == g_save_compatibility_tag)
                {
                    prompt = carda_alloc_element();
                    prompt->attr.bits.transition_step = 1;
                    prompt->attr.bits.x = CARDA_MESSAGE_X;
                    prompt->attr.bits.y = CARDA_MESSAGE_Y;
                    prompt->size.bits.width_high = CARDA_MESSAGE_WIDTH >> 8;
                    prompt->size.bits.height = CARDA_MESSAGE_HEIGHT;
                    CARDA_SET_ELEMENT_WIDTH_LOW(prompt, CARDA_MESSAGE_WIDTH);
                    carda_build_save_file();
                    carda_enable_choice_toggle();
                    prompt->draw = carda_draw_load_prompt;
                    carda_restart_card_sequence();
                    field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
                    return;
                }
            }
        }
        else
        {
            if (strncmp(g_new_save_entry_prefix, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, CARDA_NEW_SAVE_ENTRY_NAME_LENGTH) == 0)
            {
                prompt = carda_alloc_element();
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.x = CARDA_MESSAGE_X;
                prompt->attr.bits.y = CARDA_MESSAGE_Y;
                prompt->size.bits.width_high = CARDA_MESSAGE_WIDTH >> 8;
                prompt->size.bits.height = CARDA_MESSAGE_HEIGHT;
                CARDA_SET_ELEMENT_WIDTH_LOW(prompt, CARDA_MESSAGE_WIDTH);
                carda_build_save_file();
                carda_enable_choice_toggle();
                prompt->draw = carda_draw_save_prompt;
                carda_restart_card_sequence();
                field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
                return;
            }
            if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, CARDA_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                prompt = carda_alloc_element();
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.x = CARDA_MESSAGE_X;
                prompt->attr.bits.y = CARDA_MESSAGE_Y;
                prompt->size.bits.width_high = CARDA_MESSAGE_WIDTH >> 8;
                prompt->size.bits.height = CARDA_MESSAGE_HEIGHT;
                CARDA_SET_ELEMENT_WIDTH_LOW(prompt, CARDA_MESSAGE_WIDTH);
                carda_build_save_file();
                carda_enable_choice_toggle();
                prompt->draw = carda_draw_overwrite_prompt;
                carda_restart_card_sequence();
                field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
                return;
            }
        }
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
    }
}

/**
 * @brief Switch to the other card slot and restart its directory scan.
 */
void carda_switch_card(void)
{
    g_carda_format_declined = 0;
    g_carda_card_step = NULL;
    g_carda_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
    g_carda_scroll_frames = 0;
    g_carda_scroll_target_y = 0;
    g_carda_scroll_y = 0;
    g_carda_selected_row = 0;
    g_carda_selection_status = CARDA_SELECTION_NONE;
    g_carda_card_slot ^= 1;
    carda_reset_entry_ranks();
    carda_clear_hardware_card_events();
    carda_clear_software_card_events();
}

/**
 * @brief Restore the field fade target and start closing every open window.
 */
void carda_close_all_elements(void)
{
    CardaElement* element;
    s32 i;

    field_restore_fade_target();
    element = g_carda_element_pool;
    for (i = 0; i < CARDA_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.bits.state != CARDA_ELEMENT_FREE)
        {
            element->attr.bits.state = CARDA_ELEMENT_CLOSING;
            element->attr.bits.transition_step = CARDA_ELEMENT_TRANSITION_STEPS;
        }
    }
}

/**
 * @brief Retarget the list scroll so the selected row stays in the window.
 */
void carda_scroll_to_selection(void)
{
    s32 row_y;
    s32 relative_y;

    row_y = g_carda_selected_row * CARDA_ENTRY_ROW_HEIGHT;
    relative_y = row_y - g_carda_scroll_y;
    if (relative_y > CARDA_LIST_HEIGHT - CARDA_ENTRY_ROW_HEIGHT)
    {
        g_carda_scroll_target_y = row_y - CARDA_LIST_LAST_ROW_Y;
        g_carda_scroll_frames = CARDA_SCROLL_FRAMES;
    }
    if (relative_y < 0)
    {
        g_carda_scroll_target_y = g_carda_selected_row * CARDA_ENTRY_ROW_HEIGHT;
        g_carda_scroll_frames = CARDA_SCROLL_FRAMES;
    }
}

/**
 * @brief Run the window update and draw pass.
 * @param render FIELD render buffer being built this frame.
 */
void carda_update_elements(FieldRenderHalf* render)
{
    carda_update_and_draw_elements(render);
}

/**
 * @brief Draw the save-file browser: the message for a status entry state, or
 *        one row per directory entry and the selected-row highlight.
 * @param ot Ordering table the primitives are linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    /* A prompt or dialog covers the status messages, except the blank one. */
    if (g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state != CARDA_ELEMENT_FREE)
    {
        if (g_carda_entry_state >= CARDA_ENTRY_STATE_NO_GAME_DATA)
        {
            if (g_carda_entry_state < CARDA_ENTRY_STATE_BLANK)
            {
                return prim;
            }
            if (g_carda_entry_state == CARDA_ENTRY_STATE_CHECKING_CARD)
            {
                return prim;
            }
        }
    }

    switch (g_carda_entry_state)
    {
    case CARDA_ENTRY_STATE_NO_GAME_DATA:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_no_lom_save_data, CARDA_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_ENTRY_STATE_UNFORMATTED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_card_unformatted, CARDA_TEXT_CARD_UNFORMATTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_ENTRY_STATE_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_pocketstation, CARDA_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_ENTRY_STATE_CHECKING_CARD:
    {
        s32 message_x = -x_offset + CARDA_LIST_WIDTH / 2;
        u16* text_table = &g_carda_text_checking_card;

        prim =
            field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    case CARDA_ENTRY_STATE_CARD_FULL:
    {
        s32 message_x = -x_offset + CARDA_LIST_WIDTH / 2;
        u16* text_table;

        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_enough_blocks, CARDA_TEXT_NOT_ENOUGH_BLOCKS), FIELD_TEXT_COLOR_NORMAL, message_x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_not_enough_blocks, CARDA_TEXT_NOT_ENOUGH_BLOCKS);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_NEEDS_TWO_BLOCKS), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARDA_DETAILS_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    case CARDA_ENTRY_STATE_NO_ROOM_FOR_DOWNLOAD:
    {
        s32 message_x = -x_offset + CARDA_LIST_WIDTH / 2;
        u16* text_table;

        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_enough_blocks, CARDA_TEXT_NOT_ENOUGH_BLOCKS), FIELD_TEXT_COLOR_NORMAL, message_x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_not_enough_blocks, CARDA_TEXT_NOT_ENOUGH_BLOCKS);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_DOWNLOAD_RING_RING_LAND), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARDA_DETAILS_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_NEEDS_SIX_BLOCKS), FIELD_TEXT_COLOR_NORMAL, message_x,
                               CARDA_DETAILS_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    case CARDA_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_no_card, CARDA_TEXT_NO_CARD), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_LIST_WIDTH / 2,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_card_access_failed, CARDA_TEXT_CARD_ACCESS_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_ENTRY_STATE_NO_SAVE_DATA:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_no_save_data, CARDA_TEXT_NO_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_ENTRY_STATE_BLANK:
        break;
    default:
    {
        s32 row_y;
        s32 i;

        if (g_carda_entry_scan_active != 0)
        {
            s32 message_x = -x_offset + CARDA_LIST_WIDTH / 2;
            u16* text_table = &g_carda_text_checking_card;

            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                                   CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                                   CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
            break;
        }

        i = 0;
        if (i < g_carda_entry_state)
        {
            s32 list_x;
            u16 marker_offset;
            DVECTOR value_pos;
            u16* text_table;
            s32 marker_x;
            s32 marker_x_bits;
            s32 color;
            s32 label_x;

            text_table = &g_carda_text_checking_card;
            list_x = -x_offset;
            do
            {
                color = FIELD_TEXT_COLOR_NORMAL;
                marker_x = list_x + CARDA_ENTRY_MARKER_X;
                label_x = CARDA_ENTRY_LABEL_X - x_offset;
                row_y = ((i * CARDA_ENTRY_ROW_HEIGHT) - y_offset) - g_carda_scroll_y + 1;
                if (row_y > -CARDA_ENTRY_ROW_HEIGHT && row_y < CARDA_LIST_HEIGHT)
                {
                    if (g_carda_entry_ranks[i] >= 0)
                    {
                        value_pos.vx = list_x + CARDA_ENTRY_VALUE_X;
                        value_pos.vy = row_y;
                        prim = field_draw_number(ot, prim, g_carda_entry_suffix_values[i], color, &value_pos, FIELD_TEXT_ALIGN_LEFT);
                        prim = field_draw_text(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_number_label), color,
                                               list_x + CARDA_ENTRY_NUMBER_LABEL_X, row_y, FIELD_TEXT_ALIGN_LEFT);
                        value_pos.vy = row_y;
                        value_pos.vx = marker_x;
                        if ((g_carda_rank_count - 1) == g_carda_entry_ranks[i])
                        {
                            marker_offset = text_table[CARDA_TEXT_NEWEST];
                            marker_x_bits = marker_x << 16;
                            prim = field_draw_text(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, marker_offset), color, marker_x_bits >> 16, row_y,
                                                   FIELD_TEXT_ALIGN_LEFT);
                        }
                        else if (g_carda_entry_ranks[i] < 2)
                        {
                            marker_offset = text_table[CARDA_TEXT_OLDEST];
                            marker_x_bits = marker_x << 16;
                            prim = field_draw_text(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, marker_offset), color, marker_x_bits >> 16, row_y,
                                                   FIELD_TEXT_ALIGN_LEFT);
                        }
                        if (*skip_hex_digits(&g_carda_entries[g_carda_card_slot][i].name[CARDA_SAVE_FILENAME_PREFIX_LENGTH]) == '+')
                        {
                            prim = field_draw_text(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_plus_marker), color,
                                                   CARDA_ENTRY_PLUS_RIGHT_X - x_offset, row_y, FIELD_TEXT_ALIGN_RIGHT);
                        }
                    }
                    if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][i].name, CARDA_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim =
                            field_draw_text(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_mana_label), color, label_x, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_lom_pocketstation_filename_prefix, g_carda_entries[g_carda_card_slot][i].name, CARDA_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_ring_ring_land_label), color, label_x, row_y,
                                               FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_new_save_entry_prefix, g_carda_entries[g_carda_card_slot][i].name, CARDA_NEW_SAVE_ENTRY_NAME_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_new_save_label), color, label_x, row_y,
                                               FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_card_full_entry_name, g_carda_entries[g_carda_card_slot][i].name, CARDA_CARD_FULL_ENTRY_NAME_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_card_full_label), color, label_x, row_y,
                                               FIELD_TEXT_ALIGN_LEFT);
                    }
                    else
                    {
                        prim = field_draw_text(prim, ot, CARDA_TEXT_BY_OFFSET(text_table, g_carda_text_other_game_label), color, label_x, row_y,
                                               FIELD_TEXT_ALIGN_LEFT);
                    }
                }
                i++;
            } while (i < g_carda_entry_state);
        }

        row_y = ((g_carda_selected_row * CARDA_ENTRY_ROW_HEIGHT) - y_offset) - g_carda_scroll_y;
        if (g_carda_entry_scan_active == 0)
        {
            TILE* tile = (TILE*)prim;

            *(u32*)&tile->r0 = CARDA_HIGHLIGHT_COLOR;
            setlen(tile, 3);
            setcode(tile, GPU_CODE_TILE | GPU_CODE_SEMI_TRANS);
            tile->w = CARDA_LIST_WIDTH;
            setXY0(tile, 0, row_y);
            tile->h = CARDA_ENTRY_ROW_HEIGHT;
            addPrim(ot, tile);
            prim = tile + 1;
        }
        break;
    }
    }
    return prim;
}

/**
 * @brief Draw the title of the current mode: "Save", "Load" or Ring Ring Land.
 * @param ot Ordering table the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_title(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */

    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_ring_ring_land_title, CARDA_TEXT_RING_RING_LAND_TITLE), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_POCKETSTATION_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    else if (g_carda_mode == CARDA_MODE_LOAD)
    {
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_load_title, CARDA_TEXT_LOAD_TITLE), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    else
    {
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_save_title, CARDA_TEXT_SAVE_TITLE), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    }

    return prim;
}

/**
 * @brief Draw the slot 1 card label, dimmed while the other slot is selected.
 * @param ot Ordering table the primitives are linked into.
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
        *(u32*)&tile->r0 = CARDA_INACTIVE_LABEL_COLOR;
        setlen(tile, 3);
        setcode(tile, GPU_CODE_TILE | GPU_CODE_SEMI_TRANS);
        setXY0(tile, 0, 0);
        setWH(tile, CARDA_CARD_LABEL_WIDTH, CARDA_CARD_LABEL_HEIGHT);
        addPrim(ot, tile);
        prim = tile + 1;
    }
    return field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_card_slot0_label, CARDA_TEXT_CARD_SLOT0_LABEL), FIELD_TEXT_COLOR_NORMAL,
                           -x_offset + CARDA_CARD_LABEL_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Draw the slot 2 card label, dimmed while the other slot is selected.
 * @param ot Ordering table the primitives are linked into.
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
        *(u32*)&tile->r0 = CARDA_INACTIVE_LABEL_COLOR;
        setlen(tile, 3);
        setcode(tile, GPU_CODE_TILE | GPU_CODE_SEMI_TRANS);
        setXY0(tile, 0, 0);
        setWH(tile, CARDA_CARD_LABEL_WIDTH, CARDA_CARD_LABEL_HEIGHT);
        addPrim(ot, tile);
        prim = tile + 1;
    }
    return field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_card_slot1_label, CARDA_TEXT_CARD_SLOT1_LABEL), FIELD_TEXT_COLOR_NORMAL,
                           -x_offset + CARDA_CARD_LABEL_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Draw the details of the selected entry.
 *
 * Shows the new-save or full-card notice, the party icons, play time, hero
 * name and location of a Legend of Mana save (or the wrong-version message),
 * or the two-line card title of any other game's save.
 *
 * @param ot Ordering table the primitives are linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note JP changes this function; the JP build takes it from assembly.
 */
#if defined(VERSION_JP)
INCLUDE_ASM("overlays/carda/nonmatchings/carda", carda_draw_selected_entry_details);
#else
void* carda_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    void* result;
    DVECTOR pos;
    u8 name[0x100];
    s32 party_icon[FIELD_PARTY_SIZE];
    DVECTOR unused; /* never used, but the original stack frame reserves it */

    result = prim;
    if (g_carda_selection_status == CARDA_SELECTION_NONE)
    {
        return result;
    }
    if (g_carda_entry_scan_active != 0)
    {
        return result;
    }
    if (g_carda_selection_status != CARDA_SELECTION_EMPTY_CARD && g_carda_entry_state != CARDA_ENTRY_STATE_CARD_FULL &&
        g_carda_entry_state < CARDA_ENTRY_COUNT_LIMIT)
    {
        if (g_carda_selection_status == CARDA_SELECTION_NEW_SAVE)
        {
            s32 x = -x_offset;
            u16* text_table;

            result = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_new_save_title, CARDA_TEXT_NEW_SAVE_TITLE), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                     FIELD_TEXT_ALIGN_LEFT);
            text_table = CARDA_TEXT_TABLE(g_carda_text_new_save_title, CARDA_TEXT_NEW_SAVE_TITLE);
            return field_draw_text(result, ot, CARDA_TEXT(text_table, CARDA_TEXT_USES_TWO_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARDA_DETAILS_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_LEFT);
        }
        else if (g_carda_selection_status == CARDA_SELECTION_CARD_FULL)
        {
            return field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_needs_two_blocks, CARDA_TEXT_NEEDS_TWO_BLOCKS), FIELD_TEXT_COLOR_NORMAL, -x_offset,
                                   -y_offset, FIELD_TEXT_ALIGN_LEFT);
        }
        else
        {
            if (strncmp(g_lom_save_filename_prefix, g_carda_entries[g_carda_card_slot][g_carda_selected_row].name, CARDA_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                if (g_save_compatibility_tag == SAVE_TAG_ANY || g_carda_selected_file.saved_game.compatibility_tag == g_save_compatibility_tag ||
                    g_carda_selected_file.saved_game.compatibility_tag == SAVE_TAG_ANY)
                {
                    SavedGameLayout* save = &g_carda_selected_file.saved_game;
                    s32 present_count;
                    s32 i;
                    s32 j;
                    s32 step;
                    s32 half_step;
                    s32 base_x;
                    s32 base_y;
                    s32 total;
                    s32 hours;

                    total = 0;
                    party_icon[0] = save->spawn.bits.party_icon_0;
                    party_icon[1] = save->track.bits.party_icon_1;
                    party_icon[2] = save->track.bits.party_icon_2;
                    g_carda_icon_palette = save->icon_palette;

                    present_count = 0;
                    for (i = 0; i < FIELD_PARTY_SIZE; i++)
                    {
                        if (party_icon[i] != SAVE_NO_ICON)
                        {
                            present_count += 1;
                        }
                    }

                    switch (present_count)
                    {
                    case 2:
                        step = 32;
                        half_step = 16;
                        g_carda_icon_phase %= 32;
                        break;
                    case 3:
                        step = 16;
                        half_step = 32;
                        g_carda_icon_phase %= 96;
                        break;
                    default:
                        step = 16;
                        half_step = 32;
                        g_carda_icon_phase = 31;
                        break;
                    }

                    i = 0;
                    j = i;
                    for (; j < FIELD_PARTY_SIZE; j++)
                    {
                        base_y = i * half_step;
                        base_x = base_y + half_step;
                        if (party_icon[j] != SAVE_NO_ICON)
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
                        SavedGameLayout* shown_save = &g_carda_selected_file.saved_game;
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        base_y = shown_save->play_time;

                        pos.vx = x + CARDA_DETAILS_HOURS_RIGHT_X;
                        pos.vy = y;
                        hours = base_y / SAVED_PLAY_TIME_TICKS_PER_HOUR;
                        result = field_draw_number(ot, result, hours, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                        result = field_draw_text(result, ot, FIELD_UI_TEXT_AT(g_text_time_separator_offset_bytes, FIELD_UI_TEXT_TIME_SEPARATOR),
                                                 FIELD_TEXT_COLOR_NORMAL, x + CARDA_DETAILS_TIME_SEPARATOR_X, y, FIELD_TEXT_ALIGN_LEFT);
                        base_y = (base_y / SAVED_PLAY_TIME_TICKS_PER_MINUTE) - (hours * 60);
                        if (base_y < 10)
                        {
                            pos.vx = x + CARDA_DETAILS_MINUTES_TENS_RIGHT_X;
                            pos.vy = y;
                            result = field_draw_number(ot, result, 0, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                        }
                        pos.vx = x + CARDA_DETAILS_MINUTES_RIGHT_X;
                        pos.vy = y;
                        result = field_draw_number(ot, result, base_y, FIELD_TEXT_COLOR_NORMAL, &pos, FIELD_TEXT_ALIGN_RIGHT);
                        result = field_draw_text(result, ot, shown_save->summary_name, FIELD_TEXT_COLOR_NORMAL, x + CARDA_DETAILS_TEXT_X,
                                                 y + CARDA_DETAILS_LINE_HEIGHT, FIELD_TEXT_ALIGN_LEFT);
                        result = field_draw_text(result, ot, CARDA_TEXT(g_carda_location_names, shown_save->track.bits.music_track), FIELD_TEXT_COLOR_NORMAL,
                                                 x + CARDA_DETAILS_TEXT_X, y + CARDA_DETAILS_LINE_HEIGHT * 2, FIELD_TEXT_ALIGN_LEFT);
                    }
                }
                else
                {
                    result = field_draw_text(result, ot, CARDA_TEXT_AT(g_carda_text_wrong_version, CARDA_TEXT_WRONG_VERSION), FIELD_TEXT_COLOR_NORMAL,
                                             -x_offset, -y_offset, FIELD_TEXT_ALIGN_LEFT);
                }
            }
            else
            {
                s32 j;
                SaveFileHeader* header;

                terminate_multibyte_text(g_carda_selected_file.header.title);
                header = &g_carda_selected_file.header;
                if (header->title[1][0] == 0 || header->title[1][0] >= SAVE_FILE_TITLE_SJIS_LEAD_MIN)
                {
                    /*
                     * TODO: this single-iteration loop stands in for an unknown
                     * source shape (the same open lever as ADDHERO's details panel).
                     */
                    do
                    {
                        for (j = 0; j < SAVE_FILE_TITLE_LINE_BYTES; j++)
                        {
                            name[j] = *(header->title[0] + j);
                        }
                        name[j] = 0;
                        result = carda_draw_cached_text(result, ot, name, -x_offset, -y_offset, FIELD_TEXT_COLOR_NORMAL, FIELD_TEXT_ALIGN_LEFT);

                        for (j = 0; j < SAVE_FILE_TITLE_LINE_BYTES; j++)
                        {
                            name[j] = g_carda_selected_file.header.title[1][j];
                        }
                        name[j] = 0;
                        result = carda_draw_cached_text(result, ot, name, -x_offset, -y_offset + CARDA_DETAILS_LINE_HEIGHT, FIELD_TEXT_COLOR_NORMAL,
                                                        FIELD_TEXT_ALIGN_LEFT);
                    } while (0);
                }
            }
        }
    }
    return result;
}
#endif

#include "../common/terminate_multibyte_text.inc.c"

/**
 * @brief Draw FIELD's "Can't hold any more." notice, centred in a 256-pixel window.
 * @param ot Ordering table the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note Nothing in CARDA uses it.
 */
void* carda_draw_cant_hold_more(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */

    return field_draw_text(prim, ot, FIELD_UI_TEXT_AT(g_field_ui_text_cant_hold_more, FIELD_UI_TEXT_CANT_HOLD_MORE), FIELD_TEXT_COLOR_DIM,
                           CARDA_ITEM_LIST_WIDTH / 2 - x_offset, -y_offset, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Free every element of the pool and reset g_menu_element_counter to 0x20.
 */
void carda_clear_elements(void)
{
    CardaElement* p;
    s32 i;

    g_menu_element_counter = 0x20;
    p = g_carda_element_pool;
    for (i = 0; i < CARDA_ELEMENT_COUNT; i++)
    {
        p->attr.bits.state = CARDA_ELEMENT_FREE;
        p++;
    }
}

/**
 * @brief Claim the first free pool element and start its opening transition.
 * @return The claimed element, or the pool base element when none is free.
 */
CardaElement* carda_alloc_element(void)
{
    CardaElement* p;
    s32 i;

    p = g_carda_element_pool;
    for (i = 0; i < CARDA_ELEMENT_COUNT; i++, p++)
    {
        if (p->attr.bits.state == CARDA_ELEMENT_FREE)
        {
            p->attr.bits.state = CARDA_ELEMENT_OPENING;
            return p;
        }
    }
    return g_carda_element_pool;
}

/**
 * @brief Advance and draw the pool elements for one frame.
 *
 * Emits the list scroll arrows, then links a draw-environment packet for each
 * live element into the ordering table and animates it by state: an opening
 * window grows, an open window holds, a closing window shrinks, and a closed
 * window counts down to free.
 *
 * @param render FIELD render buffer; its primitive cursor is read on entry and written back on exit.
 */
void carda_update_and_draw_elements(FieldRenderHalf* render)
{
    void* prim;
    u_long* ot;
    CardaElement* element;
    s32 i;
    DRAWENV draw_env;
    s32 scaled_width;
    s32 scaled_height;

    prim = render->primitive_cursor;
    ot = render->ordering_table;

    if (g_carda_element_pool[CARDA_ELEMENT_MAIN].draw == carda_draw_entry_list)
    {
        if ((g_carda_entry_state < CARDA_ENTRY_COUNT_LIMIT) && (g_carda_element_pool[CARDA_ELEMENT_MAIN].attr.bits.state == CARDA_ELEMENT_OPEN))
        {
            if ((g_carda_entry_state * CARDA_ENTRY_ROW_HEIGHT) > (g_carda_scroll_y + CARDA_LIST_HEIGHT))
            {
                prim = field_draw_menu_scroll_arrow(prim, ot, CARDA_SCROLL_ARROW_X, CARDA_SCROLL_ARROW_DOWN_Y, FIELD_MENU_ARROW_DOWN);
            }
            if (g_carda_scroll_y != 0)
            {
                prim = field_draw_menu_scroll_arrow(prim, ot, CARDA_SCROLL_ARROW_X, CARDA_SCROLL_ARROW_UP_Y, FIELD_MENU_ARROW_UP);
            }
        }
    }
    else if ((g_carda_element_pool[CARDA_ELEMENT_MAIN].draw == carda_draw_item_list) &&
             (g_carda_element_pool[CARDA_ELEMENT_MAIN].attr.bits.state == CARDA_ELEMENT_OPEN))
    {
        if (((g_carda_received_item_count * CARDA_TEXT_LINE_HEIGHT) - g_carda_scroll_y) > CARDA_ITEM_LIST_VISIBLE_ROWS * CARDA_TEXT_LINE_HEIGHT)
        {
            prim = field_draw_menu_scroll_arrow(prim, ot, CARDA_ITEM_SCROLL_ARROW_X, CARDA_ITEM_SCROLL_ARROW_DOWN_Y, FIELD_MENU_ARROW_DOWN);
        }
        if (g_carda_scroll_y != 0)
        {
            prim = field_draw_menu_scroll_arrow(prim, ot, CARDA_ITEM_SCROLL_ARROW_X, CARDA_ITEM_SCROLL_ARROW_UP_Y, FIELD_MENU_ARROW_UP);
        }
    }

    if (render->display_rect.y != 0)
    {
        SetDefDrawEnv(&draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    }
    else
    {
        SetDefDrawEnv(&draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    }

    element = g_carda_element_pool;
    for (i = 0; i < CARDA_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.bits.state != CARDA_ELEMENT_FREE)
        {
            SetDrawEnv((DR_ENV*)prim, &draw_env);
            addPrim(ot, prim);
            prim = (DR_ENV*)prim + 1;

            switch (element->attr.bits.state)
            {
            case CARDA_ELEMENT_OPENING:
                g_pad_input = 0;
                {
                    u32 width_low = CARDA_ELEMENT_WIDTH_LOW(element);
                    s32 width = CARDA_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.bits.transition_step) / CARDA_ELEMENT_TRANSITION_STEPS;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / CARDA_ELEMENT_TRANSITION_STEPS;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = CARDA_ELEMENT_WIDTH_LOW(element);

                    prim = field_draw_menu_frame(prim, ot, x + (CARDA_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                 element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                                 render->display_rect.y, i == CARDA_ELEMENT_MODAL);
                }
                element->attr.bits.transition_step++;
                if (element->attr.bits.transition_step == CARDA_ELEMENT_TRANSITION_STEPS)
                {
                    field_reset_input_repeat();
                    element->attr.bits.state = CARDA_ELEMENT_OPEN;
                }
                break;

            case CARDA_ELEMENT_OPEN:
                prim = element->draw(ot, prim, 0, 0);
                {
                    u32 width_low = CARDA_ELEMENT_WIDTH_LOW(element);

                    prim = field_draw_menu_frame(prim, ot, element->attr.bits.x, element->attr.bits.y, CARDA_ELEMENT_WIDTH(element, width_low),
                                                 element->size.bits.height, render->display_rect.y, i == CARDA_ELEMENT_MODAL);
                }
                if (element->attr.bits.transition_step != 0)
                {
                    element->attr.bits.transition_step--;
                }
                break;

            case CARDA_ELEMENT_CLOSING:
                g_pad_input = 0;
                {
                    u32 width_low = CARDA_ELEMENT_WIDTH_LOW(element);
                    s32 width = CARDA_ELEMENT_WIDTH(element, width_low);

                    scaled_width = (width * element->attr.bits.transition_step) / CARDA_ELEMENT_TRANSITION_STEPS;
                    scaled_height = (element->size.bits.height * element->attr.bits.transition_step) / CARDA_ELEMENT_TRANSITION_STEPS;
                    prim = element->draw(ot, prim, (width - scaled_width) / 2, (element->size.bits.height - scaled_height) / 2);
                }
                {
                    s32 x = element->attr.bits.x;
                    u32 width_low = CARDA_ELEMENT_WIDTH_LOW(element);

                    prim = field_draw_menu_frame(prim, ot, x + (CARDA_ELEMENT_WIDTH(element, width_low) - scaled_width) / 2,
                                                 element->attr.bits.y + (element->size.bits.height - scaled_height) / 2, scaled_width, scaled_height,
                                                 render->display_rect.y, i == CARDA_ELEMENT_MODAL);
                }
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.transition_step = CARDA_ELEMENT_CLOSED_FRAMES;
                    element->attr.bits.state = CARDA_ELEMENT_CLOSED;
                }
                break;

            case CARDA_ELEMENT_CLOSED:
                g_pad_input = 0;
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.state = CARDA_ELEMENT_FREE;
                }
                break;
            }
        }
    }

    render->primitive_cursor = prim;
}

/**
 * @brief Free the modal window's pool element.
 */
void carda_deactivate_primary_element(void)
{
    g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
}

#include "../common/encoded_text_append.inc.c"
#include "../common/encoded_text_byte_length.inc.c"
#include "../common/encoded_text_copy.inc.c"
