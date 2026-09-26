#ifndef FIELD_MENU_ELEMENT_H
#define FIELD_MENU_ELEMENT_H

#include "common.h"
#include "field_runtime.h"

/**
 * @file
 * @brief The eight FIELD menu elements (windows) in g_field_menu_elements.
 *
 * field_claim_menu_element claims an idle element; field_draw_menu_elements
 * animates and draws every element that is not idle and calls its draw
 * callback for the contents.
 */

typedef struct FieldMenuElement FieldMenuElement;

/**
 * @brief Draw callback that emits one element's contents.
 * @note Some callbacks are defined with fewer parameters or other pointer
 *       types and are cast on assignment.
 */
typedef void *(*FieldMenuDrawFn)(u_long *ot, void *packet, s32 scroll_x, s32 scroll_y, s32 height,
                                 FieldMenuElement *element);

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

/** @brief Number of menu elements. */
#define FIELD_MENU_ELEMENT_COUNT 8
/** @brief FieldMenuAttr::word bits of the lifecycle state. */
#define FIELD_MENU_STATE_MASK 0x7
/** @brief Lifecycle states (FieldMenuAttr state). */
#define FIELD_MENU_STATE_IDLE 0
#define FIELD_MENU_STATE_OPENING 1
#define FIELD_MENU_STATE_OPEN 2
#define FIELD_MENU_STATE_CLOSING 3
/** @brief FieldMenuSize scroll_mode: not scrolled, scrolled with up/down, or by the owner. */
#define FIELD_MENU_SCROLL_NONE 0
#define FIELD_MENU_SCROLL_BY_PAD 1
#define FIELD_MENU_SCROLL_BY_OWNER 2
/** @brief FieldMenuAttr::word bits of width_low. */
#define FIELD_MENU_ATTR_WIDTH_LOW 0xFF000000
#define FIELD_MENU_ATTR_WIDTH_LOW_SHIFT 24
/** @brief FieldMenuSize::word bits of width_high, height and blink. */
#define FIELD_MENU_SIZE_WIDTH_HIGH 0x1
#define FIELD_MENU_SIZE_HEIGHT_MASK 0x1FE
#define FIELD_MENU_SIZE_HEIGHT_SHIFT 1
#define FIELD_MENU_SIZE_BLINK 0x200
/** @brief FieldMenuSize::word bits of scroll_mode. */
#define FIELD_MENU_SIZE_SCROLL_MODE 0xC00
/** @brief The height field of a FieldMenuSize word. */
#define FIELD_MENU_SIZE_HEIGHT(word) (((word) >> FIELD_MENU_SIZE_HEIGHT_SHIFT) & 0xFF)
/** @brief The x field of a FieldMenuAttr word. */
#define FIELD_MENU_ATTR_X(word) (((word) >> 7) & 0x1FF)

/** @brief Left edge of @p element. */
#define FIELD_MENU_X(element) ((element)->attr.bits.x)
/**
 * @brief Width low byte of @p element, read from the whole attribute word.
 * @note The width_low bitfield compiles to a byte load instead.
 */
#define FIELD_MENU_WIDTH_LOW(element) ((element)->attr.word >> FIELD_MENU_ATTR_WIDTH_LOW_SHIFT)
/** @brief Join the width high bit of @p element with an already-read low byte @p low. */
#define FIELD_MENU_JOIN_WIDTH(element, low) (((element)->size.bits.width_high << 8) | (low))
/** @brief Full 9-bit width of @p element. */
#define FIELD_MENU_WIDTH(element) FIELD_MENU_JOIN_WIDTH(element, FIELD_MENU_WIDTH_LOW(element))

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
extern FieldMenuElement g_field_menu_elements[FIELD_MENU_ELEMENT_COUNT];

/** @brief Frame style: 0 for the field frame, 0x20 for the sub-overlay frame. */
extern s32 g_menu_element_counter;

void *field_draw_menu_frame(void *packet, u_long *ot, s32 x, s32 y, s32 width, s32 height, s32 display_y, s32 bright);
void field_load_menu_frame_image(void);
void field_reset_menu_elements(void);
s32 field_menu_elements_animating(void);
void field_close_menu_elements(void);
FieldMenuElement *field_claim_menu_element(void);
void field_draw_menu_elements(FieldRenderHalf *render_half);
void *field_draw_menu_scroll_arrow(void *buffer, u_long *ot, s32 x, s32 y, s32 up);
s32 field_count_text_glyphs(u8 *text);

#endif
