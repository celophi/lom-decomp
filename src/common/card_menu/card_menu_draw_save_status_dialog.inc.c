#include "common/card_menu.h"

/**
 * @brief Draw the status message chosen by g_card_menu_dialog_state and close the menu on confirm.
 * @param ot Ordering table receiving the text packets.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return GPU packet cursor after the message.
 */
void* card_menu_draw_save_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];

    switch (g_card_menu_dialog_state)
    {
    case CARD_MENU_DIALOG_SAVE_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_save_failed, CARD_MENU_TEXT_SAVE_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_CARD_NOT_INSERTED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_card_not_inserted, CARD_MENU_TEXT_CARD_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_not_pocketstation, CARD_MENU_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_LOAD_FAILED:
    case CARD_MENU_DIALOG_INVALID_SAVE:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        card_menu_clear_elements();
        field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
        field_reset_input_repeat();
    }
    return prim;
}
