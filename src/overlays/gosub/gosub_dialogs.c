#include "gosub_internal.h"

/**
 * @brief Dialog handler for the selected row's action prompt.
 *
 * A nonzero @p dialog_result cancels: the selection count is dropped and the
 * dialog element is deactivated. On confirm, bit 0 of g_gosub_dialog_choice
 * picks the path. When it is clear the work is handed to gosub_open_sort_dialog. When
 * it is set the first selected row decides: a row with flag2 set is rejected
 * with message 0x22 (the selection is dropped and g_gosub_suppress_dialog_sound is raised),
 * otherwise a follow-up dialog is opened with gosub_handle_delete_dialog as its handler.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return Always 0.
 * @see decomp.me
 */
s32 gosub_handle_row_action_dialog(s32 dialog_result)
{
    GosubElement* element;

    if (dialog_result != 0)
    {
        g_gosub_selection_count = 0;
        g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
        return 0;
    }

    if (g_gosub_dialog_choice & 1)
    {
        if (g_gosub_rows[g_gosub_selected_rows[0]].flags.f.alternate_format)
        {
            GOSUB_MSG(0x22);
            g_gosub_selection_count = 0;
            g_gosub_suppress_dialog_sound = 1;
            return 0;
        }
        element = &g_gosub_elements[0];
        element->draw_handler = (void*)&gosub_draw_confirmation_prompt;
        g_gosub_dialog_handler = gosub_handle_delete_dialog;
        g_gosub_dialog_choice = 0;
        element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
        element->attr.f.transition_step = 1;
        element->attr.f.x = 0x20;
        element->attr.f.y = 0x70;
        element->geometry.f.width_high = 1;
        element->geometry.f.height = 0x24;
        SET_ELEMENT_WIDTH_LOW(element, 0);
    }
    else
    {
        gosub_open_sort_dialog();
    }
    return 0;
}

/**
 * @brief Dialog handler that drops the cursor row and re-clamps the list.
 *
 * Always deactivates the dialog element. A nonzero @p dialog_result, or bit 0
 * of g_gosub_dialog_choice, cancels: the selection count is set to 1 and
 * nothing else changes. On confirm the cursor row is handed to gosub_delete_packed_record
 * (by entry index) and to gosub_delete_list_row (by row), the selection is cleared,
 * and the viewport is re-clamped -- the cursor is pulled back to the last row
 * when it now sits past the end, and the scroll target is clamped to the
 * bottom of the shortened list over a 4-frame scroll.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return 1 when no rows remain, otherwise 0.
 * @see decomp.me
 */
s32 gosub_handle_delete_dialog(s32 dialog_result)
{
    s32 scroll_y;
    s32 max_scroll;

    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
    if (dialog_result == 0 && (g_gosub_dialog_choice & 1) == 0)
    {
        gosub_delete_packed_record(g_gosub_rows[g_gosub_cursor_row].index);
        gosub_delete_list_row(g_gosub_cursor_row);
        g_gosub_selection_count = 0;
        if (g_gosub_row_count == 0)
        {
            return 1;
        }
        if (g_gosub_cursor_row >= g_gosub_row_count)
        {
            g_gosub_cursor_row = g_gosub_row_count - 1;
        }
        scroll_y = g_gosub_scroll_y;
        max_scroll = ((g_gosub_row_count * g_gosub_row_height) - g_gosub_window_height) + 4;
        if (max_scroll < scroll_y)
        {
            scroll_y = max_scroll;
        }
        if (scroll_y < 0)
        {
            scroll_y = 0;
        }
        g_gosub_scroll_target_y = scroll_y;
        g_gosub_scroll_frames_remaining = 4;
        return 0;
    }
    g_gosub_selection_count = 1;
    return 0;
}

/**
 * @brief Dialog handler that backs the screen sequence out one step.
 *
 * Does nothing and reports 1 when the dialog was confirmed with bit 0 of
 * g_gosub_dialog_choice clear. Otherwise it releases one selection slot (only
 * when the selection is already full), steps the screen sequence back, runs
 * g_gosub_select_handler when one is installed, drops the nesting depth in
 * g_gosub_result_count, and puts element 0 into its exit animation.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return 1 when the confirm path is taken, otherwise 0.
 *
 * @see decomp.me
 */
s32 gosub_handle_backtrack_dialog(s32 dialog_result)
{
    if (dialog_result == 0 && (g_gosub_dialog_choice & 1) == 0)
    {
        return 1;
    }

    if (g_gosub_required_selection_count == g_gosub_selection_count)
    {
        g_gosub_selection_count -= 1;
    }

    g_gosub_screen_sequence_index -= 1;
    if (g_gosub_select_handler != 0)
    {
        g_gosub_select_handler();
        g_gosub_result_count -= 1;
    }
    else
    {
        g_gosub_result_count -= 1;
    }

    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_EXITING;
    g_gosub_elements[0].attr.f.transition_step = 8;
    return 0;
}

/**
 * @brief Apply the selected logic-block sort and toggle its direction.
 *
 * A confirmed sort uses the selected type, power, or shape key and the current
 * direction. The direction then flips for the next sort. Cancelling preserves
 * the rows and restores one pending selection.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return Always 0.
 * @see decomp.me
 */
s32 gosub_handle_sort_dialog(s32 dialog_result)
{
    if (dialog_result == 0)
    {
        gosub_sort_rows((g_gosub_sort_ascending << GOSUB_SORT_ASCENDING_SHIFT) + (g_gosub_dialog_choice % GOSUB_SORT_KEY_COUNT));
        g_gosub_selection_count = 0;
        g_gosub_sort_ascending ^= 1;
    }
    else
    {
        g_gosub_selection_count = 1;
    }

    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
    return 0;
}

/**
 * @brief Open the wide confirmation dialog and hand it to gosub_handle_row_action_dialog.
 *
 * Installs gosub_draw_two_option_dialog as element 0's draw handler and gosub_handle_row_action_dialog as the
 * dialog's result handler, clears the pending choice, then starts the element
 * entering at x 0x80 / y 0x24 with code 0x80. func_800AA02C runs last.
 *
 * @see decomp.me
 */
void gosub_open_row_action_dialog(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_two_option_dialog;
    g_gosub_dialog_choice = 0;
    g_gosub_dialog_handler = gosub_handle_row_action_dialog;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x80;
    element->attr.f.y = 0x70;
    element->geometry.f.width_high = 0;
    element->geometry.f.height = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 0x80);
    func_800AA02C();
}

/**
 * @brief Open the three-option row sorting dialog.
 * @see decomp.me
 */
void gosub_open_sort_dialog(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_three_option_dialog;
    g_gosub_dialog_handler = gosub_handle_sort_dialog;
    g_gosub_dialog_choice = 0;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x80;
    element->attr.f.y = 0x70;
    element->geometry.f.width_high = 0;
    element->geometry.f.height = 0x34;
    SET_ELEMENT_WIDTH_LOW(element, 0x80);
    func_800AA02C();
}

/**
 * @brief Draw the sort and delete row actions and highlight the selected one.
 * @param ordering_table Ordering table to receive the text packets.
 * @param initial_packet First free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after both actions.
 * @see decomp.me
 */
s32 gosub_draw_two_option_dialog(s32* ordering_table, s32 initial_packet, s32 x_offset, s32 y_offset)
{
    s32* archive_offset_word;
    s32 archive_base;
    void* sort_text;
    void* delete_text;
    s32 delete_color;
    s32 sort_color;
    s32 packet_cursor;
    s32 stack_padding[14];

    packet_cursor = initial_packet;
    archive_offset_word = &g_gosub_message_archive_offset;
    archive_base = (s32)archive_offset_word - 0x20;

    sort_text = GOSUB_MSG_ABS(archive_base, GOSUB_ROW_ACTION_SORT_MESSAGE_OFFSET);
    sort_color = GOSUB_TEXT_COLOR_DISABLED;
    if ((g_gosub_dialog_choice & GOSUB_ROW_ACTION_CHOICE_MASK) == 0)
    {
        sort_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = func_800A88A0(packet_cursor, ordering_table, sort_text, sort_color, GOSUB_ROW_ACTION_DIALOG_X - x_offset,
                                  GOSUB_ROW_ACTION_SORT_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);

    delete_text = GOSUB_MSG_ABS(archive_base, GOSUB_ROW_ACTION_DELETE_MESSAGE_OFFSET);
    delete_color = GOSUB_TEXT_COLOR_DISABLED;
    if ((g_gosub_dialog_choice & GOSUB_ROW_ACTION_CHOICE_MASK) != 0)
    {
        delete_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = func_800A88A0(packet_cursor, ordering_table, delete_text, delete_color, GOSUB_ROW_ACTION_DIALOG_X - x_offset,
                                  GOSUB_ROW_ACTION_DELETE_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);

    return packet_cursor;
}

/**
 * @brief Draw the three sort-key choices and highlight the selected key.
 * @param ordering_table Ordering table to receive the text packets.
 * @param initial_packet First free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after all three choices.
 * @see decomp.me
 */
s32 gosub_draw_three_option_dialog(s32* ordering_table, s32 initial_packet, s32 x_offset, s32 y_offset)
{
    s32* archive_offset_word;
    s32 archive_base;
    void* type_text;
    void* power_text;
    void* shape_text;
    s32 text_color;
    s32 type_color;
    s32 packet_cursor;
    s32 selected_sort_key;
    s32 stack_padding[14];

    packet_cursor = initial_packet;
    archive_offset_word = &g_gosub_message_archive_offset;
    archive_base = (s32)archive_offset_word - 0x20;

    type_text = GOSUB_MSG_ABS(archive_base, GOSUB_SORT_DIALOG_TYPE_MESSAGE_OFFSET);
    selected_sort_key = g_gosub_dialog_choice;
    selected_sort_key %= GOSUB_SORT_KEY_COUNT;
    type_color = GOSUB_TEXT_COLOR_DISABLED;
    if (selected_sort_key == GOSUB_SORT_BY_TYPE)
    {
        type_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = func_800A88A0(packet_cursor, ordering_table, type_text, type_color, GOSUB_SORT_DIALOG_X - x_offset, GOSUB_SORT_DIALOG_TYPE_Y - y_offset,
                                  GOSUB_TEXT_ALIGN_CENTER);

    power_text = GOSUB_MSG_ABS(archive_base, GOSUB_SORT_DIALOG_POWER_MESSAGE_OFFSET);
    text_color = GOSUB_TEXT_COLOR_DISABLED;
    if (g_gosub_dialog_choice % GOSUB_SORT_KEY_COUNT == GOSUB_SORT_BY_POWER)
    {
        text_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = func_800A88A0(packet_cursor, ordering_table, power_text, text_color, GOSUB_SORT_DIALOG_X - x_offset, GOSUB_SORT_DIALOG_POWER_Y - y_offset,
                                  GOSUB_TEXT_ALIGN_CENTER);

    shape_text = GOSUB_MSG_ABS(archive_base, GOSUB_SORT_DIALOG_SHAPE_MESSAGE_OFFSET);
    text_color = GOSUB_TEXT_COLOR_DISABLED;
    if (g_gosub_dialog_choice % GOSUB_SORT_KEY_COUNT == GOSUB_SORT_BY_SHAPE)
    {
        text_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = func_800A88A0(packet_cursor, ordering_table, shape_text, text_color, GOSUB_SORT_DIALOG_X - x_offset, GOSUB_SORT_DIALOG_SHAPE_Y - y_offset,
                                  GOSUB_TEXT_ALIGN_CENTER);

    return packet_cursor;
}

/**
 * @brief Open a modal dialog containing caller-provided text.
 * @param message_text Pointer to the encoded dialog text.
 * @see decomp.me
 */
void gosub_open_message_dialog(u8* message_text)
{
    GosubElement* element;

    g_gosub_dialog_text = message_text;
    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_message_dialog;
    g_gosub_dialog_choice = 0;
    g_gosub_dialog_accepting_input = 1;
    g_gosub_suppress_dialog_sound = 0;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x20;
    element->attr.f.y = 0x70;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 0);
    func_800AA02C();
    g_gosub_result_count = 0;
}

/**
 * @brief Draw the text stored by gosub_open_message_dialog.
 * @param ordering_table Ordering table to receive the text packets.
 * @param packet_cursor Next free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after the dialog text.
 * @see decomp.me
 */
s32 gosub_draw_message_dialog(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset)
{
    s32 stack_padding[14];

    packet_cursor = func_800A88A0(packet_cursor, ordering_table, g_gosub_dialog_text, GOSUB_TEXT_COLOR_NORMAL, GOSUB_MESSAGE_DIALOG_TEXT_X - x_offset,
                                  GOSUB_MESSAGE_DIALOG_TEXT_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);
    return packet_cursor;
}

/**
 * @brief Draw a fixed header and the current equipment row's details.
 * @param ordering_table Ordering table to receive the text packets.
 * @param packet_cursor Next free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after the header and optional details.
 * @see decomp.me
 */
s32 gosub_draw_detail_header(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset)
{
    s32* archive_offset_word;
    s32 archive_base;
    void* text;
    s32 stack_padding[14];

    archive_offset_word = &g_gosub_message_archive_offset;
    archive_base = (s32)archive_offset_word - 0x20;

    text = GOSUB_MSG_ABS(archive_base, GOSUB_DETAIL_HEADER_MESSAGE_OFFSET);
    packet_cursor = func_800A88A0(packet_cursor, ordering_table, text, GOSUB_TEXT_COLOR_NORMAL, GOSUB_DETAIL_HEADER_X - x_offset,
                                  GOSUB_DETAIL_HEADER_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);
    if (g_gosub_show_row_details != 0)
    {
        packet_cursor = gosub_draw_equipment_details(packet_cursor, ordering_table, x_offset, y_offset);
    }
    return packet_cursor;
}

/**
 * @brief Draw the two fixed header lines used by the grouped selection screen.
 * @param ordering_table Ordering table to receive the text packets.
 * @param packet_cursor Next free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after both lines.
 * @see decomp.me
 */
s32 gosub_draw_two_line_header(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset)
{
    s32* archive_offset_word;
    s32 archive_base;
    void* text;
    s32 stack_padding[14];

    archive_offset_word = &g_gosub_message_archive_offset;
    archive_base = (s32)archive_offset_word - 0x20;

    text = GOSUB_MSG_ABS(archive_base, GOSUB_TWO_LINE_HEADER_FIRST_MESSAGE_OFFSET);
    packet_cursor = func_800A88A0(packet_cursor, ordering_table, text, GOSUB_TEXT_COLOR_NORMAL, GOSUB_TWO_LINE_HEADER_X - x_offset,
                                  GOSUB_TWO_LINE_HEADER_FIRST_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);

    text = GOSUB_MSG_ABS(archive_base, GOSUB_TWO_LINE_HEADER_SECOND_MESSAGE_OFFSET);
    packet_cursor = func_800A88A0(packet_cursor, ordering_table, text, GOSUB_TEXT_COLOR_NORMAL, GOSUB_TWO_LINE_HEADER_X - x_offset,
                                  GOSUB_TWO_LINE_HEADER_SECOND_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);

    return packet_cursor;
}

/**
 * @brief Draw a confirmation title and two side-by-side choices.
 * @param ordering_table Ordering table to receive the text packets.
 * @param packet_cursor Next free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after the title and both choices.
 * @see decomp.me
 */
s32 gosub_draw_confirmation_prompt(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset)
{
    s32* archive_offset_word;
    s32 archive_base;
    void* text;
    s32 text_color;
    s32 stack_padding[14];

    archive_offset_word = &g_gosub_message_archive_offset;
    archive_base = (s32)archive_offset_word - 0x20;

    text = GOSUB_MSG_ABS(archive_base, GOSUB_CONFIRMATION_TITLE_MESSAGE_OFFSET);
    packet_cursor = func_800A88A0(packet_cursor, ordering_table, text, GOSUB_TEXT_COLOR_NORMAL, GOSUB_CONFIRMATION_TITLE_X - x_offset,
                                  GOSUB_CONFIRMATION_TITLE_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);

    text = GOSUB_MSG_ABS(archive_base, GOSUB_CONFIRMATION_FIRST_CHOICE_MESSAGE_OFFSET);
    text_color = GOSUB_TEXT_COLOR_DISABLED;
    if ((g_gosub_dialog_choice & GOSUB_CONFIRMATION_CHOICE_MASK) == 0)
    {
        text_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = func_800A88A0(packet_cursor, ordering_table, text, text_color, GOSUB_CONFIRMATION_FIRST_CHOICE_X - x_offset,
                                  GOSUB_CONFIRMATION_CHOICE_Y - y_offset, GOSUB_TEXT_ALIGN_RIGHT);

    text = GOSUB_MSG_ABS(archive_base, GOSUB_CONFIRMATION_SECOND_CHOICE_MESSAGE_OFFSET);
    text_color = GOSUB_TEXT_COLOR_NORMAL;
    if ((g_gosub_dialog_choice & GOSUB_CONFIRMATION_CHOICE_MASK) == 0)
    {
        text_color = GOSUB_TEXT_COLOR_DISABLED;
    }
    packet_cursor = func_800A88A0(packet_cursor, ordering_table, text, text_color, GOSUB_CONFIRMATION_SECOND_CHOICE_X - x_offset,
                                  GOSUB_CONFIRMATION_CHOICE_Y - y_offset, GOSUB_TEXT_ALIGN_LEFT);

    return packet_cursor;
}

/**
 * @brief Draw the current row description and its optional equipment details.
 * @param ordering_table Ordering table to receive the text packets.
 * @param packet_cursor Next free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after the description and optional details.
 * @see decomp.me
 */
s32 gosub_draw_row_description(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset)
{
    s32 stack_padding[12];

    packet_cursor = func_800A88A0(packet_cursor, ordering_table, g_gosub_rows[g_gosub_cursor_row].desc, GOSUB_TEXT_COLOR_NORMAL,
                                  GOSUB_ROW_DESCRIPTION_X - x_offset, GOSUB_ROW_DESCRIPTION_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);
    if (g_gosub_show_row_details != 0)
    {
        packet_cursor = gosub_draw_equipment_details(packet_cursor, ordering_table, x_offset, y_offset);
    }
    return packet_cursor;
}

/**
 * @brief Draw power, defense, or instrument details for the current equipment row.
 * @param packet_cursor Next free GPU packet.
 * @param ordering_table Ordering table to receive the text packets.
 * @param x_offset Horizontal dialog animation offset.
 * @param y_offset Vertical dialog animation offset.
 * @return Packet cursor after the equipment detail line.
 * @see decomp.me
 */
s32 gosub_draw_equipment_details(s32 packet_cursor, s32* ordering_table, s32 x_offset, s32 y_offset)
{
    s32 equipment_kind;
    GosubTextPosition number_position;
    u8* archive_base;
    s32* archive_block;
    s32 effect_text_offset;

    equipment_kind = g_gosub_rows[g_gosub_cursor_row].equipment_kind;

    switch (equipment_kind)
    {
    case GOSUB_EQUIPMENT_KIND_WEAPON:
        packet_cursor = func_800A88A0(packet_cursor, ordering_table, (void*)((u8*)&D_800EC3EE - 0x2A + D_800EC3EE.low + (D_800EC3EE.high << 8)),
                                      GOSUB_TEXT_COLOR_NORMAL, GOSUB_EQUIPMENT_DETAIL_LABEL_X - x_offset, GOSUB_EQUIPMENT_DETAIL_Y - y_offset, 0);
        number_position.x = GOSUB_WEAPON_POWER_X - x_offset;
        number_position.y = (s16)(GOSUB_EQUIPMENT_DETAIL_Y - y_offset);
        packet_cursor = func_800A8A78(ordering_table, packet_cursor, g_gosub_rows[g_gosub_cursor_row].primary_value, GOSUB_TEXT_COLOR_NORMAL, &number_position,
                                      GOSUB_TEXT_ALIGN_LEFT);
        break;

    case GOSUB_EQUIPMENT_KIND_ARMOR:
        packet_cursor = func_800A88A0(packet_cursor, ordering_table, (void*)((u8*)&D_800EC3F0 - 0x2C + D_800EC3F0.low + (D_800EC3F0.high << 8)),
                                      GOSUB_TEXT_COLOR_NORMAL, GOSUB_EQUIPMENT_DETAIL_LABEL_X - x_offset, GOSUB_EQUIPMENT_DETAIL_Y - y_offset, 0);
        number_position.x = GOSUB_ARMOR_DEFENSE_X - x_offset;
        number_position.y = (s16)(GOSUB_EQUIPMENT_DETAIL_Y - y_offset);
        packet_cursor = func_800A8A78(ordering_table, packet_cursor,
                                      g_gosub_rows[g_gosub_cursor_row].stats[0] + g_gosub_rows[g_gosub_cursor_row].stats[1] +
                                          g_gosub_rows[g_gosub_cursor_row].stats[2] + g_gosub_rows[g_gosub_cursor_row].stats[3],
                                      GOSUB_TEXT_COLOR_NORMAL, &number_position, GOSUB_TEXT_ALIGN_LEFT);
        break;

    /* Instruments and the reserved fourth kind use the same detail layout. */
    default:
        archive_block = &g_gosub_text_archive_offsets_6;
        packet_cursor = func_800A88A0(packet_cursor, ordering_table, (void*)((u8*)&D_800EC3F2 - 0x2E + D_800EC3F2.low + (D_800EC3F2.high << 8)),
                                      GOSUB_TEXT_COLOR_NORMAL, GOSUB_EQUIPMENT_DETAIL_LABEL_X - x_offset, GOSUB_EQUIPMENT_DETAIL_Y - y_offset, 0);
        number_position.x = GOSUB_INSTRUMENT_POWER_X - x_offset;
        number_position.y = (s16)(GOSUB_EQUIPMENT_DETAIL_Y - y_offset);
        packet_cursor = func_800A8A78(ordering_table, packet_cursor, g_gosub_rows[g_gosub_cursor_row].primary_value, GOSUB_TEXT_COLOR_NORMAL, &number_position,
                                      GOSUB_TEXT_ALIGN_LEFT);
        archive_base = (u8*)archive_block;
        archive_base -= 0x2C;
        effect_text_offset = (s32)archive_base + *(u16*)(g_gosub_rows[g_gosub_cursor_row].stats[0] * 2 + g_gosub_text_archive_offsets_6 + archive_base);
        packet_cursor = func_800A88A0(packet_cursor, ordering_table, (void*)(g_gosub_text_archive_offsets_6 + effect_text_offset), GOSUB_TEXT_COLOR_NORMAL,
                                      GOSUB_INSTRUMENT_EFFECT_X - x_offset, GOSUB_EQUIPMENT_DETAIL_Y - y_offset, GOSUB_TEXT_ALIGN_LEFT);
        break;
    }
    return packet_cursor;
}

/**
 * @brief Draw the current gosub screen title.
 * @param ot Ordering-table tag to link into.
 * @param prim Packet cursor.
 * @param x_off Horizontal dialog animation offset.
 * @param y_off Vertical dialog animation offset.
 * @return Packet cursor after the title.
 * @see decomp.me
 */
s32 gosub_draw_title(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32 stack_pad[14];

    prim = func_800A88A0(prim, ot, g_gosub_title_text, 4, 0x84 - x_off, 2 - y_off, 2);
    return prim;
}

/**
 * @brief Append one encoded string to another.
 * @param dst Null-terminated destination buffer.
 * @param src Null-terminated source string.
 * @see decomp.me
 */
void gosub_append_encoded_string(u8* dst, u8* src)
{
    s32 len1;
    s32 len2;
    s32 i;

    len1 = gosub_encoded_string_length(dst);
    len2 = gosub_encoded_string_length(src);

    for (i = 0; i < len2; i++)
    {
        dst[len1 + i] = src[i];
    }

    dst[len1 + i] = 0;
}

/**
 * @brief Count bytes in a null-terminated encoded string.
 * @param text Encoded string to measure.
 * @return Byte count excluding the terminator.
 * @see decomp.me
 */
s32 gosub_encoded_string_length(const u8* text)
{
    const u8* scan_cursor;
    s32 byte_count;

    scan_cursor = text;
    byte_count = 0;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += 2;
            byte_count += 2;
        }
        else
        {
            scan_cursor += 1;
            byte_count += 1;
        }
    }

    return byte_count;
}

/**
 * @brief Copy a null-terminated encoded string.
 * @param dst Destination buffer.
 * @param src Source string.
 * @see decomp.me
 */
void gosub_copy_encoded_string(u8* dst, u8* src)
{
    const u8* scan_cursor;
    s32 byte_count;
    s32 i;

    scan_cursor = src;
    byte_count = 0;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += 2;
            byte_count += 2;
        }
        else
        {
            scan_cursor += 1;
            byte_count += 1;
        }
    }

    for (i = 0; i < byte_count; i++)
    {
        dst[i] = src[i];
    }

    dst[i] = 0;
}
