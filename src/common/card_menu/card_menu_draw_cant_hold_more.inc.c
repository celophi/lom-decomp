#include "common/card_menu.h"

/**
 * @brief Draw FIELD's "Can't hold any more." notice, centred in a 256-pixel window.
 * @param ot Ordering table the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note Nothing in CARDA uses it.
 */
void* card_menu_draw_cant_hold_more(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;

    return field_draw_text(prim, ot, FIELD_UI_TEXT_AT(g_field_ui_text_cant_hold_more, FIELD_UI_TEXT_CANT_HOLD_MORE), FIELD_TEXT_COLOR_DIM,
                           CARD_MENU_DIALOG_WIDTH / 2 - x_offset, -y_offset, FIELD_TEXT_ALIGN_CENTER);
}
