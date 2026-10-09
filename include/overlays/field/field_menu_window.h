#ifndef FIELD_MENU_WINDOW_H
#define FIELD_MENU_WINDOW_H

#include "common.h"

/**
 * @file field_menu_window.h
 * @brief FIELD's resident menu-window drawing, used by the menu overlays.
 */

/**
 * @brief g_field_menu_frame_style values: the u offset of a frame style's artwork
 *        in the menu texture page.
 */
#define FIELD_MENU_FRAME_STYLE_FIELD 0      /**< Field frame, with the first palette. */
#define FIELD_MENU_FRAME_STYLE_SUBSCREEN 32 /**< Frame of the sub-overlay screens (shops, save screens), with the second palette. */

/** @brief field_draw_menu_scroll_arrow direction: the down arrow. */
#define FIELD_MENU_ARROW_DOWN 0

/** @brief field_draw_menu_scroll_arrow direction: the up arrow. */
#define FIELD_MENU_ARROW_UP 1

/** @brief Frame style of every menu window: a FIELD_MENU_FRAME_STYLE_* value. */
extern s32 g_field_menu_frame_style;

/**
 * @brief Emit a bordered menu frame: its clip area, frame tiles and fill.
 * @param prim First free primitive-buffer byte.
 * @param ot Ordering-table entry receiving the primitives.
 * @param x Left edge of the window, in screen pixels.
 * @param y Top edge of the window, in screen pixels.
 * @param width Window width in pixels.
 * @param height Window height in pixels.
 * @param display_y Display y of the render half; nonzero draws into the area at SCREEN_HEIGHT.
 * @param bright Nonzero selects the brighter fill.
 * @return First free buffer byte after the emitted primitives.
 */
void* field_draw_menu_frame(void* prim, u_long* ot, s32 x, s32 y, s32 width, s32 height, s32 display_y, s32 bright);

/**
 * @brief Draw a menu scroll arrow centred on a point.
 * @param prim First free primitive-buffer byte.
 * @param ot Ordering-table entry receiving the primitives.
 * @param x Arrow centre x, in screen pixels.
 * @param y Arrow centre y, in screen pixels.
 * @param up FIELD_MENU_ARROW_UP or FIELD_MENU_ARROW_DOWN.
 * @return First free buffer byte after the emitted primitives.
 */
void* field_draw_menu_scroll_arrow(void* prim, u_long* ot, s32 x, s32 y, s32 up);

#endif
