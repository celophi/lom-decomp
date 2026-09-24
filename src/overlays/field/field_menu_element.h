#ifndef FIELD_MENU_ELEMENT_H
#define FIELD_MENU_ELEMENT_H

#include "common.h"

/**
 * @file
 * @brief The eight FIELD menu elements (windows) at D_80122828.
 *
 * func_800ADF84 claims an idle element; func_800AE008 animates and draws every
 * element that is not idle and calls its draw callback for the contents.
 */

typedef struct FieldMenuRenderContext FieldMenuRenderContext;
typedef struct FieldMenuElement FieldMenuElement;

/**
 * @brief Draw callback that emits one element's contents.
 * @note Some callbacks are defined with other pointer types (u32 * ordering
 *       table, u8 * or void * cursor, or no arguments) and are cast on assignment.
 */
typedef s32 *(*FieldMenuDrawFn)(FieldMenuRenderContext *context, s32 *cursor, s32 scroll_x,
                                s32 scroll_y, s32 height, FieldMenuElement *element);

/** @brief Element word 0: lifecycle state, open/close step, position and width low byte. */
typedef union FieldMenuAttr
{
    u32 word;
    struct
    {
        u32 state : 3;     /**< 0 idle, 1 opening, 2 open, 3 closing. */
        u32 step : 4;      /**< Opening/closing animation step, 0 to 8. */
        u32 x : 9;         /**< Left edge in screen pixels. */
        u32 y : 8;         /**< Top edge in screen pixels. */
        u32 width_low : 8; /**< Width bits 0-7 (bit 8 is FieldMenuSize width_high). */
    } bits;
    struct
    {
        u16 low;           /**< state, step and x. */
        u8 y;              /**< Top edge, read and stored as a byte. */
        u8 width_low;      /**< Width bits 0-7. */
    } bytes;
} FieldMenuAttr;

/** @brief Element word 1: width high bit, visible height, blink, scroll mode, content height. */
typedef union FieldMenuSize
{
    u32 word;
    struct
    {
        u32 width_high : 1;  /**< Width bit 8. */
        u32 height : 8;      /**< Visible height in pixels. */
        u32 blink : 1;       /**< Blink the border while open. */
        u32 scroll_mode : 2; /**< 0 none, 1 pad-scrolled, 2 scrolled by the owner. */
        u32 upper : 20;      /**< Bits 12-31; content_height is bits 16-31. */
    } bits;
    struct
    {
        u16 low;             /**< width_high, height, blink and scroll_mode. */
        s16 content_height;  /**< Total content height in pixels. */
    } fields;
} FieldMenuSize;

/** @brief Twenty-byte menu element with packed geometry, scrolling, and draw callback. */
struct FieldMenuElement
{
    FieldMenuAttr attr;   /**< 0x00: state, step, x, y, width low byte. */
    FieldMenuSize size;   /**< 0x04: width high bit, height, flags, content height. */
    s16 scroll;           /**< 0x08: current vertical content scroll. */
    s16 scroll_target;    /**< 0x0A: scroll the element eases towards. */
    s16 scroll_ticks;     /**< 0x0C: frames left in the scroll ease. */
    u16 padE;             /**< 0x0E: unused. */
    FieldMenuDrawFn draw; /**< 0x10: content draw callback. */
};

/** @brief The eight menu elements. */
extern FieldMenuElement D_80122828[];

/**
 * @brief Allocate the first idle menu element and initialize it to the opening state.
 * @return The claimed element, or the first element when none are free.
 */
FieldMenuElement *func_800ADF84(void);

#endif
