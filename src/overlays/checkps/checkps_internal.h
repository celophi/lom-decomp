#ifndef CHECKPS_INTERNAL_H
#define CHECKPS_INTERNAL_H

#include "checkps.h"

/* Glyph geometry shared by the display, cached-font, and Kanji renderers. */
#define CHECKPS_GLYPH_VRAM_X 960
#define CHECKPS_GLYPH_BITMAP_ROWS 15

#define CHECKPS_HARDWARE_WARNING_SIZE 60

/**
 * @brief Shift-JIS hardware-modification warning shown before termination.
 *
 * Reads "Forced termination. The console may have been modified." as two
 * newline-separated lines, NUL-padded to 60 bytes.
 */
extern const char g_hardware_modification_warning[CHECKPS_HARDWARE_WARNING_SIZE];

#endif
