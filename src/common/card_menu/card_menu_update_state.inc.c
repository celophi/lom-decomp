#include "common/card_menu.h"

/**
 * @brief Run one frame of overlay logic: update elements, advance the load
 *        sequence when armed, sample pad input, and step the scroll animation.
 * @param render Frame drawing context passed to the element renderer.
 */
void card_menu_update_state(FieldRenderHalf* render)
{
    card_menu_update_elements(render);
    g_card_menu_icon_phase += 2;
    if (g_card_menu_element_pool[CARD_MENU_ELEMENT_MAIN].attr.bits.state == CARD_MENU_ELEMENT_OPEN && g_card_menu_element_pool[CARD_MENU_ELEMENT_MAIN].attr.bits.transition_step == 0)
    {
        card_menu_update_card_sequence();
    }
    if ((u16)g_pad_input == PAD_ALL_BUTTONS)
    {
        g_pad_input = 0;
    }
    card_menu_handle_input();
    if (g_card_menu_scroll_frames != 0)
    {
        g_card_menu_scroll_y += (g_card_menu_scroll_target_y - g_card_menu_scroll_y) / g_card_menu_scroll_frames--;
    }
    else
    {
        g_card_menu_scroll_y = g_card_menu_scroll_target_y;
    }
}
