#include "common/card_menu.h"

/**
 * @brief Free every element of the pool and select the sub-overlay frame style.
 */
inline void card_menu_clear_elements(void)
{
    CardMenuElement* p;
    s32 i;

    g_field_menu_frame_style = FIELD_MENU_FRAME_STYLE_SUBSCREEN;
    p = g_card_menu_element_pool;
    for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++)
    {
        p->attr.bits.state = CARD_MENU_ELEMENT_FREE;
        p++;
    }
}
