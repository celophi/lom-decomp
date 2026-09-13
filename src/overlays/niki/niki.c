#include "niki_internal.h"

/**
 * @brief Initialize card browsing, drawing resources, and the selected menu mode.
 * @param context_value Caller value retained for the overlay; its meaning is unresolved.
 * @param mode Menu mode, with zero selecting the entry browser.
 */
void niki_init(s32 context_value, s32 mode)
{
    RECT rect;

    g_niki_mode = mode;
    g_niki_entry_state = 0xFF;
    g_niki_card_slot = 0;
    niki_reset_entry_ranks();
    niki_init_stream_handles();
    g_niki_icon_phase = 0;
    func_80067F8C();
    rect.x = OVERLAY_INIT_CLEAR_VRAM_X;
    rect.y = OVERLAY_INIT_CLEAR_VRAM_Y;
    rect.w = OVERLAY_INIT_CLEAR_VRAM_W;
    rect.h = OVERLAY_INIT_CLEAR_VRAM_H;
    func_8001990C(&rect, 0, 0, 0);
    niki_reset_glyph_cache();
    g_niki_progress_active = 0;
    g_niki_confirm_latch = 0;
    g_niki_selection_status = 0;
    g_niki_io_busy = 0;
    g_niki_frame_parity = 0;
    g_niki_exit_requested = 0;
    func_800AA02C();
    niki_build_ui_elements();
    D_80164AE4 = context_value;
}

/**
 * @brief Draw and update one menu frame, or finish a requested exit.
 * @param frame Draw context receiving this frame's GPU packets.
 * @return One when the menu has exited, otherwise zero.
 */
s32 niki_update_frame(NikiFrameState* frame)
{
    if (g_niki_exit_requested != 0)
    {
        niki_shutdown_stream_handles();
        field_text_reset_windows();
        func_80019788(0);
        return 1;
    }
    field_text_reset_scratch();
    niki_begin_glyph_cache_frame();
    niki_update_menu(frame);
    niki_evict_unused_glyphs();
    func_80063194();
    g_niki_frame_parity ^= 1;
    return 0;
}

/** @brief Reset selection and create the windows for the active menu mode. */
void niki_build_ui_elements(void)
{
    NikiElement* element;
    g_niki_scroll_frames = 0;
    g_niki_scroll_target_y = 0;
    g_niki_scroll_y = 0;
    g_niki_selected_row = 0;
    g_niki_selection_status = 0;
    D_80164ADC = (s32)D_8012271C + 0xCE0;
    if (0)
    {
        niki_clear_elements(0, 0, 0, 0, 0);
    }
    niki_clear_elements();
    D_80164B80 = 0;
    if (g_niki_mode != 0)
    {
        g_niki_element_pool[0].attr.f.state = 1;
        element = niki_alloc_element();
        element->draw = niki_draw_state_page;
        element->attr.f.phase = 1;
        element->attr.f.x = 0x10;
        element->attr.f.y = 0x61;
        element->dimensions.f.width_high = 1;
        element->dimensions.f.height = 0x2C;
        NIKI_SET_ELEMENT_WIDTH_LOW(element, 0x20);

        element = niki_alloc_element();
        element->draw = niki_draw_card_slot0_label;
        element->attr.f.phase = 1;
        element->attr.f.x = 0x18;
        element->attr.f.y = 0x4D;
        element->dimensions.f.width_high = 0;
        element->dimensions.f.height = 0x10;
        NIKI_SET_ELEMENT_WIDTH_LOW(element, 0x80);

        element = niki_alloc_element();
        element->draw = niki_draw_card_slot1_label;
        element->attr.f.phase = 1;
        element->attr.f.x = 0xA0;
        element->attr.f.y = 0x4D;
        element->dimensions.f.width_high = 0;
        element->dimensions.f.height = 0x10;
        NIKI_SET_ELEMENT_WIDTH_LOW(element, 0x80);
        g_niki_element_pool[0].attr.f.state = 0;
        return;
    }

    g_niki_element_pool[0].attr.f.state = 1;
    element = niki_alloc_element();
    element->draw = niki_draw_entry_list;
    element->attr.f.phase = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.y = 0x32;
    element->dimensions.f.width_high = 1;
    element->dimensions.f.height = 0x58;
    NIKI_SET_ELEMENT_WIDTH_LOW(element, 8);

    element = niki_alloc_element();
    element->draw = niki_draw_header_label;
    element->attr.f.phase = 1;
    element->attr.f.x = 0x24;
    element->attr.f.y = 0x0A;
    element->dimensions.f.width_high = 0;
    element->dimensions.f.height = 0x10;
    NIKI_SET_ELEMENT_WIDTH_LOW(element, 0xF0);

    element = niki_alloc_element();
    element->draw = niki_draw_card_slot0_label;
    element->attr.f.phase = 1;
    element->attr.f.x = 0x18;
    element->attr.f.y = 0x1E;
    element->dimensions.f.width_high = 0;
    element->dimensions.f.height = 0x10;
    NIKI_SET_ELEMENT_WIDTH_LOW(element, 0x80);

    element = niki_alloc_element();
    element->draw = niki_draw_card_slot1_label;
    element->attr.f.phase = 1;
    element->attr.f.x = 0xA0;
    element->attr.f.y = 0x1E;
    element->dimensions.f.width_high = 0;
    element->dimensions.f.height = 0x10;
    NIKI_SET_ELEMENT_WIDTH_LOW(element, 0x80);

    element = niki_alloc_element();
    element->draw = niki_draw_selected_entry_details;
    element->attr.f.phase = 1;
    element->attr.f.x = 0x1E;
    element->attr.f.y = 0x8E;
    element->dimensions.f.width_high = 1;
    element->dimensions.f.height = 0x34;
    NIKI_SET_ELEMENT_WIDTH_LOW(element, 4);
    g_niki_element_pool[0].attr.f.state = 0;
}

/**
 * @brief Draw menu elements, process input and advance scrolling.
 * @param frame Draw context receiving the menu packets.
 */
void niki_update_menu(NikiFrameState* frame)
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

    if (g_niki_entry_state >= 0x10)
    {
        if (g_niki_load_step == NULL)
        {
            g_niki_load_step = g_niki_card_setup_sequence;
        }
    }

    do
    {
        result = niki_advance_load_sequence();
    } while (result == 3);

    if ((D_80164B80 != 0) && (g_pad_input & 0x220))
    {
        g_niki_entry_state = 0xF8;
        g_niki_load_step = g_niki_card_info_sequence;
    }
    else
    {
        switch (result)
        {
        case 0:
            break;
        case 4:
            g_niki_load_step = g_niki_rescan_sequence;
            D_80164B80 = 0;
            break;
        case 5:
            g_niki_entry_state = 0xF8;
            /* fallthrough */
        case 2:
            g_niki_load_step = g_niki_card_info_sequence;
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
    NikiElement* element;

    if ((g_niki_element_pool[1].attr.word & NIKI_ELEMENT_STATE_MASK) == 0)
    {
        g_niki_exit_requested = 1;
        return;
    }
    if (g_niki_exit_requested != 0)
    {
        return;
    }
    if (((s32)g_niki_element_pool[1].attr.word & NIKI_ELEMENT_STATE_MASK) >= 3)
    {
        return;
    }
    if ((g_niki_element_pool[0].attr.word & NIKI_ELEMENT_STATE_MASK) != 0)
    {
        return;
    }
    entry_count = g_niki_entry_state;
    if (entry_count == 0xFF)
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
    if ((u32)(*g_niki_load_step - 6) < 2U)
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
        D_80122994 = 3;
        func_800A3938(0x78, 0x80);
        niki_close_all_elements();
        return;
    }
    if (status & 0xA100)
    {
        func_800A3938(0x7D, 0x80);
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
                g_niki_selected_row = g_niki_entry_state - 1;
            }
        }
        if (g_pad_input & 0x4000)
        {
            g_niki_selected_row += 1;
            if (g_niki_selected_row >= g_niki_entry_state)
            {
                g_niki_selected_row = 0;
            }
        }
        navigation_steps -= 1;
    }

    if (g_pad_input & 0x5000)
    {
        niki_commit_selected_entry();
        func_800A3938(0x7D, 0x80);
        niki_scroll_to_selection();
        return;
    }

    if (g_pad_input & NIKI_CONFIRM_INPUT_MASK)
    {
        if (g_niki_mode != 0)
        {
            return;
        }
        if (func_8001714C(D_800ECF7C, g_niki_entries[g_niki_card_slot][g_niki_selected_row].name, 0xC) == 0)
        {
            NikiEntryMetadata* metadata = &g_niki_entry_preview.metadata;
            if ((metadata->identifier != D_8012271C->identifier) && (metadata->status != 0) &&
                ((D_8003EC9C == 0xFF) || (metadata->unknown_0xcf == D_8003EC9C)))
            {
                element = niki_alloc_element();
                element->attr.f.phase = 1;
                element->attr.f.x = 0x10;
                element->attr.f.y = 0x61;
                element->dimensions.f.width_high = 1;
                element->dimensions.f.height = 0x1E;
                NIKI_SET_ELEMENT_WIDTH_LOW(element, 0x20);
                niki_enable_choice_toggle();
                element->draw = niki_draw_confirm_prompt;
                niki_restart_load_sequence();
                func_800A3938(0x7E, 0x80);
                return;
            }
        }
        func_800A3938(0x78, 0x80);
    }
}

void niki_switch_card_slot(void)
{
    D_80164B80 = 0;
    g_niki_load_step = 0;
    g_niki_entry_state = 0xFF;
    g_niki_scroll_frames = 0;
    g_niki_scroll_target_y = 0;
    g_niki_scroll_y = 0;
    g_niki_selected_row = 0;
    g_niki_selection_status = 0;
    g_niki_card_slot ^= 1;
    niki_reset_entry_ranks();
}

/** @brief Start the closing animation for every active menu element. */
void niki_close_all_elements(void)
{
    s32 attributes;
    s32 element_index;
    NikiElement* element;
    s32 closing_attributes;

    func_80067F28();
    element = g_niki_element_pool;
    element_index = 0;
    for (; element_index < NIKI_ELEMENT_COUNT; element_index++, element++)
    {
        attributes = element->attr.word;
        if (attributes & NIKI_ELEMENT_STATE_MASK)
        {
            closing_attributes = (attributes & ~NIKI_ELEMENT_STATE_MASK) | 3;
            element->attr.word = (closing_attributes & ~NIKI_ELEMENT_PHASE_MASK) | 0x40;
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
void niki_update_elements(NikiFrameState* frame)
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
 * @brief Render the niki row/status list: per-entry glyphs, markers and the
 *        highlight tile, dispatched by the g_niki_entry_state list-state selector.
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from every x).
 * @param y_offset Vertical scroll offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_entry_list(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 state = g_niki_entry_state;

    switch (state)
    {
    case 0xF8:
        do
        {
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -x_offset + 0x84, -y_offset, 2);
        } while (0);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_801470FA, 2), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_801470FC, 4), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147108, 0x10), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014710A, 0x12), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFE:
        break;
    default:
    {
        s32 row_y;
        s32 entry_index;

        if (g_niki_entry_scan_active != 0)
        {
            s32 x;
            u8* glyph_table;
        case 0xFF:
            x = -x_offset + 0x84;
            glyph_table = (u8*)&D_801470F8;
            prim = func_800A88A0(prim, ot, glyph_table + D_801470F8, 4, x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_table, 0x1E), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_table, 0xB2), 4, x, 0x1C - y_offset, 2);
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
            u8* glyph_table;

            glyph_table = (u8*)&D_801470F8;
            base_x = -x_offset;
            do
            {
                row_top = ((entry_index * 14) - y_offset) - g_niki_scroll_y;
                row_y = row_top + 1;
                if ((u32)(row_top + 0xE) < 0x65U)
                {
                    rank = &g_niki_entry_ranks[entry_index];
                    if (*rank >= 0)
                    {
                        pos.x = base_x + 0x86;
                        pos.y = row_y;
                        prim = func_800A8A78(ot, prim, g_niki_entry_suffix_values[entry_index], 4, &pos, 0);
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_80147126 + (s32)glyph_table), 4, base_x + 0x70, row_y, 0);
                        if ((g_niki_rank_count - 1) == *rank)
                        {
                            marker_offset = *(u16*)(glyph_table + 0x36);
                            prim = func_800A88A0(prim, ot, (void*)((s32)marker_offset + (s32)glyph_table), 4, base_x + 0xC0, row_y, 0);
                        }
                        else if (*rank < 2)
                        {
                            marker_offset = *(u16*)(glyph_table + 0x38);
                            prim = func_800A88A0(prim, ot, (void*)((s32)marker_offset + (s32)glyph_table), 4, base_x + 0xC0, row_y, 0);
                        }
                        if (*niki_skip_hex_digits(&g_niki_entries[g_niki_card_slot][entry_index].name[12]) == '+')
                        {
                            prim = func_800A88A0(prim, ot, (void*)((s32)D_801471A8 + (s32)glyph_table), 4, 0xF2 - x_offset, row_y, 1);
                        }
                    }
                    if (func_8001714C(D_800ECF7C, g_niki_entries[g_niki_card_slot][entry_index].name, 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, glyph_table + D_801470FE, 4, 1 - x_offset, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECF8C, g_niki_entries[g_niki_card_slot][entry_index].name, 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_80147132 + (s32)glyph_table), 4, 1 - x_offset, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECFC4, g_niki_entries[g_niki_card_slot][entry_index].name, 8) == 0)
                    {
                        prim = func_800A88A0(prim, ot, glyph_table + D_8014710C, 4, 1 - x_offset, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_80147100 + (s32)glyph_table), 4, 1 - x_offset, row_y, 0);
                    }
                }
                entry_index++;
            } while (entry_index < g_niki_entry_state);
        }
        row_y = ((g_niki_selected_row * 14) - y_offset) - g_niki_scroll_y;

        if (g_niki_entry_scan_active == 0)
        {
            NikiTile* tile = (NikiTile*)prim;

            tile->color.word = 0xF080F0;
            tile->tag.bytes.length = 3;
            tile->color.bytes.code = 0x62;
            tile->w = 0x108;
            tile->x0 = 0;
            tile->y0 = row_y;
            tile->h = 0xE;
            niki_link_packet((NikiGpuTag*)ot, &tile->tag);
            prim = (s32)(tile + 1);
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
 * @see decomp.me (100%)
 */
s32 niki_draw_header_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    if (g_niki_mode == 1)
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713E, 0x46), 4, -x_offset + 0x78, -y_offset, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713C, 0x44), 4, -x_offset + 0x78, -y_offset, 2);
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
 * @see decomp.me (100%)
 */
s32 niki_draw_card_slot0_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    NikiTile* tile;

    if (g_niki_card_slot != 0)
    {
        tile = (NikiTile*)prim;
        tile->color.word = 0x101010;
        tile->tag.bytes.length = 3;
        tile->color.bytes.code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        tile->tag.word = (tile->tag.word & GPU_TAG_HIGH_MASK) | (*ot & GPU_ADDR_MASK);
        *ot = (*ot & GPU_TAG_HIGH_MASK) | ((s32)tile & GPU_ADDR_MASK);
        prim += sizeof(NikiTile);
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_80147104, 0xC), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the second card-slot label, shading it when the other slot is selected.
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the caption x).
 * @param y_offset Vertical scroll offset (subtracted from the caption y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_card_slot1_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    NikiTile* tile;

    if (g_niki_card_slot == 0)
    {
        tile = (NikiTile*)prim;
        tile->color.word = 0x101010;
        tile->tag.bytes.length = 3;
        tile->color.bytes.code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        tile->tag.word = (tile->tag.word & GPU_TAG_HIGH_MASK) | (*ot & GPU_ADDR_MASK);
        *ot = (*ot & GPU_TAG_HIGH_MASK) | ((s32)tile & GPU_ADDR_MASK);
        prim += sizeof(NikiTile);
    }

    return func_800A88A0(prim, ot, GLYPH_SYM(D_80147106, 0xE), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the niki save-slot detail panel: element glyphs, the playtime
 *        clock, the slot marker row, and a fallback name/second-line block.
 *
 * Runs only while the panel is active (g_niki_selection_status non-zero) and not suppressed
 * (g_niki_entry_scan_active zero). Depending on g_niki_selection_status it either emits a two-line caption
 * (state 2), or renders the full slot detail: up to three party markers laid out
 * by niki_draw_icon_highlight with an animated highlight (g_niki_icon_phase), the playtime split
 * into hours/minutes via func_800A8A78, and one of three status glyphs. If the
 * slot compare fails it falls back to drawing the stored name and second line.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from every x).
 * @param y_offset Vertical scroll offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_selected_entry_details(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 result;
    Vec2s pos;
    u8 name[0x21];
    char unused_pad[212];
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
    if (g_niki_selection_status != 3 && g_niki_entry_state < 0x10)
    {
        if (g_niki_selection_status == 2)
        {
            s32 x = -x_offset;
            u8* base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(D_80147120, 0x28), 4, x, -y_offset, 0);
            base = (u8*)&D_80147120 - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 4, x, 0x10 - y_offset, 0);
        }
        else
        {
            if (func_8001714C(D_800ECF7C, g_niki_entries[g_niki_card_slot][g_niki_selected_row].name, 0xC) == 0)
            {
                if (D_8003EC9C == 0xFF || g_niki_entry_preview.metadata.unknown_0xcf == D_8003EC9C)
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
                    s32 minutes;
                    s32 wrapped_phase;

                    {
                        NikiEntryMetadata* record = &g_niki_entry_preview.metadata;
                        icons[0] = (u32)(record->first_icon_word) >> 0x19;
                        icons[1] = ((u32)(record->party_word) >> 0x12) & 0x7F;
                        icons[2] = (u32)(record->party_word) >> 0x19;
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
                        NikiEntryMetadata* preview = &g_niki_entry_preview.metadata;
                        s32 x = -x_offset;
                        s32 y = -y_offset;
                        s32 playtime;

                        playtime = preview->playtime_frames;
                        pos.x = (s16)(x + 0x70);
                        pos.y = (s16)y;
                        hours = playtime / 216000;
                        result = func_800A8A78(ot, result, hours, 4, &pos, 1);
                        result = func_800A88A0(result, ot, (void*)(D_800EC3F6[0] + ((s32)&D_800EC3F6 - 0x32) + (D_800EC3F6[1] << 8)), 4, x + 0x6F, y, 0);
                        playtime = (playtime / 3600) - (hours * 0x3C);
                        if (playtime < 0xA)
                        {
                            pos.x = (s16)(x + 0x7D);
                            pos.y = (s16)y;
                            result = func_800A8A78(ot, result, 0, 4, &pos, 1);
                        }
                        pos.x = (s16)(x + 0x85);
                        pos.y = (s16)y;
                        result = func_800A8A78(ot, result, playtime, 4, &pos, 1);
                        result = func_800A88A0(result, ot, preview->text, 4, x + 0x54, y + 0x10, 0);

                        if (preview->identifier == D_8012271C->identifier)
                        {
                            result = func_800A88A0(result, ot, GLYPH_SYM(D_80147148, 0x50), 4, x + 0x54, y + 0x20, 0);
                        }
                        else if (preview->status == 0)
                        {
                            result = func_800A88A0(result, ot, GLYPH_SYM(D_80147146, 0x4E), 4, x + 0x54, y + 0x20, 0);
                        }
                        else
                        {
                            result = func_800A88A0(result, ot, GLYPH_OFF((u8*)D_801475C4, (preview->party_word & 0x3FFFF) * 2), 4, x + 0x54, y + 0x20, 0);
                        }
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, GLYPH_SYM(D_8014714C, 0x54), 4, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 slot_index;
                NikiEntryPreview* record;

                niki_terminate_multibyte_text(g_niki_entry_preview.title.text);
                record = &g_niki_entry_preview;
                if ((u32)(record->title.lines[1][0] - 1) >= 0x7FU)
                {
                    for (slot_index = 0; slot_index < 0x20; slot_index++)
                    {
                        name[slot_index] = *(record->title.lines[0] + slot_index);
                    }
                    name[slot_index] = 0;
                    result = niki_draw_cached_text(result, ot, name, -x_offset, -y_offset, 4, 0);

                    for (slot_index = 0; slot_index < 0x20; slot_index++)
                    {
                        name[slot_index] = g_niki_entry_preview.title.lines[1][slot_index];
                    }
                    name[slot_index] = 0;
                    result = niki_draw_cached_text(result, ot, name, -x_offset, -y_offset + 0x10, 4, 0);
                }
            }
        }
    }
    return result;
}

/**
 * @brief Zero-fill a 64-byte text field from its first character-boundary terminator.
 * @param text Text field to scan; bytes with the high bit set begin two-byte characters.
 * @see decomp.me (100%)
 */
void niki_terminate_multibyte_text(void* text)
{
    u8* cursor;
    s32 byte_index;

    cursor = (u8*)text;
    for (byte_index = 0;;)
    {
        if (byte_index >= 64)
        {
            return;
        }
        if (*cursor == 0)
        {
            while (byte_index < 64)
            {
                *cursor = 0;
                byte_index++;
                cursor++;
            }
            return;
        }
        if (*cursor >= 0x80)
        {
            cursor += 2;
            byte_index += 2;
        }
        else
        {
            cursor += 1;
            byte_index += 1;
        }
    }
}

/**
 * @brief Draw the niki footer glyph, anchored to the right edge of the panel.
 *
 * Resolves the glyph pointer from the D_800EC3D0 header (a 16-bit offset stored
 * across bytes [0] and [1], added to the header base less 0xC), then submits it
 * at x = 0x80 - arg2, y = -arg3.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the anchor x).
 * @param y_offset Vertical scroll offset (subtracted from the anchor y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_footer_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    return func_800A88A0(prim, ot, (void*)((u8*)D_800EC3D0 - 0xC + D_800EC3D0[0] + (D_800EC3D0[1] << 8)), 5, 0x80 - x_offset, -y_offset, 2);
}

/**
 * @brief Reset the niki element array: clear the low 3 state bits of each of
 *        the eight g_niki_element_pool entries and reload the g_menu_element_counter counter.
 *
 * @see decomp.me (100%)
 */
void niki_clear_elements(void)
{
    NikiElement* element;
    s32 element_index;

    g_menu_element_counter = 0x20;
    element = g_niki_element_pool;
    for (element_index = 0; element_index < NIKI_ELEMENT_COUNT; element_index++)
    {
        element->attr.word &= ~NIKI_ELEMENT_STATE_MASK;
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
 * @see decomp.me (100%)
 */
NikiElement* niki_alloc_element(void)
{
    NikiElement* element;
    s32 element_index;

    element = g_niki_element_pool;
    for (element_index = 0; element_index < NIKI_ELEMENT_COUNT; element_index++, element++)
    {
        if ((element->attr.word & NIKI_ELEMENT_STATE_MASK) == 0)
        {
            element->attr.word = (element->attr.word & ~NIKI_ELEMENT_STATE_MASK) | 1;
            return element;
        }
    }
    return g_niki_element_pool;
}

/**
 * @brief Animate element windows and append their content and borders to the frame.
 * @param frame_arg Draw context supplying the clip variant and primitive cursor.
 * @see decomp.me (100%)
 */
void niki_update_and_draw_elements(NikiFrameState* frame_arg)
{
    NikiPacketHeader* prim;
    NikiFrameState* frame;
    NikiElement* element;
    s32 scaled_width;
    s32 scaled_height;
    s32 element_index;
    NikiDrawEnvironment draw_area;
    u32 dispatch_word;
    s32 state;
    u32 dimensions;
    u32 width;
    s32 opening_phase;
    s32 opening_width_product;
    s32 opening_height;
    s32 opening_height_product;
    s32 opening_height_margin;
    u32 opening_attributes;
    u32 width_low;
    s32 closing_phase;
    s32 closing_value;
    s32 closing_height;
    s32 closing_height_product;
    s32 closing_height_margin;
    u32 closing_attributes;
    u32 hold_word;
    s32 entry_count;

    prim = frame_arg->prim_cursor;
    frame = frame_arg;

    if (frame_arg->frame_flag != 0)
    {
        func_8001C56C(&draw_area, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 224);
    }
    else
    {
        func_8001C56C(&draw_area, 0, 8, SCREEN_WIDTH, 224);
    }

    element = g_niki_element_pool;
    element_index = 0;

    for (; element_index < NIKI_ELEMENT_COUNT; element_index++, element++)
    {
        if (element->attr.read_word & NIKI_ELEMENT_STATE_MASK)
        {
            entry_count = g_niki_entry_state;
            if ((entry_count < 16) && (element->draw == niki_draw_entry_list) && ((g_niki_element_pool[1].attr.word & 7) == 2))
            {
                entry_count *= 14;
                if ((g_niki_scroll_y + 88) < entry_count)
                {
                    prim = (NikiPacketHeader*)func_800AE76C(prim, frame, 0x114, 0x82, 0);
                }
                if (g_niki_scroll_y != 0)
                {
                    prim = (NikiPacketHeader*)func_800AE76C(prim, frame, 0x114, 0x3A, 1);
                }
            }

            func_8001A5D4((s32)prim, &draw_area);

            prim->tag = (prim->tag & GPU_TAG_HIGH_MASK) | (frame->head_tag & GPU_ADDR_MASK);
            frame->head_tag = (s32)((frame->head_tag & GPU_TAG_HIGH_MASK) | ((s32)prim & GPU_ADDR_MASK));

            dispatch_word = element->attr.read_word;
            state = dispatch_word & NIKI_ELEMENT_STATE_MASK;

            prim = (NikiPacketHeader*)((NikiDrawEnvironmentPacket*)prim + 1);

            switch (state)
            {
            case 1:
                opening_attributes = element->attr.read_word;
                dimensions = element->dimensions.word;
                width_low = opening_attributes >> 24;
                width = ((dimensions & 1) << 8) | width_low;
                opening_phase = (opening_attributes >> 3) & 0xF;
                opening_width_product = width * opening_phase;
                g_pad_input = 0;
                if (opening_width_product < 0)
                {
                    opening_width_product += 7;
                }
                opening_height = (dimensions >> 1) & 0xFF;
                opening_height_product = opening_height * opening_phase;
                scaled_width = opening_width_product >> 3;
                if (opening_height_product < 0)
                {
                    opening_height_product += 7;
                }
                scaled_height = opening_height_product >> 3;
                opening_height_margin = (s32)(opening_height - scaled_height);

                prim = (NikiPacketHeader*)element->draw((s32*)frame, (s32)prim, (s32)(width - scaled_width) / 2, opening_height_margin / 2);
                {
                    u32 attributes;
                    u32 x;
                    u32 width_low;
                    attributes = element->attr.read_word;
                    x = (attributes >> 7) & 0x1FF;
                    width_low = attributes >> 24;
                    prim = (NikiPacketHeader*)func_800AD850(prim, frame, x + (s32)((((element->dimensions.word & 1) << 8) | width_low) - scaled_width) / 2,
                                                         (element->attr.bytes.y) + ((s32)((element->dimensions.word >> 1) & 0xFF) - scaled_height) / 2,
                                                         scaled_width, scaled_height, frame_arg->frame_flag, element_index == 0);
                }
                {
                    u32 old_word;
                    u32 new_word;
                    old_word = element->attr.read_word;
                    new_word = (old_word & ~NIKI_ELEMENT_PHASE_MASK) | (((((old_word >> 3) & 0xF) + 1) & 0xF) * 8);
                    element->attr.word = new_word;
                    if (((new_word >> 3) & 0xF) == 8)
                    {
                        func_800AA02C();
                        element->attr.word = (element->attr.read_word & ~7) | 2;
                    }
                }
                break;

            case 2:
                prim = (NikiPacketHeader*)element->draw((s32*)frame, (s32)prim, 0, 0);
                {
                    u32 attributes;
                    u32 width_low;
                    attributes = element->attr.read_word;
                    width_low = attributes >> 24;
                    prim = (NikiPacketHeader*)func_800AD850(prim, frame, (attributes >> 7) & 0x1FF, element->attr.bytes.y,
                                                         ((element->dimensions.word & 1) << 8) | width_low, (element->dimensions.word >> 1) & 0xFF, frame_arg->frame_flag,
                                                         element_index == 0);
                }
                hold_word = element->attr.read_word;
                if (((hold_word >> 3) & 0xF) != 0)
                {
                    element->attr.word = (hold_word & ~NIKI_ELEMENT_PHASE_MASK) | (((((hold_word >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case 3:
                closing_phase = element->attr.read_word;
                dimensions = element->dimensions.word;
                closing_value = (u32)closing_phase >> 24;
                width = ((dimensions & 1) << 8) | closing_value;
                closing_phase = (u32)closing_phase >> 3;
                closing_phase &= 0xF;
                closing_value = width * closing_phase;
                g_pad_input = 0;
                if (closing_value < 0)
                {
                    closing_value += 7;
                }
                closing_height = (dimensions >> 1) & 0xFF;
                closing_height_product = closing_height * closing_phase;
                scaled_width = closing_value >> 3;
                if (closing_height_product < 0)
                {
                    closing_height_product += 7;
                }
                scaled_height = closing_height_product >> 3;
                closing_height_margin = (s32)(closing_height - scaled_height);

                prim = (NikiPacketHeader*)element->draw((s32*)frame, (s32)prim, (s32)(width - scaled_width) / 2, closing_height_margin / 2);
                {
                    u32 attributes;
                    u32 x;
                    u32 width_low;
                    attributes = element->attr.read_word;
                    x = (attributes >> 7) & 0x1FF;
                    width_low = attributes >> 24;
                    prim = (NikiPacketHeader*)func_800AD850(prim, frame, x + (s32)((((element->dimensions.word & 1) << 8) | width_low) - scaled_width) / 2,
                                                         (element->attr.bytes.y) + ((s32)((element->dimensions.word >> 1) & 0xFF) - scaled_height) / 2,
                                                         scaled_width, scaled_height, frame_arg->frame_flag, element_index == 0);
                }
                {
                    u32 old_word;
                    old_word = element->attr.read_word;
                    closing_value = old_word & ~NIKI_ELEMENT_PHASE_MASK;
                    old_word >>= 3;
                    old_word &= 0xF;
                    old_word--;
                    old_word &= 0xF;
                    old_word <<= 3;
                    closing_value |= old_word;
                    element->attr.word = closing_value;
                    if (!(((u32)closing_value >> 3) & 0xF))
                    {
                        element->attr.word = ((((u32)closing_value & ~NIKI_ELEMENT_PHASE_MASK) | 0x18) & ~7) | 4;
                    }
                }
                break;

            case 4:
                closing_attributes = element->attr.word;
                g_pad_input = 0;
                hold_word = (closing_attributes & ~NIKI_ELEMENT_PHASE_MASK) | (((((closing_attributes >> 3) & 0xF) - 1) & 0xF) * 8);
                element->attr.word = hold_word;
                if (!((hold_word >> 3) & 0xF))
                {
                    element->attr.word = hold_word & ~7;
                }
                break;
            }
        }
    }

    frame_arg->prim_cursor = prim;
}

/**
 * @brief Clear the low 3 state bits of the first niki element slot.
 * @see decomp.me (100%)
 */
void niki_deactivate_primary_element(void)
{
    g_niki_element_pool[0].attr.word &= ~7;
}

/**
 * @brief Append an encoded NIKI string and terminate the result.
 * @param dst Destination string with room for the appended bytes and terminator.
 * @param src Encoded string to append.
 * @see decomp.me (100%)
 */
void niki_text_append(u8* dst, u8* src)
{
    s32 dst_length;
    s32 src_length;
    s32 byte_index;

    dst_length = niki_text_byte_length(dst);
    src_length = niki_text_byte_length(src);
    for (byte_index = 0; byte_index < src_length; byte_index++)
    {
        dst[dst_length + byte_index] = src[byte_index];
    }
    dst[dst_length + byte_index] = 0;
}

/**
 * @brief Measure an encoded NIKI string, skipping trail bytes of extended characters.
 * @param text Encoded string to measure.
 * @return Length in bytes, excluding the terminator.
 * @see decomp.me (100%)
 */
s32 niki_text_byte_length(u8* text)
{
    u8* cursor;
    u8 lead_byte;
    s32 byte_length;

    cursor = text;
    lead_byte = *cursor;
    byte_length = 0;
    while (lead_byte != 0)
    {
        if ((u32)(lead_byte - NIKI_TEXT_EXTENDED_LEAD_FIRST) < NIKI_TEXT_EXTENDED_PAGE_COUNT)
        {
            cursor += 2;
            byte_length += 2;
        }
        else
        {
            cursor += 1;
            byte_length += 1;
        }
        lead_byte = *cursor;
    }
    return byte_length;
}

/**
 * @brief Copy an encoded NIKI string and append its terminator.
 * @param dst Destination buffer with room for the string and terminator.
 * @param src Encoded string to copy.
 * @see decomp.me (100%)
 */
void niki_text_copy(u8* dst, u8* src)
{
    u8* cursor;
    u8 lead_byte;
    s32 byte_length;
    s32 byte_index;

    cursor = src;
    byte_length = 0;

    while (*cursor != 0)
    {
        lead_byte = *(volatile u8*)cursor;

        if ((u32)(lead_byte - NIKI_TEXT_EXTENDED_LEAD_FIRST) < NIKI_TEXT_EXTENDED_PAGE_COUNT)
        {
            cursor += 2;
            byte_length += 2;
        }
        else
        {
            cursor++;
            byte_length++;
        }
    }

    for (byte_index = 0; byte_index < byte_length; byte_index++)
    {
        dst[byte_index] = src[byte_index];
    }

    dst[byte_index] = 0;
}

/**
 * @brief Draw the load confirmation choice and dispatch acceptance or cancellation.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the caption x).
 * @param y_offset Vertical scroll offset (subtracted from the caption y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_confirm_prompt(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    NikiElement* element;

    x = -x_offset + 0x90;
    result = niki_draw_choice_prompt(func_800A88A0(prim, ot, (u8*)&D_80147128 + D_80147128 - 0x30, 4, x, -y_offset, 2), ot, x, 0xE - y_offset);

    if ((u32)(niki_poll_and_rewind_primary_handles() - 1) < 2U)
    {
        g_niki_element_pool[0].attr.f.state = 0;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        g_niki_entry_state = 0xFF;
        niki_reset_entry_ranks();
        g_niki_load_step = 0;
    }
    else
    {
        status = g_pad_input;
        if (status & NIKI_CANCEL_INPUT_MASK)
        {
            g_niki_element_pool[0].attr.f.state = 0;
            func_800AA02C();
            func_800A3938(0x78, 0x80);
            g_niki_load_step = g_niki_card_info_sequence;
        }
        else if (status & NIKI_CONFIRM_INPUT_MASK)
        {
            if (g_niki_choice_toggle != 0)
            {
                g_niki_element_pool[0].attr.f.state = 0;
                func_800AA02C();
                func_800A3938(0x78, 0x80);
                g_niki_load_step = g_niki_card_info_sequence;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                g_niki_confirm_latch = 1;
                g_niki_load_step = g_niki_load_save_sequence;
                element = g_niki_element_pool;
                element->draw = niki_draw_save_confirm_dialog;
                element->attr.f.phase = 1;
                element->attr.f.state = 1;
                element->attr.f.x = 0x10;
                element->attr.f.y = 0x61;
                element->dimensions.f.width_high = 1;
                element->dimensions.f.height = 0x2C;
                NIKI_SET_ELEMENT_WIDTH_LOW(element, 0x20);
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
 * @see decomp.me (100%)
 */
s32 niki_draw_save_confirm_dialog(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    u8* base;
    NikiSaveBuffer* resource;
    NikiElement* element;
    NikiElement* closing_element;
    s32 result;
    s32 x;
    s32 element_index;

    x = -x_offset + 0x90;
    result = func_800A88A0(prim, ot, (void*)((s32)&D_8014712A - 0x32 + D_8014712A), 4, x, -y_offset, 2);
    base = (u8*)&D_8014712A - 0x32;
    result = func_800A88A0(result, ot, base + *(u16*)(base + 0x1E), 4, x, 0xE - y_offset, 2);
    result = func_800A88A0(result, ot, base + *(u16*)(base + 0xB2), 4, x, 0x1C - y_offset, 2);
    result = niki_draw_progress_bar(result, ot);

    if (g_niki_confirm_latch == 0)
    {
        resource = &g_niki_save_blob;
        element = g_niki_element_pool;
        element->attr.f.state = 0;
        if (niki_validate_save_blob(&resource->save) == 0)
        {
            niki_open_status_dialog(4);
            return result;
        }

        func_800A3938(0x7B, 0x80);
        D_8011F428 = 1;
        D_801227CC = resource->loaded.unknown_0x254;
        D_801227F4 = resource->loaded.unknown_0x256;
        D_8011F418 = g_niki_card_slot;
        func_800170BC(D_8011F3D8, g_niki_selected_save_path);
        func_80016E7C(resource->loaded.trailing_data, D_80122A08, sizeof(resource->loaded.trailing_data));
        func_80067F28();

        closing_element = element;
        for (element_index = 0; element_index < NIKI_ELEMENT_COUNT; element_index++, closing_element++)
        {
            if (closing_element->attr.f.state != 0)
            {
                closing_element->attr.f.state = 3;
                closing_element->attr.f.phase = 8;
            }
        }
        func_80067F5C(8);
    }

    return result;
}

/**
 * @brief Draw the active save-progress bar with width determined by elapsed ticks.
 * @param prim GPU packet write cursor.
 * @param ot Ordering-table entry receiving the bar.
 * @return Advanced packet cursor, or the original cursor when the timer is inactive.
 * @see decomp.me (100%)
 */
s32 niki_draw_progress_bar(s32 prim, s32* ot)
{
    NikiPolyG4Packet* bar;
    s32 elapsed;
    s32 extent;
    s32 color;

    bar = (NikiPolyG4Packet*)prim;
    if (g_niki_progress_bar_active != 0)
    {
        elapsed = func_8002054C(-1) - g_niki_progress_start_tick;
        if (elapsed >= NIKI_PROGRESS_DURATION + 1)
        {
            elapsed = NIKI_PROGRESS_DURATION;
        }
        color = 0xFFFF00;
        extent = elapsed * NIKI_PROGRESS_WIDTH;
        bar->color0.word = 0xFF;
        bar->color1.word = 0xFFFF;
        bar->color3.word = 0xFF0000;
        bar->tag.bytes.length = 8;
        bar->color2.word = color;
        bar->color0.bytes.code = 0x38;
        bar->x2 = 0;
        bar->x0 = 0;
        if (extent < 0)
        {
            extent += 0xFF;
        }
        bar->x3 = extent >> 8;
        bar->x1 = extent >> 8;
        bar->y1 = 0;
        bar->y0 = 0;
        bar->y3 = NIKI_PROGRESS_HEIGHT;
        bar->y2 = NIKI_PROGRESS_HEIGHT;
        bar->tag.word = (bar->tag.word & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | (prim & 0xFFFFFF);
        prim += sizeof(NikiPolyG4Packet);
    }
    return prim;
}

/**
 * @brief Open the primary status window and reset the active card operation.
 * @param dialog_state Message selector consumed by the status draw callback.
 * @see decomp.me (100%)
 */
void niki_open_status_dialog(s32 dialog_state)
{
    func_800A3938(0x78, 0x80);
    g_niki_element_pool[0].draw = niki_draw_status_dialog;
    g_niki_element_pool[0].attr.f.phase = 1;
    g_niki_element_pool[0].attr.f.state = 1;
    g_niki_element_pool[0].attr.f.x = 0x20;
    g_niki_element_pool[0].attr.f.y = 0x70;
    g_niki_element_pool[0].dimensions.f.width_high = 1;
    g_niki_element_pool[0].dimensions.f.height = 0x14;
    NIKI_SET_ELEMENT_WIDTH_LOW(&g_niki_element_pool[0], 0);
    func_800AA02C();
    g_niki_progress_active = 0;
    g_niki_confirm_latch = 0;
    g_niki_selection_status = 0;
    g_niki_io_busy = 0;
    g_niki_entry_state = 0xFF;
    niki_reset_entry_ranks();
    g_niki_load_step = 0;
    g_niki_dialog_state = dialog_state;
}

/**
 * @brief Open the secondary status window and reset the active card operation.
 * @param dialog_state Message selector consumed by the status draw callback.
 * @see decomp.me (100%)
 */
void niki_open_secondary_status_dialog(s32 dialog_state)
{
    NikiElement* element;

    func_800A3938(0x78, 0x80);
    element = &g_niki_element_pool[1];
    element->draw = niki_draw_secondary_status_dialog;
    element->attr.f.phase = 1;
    element->attr.f.state = 1;
    element->attr.f.x = 0x20;
    element->attr.f.y = 0x70;
    element->dimensions.f.width_high = 1;
    element->dimensions.f.height = 0x14;
    NIKI_SET_ELEMENT_WIDTH_LOW(element, 0);
    func_800AA02C();
    D_8011F428 = 2;
    g_niki_progress_active = 0;
    g_niki_confirm_latch = 0;
    g_niki_selection_status = 0;
    g_niki_io_busy = 0;
    niki_reset_entry_ranks();
    g_niki_load_step = 0;
    g_niki_dialog_state = dialog_state;
}

/**
 * @brief Draw the current status message and dismiss its window on confirmation.
 * @param ot Ordering-table entry receiving the text primitives.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal displacement subtracted from the caption position.
 * @param y_offset Vertical displacement subtracted from the caption position.
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_status_dialog(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    switch (g_niki_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147134, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147138, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713A, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147136, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & NIKI_CONFIRM_INPUT_MASK)
    {
        g_niki_element_pool[0].attr.f.state = 0;
        func_800AA02C();
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
 * @see decomp.me (100%)
 */
s32 niki_draw_secondary_status_dialog(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    NikiElement* element;
    s32 element_index;

    switch (g_niki_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147134, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147138, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713A, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147136, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & NIKI_CONFIRM_INPUT_MASK)
    {
        g_menu_element_counter = 0x20;
        element = g_niki_element_pool;
        for (element_index = 0; element_index < NIKI_ELEMENT_COUNT; element_index++)
        {
            element->attr.word &= ~NIKI_ELEMENT_STATE_MASK;
            element++;
        }
        func_80067F5C(8);
        func_800AA02C();
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
 * @see decomp.me (100%)
 */
s32 niki_draw_icon_highlight(s32 prim, s32* ot, s32 x, s32 y, s32 width, s32 icon_index, s32 texture_slot, s32 palette_mode)
{
    RECT rect;
    NikiTexturedQuad* quad;
    s32 texture_column;
    s8 texture_u;

    if (icon_index == 0x7F)
    {
        return prim;
    }
    rect.x = texture_slot * 0x10;
    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    if ((palette_mode == 1) && (icon_index < 2))
    {
        func_800A5638(g_niki_icon_context, icon_index);
        func_80019A34(&rect, g_niki_icon_context);
        func_80019788(0);
    }
    else if (icon_index >= 0x4F)
    {
        func_800A55E4(g_niki_icon_context, g_niki_icon_palette);
        func_80019A34(&rect, g_niki_icon_context);
        func_80019788(0);
    }
    else
    {
        func_80019A34(&rect, ((NikiIcon*)((u8*)g_niki_icon_offsets - sizeof(s32) + g_niki_icon_offsets[icon_index]))->palette);
    }

    texture_column = texture_slot * 3;
    rect.x = texture_column * 4 + 0x140;
    rect.y = 0xD0;
    rect.w = 0xC;
    rect.h = 0x30;
    func_80019A34(&rect, ((NikiIcon*)((u8*)g_niki_icon_offsets - sizeof(s32) + g_niki_icon_offsets[icon_index]))->pixels);
    quad = (NikiTexturedQuad*)prim;
    quad->color.word = 0x808080;
    quad->tag.bytes.length = 9;
    quad->color.bytes.code = 0x2C;
    quad->x2 = x;
    quad->x0 = x;
    quad->y1 = y;
    quad->y0 = y;
    quad->x3 = x + width;
    texture_u = texture_column * 0x10;
    quad->u2 = texture_u;
    quad->u0 = texture_u;
    texture_u += 0x2F;
    quad->u3 = texture_u;
    quad->u1 = texture_u;
    quad->v1 = 0xD0;
    quad->v0 = 0xD0;
    quad->x1 = x + width;
    quad->y3 = y + 0x2F;
    quad->y2 = y + 0x2F;
    quad->v3 = 0xFF;
    quad->v2 = 0xFF;
    quad->clut = (texture_slot & 0x3F) | 0x7C80;
    quad->tpage = 5;
    quad->tag.word = (quad->tag.word & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | (prim & 0xFFFFFF);
    return (s32)(quad + 1);
}

/**
 * @brief Select the cancellation choice when opening a confirmation prompt.
 * @see decomp.me (100%)
 */
void niki_enable_choice_toggle(void)
{
    g_niki_choice_toggle = 1;
}

/**
 * @brief Draw both choices and toggle the selection on horizontal input.
 * @param prim GPU packet write cursor.
 * @param ot Ordering-table entry receiving the captions.
 * @param x Horizontal anchor between the choices.
 * @param y Caption baseline.
 * @return Advanced GPU packet cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_choice_prompt(s32 prim, s32* ot, s32 x, s32 y)
{
    u8* p;
    u8* base;
    s32 first_caption;
    s32 second_caption;
    s32 offset_high;
    s32 palette;

    p = (u8*)&D_800EC3FA;
    offset_high = p[1] << 8;
    base = p - 0x36;
    palette = 4;
    first_caption = p[0] + (offset_high + (s32)base);
    if (g_niki_choice_toggle != 0)
    {
        palette = 5;
    }
    prim = func_800A88A0(prim, ot, (void*)first_caption, palette, x - 0x10, y, 1);
    palette = 4;
    second_caption = base[0x38] + ((base[0x39] << 8) + (s32)base);
    if (g_niki_choice_toggle == 0)
    {
        palette = 5;
    }
    prim = func_800A88A0(prim, ot, (void*)second_caption, palette, x + 8, y, 0);
    if (g_pad_input & 0xA000)
    {
        g_niki_choice_toggle ^= 1;
        func_800A3938(0x7D, 0x80);
        g_pad_input = 0;
    }
    return prim;
}

/**
 * @brief Access the extended metadata in the preview transfer buffer.
 * @return Preview metadata record.
 */
static inline NikiEntryMetadata* niki_preview_metadata(void)
{
    return &g_niki_entry_preview.metadata;
}

/**
 * @brief Draw the save-menu page and process its confirmation and progress states.
 * @param ot Ordering-table entry receiving the page primitives.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal displacement subtracted from glyph positions.
 * @param y_offset Vertical displacement subtracted from glyph positions.
 * @return Advanced GPU packet cursor.
 */
s32 niki_draw_state_page(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 dispatch;
    dispatch = g_niki_entry_state;
    switch (dispatch)
    {
    case 0xf8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xf9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xff:
    {
        s32 x;
        u8* base;
        x = -x_offset + 0x90;
        base = (u8*)&D_801470F8;
        prim = func_800A88A0(prim, ot, base + D_801470F8, 4, x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
    }
    break;
    case 0xfa:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xfd:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_801470FC, 4), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xfb:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147108, 0x10), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xfc:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014710A, 0x12), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xf7:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147160, 0x68), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xf6:
    {
        s32 x;
        u8* base;
        NikiPolyG4Packet* bar;
        s32 next;
        s32 elapsed;
        s32 extent;
        s32 color;
        s32 dialog_state;

        x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, (void*)((s32)&D_8014712A - 0x32 + D_8014712A), 4, x, -y_offset, 2);
        base = (u8*)&D_8014712A - 0x32;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);

        next = prim;
        bar = (NikiPolyG4Packet*)prim;
        if (g_niki_progress_bar_active != 0)
        {
            elapsed = func_8002054C(-1) - g_niki_progress_start_tick;
            if (elapsed >= NIKI_PROGRESS_DURATION + 1)
            {
                elapsed = NIKI_PROGRESS_DURATION;
            }
            color = 0xFFFF00;
            extent = elapsed * NIKI_PROGRESS_WIDTH;
            bar->color0.word = 0xFF;
            bar->color1.word = 0xFFFF;
            bar->color3.word = 0xFF0000;
            bar->tag.bytes.length = 8;
            bar->color2.word = color;
            bar->color0.bytes.code = 0x38;
            bar->x2 = 0;
            bar->x0 = 0;
            if (extent < 0)
            {
                extent += 0xFF;
            }
            bar->x3 = extent >> 8;
            bar->x1 = extent >> 8;
            bar->y1 = 0;
            bar->y0 = 0;
            bar->y3 = NIKI_PROGRESS_HEIGHT;
            bar->y2 = NIKI_PROGRESS_HEIGHT;
            bar->tag.word = (bar->tag.word & 0xFF000000) | (*ot & 0xFFFFFF);
            *ot = (*ot & 0xFF000000) | (prim & 0xFFFFFF);
            next = prim + sizeof(NikiPolyG4Packet);
        }
        prim = next;

        if (g_niki_confirm_latch == 0)
        {
            if (niki_validate_save_blob(&g_niki_save_blob.save) == 0)
            {
                func_800A3938(0x78, 0x80);
                g_niki_element_pool[0].draw = niki_draw_status_dialog;
                g_niki_element_pool[0].attr.f.phase = 1;
                g_niki_element_pool[0].attr.f.state = 1;
                g_niki_element_pool[0].attr.f.x = 0x20;
                g_niki_element_pool[0].attr.f.y = 0x70;
                g_niki_element_pool[0].dimensions.f.width_high = 1;
                g_niki_element_pool[0].dimensions.f.height = 0x14;
                NIKI_SET_ELEMENT_WIDTH_LOW(&g_niki_element_pool[0], 0);
                func_800AA02C();
                g_niki_progress_active = 0;
                g_niki_selection_status = 0;
                g_niki_io_busy = 0;
                g_niki_confirm_latch = 0;
                g_niki_entry_state = 0xFF;
                niki_reset_entry_ranks();
                dialog_state = 4;
                g_niki_load_step = 0;
                g_niki_dialog_state = dialog_state;
                return prim;
            }
            func_800A3938(0x7B, 0x80);
            g_niki_entry_state = 0xF4;
            g_niki_choice_toggle = 1;
            func_800AA02C();
        }
    }
    break;
    case 0xf3:
    {
        s32 x;
        s32 result;
        s32 y;
        u8* p;
        u8* base;
        s32 first_caption;
        s32 second_caption;
        s32 offset_high;
        s32 palette;
        NikiElement* packet;
        s32 i;

        x = -x_offset;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014716A, 0x72), 4, x + 0x90, -y_offset, 2);
        y = 0xE - y_offset;
        p = (u8*)&D_800EC3FA;
        offset_high = p[1] << 8;
        base = p - 0x36;
        palette = 4;
        first_caption = p[0] + (offset_high + (s32)base);
        if (g_niki_choice_toggle != 0)
        {
            palette = 5;
        }
        result = func_800A88A0(prim, ot, (void*)first_caption, palette, x + 0x80, y, 1);
        palette = 4;
        second_caption = base[0x38] + ((base[0x39] << 8) + (s32)base);
        if (g_niki_choice_toggle == 0)
        {
            palette = 5;
        }
        result = func_800A88A0(result, ot, (void*)second_caption, palette, x + 0x98, y, 0);
        if (g_pad_input & 0xA000)
        {
            g_niki_choice_toggle ^= 1;
            func_800A3938(0x7D, 0x80);
            g_pad_input = 0;
        }

        prim = result;

        if (g_pad_input & NIKI_CANCEL_INPUT_MASK)
        {
            func_800A3938(0x78, 0x80);
            g_niki_choice_toggle = 1;
            g_niki_entry_state = 0xF4;
            func_800AA02C();
        }
        else if (g_pad_input & NIKI_CONFIRM_INPUT_MASK)
        {
            if (g_niki_choice_toggle != 0)
            {
                func_800A3938(0x78, 0x80);
                g_niki_choice_toggle = 1;
                g_niki_entry_state = 0xF4;
                func_800AA02C();
            }
            else
            {
                func_800A3938(0x7D, 0x80);
                packet = g_niki_element_pool;
                D_8011F428 = 2;
                g_menu_element_counter = 0x20;
                for (i = 0; i < NIKI_ELEMENT_COUNT; i++, packet++)
                {
                    packet->attr.f.state = 0;
                }
                func_80067F5C(8);
                func_800AA02C();
            }
        }
    }
    break;
    case 0xf4:
    {
        s32 x;
        s32 result;
        s32 one;
        s32 y;
        u8* caption_table;
        u8* p;
        u8* base;
        s32 first_caption;
        s32 second_caption;
        s32 offset_high;
        s32 palette;
        s32 record_count;
        s32 record_index;
        u8(*records)[64];
        NikiSaveBuffer* resource;
        s32 checksum;

        x = -x_offset;
        prim = func_800A88A0(prim, ot, (void*)((s32)&D_80147162 - 0x6A + D_80147162), 4, x + 0x90, -y_offset, 2);
        caption_table = (u8*)&D_80147162 - 0x6A;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(caption_table, 0x70), 4, x + 0x90, 0xE - y_offset, 2);

        y = 0x1C - y_offset;
        p = (u8*)&D_800EC3FA;
        offset_high = p[1] << 8;
        base = p - 0x36;
        palette = 4;
        first_caption = p[0] + (offset_high + (s32)base);
        if (g_niki_choice_toggle != 0)
        {
            palette = 5;
        }
        one = 1;
        result = func_800A88A0(prim, ot, (void*)first_caption, palette, x + 0x80, y, one);
        palette = 4;
        second_caption = base[0x38] + ((base[0x39] << 8) + (s32)base);
        if (g_niki_choice_toggle == 0)
        {
            palette = 5;
        }
        result = func_800A88A0(result, ot, (void*)second_caption, palette, x + 0x98, y, 0);
        if (g_pad_input & 0xA000)
        {
            g_niki_choice_toggle ^= 1;
            func_800A3938(0x7D, 0x80);
            g_pad_input = 0;
        }

        prim = result;

        if ((g_pad_input & NIKI_CANCEL_INPUT_MASK) || ((g_pad_input & NIKI_CONFIRM_INPUT_MASK) && g_niki_choice_toggle != 0))
        {
            g_niki_choice_toggle = one;
            g_niki_entry_state = 0xF3;
            func_800A3938(0x78, 0x80);
            func_800AA02C();
        }
        else if (g_pad_input & NIKI_CONFIRM_INPUT_MASK)
        {
            func_800A3938(0x7E, 0x80);
            records = (u8(*)[64])D_80122A08;
            resource = &g_niki_save_blob;
            func_80016E7C(D_80122A08, resource->loaded.trailing_data, sizeof(resource->loaded.trailing_data));
            record_count = 0;
            for (record_index = 0; record_index < 4; record_index++)
            {
                if (records[record_index][0] != 0)
                {
                    record_count++;
                }
            }
            resource->loaded.trailing_record_count = record_count;
            checksum = niki_compute_save_checksum(resource->save.payload);
            resource->save.magic = NIKI_SAVE_MAGIC;
            resource->save.checksum = checksum;
            g_niki_progress_active = 1;
            g_niki_load_step = g_niki_write_save_sequence;
            g_niki_entry_state = 0xF5;
        }
    }
    break;
    case 0xf5:
    {
        s32 x;
        u8* base;
        NikiPolyG4Packet* bar;
        s32 next;
        s32 elapsed;
        s32 extent;
        s32 color;
        NikiElement* packet;
        s32 i;

        x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, (void*)((s32)&D_80147114 - 0x1C + D_80147114), 4, x, -y_offset, 2);
        base = (u8*)&D_80147114 - 0x1C;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);

        next = prim;
        bar = (NikiPolyG4Packet*)prim;
        if (g_niki_progress_bar_active != 0)
        {
            elapsed = func_8002054C(-1) - g_niki_progress_start_tick;
            if (elapsed >= NIKI_PROGRESS_DURATION + 1)
            {
                elapsed = NIKI_PROGRESS_DURATION;
            }
            color = 0xFFFF00;
            extent = elapsed * NIKI_PROGRESS_WIDTH;
            bar->color0.word = 0xFF;
            bar->color1.word = 0xFFFF;
            bar->color3.word = 0xFF0000;
            bar->tag.bytes.length = 8;
            bar->color2.word = color;
            bar->color0.bytes.code = 0x38;
            bar->x2 = 0;
            bar->x0 = 0;
            if (extent < 0)
            {
                extent += 0xFF;
            }
            bar->x3 = extent >> 8;
            bar->x1 = extent >> 8;
            bar->y1 = 0;
            bar->y0 = 0;
            bar->y3 = NIKI_PROGRESS_HEIGHT;
            bar->y2 = NIKI_PROGRESS_HEIGHT;
            bar->tag.word = (bar->tag.word & 0xFF000000) | (*ot & 0xFFFFFF);
            *ot = (*ot & 0xFF000000) | (prim & 0xFFFFFF);
            next = prim + sizeof(NikiPolyG4Packet);
        }
        prim = next;

        if (g_niki_progress_active == 0)
        {
            func_800A3938(0x7A, 0x80);
            g_menu_element_counter = 0x20;
            packet = g_niki_element_pool;
            for (i = 0; i < NIKI_ELEMENT_COUNT; i++, packet++)
            {
                packet->attr.f.state = 0;
            }
            func_80067F5C(8);
            D_8011F428 = 0;
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
        base = (u8*)&D_801470F8;
        prim = func_800A88A0(prim, ot, base + D_801470F8, 4, x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);

        if (g_niki_entry_scan_active == 0)
        {
            if (g_niki_io_busy != 0)
            {
                return prim;
            }
            if ((u32)(*g_niki_load_step - 6) < 2U)
            {
                return prim;
            }
            if ((func_8001714C(D_800ECF7C, g_niki_entries[g_niki_card_slot][g_niki_selected_row].name, 0xC) != 0) ||
                (niki_preview_metadata()->identifier != D_801227CC) || (niki_preview_metadata()->unknown_0xd6 != D_801227F4))
            {
                g_niki_selected_row++;
                if (g_niki_selected_row >= g_niki_entry_state)
                {
                    if (g_niki_entry_state != 0)
                    {
                        g_niki_entry_state = 0xF7;
                    }
                    else
                    {
                        g_niki_entry_state = 0xF8;
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
                g_niki_progress_start_tick = func_8002054C(-1);
                g_niki_confirm_latch = 1;
                g_niki_load_step = g_niki_read_saved_copy_sequence;
                g_niki_entry_state = 0xF6;
            }
        }
    }
    break;
    case 0xFE:
        break;
    }

    if (g_niki_io_busy != 0)
    {
        return prim;
    }
    if (g_niki_entry_state == 0xF6)
    {
        return prim;
    }
    if (g_niki_entry_state == 0xF5)
    {
        return prim;
    }
    if (g_niki_entry_state == 0xF4)
    {
        return prim;
    }
    if (g_niki_entry_state == 0xF3)
    {
        return prim;
    }

    if (g_pad_input & NIKI_CANCEL_INPUT_MASK)
    {
        NikiElement* element;
        s32 i;
        s32 word;
        D_80122994 = 3;
        func_800A3938(0x78, 0x80);
        func_80067F28();
        element = g_niki_element_pool;
        i = 0;
        do
        {
            word = element->attr.word;
            if (word & 7)
            {
                element->attr.word = (((word & ~7) | 3) & ~NIKI_ELEMENT_PHASE_MASK) | 0x40;
            }
            i++;
            element++;
        } while (i < NIKI_ELEMENT_COUNT);
        return prim;
    }

    if ((g_pad_input & 0xA100) && (g_niki_entry_state != 0xFF))
    {
        func_800A3938(0x7D, 0x80);
        D_80164B80 = 0;
        g_niki_load_step = 0;
        g_niki_scroll_frames = 0;
        g_niki_scroll_target_y = 0;
        g_niki_scroll_y = 0;
        g_niki_selected_row = 0;
        g_niki_entry_state = 0xFF;
        g_niki_selection_status = 0;
        g_niki_card_slot ^= 1;
        niki_reset_entry_ranks();
        g_niki_progress_bar_active = 0;
        g_niki_load_step = g_niki_card_setup_sequence;
    }

    return prim;
}

/**
 * @brief Advance past a run of ASCII hexadecimal-digit characters.
 * @param text Pointer to the start of the scan.
 * @return Pointer to the first byte that is not a hex digit
 *         (@c '0'-'9', @c 'a'-'f' or @c 'A'-'F').
 * @see decomp.me (100.00%)
 */
u8* niki_skip_hex_digits(void* text)
{
    u8* cursor = text;

    while ((u32)(*cursor - '0') < 10 || (u32)(*cursor - 'a') < 6 || (u32)(*cursor - 'A') < 6)
    {
        cursor++;
    }
    return cursor;
}

/** @brief Full-width MAX text used when a decimal value exceeds five digits. */
const NikiDecimalOverflow g_niki_decimal_overflow_text __attribute__((aligned(4))) = {{0x82, 0x6C, 0x82, 0x60, 0x82, 0x77, 0}};

/**
 * @brief Validate the save payload checksum and format marker.
 * @param blob Serialized save data to validate.
 * @return One when both checks pass, otherwise zero.
 * @see decomp.me (100.00%)
 */
s32 niki_validate_save_blob(NikiSaveBlob* blob)
{
    if (blob->checksum == niki_compute_save_checksum(blob->payload))
    {
        if (blob->magic == NIKI_SAVE_MAGIC)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Sum the save payload bytes and apply the checksum scale and bias.
 * @param data Start of the fixed-size save payload.
 * @return Twice the byte sum plus NIKI_SAVE_CHECKSUM_BIAS.
 * @see decomp.me (100.00%)
 */
s32 niki_compute_save_checksum(u8* data)
{
    s32 sum;
    u32 byte_index;
    u8* cursor;

    cursor = data;
    sum = 0;
    byte_index = 0;
    do
    {
        byte_index++;
        sum += *cursor;
        cursor++;
    } while (byte_index < NIKI_SAVE_PAYLOAD_BYTES);
    return sum * 2 + NIKI_SAVE_CHECKSUM_BIAS;
}

/**
 * @brief Format up to six decimal digits as full-width Shift-JIS characters.
 * @param out Destination byte buffer.
 * @param value Number to format.
 * @return Pointer to the terminator, or six bytes past the start for the overflow string.
 * @note Values at least 1000000 use the fixed overflow string; leading zeroes are suppressed.
 * @see decomp.me (100.00%)
 */
s8* niki_format_decimal(s8* out, s32 value)
{
    s32 digit;
    s32 divisor;
    s32 started;
    s8* cursor;

    cursor = out;
    divisor = 100000;
    if (value >= divisor * 10)
    {
        *(NikiDecimalOverflow*)cursor = g_niki_decimal_overflow_text;
        return cursor + 6;
    }

    started = 0;
    do
    {
        digit = value / divisor;
        if (digit != 0 || started != 0)
        {
            *cursor++ = (digit + 0x824F) >> 8;
            *cursor++ = digit + 0x4F;
            started = 1;
        }
        if (divisor == 1)
        {
            break;
        }
        if (divisor == 10)
        {
            started = 1;
        }
        value -= digit * divisor;
        divisor /= 10;
    } while (1);
    *cursor = 0;
    return cursor;
}

/**
 * @brief Format a hexadecimal string with leading zeroes suppressed.
 * @param out Destination character buffer.
 * @param value Number to format.
 * @param max_chars Maximum number of digits to emit, excluding the terminator.
 * @see decomp.me (100.00%)
 */
void niki_format_hex(s8* out, s32 value, s32 max_chars)
{
    s32 nibble;
    s32 shift_index;
    s32 started;

    shift_index = 7;
    started = 0;
    if (max_chars != 0)
    {
        do
        {
            nibble = (value >> (shift_index * 4)) & 0xF;
            if (nibble != 0 || started != 0)
            {
                niki_hex_nibble_to_ascii(out, nibble);
                out++;
                max_chars--;
                started = 1;
                value -= nibble << (shift_index * 4);
            }
            shift_index--;
            if (shift_index == -1)
            {
                break;
            }
            if (shift_index == 0)
            {
                started = 1;
            }
        } while (max_chars);
    }
    *out = 0;
}

/**
 * @brief Convert a 0-15 value to its ASCII hexadecimal digit.
 * @param out Destination byte.
 * @param value Nibble value; 0-9 -> '0'-'9', 10-15 -> 'A'-'F', else '_'.
 * @return None.
 * @see decomp.me (100.00%)
 */
void niki_hex_nibble_to_ascii(s8* out, s32 value)
{
    if (value < 10)
    {
        *out = value + 0x30;
    }
    else if (value < 16)
    {
        *out = value + 0x37;
    }
    else
    {
        *out = 0x5F;
    }
}

/**
 * @brief Parse a bounded run of ASCII hexadecimal digits.
 * @param text First digit to parse.
 * @param digits_left Maximum number of digits to consume.
 * @return Accumulated value; parsing stops at the digit limit or first non-hex byte.
 * @see decomp.me (100.00%)
 */
u32 niki_parse_hex(u8* text, s32 digits_left)
{
    u32 result;

    result = 0;
    while (((u8)(*text - '0') < 10) || ((u8)(*text - 'a') < 6) || ((u8)(*text - 'A') < 6))
    {
        if (digits_left == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*text - '0') < 10)
        {
            u32 decimal_base;

            decimal_base = result - '0';
            result = decimal_base + *text;
        }
        else if ((u8)(*text - 'A') < 6)
        {
            u32 uppercase_base;

            uppercase_base = result - ('A' - 10);
            result = uppercase_base + *text;
        }
        else if ((u8)(*text - 'a') < 6)
        {
            u32 lowercase_base;

            lowercase_base = result - ('a' - 10);
            result = lowercase_base + *text;
        }
        text++;
        digits_left--;
    }
    return result;
}

/**
 * @brief Skip a hexadecimal run and its separator, then parse up to two hex digits.
 * @param text Start of the leading hexadecimal run.
 * @return Parsed suffix byte.
 * @see decomp.me (100.00%)
 */
s32 niki_parse_hex_suffix_byte(u8* text)
{
    s32 digits_left;
    u32 result;

    while ((u32)(*text - '0') < 10 || (u32)(*text - 'a') < 6 || (u32)(*text - 'A') < 6)
    {
        text++;
    }

    text++;
    digits_left = 2;
    result = 0;
    while (((u8)(*text - '0') < 10) || ((u8)(*text - 'a') < 6) || ((u8)(*text - 'A') < 6))
    {
        if (digits_left == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*text - '0') < 10)
        {
            u32 decimal_base;

            decimal_base = result - '0';
            result = decimal_base + *text;
        }
        else if ((u8)(*text - 'A') < 6)
        {
            u32 uppercase_base;

            uppercase_base = result - ('A' - 10);
            result = uppercase_base + *text;
        }
        else if ((u8)(*text - 'a') < 6)
        {
            u32 lowercase_base;

            lowercase_base = result - ('a' - 10);
            result = lowercase_base + *text;
        }
        text++;
        digits_left--;
    }
    return result;
}

/**
 * @brief Parse field values and suffix bytes from recognized save-file names.
 * @return Largest suffix byte among the recognized entries.
 * @see decomp.me (100%)
 */
s32 niki_parse_entry_fields(void)
{
    s32 entry_index;
    s32 max_suffix;
    u8* cursor;
    u8* suffix;
    s32 digits_left;
    s32 value;
    u32 decimal_base;
    u32 uppercase_base;
    u32 lowercase_base;
    s32 suffix_value;

    entry_index = 0;
    max_suffix = entry_index;
    while (entry_index < g_niki_entry_state)
    {
        u8* pattern;
        pattern = (u8*)&D_800ECF7C;
        if (func_8001714C(pattern, g_niki_entries[g_niki_card_slot][entry_index].name, 0xC) == 0)
        {
            digits_left = 5;
            cursor = (u8*)(g_niki_card_slot * NIKI_CARD_DIRECTORY_BYTES + entry_index * NIKI_DIRECTORY_ENTRY_BYTES + (s32)g_niki_entries + 0xC);
            value = 0;
            while (((u8)(*cursor - '0') < 10) || ((u8)(*cursor - 'a') < 6) || ((u8)(*cursor - 'A') < 6))
            {
                if (digits_left == 0)
                {
                    break;
                }
                value <<= 4;
                if ((u8)(*cursor - '0') < 10)
                {
                    decimal_base = value - '0';
                    value = decimal_base + *cursor;
                }
                else if ((u8)(*cursor - 'A') < 6)
                {
                    uppercase_base = value - ('A' - 10);
                    value = uppercase_base + *cursor;
                }
                else if ((u8)(*cursor - 'a') < 6)
                {
                    lowercase_base = value - ('a' - 10);
                    value = lowercase_base + *cursor;
                }
                cursor++;
                digits_left--;
            }
            suffix = &g_niki_entries[g_niki_card_slot][entry_index].name[0xC];
            {
                s32* fields = &g_niki_entry_fields[g_niki_card_slot * NIKI_DIRECTORY_ENTRY_COUNT];
                fields[entry_index] = value;
            }
            suffix_value = niki_parse_hex_suffix_byte(suffix);
            g_niki_entry_suffix_values[entry_index] = suffix_value;
            if (max_suffix < suffix_value)
            {
                max_suffix = suffix_value;
            }
        }
        else
        {
            s32* fields = &g_niki_entry_fields[g_niki_card_slot * NIKI_DIRECTORY_ENTRY_COUNT];
            fields[entry_index] = -1;
            g_niki_entry_suffix_values[entry_index] = 0;
        }
        entry_index++;
    }
    return max_suffix;
}

/**
 * @brief Rank recognized entries and select the entry with the greatest field value.
 * @param unused0 Unused.
 * @param unused1 Unused.
 * @param unused2 Unused.
 * @return Index of the greatest field value, or zero when none is present.
 */
s32 niki_rank_entries(s32 unused0, s32 unused1, s32 unused2)
{
    s32* fields;
    s32* field;
    s32* rank;
    s32* previous_field;
    s32* previous_rank;
    s32* ranks;
    s32* current_field;
    s32* candidate;
    s32* field_table;
    s32* entry_field_table;
    s32 slot;
    s32* suffix_output;
    NikiDirEntry* entry_cursor;
    s32 next_rank;
    s32 entry_index;
    s32 maximum;
    s32 entry_count;
    s32 max_suffix;
    s32 higher_count;
    s32 previous_index;

    niki_parse_entry_fields();
    maximum = -1;
    niki_sort_entries_by_type();
    entry_index = 0;
    max_suffix = niki_parse_entry_fields();
    niki_reset_entry_ranks();
    next_rank = 1;
    if (g_niki_entry_state > 0)
    {
        entry_count = g_niki_entry_state;
        ranks = &g_niki_entry_ranks[0];
        rank = ranks;
        slot = g_niki_card_slot;
        entry_field_table = g_niki_entry_fields;
        fields = entry_field_table + slot * NIKI_DIRECTORY_ENTRY_COUNT;
        field = fields;
        do
        {
            if (*field >= 0)
            {
                previous_index = 0;
                if (entry_index > 0)
                {
                    previous_index += 1;
                    previous_index -= 1;
                }
                if (*field >= maximum)
                {
                    *rank = next_rank;
                    maximum = *field;
                    next_rank += 1;
                }
                else
                {
                    higher_count = previous_index;
                    if (entry_index > 0)
                    {
                        current_field = field;
                        previous_rank = ranks;
                        previous_field = fields;
                        do
                        {
                            if (*current_field < *previous_field)
                            {
                                higher_count += 1;
                                *previous_rank += 1;
                            }
                            previous_rank += 1;
                            previous_index += 1;
                            previous_field += 1;
                        } while (previous_index < entry_index);
                    }
                    {
                        s32 rank_value;
                        do
                        {
                            do
                            {
                                do
                                {
                                    rank_value = next_rank - higher_count;
                                } while (0);
                            } while (0);
                        } while (0);
                        *rank = rank_value;
                    }
                    next_rank += 1;
                }
            }
            rank += 1;
            entry_index += 1;
            field += 1;
        } while (entry_index < entry_count);
    }
    previous_field = ranks;
    previous_rank = fields;
    g_niki_rank_count = next_rank;
    next_rank = -1;
    entry_index = 0;
    maximum = 0;
    if (g_niki_entry_state > 0)
    {
        s32 max_count;
        max_count = g_niki_entry_state;
        slot = g_niki_card_slot;
        field_table = g_niki_entry_fields;
        candidate = &field_table[slot * NIKI_DIRECTORY_ENTRY_COUNT];
        do
        {
            if (next_rank < *candidate)
            {
                next_rank = *candidate;
                maximum = entry_index;
            }
            entry_index += 1;
            candidate += 1;
        } while (entry_index < max_count);
        entry_index = 0;
    }
    g_niki_entry_value_limit = next_rank + 1;
    if (g_niki_entry_state > 0)
    {
        suffix_output = &g_niki_entry_suffix_values[0];
        entry_cursor = g_niki_entries[0];
    loop_20:
        if (func_8001714C(&D_800ECFC4[0], ((NikiDirEntry*)((g_niki_card_slot * NIKI_CARD_DIRECTORY_BYTES) + (s32)entry_cursor))->name, 8) == 0)
        {
            *suffix_output = max_suffix + 1;
        }
        else
        {
            suffix_output += 1;
            entry_cursor++;
            entry_index += 1;
            if (entry_index < g_niki_entry_state)
            {
                goto loop_20;
            }
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

    for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
    {
        if (func_8001714C(D_800ECF7C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) == 0 ||
            func_8001714C(D_800ECF8C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) == 0)
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
    for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
    {
        total_blocks += g_niki_entries[g_niki_card_slot][entry_index].size / NIKI_MEMORY_CARD_BLOCK_BYTES;
    }
    return total_blocks >= 14;
}

/** @brief Remove both placeholder save files from the selected memory card. */
void niki_remove_placeholder_saves(void)
{
    NikiPlaceholderPath path;

    memcpy(&path, &g_niki_file_template, 6);
    path.device.characters.slot += (u8)g_niki_card_slot;
    func_80016F9C(&path, &D_800ECF9C);
    func_8001686C(&path);

    memcpy(&path, &g_niki_file_template, 6);
    path.device.characters.slot += (u8)g_niki_card_slot;
    func_80016F9C(&path, &D_800ECFB0);
    func_8001686C(&path);
}

/** @brief Device prefix used to construct memory-card file paths. */
const char g_niki_file_template[8] __attribute__((aligned(4))) = "bu00:";
