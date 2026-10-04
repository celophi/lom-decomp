#include "internal/gosub_internal.h"

/**
 * @brief Handle the Sort/Discard dialog of a logic block.
 *
 * Sort opens the sort-key dialog. Discard asks for confirmation, unless the
 * block is in use.
 *
 * @param dialog_result Zero to confirm the highlighted choice; nonzero to cancel.
 * @return Always 0.
 */
s32 gosub_handle_block_action_dialog(s32 dialog_result)
{
    if (dialog_result != 0)
    {
        g_gosub_selection_count = 0;
        g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
        return 0;
    }

    if (g_gosub_dialog_choice & GOSUB_ROW_ACTION_CHOICE_MASK)
    {
        if (g_gosub_rows[g_gosub_selected_rows[0]].flags.block.in_use)
        {
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_IN_USE_CANNOT_DISCARD));
            g_gosub_selection_count = 0;
            g_gosub_suppress_dialog_sound = 1;
            return 0;
        }
        gosub_open_confirmation_dialog();
        g_gosub_dialog_handler = gosub_handle_discard_dialog;
    }
    else
    {
        gosub_open_sort_dialog();
    }
    return 0;
}

/**
 * @brief Handle the confirmation of discarding the logic block under the cursor.
 *
 * On Yes the block and its row are deleted, and the cursor and scroll
 * position are pulled back inside the shortened list.
 *
 * @param dialog_result Zero to confirm the highlighted choice; nonzero to cancel.
 * @return 1 when no blocks remain, otherwise 0.
 */
s32 gosub_handle_discard_dialog(s32 dialog_result)
{
    s32 scroll_y;
    s32 max_scroll;

    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
    if (dialog_result == 0 && (g_gosub_dialog_choice & GOSUB_CONFIRMATION_CHOICE_MASK) == 0)
    {
        gosub_delete_logic_block(g_gosub_rows[g_gosub_cursor_row].index);
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
        max_scroll = ((g_gosub_row_count * g_gosub_row_height) - g_gosub_window_height) + GOSUB_LIST_PANEL_PADDING;
        if (max_scroll < scroll_y)
        {
            scroll_y = max_scroll;
        }
        if (scroll_y < 0)
        {
            scroll_y = 0;
        }
        g_gosub_scroll_target_y = scroll_y;
        g_gosub_scroll_frames_remaining = GOSUB_SCROLL_FRAMES;
        return 0;
    }
    g_gosub_selection_count = 1;
    return 0;
}

/**
 * @brief Handle the final confirmation of a screen sequence.
 *
 * Yes finishes the sequence. No steps back to the last screen and drops its
 * queued result.
 *
 * @param dialog_result Zero to confirm the highlighted choice; nonzero to cancel.
 * @return 1 when the sequence is confirmed, otherwise 0.
 */
s32 gosub_handle_backtrack_dialog(s32 dialog_result)
{
    if (dialog_result == 0 && (g_gosub_dialog_choice & GOSUB_CONFIRMATION_CHOICE_MASK) == 0)
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
 */
s32 gosub_handle_sort_dialog(s32 dialog_result)
{
    if (dialog_result == 0)
    {
        gosub_sort_logic_blocks((g_gosub_sort_ascending << GOSUB_SORT_ASCENDING_SHIFT) + (g_gosub_dialog_choice % GOSUB_SORT_KEY_COUNT));
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
 * @brief Open the Sort/Discard dialog for the selected logic block.
 */
void gosub_open_block_action_dialog(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_block_action_dialog;
    g_gosub_dialog_choice = 0;
    g_gosub_dialog_handler = gosub_handle_block_action_dialog;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_CHOICE_PANEL_X;
    element->attr.f.y = GOSUB_DIALOG_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_CHOICE_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_TWO_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_CHOICE_PANEL_WIDTH);
    field_reset_input_repeat();
}

/**
 * @brief Open the dialog that picks the logic-block sort key.
 */
void gosub_open_sort_dialog(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_sort_dialog;
    g_gosub_dialog_handler = gosub_handle_sort_dialog;
    g_gosub_dialog_choice = 0;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_CHOICE_PANEL_X;
    element->attr.f.y = GOSUB_DIALOG_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_CHOICE_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_THREE_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_CHOICE_PANEL_WIDTH);
    field_reset_input_repeat();
}

/**
 * @brief Draw the Sort and Discard choices and highlight the selected one.
 * @param ordering_table Ordering table to receive the text packets.
 * @param initial_packet First free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after both actions.
 */
u8* gosub_draw_block_action_dialog(s32* ordering_table, u8* initial_packet, s32 x_offset, s32 y_offset)
{
    void* sort_text;
    void* delete_text;
    s32 delete_color;
    s32 sort_color;
    u8* packet_cursor;
    s32 unused[14];

    packet_cursor = initial_packet;
    sort_text = GOSUB_MESSAGE(GOSUB_MSG_SORT);
    sort_color = GOSUB_TEXT_COLOR_DISABLED;
    if ((g_gosub_dialog_choice & GOSUB_ROW_ACTION_CHOICE_MASK) == 0)
    {
        sort_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = field_draw_text(packet_cursor, ordering_table, sort_text, sort_color, GOSUB_ROW_ACTION_DIALOG_X - x_offset,
                                    GOSUB_ROW_ACTION_SORT_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);

    delete_text = GOSUB_MESSAGE(GOSUB_MSG_DISCARD);
    delete_color = GOSUB_TEXT_COLOR_DISABLED;
    if ((g_gosub_dialog_choice & GOSUB_ROW_ACTION_CHOICE_MASK) != 0)
    {
        delete_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = field_draw_text(packet_cursor, ordering_table, delete_text, delete_color, GOSUB_ROW_ACTION_DIALOG_X - x_offset,
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
 */
u8* gosub_draw_sort_dialog(s32* ordering_table, u8* initial_packet, s32 x_offset, s32 y_offset)
{
    void* type_text;
    void* power_text;
    void* shape_text;
    s32 text_color;
    s32 type_color;
    u8* packet_cursor;
    s32 selected_sort_key;
    s32 unused[14];

    packet_cursor = initial_packet;
    type_text = GOSUB_MESSAGE(GOSUB_MSG_SORT_BY_TYPE);
    selected_sort_key = g_gosub_dialog_choice;
    selected_sort_key %= GOSUB_SORT_KEY_COUNT;
    type_color = GOSUB_TEXT_COLOR_DISABLED;
    if (selected_sort_key == GOSUB_SORT_BY_TYPE)
    {
        type_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = field_draw_text(packet_cursor, ordering_table, type_text, type_color, GOSUB_SORT_DIALOG_X - x_offset, GOSUB_SORT_DIALOG_TYPE_Y - y_offset,
                                    GOSUB_TEXT_ALIGN_CENTER);

    power_text = GOSUB_MESSAGE(GOSUB_MSG_SORT_BY_POWER);
    text_color = GOSUB_TEXT_COLOR_DISABLED;
    if (g_gosub_dialog_choice % GOSUB_SORT_KEY_COUNT == GOSUB_SORT_BY_POWER)
    {
        text_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = field_draw_text(packet_cursor, ordering_table, power_text, text_color, GOSUB_SORT_DIALOG_X - x_offset, GOSUB_SORT_DIALOG_POWER_Y - y_offset,
                                    GOSUB_TEXT_ALIGN_CENTER);

    shape_text = GOSUB_MESSAGE(GOSUB_MSG_SORT_BY_SHAPE);
    text_color = GOSUB_TEXT_COLOR_DISABLED;
    if (g_gosub_dialog_choice % GOSUB_SORT_KEY_COUNT == GOSUB_SORT_BY_SHAPE)
    {
        text_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = field_draw_text(packet_cursor, ordering_table, shape_text, text_color, GOSUB_SORT_DIALOG_X - x_offset, GOSUB_SORT_DIALOG_SHAPE_Y - y_offset,
                                    GOSUB_TEXT_ALIGN_CENTER);

    return packet_cursor;
}

/**
 * @brief Open a modal dialog containing caller-provided text.
 * @param message_text Pointer to the encoded dialog text.
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
    element->attr.f.x = GOSUB_MESSAGE_PANEL_X;
    element->attr.f.y = GOSUB_DIALOG_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_MESSAGE_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_ONE_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_MESSAGE_PANEL_WIDTH);
    field_reset_input_repeat();
    g_gosub_result_count = 0;
}

/**
 * @brief Draw the text stored by gosub_open_message_dialog.
 * @param ordering_table Ordering table to receive the text packets.
 * @param packet_cursor Next free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after the dialog text.
 */
u8* gosub_draw_message_dialog(s32* ordering_table, u8* packet_cursor, s32 x_offset, s32 y_offset)
{
    s32 unused[14];

    packet_cursor = field_draw_text(packet_cursor, ordering_table, g_gosub_dialog_text, GOSUB_TEXT_COLOR_NORMAL, GOSUB_MESSAGE_DIALOG_TEXT_X - x_offset,
                                    GOSUB_MESSAGE_DIALOG_TEXT_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);
    return packet_cursor;
}

/**
 * @brief Draw the logic-block component instructions and the current row's details.
 * @param ordering_table Ordering table to receive the text packets.
 * @param packet_cursor Next free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after the header and optional details.
 */
u8* gosub_draw_block_components_header(s32* ordering_table, u8* packet_cursor, s32 x_offset, s32 y_offset)
{
    void* text;
    s32 unused[14];

    text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_BLOCK_COMPONENTS);
    packet_cursor = field_draw_text(packet_cursor, ordering_table, text, GOSUB_TEXT_COLOR_NORMAL, GOSUB_DETAIL_HEADER_X - x_offset,
                                    GOSUB_DETAIL_HEADER_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);
    if (g_gosub_show_row_details != 0)
    {
        packet_cursor = gosub_draw_equipment_details(packet_cursor, ordering_table, x_offset, y_offset);
    }
    return packet_cursor;
}

/**
 * @brief Draw the two lines of golem parts instructions.
 * @param ordering_table Ordering table to receive the text packets.
 * @param packet_cursor Next free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after both lines.
 */
u8* gosub_draw_golem_parts_header(s32* ordering_table, u8* packet_cursor, s32 x_offset, s32 y_offset)
{
    void* text;
    s32 unused[14];

    text = GOSUB_MESSAGE(GOSUB_MSG_SELECT_GOLEM_PARTS);
    packet_cursor = field_draw_text(packet_cursor, ordering_table, text, GOSUB_TEXT_COLOR_NORMAL, GOSUB_TWO_LINE_HEADER_X - x_offset,
                                    GOSUB_TWO_LINE_HEADER_FIRST_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);

    text = GOSUB_MESSAGE(GOSUB_MSG_PRESS_START_TO_SELECT);
    packet_cursor = field_draw_text(packet_cursor, ordering_table, text, GOSUB_TEXT_COLOR_NORMAL, GOSUB_TWO_LINE_HEADER_X - x_offset,
                                    GOSUB_TWO_LINE_HEADER_SECOND_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);

    return packet_cursor;
}

/**
 * @brief Draw the "Is this okay?" title and the Yes/No choices.
 * @param ordering_table Ordering table to receive the text packets.
 * @param packet_cursor Next free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after the title and both choices.
 */
u8* gosub_draw_confirmation_prompt(s32* ordering_table, u8* packet_cursor, s32 x_offset, s32 y_offset)
{
    void* text;
    s32 text_color;
    s32 unused[14];

    text = GOSUB_MESSAGE(GOSUB_MSG_IS_THIS_OKAY);
    packet_cursor = field_draw_text(packet_cursor, ordering_table, text, GOSUB_TEXT_COLOR_NORMAL, GOSUB_CONFIRMATION_TITLE_X - x_offset,
                                    GOSUB_CONFIRMATION_TITLE_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);

    text = GOSUB_MESSAGE(GOSUB_MSG_YES);
    text_color = GOSUB_TEXT_COLOR_DISABLED;
    if ((g_gosub_dialog_choice & GOSUB_CONFIRMATION_CHOICE_MASK) == 0)
    {
        text_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    packet_cursor = field_draw_text(packet_cursor, ordering_table, text, text_color, GOSUB_CONFIRMATION_FIRST_CHOICE_X - x_offset,
                                    GOSUB_CONFIRMATION_CHOICE_Y - y_offset, GOSUB_TEXT_ALIGN_RIGHT);

    text = GOSUB_MESSAGE(GOSUB_MSG_NO);
    text_color = GOSUB_TEXT_COLOR_NORMAL;
    if ((g_gosub_dialog_choice & GOSUB_CONFIRMATION_CHOICE_MASK) == 0)
    {
        text_color = GOSUB_TEXT_COLOR_DISABLED;
    }
    packet_cursor = field_draw_text(packet_cursor, ordering_table, text, text_color, GOSUB_CONFIRMATION_SECOND_CHOICE_X - x_offset,
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
 */
u8* gosub_draw_row_description(s32* ordering_table, u8* packet_cursor, s32 x_offset, s32 y_offset)
{
    s32 unused[12];

    packet_cursor = field_draw_text(packet_cursor, ordering_table, g_gosub_rows[g_gosub_cursor_row].desc, GOSUB_TEXT_COLOR_NORMAL,
                                    GOSUB_ROW_DESCRIPTION_X - x_offset, GOSUB_ROW_DESCRIPTION_Y - y_offset, GOSUB_TEXT_ALIGN_CENTER);
    if (g_gosub_show_row_details != 0)
    {
        packet_cursor = gosub_draw_equipment_details(packet_cursor, ordering_table, x_offset, y_offset);
    }
    return packet_cursor;
}

/**
 * @brief Draw the attack power, total defense, or instrument power and spell of the current row.
 * @param packet_cursor Next free GPU packet.
 * @param ordering_table Ordering table to receive the text packets.
 * @param x_offset Horizontal dialog animation offset.
 * @param y_offset Vertical dialog animation offset.
 * @return Packet cursor after the equipment detail line.
 */
u8* gosub_draw_equipment_details(u8* packet_cursor, s32* ordering_table, s32 x_offset, s32 y_offset)
{
    s32 equipment_kind;
    GosubTextPosition number_position;

    equipment_kind = g_gosub_rows[g_gosub_cursor_row].equipment_kind;

    switch (equipment_kind)
    {
    case GOSUB_EQUIPMENT_KIND_WEAPON:
        packet_cursor = field_draw_text(packet_cursor, ordering_table, FIELD_UI_TEXT_AT(D_800EC3EE, FIELD_UI_TEXT_ATTACK_POWER), GOSUB_TEXT_COLOR_NORMAL,
                                        GOSUB_EQUIPMENT_DETAIL_LABEL_X - x_offset, GOSUB_EQUIPMENT_DETAIL_Y - y_offset, 0);
        number_position.x = GOSUB_WEAPON_POWER_X - x_offset;
        number_position.y = (s16)(GOSUB_EQUIPMENT_DETAIL_Y - y_offset);
        packet_cursor = field_draw_number(ordering_table, packet_cursor, g_gosub_rows[g_gosub_cursor_row].primary_value, GOSUB_TEXT_COLOR_NORMAL,
                                          &number_position, GOSUB_TEXT_ALIGN_LEFT);
        break;

    case GOSUB_EQUIPMENT_KIND_ARMOR:
        packet_cursor = field_draw_text(packet_cursor, ordering_table, FIELD_UI_TEXT_AT(D_800EC3F0, FIELD_UI_TEXT_TOTAL_DEFENSE), GOSUB_TEXT_COLOR_NORMAL,
                                        GOSUB_EQUIPMENT_DETAIL_LABEL_X - x_offset, GOSUB_EQUIPMENT_DETAIL_Y - y_offset, 0);
        number_position.x = GOSUB_ARMOR_DEFENSE_X - x_offset;
        number_position.y = (s16)(GOSUB_EQUIPMENT_DETAIL_Y - y_offset);
        packet_cursor = field_draw_number(ordering_table, packet_cursor,
                                          g_gosub_rows[g_gosub_cursor_row].stats[0] + g_gosub_rows[g_gosub_cursor_row].stats[1] +
                                              g_gosub_rows[g_gosub_cursor_row].stats[2] + g_gosub_rows[g_gosub_cursor_row].stats[3],
                                          GOSUB_TEXT_COLOR_NORMAL, &number_position, GOSUB_TEXT_ALIGN_LEFT);
        break;

    /* Instruments and the reserved fourth kind use the same detail layout. */
    default:
        packet_cursor = field_draw_text(packet_cursor, ordering_table, FIELD_UI_TEXT_AT(D_800EC3F2, FIELD_UI_TEXT_POWER), GOSUB_TEXT_COLOR_NORMAL,
                                        GOSUB_EQUIPMENT_DETAIL_LABEL_X - x_offset, GOSUB_EQUIPMENT_DETAIL_Y - y_offset, 0);
        number_position.x = GOSUB_INSTRUMENT_POWER_X - x_offset;
        number_position.y = (s16)(GOSUB_EQUIPMENT_DETAIL_Y - y_offset);
        packet_cursor = field_draw_number(ordering_table, packet_cursor, g_gosub_rows[g_gosub_cursor_row].primary_value, GOSUB_TEXT_COLOR_NORMAL,
                                          &number_position, GOSUB_TEXT_ALIGN_LEFT);
        packet_cursor =
            field_draw_text(packet_cursor, ordering_table, GOSUB_TEXT(GOSUB_TEXT_INSTRUMENT_SPELLS, g_gosub_rows[g_gosub_cursor_row].stats[0]),
                            GOSUB_TEXT_COLOR_NORMAL, GOSUB_INSTRUMENT_EFFECT_X - x_offset, GOSUB_EQUIPMENT_DETAIL_Y - y_offset, GOSUB_TEXT_ALIGN_LEFT);
        break;
    }
    return packet_cursor;
}

/**
 * @brief Draw the current screen's title.
 * @param ordering_table Ordering table to receive the text packets.
 * @param packet_cursor Next free GPU packet.
 * @param x_offset Horizontal element animation offset.
 * @param y_offset Vertical element animation offset.
 * @return Packet cursor after the title.
 */
u8* gosub_draw_title(s32* ordering_table, u8* packet_cursor, s32 x_offset, s32 y_offset)
{
    s32 unused[14];

    packet_cursor = field_draw_text(packet_cursor, ordering_table, g_gosub_title_text, GOSUB_TEXT_COLOR_NORMAL, GOSUB_TEXT_PANEL_WIDTH / 2 - x_offset,
                                    2 - y_offset, GOSUB_TEXT_ALIGN_CENTER);
    return packet_cursor;
}

#include "../../common/encoded_text/encoded_text_append.inc.c"
#include "../../common/encoded_text/encoded_text_byte_length.inc.c"
#include "../../common/encoded_text/encoded_text_copy.inc.c"
