#include "gosub_internal.h"

/**
 * @brief Upload the gosub interface image and CLUT to their fixed VRAM slots.
 * @see decomp.me
 */
void gosub_upload_ui_image(void)
{
    GosubImageVramLayout destinations;

    destinations.pixel_x = 0x140;
    destinations.pixel_y = 0;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1F2;
    gosub_upload_image_archive(&destinations, &g_gosub_image_archive);
}

/**
 * @brief Upload a TIM's optional CLUT and pixel data to selected VRAM positions.
 * @param destinations VRAM destinations for the pixel and CLUT blocks.
 * @param tim TIM resource to upload.
 * @see decomp.me
 */
void gosub_upload_image_archive(GosubImageVramLayout* destinations, TimPrefix* tim)
{
    RECT upload_rect;
    s32 flags;
    s32 clut_block_size;
    TimDimensions* pixel_dimensions;

    flags = tim->flags;
    clut_block_size = tim->clut_block.bnum;

    if (flags & GOSUB_TIM_HAS_CLUT)
    {
        setRECT(&upload_rect, destinations->clut_x, destinations->clut_y, CLUT_ENTRY_COUNT, 1);
        LoadImage(&upload_rect, (u_long*)tim->clut_data);
        pixel_dimensions = &((TimBlock*)(clut_block_size + (s32)tim + TIM_HEADER_SIZE))->dimensions;
    }
    else
    {
        pixel_dimensions = &tim->clut_block.dimensions;
    }

    setRECT(&upload_rect, destinations->pixel_x, destinations->pixel_y, pixel_dimensions->width, pixel_dimensions->height);
    LoadImage(&upload_rect, (u_long*)(((TimBlock*)(clut_block_size + (s32)tim + TIM_HEADER_SIZE)) + 1));
}

/**
 * @brief Draw a composite icon from its base glyph and positioned parts.
 * @param initial_packet Next free GPU packet.
 * @param ordering_table Ordering table to receive the glyph packets.
 * @param x Base screen x coordinate.
 * @param y Base screen y coordinate.
 * @param icon_id Icon identifier used for the base glyph and part CLUT.
 * @param layout_index Composite layout index.
 * @return Packet cursor after closing the glyph run.
 * @see decomp.me
 */
s32 gosub_draw_composite_icon(s32 initial_packet, s32* ordering_table, s32 x, s32 y, s32 icon_id, s32 layout_index)
{
    GosubCompositeIconView layout_view;
    s32 clut;
    s16 layout_x;
    s16 layout_y;
    s8 base_glyph_x;
    s8 base_glyph_y;
    s32 icon_x;
    s32 icon_y;
    GosubCompositeIconView part_view;
    s32 part_index;
    s32 packet_cursor;
    u8* table_bytes;

    clut = D_800F2180[icon_id];
    table_bytes = D_800F1CD0;
    layout_view.bytes = (u8*)(layout_index * (s32)sizeof(GosubCompositeIconLayout) + (s32)table_bytes);
    layout_x = layout_view.layout->origin_x;
    layout_y = layout_view.layout->origin_y;
    base_glyph_x = layout_view.layout->base_x;
    base_glyph_y = layout_view.layout->base_y;
    icon_x = x + layout_x * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE;
    icon_y = y + layout_y * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE;
    packet_cursor = gosub_emit_glyph(initial_packet, ordering_table, icon_id + GOSUB_COMPOSITE_ICON_BASE_GLYPH_OFFSET,
                                     base_glyph_x * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE + icon_x, base_glyph_y * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE + icon_y,
                                     GOSUB_COMPOSITE_ICON_BASE_CLUT);

    /* The part count is byte zero of the packed layout. */
    layout_y = 0;
    for (part_index = layout_y; part_index < layout_view.bytes[layout_y]; part_index++)
    {
        u8* loop_base = &D_800F1CD0[layout_y];

        part_view.bytes = (u8*)(layout_index * (s32)sizeof(GosubCompositeIconLayout) + part_index * (s32)sizeof(GosubCompositeIconPart) + (s32)loop_base);
 * @see decomp.me
        /* The shifted layout view exposes the current tuple as parts[0]. */
        packet_cursor = gosub_emit_glyph(packet_cursor, ordering_table, part_view.layout->parts[0].glyph_id,
                                         part_view.layout->parts[0].x * GOSUB_COMPOSITE_ICON_PART_CELL_SIZE + icon_x,
                                         part_view.layout->parts[0].y * GOSUB_COMPOSITE_ICON_PART_CELL_SIZE + icon_y, clut);
    }
    return gosub_finish_glyph_run(packet_cursor, ordering_table);
}

/**
 * @brief Append the texture-page packet that closes a glyph run.
 *
 * @param packet_cursor Packet cursor.
 * @param ordering_table Ordering-table tag to link the packet into.
 * @return Packet cursor past the 8-byte draw-mode packet.
 */
s32 gosub_finish_glyph_run(s32 packet_cursor, s32* ordering_table)
{
    DR_TPAGE* draw_tpage;

    draw_tpage = (DR_TPAGE*)packet_cursor;
    setDrawTPage(draw_tpage, 0, 0, GOSUB_FONT_TPAGE);
    addPrim(ordering_table, draw_tpage);
    return packet_cursor + sizeof(DR_TPAGE);
}

/**
 * @brief Emit one glyph sprite described by the g_gosub_glyph_metrics cell table.
 *
 * @param packet_cursor Packet cursor.
 * @param ordering_table Ordering-table tag to link the sprite into.
 * @param glyph_id Index into g_gosub_glyph_metrics supplying the cell u/v and size.
 * @param x Sprite left edge.
 * @param y Sprite top edge.
 * @param clut_index CLUT slot on the glyph palette row.
 * @return Packet cursor past the 0x14-byte sprite.
 * @see decomp.me
 */
s32 gosub_emit_glyph(s32 packet_cursor, s32* ordering_table, s32 glyph_id, s32 x, s32 y, s32 clut_index)
{
    SPRT* sprite;

    sprite = (SPRT*)packet_cursor;
    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    setXY0(sprite, x, y);
    setWH(sprite, g_gosub_glyph_metrics[glyph_id].w, g_gosub_glyph_metrics[glyph_id].h);
    setUV0(sprite, g_gosub_glyph_metrics[glyph_id].u0, g_gosub_glyph_metrics[glyph_id].v0);
    setClut(sprite, clut_index << GOSUB_GLYPH_CLUT_X_SHIFT, GOSUB_GLYPH_CLUT_Y);
    addPrim(ordering_table, sprite);
    return packet_cursor + sizeof(SPRT);
}

/**
 * @brief Delete one packed logic-block record and close the gap.
 *
 * @param record_index Index of the record to remove.
 * @see decomp.me
 */
void gosub_delete_packed_record(s32 record_index)
{
    s32 shift_index;

    for (shift_index = record_index; shift_index < GOSUB_LOGIC_BLOCK_COUNT - 1; shift_index++)
    {
        gosub_copy_packed_record(&GOSUB_LOGIC_BLOCK_RECORDS[shift_index], &GOSUB_LOGIC_BLOCK_RECORDS[shift_index + 1]);
    }
    GOSUB_LOGIC_BLOCK_COUNT--;
}

/**
 * @brief Delete one row from g_gosub_rows and renumber the rows above it.
 *
 * Rows above @p row are shifted down one slot and g_gosub_row_count is
 * decremented; every surviving row whose index still points past @p row has
 * that index pulled down by one so it keeps naming the same record.
 *
 * @param row Index of the row to remove.
 * @see decomp.me
 */
void gosub_delete_list_row(s32 row)
{
    s32 i;

    for (i = row; i < g_gosub_row_count - 1; i++)
    {
        gosub_copy_list_row(&g_gosub_rows[i], &g_gosub_rows[i + 1]);
    }
    g_gosub_row_count--;
    for (i = 0; i < g_gosub_row_count; i++)
    {
        if (row < g_gosub_rows[i].index)
        {
            g_gosub_rows[i].index--;
        }
    }
}

/**
 * @brief Copy one 4-byte record.
 *
 * @param dst Destination record.
 * @param src Source record.
 * @see decomp.me
 */
inline void gosub_copy_packed_record(void* dst, void* src)
{
    u8* dst_bytes;
    u8* src_bytes;
    u32 byte_index;

    dst_bytes = (u8*)dst;
    src_bytes = (u8*)src;
    for (byte_index = 0; byte_index < sizeof(GosubPackedRecord);)
    {
        byte_index++;
        *dst_bytes = *src_bytes;
        src_bytes += 1;
        dst_bytes += 1;
    }
}

/**
 * @brief Copy one 0x20-byte GosubListRow.
 *
 * @param dst Destination row.
 * @param src Source row.
 * @see decomp.me
 */
inline void gosub_copy_list_row(void* dst, void* src)
{
    u8* dst_bytes;
    u8* src_bytes;
    u32 byte_index;

    dst_bytes = (u8*)dst;
    src_bytes = (u8*)src;
    for (byte_index = 0; byte_index < sizeof(GosubListRow);)
    {
        byte_index++;
        *dst_bytes = *src_bytes;
        src_bytes += 1;
        dst_bytes += 1;
    }
}

/**
 * @brief Sort the gosub row list, carrying each row's backing record with it.
 *
 * An insertion sort first builds a row permutation. The packed records and
 * display rows are then snapshotted and rewritten through that permutation.
 * Rebuilding the list last refreshes its derived names and fields.
 *
 * @param sort_mode Encoded type, power, or shape key and sort direction.
 *
 * @see decomp.me
 */
void gosub_sort_rows(s32 sort_mode)
{
    GosubSortWorkspace workspace;
    s32 row_index;
    s32 insertion_index;
    s32 shift_index;

    for (row_index = 0; row_index < g_gosub_row_count; row_index++)
    {
        for (insertion_index = 0; insertion_index < row_index; insertion_index++)
        {
            if (gosub_compare_rows(sort_mode, row_index, workspace.row_order[insertion_index]) == 0)
            {
                break;
            }
        }
        if (insertion_index != row_index)
        {
            for (shift_index = row_index; shift_index > insertion_index; shift_index--)
            {
                workspace.row_order[shift_index] = workspace.row_order[shift_index - 1];
            }
        }
        workspace.row_order[insertion_index] = row_index;
    }

    bcopy(GOSUB_LOGIC_BLOCK_RECORDS, workspace.packed_records, sizeof(workspace.packed_records));
    bcopy(g_gosub_rows, workspace.rows, sizeof(workspace.rows));

    for (row_index = 0; row_index < g_gosub_row_count; row_index++)
    {
        gosub_copy_packed_record(&GOSUB_LOGIC_BLOCK_RECORDS[row_index], &workspace.packed_records[workspace.row_order[row_index]]);
        gosub_copy_list_row(&g_gosub_rows[row_index], &workspace.rows[workspace.row_order[row_index]]);
    }

    gosub_build_packed_record_list();
}

/**
 * @brief Compare two logic-block rows by type, power, or shape.
 *
 * @param mode Low nibble selects the key; a nonzero high nibble swaps the operands.
 * @param left_row_index  First row index before the optional direction swap.
 * @param right_row_index Second row index before the optional direction swap.
 * @return 1 when the left operand sorts before the right, otherwise 0.
 * @see decomp.me
 */
s32 gosub_compare_rows(s32 mode, s32 left_row_index, s32 right_row_index)
{
    s32 swapped_row_index;

    if (mode & GOSUB_SORT_ASCENDING_MASK)
    {
        swapped_row_index = left_row_index;
        left_row_index = right_row_index;
        right_row_index = swapped_row_index;
    }
    switch (mode & GOSUB_SORT_KEY_MASK)
    {
    case GOSUB_SORT_BY_TYPE:
        if (g_gosub_rows[left_row_index].detail_group < g_gosub_rows[right_row_index].detail_group)
        {
            return 1;
        }
        break;
    case GOSUB_SORT_BY_POWER:
        if (g_gosub_rows[left_row_index].detail_id < g_gosub_rows[right_row_index].detail_id)
        {
            return 1;
        }
        break;
    case GOSUB_SORT_BY_SHAPE:
        if (g_gosub_rows[left_row_index].detail_variant < g_gosub_rows[right_row_index].detail_variant)
        {
            return 1;
        }
        break;
    }
    return 0;
}

/**
 * @brief Upload the gosub font CLUT and texture strip to their fixed VRAM slots.
 *
 * @note The 0x200-byte texture transfer continues through the first 0x5C bytes
 *       of g_gosub_item_metadata; its live metadata begins at index 0x60.
 * @see decomp.me
 */
void gosub_upload_font_texture(void)
{
    RECT upload_rect;

    setRECT(&upload_rect, GOSUB_FONT_CLUT_X, GOSUB_FONT_CLUT_Y, GOSUB_FONT_CLUT_WIDTH, GOSUB_FONT_CLUT_HEIGHT);
    LoadImage(&upload_rect, (u_long*)g_gosub_font_texture);

    setRECT(&upload_rect, GOSUB_FONT_TEXTURE_X, GOSUB_FONT_TEXTURE_Y, GOSUB_FONT_TEXTURE_WIDTH, GOSUB_FONT_TEXTURE_HEIGHT);
    LoadImage(&upload_rect, (u_long*)(g_gosub_font_texture + GOSUB_FONT_TEXTURE_DATA_OFFSET));
    DrawSync(0);
}

/**
 * @brief Emit the four corner sprites of a gosub panel frame and close the run
 *        with a texture-page packet.
 *
 * The corners sit two pixels outside (@p x, @p y) and five pixels inside the
 * far edge; the top pair uses texture row 0xF0 and the bottom pair 0xF8.
 *
 * @param prim Packet cursor; four sprites and one draw-mode packet are written.
 * @param ot   Ordering-table tag every packet is linked into.
 * @param x    Panel left edge.
 * @param y    Panel top edge.
 * @param w    Panel width.
 * @param h    Panel height.
 * @return Packet cursor past the draw-mode packet.
 * @see decomp.me
 */
GosubGpuPacket* gosub_emit_panel_corners(SPRT* prim, s32* ot, s32 x, s32 y, s32 w, s32 h)
{
    DR_TPAGE* draw_tpage;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;

    x0 = x - GOSUB_PANEL_CORNER_OUTSET;
    y0 = y - GOSUB_PANEL_CORNER_OUTSET;

    SET_BGR0_PACKED(prim, GPU_TINT_NEUTRAL);
    setSprt(prim);
    setXY0(prim, x0, y0);
    setUV0(prim, 0, GOSUB_PANEL_CORNER_TEXTURE_V);
    setWH(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_SIZE);
    setClut(prim, GOSUB_FONT_CLUT_X, GOSUB_FONT_CLUT_Y);
    addPrim(ot, prim);
    prim += 1;

    x1 = x + w - GOSUB_PANEL_CORNER_FAR_INSET;
    y1 = y + h - GOSUB_PANEL_CORNER_FAR_INSET;

    SET_BGR0_PACKED(prim, GPU_TINT_NEUTRAL);
    setSprt(prim);
    setXY0(prim, x1, y0);
    setUV0(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_TEXTURE_V);
    setWH(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_SIZE);
    setClut(prim, GOSUB_FONT_CLUT_X, GOSUB_FONT_CLUT_Y);
    addPrim(ot, prim);
    prim += 1;

    SET_BGR0_PACKED(prim, GPU_TINT_NEUTRAL);
    setSprt(prim);
    setXY0(prim, x0, y1);
    setUV0(prim, 0, GOSUB_PANEL_CORNER_TEXTURE_V + GOSUB_PANEL_CORNER_SIZE);
    setWH(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_SIZE);
    setClut(prim, GOSUB_FONT_CLUT_X, GOSUB_FONT_CLUT_Y);
    addPrim(ot, prim);
    prim += 1;

    SET_BGR0_PACKED(prim, GPU_TINT_NEUTRAL);
    setSprt(prim);
    setXY0(prim, x1, y1);
    setUV0(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_TEXTURE_V + GOSUB_PANEL_CORNER_SIZE);
    setWH(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_SIZE);
    setClut(prim, GOSUB_FONT_CLUT_X, GOSUB_FONT_CLUT_Y);
    addPrim(ot, prim);
    prim += 1;

    draw_tpage = (DR_TPAGE*)prim;
    setDrawTPage(draw_tpage, 0, 0, GOSUB_FONT_TPAGE);
    addPrim(ot, draw_tpage);
    return (GosubGpuPacket*)(draw_tpage + 1);
}
