typedef int s32;
typedef unsigned int u32;
typedef short s16;
typedef unsigned short u16;
typedef unsigned char u8;
/** Window header containing the packed clipping height at offset four. */
typedef struct Window
{
    u32 unused;
    u32 size;
} Window;
/** Sixteen-byte GPU tile packet, including its ordering-table tag. */
typedef struct Tile
{
    u32 tag;
    u32 color;
    s16 x, y, w, h;
} Tile;
extern u8 D_800EE72C[];
extern s32 D_80122714, D_80122734, D_80122A00;
extern u8 D_80122738[];
extern u8 *func_800A88A0(u8 *, u32 *, u8 *, s32, s32, s32, s32);
extern void func_800A8B90(u8 *, s32, s32);
extern void func_800AF350(Window *);
/**
 * Format and draw one numeric value using a temporary text buffer.
 * @param ot Ordering table.
 * @param cursor Primitive buffer cursor.
 * @param number Value to format.
 * @param color Text color selector.
 * @param position Signed coordinates stored as halfwords.
 * @param flags Text rendering flags.
 * @return Cursor after the generated text primitives.
 */
static __inline__ u8 *draw_number(u32 *ot, u8 *cursor, s32 number, s32 color, u16 *position,
                                  s32 flags)
{
    u8 text[64];
    func_800A8B90(text, number, 0);
    return func_800A88A0(cursor, ot, text, color, (s16)position[0], (s16)position[1], flags);
}
/**
 * Draw the visible name/value rows and append the highlighted-row tile.
 * @param ot Ordering table.
 * @param cursor Primitive buffer cursor.
 * @param scroll_x Horizontal scroll offset.
 * @param scroll_y Vertical scroll offset.
 * @param unused Unused callback argument retained for the six-argument ABI.
 * @param window Window used to prepare the list and determine its clipping height.
 * @return Cursor after the list, highlight tile, and draw-mode packet.
 * @note Partial match; detailed probe results are retained in working/func_800AF0E8.
 */
u8 *func_800AF0E8(u32 *ot, u8 *cursor, s32 scroll_x, s32 scroll_y, s32 unused, Window *window)
{
    u16 point[4];
    s32 number_x;
    u8 *entry;
    u8 *names;
    u16 *position;
    s32 y;
    s32 index;
    u8 *mode;
    Tile *tile;
    func_800AF350(window);
    if (D_80122714 == 0)
    {
        return cursor;
    }
    index = 0;
    if (D_80122734 > 0)
    {
        names = D_800EE72C;
        number_x = 0xCA - scroll_x;
        position = point;
        entry = D_80122738;
        do
        {
            y = index * 16 - scroll_y;
            if (y >= -15 && y < (s32)((window->size >> 1) & 255))
            {
                cursor =
                    func_800A88A0(cursor, ot, names + ((u16 *)names)[entry[0]], 4, -scroll_x, y, 0);
                point[1] = y;
                point[0] = number_x;
                cursor = draw_number(ot, cursor, entry[1], 4, position, 1);
            }
            entry += 2;
        } while (++index < D_80122734);
    }
    tile = (Tile *)cursor;
    tile->color = 0xF080F0;
    cursor[3] = 3;
    cursor[7] = 0x62;
    tile->w = 0xE8;
    mode = cursor + 16;
    tile->x = 0;
    tile->h = 14;
    tile->y = D_80122A00 * 16 - scroll_y;
    tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((u32)cursor & 0xFFFFFF);
    mode[3] = 1;
    *(u32 *)(mode + 4) = 0xE1000005;
    *(u32 *)(cursor + 16) = (*(u32 *)(cursor + 16) & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((u32)mode & 0xFFFFFF);
    return cursor + 24;
}
