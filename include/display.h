#ifndef _DISPLAY_H
#define _DISPLAY_H

/**
 * @file display.h
 * @brief Shared screen and VRAM layout constants.
 *
 * @details The PSX video subsystem maps a single 1024x512 16bpp framebuffer for
 * both display output and texture/CLUT storage. Game code typically reserves
 * the upper-left region for two stacked screen-sized double buffers and uses
 * the rest for textures and palettes.
 *
 *   VRAM (1024 x 512)
 *   +---------------------------+--------------------+
 *   | Front buffer  (0, 0)      |                    |
 *   |   320 x 240               |   Texture / CLUT   |
 *   +---------------------------+    storage area    |
 *   | Back buffer   (0, 232)    |                    |
 *   |   320 x 240               |                    |
 *   +---------------------------+--------------------+
 *
 * Front buffer draws at Y = SCREEN_HEIGHT, back buffer draws at
 * Y = VRAM_BACK_DRAW_Y; both write VRAM_DRAW_HEIGHT rows so the unused
 * 8-pixel sliver can be overlaid by the back buffer's display region.
 */

/** Width of the visible screen area, excluding overscan. */
#define SCREEN_WIDTH    320

/** Height of the visible screen area, excluding overscan. */
#define SCREEN_HEIGHT   240

/** Total VRAM width in 16bpp texels. */
#define VRAM_WIDTH      1024

/** Total VRAM height in lines. */
#define VRAM_HEIGHT     512

/** Vertical start of the back buffer's draw region. */
#define VRAM_BACK_DRAW_Y    8

/** Vertical start of the back buffer's display region. */
#define VRAM_BACK_DISP_Y    232

/** Height of the draw region for each buffer. */
#define VRAM_DRAW_HEIGHT    224

/** Y coordinate of the CLUT storage row in VRAM.
 * Overlays store their 256-entry palettes in the lines starting here,
 * below both display buffers. */
#define VRAM_CLUT_Y         498

#endif
