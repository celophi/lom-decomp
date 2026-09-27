#include "gosub_internal.h"

/**
 * @brief Width of an element: the low eight bits live in the top byte of
 *        GosubElement::attr, bit 8 in GosubElement::geometry.
 * @param element Element to read.
 * @return Width in pixels.
 */
static inline s32 gosub_element_width(GosubElement* element)
{
    u32 width_low = element->attr.word >> 24;

    return (element->geometry.f.width_high << 8) | width_low;
}

/**
 * @brief Close every element that is in use.
 * @note The state is cleared rather than set to GOSUB_ELEMENT_STATE_EXITING, so
 *       the elements vanish at once; the transition step is left at 8, where an
 *       exit animation would start.
 * @see decomp.me (100%) https://decomp.me/scratch/RsBVl
 */
void gosub_close_elements(void)
{
    GosubElement* element;
    s32 i;

    element = g_gosub_elements;
    for (i = 0; i < GOSUB_ELEMENT_COUNT; i++)
    {
        if (element->attr.f.state != GOSUB_ELEMENT_STATE_INACTIVE)
        {
            element->attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
            element->attr.f.transition_step = 8;
        }
        element++;
    }
}

/**
 * @brief Render and animate all allocated gosub elements.
 * @param render_context Field render context and packet cursor.
 * @see decomp.me (100%) https://decomp.me/scratch/nVefu
 */
void gosub_render_elements(GosubRenderContext* render_context)
{
    gosub_update_and_render_elements(render_context);
}

/**
 * @brief Mark every gosub element slot inactive.
 * @see decomp.me (100%) https://decomp.me/scratch/dib6Q
 */
void gosub_clear_elements(void)
{
    GosubElement* element;
    s32 i;

    element = g_gosub_elements;
    for (i = 0; i < GOSUB_ELEMENT_COUNT; i++)
    {
        element->attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
        element++;
    }
}

/**
 * @brief Allocate the first inactive element after reserved element 0.
 * @return The element, now entering; element 0 when every other slot is in use.
 * @see decomp.me (100%) https://decomp.me/scratch/X1pXK
 */
GosubElement* gosub_allocate_element(void)
{
    GosubElement* element;
    s32 i;

    element = &g_gosub_elements[1];
    for (i = 1; i < GOSUB_ELEMENT_COUNT; i++)
    {
        if (element->attr.f.state == GOSUB_ELEMENT_STATE_INACTIVE)
        {
            element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
            return element;
        }
        element++;
    }

    return &g_gosub_elements[0];
}

/**
 * @brief Animate, draw, frame, and link every allocated gosub element.
 * @param render_context Field render context and packet cursor.
 * @see decomp.me (100%) https://decomp.me/scratch/t79hi
 */
void gosub_update_and_render_elements(GosubRenderContext* render_context)
{
    GosubTilePacket* packet_cursor;
    s32 animated_width;
    s32 animated_height;
    s32* ordering_table;
    GosubElement* element;
    s32 element_index;
    DRAWENV draw_env;
    u32 geometry_word;
    s32 element_height;
    u16 visible_height;
    u32 geometry;
    u32 element_width;
    u32 element_word;
    s32 element_height_calc;
    s32 content_height;
    u32 state_word;
    s32 working_word;
    s32 entering_step;
    s32 entering_scaled_width;
    s32 entering_full_height;
    s32 entering_scaled_height;
    s32 entering_remaining_height;
    s32 exiting_step;
    s32 exiting_scaled_width;
    s32 exiting_full_height;
    s32 exiting_scaled_height;
    s32 exiting_remaining_height;

    packet_cursor = render_context->packet_cursor;
    ordering_table = &render_context->tag;

    if (render_context->display_buffer_index != 0)
    {
        SetDefDrawEnv(&draw_env, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        SetDefDrawEnv(&draw_env, 0, 8, 0x140, 0xE0);
    }

    element = g_gosub_elements;
    element_index = 0;

    for (; element_index < GOSUB_ELEMENT_COUNT; element_index++)
    {
        element_word = element->attr.word;
        if (element->attr.f.state != GOSUB_ELEMENT_STATE_INACTIVE)
        {
            if ((GosubElementDrawHandler)element->draw_handler == (GosubElementDrawHandler)gosub_draw_item_list)
            {
                geometry_word = element->geometry.word;
                element_height = (geometry_word >> 1) & 0xFF;

                content_height = g_gosub_row_count * g_gosub_row_height;
                if ((g_gosub_scroll_y + element_height) < content_height)
                {
                    /* Uses the words read at the top of the loop, as the original does. */
                    u32 x = (element_word >> 7) & 0x1FF;
                    u32 width_low = element_word >> 24;

                    packet_cursor = gosub_emit_scroll_marker((GosubScrollMarkerPacket*)packet_cursor, ordering_table,
                                                             (x + (((geometry_word & 1) << 8) | width_low)) - 16, element->attr.f.y + element_height, 0);
                }
                if (g_gosub_scroll_y != 0)
                {
                    s32 x = element->attr.f.x;
                    s32 width = gosub_element_width(element);

                    packet_cursor = gosub_emit_scroll_marker((GosubScrollMarkerPacket*)packet_cursor, ordering_table, x + width - 16, element->attr.f.y, 1);
                }
                SetDrawEnv((DR_ENV*)packet_cursor, &draw_env);
                addPrim(ordering_table, packet_cursor);
                packet_cursor = (GosubTilePacket*)((DR_ENV*)packet_cursor + 1);

                if (g_gosub_row_count != 0)
                {
                    packet_cursor->color = 0xFFFF00;
                    setTile(packet_cursor);
                    packet_cursor->w = 6;
                    element_height_calc = (element->geometry.word >> 1) & 0xFF;
                    {
                        /* The volatile pointer is required: the original reloads h after storing it. */
                        volatile u16* bar_height;
                        s32 initial_height;

                        bar_height = &packet_cursor->h;
                        *bar_height = (element_height_calc * (element_height_calc / g_gosub_row_height)) / g_gosub_row_count;
                        initial_height = (s16)*bar_height;
                        working_word = element->geometry.word;
                        visible_height = ((u32)working_word >> 1) & 0xFF;
                        if (initial_height >= visible_height - 2)
                        {
                            *bar_height = visible_height;
                        }
                    }
                    packet_cursor->x = 1;
                    packet_cursor->y = (element->geometry.f.height * (g_gosub_scroll_y / g_gosub_row_height)) / g_gosub_row_count;
                    addPrim(ordering_table, packet_cursor);
                    packet_cursor++;
                }
                {
                    s32 x = element->attr.f.x;
                    s32 width = gosub_element_width(element);

                    packet_cursor = gosub_emit_panel(packet_cursor, ordering_table, x + width + 3, element->attr.f.y, 10, element->geometry.f.height,
                                                     render_context->display_buffer_index);
                }
            }
            SetDrawEnv((DR_ENV*)packet_cursor, &draw_env);
            addPrim(ordering_table, packet_cursor);

            state_word = element->attr.word;

            packet_cursor = (GosubTilePacket*)((DR_ENV*)packet_cursor + 1);

            switch (element->attr.f.state)
            {
            case GOSUB_ELEMENT_STATE_ENTERING:
                geometry = element->geometry.word;
                element_width = ((geometry & 1) << 8) | (state_word >> 24);
                entering_step = (state_word >> 3) & 0xF;
                entering_scaled_width = element_width * entering_step;
                if (entering_scaled_width < 0)
                {
                    entering_scaled_width += 7;
                }
                entering_full_height = (geometry >> 1) & 0xFF;
                entering_scaled_height = entering_full_height * entering_step;
                animated_width = entering_scaled_width >> 3;
                if (entering_scaled_height < 0)
                {
                    entering_scaled_height += 7;
                }
                animated_height = entering_scaled_height >> 3;
                entering_remaining_height = (s32)(entering_full_height - animated_height);

                packet_cursor = ((GosubElementDrawHandler)element->draw_handler)(ordering_table, packet_cursor, (s32)(element_width - animated_width) / 2,
                                                                                 entering_remaining_height / 2);
                {
                    s32 x = element->attr.f.x;
                    s32 width = gosub_element_width(element);

                    packet_cursor = gosub_emit_panel(packet_cursor, ordering_table, x + (width - animated_width) / 2,
                                                     element->attr.f.y + ((s32)element->geometry.f.height - animated_height) / 2, animated_width,
                                                     animated_height, render_context->display_buffer_index);
                }
                element->attr.f.transition_step++;
                if (element->attr.f.transition_step == 8)
                {
                    element->attr.f.state = GOSUB_ELEMENT_STATE_ACTIVE;
                }
                break;

            case GOSUB_ELEMENT_STATE_ACTIVE:
                packet_cursor = ((GosubElementDrawHandler)element->draw_handler)(ordering_table, packet_cursor, 0, 0);
                packet_cursor = gosub_emit_panel(packet_cursor, ordering_table, element->attr.f.x, element->attr.f.y, gosub_element_width(element),
                                                 element->geometry.f.height, render_context->display_buffer_index);
                break;

            case GOSUB_ELEMENT_STATE_EXITING:
                geometry = element->geometry.word;
                element_width = ((geometry & 1) << 8) | (state_word >> 24);
                exiting_step = (state_word >> 3) & 0xF;
                exiting_scaled_width = element_width * exiting_step;
                if (exiting_scaled_width < 0)
                {
                    exiting_scaled_width += 7;
                }
                exiting_full_height = (geometry >> 1) & 0xFF;
                exiting_scaled_height = exiting_full_height * exiting_step;
                animated_width = exiting_scaled_width >> 3;
                if (exiting_scaled_height < 0)
                {
                    exiting_scaled_height += 7;
                }
                animated_height = exiting_scaled_height >> 3;
                exiting_remaining_height = (s32)(exiting_full_height - animated_height);

                packet_cursor = ((GosubElementDrawHandler)element->draw_handler)(ordering_table, packet_cursor, (s32)(element_width - animated_width) / 2,
                                                                                 exiting_remaining_height / 2);
                {
                    s32 x = element->attr.f.x;
                    s32 width = gosub_element_width(element);

                    packet_cursor = gosub_emit_panel(packet_cursor, ordering_table, x + (width - animated_width) / 2,
                                                     element->attr.f.y + ((s32)element->geometry.f.height - animated_height) / 2, animated_width,
                                                     animated_height, render_context->display_buffer_index);
                }
                element->attr.f.transition_step--;
                if (element->attr.f.transition_step == 0)
                {
                    element->attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
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
 */
void* gosub_emit_scroll_marker(GosubScrollMarkerPacket* prim, s32* ot, s32 x, s32 y, s32 flag)
{
    s32 pulse_value;
    s32 working_y;
    u32 i;
    u32 addr_mask;
    u8* source_bytes;
    GosubScrollFillPacket* fill_packet;

    setlen(prim, 6);
    setcode(prim, 0x4C);
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
    fill_packet = (GosubScrollFillPacket*)(prim + 1);
    /* prim doubles as the copy's destination cursor; a separate local changes register allocation. */
    prim = (GosubScrollMarkerPacket*)fill_packet;
    setaddr(source_bytes, getaddr(ot) & addr_mask);
    i = 0;
    setaddr(ot, (u32)source_bytes & addr_mask);
    do
    {
        i += 1;
        *(u8*)prim = *source_bytes;
        source_bytes += 1;
        prim = (GosubScrollMarkerPacket*)((u8*)prim + 1);
    } while (i < sizeof(GosubScrollFillPacket));

    setlen(fill_packet, 4);
    SET_BGR0_PACKED(fill_packet, 0);
    setcode(fill_packet, 0x20);
    addPrim(ot, fill_packet);
    return fill_packet + 1;
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
 */
GosubTilePacket* gosub_emit_panel(GosubTilePacket* prim, s32* ot, s32 x, s32 y, s32 w, s32 h, s32 flag)
{
    DR_ENV* draw_env_packet;
    GosubLinePacket* outline;
    TILE* fill;
    DR_TPAGE* draw_mode_packet;
    DRAWENV draw_env;
    s32 working_value;

    draw_env_packet = (DR_ENV*)prim;
    if (flag != 0)
    {
        working_value = y + 0xF2;
        SetDefDrawEnv(&draw_env, x + 2, working_value, w - 4, h - 4);
    }
    else
    {
        working_value = y + 0xA;
        SetDefDrawEnv(&draw_env, x + 2, working_value, w - 4, h - 4);
    }
    SetDrawEnv(draw_env_packet, &draw_env);
    addPrim(ot, draw_env_packet);
    draw_env_packet++;

    outline = (GosubLinePacket*)gosub_emit_panel_corners((SPRT*)draw_env_packet, ot, x, y, w, h);
    outline = gosub_emit_panel_outline(outline, ot, x, y, w, h, 0xFFFFFF);
    outline = gosub_emit_panel_outline(outline, ot, x + 1, y + 1, w - 2, h - 2, 0);
    outline = gosub_emit_panel_outline(outline, ot, x - 1, y - 1, w + 2, h + 2, 0);
    /* Reusing the clip-y slot for the fill pointer and the do/while(0) barrier are both required to match. */
    do
    {
        working_value = (s32)outline;
    } while (0);
    fill = (TILE*)working_value;

    SET_BGR0_PACKED(fill, 0xC0C0C0);
    setTile(fill);
    setSemiTrans(fill, 1);
    setXY0(fill, x, y);
    setWH(fill, w, h);
    addPrim(ot, fill);

    draw_mode_packet = (DR_TPAGE*)(fill + 1);
    setDrawTPage(draw_mode_packet, 0, 0, 0x45);
    addPrim(ot, draw_mode_packet);
    return (GosubTilePacket*)(draw_mode_packet + 1);
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
 */
GosubLinePacket* gosub_emit_panel_outline(GosubLinePacket* line, s32* ot, s32 x, s32 y, s32 w, s32 h, s32 color)
{
    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY0(line, x + 4, y);
    line->x1 = (x + w) - 4;
    line->y1 = y;
    addPrim(ot, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY0(line, x + w, y + 4);
    line->x1 = x + w;
    line->y1 = (y + h) - 4;
    addPrim(ot, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY0(line, (x + w) - 4, y + h);
    line->x1 = x + 4;
    line->y1 = y + h;
    addPrim(ot, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY0(line, x, y + 4);
    line->x1 = x;
    line->y1 = (y + h) - 4;
    addPrim(ot, line);
    return line + 1;
}

/**
 * @brief Draw the visible rows of the list, then the cursor and selection highlights.
 *
 * GOSUB_ROW_COMPANION rows show a portrait and three lines of pet or golem
 * details, GOSUB_ROW_LOGIC_BLOCK rows the block's icon and name, and plain
 * rows a name with an optional count.
 *
 * @param ot Ordering table to receive the packets.
 * @param initial_prim First free GPU packet.
 * @param x_off Horizontal element animation offset.
 * @param y_off Vertical element animation offset.
 * @return Packet cursor after the last highlight.
 */
GosubTilePacket* gosub_draw_item_list(s32* ot, s32 initial_prim, s32 x_off, s32 y_off)
{
    s32 prim;
    s32 drawn_count;
    GosubTextPosition* pos_p;
    GosubTextPosition pos;
    s32 row;
    s32 y;
    s32 y_top;
    s32 sel_mul;
    s32 y_top2;
    s32 y_top3;
    s32 label_x;
    s32 status_pad;
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
        do
        {
            if (g_gosub_rows[row].value == GOSUB_ROW_COMPANION)
            {
                y = ((row * 0x30) - y_off) - g_gosub_scroll_y;
                if (y >= -0x2F && y < g_gosub_window_height)
                {
                    prim = field_draw_text(gosub_draw_portrait(prim, ot, row, -x_off, y, drawn_count), ot, g_gosub_rows[row].name, g_gosub_rows[row].text_color,
                                           label_x, y, 0);
                    if (g_gosub_rows[row].flags.companion.pet)
                    {
                        if ((g_gosub_rows[row].flags.half & 1) == 0)
                        {
                            prim = field_draw_text(prim, ot, GOSUB_MESSAGE(GOSUB_MSG_LEVEL), g_gosub_rows[row].text_color, label_x, y + 0x10, 0);
                            pos.x = 0x54 - x_off;
                            pos.y = y + 0x10;
                            prim = field_draw_number(ot, prim, g_gosub_rows[row].detail_variant, g_gosub_rows[row].text_color, pos_p, 0);
                            prim = field_draw_text(prim, ot, GOSUB_TEXT(GOSUB_TEXT_PET_SPECIES, g_gosub_rows[row].detail_id), g_gosub_rows[row].text_color,
                                                   0x84 - x_off, y + 0x10, 0);
                        }
                        else
                        {
                            prim = field_draw_text(prim, ot, GOSUB_MESSAGE(GOSUB_MSG_HATCH_ANY_TIME + g_gosub_rows[row].detail_variant),
                                                   g_gosub_rows[row].text_color, label_x, y + 0x10, 0);
                        }
                    }
                    else
                    {
                        prim = field_draw_text(prim, ot, GOSUB_TEXT(GOSUB_TEXT_GOLEM_TYPES, g_gosub_rows[row].detail_id), g_gosub_rows[row].text_color, label_x,
                                               y + 0x10, 0);
                    }
                    prim = field_draw_text(prim, ot, GOSUB_MESSAGE(GOSUB_MSG_HP), g_gosub_rows[row].text_color, label_x, y + 0x20, 0);
                    pos.x = GOSUB_CARD_SECONDARY_VALUE_X - x_off;
                    pos.y = y + 0x20;
                    prim = field_draw_number(ot, prim, g_gosub_rows[row].secondary_value, g_gosub_rows[row].text_color, pos_p, 0);
                    prim = field_draw_text(prim, ot, GOSUB_MESSAGE(GOSUB_MSG_ATTACK_POWER), g_gosub_rows[row].text_color, GOSUB_CARD_VALUE_LABEL_X - x_off,
                                           y + 0x20, 0);
                    pos.x = GOSUB_CARD_PRIMARY_VALUE_X - x_off;
                    pos.y = y + 0x20;
                    prim = field_draw_number(ot, prim, g_gosub_rows[row].primary_value, g_gosub_rows[row].text_color, pos_p, 0);
                    if (g_gosub_rows[row].detail_group != 0)
                    {
                        prim = field_draw_text(prim, ot, GOSUB_MESSAGE(GOSUB_MSG_ON_FIELD), g_gosub_rows[row].text_color, g_gosub_window_width - 0xC - x_off,
                                               y + 0x20, 1);
                    }
                    else if (g_gosub_rows[row].flags.half & 1)
                    {
                        prim = field_draw_text(prim, ot, GOSUB_MESSAGE(GOSUB_MSG_MONSTER_EGG), g_gosub_rows[row].text_color, g_gosub_window_width - 0xC - x_off,
                                               y + 0x20, 1);
                    }
                    else if (g_gosub_rows[row].flags.companion.grazing)
                    {
                        prim = field_draw_text(prim, ot, GOSUB_MESSAGE(GOSUB_MSG_GRAZING), g_gosub_rows[row].text_color, g_gosub_window_width - 0xC - x_off,
                                               y + 0x20, 1);
                    }
                    drawn_count += 1;
                }
            }
            else if (g_gosub_rows[row].value == GOSUB_ROW_LOGIC_BLOCK)
            {
                y = row * 32 - y_off - g_gosub_scroll_y;
                if (y >= -0x1F && y < g_gosub_window_height)
                {
                    prim = gosub_draw_composite_icon(prim, ot, 0xC - x_off, y, g_gosub_rows[row].detail_group, g_gosub_rows[row].detail_variant);
                    prim = field_draw_text(prim, ot, g_gosub_rows[row].name, g_gosub_rows[row].text_color, 0x4C - x_off, y + 8, 0);
                    if (g_gosub_rows[row].flags.block.in_use)
                    {
                        prim = field_draw_text(prim, ot, GOSUB_MESSAGE(GOSUB_MSG_IN_USE), g_gosub_rows[row].text_color, 0x110 - x_off, y + 8, 1);
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
                    prim = field_draw_text(prim, ot, g_gosub_rows[row].name, g_gosub_rows[row].text_color, 0xC - x_off, y, 0);
                    pos.y = y;
                    pos.x = g_gosub_window_width - 0xC - x_off;
                    if (g_gosub_rows[row].value >= 0)
                    {
                        prim = field_draw_number(ot, prim, g_gosub_rows[row].value, g_gosub_rows[row].text_color, pos_p, 1);
                    }
                }
            }
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
    setlen(tile, 3);
    tile->color = cursor_color;
    setcode(tile, 0x62);
    row = 0;
    tile->w = g_gosub_window_width;
    tile->y = y - 2;
    tile->x = 1;
    tile->h = g_gosub_row_height - 1;
    setaddr(tile, getaddr(ot) & addr_mask);
    setaddr(ot, tile);
    while (row < g_gosub_selection_count)
    {
        mark->x = (row | 1) & 1;
        mark->color = 0x808080;
        setlen(mark, 3);
        setcode(mark, 0x62);
        sel_mul = g_gosub_selected_rows[row] * g_gosub_row_height;
        y_top3 = y_off - 2;
        y = (sel_mul - y_top3) - g_gosub_scroll_y;
        mark->w = g_gosub_window_width;
        mark->y = y - 2;
        mark->h = g_gosub_row_height - 1;
        row += 1;
        addPrim(ot, mark);
        mark += 1;
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
 */
s32 gosub_draw_portrait(s32 prim, s32* ot, s32 row, s32 x, s32 y, s32 count)
{
    SPRT* sprite;
    RECT rect;
    s32 portrait;

    if (count >= GOSUB_PORTRAIT_SLOTS)
    {
        return prim;
    }

    if (g_gosub_rows[row].flags.companion.pet)
    {
        portrait = g_gosub_rows[row].detail_id;
    }
    else
    {
        portrait = g_gosub_rows[row].detail_id + GOSUB_GOLEM_PORTRAIT_FIRST;
    }

    rect.x = count * GOSUB_PORTRAIT_VRAM_WIDTH + GOSUB_PORTRAIT_VRAM_X;
    rect.w = GOSUB_PORTRAIT_VRAM_WIDTH;
    rect.h = GOSUB_PORTRAIT_SIZE;
    rect.y = g_gosub_frame_parity * GOSUB_PORTRAIT_SIZE;
    LoadImage(&rect, (u_long*)((u8*)g_gosub_portrait_archive + g_gosub_portrait_archive[portrait] + GOSUB_PORTRAIT_PIXEL_OFFSET));

    rect.y = GOSUB_GLYPH_CLUT_Y;
    rect.w = GOSUB_PORTRAIT_CLUT_SIZE;
    rect.h = 1;
    rect.x = count * GOSUB_PORTRAIT_CLUT_SIZE + g_gosub_frame_parity * (GOSUB_PORTRAIT_SLOTS * GOSUB_PORTRAIT_CLUT_SIZE);
    LoadImage(&rect, (u_long*)((u8*)g_gosub_portrait_archive + g_gosub_portrait_archive[portrait] + GOSUB_PORTRAIT_CLUT_OFFSET));

    sprite = (SPRT*)prim;
    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    sprite->u0 = count * GOSUB_PORTRAIT_SIZE;
    sprite->x0 = x;
    sprite->v0 = g_gosub_frame_parity * GOSUB_PORTRAIT_SIZE;
    sprite->y0 = y;
    setWH(sprite, GOSUB_PORTRAIT_SIZE, GOSUB_PORTRAIT_SIZE);
    setClut(sprite, count * GOSUB_PORTRAIT_CLUT_SIZE + g_gosub_frame_parity * (GOSUB_PORTRAIT_SLOTS * GOSUB_PORTRAIT_CLUT_SIZE), GOSUB_GLYPH_CLUT_Y);
    addPrim(ot, sprite);
    return gosub_finish_glyph_run((s32)(sprite + 1), ot);
}

/**
 * @brief Draw the logic block that the selected component and the cursor row would make.
 *
 * equipment_combination_find fills g_gosub_block_id, g_gosub_block_level and
 * g_gosub_block_shape; nothing is drawn when the pair makes no block.
 *
 * @param ot Ordering table to receive the packets.
 * @param initial_prim First free GPU packet.
 * @param x_off Horizontal element animation offset.
 * @param y_off Vertical element animation offset.
 * @return Packet cursor after the preview.
 */
s32 gosub_draw_block_preview(s32* ot, s32 initial_prim, s32 x_off, s32 y_off)
{
    s32 unused[2]; /* never used, but the original stack frame has room for it */
    u8 result_name[0x50];
    u8 number_text[0x50];
    s32 pair[3];
    u8* name_cursor;
    s32 prim;

    g_gosub_block_id = 0;
    prim = initial_prim;
    if (g_gosub_selection_count != 0)
    {
        if (g_gosub_cursor_row != g_gosub_selected_rows[0])
        {
            pair[0] = g_gosub_rows[g_gosub_selected_rows[0]].index;
            pair[1] = g_gosub_rows[g_gosub_cursor_row].index;
            g_gosub_block_id = equipment_combination_find(pair, &g_gosub_block_level, &g_gosub_block_shape);
        }
    }
    if (g_gosub_block_id != 0)
    {
        prim = gosub_draw_composite_icon(prim, ot, 0xC - x_off, -y_off, g_gosub_block_id, g_gosub_block_shape);
        name_cursor = result_name;
        gosub_copy_encoded_string(name_cursor, GOSUB_TEXT(GOSUB_TEXT_LOGIC_BLOCK_NAMES, g_gosub_block_id));
        if (g_gosub_block_level != 0)
        {
            gosub_append_encoded_string(name_cursor, FIELD_UI_TEXT_AT(D_800EC3DA, FIELD_UI_TEXT_PLUS));
            field_format_number(number_text, g_gosub_block_level, 1);
            gosub_append_encoded_string(name_cursor, number_text);
        }
        prim = field_draw_text(prim, ot, name_cursor, 4, 0x4C - x_off, 0xA - y_off, 0);
    }
    return prim;
}
