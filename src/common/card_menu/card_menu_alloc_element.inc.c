#include "common/card_menu.h"

/**
 * @brief Claim the first free pool element and start its opening transition.
 * @return The claimed element, or the pool base element when none are free.
 */
inline CardMenuElement* card_menu_alloc_element(void)
{
    CardMenuElement* p;
    s32 i;

    p = g_card_menu_element_pool;
    for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++, p++)
    {
        if (p->attr.bits.state == CARD_MENU_ELEMENT_FREE)
        {
            p->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
            return p;
        }
    }
    return g_card_menu_element_pool;
}
