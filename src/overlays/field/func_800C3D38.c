#include "common.h"

/** @brief Byte-aligned shape record with count and signed coordinate access fields. */
typedef struct
{
    u8 count;
    u8 pad[11];
    s8 x, y;
    u8 tail[74];
} Shape;
/** @brief Eleven shape records copied locally before updating layout cells. */
typedef struct
{
    Shape shapes[11];
} ShapeTable;
extern ShapeTable D_80051888;
extern u8 g_menuLayoutBuffer[];
/**
 * @brief Populate layout cells for the selected shape and rotation.
 * @param index Menu entry whose packed flags select the shape and cell metadata.
 * @param rotation Orientation selecting a twenty-byte coordinate block.
 * @param x Horizontal cell origin.
 * @param y Vertical cell origin in the six-column grid.
 */
void func_800C3D38(s32 index, s32 rotation, s32 x, s32 y)
{
    ShapeTable table;
    s32 offset, i, step;
    u8 *layout, *entry, *cell, *loop_layout;
    Shape *point;
    table = D_80051888;
    offset = index * 4;
    layout = g_menuLayoutBuffer;
    i = 0;
    if (table.shapes[(*(u32 *)(layout + offset + 0x29DC) >> 12) & 0xF].count != 0)
    {
        loop_layout = layout;
        step = rotation * 20;
        entry = loop_layout + 0x29DC + offset;
        do
        {
            point = (Shape *)((u8 *)&table + (step + ((*(u32 *)(entry) >> 12) & 0xF) * 88));
            cell = loop_layout + (x + point->x + (y + point->y) * 6) * 4;
            cell[0x2A7F] = index;
            cell[0x2A7C] = entry[0] >> 2;
            cell[0x2A7D] = (*(u32 *)(entry) >> 8) & 0xF;
            i++;
            step += 4;
        } while (i < table.shapes[(*(u32 *)(entry) >> 12) & 0xF].count);
    }
}
