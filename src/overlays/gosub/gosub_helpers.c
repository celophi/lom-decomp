#include "gosub_internal.h"

/**
 * @brief Upload the gosub interface image and CLUT to their fixed VRAM slots.
 */
void gosub_upload_ui_image(void)
{
    GosubImageVramLayout destinations;

    destinations.pixel_x = GOSUB_UI_IMAGE_X;
    destinations.pixel_y = 0;
    destinations.clut_x = 0;
    destinations.clut_y = GOSUB_GLYPH_CLUT_Y;
    gosub_upload_image_archive(&destinations, &g_gosub_image_archive);
}

/**
 * @brief Upload a TIM's optional CLUT and pixel data to selected VRAM positions.
 * @param destinations VRAM destinations for the pixel and CLUT blocks.
 * @param tim TIM resource to upload.
 * @note The pixel data is always taken from after a CLUT block, so a TIM
 *       without one would upload the wrong bytes; every GOSUB image has one.
 */
void gosub_upload_image_archive(GosubImageVramLayout* destinations, TimPrefix* tim)
{
    RECT upload_rect;
    s32 flags;
    s32 clut_block_size;
    TimDimensions* pixel_dimensions;

    flags = tim->flags;
    clut_block_size = tim->clut_block.bnum;

    if (flags & TIM_FLAG_HAS_CLUT)
    {
        setRECT(&upload_rect, destinations->clut_x, destinations->clut_y, CLUT_ENTRY_COUNT, 1);
        LoadImage(&upload_rect, (u_long*)tim->clut_data);
        pixel_dimensions = &TIM_PIXEL_BLOCK(tim, clut_block_size)->dimensions;
    }
    else
    {
        pixel_dimensions = &tim->clut_block.dimensions;
    }

    setRECT(&upload_rect, destinations->pixel_x, destinations->pixel_y, pixel_dimensions->width, pixel_dimensions->height);
    LoadImage(&upload_rect, (u_long*)(TIM_PIXEL_BLOCK(tim, clut_block_size) + 1));
}

/**
 * @brief Draw a logic block's icon: its base glyph and one glyph per cell of the shape.
 * @param packet Next free GPU packet.
 * @param ot Ordering table to receive the glyph packets.
 * @param x Icon left edge.
 * @param y Icon top edge.
 * @param block_id Logic-block id; selects the base glyph and the cell CLUT.
 * @param shape Index into g_golem_shape_table; rotation 0 is drawn.
 * @return Packet cursor after closing the glyph run.
 */
s32 gosub_draw_composite_icon(s32 packet, s32* ot, s32 x, s32 y, s32 block_id, s32 shape)
{
    s32 clut;
    GolemShape* shapes;
    GolemShape* layout;
    s32 icon_x;
    s32 icon_y;
    s32 i;

    clut = g_golem_logic_block_icons[block_id];
    shapes = g_golem_shape_table;
    layout = &shapes[shape];
    icon_x = x + layout->origin_x * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE;
    icon_y = y + layout->origin_y * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE;
    packet = gosub_emit_glyph(packet, ot, block_id + GOSUB_COMPOSITE_ICON_BASE_GLYPH_OFFSET,
                              layout->rotations[0].origin.x * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE + icon_x,
                              layout->rotations[0].origin.y * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE + icon_y, GOSUB_COMPOSITE_ICON_BASE_CLUT);
    for (i = 0; i < g_golem_shape_table[shape].count; i++)
    {
        packet = gosub_emit_glyph(packet, ot, g_golem_shape_table[shape].rotations[0].parts[i].glyph_id,
                                  g_golem_shape_table[shape].rotations[0].parts[i].x * GOSUB_COMPOSITE_ICON_PART_CELL_SIZE + icon_x,
                                  g_golem_shape_table[shape].rotations[0].parts[i].y * GOSUB_COMPOSITE_ICON_PART_CELL_SIZE + icon_y, clut);
    }
    return gosub_finish_glyph_run(packet, ot);
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
 * @brief Discard a logic block and close the gap in the block list.
 * @param record_index Index of the block to remove.
 */
void gosub_delete_logic_block(s32 record_index)
{
    s32 shift_index;

    for (shift_index = record_index; shift_index < g_saved_game_ctx->logic_block_count - 1; shift_index++)
    {
        gosub_copy_logic_block(&g_saved_game_ctx->logic_blocks[shift_index], &g_saved_game_ctx->logic_blocks[shift_index + 1]);
    }
    g_saved_game_ctx->logic_block_count--;
}

/**
 * @brief Delete one row from g_gosub_rows and renumber the rows above it.
 *
 * Rows above @p row are shifted down one slot and g_gosub_row_count is
 * decremented; every surviving row whose index still points past @p row has
 * that index pulled down by one so it keeps naming the same record.
 *
 * @param row Index of the row to remove.
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
 */
inline void gosub_copy_logic_block(void* dst, void* src)
{
    u8* dst_bytes;
    u8* src_bytes;
    u32 i;

    dst_bytes = (u8*)dst;
    src_bytes = (u8*)src;
    i = 0;
    do
    {
        i++;
        *dst_bytes++ = *src_bytes++;
    } while (i < sizeof(LogicBlock));
}

/**
 * @brief Copy one 0x20-byte GosubListRow.
 *
 * @param dst Destination row.
 * @param src Source row.
 */
inline void gosub_copy_list_row(void* dst, void* src)
{
    u8* dst_bytes;
    u8* src_bytes;
    u32 i;

    dst_bytes = (u8*)dst;
    src_bytes = (u8*)src;
    i = 0;
    do
    {
        i++;
        *dst_bytes++ = *src_bytes++;
    } while (i < sizeof(GosubListRow));
}

/**
 * @brief Sort the logic blocks, and their rows, by type, level or shape.
 *
 * An insertion sort builds the new order, then the blocks and rows are
 * copied back in that order and the list is rebuilt.
 *
 * @param sort_mode GosubSortKey in the low nibble; GOSUB_SORT_ASCENDING_MASK bits reverse the order.
 */
void gosub_sort_logic_blocks(s32 sort_mode)
{
    GosubSortWorkspace workspace;
    s32 row_index;
    s32 insertion_index;
    s32 shift_index;

    for (row_index = 0; row_index < g_gosub_row_count; row_index++)
    {
        for (insertion_index = 0; insertion_index < row_index; insertion_index++)
        {
            if (gosub_compare_logic_blocks(sort_mode, row_index, workspace.row_order[insertion_index]) == 0)
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

    bcopy((u8*)g_saved_game_ctx->logic_blocks, (u8*)workspace.blocks, sizeof(workspace.blocks));
    bcopy((u8*)g_gosub_rows, (u8*)workspace.rows, sizeof(workspace.rows));

    for (row_index = 0; row_index < g_gosub_row_count; row_index++)
    {
        gosub_copy_logic_block(&g_saved_game_ctx->logic_blocks[row_index], &workspace.blocks[workspace.row_order[row_index]]);
        gosub_copy_list_row(&g_gosub_rows[row_index], &workspace.rows[workspace.row_order[row_index]]);
    }

    gosub_build_logic_block_list();
}

/**
 * @brief Compare two logic-block rows by type, power, or shape.
 *
 * @param mode Low nibble selects the key; a nonzero high nibble swaps the operands.
 * @param left_row_index  First row index before the optional direction swap.
 * @param right_row_index Second row index before the optional direction swap.
 * @return 1 when the left operand sorts before the right, otherwise 0.
 */
s32 gosub_compare_logic_blocks(s32 mode, s32 left_row_index, s32 right_row_index)
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
 * @note The texture transfer reads past the font data into the start of
 *       g_gosub_item_colors, whose entries below GOSUB_COLOR_MATERIAL_FIRST are unused.
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
 */
GosubTilePacket* gosub_emit_panel_corners(SPRT* prim, s32* ot, s32 x, s32 y, s32 w, s32 h)
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
    return (GosubTilePacket*)(draw_tpage + 1);
}
