#include "overlays/field/field_text.h"
#include "internal/niki_internal.h"
#include "overlays/menu/menu.h"

void niki_init_card_events(void);
void* niki_draw_progress_bar(POLY_G4* quad, u_long* ot);
u8* niki_draw_choice_prompt(u8* prim, u_long* ot, s32 x, s32 y);

/**
 * @brief Initialize card browsing, drawing resources, and the selected menu mode.
 * @param context_value Caller value retained for the overlay; its meaning is unresolved.
 * @param mode Menu mode, with zero selecting the entry browser.
 */
void niki_init(s32 context_value, s32 mode)
{
    RECT rect;

    g_niki_mode = mode;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    g_card_slot = 0;
    niki_reset_entry_ranks();
    niki_init_card_events();
    g_niki_icon_phase = 0;
    field_set_default_fade_target();
    rect.x = OVERLAY_INIT_CLEAR_VRAM_X;
    rect.y = OVERLAY_INIT_CLEAR_VRAM_Y;
    rect.w = OVERLAY_INIT_CLEAR_VRAM_W;
    rect.h = OVERLAY_INIT_CLEAR_VRAM_H;
    ClearImage(&rect, 0, 0, 0);
    reset_glyph_cache();
    g_niki_progress_active = 0;
    g_niki_confirm_latch = 0;
    g_niki_selection_status = 0;
    g_niki_io_busy = 0;
    g_niki_frame_parity = 0;
    g_niki_exit_requested = 0;
    field_reset_input_repeat();
    niki_build_ui_elements();
    D_80164AE4 = context_value;
}

/**
 * @brief Draw and update one menu frame, or finish a requested exit.
 * @param frame Draw context receiving this frame's GPU packets.
 * @return One when the menu has exited, otherwise zero.
 */
s32 niki_update_frame(FieldRenderHalf* frame)
{
    if (g_niki_exit_requested != 0)
    {
        shutdown_card_events();
        field_text_reset_windows();
        DrawSync(0);
        return 1;
    }
    field_text_reset_scratch();
    begin_glyph_cache_frame();
    niki_update_menu(frame);
    evict_unused_glyphs();
    field_text_upload_immediate_cache();
    g_niki_frame_parity ^= 1;
    return 0;
}

/** @brief Reset selection and create the windows for the active menu mode. */
void niki_build_ui_elements(void)
{
    CardMenuElement* element;
    g_niki_scroll_frames = 0;
    g_niki_scroll_target_y = 0;
    g_niki_scroll_y = 0;
    g_niki_selected_row = 0;
    g_niki_selection_status = 0;
    g_niki_items = g_saved_game_ctx->items;
    if (0)
    {
        niki_clear_elements(0, 0, 0, 0, 0);
    }
    niki_clear_elements();
    D_80164B80 = 0;
    if (g_niki_mode != 0)
    {
        g_niki_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_OPENING;
        element = niki_alloc_element();
        element->draw = niki_draw_state_page;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARD_MENU_MESSAGE_X;
        element->attr.bits.y = CARD_MENU_EXCHANGE_MESSAGE_Y;
        element->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
        element->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_MESSAGE_WIDTH);

        element = niki_alloc_element();
        element->draw = niki_draw_card_slot0_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT0_LABEL_X;
        element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_TRANSFER_Y;
        element->size.bits.width_high = 0;
        element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

        element = niki_alloc_element();
        element->draw = niki_draw_card_slot1_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT1_LABEL_X;
        element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_TRANSFER_Y;
        element->size.bits.width_high = 0;
        element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
        CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);
        g_niki_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
        return;
    }

    g_niki_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_OPENING;
    element = niki_alloc_element();
    element->draw = niki_draw_entry_list;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_LIST_X;
    element->attr.bits.y = CARD_MENU_LIST_Y;
    element->size.bits.width_high = CARD_MENU_EXCHANGE_LIST_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_LIST_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_EXCHANGE_LIST_WIDTH);

    element = niki_alloc_element();
    element->draw = niki_draw_header_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_TITLE_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_TITLE_Y;
    element->size.bits.width_high = CARD_MENU_EXCHANGE_TITLE_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_EXCHANGE_TITLE_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_EXCHANGE_TITLE_WIDTH);

    element = niki_alloc_element();
    element->draw = niki_draw_card_slot0_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT0_LABEL_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_BROWSER_Y;
    element->size.bits.width_high = 0;
    element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

    element = niki_alloc_element();
    element->draw = niki_draw_card_slot1_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_EXCHANGE_CARD_SLOT1_LABEL_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_CARD_LABEL_BROWSER_Y;
    element->size.bits.width_high = 0;
    element->size.bits.height = CARD_MENU_CARD_LABEL_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_CARD_LABEL_WIDTH);

    element = niki_alloc_element();
    element->draw = niki_draw_selected_entry_details;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = CARD_MENU_DETAILS_X;
    element->attr.bits.y = CARD_MENU_DETAILS_Y;
    element->size.bits.width_high = CARD_MENU_DETAILS_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_DETAILS_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_DETAILS_WIDTH);
    g_niki_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
}

/**
 * @brief Draw menu elements, process input and advance scrolling.
 * @param frame Draw context receiving the menu packets.
 */
void niki_update_menu(FieldRenderHalf* frame)
{
    s32 delta;

    niki_update_elements(frame);
    g_niki_icon_phase += 2;
    if ((g_niki_element_pool[1].attr.word & 0x7F) == 2)
    {
        niki_update_load_sequence();
    }
    if ((u16)g_pad_input == 0xFFFF)
    {
        g_pad_input = 0;
    }
    niki_handle_input();
    if (g_niki_scroll_frames != 0)
    {
        s32 base = g_niki_scroll_y;
        delta = (g_niki_scroll_target_y - g_niki_scroll_y) / g_niki_scroll_frames;
        g_niki_scroll_frames -= 1;
        g_niki_scroll_y += delta;
    }
    else
    {
        g_niki_scroll_y = g_niki_scroll_target_y;
    }
}

/**
 * @brief Run immediate sequence steps, then select the next wait or recovery sequence.
 * @return Unspecified; callers ignore the return value.
 */
s32 niki_update_load_sequence(void)
{
    s32 result;

    if (g_card_entry_state >= CARD_MENU_ENTRY_COUNT_LIMIT)
    {
        if (g_card_step == NULL)
        {
            g_card_step = g_niki_card_setup_sequence;
        }
    }

    do
    {
        result = niki_advance_load_sequence();
    } while (result == CARD_MENU_SEQUENCE_RUN_AGAIN);

    if ((D_80164B80 != 0) && (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK))
    {
        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
        g_card_step = g_niki_card_info_sequence;
    }
    else
    {
        switch (result)
        {
        case CARD_MENU_SEQUENCE_NONE:
            break;
        case CARD_MENU_SEQUENCE_NO_CARD:
            g_card_step = g_niki_rescan_sequence;
            D_80164B80 = 0;
            break;
        case CARD_MENU_SEQUENCE_UNFORMATTED:
            g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
            /* fallthrough */
        case CARD_MENU_SEQUENCE_FINISHED:
            g_card_step = g_niki_card_info_sequence;
            break;
        }
    }
}

/**
 * @brief Handle card switching, entry navigation, cancellation, and load confirmation.
 * @return Unspecified; callers ignore the return value.
 * @return Unspecified; callers ignore the return value.
 */
s32 niki_handle_input(void)
{
    s32 entry_count;
    s32 status;
    s32 navigation_steps;
    CardMenuElement* element;

    if ((g_niki_element_pool[1].attr.word & CARD_MENU_ELEMENT_STATE_MASK) == 0)
    {
        g_niki_exit_requested = 1;
        return;
    }
    if (g_niki_exit_requested != 0)
    {
        return;
    }
    if (((s32)g_niki_element_pool[1].attr.word & CARD_MENU_ELEMENT_STATE_MASK) >= 3)
    {
        return;
    }
    if ((g_niki_element_pool[0].attr.word & CARD_MENU_ELEMENT_STATE_MASK) != 0)
    {
        return;
    }
    entry_count = g_card_entry_state;
    if (entry_count == CARD_MENU_ENTRY_STATE_CHECKING_CARD)
    {
        return;
    }
    if (g_niki_entry_scan_active != 0)
    {
        return;
    }
    if (g_niki_io_busy != 0)
    {
        return;
    }
    if ((u32)(*g_card_step - 6) < 2U)
    {
        return;
    }
    if (g_niki_mode != 0)
    {
        return;
    }

    status = g_pad_input;
    if (status & NIKI_CANCEL_INPUT_MASK)
    {
        g_field_niki_addhero_state = 3;
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        niki_close_all_elements();
        return;
    }
    if (status & CARD_MENU_CARD_SWITCH_BUTTON_MASK)
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        niki_switch_card_slot();
        return;
    }
    if (entry_count >= 0x10)
    {
        return;
    }

    navigation_steps = 1;
    if (status & 8)
    {
        g_pad_input = 0x4000;
        navigation_steps = 1;
    }
    if (g_pad_input & 4)
    {
        g_pad_input = 0x1000;
        navigation_steps = 1;
    }

    while (navigation_steps != 0)
    {
        if (g_pad_input & 0x1000)
        {
            g_niki_selected_row -= 1;
            if (g_niki_selected_row < 0)
            {
                g_niki_selected_row = g_card_entry_state - 1;
            }
        }
        if (g_pad_input & 0x4000)
        {
            g_niki_selected_row += 1;
            if (g_niki_selected_row >= g_card_entry_state)
            {
                g_niki_selected_row = 0;
            }
        }
        navigation_steps -= 1;
    }

    if (g_pad_input & 0x5000)
    {
        niki_commit_selected_entry();
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        niki_scroll_to_selection();
        return;
    }

    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        if (g_niki_mode != 0)
        {
            return;
        }
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_niki_selected_row].name, 0xC) == 0)
        {
            SavedGameLayout* metadata = &g_niki_entry_file.saved_game;
            if ((metadata->identity.ids.game_id != g_saved_game_ctx->identity.ids.game_id) && (metadata->summary_slot_count != 0) &&
                ((g_save_compatibility_tag == SAVE_TAG_ANY) || (metadata->compatibility_tag == g_save_compatibility_tag)))
            {
                element = niki_alloc_element();
                element->attr.bits.transition_step = 1;
                element->attr.bits.x = CARD_MENU_MESSAGE_X;
                element->attr.bits.y = CARD_MENU_EXCHANGE_MESSAGE_Y;
                element->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
                element->size.bits.height = CARD_MENU_EXCHANGE_PROMPT_HEIGHT;
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_MESSAGE_WIDTH);
                niki_enable_choice_toggle();
                element->draw = niki_draw_confirm_prompt;
                restart_card_sequence();
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                return;
            }
        }
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    }
}

void niki_switch_card_slot(void)
{
    D_80164B80 = 0;
    g_card_step = 0;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    g_niki_scroll_frames = 0;
    g_niki_scroll_target_y = 0;
    g_niki_scroll_y = 0;
    g_niki_selected_row = 0;
    g_niki_selection_status = 0;
    g_card_slot ^= 1;
    niki_reset_entry_ranks();
}

/** @brief Start the closing animation for every active menu element. */
void niki_close_all_elements(void)
{
    s32 attributes;
    s32 element_index;
    CardMenuElement* element;
    s32 closing_attributes;

    field_restore_fade_target();
    element = g_niki_element_pool;
    element_index = 0;
    for (; element_index < CARD_MENU_ELEMENT_COUNT; element_index++, element++)
    {
        attributes = element->attr.word;
        if (attributes & CARD_MENU_ELEMENT_STATE_MASK)
        {
            closing_attributes = (attributes & ~CARD_MENU_ELEMENT_STATE_MASK) | 3;
            element->attr.word = (closing_attributes & ~NIKI_ELEMENT_TRANSITION_STEP_MASK) | 0x40;
        }
    }
}

/** @brief Scroll the browser over four frames to keep the selected row visible. */
void niki_scroll_to_selection(void)
{
    s32 selected_row;
    s32 half_row_y;
    s32 scroll_y;
    s32 selected_y;
    s32 relative_y;

    selected_row = g_niki_selected_row;
    half_row_y = (selected_row << 3) - selected_row;
    scroll_y = g_niki_scroll_y;
    selected_y = half_row_y << 1;
    relative_y = selected_y - scroll_y;

    if (relative_y >= 75)
    {
        g_niki_scroll_target_y = selected_y - 70;
        g_niki_scroll_frames = 4;
    }
    if (relative_y < 0)
    {
        g_niki_scroll_target_y = selected_y;
        g_niki_scroll_frames = 4;
    }
}

/**
 * @brief Update and draw the active menu elements.
 * @param frame Draw context receiving the element packets.
 */
void niki_update_elements(FieldRenderHalf* frame)
{
    niki_update_and_draw_elements(frame);
}

/**
 * @brief Prepend a GPU packet while retaining each tag's packet-length byte.
 * @param ot Ordering-table entry receiving the packet.
 * @param tag Tag at the start of the packet.
 */
static inline void niki_link_packet(NikiGpuTag* ot, NikiGpuTag* tag)
{
    NIKI_ADD_PRIMITIVE(ot, tag);
}

/**
 * @brief Draw the three-line card-scan status message.
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from every x).
 * @param y_offset Vertical scroll offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 */
static inline u8* niki_draw_scan_message(u_long* ot, u8* prim, s32 x_offset, s32 y_offset)
{
    s32 x;
    u8* glyph_table;

    x = -x_offset + 0x84;
    glyph_table = (u8*)&g_niki_text_table;
    prim = field_draw_text((SPRT*)prim, ot, glyph_table + g_niki_text_table, 4, x, -y_offset, 2);
    prim = field_draw_text((SPRT*)prim, ot, GLYPH_OFF(glyph_table, 0x1E), 4, x, 0xE - y_offset, 2);
    prim = field_draw_text((SPRT*)prim, ot, GLYPH_OFF(glyph_table, 0xB2), 4, x, 0x1C - y_offset, 2);
    return prim;
}

/**
 * @brief Render the niki row/status list: per-entry glyphs, markers and the
 *        highlight tile, dispatched by the g_card_entry_state list-state selector.
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from every x).
 * @param y_offset Vertical scroll offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 */
void* niki_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 state = g_card_entry_state;

    switch (state)
    {
    case CARD_MENU_ENTRY_STATE_NO_GAME_DATA:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_UNFORMATTED:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_CARD_FULL:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_not_enough_blocks, CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_NO_CARD:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_no_card, CARD_MENU_TEXT_NO_CARD), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_card_access_failed, CARD_MENU_TEXT_CARD_ACCESS_FAILED), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_NO_SAVE_DATA:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_no_save_data, CARD_MENU_TEXT_NO_SAVE_DATA), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_BLANK:
        break;
    case CARD_MENU_ENTRY_STATE_CHECKING_CARD:
        prim = niki_draw_scan_message(ot, prim, x_offset, y_offset);
        break;
    default:
    {
        s32 row_y;
        s32 entry_index;

        if (g_niki_entry_scan_active != 0)
        {
            prim = niki_draw_scan_message(ot, prim, x_offset, y_offset);
            break;
        }
        entry_index = 0;
        if (state > 0)
        {
            s32 base_x;
            s32* rank;
            u16 marker_offset;
            Vec2s pos;
            s32 row_top;
            uintptr_t glyph_table;

            glyph_table = (uintptr_t)&g_niki_text_table;
            base_x = -x_offset;
            do
            {
                row_top = ((entry_index * CARD_MENU_ENTRY_ROW_HEIGHT) - y_offset) - g_niki_scroll_y;
                row_y = row_top + 1;
                if ((u32)(row_top + 0xE) < 0x65U)
                {
                    rank = &g_niki_entry_ranks[entry_index];
                    if (*rank >= 0)
                    {
                        pos.x = base_x + CARD_MENU_ENTRY_VALUE_X;
                        pos.y = row_y;
                        prim = field_draw_number(ot, (SPRT*)prim, g_card_entry_suffix_values[entry_index], 4, &pos, 0);
                        prim = field_draw_text((SPRT*)prim, ot, (u8*)(g_niki_text_number_label + glyph_table), 4, base_x + 0x70, row_y, 0);
                        if ((g_niki_rank_count - 1) == *rank)
                        {
                            marker_offset = *(u16*)(glyph_table + 0x36);
                            prim = field_draw_text((SPRT*)prim, ot, (u8*)(marker_offset + glyph_table), 4, base_x + CARD_MENU_EXCHANGE_ENTRY_MARKER_X, row_y, 0);
                        }
                        else if (*rank < 2)
                        {
                            marker_offset = *(u16*)(glyph_table + 0x38);
                            prim = field_draw_text((SPRT*)prim, ot, (u8*)(marker_offset + glyph_table), 4, base_x + CARD_MENU_EXCHANGE_ENTRY_MARKER_X, row_y, 0);
                        }
                        if (*skip_hex_digits(&g_card_entries[g_card_slot][entry_index].name[12]) == '+')
                        {
                            prim = field_draw_text((SPRT*)prim, ot, (u8*)(g_niki_text_plus_marker + glyph_table), 4, CARD_MENU_EXCHANGE_ENTRY_PLUS_RIGHT_X - x_offset, row_y, 1);
                        }
                    }
                    if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 0xC) == 0)
                    {
                        prim = field_draw_text((SPRT*)prim, ot, (u8*)(glyph_table + g_niki_text_mana_label), 4, 1 - x_offset, row_y, 0);
                    }
                    else if (strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 0xC) == 0)
                    {
                        prim = field_draw_text((SPRT*)prim, ot, (u8*)(g_niki_text_ring_ring_land_label + glyph_table), 4, 1 - x_offset, row_y, 0);
                    }
                    else if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, 8) == 0)
                    {
                        prim = field_draw_text((SPRT*)prim, ot, (u8*)(glyph_table + g_niki_text_new_save_label), 4, 1 - x_offset, row_y, 0);
                    }
                    else
                    {
                        prim = field_draw_text((SPRT*)prim, ot, (u8*)(g_niki_text_other_game_label + glyph_table), 4, 1 - x_offset, row_y, 0);
                    }
                }
                entry_index++;
            } while (entry_index < g_card_entry_state);
        }
        row_y = ((g_niki_selected_row * CARD_MENU_ENTRY_ROW_HEIGHT) - y_offset) - g_niki_scroll_y;

        if (g_niki_entry_scan_active == 0)
        {
            NikiTile* tile = (NikiTile*)prim;

            tile->color.word = CARD_MENU_HIGHLIGHT_COLOR;
            tile->tag.bytes.length = 3;
            tile->color.bytes.code = 0x62;
            tile->w = 0x108;
            tile->x0 = 0;
            tile->y0 = row_y;
            tile->h = 0xE;
            niki_link_packet((NikiGpuTag*)ot, &tile->tag);
            prim = (u8*)(tile + 1);
        }
    }
    break;
    }
    return prim;
}

/**
 * @brief Draw the niki header banner glyph, picking one of two captions
 *        according to the g_niki_mode mode selector.
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the banner x).
 * @param y_offset Vertical scroll offset (subtracted from the banner y).
 * @return Advanced primitive-buffer write cursor.
 */
void* niki_draw_header_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    if (g_niki_mode == 1)
    {
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_select_item, CARD_MENU_TEXT_SELECT_ITEM), 4, -x_offset + 0x78, -y_offset, 2);
    }
    else
    {
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_select_save_data, CARD_MENU_TEXT_SELECT_SAVE_DATA), 4, -x_offset + 0x78, -y_offset, 2);
    }
    return prim;
}

/**
 * @brief Draw the first card-slot label, shading it when the other slot is selected.
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the caption x).
 * @param y_offset Vertical scroll offset (subtracted from the caption y).
 * @return Advanced primitive-buffer write cursor.
 */
void* niki_draw_card_slot0_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    NikiTile* tile;

    if (g_card_slot != 0)
    {
        tile = (NikiTile*)prim;
        tile->color.word = CARD_MENU_INACTIVE_LABEL_COLOR;
        tile->tag.bytes.length = 3;
        tile->color.bytes.code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = CARD_MENU_CARD_LABEL_WIDTH;
        tile->h = 0x10;
        tile->tag.word = (tile->tag.word & GPU_TAG_HIGH_MASK) | (*ot & GPU_ADDR_MASK);
        *ot = (*ot & GPU_TAG_HIGH_MASK) | ((uintptr_t)tile & GPU_ADDR_MASK);
        prim += sizeof(NikiTile);
    }
    return field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_card_slot0_label, CARD_MENU_TEXT_CARD_SLOT0_LABEL), 4, -x_offset + CARD_MENU_CARD_LABEL_TEXT_X, -y_offset, 2);
}

/**
 * @brief Draw the second card-slot label, shading it when the other slot is selected.
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the caption x).
 * @param y_offset Vertical scroll offset (subtracted from the caption y).
 * @return Advanced primitive-buffer write cursor.
 */
void* niki_draw_card_slot1_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    NikiTile* tile;

    if (g_card_slot == 0)
    {
        tile = (NikiTile*)prim;
        tile->color.word = CARD_MENU_INACTIVE_LABEL_COLOR;
        tile->tag.bytes.length = 3;
        tile->color.bytes.code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = CARD_MENU_CARD_LABEL_WIDTH;
        tile->h = 0x10;
        tile->tag.word = (tile->tag.word & GPU_TAG_HIGH_MASK) | (*ot & GPU_ADDR_MASK);
        *ot = (*ot & GPU_TAG_HIGH_MASK) | ((uintptr_t)tile & GPU_ADDR_MASK);
        prim += sizeof(NikiTile);
    }

    return field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_card_slot1_label, CARD_MENU_TEXT_CARD_SLOT1_LABEL), 4, -x_offset + CARD_MENU_CARD_LABEL_TEXT_X, -y_offset, 2);
}

/**
 * @brief Draw the niki save-slot detail panel: element glyphs, the playtime
 *        clock, the slot marker row, and a fallback name/second-line block.
 *
 * Runs only while the panel is active (g_niki_selection_status non-zero) and not suppressed
 * (g_niki_entry_scan_active zero). Depending on g_niki_selection_status it either emits a two-line caption
 * (state 2), or renders the full slot detail: up to three party markers laid out
 * by niki_draw_icon_highlight with an animated highlight (g_niki_icon_phase), the playtime split
 * into hours/minutes via field_draw_number, and one of three status glyphs. If the
 * slot compare fails it falls back to drawing the stored name and second line.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from every x).
 * @param y_offset Vertical scroll offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 */
void* niki_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    u8* result;
    Vec2s pos;
    u8 name[0x100];
    s32 icons[3];

    result = prim;
    if (g_niki_selection_status == 0)
    {
        return result;
    }
    if (g_niki_entry_scan_active != 0)
    {
        return result;
    }
    if (g_niki_selection_status != 3 && g_card_entry_state < 0x10)
    {
        if (g_niki_selection_status == 2)
        {
            s32 x = -x_offset;
            u8* base;

            result = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_new_save_title, CARD_MENU_TEXT_NEW_SAVE_TITLE), 4, x, -y_offset, 0);
            base = (u8*)&g_niki_text_new_save_title - 0x28;
            return field_draw_text((SPRT*)result, ot, GLYPH_OFF(base, 0x2A), 4, x, 0x10 - y_offset, 0);
        }
        else
        {
            if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_niki_selected_row].name, 0xC) == 0)
            {
                if (g_save_compatibility_tag == SAVE_TAG_ANY || g_niki_entry_file.saved_game.compatibility_tag == g_save_compatibility_tag)
                {
                    s32 icon_count;
                    s32 visible_icon_index;
                    s32 slot_index;
                    s32 base_icon_width;
                    s32 phase_span;
                    s32 phase_end;
                    s32 phase_start;
                    s32 icon_x;
                    s32 hours;
                    s32 wrapped_phase;

                    {
                        SavedGameLayout* record = &g_niki_entry_file.saved_game;
                        icons[0] = (u32)(record->spawn.word) >> 0x19;
                        icons[1] = (record->track.word >> 0x12) & 0x7F;
                        icons[2] = record->track.word >> 0x19;
                        g_niki_icon_palette = (s32)record->icon_palette;
                    }

                    icon_x = 0;
                    icon_count = 0;
                    for (visible_icon_index = 0; visible_icon_index < 3; visible_icon_index++)
                    {
                        if (icons[visible_icon_index] != 0x7F)
                        {
                            icon_count += 1;
                        }
                    }

                    switch (icon_count)
                    {
                    case 2:
                        base_icon_width = 0x20;
                        phase_span = 0x10;
                        wrapped_phase = g_niki_icon_phase;
                        if (g_niki_icon_phase < 0)
                        {
                            wrapped_phase = g_niki_icon_phase + 0x1F;
                        }
                        g_niki_icon_phase -= (wrapped_phase >> 5) << 5;
                        break;
                    case 3:
                        base_icon_width = 0x10;
                        phase_span = 0x20;
                        g_niki_icon_phase %= 0x60;
                        break;
                    default:
                        base_icon_width = 0x10;
                        phase_span = 0x20;
                        g_niki_icon_phase = 0x1F;
                        break;
                    }

                    visible_icon_index = 0;
                    slot_index = visible_icon_index;
                    for (; slot_index < 3; slot_index++)
                    {
                        phase_start = visible_icon_index * phase_span;
                        phase_end = phase_start + phase_span;
                        if (icons[slot_index] != 0x7F)
                        {
                            s32 icon_width = base_icon_width;
                            s32 wrapped_start;
                            s32 wrapped_end;
                            s32 delta;

                            if (g_niki_icon_phase >= phase_start && g_niki_icon_phase < phase_end)
                            {
                                delta = g_niki_icon_phase - phase_start;
                                icon_width += delta;
                            }
                            else
                            {
                                wrapped_start = phase_end % (phase_span * icon_count);
                                if (g_niki_icon_phase >= wrapped_start && g_niki_icon_phase < (wrapped_end = wrapped_start + phase_span))
                                {
                                    delta = wrapped_end - g_niki_icon_phase;
                                    icon_width += delta;
                                }
                            }
                            result = niki_draw_icon_highlight(result, ot, icon_x - x_offset, -y_offset, icon_width, icons[slot_index], visible_icon_index,
                                                              slot_index);
                            visible_icon_index += 1;
                            icon_x += icon_width;
                        }
                    }

                    {
                        SavedGameLayout* preview = &g_niki_entry_file.saved_game;
                        s32 x = -x_offset;
                        s32 y = -y_offset;
                        s32 playtime;

                        playtime = preview->play_time;
                        pos.x = (s16)(x + CARD_MENU_DETAILS_HOURS_RIGHT_X);
                        pos.y = (s16)y;
                        hours = playtime / 216000;
                        result = field_draw_number(ot, (SPRT*)result, hours, 4, &pos, 1);
                        result = field_draw_text((SPRT*)result, ot, (u8*)(g_text_time_separator_offset_bytes[0] + ((uintptr_t)&g_text_time_separator_offset_bytes - 0x32) + (g_text_time_separator_offset_bytes[1] << 8)), 4,
                                               x + CARD_MENU_DETAILS_TIME_SEPARATOR_X, y, 0);
                        playtime = (playtime / 3600) - (hours * 0x3C);
                        if (playtime < 0xA)
                        {
                            pos.x = (s16)(x + CARD_MENU_DETAILS_MINUTES_TENS_RIGHT_X);
                            pos.y = (s16)y;
                            result = field_draw_number(ot, (SPRT*)result, 0, 4, &pos, 1);
                        }
                        pos.x = (s16)(x + CARD_MENU_DETAILS_MINUTES_RIGHT_X);
                        pos.y = (s16)y;
                        result = field_draw_number(ot, (SPRT*)result, playtime, 4, &pos, 1);
                        result = field_draw_text((SPRT*)result, ot, preview->summary_name, 4, x + CARD_MENU_DETAILS_TEXT_X, y + 0x10, 0);

                        if (preview->identity.ids.game_id == g_saved_game_ctx->identity.ids.game_id)
                        {
                            result = field_draw_text((SPRT*)result, ot, CARD_MENU_TEXT_AT(g_niki_text_same_hero_data, CARD_MENU_TEXT_SAME_HERO_DATA), 4, x + CARD_MENU_DETAILS_TEXT_X, y + 0x20, 0);
                        }
                        else if (preview->summary_slot_count == 0)
                        {
                            result = field_draw_text((SPRT*)result, ot, CARD_MENU_TEXT_AT(g_niki_text_no_items, CARD_MENU_TEXT_NO_ITEMS), 4, x + CARD_MENU_DETAILS_TEXT_X, y + 0x20, 0);
                        }
                        else
                        {
                            result = field_draw_text((SPRT*)result, ot, GLYPH_OFF((u8*)g_niki_location_names, (preview->track.word & 0x3FFFF) * 2), 4,
                                                   x + CARD_MENU_DETAILS_TEXT_X, y + 0x20, 0);
                        }
                    }
                }
                else
                {
                    result = field_draw_text((SPRT*)result, ot, CARD_MENU_TEXT_AT(g_niki_text_wrong_version, CARD_MENU_TEXT_WRONG_VERSION), 4, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 slot_index;
                SaveFileHeader* header;

                terminate_multibyte_text(g_niki_entry_file.header.title);
                header = &g_niki_entry_file.header;
                if ((u32)(header->title[1][0] - 1) >= 0x7FU)
                {
                    for (slot_index = 0; slot_index < 0x20; slot_index++)
                    {
                        name[slot_index] = *(header->title[0] + slot_index);
                    }
                    name[slot_index] = 0;
                    result = draw_cached_text(result, ot, name, -x_offset, -y_offset, 4, 0);

                    for (slot_index = 0; slot_index < 0x20; slot_index++)
                    {
                        name[slot_index] = g_niki_entry_file.header.title[1][slot_index];
                    }
                    name[slot_index] = 0;
                    result = draw_cached_text(result, ot, name, -x_offset, -y_offset + 0x10, 4, 0);
                }
            }
        }
    }
    return result;
}

#include "../../common/save_file/terminate_multibyte_text.inc.c"

/**
 * @brief Draw the niki footer glyph, anchored to the right edge of the panel.
 *
 * Resolves the glyph pointer from the g_field_ui_text_cant_hold_more header (a 16-bit offset stored
 * across bytes [0] and [1], added to the header base less 0xC), then submits it
 * at x = 0x80 - arg2, y = -arg3.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the anchor x).
 * @param y_offset Vertical scroll offset (subtracted from the anchor y).
 * @return Advanced primitive-buffer write cursor.
 */
void* niki_draw_footer_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    return field_draw_text((SPRT*)prim, ot, (void*)((u8*)g_field_ui_text_cant_hold_more - 0xC + g_field_ui_text_cant_hold_more[0] + (g_field_ui_text_cant_hold_more[1] << 8)), 5, 0x80 - x_offset, -y_offset, 2);
}

/**
 * @brief Reset the niki element array: clear the low 3 state bits of each of
 *        the eight g_niki_element_pool entries and reload the g_field_menu_frame_style counter.
 *
 */
void niki_clear_elements(void)
{
    CardMenuElement* element;
    s32 element_index;

    g_field_menu_frame_style = FIELD_MENU_FRAME_STYLE_SUBSCREEN;
    element = g_niki_element_pool;
    for (element_index = 0; element_index < CARD_MENU_ELEMENT_COUNT; element_index++)
    {
        element->attr.word &= ~CARD_MENU_ELEMENT_STATE_MASK;
        element++;
    }
}

/**
 * @brief Claim the first free niki element slot, marking its state bits to 1.
 *
 * Scans the eight g_niki_element_pool entries for one whose low 3 state bits are clear,
 * sets them to 1, and returns it. Falls back to the first entry if none free.
 *
 * @return Pointer to the claimed (or fallback) element.
 */
CardMenuElement* niki_alloc_element(void)
{
    CardMenuElement* element;
    s32 element_index;

    element = g_niki_element_pool;
    for (element_index = 0; element_index < CARD_MENU_ELEMENT_COUNT; element_index++, element++)
    {
        if ((element->attr.word & CARD_MENU_ELEMENT_STATE_MASK) == 0)
        {
            element->attr.word = (element->attr.word & ~CARD_MENU_ELEMENT_STATE_MASK) | 1;
            return element;
        }
    }
    return g_niki_element_pool;
}

/**
 * @brief Animate element windows and append their content and borders to the frame.
 * @param frame_arg Draw context supplying the clip variant and primitive cursor.
 */
void niki_update_and_draw_elements(FieldRenderHalf* frame_arg)
{
    NikiPacketHeader* prim;
    FieldRenderHalf* frame;
    CardMenuElement* element;
    s32 inset_width;
    s32 inset_height;
    s32 element_index;
    s32 entry_count;
    s32 opening_width_low;
    s32 closing_width_low;
    s32 opening_border_x;
    s32 opening_border_width_low;
    s32 closing_border_x;
    s32 closing_border_width_low;
    s32 open_border_width_low;
    DRAWENV draw_area;

    prim = (NikiPacketHeader*)frame_arg->primitive_cursor;
    frame = frame_arg;

    if (frame_arg->display_rect.y != 0)
    {
        SetDefDrawEnv(&draw_area, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 224);
    }
    else
    {
        SetDefDrawEnv(&draw_area, 0, 8, SCREEN_WIDTH, 224);
    }

    element = g_niki_element_pool;
    for (element_index = 0; element_index < CARD_MENU_ELEMENT_COUNT; element_index++, element++)
    {
        if (element->attr.bits.state != 0)
        {
            entry_count = g_card_entry_state;
            if ((entry_count < 16) && (element->draw == niki_draw_entry_list) && (g_niki_element_pool[1].attr.bits.state == 2))
            {
                if (entry_count * 14 > g_niki_scroll_y + 88)
                {
                    prim = (NikiPacketHeader*)field_draw_menu_scroll_arrow(prim, frame->ordering_table, CARD_MENU_EXCHANGE_SCROLL_ARROW_X, CARD_MENU_SCROLL_ARROW_DOWN_Y, FIELD_MENU_ARROW_DOWN);
                }
                if (g_niki_scroll_y != 0)
                {
                    prim = (NikiPacketHeader*)field_draw_menu_scroll_arrow(prim, frame->ordering_table, CARD_MENU_EXCHANGE_SCROLL_ARROW_X, CARD_MENU_SCROLL_ARROW_UP_Y, FIELD_MENU_ARROW_UP);
                }
            }

            SetDrawEnv((DR_ENV*)prim, &draw_area);
            prim->tag = (prim->tag & GPU_TAG_HIGH_MASK) | (frame->ordering_table[0] & GPU_ADDR_MASK);
            frame->ordering_table[0] = ((frame->ordering_table[0] & GPU_TAG_HIGH_MASK) | ((uintptr_t)prim & GPU_ADDR_MASK));
            prim = (NikiPacketHeader*)((DR_ENV*)prim + 1);

            switch (element->attr.bits.state)
            {
            case 1:
                g_pad_input = 0;
                opening_width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                inset_width = (CARD_MENU_ELEMENT_WIDTH(element, opening_width_low) * element->attr.bits.transition_step) / 8;
                inset_height = (element->size.bits.height * element->attr.bits.transition_step) / 8;
                prim = (NikiPacketHeader*)element->draw(frame->ordering_table, (u8*)prim, (CARD_MENU_ELEMENT_WIDTH(element, opening_width_low) - inset_width) / 2,
                                                        (element->size.bits.height - inset_height) / 2);
                opening_border_x = element->attr.bits.x;
                opening_border_width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                prim = (NikiPacketHeader*)field_draw_menu_frame(prim, frame->ordering_table,
                                                        opening_border_x + (CARD_MENU_ELEMENT_WIDTH(element, opening_border_width_low) - inset_width) / 2,
                                                        element->attr.bits.y + (element->size.bits.height - inset_height) / 2, inset_width, inset_height,
                                                        frame_arg->display_rect.y, element_index == 0);
                element->attr.bits.transition_step++;
                if (element->attr.bits.transition_step == 8)
                {
                    field_reset_input_repeat();
                    element->attr.bits.state = 2;
                }
                break;

            case 2:
                prim = (NikiPacketHeader*)element->draw(frame->ordering_table, (u8*)prim, 0, 0);
                open_border_width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                prim =
                    (NikiPacketHeader*)field_draw_menu_frame(prim, frame->ordering_table, element->attr.bits.x, element->attr.bits.y, CARD_MENU_ELEMENT_WIDTH(element, open_border_width_low),
                                                     element->size.bits.height, frame_arg->display_rect.y, element_index == 0);
                if (element->attr.bits.transition_step != 0)
                {
                    element->attr.bits.transition_step--;
                }
                break;

            case 3:
                g_pad_input = 0;
                closing_width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                inset_width = (CARD_MENU_ELEMENT_WIDTH(element, closing_width_low) * element->attr.bits.transition_step) / 8;
                inset_height = (element->size.bits.height * element->attr.bits.transition_step) / 8;
                prim = (NikiPacketHeader*)element->draw(frame->ordering_table, (u8*)prim, (CARD_MENU_ELEMENT_WIDTH(element, closing_width_low) - inset_width) / 2,
                                                        (element->size.bits.height - inset_height) / 2);
                closing_border_x = element->attr.bits.x;
                closing_border_width_low = CARD_MENU_ELEMENT_WIDTH_LOW(element);
                prim = (NikiPacketHeader*)field_draw_menu_frame(prim, frame->ordering_table,
                                                        closing_border_x + (CARD_MENU_ELEMENT_WIDTH(element, closing_border_width_low) - inset_width) / 2,
                                                        element->attr.bits.y + (element->size.bits.height - inset_height) / 2, inset_width, inset_height,
                                                        frame_arg->display_rect.y, element_index == 0);
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.transition_step = 3;
                    element->attr.bits.state = 4;
                }
                break;

            case 4:
                g_pad_input = 0;
                element->attr.bits.transition_step--;
                if (element->attr.bits.transition_step == 0)
                {
                    element->attr.bits.state = 0;
                }
                break;
            }
        }
    }

    frame_arg->primitive_cursor = (u8*)prim;
}

/**
 * @brief Clear the low 3 state bits of the first niki element slot.
 */
void niki_deactivate_primary_element(void)
{
    g_niki_element_pool[0].attr.word &= ~7;
}

#include "../../common/encoded_text/encoded_text_append.inc.c"
#include "../../common/encoded_text/encoded_text_byte_length.inc.c"
#include "../../common/encoded_text/encoded_text_copy.inc.c"

/**
 * @brief Draw the load confirmation choice and dispatch acceptance or cancellation.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the caption x).
 * @param y_offset Vertical scroll offset (subtracted from the caption y).
 * @return Advanced primitive-buffer write cursor.
 */
void* niki_draw_confirm_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    u8* result;
    s32 x;
    s32 status;
    CardMenuElement* element;

    x = -x_offset + 0x90;
    result = niki_draw_choice_prompt(field_draw_text((SPRT*)prim, ot, (u8*)&g_niki_text_load_prompt + g_niki_text_load_prompt - 0x30, 4, x, -y_offset, 2), ot, x,
                                     0xE - y_offset);

    if ((u32)(poll_and_retry_card_info() - 1) < 2U)
    {
        g_niki_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        niki_reset_entry_ranks();
        g_card_step = 0;
    }
    else
    {
        status = g_pad_input;
        if (status & NIKI_CANCEL_INPUT_MASK)
        {
            g_niki_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
            field_reset_input_repeat();
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            g_card_step = g_niki_card_info_sequence;
        }
        else if (status & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            if (g_niki_choice_toggle != 0)
            {
                g_niki_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
                field_reset_input_repeat();
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                g_card_step = g_niki_card_info_sequence;
            }
            else
            {
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                g_niki_confirm_latch = 1;
                g_card_step = g_niki_load_save_sequence;
                element = g_niki_element_pool;
                element->draw = niki_draw_save_confirm_dialog;
                element->attr.bits.transition_step = 1;
                element->attr.bits.state = 1;
                element->attr.bits.x = CARD_MENU_MESSAGE_X;
                element->attr.bits.y = CARD_MENU_EXCHANGE_MESSAGE_Y;
                element->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
                element->size.bits.height = CARD_MENU_MESSAGE_HEIGHT;
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_MESSAGE_WIDTH);
            }
        }
    }
    return result;
}

/**
 * @brief Draw loading progress, then restore a validated save and close the menu.
 *
 * A failed validation opens the status dialog. Once loading finishes successfully,
 * restore the saved fields and trailing data, then start every active window's
 * closing animation before returning to the game.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from every caption x).
 * @param y_offset Vertical scroll offset (subtracted from every caption y).
 * @return Advanced primitive-buffer write cursor.
 */
void* niki_draw_save_confirm_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    u8* base;
    NikiSaveBuffer* resource;
    CardMenuElement* element;
    CardMenuElement* closing_element;
    void* result;
    s32 x;
    s32 element_index;

    x = -x_offset + 0x90;
    result = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_loading, CARD_MENU_TEXT_LOADING), 4, x, -y_offset, 2);
    base = (u8*)&g_niki_text_loading - 0x32;
    result = field_draw_text((SPRT*)result, ot, base + *(u16*)(base + 0x1E), 4, x, 0xE - y_offset, 2);
    result = field_draw_text((SPRT*)result, ot, base + *(u16*)(base + 0xB2), 4, x, 0x1C - y_offset, 2);
    result = niki_draw_progress_bar(result, ot);

    if (g_niki_confirm_latch == 0)
    {
        resource = &g_niki_save_blob;
        element = g_niki_element_pool;
        element->attr.bits.state = 0;
        if (validate_save_file(&resource->save) == 0)
        {
            niki_open_status_dialog(4);
            return result;
        }

        field_play_sound(FIELD_SOUND_LOAD_DONE, AKAO_PAN_CENTER);
        g_field_niki_state = 1;
        D_801227CC = resource->loaded.unknown_0x254;
        D_801227F4 = resource->loaded.unknown_0x256;
        D_8011F418 = g_card_slot;
        strcpy(D_8011F3D8, g_niki_selected_save_path);
        bcopy(resource->loaded.trailing_data, g_field_shared_items, sizeof(resource->loaded.trailing_data));
        field_restore_fade_target();

        closing_element = element;
        for (element_index = 0; element_index < CARD_MENU_ELEMENT_COUNT; element_index++, closing_element++)
        {
            if (closing_element->attr.bits.state != 0)
            {
                closing_element->attr.bits.state = 3;
                closing_element->attr.bits.transition_step = 8;
            }
        }
        field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
    }

    return result;
}

/**
 * @brief Draw the transfer progress bar: a gradient across the top of the message
 *        window that fills over CARD_MENU_PROGRESS_FULL_TICKS VSyncs.
 * @param quad Primitive-buffer cursor the bar is written to.
 * @param ot Ordering-table entry receiving the bar.
 * @return Advanced packet cursor, or the original cursor when the timer is inactive.
 */
inline void* niki_draw_progress_bar(POLY_G4* quad, u_long* ot)
{
    s32 elapsed;
    s32 width;

    if (g_niki_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_niki_progress_start_tick;
        if (elapsed > CARD_MENU_PROGRESS_FULL_TICKS)
        {
            elapsed = CARD_MENU_PROGRESS_FULL_TICKS;
        }
        width = elapsed * CARD_MENU_MESSAGE_WIDTH;
        SET_BGR0_PACKED(quad, CARD_MENU_PROGRESS_TOP_LEFT_COLOR);
        SET_POLY_G4_BGR1_PACKED(quad, CARD_MENU_PROGRESS_TOP_RIGHT_COLOR);
        SET_POLY_G4_BGR3_PACKED(quad, CARD_MENU_PROGRESS_BOTTOM_RIGHT_COLOR);
        SET_POLY_G4_BGR2_PACKED(quad, CARD_MENU_PROGRESS_BOTTOM_LEFT_COLOR);
        setPolyG4(quad);
        quad->x0 = quad->x2 = 0;
        quad->x1 = quad->x3 = width / CARD_MENU_PROGRESS_FULL_TICKS;
        quad->y0 = quad->y1 = 0;
        quad->y2 = quad->y3 = CARD_MENU_MESSAGE_HEIGHT;
        addPrim(ot, quad);
        quad++;
    }
    return quad;
}

/**
 * @brief Open the primary status window and reset the active card operation.
 * @param dialog_state Message selector consumed by the status draw callback.
 */
void niki_open_status_dialog(s32 dialog_state)
{
    field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    g_niki_element_pool[0].draw = niki_draw_status_dialog;
    g_niki_element_pool[0].attr.bits.transition_step = 1;
    g_niki_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_OPENING;
    g_niki_element_pool[0].attr.bits.x = CARD_MENU_DIALOG_X;
    g_niki_element_pool[0].attr.bits.y = CARD_MENU_EXCHANGE_DIALOG_Y;
    g_niki_element_pool[0].size.bits.width_high = CARD_MENU_DIALOG_WIDTH >> 8;
    g_niki_element_pool[0].size.bits.height = CARD_MENU_EXCHANGE_DIALOG_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(&g_niki_element_pool[0], CARD_MENU_DIALOG_WIDTH);
    field_reset_input_repeat();
    g_niki_progress_active = 0;
    g_niki_confirm_latch = 0;
    g_niki_selection_status = 0;
    g_niki_io_busy = 0;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    niki_reset_entry_ranks();
    g_card_step = 0;
    g_niki_dialog_state = dialog_state;
}

/**
 * @brief Open the secondary status window and reset the active card operation.
 * @param dialog_state Message selector consumed by the status draw callback.
 */
void niki_open_secondary_status_dialog(s32 dialog_state)
{
    CardMenuElement* element;

    field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    element = &g_niki_element_pool[1];
    element->draw = niki_draw_secondary_status_dialog;
    element->attr.bits.transition_step = 1;
    element->attr.bits.state = 1;
    element->attr.bits.x = CARD_MENU_DIALOG_X;
    element->attr.bits.y = CARD_MENU_EXCHANGE_DIALOG_Y;
    element->size.bits.width_high = CARD_MENU_DIALOG_WIDTH >> 8;
    element->size.bits.height = CARD_MENU_EXCHANGE_DIALOG_HEIGHT;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, CARD_MENU_DIALOG_WIDTH);
    field_reset_input_repeat();
    g_field_niki_state = 2;
    g_niki_progress_active = 0;
    g_niki_confirm_latch = 0;
    g_niki_selection_status = 0;
    g_niki_io_busy = 0;
    niki_reset_entry_ranks();
    g_card_step = 0;
    g_niki_dialog_state = dialog_state;
}

/**
 * @brief Draw the current status message and dismiss its window on confirmation.
 * @param ot Ordering-table entry receiving the text primitives.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal displacement subtracted from the caption position.
 * @param y_offset Vertical displacement subtracted from the caption position.
 * @return Advanced primitive-buffer write cursor.
 */
void* niki_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    switch (g_niki_dialog_state)
    {
    case 0:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_save_failed, CARD_MENU_TEXT_SAVE_FAILED), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_card_not_inserted, CARD_MENU_TEXT_CARD_NOT_INSERTED), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_not_pocketstation, CARD_MENU_TEXT_NOT_POCKETSTATION), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        g_niki_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Draw the current status message and exit the menu on confirmation.
 * @param ot Ordering-table entry receiving the text primitives.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal displacement subtracted from the caption position.
 * @param y_offset Vertical displacement subtracted from the caption position.
 * @return Advanced primitive-buffer write cursor.
 */
void* niki_draw_secondary_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    CardMenuElement* element;
    s32 element_index;

    switch (g_niki_dialog_state)
    {
    case 0:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_save_failed, CARD_MENU_TEXT_SAVE_FAILED), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_card_not_inserted, CARD_MENU_TEXT_CARD_NOT_INSERTED), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_not_pocketstation, CARD_MENU_TEXT_NOT_POCKETSTATION), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        g_field_menu_frame_style = FIELD_MENU_FRAME_STYLE_SUBSCREEN;
        element = g_niki_element_pool;
        for (element_index = 0; element_index < CARD_MENU_ELEMENT_COUNT; element_index++)
        {
            element->attr.word &= ~CARD_MENU_ELEMENT_STATE_MASK;
            element++;
        }
        field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Upload a save-file icon and append its textured quad to the ordering table.
 * @param prim GPU packet write cursor.
 * @param ot Ordering-table entry receiving the quad.
 * @param x Left edge of the icon.
 * @param y Top edge of the icon.
 * @param width Displayed icon width.
 * @param icon_index Icon resource index; 0x7F skips drawing.
 * @param texture_slot VRAM slot for the icon texture and palette.
 * @param palette_mode Selects the special palette path when one and the icon index is below two.
 * @return Advanced packet cursor, or the original cursor when drawing is skipped.
 */
u8* niki_draw_icon_highlight(u8* prim, u_long* ot, s32 x, s32 y, s32 width, s32 icon_index, s32 texture_slot, s32 palette_mode)
{
    RECT rect;
    POLY_FT4* quad;
    u8 u;

    if (icon_index == SAVE_NO_ICON)
    {
        return prim;
    }

    setRECT(&rect, texture_slot * GPU_CLUT_4BIT_COLORS, VRAM_CLUT_Y, GPU_CLUT_4BIT_COLORS, 1);
    if ((palette_mode == FIELD_PARTY_GUEST) && (icon_index < SAVE_ICON_HERO_COUNT))
    {
        field_copy_portrait_palette(g_niki_icon_context, icon_index);
        LoadImage(&rect, (u_long*)g_niki_icon_context);
        DrawSync(0);
    }
    else if (icon_index >= SAVE_ICON_GOLEM_BASE)
    {
        field_copy_golem_portrait_palette(g_niki_icon_context, g_niki_icon_palette);
        LoadImage(&rect, (u_long*)g_niki_icon_context);
        DrawSync(0);
    }
    else
    {
        LoadImage(&rect, (u_long*)CARD_MENU_ICON_IMAGE(g_niki_icon_offsets, icon_index)->clut);
    }

    setRECT(&rect, texture_slot * (FIELD_PORTRAIT_SIZE / 4) + CARD_MENU_ICON_VRAM_X, CARD_MENU_ICON_VRAM_Y, FIELD_PORTRAIT_SIZE / 4, FIELD_PORTRAIT_SIZE);
    LoadImage(&rect, (u_long*)CARD_MENU_ICON_IMAGE(g_niki_icon_offsets, icon_index)->pixels);

    quad = (POLY_FT4*)prim;
    SET_BGR0_PACKED(quad, GPU_TINT_NEUTRAL);
    setPolyFT4(quad);
    quad->x0 = quad->x2 = x;
    quad->y0 = quad->y1 = y;
    quad->x1 = quad->x3 = x + width;
    quad->y2 = quad->y3 = y + FIELD_PORTRAIT_SIZE - 1;
    u = texture_slot * FIELD_PORTRAIT_SIZE;
    quad->u0 = quad->u2 = u;
    quad->u1 = quad->u3 = u + FIELD_PORTRAIT_SIZE - 1;
    quad->v0 = quad->v1 = CARD_MENU_ICON_VRAM_Y;
    quad->v2 = quad->v3 = CARD_MENU_ICON_VRAM_Y + FIELD_PORTRAIT_SIZE - 1;
    quad->clut = getClut(texture_slot * GPU_CLUT_4BIT_COLORS, VRAM_CLUT_Y);
    quad->tpage = getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_HALF, CARD_MENU_ICON_VRAM_X, 0);
    addPrim(ot, quad);

    return (u8*)(quad + 1);
}

/**
 * @brief Select the default choice (CARD_MENU_CHOICE_DEFAULT) when opening a confirmation prompt.
 */
void niki_enable_choice_toggle(void)
{
    g_niki_choice_toggle = CARD_MENU_CHOICE_DEFAULT;
}

/**
 * @brief Draw both choices and toggle the selection on horizontal input.
 * @param prim GPU packet write cursor.
 * @param ot Ordering-table entry receiving the captions.
 * @param x Horizontal anchor between the choices.
 * @param y Caption baseline.
 * @return Advanced GPU packet cursor.
 */
u8* niki_draw_choice_prompt(u8* prim, u_long* ot, s32 x, s32 y)
{
    u8* p;
    u8* base;
    uintptr_t first_caption;
    uintptr_t second_caption;
    s32 offset_high;
    s32 palette;

    p = (u8*)&g_text_choice_glyph_offsets;
    offset_high = p[1] << 8;
    base = p - 0x36;
    palette = 4;
    first_caption = p[0] + (offset_high + (uintptr_t)base);
    if (g_niki_choice_toggle != 0)
    {
        palette = 5;
    }
    prim = field_draw_text((SPRT*)prim, ot, (u8*)first_caption, palette, x - CARD_MENU_CHOICE_YES_GAP, y, 1);
    palette = 4;
    second_caption = base[0x38] + ((base[0x39] << 8) + (uintptr_t)base);
    if (g_niki_choice_toggle == 0)
    {
        palette = 5;
    }
    prim = field_draw_text((SPRT*)prim, ot, (u8*)second_caption, palette, x + CARD_MENU_CHOICE_NO_GAP, y, 0);
    if (g_pad_input & CARD_MENU_CHOICE_BUTTON_MASK)
    {
        g_niki_choice_toggle ^= 1;
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        g_pad_input = 0;
    }
    return prim;
}

/**
 * @brief Access the extended metadata in the preview transfer buffer.
 * @return Preview metadata record.
 */
static inline SavedGameLayout* niki_preview_metadata(void)
{
    return &g_niki_entry_file.saved_game;
}

/**
 * @brief Draw the save-menu page and process its confirmation and progress states.
 * @param ot Ordering-table entry receiving the page primitives.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal displacement subtracted from glyph positions.
 * @param y_offset Vertical displacement subtracted from glyph positions.
 * @return Advanced GPU packet cursor.
 */
void* niki_draw_state_page(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 dispatch;
    dispatch = g_card_entry_state;
    switch (dispatch)
    {
    case CARD_MENU_ENTRY_STATE_NO_GAME_DATA:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_UNFORMATTED:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_CHECKING_CARD:
    {
        s32 x;
        u8* base;
        x = -x_offset + 0x90;
        base = (u8*)&g_niki_text_table;
        prim = field_draw_text((SPRT*)prim, ot, base + g_niki_text_table, 4, x, -y_offset, 2);
        prim = field_draw_text((SPRT*)prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = field_draw_text((SPRT*)prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
    }
    break;
    case CARD_MENU_ENTRY_STATE_CARD_FULL:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_no_game_save_data, CARD_MENU_TEXT_NO_GAME_SAVE_DATA), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_NO_CARD:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_no_card, CARD_MENU_TEXT_NO_CARD), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_card_access_failed, CARD_MENU_TEXT_CARD_ACCESS_FAILED), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case CARD_MENU_ENTRY_STATE_NO_SAVE_DATA:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_no_save_data, CARD_MENU_TEXT_NO_SAVE_DATA), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xf7:
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_no_load_file, CARD_MENU_TEXT_NO_LOAD_FILE), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xf6:
    {
        s32 x;
        u8* base;
        s32 dialog_state;

        x = -x_offset + 0x90;
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_loading, CARD_MENU_TEXT_LOADING), 4, x, -y_offset, 2);
        base = (u8*)&g_niki_text_loading - 0x32;
        prim = field_draw_text((SPRT*)prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = field_draw_text((SPRT*)prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);

        prim = niki_draw_progress_bar(prim, ot);

        if (g_niki_confirm_latch == 0)
        {
            if (validate_save_file(&g_niki_save_blob.save) == 0)
            {
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                g_niki_element_pool[0].draw = niki_draw_status_dialog;
                g_niki_element_pool[0].attr.bits.transition_step = 1;
                g_niki_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_OPENING;
                g_niki_element_pool[0].attr.bits.x = CARD_MENU_DIALOG_X;
                g_niki_element_pool[0].attr.bits.y = CARD_MENU_EXCHANGE_DIALOG_Y;
                g_niki_element_pool[0].size.bits.width_high = CARD_MENU_DIALOG_WIDTH >> 8;
                g_niki_element_pool[0].size.bits.height = CARD_MENU_EXCHANGE_DIALOG_HEIGHT;
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(&g_niki_element_pool[0], CARD_MENU_DIALOG_WIDTH);
                field_reset_input_repeat();
                g_niki_progress_active = 0;
                g_niki_selection_status = 0;
                g_niki_io_busy = 0;
                g_niki_confirm_latch = 0;
                g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
                niki_reset_entry_ranks();
                dialog_state = 4;
                g_card_step = 0;
                g_niki_dialog_state = dialog_state;
                return prim;
            }
            field_play_sound(FIELD_SOUND_LOAD_DONE, AKAO_PAN_CENTER);
            g_card_entry_state = 0xF4;
            g_niki_choice_toggle = CARD_MENU_CHOICE_DEFAULT;
            field_reset_input_repeat();
        }
    }
    break;
    case 0xf3:
    {
        s32 x;
        u8* result;
        s32 y;
        u8* p;
        u8* base;
        uintptr_t first_caption;
        uintptr_t second_caption;
        s32 offset_high;
        s32 palette;
        CardMenuElement* packet;
        s32 i;

        x = -x_offset;
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_trade_data_not_saved, CARD_MENU_TEXT_TRADE_DATA_NOT_SAVED), 4, x + 0x90, -y_offset, 2);
        y = 0xE - y_offset;
        p = (u8*)&g_text_choice_glyph_offsets;
        offset_high = p[1] << 8;
        base = p - 0x36;
        palette = 4;
        first_caption = p[0] + (offset_high + (uintptr_t)base);
        if (g_niki_choice_toggle != 0)
        {
            palette = 5;
        }
        result = field_draw_text((SPRT*)prim, ot, (u8*)first_caption, palette, x + 0x80, y, 1);
        palette = 4;
        second_caption = base[0x38] + ((base[0x39] << 8) + (uintptr_t)base);
        if (g_niki_choice_toggle == 0)
        {
            palette = 5;
        }
        result = field_draw_text((SPRT*)result, ot, (u8*)second_caption, palette, x + 0x98, y, 0);
        if (g_pad_input & CARD_MENU_CHOICE_BUTTON_MASK)
        {
            g_niki_choice_toggle ^= 1;
            field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
            g_pad_input = 0;
        }

        prim = result;

        if (g_pad_input & NIKI_CANCEL_INPUT_MASK)
        {
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            g_niki_choice_toggle = CARD_MENU_CHOICE_DEFAULT;
            g_card_entry_state = 0xF4;
            field_reset_input_repeat();
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            if (g_niki_choice_toggle != 0)
            {
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                g_niki_choice_toggle = CARD_MENU_CHOICE_DEFAULT;
                g_card_entry_state = 0xF4;
                field_reset_input_repeat();
            }
            else
            {
                field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
                packet = g_niki_element_pool;
                g_field_niki_state = 2;
                g_field_menu_frame_style = FIELD_MENU_FRAME_STYLE_SUBSCREEN;
                for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++, packet++)
                {
                    packet->attr.bits.state = 0;
                }
                field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
                field_reset_input_repeat();
            }
        }
    }
    break;
    case 0xf4:
    {
        s32 x;
        u8* result;
        s32 y;
        u8* caption_table;
        u8* p;
        u8* base;
        uintptr_t first_caption;
        uintptr_t second_caption;
        s32 offset_high;
        s32 palette;
        s32 record_count;
        s32 record_index;
        u8(*records)[64];
        NikiSaveBuffer* resource;
        s32 checksum;

        x = -x_offset;
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_found_load_file, CARD_MENU_TEXT_FOUND_LOAD_FILE), 4, x + 0x90, -y_offset, 2);
        caption_table = (u8*)&g_niki_text_found_load_file - 0x6A;
        prim = field_draw_text((SPRT*)prim, ot, GLYPH_OFF(caption_table, 0x70), 4, x + 0x90, 0xE - y_offset, 2);

        y = 0x1C - y_offset;
        p = (u8*)&g_text_choice_glyph_offsets;
        offset_high = p[1] << 8;
        base = p - 0x36;
        palette = 4;
        first_caption = p[0] + (offset_high + (uintptr_t)base);
        if (g_niki_choice_toggle != 0)
        {
            palette = 5;
        }
        result = field_draw_text((SPRT*)prim, ot, (u8*)first_caption, palette, x + 0x80, y, 1);
        palette = 4;
        second_caption = base[0x38] + ((base[0x39] << 8) + (uintptr_t)base);
        if (g_niki_choice_toggle == 0)
        {
            palette = 5;
        }
        result = field_draw_text((SPRT*)result, ot, (u8*)second_caption, palette, x + 0x98, y, 0);
        if (g_pad_input & CARD_MENU_CHOICE_BUTTON_MASK)
        {
            g_niki_choice_toggle ^= 1;
            field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
            g_pad_input = 0;
        }

        prim = result;

        if ((g_pad_input & NIKI_CANCEL_INPUT_MASK) || ((g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK) && g_niki_choice_toggle != 0))
        {
            g_niki_choice_toggle = CARD_MENU_CHOICE_DEFAULT;
            g_card_entry_state = 0xF3;
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            field_reset_input_repeat();
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
            records = (u8(*)[64])g_field_shared_items;
            resource = &g_niki_save_blob;
            bcopy(g_field_shared_items, resource->loaded.trailing_data, sizeof(resource->loaded.trailing_data));
            record_count = 0;
            for (record_index = 0; record_index < 4; record_index++)
            {
                if (records[record_index][0] != 0)
                {
                    record_count++;
                }
            }
            resource->loaded.trailing_record_count = record_count;
            checksum = compute_save_checksum(&resource->save);
            resource->save.magic = SAVE_FILE_MAGIC;
            resource->save.checksum = checksum;
            g_niki_progress_active = 1;
            g_card_step = g_niki_write_save_sequence;
            g_card_entry_state = 0xF5;
        }
    }
    break;
    case 0xf5:
    {
        s32 x;
        u8* base;
        CardMenuElement* packet;
        s32 i;

        x = -x_offset + 0x90;
        prim = field_draw_text((SPRT*)prim, ot, CARD_MENU_TEXT_AT(g_niki_text_saving, CARD_MENU_TEXT_SAVING), 4, x, -y_offset, 2);
        base = (u8*)&g_niki_text_saving - 0x1C;
        prim = field_draw_text((SPRT*)prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = field_draw_text((SPRT*)prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);

        prim = niki_draw_progress_bar(prim, ot);

        if (g_niki_progress_active == 0)
        {
            field_play_sound(FIELD_SOUND_SAVE_DONE, AKAO_PAN_CENTER);
            g_field_menu_frame_style = FIELD_MENU_FRAME_STYLE_SUBSCREEN;
            packet = g_niki_element_pool;
            for (i = 0; i < CARD_MENU_ELEMENT_COUNT; i++, packet++)
            {
                packet->attr.bits.state = 0;
            }
            field_restore_fade_target_with_duration(CARD_MENU_EXIT_FADE_FRAMES);
            g_field_niki_state = 0;
        }
    }
    break;
    default:
    {
        s32 x;
        u8* base;
        s32 pos;
        s32 diff;

        x = -x_offset + 0x90;
        base = (u8*)&g_niki_text_table;
        prim = field_draw_text((SPRT*)prim, ot, base + g_niki_text_table, 4, x, -y_offset, 2);
        prim = field_draw_text((SPRT*)prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = field_draw_text((SPRT*)prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);

        if (g_niki_entry_scan_active == 0)
        {
            if (g_niki_io_busy != 0)
            {
                return prim;
            }
            if ((u32)(*g_card_step - 6) < 2U)
            {
                return prim;
            }
            if ((strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_niki_selected_row].name, 0xC) != 0) ||
                (niki_preview_metadata()->identity.ids.game_id != D_801227CC) || (niki_preview_metadata()->identity.ids.save_id != D_801227F4))
            {
                g_niki_selected_row++;
                if (g_niki_selected_row >= g_card_entry_state)
                {
                    if (g_card_entry_state != 0)
                    {
                        g_card_entry_state = 0xF7;
                    }
                    else
                    {
                        g_card_entry_state = CARD_MENU_ENTRY_STATE_NO_GAME_DATA;
                    }
                }
                else
                {
                    niki_commit_selected_entry();
                    pos = g_niki_selected_row * 0xE;
                    diff = pos - g_niki_scroll_y;
                    if (diff >= 0x4B)
                    {
                        g_niki_scroll_target_y = pos - 0x46;
                        g_niki_scroll_frames = 4;
                    }
                    if (diff < 0)
                    {
                        g_niki_scroll_target_y = pos;
                        g_niki_scroll_frames = 4;
                    }
                }
            }
            else
            {
                g_niki_progress_start_tick = VSync(-1);
                g_niki_confirm_latch = 1;
                g_card_step = g_niki_read_saved_copy_sequence;
                g_card_entry_state = 0xF6;
            }
        }
    }
    break;
    case CARD_MENU_ENTRY_STATE_BLANK:
        break;
    }

    if (g_niki_io_busy != 0)
    {
        return prim;
    }
    if (g_card_entry_state == 0xF6)
    {
        return prim;
    }
    if (g_card_entry_state == 0xF5)
    {
        return prim;
    }
    if (g_card_entry_state == 0xF4)
    {
        return prim;
    }
    if (g_card_entry_state == 0xF3)
    {
        return prim;
    }

    if (g_pad_input & NIKI_CANCEL_INPUT_MASK)
    {
        CardMenuElement* element;
        s32 i;
        s32 word;
        g_field_niki_addhero_state = 3;
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        field_restore_fade_target();
        element = g_niki_element_pool;
        i = 0;
        do
        {
            word = element->attr.word;
            if (word & 7)
            {
                element->attr.word = (((word & ~7) | 3) & ~NIKI_ELEMENT_TRANSITION_STEP_MASK) | 0x40;
            }
            i++;
            element++;
        } while (i < CARD_MENU_ELEMENT_COUNT);
        return prim;
    }

    if ((g_pad_input & CARD_MENU_CARD_SWITCH_BUTTON_MASK) && (g_card_entry_state != CARD_MENU_ENTRY_STATE_CHECKING_CARD))
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        D_80164B80 = 0;
        g_card_step = 0;
        g_niki_scroll_frames = 0;
        g_niki_scroll_target_y = 0;
        g_niki_scroll_y = 0;
        g_niki_selected_row = 0;
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        g_niki_selection_status = 0;
        g_card_slot ^= 1;
        niki_reset_entry_ranks();
        g_niki_progress_bar_active = 0;
        g_card_step = g_niki_card_setup_sequence;
    }

    return prim;
}

#include "../../common/save_file/skip_hex_digits.inc.c"

/** @brief Full-width MAX text used when a decimal value exceeds five digits. */
const SjisDecimalOverflowText g_decimal_overflow_text __attribute__((aligned(4))) = {{0x82, 0x6C, 0x82, 0x60, 0x82, 0x77, 0}};

#include "../../common/save_file/validate_save_file.inc.c"
#include "../../common/save_file/compute_save_checksum.inc.c"
#include "../../common/sjis/format_decimal.inc.c"
#include "../../common/save_file/format_hex.inc.c"
#include "../../common/save_file/hex_nibble_to_ascii.inc.c"
#include "../../common/save_file/parse_hex.inc.c"
#include "../../common/save_file/parse_hex_suffix_byte.inc.c"
#include "../../common/card_directory/parse_entry_fields.inc.c"

/**
 * @brief Rank recognized entries and select the entry with the greatest field value.
 * @return Index of the greatest field value, or zero when none is present.
 */
s32 niki_rank_entries(void)
{
    s32 entry_index;
    s32 previous_index;
    s32 higher_count;
    s32 next_rank;
    s32 maximum;
    s32 max_suffix;

    parse_entry_fields();
    maximum = -1;
    niki_sort_entries_by_type();
    max_suffix = parse_entry_fields();
    niki_reset_entry_ranks();
    next_rank = 1;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (g_card_entry_fields[g_card_slot][entry_index] >= 0)
        {
            if (g_card_entry_fields[g_card_slot][entry_index] >= maximum)
            {
                g_niki_entry_ranks[entry_index] = next_rank;
                maximum = g_card_entry_fields[g_card_slot][entry_index];
                next_rank++;
            }
            else
            {
                higher_count = 0;
                for (previous_index = 0; previous_index < entry_index; previous_index++)
                {
                    if (g_card_entry_fields[g_card_slot][entry_index] < g_card_entry_fields[g_card_slot][previous_index])
                    {
                        higher_count++;
                        g_niki_entry_ranks[previous_index]++;
                    }
                }
                g_niki_entry_ranks[entry_index] = next_rank - higher_count;
                next_rank++;
            }
        }
    }
    g_niki_rank_count = next_rank;
    /* Reuse next_rank as the running maximum and maximum as its index. */
    next_rank = -1;
    maximum = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (next_rank < g_card_entry_fields[g_card_slot][entry_index])
        {
            next_rank = g_card_entry_fields[g_card_slot][entry_index];
            maximum = entry_index;
        }
    }
    g_niki_entry_value_limit = next_rank + 1;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(&g_new_save_entry_prefix[0], g_card_entries[g_card_slot][entry_index].name, 8) == 0)
        {
            g_card_entry_suffix_values[entry_index] = max_suffix + 1;
            break;
        }
    }
    return maximum;
}

/** @brief Mark all fifteen rank slots unused and reset the rank-count sentinel. */
void niki_reset_entry_ranks(void)
{
    s32 rank_index;
    s32 unused_rank;

    g_niki_rank_count = 0x28;
    unused_rank = -1;
    for (rank_index = 14; rank_index >= 0; rank_index--)
    {
        g_niki_entry_ranks[rank_index] = unused_rank;
    }
}

/**
 * @brief Check whether the selected card contains either recognized save-file prefix.
 * @return One if a recognized entry exists, otherwise zero.
 */
s32 niki_has_known_entry_type(void)
{
    s32 entry_index;

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 12) == 0 ||
            strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, 12) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Check whether directory entries occupy at least fourteen memory-card blocks.
 * @return One when the block limit is reached, otherwise zero.
 */
s32 niki_entry_blocks_reach_limit(void)
{
    s32 entry_index;
    s32 total_blocks;

    total_blocks = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        total_blocks += g_card_entries[g_card_slot][entry_index].size / CARD_BLOCK_BYTES;
    }
    return total_blocks >= 14;
}

/** @brief Remove both placeholder save files from the selected memory card. */
void niki_remove_placeholder_saves(void)
{
    CardFilePath path;

    memcpy(&path, &g_niki_file_template, CARD_DEVICE_BYTES);
    path.device.characters.slot += (u8)g_card_slot;
    strcat(path.text, g_lom_save_dummy_filename);
    erase(path.text);

    memcpy(&path, &g_niki_file_template, CARD_DEVICE_BYTES);
    path.device.characters.slot += (u8)g_card_slot;
    strcat(path.text, g_lom_pocketstation_dummy_filename);
    erase(path.text);
}

/** @brief Device prefix used to construct memory-card file paths. */
const CardPathTemplate g_niki_file_template = {"bu00:"};
