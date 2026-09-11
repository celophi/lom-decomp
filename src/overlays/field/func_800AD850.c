#include "common.h"
#include "sdk/libgpu.h"

s32 *func_800ADCD0(s32 *, s32 *, RECT *, RECT *);
extern u16 g_menu_element_counter;

/**
 * @brief Emit a bordered menu rectangle, fill tile and draw-mode command.
 * @param buffer First free primitive-buffer address.
 * @param ordering Ordering-table entry receiving the new primitive chain.
 * @param x Rectangle origin in screen coordinates.
 * @param y Rectangle origin in screen coordinates.
 * @param width Rectangle width in pixels.
 * @param height Rectangle height in pixels.
 * @param bottom_buffer Nonzero selects the lower framebuffer clipping region.
 * @param bright Nonzero selects the brighter fill color.
 * @return First free buffer address after all emitted primitives.
 */
s32 *func_800AD850(s32 *buffer, s32 *ordering, s32 x, s32 y, s32 width, s32 height,
                   s32 bottom_buffer, s32 bright)
{
    s32 *ot = ordering;
    DR_ENV *draw_packet = (DR_ENV *)buffer;
    DRAWENV draw_env;
    RECT destination;
    RECT texture;
    s32 *cursor;
    u_long *packet;

    if (bottom_buffer != 0)
    {
        SetDefDrawEnv(&draw_env, x + 2, y + 0xF2, width - 4, height - 4);
    }
    else
    {
        SetDefDrawEnv(&draw_env, x + 2, y + 0xA, width - 4, height - 4);
    }
    SetDrawEnv(draw_packet, &draw_env);
    addPrim(ot, draw_packet);
    cursor = (s32 *)(draw_packet + 1);
    setRECT(&destination, x - 4, y - 4, 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x40, 0xE0, 8, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x + width - 4, y - 4, 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x58, 0xE0, 8, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x - 4, y + height - 4, 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x40, 0xF8, 8, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x + width - 4, y + height - 4, 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x58, 0xF8, 8, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x + 4, y - 4, width - 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x48, 0xE0, 16, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x + 4, y + height - 4, width - 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x48, 0xF8, 16, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x - 4, y + 4, 8, height - 8);
    setRECT(&texture, g_menu_element_counter + 0x40, 0xE8, 8, 16);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x + width - 4, y + 4, 8, height - 8);
    setRECT(&texture, g_menu_element_counter + 0x58, 0xE8, 8, 16);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    packet = (u_long *)cursor;
    if (bright != 0)
    {
        *(u32 *)&((TILE *)packet)->r0 = 0xA0A0A0;
    }
    else
    {
        *(u32 *)&((TILE *)packet)->r0 = 0x303030;
    }

    setTile((TILE *)packet);
    setSemiTrans((TILE *)packet, 1);
    setXY0((TILE *)packet, x, y);
    setWH((TILE *)packet, width, height);
    addPrim(ot, packet);
    packet += sizeof(TILE) / sizeof(*packet);
    setlen((DR_TPAGE *)packet, 1);
    ((DR_TPAGE *)packet)->code[0] = 0xE1000054;
    addPrim(ot, (DR_TPAGE *)packet);
    return (s32 *)((DR_TPAGE *)packet + 1);
}
