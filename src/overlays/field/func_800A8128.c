typedef int s32;
typedef unsigned short u16;
s32 func_800A88A0(s32, s32, void *, s32, s32, s32, s32); /* extern */
/** Packed little-endian offset into the shared header text block. */
typedef struct PackedOffset
{
    unsigned char low;
    unsigned char high;
} PackedOffset;
extern unsigned char D_800EC3C4[];
extern PackedOffset D_800EC3CC;
extern PackedOffset D_800EC3CE;
/** Text resource header with offsets to both item-name tables. */
typedef struct TextResource
{
    s32 unused;
    s32 normal;
    s32 special;
} TextResource;
extern TextResource *D_8010D038;
extern u16 D_80122920;
extern u16 D_80122998;

/**
 * Draw the visible rows of the two-group item list, inserting each group header once.
 *
 * @param ordering_table Ordering-table address used by the text renderer.
 * @param cursor Current primitive buffer cursor.
 * @param scroll_x Horizontal scroll offset.
 * @param scroll_y Vertical scroll offset.
 * @param viewport_height Bottom clipping boundary.
 * @return Primitive buffer cursor after drawing the visible text.
 * @note Partial assembly match; probe evidence is retained in working/func_800A8128.
 */
s32 func_800A8128(s32 ordering_table, s32 cursor, s32 scroll_x, s32 scroll_y, s32 viewport_height)
{
    s32 header_x;
    s32 item_x;
    void *special_names;
    void *normal_names;
    s32 normal_header_y;
    s32 special_header_y;
    s32 draw_cursor;
    s32 next_cursor;
    s32 special_header_drawn;
    s32 row;
    s32 index;
    s32 normal_header_drawn;
    s32 special_row_y;
    s32 normal_row_y;
    s32 item_y;
    u16 *entry;
    u16 name_index;
    void *name_table;

    next_cursor = cursor;
    special_header_drawn = 0;
    row = 0;
    normal_header_drawn = 0;
    index = 0;
    normal_names = (unsigned char *)D_8010D038 + D_8010D038->normal;
    special_names = (unsigned char *)D_8010D038 + D_8010D038->special;
    if (D_80122998 != 0)
    {
        header_x = 0x20 - scroll_x;
        item_x = 0x80 - scroll_x;
        entry = &D_80122920;
        do
        {
            if (*entry & 0x8000)
            {
                special_row_y = row * 0x10;
                if (special_header_drawn == 0)
                {
                    special_header_y = special_row_y - scroll_y;
                    if ((special_header_y >= -0xF) && (special_header_y < viewport_height))
                    {
                        next_cursor =
                            func_800A88A0(next_cursor, ordering_table,
                                          D_800EC3CE.low + ((D_800EC3CE.high << 8) +
                                                            (unsigned char *)&D_800EC3CE - 10),
                                          4, header_x, special_header_y, 0);
                    }
                    special_header_drawn = 1;
                    row += 1;
                    special_row_y = row * 0x10;
                }
                item_y = special_row_y - scroll_y;
                if (item_y >= -0xF)
                {
                    draw_cursor = next_cursor;
                    if (item_y < viewport_height)
                    {
                        name_table = special_names;
                        name_index = *entry & 0x7FFF;
                        next_cursor = func_800A88A0(draw_cursor, ordering_table,
                                                    (unsigned char *)name_table +
                                                        ((u16 *)name_table)[name_index],
                                                    4, item_x, item_y, 2);
                    }
                }
            }
            else
            {
                normal_row_y = row * 0x10;
                if (normal_header_drawn == 0)
                {
                    normal_header_y = normal_row_y - scroll_y;
                    if ((normal_header_y >= -0xF) && (normal_header_y < viewport_height))
                    {
                        next_cursor =
                            func_800A88A0(next_cursor, ordering_table,
                                          D_800EC3CC.low + ((D_800EC3CC.high << 8) +
                                                            (unsigned char *)&D_800EC3CC - 8),
                                          4, header_x, normal_header_y, 0);
                    }
                    normal_header_drawn = 1;
                    row += 1;
                    normal_row_y = row * 0x10;
                }
                item_y = normal_row_y - scroll_y;
                if (item_y >= -0xF)
                {
                    draw_cursor = next_cursor;
                    if (item_y < viewport_height)
                    {
                        name_index = *entry;
                        name_table = normal_names;
                        next_cursor = func_800A88A0(draw_cursor, ordering_table,
                                                    (unsigned char *)name_table +
                                                        ((u16 *)name_table)[name_index],
                                                    4, item_x, item_y, 2);
                    }
                }
            }
            row += 1;
            entry += 1;
        } while (++index < (s32)D_80122998);
    }
    return next_cursor;
}
