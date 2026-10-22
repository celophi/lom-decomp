#ifndef FIELD_VRAM_H
#define FIELD_VRAM_H

/**
 * @file field_vram.h
 * @brief FIELD's UI texture page: one 4-bit page holding the HUD and icon image at its top,
 *        the dialogue text cache and the text window frame textures.
 */

/** @brief Top-left corner of the UI texture page. */
#define FIELD_UI_PAGE_X 960
#define FIELD_UI_PAGE_Y 256
/** @brief 4-bit texture page word of the UI texture page with blend mode @p blend (GPU_BLEND_*). */
#define FIELD_UI_TPAGE(blend) getTPage(GPU_TEXTURE_4BIT, (blend), FIELD_UI_PAGE_X, FIELD_UI_PAGE_Y)

/** @brief First of the two CLUT rows of the HUD and icon image. */
#define FIELD_UI_CLUT_X 256
#define FIELD_UI_CLUT_Y 480

#endif
