#include "gosub_internal.h"

/**
 * @brief Build the list, header, and detail elements for screen 9.
 * @see decomp.me (100%)
 */
void gosub_build_screen_9_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.y = 0x38;
    element->geometry.f.width_high = 0;
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0xE8);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_two_line_header;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.y = 0x10;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 8);

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_row_description;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.y = 0xB0;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}

/**
 * @brief Build the list, detail header, and preview elements for screen 10.
 * @see decomp.me (100%)
 */
void gosub_build_screen_10_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.y = 0x48;
    element->geometry.f.width_high = 0;
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0xE8);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_detail_header;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.y = 0xB0;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 8);

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_combination_preview;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.y = 0x20;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}

/**
 * @brief Configure reserved element 0 and reset the dialog choice.
 * @see decomp.me (100%)
 */
void gosub_initialize_fixed_element(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_confirmation_prompt;
    g_gosub_dialog_choice = 0;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x20;
    element->attr.f.y = 0x70;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 0);
}

/**
 * @brief Build the three elements of the gosub screens entered by arms 2 to 5.
 * @see decomp.me (100%)
 */
void gosub_build_category_screen_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.y = 0x28;
    element->geometry.f.width_high = 0;
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0xE8);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_row_description;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.y = 0xB0;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 8);

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.y = 0x10;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}

/**
 * @brief Build the elements of the gosub screens entered by arms 0, 1, 6-8,
 *        15, 16 and 19.
 * @param include_middle Non-zero to include the middle element, zero to skip it.
 * @see decomp.me (100%)
 */
void gosub_build_list_screen_elements(s32 include_middle)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.y = 0x28;
    element->geometry.f.width_high = 0;
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0xE8);
    g_gosub_selection_count = 0;

    if (include_middle != 0)
    {
        element = gosub_allocate_element();
        element->draw_handler = (void*)&gosub_draw_row_description;
        element->attr.f.transition_step = 1;
        element->attr.f.x = 0x1C;
        element->attr.f.y = 0xB0;
        element->geometry.f.width_high = 1;
        element->geometry.f.height = 0x14;
        SET_ELEMENT_WIDTH_LOW(element, 8);
    }

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.y = 0x10;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}

/**
 * @brief Build the three elements of the gosub screen entered by arm 11.
 * @see decomp.me (100%)
 */
void gosub_build_screen_11_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.y = 0x28;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0x20);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_row_description;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.y = 0xB0;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 8);

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.y = 0x10;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}

/**
 * @brief Build the two elements of the gosub screens entered by arms 12-14 and
 *        17-18.
 * @see decomp.me (100%)
 */
void gosub_build_compact_list_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.y = 0x30;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0x18);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.y = 0x10;
    element->geometry.f.width_high = 1;
    element->geometry.f.height = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}
