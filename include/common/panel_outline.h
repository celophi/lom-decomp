#ifndef PANEL_OUTLINE_H
#define PANEL_OUTLINE_H

#include "common.h"
#include <libgpu.h>

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
 * @note Compiled into GOLEM and ZUKAN from src/common/panel_outline/emit_panel_outline.inc.c; neither calls it.
 */
LINE_F2* emit_panel_outline(LINE_F2* line, u_long* ordering_table, s32 x, s32 y, s32 width, s32 height, s32 color);

#endif
