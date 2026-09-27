#include "gosub_internal.h"

/**
 * @brief Inventory record @p index.
 * @note Summed as integers, index first, as the original stat reads do.
 */
#define GOSUB_INVENTORY_RECORD(index) ((InventoryRecord*)((index) * sizeof(InventoryRecord) + (u32)g_pad_ctx->inventory))

/**
 * @brief Select the pet or golem under the cursor to take along; eggs and grazing pets are refused.
 * @return 0 if the row was refused, otherwise 1 (also when nothing is selected and nothing was queued).
 */
s32 gosub_select_companion_to_take(void)
{
    s32 row;
    GosubListRow* list;
    GosubListRow* row_entry;

    list = g_gosub_rows;
    row = g_gosub_cursor_row;
    row_entry = &list[row];
    if (row_entry->flags.half & 1)
    {
        gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_EGG_CANNOT_LEAVE));
        g_gosub_selection_count = 0;
        g_gosub_suppress_dialog_sound = 1;
        return 0;
    }
    if (row_entry->flags.companion.grazing)
    {
        gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_CANNOT_LEAVE));
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
 * @brief Queue the row under the cursor as the screen's result.
 * @return Always 1. Nothing is queued while no row is selected.
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
 * @brief Accept the second logic-block component only when the pair makes a block.
 * @return gosub_publish_block_components's result when the preview found a block, otherwise 0.
 */
s32 gosub_select_block_component(void)
{
    if (g_gosub_selection_count != 0)
    {
        if (g_gosub_block_id == 0)
        {
            if (g_gosub_selection_count == 2)
            {
                g_gosub_selection_count = 1;
            }
            return 0;
        }
        return gosub_publish_block_components();
    }
    return 0;
}

/**
 * @brief Swap two selected logic blocks, or open the sort/discard dialog when the same block is picked twice.
 * @return Always 0.
 * @note JP does not swap the two rows' index fields back after swapping the rows.
 */
s32 gosub_select_logic_block(void)
{
    GosubListRow saved_row;
    LogicBlock saved_block;
#if !defined(VERSION_JP)
    s32 saved_index;
#endif

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
        gosub_copy_logic_block(&saved_block, &g_pad_ctx->logic_blocks[g_gosub_rows[g_gosub_selected_rows[0]].index]);
        gosub_copy_logic_block(&g_pad_ctx->logic_blocks[g_gosub_rows[g_gosub_selected_rows[0]].index],
                               &g_pad_ctx->logic_blocks[g_gosub_rows[g_gosub_selected_rows[1]].index]);
        gosub_copy_logic_block(&g_pad_ctx->logic_blocks[g_gosub_rows[g_gosub_selected_rows[1]].index], &saved_block);
        gosub_copy_list_row(&saved_row, &g_gosub_rows[g_gosub_selected_rows[0]]);
        gosub_copy_list_row(&g_gosub_rows[g_gosub_selected_rows[0]], &g_gosub_rows[g_gosub_selected_rows[1]]);
        gosub_copy_list_row(&g_gosub_rows[g_gosub_selected_rows[1]], &saved_row);
#if !defined(VERSION_JP)
        saved_index = g_gosub_rows[g_gosub_selected_rows[0]].index;
        g_gosub_rows[g_gosub_selected_rows[0]].index = g_gosub_rows[g_gosub_selected_rows[1]].index;
        g_gosub_rows[g_gosub_selected_rows[1]].index = saved_index;
#endif
        g_gosub_selection_count = 0;
    }
    else
    {
        gosub_open_block_action_dialog();
        g_gosub_selection_count = 1;
    }
    return 0;
}

/**
 * @brief Grey out the rows that can no longer be picked as golem parts.
 *
 * A golem takes one weapon and up to GOSUB_GOLEM_ARMOR_PARTS pieces of armor:
 * once a weapon is picked the other weapons are greyed out, and once all
 * armor is picked the other armor is.
 *
 * @return 1 after publishing a complete set of parts, otherwise 0.
 */
s32 gosub_select_golem_part(void)
{
    s32 i;
    s32 weapon_count;
    s32 armor_count;

    for (i = 0; i < g_gosub_row_count; i++)
    {
        g_gosub_rows[i].text_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    weapon_count = 0;
    armor_count = 0;
    for (i = 0; i < g_gosub_selection_count; i++)
    {
        if (g_gosub_rows[g_gosub_selected_rows[i]].equipment_kind == GOSUB_EQUIPMENT_KIND_WEAPON)
        {
            weapon_count++;
        }
        else
        {
            armor_count++;
        }
    }
    if (weapon_count != 0)
    {
        for (i = 0; i < g_gosub_row_count; i++)
        {
            if (g_gosub_rows[i].equipment_kind == GOSUB_EQUIPMENT_KIND_WEAPON && gosub_is_row_unselected(i) != 0)
            {
                g_gosub_rows[i].text_color = GOSUB_TEXT_COLOR_DISABLED;
            }
        }
    }
    if (armor_count == GOSUB_GOLEM_ARMOR_PARTS)
    {
        for (i = 0; i < g_gosub_row_count; i++)
        {
            if (g_gosub_rows[i].equipment_kind != GOSUB_EQUIPMENT_KIND_WEAPON && gosub_is_row_unselected(i) != 0)
            {
                g_gosub_rows[i].text_color = GOSUB_TEXT_COLOR_DISABLED;
            }
        }
    }
    if (weapon_count != 0)
    {
        if (armor_count == GOSUB_GOLEM_ARMOR_PARTS)
        {
            gosub_publish_selection();
            return 1;
        }
        return 0;
    }
    return 0;
}

/**
 * @brief Publish the two logic-block components as the screen's result.
 * @return 1 if both components were picked and published, otherwise 0.
 */
s32 gosub_publish_block_components(void)
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
 * @brief Handle the confirmation dialog that turns two components into a logic block.
 *
 * Confirming consumes both components, adds the block and restarts the
 * component list, unless the block list is now full or no components are left.
 *
 * @param dialog_result Zero to confirm; nonzero to return to the selection.
 * @return 1 if confirming leaves no components, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/2OzmD
 */
s32 gosub_handle_make_block_dialog(s32 dialog_result)
{
    s32 count;

    if (dialog_result == 0 && (g_gosub_dialog_choice & GOSUB_CONFIRMATION_CHOICE_MASK) == 0)
    {
        count = g_pad_ctx->logic_block_count;
        if (count < LOGIC_BLOCK_CAPACITY)
        {
            g_pad_ctx->logic_blocks[count].f.id = g_gosub_block_id;
            g_pad_ctx->logic_blocks[count].f.quantity = g_gosub_block_level;
            g_pad_ctx->logic_blocks[count].f.shape = g_gosub_block_shape;
            g_pad_ctx->logic_blocks[count].f.logic_type = LOGIC_BLOCK_UNASSIGNED;
            g_pad_ctx->logic_blocks[count].f.placed = 0;
            g_pad_ctx->logic_blocks[count].f.rotation = 0;
            g_pad_ctx->logic_blocks[count].f.grid_x = 0;
            g_pad_ctx->logic_blocks[count].f.grid_y = 0;
            g_pad_ctx->logic_blocks[count].f.unknown_bits = 0;
            g_pad_ctx->logic_block_count = g_pad_ctx->logic_block_count + 1;
            g_pad_ctx->inventory[g_gosub_result_values[0]].name[0] = 0;
            g_pad_ctx->inventory[g_gosub_result_values[1]].name[0] = 0;
            field_compact_inventory();
        }
        if (g_pad_ctx->logic_block_count >= LOGIC_BLOCK_CAPACITY)
        {
            gosub_close_elements();
            g_field_gosub_state = 0;
            gosub_open_message_dialog(GOSUB_MESSAGE(GOSUB_MSG_LOGIC_BLOCKS_FULL));
            return 0;
        }
        g_gosub_scroll_frames_remaining = 0;
        g_gosub_scroll_target_y = 0;
        g_gosub_scroll_y = 0;
        g_gosub_cursor_row = 0;
        g_gosub_allow_duplicate_selection = 0;
        gosub_build_equipment_list(GOSUB_EQUIPMENT_KIND_ANY);
        g_gosub_visible_row_count = 6;
        g_gosub_row_height = GOSUB_ITEM_ROW_HEIGHT;
        g_gosub_window_width = GOSUB_LIST_PANEL_WIDTH;
        g_gosub_window_height = 6 * GOSUB_ITEM_ROW_HEIGHT + GOSUB_LIST_PANEL_PADDING;
        g_gosub_block_shape = 0;
        g_gosub_block_id = 0;
        g_gosub_block_level = 0;
        g_gosub_required_selection_count = 2;
        g_gosub_selection_mode = 2;
        g_gosub_selection_count = 0;
        g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
        g_gosub_screen_sequence_index -= 1;
        if (g_gosub_row_count == 0)
        {
            g_field_gosub_state = 0;
            field_restore_fade_target();
            gosub_close_elements();
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
 * @brief Publish the picked golem parts as result values.
 *
 * @return 1 when at least one row was published, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/pOY6i
 */
s32 gosub_publish_golem_parts(void)
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
 * @brief List the weapons, armor or instruments held, each described by material and type.
 * @param item_kind GosubEquipmentKind category, GOSUB_EQUIPMENT_KIND_ANY or GOSUB_EQUIPMENT_KIND_GOLEM_PARTS.
 * @see decomp.me (100%) https://decomp.me/scratch/CJYqj
 */
void gosub_build_equipment_list(u32 item_kind)
{
    s32 item_index;
    s32 stat_index;
    s32 row_count;
    u32 attributes;

    g_gosub_show_row_details = 1;
    row_count = 0;

    for (item_index = 0; item_index < INVENTORY_RECORD_COUNT; item_index++)
    {
        if (g_pad_ctx->inventory[item_index].name[0] != 0)
        {
            if ((item_kind == GOSUB_EQUIPMENT_KIND_ANY) || (INVENTORY_KIND(g_pad_ctx->inventory[item_index].attributes.packed) == item_kind) ||
                ((item_kind == GOSUB_EQUIPMENT_KIND_GOLEM_PARTS) &&
                 (INVENTORY_KIND(g_pad_ctx->inventory[item_index].attributes.packed) != GOSUB_EQUIPMENT_KIND_INSTRUMENT)))
            {
                g_gosub_rows[row_count].name = g_pad_ctx->inventory[item_index].name;

                gosub_copy_encoded_string(g_gosub_text_buffers[row_count],
                                          GOSUB_TEXT(GOSUB_TEXT_ITEM_NAMES, g_pad_ctx->inventory[item_index].attributes.halves.high & INVENTORY_MATERIAL_MASK));
                gosub_append_encoded_string(g_gosub_text_buffers[row_count], FIELD_UI_TEXT_AT(D_800EC3E2, FIELD_UI_TEXT_SPACE));

                g_gosub_rows[row_count].equipment_kind = INVENTORY_KIND(g_pad_ctx->inventory[item_index].attributes.packed);
                attributes = g_pad_ctx->inventory[item_index].attributes.packed;

                switch (INVENTORY_KIND(attributes))
                {
                case GOSUB_EQUIPMENT_KIND_WEAPON:
                    gosub_append_encoded_string(g_gosub_text_buffers[row_count],
                                                GOSUB_TEXT(GOSUB_TEXT_EQUIPMENT_TYPES, GOSUB_WEAPON_TYPE_FIRST + INVENTORY_CATEGORY(attributes)));
                    g_gosub_rows[row_count].primary_value = GOSUB_INVENTORY_RECORD(item_index)->stats.values[0];
                    break;
                case GOSUB_EQUIPMENT_KIND_ARMOR:
                    gosub_append_encoded_string(g_gosub_text_buffers[row_count],
                                                GOSUB_TEXT(GOSUB_TEXT_EQUIPMENT_TYPES, INVENTORY_CATEGORY(attributes) + GOSUB_ARMOR_TYPE_FIRST));
                    for (stat_index = 0; stat_index < HISTORY_RECORD_STAT_COUNT; stat_index++)
                    {
                        g_gosub_rows[row_count].stats[stat_index] = GOSUB_INVENTORY_RECORD(item_index)->stats.values[stat_index];
                    }
                    break;
                default:
                    g_gosub_rows[row_count].primary_value = GOSUB_INVENTORY_RECORD(item_index)->stats.bytes[2];
                    g_gosub_rows[row_count].stats[0] =
                        GOSUB_INVENTORY_RECORD(item_index)->stats.bytes[1] + (GOSUB_INVENTORY_RECORD(item_index)->stats.bytes[0] * 14);
                    gosub_append_encoded_string(g_gosub_text_buffers[row_count],
                                                GOSUB_TEXT(GOSUB_TEXT_EQUIPMENT_TYPES, INVENTORY_CATEGORY(g_pad_ctx->inventory[item_index].attributes.packed) +
                                                                                           GOSUB_INSTRUMENT_TYPE_FIRST));
                    break;
                }

                g_gosub_rows[row_count].desc = g_gosub_text_buffers[row_count];
                g_gosub_rows[row_count].value = GOSUB_ROW_NO_COUNT;
                g_gosub_rows[row_count].index = item_index;
                g_gosub_rows[row_count].text_color = GOSUB_TEXT_COLOR_NORMAL;
                row_count++;
            }
        }
    }

    g_gosub_row_count = row_count;
    g_gosub_visible_row_count = 8;

    switch (item_kind)
    {
    case 0:
        g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_WEAPON);
        break;
    case 1:
        g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_ARMOR);
        break;
    case 2:
        g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_INSTRUMENT);
        break;
    case 3:
        g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_EQUIPMENT);
        break;
    case 4:
        g_gosub_visible_row_count = 7;
        g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_EQUIPMENT);
        break;
    }

    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = (g_gosub_visible_row_count * 0x10) + 4;
}
/**
 * @brief List the weapon, armor or instrument types that can be made.
 *
 * @param group GosubEquipmentKind category (weapon, armor or instrument).
 * @note Each name is looked up and stored twice, as in the original.
 */
void gosub_build_equipment_type_list(s32 group)
{
    s32 option_index;
    GosubCategoryTable firsts = g_gosub_equipment_type_firsts;
    GosubCategoryTable counts = g_gosub_equipment_type_counts;

    for (option_index = 0; option_index < counts.values[group]; option_index++)
    {
        GosubListRow* row = &g_gosub_rows[option_index];
        u8* text;

        row->name = GOSUB_TEXT(GOSUB_TEXT_EQUIPMENT_TYPES, option_index + firsts.values[group]);
        text = GOSUB_TEXT(GOSUB_TEXT_EQUIPMENT_TYPES, option_index + firsts.values[group]);
        row->name = text;
        row->value = GOSUB_ROW_NO_COUNT;
        row->index = option_index;
        row->equipment_kind = 0;
        row->desc = text;
        row->text_color = GOSUB_TEXT_COLOR_NORMAL;
    }

    g_gosub_row_count = counts.values[group];

    switch (group)
    {
    case 0:
        g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_WEAPON_TYPE);
        break;
    case 1:
        g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_ARMOR_TYPE);
        break;
    case 2:
        g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_INSTRUMENT_TYPE);
        break;
    }

    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
}
