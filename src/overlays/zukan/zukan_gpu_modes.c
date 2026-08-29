#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

extern s32 D_80157D5C;

/** @brief Append a fixed draw-mode packet to the ordering table. */
s32 func_80141794(s32 packet_cursor, s32 ordering_table)
{
    DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;

    setDrawTPage(draw_mode, 0, 0, 5);
    addPrim(ordering_table, draw_mode);
    return packet_cursor + sizeof(DR_TPAGE);
}

/** @brief Append the alternate fixed draw-mode packet to the ordering table. */
s32 func_801417E8(s32 packet_cursor, s32 ordering_table)
{
    DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;

    setDrawTPage(draw_mode, 0, 0, 0x1D);
    addPrim(ordering_table, draw_mode);
    return packet_cursor + sizeof(DR_TPAGE);
}

/** @brief Append the draw-mode packet selected by the current page bits. */
s32 func_8014183C(s32 packet_cursor, s32 ordering_table)
{
    DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;

    setDrawTPage(draw_mode, 0, 0, 0x1E | ((D_80157D5C & 3) << 7));
    addPrim(ordering_table, draw_mode);
    return packet_cursor + sizeof(DR_TPAGE);
}
