#include "gosub_internal.h"

/**
 * @brief Confirm the currently highlighted row of the gosub list.
 * @return 0 if the row was rejected by a flag, 1 otherwise. Note that 1 is also
 *         returned when g_gosub_selection_count is clear and nothing was appended.
 * @see decomp.me (100%)
 */
s32 gosub_select_row_with_validation(void)
{
    s32 row;
    GosubListRow* list;
    GosubListRow* row_entry;

    list = g_gosub_rows;
    row = g_gosub_cursor_row;
    row_entry = &list[row];
    if (row_entry->flags.half & 1)
    {
        GOSUB_MSG(0x42);
        g_gosub_selection_count = 0;
        g_gosub_suppress_dialog_sound = 1;
        return 0;
    }
    if (row_entry->flags.f.selection_restricted)
    {
        GOSUB_MSG(0x50);
        g_gosub_selection_count = 0;
        g_gosub_suppress_dialog_sound = 1;
        return 0;
    }
    if (g_gosub_selection_count != 0)
    {
        g_gosub_result_values[g_gosub_result_count] = row_entry->index;
        g_gosub_result_rows[g_gosub_result_count] = row;
        g_gosub_result_count++;
    }
    return 1;
}

/**
 * @brief Append the highlighted row to the selection, with no flag checks.
 * @return Always 1. Nothing is appended while g_gosub_selection_count is clear.
 * @see decomp.me (100%)
 */
s32 gosub_select_row(void)
{
    s32 n;

    if (g_gosub_selection_count != 0)
    {
        n = g_gosub_result_count;
        g_gosub_result_count = n + 1;
        g_gosub_result_values[n] = g_gosub_rows[g_gosub_cursor_row].index;
        g_gosub_result_rows[n] = g_gosub_cursor_row;
    }
    return 1;
}

/**
 * @brief Validate a pending two-row selection before publishing it.
 * @return gosub_publish_two_row_selection's result while g_gosub_combination_result_id is set, 0 on every other path.
 * @see decomp.me (100%)
 */
s32 gosub_validate_pending_pair_selection(void)
{
    if (g_gosub_selection_count != 0)
    {
        if (g_gosub_combination_result_id == 0)
        {
            if (g_gosub_selection_count == 2)
            {
                g_gosub_selection_count = 1;
            }
            return 0;
        }
        return gosub_publish_two_row_selection();
    }
    return 0;
}

/**
 * @brief Commit a pending row move by swapping the two marked rows.
 * @return Always 0.
 * @see decomp.me (100%)
 */
s32 gosub_commit_row_reorder(void)
{
    GosubListRow entry_tmp;
    u32 rec_tmp;
    s32 saved_index;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }
    if (g_gosub_selection_count != 2)
    {
        return 0;
    }
    if (g_gosub_selected_rows[0] != g_gosub_selected_rows[1])
    {
        gosub_copy_packed_record(&rec_tmp, g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[0]].index * 4 + 0x29DC));
        gosub_copy_packed_record(g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[0]].index * 4 + 0x29DC),
                      g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[1]].index * 4 + 0x29DC));
        gosub_copy_packed_record(g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[1]].index * 4 + 0x29DC), &rec_tmp);
        gosub_copy_list_row(&entry_tmp, &g_gosub_rows[g_gosub_selected_rows[0]]);
        gosub_copy_list_row(&g_gosub_rows[g_gosub_selected_rows[0]], &g_gosub_rows[g_gosub_selected_rows[1]]);
        gosub_copy_list_row(&g_gosub_rows[g_gosub_selected_rows[1]], &entry_tmp);
        saved_index = g_gosub_rows[g_gosub_selected_rows[0]].index;
        g_gosub_rows[g_gosub_selected_rows[0]].index = g_gosub_rows[g_gosub_selected_rows[1]].index;
        g_gosub_rows[g_gosub_selected_rows[1]].index = saved_index;
        g_gosub_selection_count = 0;
    }
    else
    {
        gosub_open_row_action_dialog();
        g_gosub_selection_count = 1;
    }
    return 0;
}

/**
 * @brief Update row colors for the current group selection.
 * @return 1 after publishing a complete mixed-group selection, otherwise 0.
 * @see decomp.me (100%)
 */
s32 gosub_update_group_selection(void)
{
    s32 i;
    s32 open_count;
    s32 marked_count;

    for (i = 0; i < g_gosub_row_count; i++)
    {
        g_gosub_rows[i].text_color = 4;
    }
    open_count = 0;
    marked_count = 0;
    for (i = 0; i < g_gosub_selection_count; i++)
    {
        if (g_gosub_rows[g_gosub_selected_rows[i]].equipment_kind == 0)
        {
            open_count++;
        }
        else
        {
            marked_count++;
        }
    }
    if (open_count != 0)
    {
        for (i = 0; i < g_gosub_row_count; i++)
        {
            if (g_gosub_rows[i].equipment_kind == 0 && gosub_is_row_unselected(i) != 0)
            {
                g_gosub_rows[i].text_color = 5;
            }
        }
    }
    if (marked_count == 3)
    {
        for (i = 0; i < g_gosub_row_count; i++)
        {
            if (g_gosub_rows[i].equipment_kind != 0 && gosub_is_row_unselected(i) != 0)
            {
                g_gosub_rows[i].text_color = 5;
            }
        }
    }
    if (open_count != 0)
    {
        if (marked_count == 3)
        {
            gosub_publish_selection();
            return 1;
        }
        return 0;
    }
    return 0;
}

/**
 * @brief Publish the picked rows' indices as the screen's result.
 * @return 1 if the result was published, 0 if the picker was not in state 2.
 * @see decomp.me (100%)
 */
s32 gosub_publish_two_row_selection(void)
{
    s32 i;

    if (g_gosub_selection_count == 2)
    {
        g_gosub_result_count = g_gosub_selection_count;
        for (i = 0; i < g_gosub_selection_count; i++)
        {
            g_gosub_result_values[i] = g_gosub_rows[g_gosub_selected_rows[i]].index;
        }
        return 1;
    }
    return 0;
}

/**
 * @brief Handle the confirmation dialog for creating a two-item combination.
 *
 * @param dialog_result Zero to confirm; nonzero to return to the selection.
 * @return 1 if confirming leaves no equipment rows, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/2OzmD
 */
s32 gosub_handle_combination_dialog(s32 dialog_result)
{
    s32 combination_count;
    GosubPackedRecord* record;
    u32 packed;
    s32 result_id;
    s32 packed_result;
    s32 secondary_value;
    s32 clear_config_mask;
    u16 stored_word;

    if (dialog_result == 0 && (g_gosub_dialog_choice & 1) == 0)
    {
        clear_config_mask = ~0xFC;
        combination_count = *(g_pad_ctx + 0x29D6);
        if (combination_count < 0x28)
        {
            record = (GosubPackedRecord*)(g_pad_ctx + combination_count * 4 + 0x29DC);
            result_id = g_gosub_combination_result_id;
            packed = record->word & clear_config_mask;
            packed = packed | ((result_id & 0x3F) << 2);
            record->word = packed;
            secondary_value = g_gosub_combination_quantity;
            dialog_result = (packed & ~0xF00) | ((secondary_value & 0xF) << 8);
            record->word = dialog_result;
            packed_result = ((dialog_result & 0xFFFF0FFF) | ((g_gosub_combination_variant & 0xF) << 12) | 3) & 0xFFFF;
            stored_word = packed_result;
            record->word = stored_word;
            *(g_pad_ctx + 0x29D6) = *(g_pad_ctx + 0x29D6) + 1;
            GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(g_gosub_result_values[0])->name[0] = 0;
            GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(g_gosub_result_values[1])->name[0] = 0;
            func_800A8FB4();
        }
        if (*(g_pad_ctx + 0x29D6) >= 0x28)
        {
            gosub_start_element_exit();
            g_field_gosub_state = 0;
            GOSUB_MSG(-4);
            return 0;
        }
        g_gosub_scroll_frames_remaining = 0;
        g_gosub_scroll_target_y = 0;
        g_gosub_scroll_y = 0;
        g_gosub_cursor_row = 0;
        g_gosub_allow_duplicate_selection = 0;
        gosub_build_equipment_list(3);
        g_gosub_visible_row_count = 6;
        g_gosub_row_height = 0x10;
        g_gosub_window_width = 0xE8;
        g_gosub_window_height = 0x64;
        g_gosub_combination_variant = 0;
        g_gosub_combination_result_id = 0;
        g_gosub_combination_quantity = 0;
        g_gosub_required_selection_count = 2;
        g_gosub_selection_mode = 2;
        g_gosub_selection_count = 0;
        g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
        g_gosub_screen_sequence_index -= 1;
        if (g_gosub_row_count == 0)
        {
            g_field_gosub_state = 0;
            func_80067F28();
            gosub_start_element_exit();
            return 1;
        }
        return 0;
    }
    g_gosub_selection_count -= 1;
    g_gosub_screen_sequence_index -= 1;
    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
    return 0;
}

/**
 * @brief Publish the selected group rows as result values.
 *
 * @return 1 when at least one row was published, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/pOY6i
 */
s32 gosub_publish_group_selection(void)
{
    s32 i;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }

    g_gosub_result_count = g_gosub_selection_count;

    for (i = 0; i < g_gosub_selection_count; i++)
    {
        g_gosub_result_values[i] = g_gosub_rows[g_gosub_selected_rows[i]].index;
    }

    return 1;
}

/**
 * @brief Publish the selected rows' entry indices as result values.
 *
 * @return 1 when at least one row was published, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/FN7DQ
 */
s32 gosub_publish_selection(void)
{
    s32 i;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }

    g_gosub_result_count = g_gosub_selection_count;

    for (i = 0; i < g_gosub_selection_count; i++)
    {
        g_gosub_result_values[i] = g_gosub_rows[g_gosub_selected_rows[i]].index;
    }

    return 1;
}

/**
 * @brief Test whether a row is absent from the current selection.
 *
 * @param row Row index to test.
 * @return 1 if the row is unselected, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/lBIH9
 */
s32 gosub_is_row_unselected(s32 row)
{
    s32 i;
    s32 count = g_gosub_selection_count;

    for (i = 0; i < count; i++)
    {
        if (g_gosub_selected_rows[i] == row)
        {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Build list rows from nonempty equipment records of the requested kind.
 *
 * Kind 3 accepts every record; kind 4 accepts every record except kind 2.
 *
 * @param item_kind Equipment kind filter, or 3/4 for the aggregate filters.
 * @see decomp.me (100%) https://decomp.me/scratch/CJYqj
 */
void gosub_build_equipment_list(u32 item_kind)
{
    s32 item_index;
    s32 stat_index;
    s32 row_count;
    u8* item_base;
    u8* entry;
    u8* item;
    s32 separator_offset;
    GosubEquipmentRecord* record;
    u32 attributes;

    g_gosub_show_row_details = 1;
    row_count = 0;

    for (item_index = 0; item_index < 100; item_index++)
    {
        entry = GOSUB_EQUIPMENT_BASE_FROM_INDEX(item_index);
        if (GOSUB_EQUIPMENT_RECORD(entry)->name[0] != 0)
        {
            if ((item_kind == 3) || (GOSUB_EQUIPMENT_KIND(GOSUB_EQUIPMENT_RECORD(entry)->attributes.word) == item_kind) ||
                ((item_kind == 4) && (GOSUB_EQUIPMENT_KIND(GOSUB_EQUIPMENT_RECORD(entry)->attributes.word) != 2)))
            {

                g_gosub_rows[row_count].name = GOSUB_EQUIPMENT_AT(item_index)->name;

                gosub_copy_encoded_string(GOSUB_TEXT_BUFFER(row_count),
                              ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(item_index)->attributes.half.material & 0x3F));
                separator_offset = (s32)(D_800EC3E2 - 0x1E) + (D_800EC3E2[1] << 8);
                gosub_append_encoded_string(GOSUB_TEXT_BUFFER(row_count), D_800EC3E2[0] + separator_offset);

                item_base = GOSUB_EQUIPMENT_BASE_FROM_INDEX(item_index);
                g_gosub_rows[row_count].equipment_kind = GOSUB_EQUIPMENT_KIND(GOSUB_EQUIPMENT_RECORD(item_base)->attributes.word);
                attributes = GOSUB_EQUIPMENT_RECORD(item_base)->attributes.word;

                switch (GOSUB_EQUIPMENT_KIND(attributes))
                {
                case 0:
                    gosub_append_encoded_string(GOSUB_TEXT_BUFFER(row_count), ARCHIVE_ENTRY(GOSUB_TEXT_ARCHIVE->block_offsets[3], GOSUB_EQUIPMENT_CATEGORY(attributes)));
                    g_gosub_rows[row_count].primary_value = GOSUB_EQUIPMENT_AT(item_index)->data.kind0_value;
                    break;
                case 1:
                    gosub_append_encoded_string(GOSUB_TEXT_BUFFER(row_count),
                                  ARCHIVE_ENTRY(GOSUB_TEXT_ARCHIVE->block_offsets[3], GOSUB_EQUIPMENT_CATEGORY(attributes) + 0xB));
                    record = GOSUB_EQUIPMENT_FROM_INDEX(item_index);
                    for (stat_index = 0; stat_index < 4; stat_index++)
                    {
                        g_gosub_rows[row_count].stats[stat_index] = record->data.kind1_stats[stat_index];
                    }
                    break;
                default:
                    item = GOSUB_EQUIPMENT_SOURCE_FROM_INDEX(item_index);
                    record = GOSUB_EQUIPMENT_RECORD(item);
                    g_gosub_rows[row_count].primary_value = record->data.kind2.value;
                    g_gosub_rows[row_count].stats[0] = record->data.kind2.index + (record->data.kind2.group * 14);
                    gosub_append_encoded_string(GOSUB_TEXT_BUFFER(row_count), GOSUB_KIND2_ARCHIVE_ENTRY(GOSUB_EQUIPMENT_RECORD(item)->attributes.word));
                    break;
                }

                g_gosub_rows[row_count].desc = GOSUB_TEXT_BUFFER(row_count);
                g_gosub_rows[row_count].value = -1;
                g_gosub_rows[row_count].index = item_index;
                g_gosub_rows[row_count].text_color = 4;
                row_count++;
            }
        }
    }

    g_gosub_row_count = row_count;
    g_gosub_visible_row_count = 8;

    switch (item_kind)
    {
    case 0:
        g_gosub_title_text = GOSUB_MSG_PTR(0xC);
        break;
    case 1:
        g_gosub_title_text = GOSUB_MSG_PTR(0xE);
        break;
    case 2:
        g_gosub_title_text = GOSUB_MSG_PTR(0x10);
        break;
    case 3:
        g_gosub_title_text = GOSUB_MSG_PTR(0x16);
        break;
    case 4:
        g_gosub_visible_row_count = 7;
        g_gosub_title_text = GOSUB_MSG_PTR(0x16);
        break;
    }

    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = (g_gosub_visible_row_count * 0x10) + 4;
}
/**
 * @brief Build one of the three grouped option lists from the text archive.
 *
 * @param group Option group index, from 0 through 2.
 * @see decomp.me (100%)
 */
void gosub_build_grouped_option_list(s32 group)
{
    s32 option_index;
    GosubGroupTable first_indices = g_gosub_group_first_indices;
    GosubGroupTable counts = g_gosub_group_counts;

    for (option_index = 0; option_index < counts.values[group]; option_index++)
    {
        GosubListRow* row = &g_gosub_rows[option_index];
        u8* text;

        row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_2[0], option_index + first_indices.values[group]);
        text = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_2[0], option_index + first_indices.values[group]);
        row->name = text;
        row->value = -1;
        row->index = option_index;
        row->equipment_kind = 0;
        row->desc = text;
        row->text_color = 4;
    }

    g_gosub_row_count = counts.values[group];

    switch (group)
    {
    case 0:
        g_gosub_title_text = GOSUB_MSG_PTR(0x1A);
        break;
    case 1:
        g_gosub_title_text = GOSUB_MSG_PTR(0x1C);
        break;
    case 2:
        g_gosub_title_text = GOSUB_MSG_PTR(0x1E);
        break;
    }

    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
}
