#include "common/card_menu.h"

/**
 * @brief Put the yes/no prompt on its starting choice, CARD_MENU_CHOICE_DEFAULT.
 */
inline void card_menu_enable_choice_toggle(void)
{
    g_card_menu_choice_toggle = CARD_MENU_CHOICE_DEFAULT;
}
