#include "common/card_menu.h"

/**
 * @brief Turn the modal window into a dialog showing @p message_id and abandon
 *        any card transfer; acknowledging the dialog returns to the browser.
 * @param message_id CARD_MENU_DIALOG_* or CARD_MENU_DIALOG_INVALID_SAVE message to show.
 */
inline void card_menu_open_status_dialog(s32 message_id)
{
    CardMenuElement* element;

    field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    element = &g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL];
    element->draw = card_menu_draw_status_dialog;
    element->attr.bits.transition_step = 1;
    element->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
    element->attr.bits.x = CARD_MENU_DIALOG_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_DIALOG_Y;
    element->size.bits.width_high = CARD_MENU_DIALOG_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_EXCHANGE_DIALOG_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_DIALOG_WIDTH);
    field_reset_input_repeat();
    g_card_menu_write_in_progress = 0;
    g_card_menu_progress_active = 0;
    g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
    g_card_menu_io_busy = 0;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    card_reset_entry_ranks();
    g_card_step = NULL;
    g_card_menu_dialog_state = message_id;
}
