#include "common/card_menu.h"

/**
 * @brief Draw the two choices of a yes/no prompt from the FIELD UI string table,
 *        highlighting the selected one, and toggle the selection on left/right.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table entry the text is linked into.
 * @param x Prompt center; the choices are drawn CARD_MENU_CHOICE_YES_GAP left and CARD_MENU_CHOICE_NO_GAP right of it.
 * @param y Prompt baseline.
 * @return Advanced primitive-buffer cursor.
 */
inline void* card_menu_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y)
{
    u8* text_table;
    u8* yes_text;
    u8* no_text;
    s32 color;

    color = FIELD_TEXT_COLOR_NORMAL;
    yes_text = FIELD_UI_TEXT_AT(g_text_choice_glyph_offsets, FIELD_UI_TEXT_YES);
    text_table = FIELD_UI_TEXT_TABLE(g_text_choice_glyph_offsets, FIELD_UI_TEXT_YES);
    if (g_card_menu_choice_toggle != 0)
    {
        color = FIELD_TEXT_COLOR_DIM;
    }
    prim = field_draw_text(prim, ot, yes_text, color, x - CARD_MENU_CHOICE_YES_GAP, y, FIELD_TEXT_ALIGN_RIGHT);

    color = FIELD_TEXT_COLOR_NORMAL;
    no_text = FIELD_UI_TEXT(text_table, FIELD_UI_TEXT_NO);
    if (g_card_menu_choice_toggle == 0)
    {
        color = FIELD_TEXT_COLOR_DIM;
    }
    prim = field_draw_text(prim, ot, no_text, color, x + CARD_MENU_CHOICE_NO_GAP, y, FIELD_TEXT_ALIGN_LEFT);

    if (g_pad_input & CARD_MENU_CHOICE_BUTTON_MASK)
    {
        g_card_menu_choice_toggle ^= 1;
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        g_pad_input = 0;
    }
    return prim;
}
