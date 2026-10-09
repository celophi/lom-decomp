#include "internal/gosub_internal.h"
#include "main/audio/akao.h"

/**
 * @brief Process input, advance scroll interpolation, and draw the active screen.
 *
 * @param render_context Rendering context forwarded to the draw handler.
 */
void gosub_update_screen(GosubRenderContext* render_context)
{
    gosub_handle_input();

    if (g_gosub_finished == 0)
    {
        if (g_gosub_scroll_frames_remaining != 0)
        {
            g_gosub_scroll_y += (g_gosub_scroll_target_y - g_gosub_scroll_y) / g_gosub_scroll_frames_remaining;
            g_gosub_scroll_frames_remaining -= 1;
        }
        else
        {
            g_gosub_scroll_y = g_gosub_scroll_target_y;
        }

        gosub_render_elements(render_context);
    }
}

/**
 * @brief Handle dialog, navigation, selection, completion, and cancellation input.
 * @return Undefined; callers ignore the value.
 */
s32 gosub_handle_input(void)
{
    s32 steps_remaining;
    s32 restored_row;
    s32 max_scroll_y;

    if (g_gosub_elements[1].attr.f.state == GOSUB_ELEMENT_STATE_INACTIVE && g_gosub_elements[0].attr.f.state == GOSUB_ELEMENT_STATE_INACTIVE)
    {
        g_gosub_finished = 1;
        return;
    }

    if (gosub_are_elements_idle() == 0)
    {
        return;
    }

    if (g_gosub_elements[0].attr.f.state == GOSUB_ELEMENT_STATE_ACTIVE)
    {
        if (g_gosub_dialog_accepting_input != 0)
        {
            if ((g_pad_input & GOSUB_BUTTONS_DISMISS) == 0)
            {
                return;
            }
            if (g_gosub_suppress_dialog_sound == 0)
            {
                play_menu_sfx(GOSUB_SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
                field_restore_fade_target();
                gosub_close_elements();
                return;
            }
            g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
            g_gosub_dialog_accepting_input = 0;
            return;
        }

        if (g_pad_input & GOSUB_BUTTONS_PREVIOUS)
        {
            play_menu_sfx(GOSUB_SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
            g_gosub_dialog_choice -= 1;
            if (g_gosub_dialog_choice < 0)
            {
                g_gosub_dialog_choice = GOSUB_DIALOG_CHOICE_COUNT - 1;
            }
            return;
        }

        if (g_pad_input & GOSUB_BUTTONS_NEXT)
        {
            play_menu_sfx(GOSUB_SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
            g_gosub_dialog_choice += 1;
            if (g_gosub_dialog_choice == GOSUB_DIALOG_CHOICE_COUNT)
            {
                g_gosub_dialog_choice = 0;
            }
            return;
        }

        if (g_pad_input & GOSUB_BUTTONS_CONFIRM)
        {
            play_menu_sfx(GOSUB_SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
            if (g_gosub_dialog_handler == 0)
            {
                return;
            }
            if (g_gosub_dialog_handler(0) == 0)
            {
                return;
            }
            field_restore_fade_target();
            gosub_close_elements();
            return;
        }

        if (g_pad_input & GOSUB_BUTTON_CANCEL)
        {
            play_menu_sfx(GOSUB_SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
            if (g_gosub_dialog_handler == 0)
            {
                return;
            }
            if (g_gosub_dialog_handler(1) == 0)
            {
                return;
            }
            field_restore_fade_target();
            gosub_close_elements();
            return;
        }

        return;
    }

    if (g_gosub_scroll_frames_remaining != 0)
    {
        return;
    }

    steps_remaining = 1;
    if (g_pad_input & GOSUB_BUTTON_PAGE_DOWN)
    {
        steps_remaining = g_gosub_visible_row_count;
        g_pad_input = PAD_BTN_DOWN;
    }
    if (g_pad_input & GOSUB_BUTTON_PAGE_UP)
    {
        steps_remaining = g_gosub_visible_row_count;
        g_pad_input = PAD_BTN_UP;
    }

    if (steps_remaining != 0)
    {
        do
        {
            if (g_pad_input & PAD_BTN_UP)
            {
                g_gosub_cursor_row -= 1;
                if (g_gosub_cursor_row == 0)
                {
                    steps_remaining = 1;
                }
                if (g_gosub_cursor_row < 0)
                {
                    g_gosub_cursor_row = g_gosub_row_count - 1;
                    steps_remaining = 1;
                }
            }
            if (g_pad_input & PAD_BTN_DOWN)
            {
                g_gosub_cursor_row += 1;
                if (g_gosub_cursor_row == g_gosub_row_count - 1)
                {
                    steps_remaining = 1;
                }
                if (g_gosub_cursor_row >= g_gosub_row_count)
                {
                    g_gosub_cursor_row = 0;
                    steps_remaining = 1;
                }
            }
            steps_remaining -= 1;
        } while (steps_remaining != 0);
    }

    if (g_pad_input & (PAD_BTN_UP | PAD_BTN_DOWN))
    {
        play_menu_sfx(GOSUB_SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
        gosub_scroll_to_cursor();
        return;
    }

    if (g_pad_input & GOSUB_BUTTONS_CONFIRM)
    {
        play_menu_sfx(GOSUB_SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
        if (g_gosub_rows[g_gosub_cursor_row].text_color != GOSUB_TEXT_COLOR_NORMAL)
        {
            return;
        }
        if (gosub_toggle_cursor_selection() != 0)
        {
            g_gosub_selected_rows[g_gosub_selection_count] = g_gosub_cursor_row;
            g_gosub_selection_count += 1;
            if (g_gosub_select_handler != 0)
            {
                if (g_gosub_select_handler() == 0)
                {
                    return;
                }
                if (gosub_advance_screen_sequence() == 0)
                {
                    return;
                }
                field_restore_fade_target();
                gosub_close_elements();
                return;
            }
            if (g_gosub_selection_count != g_gosub_required_selection_count)
            {
                return;
            }
            if (g_gosub_finish_handler == 0)
            {
                return;
            }
            if (g_gosub_finish_handler() == 0)
            {
                return;
            }
            if (gosub_advance_screen_sequence() == 0)
            {
                return;
            }
            field_restore_fade_target();
            gosub_close_elements();
            return;
        }
        if (g_gosub_select_handler == 0)
        {
            return;
        }
        if (g_gosub_select_handler() == 0)
        {
            return;
        }
        if (gosub_advance_screen_sequence() == 0)
        {
            return;
        }
        field_restore_fade_target();
        gosub_close_elements();
        return;
    }

    if (g_pad_input & GOSUB_BUTTON_FINISH)
    {
        play_menu_sfx(GOSUB_SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
        if (g_gosub_finish_handler != 0)
        {
            if (g_gosub_finish_handler() == 0)
            {
                return;
            }
            if (gosub_advance_screen_sequence() == 0)
            {
                return;
            }
            field_restore_fade_target();
            gosub_close_elements();
            return;
        }
        if (gosub_advance_screen_sequence() == 0)
        {
            return;
        }
        field_restore_fade_target();
        gosub_close_elements();
        return;
    }

    if ((g_pad_input & GOSUB_BUTTON_CANCEL) == 0)
    {
        return;
    }

    play_menu_sfx(GOSUB_SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);

    if (g_gosub_selection_count != 0)
    {
        g_gosub_selection_count -= 1;
        g_gosub_cursor_row = g_gosub_selected_rows[g_gosub_selection_count];
        gosub_scroll_to_cursor();
        if (g_gosub_select_handler != 0)
        {
            g_gosub_select_handler();
        }
        return;
    }

    if (g_gosub_screen_sequence_index != 0)
    {
        g_gosub_screen_sequence_index -= 1;
        gosub_enter_screen(g_gosub_screen_sequence[g_gosub_screen_sequence_index]);
        g_gosub_result_count -= 1;
        restored_row = g_gosub_result_rows[g_gosub_result_count];
        g_gosub_cursor_row = restored_row;
        g_gosub_scroll_y = g_gosub_row_height * restored_row;
        max_scroll_y = (g_gosub_row_count * g_gosub_row_height) - g_gosub_window_height + 4;
        if (max_scroll_y < g_gosub_scroll_y)
        {
            g_gosub_scroll_y = max_scroll_y;
        }
        if (g_gosub_scroll_y < 0)
        {
            g_gosub_scroll_y = 0;
        }
        g_gosub_scroll_frames_remaining = 0;
        g_gosub_scroll_target_y = g_gosub_scroll_y;
        return;
    }

    g_gosub_result_count = 0;
    field_restore_fade_target();
    gosub_close_elements();
}

/**
 * @brief Scroll the list viewport toward the cursor when it leaves view.
 *
 */
void gosub_scroll_to_cursor(void)
{
    s32 cursor_offset;
    s32 cursor_y;

    cursor_y = g_gosub_cursor_row * g_gosub_row_height;
    cursor_offset = cursor_y - g_gosub_scroll_y;

    if ((g_gosub_window_height - g_gosub_row_height) < cursor_offset)
    {
        g_gosub_scroll_frames_remaining = GOSUB_SCROLL_FRAMES;
        g_gosub_scroll_target_y = (g_gosub_cursor_row - (g_gosub_visible_row_count - 1)) * g_gosub_row_height;
    }

    if (cursor_offset < 0)
    {
        g_gosub_scroll_target_y = cursor_y;
        g_gosub_scroll_frames_remaining = GOSUB_SCROLL_FRAMES;
    }
}

/**
 * @brief Remove the cursor row if selected, or permit the caller to add it.
 *
 * @return 0 if the row was removed, otherwise 1.
 */
s32 gosub_toggle_cursor_selection(void)
{
    s32 selection_index;
    s32 shift_index;

    if (g_gosub_allow_duplicate_selection != 0)
    {
        return 1;
    }

    for (selection_index = 0; selection_index < g_gosub_selection_count; selection_index++)
    {
        if (g_gosub_selected_rows[selection_index] == g_gosub_cursor_row)
        {
            for (shift_index = selection_index; shift_index < 3; shift_index++)
            {
                g_gosub_selected_rows[shift_index] = g_gosub_selected_rows[shift_index + 1];
            }
            g_gosub_selection_count -= 1;
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Advance to the next screen or open the sequence's final dialog.
 *
 * @return 1 at the sequence terminator, otherwise 0.
 */
s32 gosub_advance_screen_sequence(void)
{
    g_gosub_screen_sequence_index += 1;

    if (g_gosub_screen_sequence[g_gosub_screen_sequence_index] == GOSUB_SCREEN_SEQUENCE_END)
    {
        return 1;
    }

    if (g_gosub_screen_sequence[g_gosub_screen_sequence_index] == GOSUB_SCREEN_SEQUENCE_DIALOG)
    {
        gosub_open_confirmation_dialog();
    }
    else
    {
        gosub_enter_screen(g_gosub_screen_sequence[g_gosub_screen_sequence_index]);
    }

    return 0;
}

/**
 * @brief Test whether all fixed elements have finished transitioning.
 *
 * @return 1 when all elements are idle, otherwise 0.
 */
s32 gosub_are_elements_idle(void)
{
    GosubElement* element;
    s32 i;

    element = g_gosub_elements;
    for (i = 0; i < GOSUB_ELEMENT_COUNT; i++)
    {
        if (element->attr.f.state == GOSUB_ELEMENT_STATE_ENTERING || element->attr.f.state == GOSUB_ELEMENT_STATE_EXITING)
        {
            return 0;
        }
        element++;
    }

    return 1;
}
