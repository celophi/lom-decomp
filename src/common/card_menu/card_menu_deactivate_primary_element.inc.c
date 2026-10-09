#include "common/card_menu.h"

/**
 * @brief Free the modal window at once, without a closing transition.
 */
inline void card_menu_deactivate_primary_element(void)
{
    g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
}
