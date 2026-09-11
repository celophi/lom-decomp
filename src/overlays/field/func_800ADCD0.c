#include "common.h"
#include "sdk/libgpu.h"

extern s32 g_menu_element_counter;

/**
 * @brief Tile a rectangle with sprite primitives and link them into an ordering table.
 * @param packet_cursor Next free primitive-buffer address.
 * @param ordering_table Ordering-table entry receiving the primitive chain.
 * @param destination Destination rectangle position and dimensions.
 * @param texture Texture origin and maximum tile dimensions.
 * @return The first free buffer address after the emitted primitives.
 */
void *func_800ADCD0(void *packet_cursor, u32 *ordering_table, RECT *destination, RECT *texture)
{
    s32 remaining_height;
    s32 remaining_width;
    s32 y_offset;
    s32 x_offset;
    s32 tile_height;
    s32 tile_width;
    u32 primitive_addr;
    s32 tag_length;
    s32 command;
    s16 clut;
    s32 color_word;
    s32 tag_length_mask;

    if (destination->w > 0)
    {
        if (destination->h > 0)
        {
            y_offset = 0;
            remaining_height = destination->h;
            color_word = 0x808080;
            tag_length = 4;
            command = 0x64;
            do
            {
                x_offset = 0;
                tile_height = remaining_height;
                if (texture->h < remaining_height)
                {
                    tile_height = texture->h;
                }
                remaining_width = destination->w;
                tag_length_mask = 0xFF000000;
                do
                {
                    tile_width = remaining_width;
                    if (texture->w < remaining_width)
                    {
                        tile_width = texture->w;
                    }

                    *(u32 *)&((SPRT *)packet_cursor)->r0 = color_word;
                    setlen((SPRT *)packet_cursor, tag_length);
                    setcode((SPRT *)packet_cursor, command);
                    ((SPRT *)packet_cursor)->x0 = (s16)((u16)destination->x + x_offset);
                    ((SPRT *)packet_cursor)->y0 = (s16)((u16)destination->y + y_offset);
                    ((SPRT *)packet_cursor)->u0 = (u8)texture->x;
                    ((SPRT *)packet_cursor)->v0 = (u8)texture->y;
                    ((SPRT *)packet_cursor)->w = tile_width;
                    ((SPRT *)packet_cursor)->h = tile_height;
                    clut = 0x7CD0;
                    if (g_menu_element_counter != 0)
                    {
                        clut = 0x7D10;
                    }
                    ((SPRT *)packet_cursor)->clut = clut;

                    ((SPRT *)packet_cursor)->tag = (((SPRT *)packet_cursor)->tag & tag_length_mask) | (*ordering_table & 0xFFFFFF);
                    primitive_addr = (u32)packet_cursor & 0xFFFFFF;
                    *ordering_table = (*ordering_table & tag_length_mask) | (primitive_addr & 0xFFFFFF);
                    packet_cursor = (u8 *)packet_cursor + sizeof(SPRT);

                    x_offset += tile_width;
                    remaining_width -= tile_width;
                } while (remaining_width != 0);

                remaining_height -= tile_height;
                y_offset += tile_height;
            } while (remaining_height != 0);
        }
        return packet_cursor;
    }
    return packet_cursor;
}
