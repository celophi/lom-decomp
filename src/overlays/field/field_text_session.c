/** @file field_text_session.c
 * @brief Reset and per-frame update of the field item menu, which draws through the text system.
 */

#include "field_text.h"
#include "common.h"
#include "field_calls.h"
#include "field_menu_element.h"

extern s32 D_80122714;
extern s32 g_field_item_list_count;
extern s32 D_80122980;
extern s32 g_field_item_list_cursor;

/** @brief Close the item menu and clear its list, cursor and script toggle. */
void field_reset_item_menu(void)
{
    D_80122980 = 0;
    g_field_item_list_count = 0;
    D_80122714 = 0;
    g_field_item_list_cursor = 0;
}

/**
 * @brief Run one frame of the item menu while it is open.
 *
 * Resets the text scratch state, updates and draws the menu elements and
 * uploads the glyphs they typeset. If the menu closed during the update, the
 * text windows are reset as well.
 *
 * @param render_half Render half the menu elements are drawn into.
 */
void field_update_item_menu(FieldRenderHalf* render_half)
{
    if (D_80122714 != 0)
    {
        field_text_reset_scratch();
        field_draw_menu_elements(render_half);
        field_text_upload_immediate_cache();
        if (D_80122714 == 0)
        {
            field_text_reset_windows();
        }
    }
}
