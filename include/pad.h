#ifndef _PAD_H
#define _PAD_H

#include "common.h"

/**
 * Button bit masks for the game's internal controller state (g_lastInputState / g_debouncedInput).
 *
 * These differ from the raw PSX hardware bit positions. The SCD driver stores
 * the two controller bytes in reversed order, so UpdateControllerInput byte-swaps
 * held_buttons first. The face button bits (4-7) are then remapped so the hardware
 * order (Triangle, Circle, Cross, Square) becomes (Square, Cross, Circle, Triangle).
 *
 * Set bit = button pressed.
 */
typedef enum
{
    PAD_BTN_L2 = 0x0001,
    PAD_BTN_R2 = 0x0002,
    PAD_BTN_L1 = 0x0004,
    PAD_BTN_R1 = 0x0008,
    PAD_BTN_SQUARE = 0x0010,
    PAD_BTN_CROSS = 0x0020,
    PAD_BTN_CIRCLE = 0x0040,
    PAD_BTN_TRIANGLE = 0x0080,
    PAD_BTN_SELECT = 0x0100,
    PAD_BTN_L3 = 0x0200,
    PAD_BTN_R3 = 0x0400,
    PAD_BTN_START = 0x0800,
    PAD_BTN_UP = 0x1000,
    PAD_BTN_RIGHT = 0x2000,
    PAD_BTN_DOWN = 0x4000,
    PAD_BTN_LEFT = 0x8000
} PadButton;

typedef struct
{
    u8 device_type; // 0x00 - 0 digital, 1 analog joystick, 2 analog controller, 0xFE/0xFF unavailable
    u8 _pad1;
    u16 held_buttons; // 0x02 - currently held buttons in controller-protocol bit order
    u16 pressed_buttons; // 0x04 - newly pressed buttons, OR-merged across sampled VSyncs

    u8 _pad2[0x28 - 2]; // 0x06-0x2B - unused here

    s16 axis_x; // 0x2C - signed axis (negative/positive thresholded)
    s16 axis_y; // 0x2E - signed axis (negative/positive thresholded)
} SCDRegs;

/* Merged-controller register block lives at a fixed RAM address.
 * Token-equivalent to the cast so codegen is unchanged. */
#define SCD_REGS ((SCDRegs*)0x801ED600)

/**
 * Remaps the four face-button bits in a byte-swapped controller word from the
 * PSX hardware layout (Triangle/Circle/Cross/Square at bits 7/6/5/4) to the
 * game's logical layout (Square/Cross/Circle/Triangle at the same bits) by
 * swapping Triangle/Square and Circle/Cross. All other bits are preserved.
 *
 * Equivalent to writing the four-line nested-OR remap inline; expands to the
 * exact same expression tree so existing matched functions stay matched.
 */
#define PAD_REMAP_FACE_BITS(b)              \
    ((((b) & PAD_BTN_CIRCLE)   >> 1)   |    \
     (((b) & PAD_BTN_CROSS)    << 1)   |    \
     (((b) & PAD_BTN_TRIANGLE) >> 3)   |    \
     (((b) & PAD_BTN_SQUARE)   << 3)   |    \
     ((b) & ~0xF0))

#endif
