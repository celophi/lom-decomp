#include "common/card_menu.h"

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
void* card_menu_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 entry_state = g_card_entry_state;

    switch (entry_state)
    {
    case CARD_MENU_ENTRY_STATE_NO_GAME_DATA:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_UNFORMATTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_CARD_FULL:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_no_card, CARD_MENU_TEXT_NO_CARD), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_card_access_failed, CARD_MENU_TEXT_CARD_ACCESS_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_NO_SAVE_DATA:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_no_save_data, CARD_MENU_TEXT_NO_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_ENTRY_STATE_BLANK:
        break;
    case CARD_MENU_ENTRY_STATE_CHECKING_CARD:
    {
        s32 message_x;
        u16* text_table;

        message_x = -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2;
        text_table = &g_card_menu_text_table;
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

        if (g_card_menu_entry_scan_active != 0)
        {
            s32 message_x;
            u16* text_table;

            message_x = -x_offset + CARD_MENU_EXCHANGE_LIST_WIDTH / 2;
            text_table = &g_card_menu_text_table;
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

            text_table = &g_card_menu_text_table;
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
                            ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_card_menu_text_number_label), FIELD_TEXT_COLOR_NORMAL, list_x + CARD_MENU_ENTRY_NUMBER_LABEL_X,
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
                            prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_card_menu_text_plus_marker), FIELD_TEXT_COLOR_NORMAL,
                                                   CARD_MENU_EXCHANGE_ENTRY_PLUS_RIGHT_X - x_offset, row_y, FIELD_TEXT_ALIGN_RIGHT);
                        }
                    }
                    if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_card_menu_text_mana_label), FIELD_TEXT_COLOR_NORMAL,
                                               CARD_MENU_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_card_menu_text_ring_ring_land_label), FIELD_TEXT_COLOR_NORMAL,
                                               CARD_MENU_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) == 0)
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_card_menu_text_new_save_label), FIELD_TEXT_COLOR_NORMAL,
                                               CARD_MENU_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                    else
                    {
                        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_BY_OFFSET(text_table, g_card_menu_text_other_game_label), FIELD_TEXT_COLOR_NORMAL,
                                               CARD_MENU_ENTRY_LABEL_X - x_offset, row_y, FIELD_TEXT_ALIGN_LEFT);
                    }
                }
                entry_index++;
            } while (entry_index < g_card_entry_state);
        }
        row_y = ((g_card_menu_selected_row * CARD_MENU_ENTRY_ROW_HEIGHT) - y_offset) - g_card_menu_scroll_y;

        if (g_card_menu_entry_scan_active == 0)
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
