#include "common/card_menu.h"

/**
 * @brief Draw the slot-0 card label, dimming it with a backing tile when that
 *        slot is not the active one.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 */
void* card_menu_draw_card_slot0_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;
    TILE* tile;

    if (g_card_slot != 0)
    {
        tile = (TILE*)prim;
        SET_BGR0_PACKED(tile, CARD_MENU_INACTIVE_LABEL_COLOR);
        setTile(tile);
        setSemiTrans(tile, 1);
        setXY0(tile, 0, 0);
        setWH(tile, CARD_MENU_CARD_LABEL_WIDTH, CARD_MENU_CARD_LABEL_HEIGHT);
        addPrim(ot, tile);
        prim = tile + 1;
    }
    return field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_card_slot0_label, CARD_MENU_TEXT_CARD_SLOT0_LABEL), FIELD_TEXT_COLOR_NORMAL,
                           -x_offset + CARD_MENU_CARD_LABEL_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
}
