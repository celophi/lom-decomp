#include "common/card_menu.h"

/**
 * @brief Restore FIELD's fade target and start the closing transition of every open window.
 */
inline void card_menu_close_all_elements(void)
{
    CardMenuElement* element;
    s32 slot;

    field_restore_fade_target();
    element = g_card_menu_element_pool;
    for (slot = 0; slot < CARD_MENU_ELEMENT_COUNT; slot++, element++)
    {
        if (element->attr.bits.state != CARD_MENU_ELEMENT_FREE)
        {
            element->attr.bits.state = CARD_MENU_ELEMENT_CLOSING;
            element->attr.bits.transition_step = CARD_MENU_ELEMENT_TRANSITION_STEPS;
        }
    }
}
