#ifndef _PAD_H
#define _PAD_H

#include "common.h"

/**
 * Button bit masks for the game's internal controller state (g_lastInputState / g_debouncedInput).
 *
 * These differ from the raw PSX hardware bit positions. The SCD driver stores
 * the two controller bytes in reversed order, so UpdateControllerInput byte-swaps
 * buttonData first. The face button bits (4-7) are then remapped so the hardware
 * order (Triangle, Circle, Cross, Square) becomes (Square, Cross, Circle, Triangle).
 *
 * Set bit = button pressed.
 */
typedef enum
{
    PAD_BTN_SQUARE = 0x0010,
    PAD_BTN_CROSS = 0x0020,
    PAD_BTN_CIRCLE = 0x0040,
    PAD_BTN_TRIANGLE = 0x0080,
    PAD_BTN_UP = 0x1000,
    PAD_BTN_RIGHT = 0x2000,
    PAD_BTN_DOWN = 0x4000,
    PAD_BTN_LEFT = 0x8000
} PadButton;

typedef struct
{
    u8 deviceState; // 0x00 - status / mode flag
    u8 _pad1;
    u16 buttonData; // 0x02 - raw 16-bit input (pre-remap)

    u8 _pad2[0x28]; // 0x04–0x2B - unused here

    s16 axisX; // 0x2C - signed axis (negative/positive thresholded)
    s16 axisY; // 0x2E - signed axis (negative/positive thresholded)
} SCDRegs;

#endif