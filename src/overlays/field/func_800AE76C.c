#include "common.h"
#include "sdk/libgpu.h"

extern u16 g_menu_element_counter;
u_long *func_800ADCD0(u_long *, u_long *, RECT *, RECT *);

/**
 * @brief Build a clipped menu primitive and append its draw-mode packet to the ordering table.
 * @param packet_buffer Current GPU packet cursor.
 * @param ordering_table Ordering-table entry to link the packet into.
 * @param x Source rectangle x coordinate before the four-pixel inset.
 * @param y Source rectangle y coordinate before the eight-pixel inset.
 * @param alternate Select the menu-counter offset used for the destination rectangle.
 * @return GPU packet cursor advanced past the emitted draw-mode packet.
 */
u_long *func_800AE76C(u_long *packet_buffer, u_long *ordering_table, s32 x, s32 y, s32 alternate)
{
    u_long *packet;
    RECT source;
    RECT destination;

    packet = packet_buffer;
    setRECT(&source, x - 4, y - 8, 8, 0x10);

    if (alternate != 0)
    {
        setRECT(&destination, g_menu_element_counter + 0x48, 0xE8, 8, 0x10);
    }
    else
    {
        setRECT(&destination, g_menu_element_counter + 0x50, 0xE8, 8, 0x10);
    }

    packet = func_800ADCD0(packet, ordering_table, &source, &destination);
    setDrawTPage((DR_TPAGE *)packet, 0, 0, 0x14);
    addPrim(ordering_table, packet);
    return packet + 2;
}
