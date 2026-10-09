#include "common/card_menu.h"

/**
 * @brief Draw the header glyph that reflects the current mode (load vs save).
 * @param ot   Ordering table the glyph is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 */
void* card_menu_draw_mode_title(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;

    if (g_card_menu_mode == 1)
    {
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_select_item, CARD_MENU_TEXT_SELECT_ITEM), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    else
    {
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_select_save_data, CARD_MENU_TEXT_SELECT_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_EXCHANGE_TITLE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    return prim;
}
