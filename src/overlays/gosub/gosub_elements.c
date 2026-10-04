#include "internal/gosub_internal.h"

/**
 * @brief Build the golem parts screen: the equipment list, the instructions and the row description.
 */
void gosub_build_golem_parts_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = SCREEN_WIDTH / 2 - g_gosub_window_width / 2;
    element->attr.f.y = 56;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_LIST_PANEL_WIDTH);
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + GOSUB_LIST_PANEL_PADDING;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_LIST_PANEL_WIDTH);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_golem_parts_header;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_TEXT_PANEL_X;
    element->attr.f.y = GOSUB_TITLE_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_TEXT_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_TWO_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_TEXT_PANEL_WIDTH);

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_row_description;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_TEXT_PANEL_X;
    element->attr.f.y = GOSUB_DESCRIPTION_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_TEXT_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_TWO_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_TEXT_PANEL_WIDTH);
}

/**
 * @brief Build the logic-block screen: the component list, the instructions and the block preview.
 */
void gosub_build_block_components_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = SCREEN_WIDTH / 2 - g_gosub_window_width / 2;
    element->attr.f.y = 72;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_LIST_PANEL_WIDTH);
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + GOSUB_LIST_PANEL_PADDING;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_LIST_PANEL_WIDTH);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_block_components_header;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_TEXT_PANEL_X;
    element->attr.f.y = GOSUB_DESCRIPTION_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_TEXT_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_TWO_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_TEXT_PANEL_WIDTH);

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_block_preview;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_TEXT_PANEL_X;
    element->attr.f.y = 0x20;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_TEXT_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_TWO_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_TEXT_PANEL_WIDTH);
}

/**
 * @brief Open the Yes/No confirmation dialog in reserved element 0.
 * @note The other callers inline the copy in gosub_internal.h.
 */
void gosub_open_confirmation_dialog(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_confirmation_prompt;
    g_gosub_dialog_choice = 0;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_MESSAGE_PANEL_X;
    element->attr.f.y = GOSUB_DIALOG_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_MESSAGE_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_TWO_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_MESSAGE_PANEL_WIDTH);
}

/**
 * @brief Build an equipment screen: the item list, the row description and the title.
 */
void gosub_build_equipment_screen_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = SCREEN_WIDTH / 2 - g_gosub_window_width / 2;
    element->attr.f.y = 40;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_LIST_PANEL_WIDTH);
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + GOSUB_LIST_PANEL_PADDING;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_LIST_PANEL_WIDTH);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_row_description;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_TEXT_PANEL_X;
    element->attr.f.y = GOSUB_DESCRIPTION_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_TEXT_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_TWO_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_TEXT_PANEL_WIDTH);

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_TEXT_PANEL_X;
    element->attr.f.y = GOSUB_TITLE_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_TEXT_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_ONE_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_TEXT_PANEL_WIDTH);
}

/**
 * @brief Build an item or equipment type screen: the list, the optional row description and the title.
 * @param include_middle Nonzero to show the row description panel.
 */
void gosub_build_list_screen_elements(s32 include_middle)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = SCREEN_WIDTH / 2 - g_gosub_window_width / 2;
    element->attr.f.y = 40;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_LIST_PANEL_WIDTH);
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + GOSUB_LIST_PANEL_PADDING;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_LIST_PANEL_WIDTH);
    g_gosub_selection_count = 0;

    if (include_middle != 0)
    {
        element = gosub_allocate_element();
        element->draw_handler = (void*)&gosub_draw_row_description;
        element->attr.f.transition_step = 1;
        element->attr.f.x = GOSUB_TEXT_PANEL_X;
        element->attr.f.y = GOSUB_DESCRIPTION_PANEL_Y;
        element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_TEXT_PANEL_WIDTH);
        element->geometry.f.height = GOSUB_ONE_LINE_PANEL_HEIGHT;
        GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_TEXT_PANEL_WIDTH);
    }

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_TEXT_PANEL_X;
    element->attr.f.y = GOSUB_TITLE_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_TEXT_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_ONE_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_TEXT_PANEL_WIDTH);
}

/**
 * @brief Build the logic-block list screen: the list, the block description and the title.
 */
void gosub_build_logic_block_list_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = SCREEN_WIDTH / 2 - g_gosub_window_width / 2;
    element->attr.f.y = 40;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_LOGIC_BLOCK_PANEL_WIDTH);
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + GOSUB_LIST_PANEL_PADDING;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_LOGIC_BLOCK_PANEL_WIDTH);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_row_description;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_TEXT_PANEL_X;
    element->attr.f.y = GOSUB_DESCRIPTION_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_TEXT_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_ONE_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_TEXT_PANEL_WIDTH);

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_TEXT_PANEL_X;
    element->attr.f.y = GOSUB_TITLE_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_TEXT_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_ONE_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_TEXT_PANEL_WIDTH);
}

/**
 * @brief Build a pet or golem screen: the list and the title.
 */
void gosub_build_companion_list_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = SCREEN_WIDTH / 2 - g_gosub_window_width / 2;
    element->attr.f.y = 48;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_COMPANION_PANEL_WIDTH);
    element->geometry.f.height = g_gosub_row_height * g_gosub_visible_row_count + GOSUB_LIST_PANEL_PADDING;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_COMPANION_PANEL_WIDTH);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_TEXT_PANEL_X;
    element->attr.f.y = GOSUB_TITLE_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_TEXT_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_ONE_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_TEXT_PANEL_WIDTH);
}
