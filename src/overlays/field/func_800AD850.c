#include "common.h"
#include "sdk/libgpu.h"

/** @brief Position or texture origin followed by signed tile dimensions. */
typedef struct
{
    u16 x, y;
    s16 w, h;
} Rect;
/** @brief Fill tile packet, also providing the tag and command of a draw-mode packet. */
typedef struct
{
    u32 tag, color;
    u16 x, y, w, h;
} Packet;
s32 *func_800ADCD0(s32 *, s32 *, Rect *, Rect *);
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
    s32 *draw_packet = buffer;
    DRAWENV draw_env;
    Rect destination;
    Rect texture;
    s32 *cursor;
    u32 packet_link;
    Packet *packet;

    if (bottom_buffer != 0)
    {
        SetDefDrawEnv(&draw_env, x + 2, y + 0xF2, width - 4, height - 4);
    }
    else
    {
        SetDefDrawEnv(&draw_env, x + 2, y + 0xA, width - 4, height - 4);
    }
    SetDrawEnv((DR_ENV *)draw_packet, &draw_env);
    *draw_packet = (*draw_packet & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)draw_packet & 0xFFFFFF);
    cursor = (s32 *)((u8 *)draw_packet + 0x40);
    destination.x = x - 4;
    destination.y = y - 4;
    destination.w = 8;
    destination.h = 8;
    texture.x = g_menu_element_counter + 0x40;
    texture.y = 0xE0;
    texture.w = 8;
    texture.h = 8;
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    destination.x = x + width - 4;
    destination.y = y - 4;
    destination.w = 8;
    destination.h = 8;
    texture.x = g_menu_element_counter + 0x58;
    texture.y = 0xE0;
    texture.w = 8;
    texture.h = 8;
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    destination.x = x - 4;
    destination.y = y + height - 4;
    destination.w = 8;
    destination.h = 8;
    texture.x = g_menu_element_counter + 0x40;
    texture.y = 0xF8;
    texture.w = 8;
    texture.h = 8;
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    destination.x = x + width - 4;
    destination.y = y + height - 4;
    destination.w = 8;
    destination.h = 8;
    texture.x = g_menu_element_counter + 0x58;
    texture.y = 0xF8;
    texture.w = 8;
    texture.h = 8;
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    destination.x = x + 4;
    destination.y = y - 4;
    destination.w = width - 8;
    destination.h = 8;
    texture.x = g_menu_element_counter + 0x48;
    texture.y = 0xE0;
    texture.w = 16;
    texture.h = 8;
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    destination.x = x + 4;
    destination.y = y + height - 4;
    destination.w = width - 8;
    destination.h = 8;
    texture.x = g_menu_element_counter + 0x48;
    texture.y = 0xF8;
    texture.w = 16;
    texture.h = 8;
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    destination.x = x - 4;
    destination.y = y + 4;
    destination.w = 8;
    destination.h = height - 8;
    texture.x = g_menu_element_counter + 0x40;
    texture.y = 0xE8;
    texture.w = 8;
    texture.h = 16;
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    destination.x = x + width - 4;
    destination.y = y + 4;
    destination.w = 8;
    destination.h = height - 8;
    texture.x = g_menu_element_counter + 0x58;
    texture.y = 0xE8;
    texture.w = 8;
    texture.h = 16;
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    packet = (Packet *)cursor;
    if (bright != 0)
    {
        packet->color = 0xA0A0A0;
    }
    else
    {
        packet->color = 0x303030;
    }

    ((u8 *)packet)[3] = 3;
    ((u8 *)packet)[7] = 0x62;
    packet->x = (u16)x;
    packet->y = (u16)y;
    packet->w = (u16)width;
    packet->h = (u16)height;
    packet->tag = (packet->tag & 0xFF000000) | (*ot & 0xFFFFFF);
    packet_link = (s32)packet & 0xFFFFFF;
    packet = (Packet *)((u8 *)packet + 0x10);
    *ot = (*ot & 0xFF000000) | packet_link;
    ((u8 *)packet)[3] = 1;
    packet->color = 0xE1000054;
    packet->tag = (s32)((packet->tag & 0xFF000000) | (*ot & 0xFFFFFF));
    *ot = (*ot & 0xFF000000) | ((s32)packet & 0xFFFFFF);
    return (s32 *)((u8 *)packet + 8);
}
