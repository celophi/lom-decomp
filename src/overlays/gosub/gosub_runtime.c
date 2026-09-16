#include "gosub_internal.h"

/**
 * @brief Process input, advance scroll interpolation, and draw the active screen.
 *
 * @param render_ctx Rendering context forwarded to the input and draw handlers.
 * @see decomp.me (100%)
 */
void gosub_update_screen(s32 render_ctx)
{
    gosub_handle_input(render_ctx);

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

        gosub_render_elements(render_ctx);
    }
}

/**
 * @brief Handle dialog, navigation, selection, completion, and cancellation input.
 *
 * @param unused Unused rendering context.
 * @return Undefined; callers ignore the value.
 * @see decomp.me (100%)
 */
s32 gosub_handle_input(s32 unused)
{
    GosubElement* elements;
    s32 steps_remaining;
    s32 restored_row;
    s32 max_scroll_y;

    elements = g_gosub_elements;
    if ((elements[1].attr.word & 7) == 0 && (elements[0].attr.word & 7) == 0)
    {
        g_gosub_finished = 1;
        return;
    }

    if (gosub_are_elements_idle() == 0)
    {
        return;
    }

    if ((g_gosub_elements[0].attr.word & 7) == 2)
    {
        if (g_gosub_dialog_accepting_input != 0)
        {
            if ((g_pad_input & 0x260) == 0)
            {
                return;
            }
            if (g_gosub_suppress_dialog_sound == 0)
            {
                func_800A3938(0x7D, 0x80);
                func_80067F28();
                gosub_start_element_exit();
                return;
            }
            g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
            g_gosub_dialog_accepting_input = 0;
            return;
        }

        if (g_pad_input & 0x9000)
        {
            func_800A3938(0x7D, 0x80);
            g_gosub_dialog_choice -= 1;
            if (g_gosub_dialog_choice < 0)
            {
                g_gosub_dialog_choice = 0xB;
            }
            return;
        }

        if (g_pad_input & 0x6000)
        {
            func_800A3938(0x7D, 0x80);
            g_gosub_dialog_choice += 1;
            if (g_gosub_dialog_choice == 0xC)
            {
                g_gosub_dialog_choice = 0;
            }
            return;
        }

        if (g_pad_input & 0x220)
        {
            func_800A3938(0x7D, 0x80);
            if (g_gosub_dialog_handler == 0)
            {
                return;
            }
            if (g_gosub_dialog_handler(0) == 0)
            {
                return;
            }
            func_80067F28();
            gosub_start_element_exit();
            return;
        }

        if (g_pad_input & 0x40)
        {
            func_800A3938(0x7D, 0x80);
            if (g_gosub_dialog_handler == 0)
            {
                return;
            }
            if (g_gosub_dialog_handler(1) == 0)
            {
                return;
            }
            func_80067F28();
            gosub_start_element_exit();
            return;
        }

        return;
    }

    if (g_gosub_scroll_frames_remaining != 0)
    {
        return;
    }

    steps_remaining = 1;
    if (g_pad_input & 8)
    {
        steps_remaining = g_gosub_visible_row_count;
        g_pad_input = 0x4000;
    }
    if (g_pad_input & 4)
    {
        steps_remaining = g_gosub_visible_row_count;
        g_pad_input = 0x1000;
    }

    if (steps_remaining != 0)
    {
        do
        {
            if (g_pad_input & 0x1000)
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
            if (g_pad_input & 0x4000)
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

    if (g_pad_input & 0x5000)
    {
        func_800A3938(0x7D, 0x80);
        gosub_scroll_to_cursor();
        return;
    }

    if (g_pad_input & 0x220)
    {
        func_800A3938(0x7D, 0x80);
        if ((g_gosub_rows[g_gosub_cursor_row].text_color & 0xF) != 4)
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
                func_80067F28();
                gosub_start_element_exit();
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
            func_80067F28();
            gosub_start_element_exit();
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
        func_80067F28();
        gosub_start_element_exit();
        return;
    }

    if (g_pad_input & 0x800)
    {
        func_800A3938(0x7D, 0x80);
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
            func_80067F28();
            gosub_start_element_exit();
            return;
        }
        if (gosub_advance_screen_sequence() == 0)
        {
            return;
        }
        func_80067F28();
        gosub_start_element_exit();
        return;
    }

    if ((g_pad_input & 0x40) == 0)
    {
        return;
    }

    func_800A3938(0x7F, 0x80);

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
    func_80067F28();
    gosub_start_element_exit();
}

/**
 * @brief Scroll the list viewport toward the cursor when it leaves view.
 *
 * @see decomp.me (100%)
 */
void gosub_scroll_to_cursor(void)
{
    s32 cursor_offset;
    s32 cursor_y;

    cursor_y = g_gosub_cursor_row * g_gosub_row_height;
    cursor_offset = cursor_y - g_gosub_scroll_y;

    if ((g_gosub_window_height - g_gosub_row_height) < cursor_offset)
    {
        g_gosub_scroll_frames_remaining = 4;
        g_gosub_scroll_target_y = (g_gosub_cursor_row - (g_gosub_visible_row_count - 1)) * g_gosub_row_height;
    }

    if (cursor_offset < 0)
    {
        g_gosub_scroll_target_y = cursor_y;
        g_gosub_scroll_frames_remaining = 4;
    }
}

/**
 * @brief Remove the cursor row if selected, or permit the caller to add it.
 *
 * @return 0 if the row was removed, otherwise 1.
 * @see decomp.me (100%)
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
 * @see decomp.me (100%)
 */
s32 gosub_advance_screen_sequence(void)
{
    GosubElement* element;

    g_gosub_screen_sequence_index += 1;

    if (g_gosub_screen_sequence[g_gosub_screen_sequence_index] == GOSUB_SCREEN_SEQUENCE_END)
    {
        return 1;
    }

    if (g_gosub_screen_sequence[g_gosub_screen_sequence_index] == GOSUB_SCREEN_SEQUENCE_DIALOG)
    {
        element = &g_gosub_elements[0];
        element->draw_handler = (void*)&gosub_draw_confirmation_prompt;
        g_gosub_dialog_choice = 0;
        element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
        element->attr.f.transition_step = 1;
        element->attr.f.x = 0x20;
        element->attr.f.width_low = 0x70;
        element->width_high = 1;
        element->y = 0x24;
        SET_ELEMENT_WIDTH_LOW(element, 0);
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
 * @see decomp.me (100%)
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
