/** @file field_text_session.c
 * @brief Reset and per-frame update of the field item-drop menu, which draws through the text system.
 */

#include "field_text.h"
#include "common.h"
#include "field_calls.h"
#include "field_menu_element.h"

extern s32 g_field_item_drop_menu_open;
extern s32 g_field_item_list_count;
extern s32 g_field_item_drop_enabled;
extern s32 g_field_item_list_cursor;

/**
 * @brief Close the item-drop menu and clear its list, cursor and drop permission.
 * @note Clearing g_field_item_drop_enabled means Square does nothing until a
 *       script runs command 0x32 again.
 */
void field_reset_item_menu(void)
{
    g_field_item_drop_enabled = 0;
    g_field_item_list_count = 0;
    g_field_item_drop_menu_open = 0;
    g_field_item_list_cursor = 0;
}

/**
 * @brief Run one frame of the item-drop menu while it is open.
 *
 * Resets the text scratch state, updates and draws the menu elements and
 * uploads the glyphs they typeset. If the menu closed during the update, the
 * text windows are reset as well. The menu is opened with Square from
 * field_process_input() and closes when an item is dropped or Cross cancels.
 *
 * @param render_half Render half the menu elements are drawn into.
 */
void field_update_item_menu(FieldRenderHalf* render_half)
{
    if (g_field_item_drop_menu_open != 0)
    {
        field_text_reset_scratch();
        field_draw_menu_elements(render_half);
        field_text_upload_immediate_cache();
        if (g_field_item_drop_menu_open == 0)
        {
            field_text_reset_windows();
        }
    }
}
