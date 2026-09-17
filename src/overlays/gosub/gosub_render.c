#include "gosub_internal.h"

/**
 * @brief Start the exit transition for every allocated element.
 * @see https://decomp.me/scratch/RsBVl
 */
void gosub_start_element_exit(void)
{
    s32 element_word;
    s32 element_index;
    GosubElement* element;
    s32 state_word;

    element = g_gosub_elements;
    element_index = 0;
    do
    {
        element_word = element->attr.word;
        if (element_word & 7)
        {
            state_word = element_word & ~7;
            element->attr.word = (state_word & ~0x78) | 0x40;
        }
        element_index += 1;
        element += 1;
    } while (element_index < GOSUB_ELEMENT_COUNT);
}

/**
 * @brief Render and animate all allocated gosub elements.
 * @param render_context Field render context and packet cursor.
 * @see https://decomp.me/scratch/nVefu
 */
void gosub_render_elements(GosubRenderContext* render_context)
{
    gosub_update_and_render_elements(render_context);
}

/**
 * @brief Mark every gosub element slot inactive.
 * @see https://decomp.me/scratch/dib6Q
 */
void gosub_clear_elements(void)
{
    s32 element_index;
    GosubElement* element;

    element = g_gosub_elements;
    element_index = 0;
    do
    {
        element_index += 1;
        element->attr.word &= ~7;
        element += 1;
    } while (element_index < GOSUB_ELEMENT_COUNT);
}

/**
 * @brief Allocate the first inactive dynamic element slot.
 * @return Allocated element, or element 0 when the pool is full.
 * @see https://decomp.me/scratch/X1pXK
 */
GosubElement* gosub_allocate_element(void)
{
    s32 element_word;
    s32 element_index;
    GosubElement* element;

    element = g_gosub_dynamic_elements;

    for (element_index = 1; element_index < GOSUB_ELEMENT_COUNT; element_index++, element++)
    {
        element_word = element->attr.word;
        if (!(element_word & 7))
        {
            element->attr.word = (element_word & ~7) | GOSUB_ELEMENT_STATE_ENTERING;
            return element;
        }
    }

    return g_gosub_elements;
}

/**
 * @brief Animate, draw, frame, and link every allocated gosub element.
 * @param render_context Field render context and packet cursor.
 * @see https://decomp.me/scratch/t79hi
 */
void gosub_update_and_render_elements(GosubRenderContext* render_context)
{
    GosubGpuPacket* packet_cursor;
    s32 animated_width;
    s32 animated_height;
    GosubRenderContext* ordering_table;
    GosubElement* element;
    u32 address_mask;
    u32 tag_mask;
    s32 element_index;
    s32 draw_env[24];
    u32 geometry_word;
    s32 element_height;
    u16 visible_height;
    u32 geometry;
    u32 element_width;
    u32 element_word;
    s32 element_height_calc;
    s32 packet_address;
    s32 content_height;
    GosubGpuPacket* draw_cursor;
    u32 marker_word;
    u32 panel_word;
    u32 state_word;
    s32 working_word;
    s32 exit_transition_step;
    s32 exit_scaled_width;
    s32 exit_full_height;
    s32 exit_scaled_height;
    s32 exit_remaining_height;
    u32 entering_word;
    u32 updated_entering_word;
    s32 transition_step;
    s32 scaled_width;
    s32 full_height;
    s32 scaled_height;
    s32 remaining_height;
    u32 exiting_word;
    u32 updated_exiting_word;

    packet_cursor = render_context->packet_cursor;
    ordering_table = render_context;

    if (render_context->display_buffer_index != 0)
    {
        SetDefDrawEnv((DRAWENV*)draw_env, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        SetDefDrawEnv((DRAWENV*)draw_env, 0, 8, 0x140, 0xE0);
    }

    element = g_gosub_elements;
    element_index = 0;
    address_mask = 0x00FFFFFF;
    tag_mask = 0xFF000000;

    for (; element_index < GOSUB_ELEMENT_COUNT; element_index++)
    {
        element_word = element->attr.word;
        if (element_word & 7)
        {
            draw_cursor = packet_cursor;

            if ((GosubElementDrawHandler)element->draw_handler == (GosubElementDrawHandler)gosub_draw_item_list)
            {
                geometry_word = element->geometry.word;
                element_height = (geometry_word >> 1) & 0xFF;

                content_height = g_gosub_row_count * g_gosub_row_height;
                if ((g_gosub_scroll_y + element_height) < content_height)
                {
                    {
                        u32 field;
                        u32 high;
                        field = (element_word >> 7) & 0x1FF;
                        high = element_word >> 24;
                        packet_cursor = gosub_emit_scroll_marker((GosubScrollMarkerPacket*)draw_cursor, (s32*)ordering_table,
                                                                 (field + (((geometry_word & 1) << 8) | high)) - 0x10, element->attr.f.y + element_height, 0);
                    }
                }
                if (g_gosub_scroll_y != 0)
                {
                    {
                        u32 field;
                        u32 high;
                        marker_word = element->attr.word;
                        field = (marker_word >> 7) & 0x1FF;
                        high = marker_word >> 24;
                        packet_cursor = gosub_emit_scroll_marker((GosubScrollMarkerPacket*)packet_cursor, (s32*)ordering_table,
                                                                 (field + (((element->geometry.word & 1) << 8) | high)) - 0x10, element->attr.f.y, 1);
                    }
                }
                SetDrawEnv((DR_ENV*)packet_cursor, (DRAWENV*)draw_env);

                packet_cursor->tag = (packet_cursor->tag & tag_mask) | (ordering_table->tag & address_mask);
                ordering_table->tag = (s32)((ordering_table->tag & tag_mask) | ((s32)packet_cursor & address_mask));

                packet_cursor = (GosubGpuPacket*)((u8*)packet_cursor + 0x40);

                if (g_gosub_row_count != 0)
                {
                    packet_cursor->color = 0xFFFF00;
                    ((u8*)packet_cursor)[3] = 3;
                    ((u8*)packet_cursor)[7] = 0x60;
                    packet_cursor->w = 6;
                    element_height_calc = (element->geometry.word >> 1) & 0xFF;
                    packet_cursor->h = (u16)((s32)(element_height_calc * (element_height_calc / g_gosub_row_height)) / g_gosub_row_count);
                    {
                        s32 clamp_h;
                        clamp_h = (s16)packet_cursor->h;
                        working_word = element->geometry.word;
                        visible_height = ((u32)working_word >> 1) & 0xFF;
                        if (clamp_h >= (s32)visible_height - 2)
                        {
                            packet_cursor->h = visible_height;
                        }
                    }
                    packet_cursor->x = 1;
                    packet_cursor->y = (s16)((s32)(((element->geometry.word >> 1) & 0xFF) * (g_gosub_scroll_y / g_gosub_row_height)) / g_gosub_row_count);
                    packet_cursor->tag = (packet_cursor->tag & tag_mask) | (ordering_table->tag & address_mask);

                    packet_address = (s32)packet_cursor & address_mask;
                    packet_cursor = (GosubGpuPacket*)((u8*)packet_cursor + 0x10);
                    ordering_table->tag = (s32)((ordering_table->tag & tag_mask) | packet_address);
                }
                {
                    u32 field;
                    u32 high;
                    panel_word = element->attr.word;
                    field = (panel_word >> 7) & 0x1FF;
                    high = panel_word >> 24;
                    packet_cursor = gosub_emit_panel(packet_cursor, (s32*)ordering_table, field + (((element->geometry.word & 1) << 8) | high) + 3,
                                                     element->attr.f.y, 0xA, (element->geometry.word >> 1) & 0xFF, render_context->display_buffer_index);
                }
                draw_cursor = packet_cursor;
            }
            SetDrawEnv((DR_ENV*)draw_cursor, (DRAWENV*)draw_env);
            packet_cursor->tag = (packet_cursor->tag & tag_mask) | (ordering_table->tag & address_mask);
            ordering_table->tag = (s32)((ordering_table->tag & tag_mask) | ((s32)packet_cursor & address_mask));

            state_word = element->attr.word;
            working_word = state_word & 7;

            packet_cursor = (GosubGpuPacket*)((u8*)packet_cursor + 0x40);

            switch (working_word)
            {
            case GOSUB_ELEMENT_STATE_ENTERING:
                geometry = element->geometry.word;
                element_width = ((geometry & 1) << 8) | (state_word >> 24);
                exit_transition_step = (state_word >> 3) & 0xF;
                exit_scaled_width = element_width * exit_transition_step;
                if (exit_scaled_width < 0)
                {
                    exit_scaled_width += 7;
                }
                exit_full_height = (geometry >> 1) & 0xFF;
                exit_scaled_height = exit_full_height * exit_transition_step;
                animated_width = exit_scaled_width >> 3;
                if (exit_scaled_height < 0)
                {
                    exit_scaled_height += 7;
                }
                animated_height = exit_scaled_height >> 3;
                exit_remaining_height = (s32)(exit_full_height - animated_height);

                packet_cursor = ((GosubElementDrawHandler)element->draw_handler)(ordering_table, packet_cursor, (s32)(element_width - animated_width) / 2,
                                                                                 exit_remaining_height / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = element->attr.word;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    packet_cursor =
                        gosub_emit_panel(packet_cursor, (s32*)ordering_table, field + (s32)((((element->geometry.word & 1) << 8) | high) - animated_width) / 2,
                                         element->attr.f.y + ((s32)((element->geometry.word >> 1) & 0xFF) - animated_height) / 2, animated_width,
                                         animated_height, render_context->display_buffer_index);
                }
                entering_word = element->attr.word;
                updated_entering_word = entering_word & ~0x78;
                updated_entering_word |= (((((entering_word >> 3) & 0xF) + 1) & 0xF) * 8);
                element->attr.word = updated_entering_word;
                if (((updated_entering_word >> 3) & 0xF) == 8)
                {
                    element->attr.word = (updated_entering_word & ~7) | 2;
                }
                break;

            case GOSUB_ELEMENT_STATE_ACTIVE:
                packet_cursor = ((GosubElementDrawHandler)element->draw_handler)(ordering_table, packet_cursor, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = element->attr.word;
                    high = case_word >> 24;
                    packet_cursor = gosub_emit_panel(packet_cursor, (s32*)ordering_table, (case_word >> 7) & 0x1FF, element->attr.f.y,
                                                     ((element->geometry.word & 1) << 8) | high, (element->geometry.word >> 1) & 0xFF,
                                                     render_context->display_buffer_index);
                }
                break;

            case GOSUB_ELEMENT_STATE_EXITING:
                geometry = element->geometry.word;
                element_width = ((geometry & 1) << 8) | (state_word >> 24);
                transition_step = (state_word >> 3) & 0xF;
                scaled_width = element_width * transition_step;
                if (scaled_width < 0)
                {
                    scaled_width += 7;
                }
                full_height = (geometry >> 1) & 0xFF;
                scaled_height = full_height * transition_step;
                animated_width = scaled_width >> 3;
                if (scaled_height < 0)
                {
                    scaled_height += 7;
                }
                animated_height = scaled_height >> 3;
                remaining_height = (s32)(full_height - animated_height);

                packet_cursor = ((GosubElementDrawHandler)element->draw_handler)(ordering_table, packet_cursor, (s32)(element_width - animated_width) / 2,
                                                                                 remaining_height / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = element->attr.word;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    packet_cursor =
                        gosub_emit_panel(packet_cursor, (s32*)ordering_table, field + (s32)((((element->geometry.word & 1) << 8) | high) - animated_width) / 2,
                                         element->attr.f.y + ((s32)((element->geometry.word >> 1) & 0xFF) - animated_height) / 2, animated_width,
                                         animated_height, render_context->display_buffer_index);
                }
                exiting_word = element->attr.word;
                updated_exiting_word = (exiting_word & ~0x78) | (((((exiting_word >> 3) & 0xF) - 1) & 0xF) * 8);
                element->attr.word = updated_exiting_word;
                if (!((updated_exiting_word >> 3) & 0xF))
                {
                    element->attr.word = updated_exiting_word & ~7;
                }
                g_gosub_dialog_accepting_input = 0;
                break;
            }
        }

        element += 1;
    }

    render_context->packet_cursor = packet_cursor;
}

/**
 * @brief Emit an animated scroll arrow and its fill packet.
 * @param prim Destination packet buffer.
 * @param ot   Ordering-table tag for both packets.
 * @param x    Center X coordinate.
 * @param y    Center Y coordinate.
 * @param flag Selects the up vs down vertex arrangement.
 * @return Pointer to the next free packet slot.
 * @see decomp.me
 */
void* gosub_emit_scroll_marker(GosubScrollMarkerPacket* prim, s32* ot, s32 x, s32 y, s32 flag)
{
    s32 pulse_value;
    s32 working_y;
    u32 i;
    u32 addr_mask;
    u8* source_bytes;
    GosubScrollMarkerPacket* fill_packet;

    prim->len = 6;
    prim->code = 0x4C;
    prim->mask = 0x55555555;
    if (g_frame_counter & 0x10)
    {
        pulse_value = g_frame_counter & 0xF;
    }
    else
    {
        pulse_value = (~g_frame_counter) & 0xF;
    }
    working_y = pulse_value * 4;
    pulse_value = working_y + 0x70;
    prim->b = pulse_value;
    prim->g = pulse_value;
    prim->r = pulse_value;
    if (flag != 0)
    {
        pulse_value = y - 8;
        prim->y3 = pulse_value;
        prim->y0 = pulse_value;
        pulse_value = x - 6;
        working_y = y + 4;
        prim->x1 = pulse_value;
        prim->x3 = x;
        prim->x0 = x;
        prim->y1 = working_y;
        prim->x2 = x + 6;
        prim->y2 = working_y;
    }
    else
    {
        pulse_value = y + 8;
        prim->y3 = pulse_value;
        prim->y0 = pulse_value;
        pulse_value = x - 6;
        working_y = y - 4;
        prim->x1 = pulse_value;
        prim->x3 = x;
        prim->x0 = x;
        prim->y1 = working_y;
        prim->x2 = x + 6;
        prim->y2 = working_y;
    }

    addr_mask = 0xFFFFFF;
    source_bytes = (u8*)prim;
    fill_packet = (GosubScrollMarkerPacket*)(source_bytes + 0x1C);
    prim = fill_packet;
    *(u32*)source_bytes = (*(u32*)source_bytes & 0xFF000000) | (*ot & addr_mask);
    i = 0;
    *ot = (*ot & 0xFF000000) | ((u32)source_bytes & addr_mask);
    do
    {
        i += 1;
        *(u8*)prim = *source_bytes;
        source_bytes += 1;
        prim = (GosubScrollMarkerPacket*)((u8*)prim + 1);
    } while (i < 0x14U);

    fill_packet->len = 4;
    *(u32*)&fill_packet->r = 0;
    fill_packet->code = 0x20;
    *(u32*)fill_packet = (*(u32*)fill_packet & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((u32)fill_packet & 0xFFFFFF);
    return (u8*)fill_packet + 0x14;
}

/**
 * @brief Emit a clipped, framed gosub panel.
 * @param prim Destination packet buffer.
 * @param ot   Ordering-table tag for the panel packets.
 * @param x    Panel left edge.
 * @param y    Panel top edge.
 * @param w    Panel width.
 * @param h    Panel height.
 * @param flag Non-zero selects the lower frame-buffer half.
 * @return Pointer to the next free packet slot.
 * @see decomp.me
 */
GosubGpuPacket* gosub_emit_panel(GosubGpuPacket* prim, s32* ot, s32 x, s32 y, s32 w, s32 h, s32 flag)
{
    GosubGpuPacket* packet_cursor;
    GosubGpuPacket* draw_env_packet;
    DR_TPAGE* draw_mode_packet;
    s32 draw_env[24];
    s32 working_value;

    draw_env_packet = prim;
    if (flag != 0)
    {
        working_value = y + 0xF2;
        SetDefDrawEnv((DRAWENV*)draw_env, x + 2, working_value, w - 4, h - 4);
    }
    else
    {
        working_value = y + 0xA;
        SetDefDrawEnv((DRAWENV*)draw_env, x + 2, working_value, w - 4, h - 4);
    }
    SetDrawEnv((DR_ENV*)draw_env_packet, (DRAWENV*)draw_env);

    draw_env_packet->tag = (draw_env_packet->tag & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)draw_env_packet & 0xFFFFFF);

    draw_env_packet = (GosubGpuPacket*)((u8*)draw_env_packet + 0x40);
    packet_cursor = gosub_emit_panel_corners((SPRT*)draw_env_packet, ot, x, y, w, h);
    packet_cursor = (GosubGpuPacket*)gosub_emit_panel_outline((GosubLinePacket*)packet_cursor, ot, x, y, w, h, 0xFFFFFF);
    packet_cursor = (GosubGpuPacket*)gosub_emit_panel_outline((GosubLinePacket*)packet_cursor, ot, x + 1, y + 1, w - 2, h - 2, 0);
    packet_cursor = (GosubGpuPacket*)gosub_emit_panel_outline((GosubLinePacket*)packet_cursor, ot, x - 1, y - 1, w + 2, h + 2, 0);

    do
    {
        working_value = (s32)packet_cursor;
    } while (0);
    SET_BGR0_PACKED((TILE*)working_value, 0xC0C0C0);
    setTile((TILE*)working_value);
    setSemiTrans((TILE*)working_value, 1);
    setXY0((TILE*)working_value, x, y);
    setWH((TILE*)working_value, w, h);
    addPrim(ot, (TILE*)working_value);

    draw_mode_packet = (DR_TPAGE*)(working_value + sizeof(TILE));
    setDrawTPage(draw_mode_packet, 0, 0, 0x45);
    addPrim(ot, draw_mode_packet);
    return (GosubGpuPacket*)(working_value + sizeof(TILE) + sizeof(DR_TPAGE));
}

/**
 * @brief Emit a rectangle outline as four flat lines linked into @p ot.
 * @param line  Destination packet buffer.
 * @param ot    Ordering-table tag all four lines are linked into.
 * @param x     Rectangle left edge.
 * @param y     Rectangle top edge.
 * @param w     Rectangle width.
 * @param h     Rectangle height.
 * @param color Packed 0x00BBGGRR colour written to every line.
 * @return Pointer to the next free packet slot.
 * @see decomp.me
 */
GosubLinePacket* gosub_emit_panel_outline(GosubLinePacket* line, s32* ot, s32 x, s32 y, s32 w, s32 h, s32 color)
{
    *(u32*)&line->r0 = color;
    setLineF2(line);
    line->x0 = x + 4;
    line->y0 = y;
    line->x1 = (x + w) - 4;
    line->y1 = y;
    addPrim(ot, line);
    line++;

    *(u32*)&line->r0 = color;
    setLineF2(line);
    line->x0 = x + w;
    line->y0 = y + 4;
    line->x1 = x + w;
    line->y1 = (y + h) - 4;
    addPrim(ot, line);
    line++;

    *(u32*)&line->r0 = color;
    setLineF2(line);
    line->x0 = (x + w) - 4;
    line->y0 = y + h;
    line->x1 = x + 4;
    line->y1 = y + h;
    addPrim(ot, line);
    line++;

    *(u32*)&line->r0 = color;
    setLineF2(line);
    line->x0 = x;
    line->y0 = y + 4;
    line->x1 = x;
    line->y1 = (y + h) - 4;
    addPrim(ot, line);
    return line + 1;
}

/**
 * @brief Draw the gosub item list: one packet run per row, then the cursor
 *        highlight and one highlight per selected row.
 *
 * Rows dispatch on @c value: -3 is a full equipment card (icon strip via
 * gosub_draw_portrait plus three text lines and up to one status glyph), -2 is a
 * combination header (gosub_draw_composite_icon frame plus a label), anything else is a
 * plain label with an optional trailing glyph. Each row is culled against
 * g_gosub_window_height before any packet is emitted. The tail appends a
 * 0xF080F0 TILE for the cursor row and a 0x808080 TILE per entry of
 * g_gosub_selected_rows.
 *
 * @param ot       Ordering-table tag every packet is linked into.
 * @param initial_prim Packet cursor; copied into the local @c prim, which is what
 *                 the body advances.
 * @param x_off    Horizontal offset subtracted from every column position.
 * @param y_off    Vertical scroll offset subtracted from every row position.
 * @return Packet cursor just past the last highlight tile.
 * @see decomp.me
 */
GosubTilePacket* gosub_draw_item_list(s32* ot, s32 initial_prim, s32 x_off, s32 y_off)
{
    s32 prim;
    s32 drawn_count;
    GosubTextPosition* pos_p;
    s32 row_offset;
    GosubTextPosition pos;
    s32 row;
    s32 y;
    s32 line_y;
    s32 y_top;
    s32 sel_mul;
    s32 msg_off;
    u8* msg_p;
    s32 y_top2;
    s32 y_top3;
    s32 line_y2;
    s32 line_y3;
    s32 label_x;
    s32 x_pad;
    s32 status_pad;
    s32* table;
    s32 base;
    s32 archive_block;
    s32 detail_block;
    u8* archive_data;
    GosubTilePacket* tile;
    GosubTilePacket* mark;
    s32* cursor_p;
    s32* height_p;
    s32* scroll_p;
    u32 cursor_color;
    u32 addr_mask;

    prim = initial_prim;
    row = 0;
    drawn_count = 0;
    if (g_gosub_row_count > 0)
    {
        label_x = 0x30 - x_off;
        pos_p = &pos;
        row_offset = 0;
        archive_data = (u8*)&g_gosub_text_archive_0;
        do
        {
            table = &g_gosub_message_archive_offset;
            base = (s32)table - 0x20;
            if (g_gosub_rows[row].value == -3)
            {
                y = ((row * 0x30) - y_off) - g_gosub_scroll_y;
                if (y >= -0x2F && y < g_gosub_window_height)
                {
                    prim = func_800A88A0(gosub_draw_portrait(prim, ot, row, -x_off, y, drawn_count), ot, g_gosub_rows[row].name, g_gosub_rows[row].text_color,
                                         label_x, y, 0);
                    if (g_gosub_rows[row].flags.f.alternate_format)
                    {
                        if ((g_gosub_rows[row].flags.half & 1) == 0)
                        {
                            line_y = y + 0x10;
                            prim = func_800A88A0(prim, ot, MSG_HI(0x24), g_gosub_rows[row].text_color, label_x, line_y, 0);
                            pos.x = 0x54 - x_off;
                            pos.y = line_y;
                            prim = func_800A8A78(ot, prim, g_gosub_rows[row].detail_variant, g_gosub_rows[row].text_color, pos_p, 0);
                            detail_block = *(s32*)(base + 0x24);
                            prim = func_800A88A0(prim, ot, (void*)(detail_block + (*(u16*)((detail_block + g_gosub_rows[row].detail_id * 2) + base) + base)),
                                                 g_gosub_rows[row].text_color, 0x84 - x_off, line_y, 0);
                        }
                        else
                        {
                            msg_off =
                                *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset + g_gosub_rows[row].detail_variant * 2 + 0x44);
                            prim = func_800A88A0(prim, ot, (void*)(g_gosub_message_archive_offset + (msg_off + base)), g_gosub_rows[row].text_color, label_x,
                                                 y + 0x10, 0);
                        }
                    }
                    else
                    {
                        archive_block = g_gosub_text_archive_offsets_5;
                        prim = func_800A88A0(prim, ot, (void*)(archive_block + (*(u16*)((archive_block + g_gosub_rows[row].detail_id * 2) + base) + base)),
                                             g_gosub_rows[row].text_color, label_x, y + 0x10, 0);
                    }
                    line_y2 = y + 0x20;
                    prim = func_800A88A0(prim, ot, MSG_HI(0x26), g_gosub_rows[row].text_color, label_x, line_y2, 0);
                    pos.x = 0x48 - x_off;
                    pos.y = line_y2;
                    prim = func_800A8A78(ot, prim, g_gosub_rows[row].secondary_value, g_gosub_rows[row].text_color, pos_p, 0);
                    msg_off = g_gosub_message_archive_offset - -(*(u16*)((s32)g_gosub_message_archive_offset - -(s32)archive_data) + base);
                    prim = func_800A88A0(prim, ot, (void*)msg_off, g_gosub_rows[row].text_color, 0x64 - x_off, line_y2, 0);
                    pos.x = 0xB0 - x_off;
                    pos.y = line_y2;
                    prim = func_800A8A78(ot, prim, g_gosub_rows[row].primary_value, g_gosub_rows[row].text_color, pos_p, 0);
                    if (g_gosub_rows[row].detail_group != 0)
                    {
                        s32 right_padding;
                        prim = func_800A88A0(prim, ot, MSG_LO(0x4A), g_gosub_rows[row].text_color,
                                             g_gosub_window_width - (right_padding = x_off, right_padding += 0xC), line_y2, 1);
                    }
                    else if (g_gosub_rows[row].flags.half & 1)
                    {
                        s32 right_padding;
                        prim = func_800A88A0(prim, ot, MSG_LO(0x60), g_gosub_rows[row].text_color,
                                             g_gosub_window_width - (right_padding = x_off, right_padding += 0xC), line_y2, 1);
                    }
                    else if (g_gosub_rows[row].flags.f.selection_restricted)
                    {
                        s32 right_padding;
                        prim = func_800A88A0(prim, ot, MSG_LO(0x6E), g_gosub_rows[row].text_color,
                                             g_gosub_window_width - (right_padding = x_off, right_padding += 0xC), line_y2, 1);
                    }
                    drawn_count += 1;
                }
            }
            else if (g_gosub_rows[row].value == -2)
            {
                y = (row_offset - y_off) - g_gosub_scroll_y;
                if (y >= -0x1F && y < g_gosub_window_height)
                {
                    line_y3 = y + 8;
                    prim = gosub_draw_composite_icon(prim, ot, 0xC - x_off, y, g_gosub_rows[row].detail_group, g_gosub_rows[row].detail_variant);
                    prim = func_800A88A0(prim, ot, g_gosub_rows[row].name, g_gosub_rows[row].text_color, 0x4C - x_off, line_y3, 0);
                    if (g_gosub_rows[row].flags.f.alternate_format)
                    {
                        prim = func_800A88A0(prim, ot, MSG_HI(0x20), g_gosub_rows[row].text_color, 0x110 - x_off, line_y3, 1);
                    }
                }
            }
            else
            {
                status_pad = row * g_gosub_row_height;
                y_top = y_off - 2;
                y = (status_pad - y_top) - g_gosub_scroll_y;
                if (-g_gosub_row_height < y && y < g_gosub_window_height)
                {
                    prim = func_800A88A0(prim, ot, g_gosub_rows[row].name, g_gosub_rows[row].text_color, 0xC - x_off, y, 0);
                    pos.y = y;
                    x_pad = x_off + 0xC;
                    pos.x = g_gosub_window_width - x_pad;
                    if (g_gosub_rows[row].value >= 0)
                    {
                        prim = func_800A8A78(ot, prim, g_gosub_rows[row].value, g_gosub_rows[row].text_color, pos_p, 1);
                    }
                }
            }
            row_offset += 0x20;
            row += 1;
        } while (row < g_gosub_row_count);
    }

    tile = (GosubTilePacket*)prim;
    cursor_color = 0xF080F0;
    addr_mask = 0xFFFFFF;
    for (;;)
    {
        cursor_p = &g_gosub_cursor_row;
        height_p = &g_gosub_row_height;
        scroll_p = &g_gosub_scroll_y;
        break;
    }
    mark = tile + 1;
    status_pad = *cursor_p * *height_p;
    y_top2 = y_off - 2;
    y = (status_pad - y_top2) - *scroll_p;
    ((u8*)tile)[3] = 3;
    tile->color = cursor_color;
    ((u8*)tile)[7] = 0x62;
    row = 0;
    tile->w = g_gosub_window_width;
    tile->y = y - 2;
    tile->x = 1;
    tile->h = g_gosub_row_height - 1;
    setaddr(tile, getaddr(ot) & addr_mask);
    setaddr(ot, tile);
    while (row < g_gosub_selection_count)
    {
        {
            mark->x = (row | 1) & 1;
            mark->color = 0x808080;
            ((u8*)mark)[3] = 3;
            ((u8*)mark)[7] = 0x62;
            sel_mul = g_gosub_selected_rows[row] * g_gosub_row_height;
            y_top3 = y_off - 2;
            y = (sel_mul - y_top3) - g_gosub_scroll_y;
            mark->w = g_gosub_window_width;
            mark->y = y - 2;
            mark->h = g_gosub_row_height - 1;
            row += 1;
            ADD_PRIM_MASKED(ot, mark);
            mark += 1;
        }
    }
    return mark;
}

/**
 * @brief Upload a row's portrait strip and CLUT, then emit its 48x48 sprite.
 *
 * The portrait index comes from the row's detail id (offset by 0x41 for the
 * alternate record format); the pixel and CLUT rectangles are double-buffered by
 * g_gosub_frame_parity. Draws nothing once five portraits are on screen.
 *
 * @param prim  Packet cursor.
 * @param ot    Ordering-table tag to link into.
 * @param row   g_gosub_rows index to portray.
 * @param x     Sprite left edge.
 * @param y     Sprite top edge.
 * @param count How many portraits were already emitted this frame.
 * @return Packet cursor past the sprite (gosub_finish_glyph_run's return), or prim
 *         when count is 5 or more.
 * @see decomp.me
 */
s32 gosub_draw_portrait(s32 prim, s32* ot, s32 row, s32 x, s32 y, s32 count)
{
    SPRT* sprt;
    RECT rect;
    s32 idx;
    s32 cell;
    s32 n;

    if (count >= 5)
    {
        return prim;
    }

    if (g_gosub_rows[row].flags.f.alternate_format)
    {
        idx = g_gosub_rows[row].detail_id;
    }
    else
    {
        idx = g_gosub_rows[row].detail_id + 0x41;
    }
    n = count;
    cell = n * 3;

    rect.x = cell * 4 + 0x140;
    rect.w = 0xC;
    rect.h = 0x30;
    rect.y = g_gosub_frame_parity * 0x30;
    LoadImage(&rect, (u_long*)((u8*)g_gosub_portrait_archive + g_gosub_portrait_archive[idx] + 0x1C));

    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    n = n * 0x10;
    rect.x = n + g_gosub_frame_parity * 0x50;
    LoadImage(&rect, (u_long*)((u8*)g_gosub_portrait_archive + g_gosub_portrait_archive[idx] - 4));

    sprt = (SPRT*)prim;
    SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
    setSprt(sprt);
    sprt->u0 = cell * 0x10;
    sprt->x0 = x;
    sprt->v0 = g_gosub_frame_parity * 0x30;
    sprt->y0 = y;
    sprt->w = 0x30;
    sprt->h = 0x30;
    sprt->clut = (((n + g_gosub_frame_parity * 0x50) >> 4) & 0x3F) | 0x7C80;
    addPrim(ot, sprt);
    return gosub_finish_glyph_run(prim + 0x14, ot);
}

/**
 * @brief Draw the combination preview for the cursor row.
 *
 * When at least one row is selected and the cursor sits on a different row,
 * equipment_combination_find is asked whether the two rows' item indices combine. It
 * returns the resulting item in g_gosub_combination_result_id and fills g_gosub_combination_quantity and
 * g_gosub_combination_variant; a zero result means the pair does not combine and nothing is
 * drawn. Otherwise a frame is emitted, then the result's archive name, with
 * g_gosub_combination_quantity's decimal form appended after a separator when it is nonzero.
 *
 * @param ot       Ordering-table tag every packet is linked into.
 * @param initial_prim Packet cursor.
 * @param x_off    Horizontal offset subtracted from every column position.
 * @param y_off    Vertical offset subtracted from every row position.
 * @return Packet cursor past the last packet, or the incoming cursor when
 *         there is no combination to show.
 *
 * @see decomp.me
 */
s32 gosub_draw_combination_preview(s32* ot, s32 initial_prim, s32 x_off, s32 y_off)
{
    s32 stack_pad[2];
    u8 result_name[0x50];
    u8 number_text[0x50];
    s32 pair[3];
    s32 base;
    u8* name_cursor;
    u8* archive;
    s32 block_offset;
    s32 prim;

    g_gosub_combination_result_id = 0;
    prim = initial_prim;
    if (g_gosub_selection_count != 0)
    {
        if (g_gosub_cursor_row != g_gosub_selected_rows[0])
        {
            pair[0] = g_gosub_rows[g_gosub_selected_rows[0]].index;
            pair[1] = g_gosub_rows[g_gosub_cursor_row].index;
            g_gosub_combination_result_id = equipment_combination_find(pair, &g_gosub_combination_quantity, &g_gosub_combination_variant);
        }
    }
    if (g_gosub_combination_result_id != 0)
    {
        prim = gosub_draw_composite_icon(prim, ot, 0xC - x_off, -y_off, g_gosub_combination_result_id, g_gosub_combination_variant);
        name_cursor = result_name;
        archive = (u8*)g_gosub_text_archive_offsets_3;
        base = (s32)archive;
        base -= 0x18;
        block_offset = g_gosub_text_archive_offsets_3[0];
        gosub_copy_encoded_string(name_cursor, (u8*)(block_offset + (*(u16*)(g_gosub_combination_result_id * 2 + block_offset + base) + base)));
        if (g_gosub_combination_quantity != 0)
        {
            gosub_append_encoded_string(name_cursor, D_800EC3DA - 0x16 + D_800EC3DA[0] + (D_800EC3DA[1] << 8));
            func_800A8B90(number_text, g_gosub_combination_quantity, 1);
            gosub_append_encoded_string(name_cursor, number_text);
        }
        prim = func_800A88A0(prim, ot, name_cursor, 4, 0x4C - x_off, 0xA - y_off, 0);
    }
    return prim;
}
