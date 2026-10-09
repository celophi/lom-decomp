#include "common/card_menu.h"

/**
 * @brief Draw the load prompt in the modal window and act on its answer.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset; the text is centred in the message window.
 * @param y_offset Vertical transition offset.
 * @return Primitive-buffer cursor after the prompt.
 * @note A card error closes the prompt and rescans the card; Circle or "no" closes
 *       it and aborts the load; "yes" turns the window into the load progress screen.
 */
void* card_menu_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused;
    void* result;
    s32 x;
    s32 status;
    CardMenuElement* element;

    x = -x_offset + CARD_MENU_MESSAGE_WIDTH / 2;
    result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_load_prompt, CARD_MENU_TEXT_LOAD_PROMPT), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                             FIELD_TEXT_ALIGN_CENTER);
    result = card_menu_draw_choice_prompt(result, ot, x, CARD_MENU_LINE_HEIGHT - y_offset);

    status = poll_and_retry_card_info();
    if (status == CARD_EVENT_ERROR || status == CARD_EVENT_TIMEOUT)
    {
        card_menu_deactivate_primary_element();
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        card_reset_entry_ranks();
        g_card_step = NULL;
    }
    else if ((g_pad_input & PAD_BTN_CIRCLE) || ((g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK) && (g_card_menu_choice_toggle != CARD_MENU_CHOICE_YES)))
    {
        card_menu_deactivate_primary_element();
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_step = g_card_steps_card_info;
    }
    else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
        g_card_menu_progress_active = 1;
        g_card_step = g_card_steps_read_save;
        element = &g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL];
        element->draw = card_menu_draw_transfer_window;
        element->attr.bits.transition_step = 1;
        element->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
        element->attr.bits.x = CARD_MENU_MESSAGE_X;
        element->attr.bits.y = CARD_MENU_EXCHANGE_MESSAGE_Y;
        element->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
        element->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_MESSAGE_WIDTH);
    }
    return result;
}
