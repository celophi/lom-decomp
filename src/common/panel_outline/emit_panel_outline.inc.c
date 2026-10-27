/*
 * Shared panel-outline function; see include/common/panel_outline.h. Included by
 * each overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/gpu_packet.h"
#include "common/panel_outline.h"

/**
 * @brief Emit a rectangular outline as four flat lines.
 * @param line Next line packet in the primitive buffer.
 * @param ordering_table Ordering-table entry receiving the lines.
 * @param x Left edge.
 * @param y Top edge.
 * @param width Panel width.
 * @param height Panel height.
 * @param color Packed line colour.
 * @return First packet after the four lines.
 */
LINE_F2* emit_panel_outline(LINE_F2* line, u_long* ordering_table, s32 x, s32 y, s32 width, s32 height, s32 color)
{
    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY2(line, x, y, x + width, y);
    addPrim(ordering_table, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY2(line, x + width, y, x + width, y + height);
    addPrim(ordering_table, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY2(line, x + width, y + height, x, y + height);
    addPrim(ordering_table, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY2(line, x, y, x, y + height);
    addPrim(ordering_table, line);
    return line + 1;
}
