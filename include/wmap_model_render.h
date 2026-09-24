#ifndef WMAP_MODEL_RENDER_H
#define WMAP_MODEL_RENDER_H

#include "common.h"

#define WMAP_MODEL_DRAW_BACKFACES 0x1000
#define WMAP_MODEL_FORCE_OPAQUE 0x10000
#define WMAP_MODEL_COLOR_MASK 0xFFFF

/**
 * @brief Project a model's faces into the current world-map ordering table.
 * @param resource_table Packed resource header and relative model offsets.
 * @param resource_index Zero-based model index in the resource.
 * @param ot_index Front-face ordering-table bucket; backfaces use four buckets higher.
 * @param tpage Texture page for textured faces.
 * @param clut Palette for textured faces.
 * @param blend_mode Blend setting with WMAP_MODEL_DRAW_BACKFACES; low two bits select the GPU blend equation.
 * @param color_scale RGB multiplier in units of 1/128; textured faces use the low 16 bits and WMAP_MODEL_FORCE_OPAQUE.
 * @param x_offset Screen X displacement, ignored by flat untextured quads.
 * @param y_offset Screen Y displacement, ignored by flat untextured quads.
 * @param z_divisor Vertex Z divisor, or -1 to keep Z; zero acts as one. Untextured quads ignore it.
 * @note Uses the caller's GTE transform. Untextured colors use -1 to bypass scaling.
 * @note The backface flag is stripped before the signed blend-mode test, including for negative inputs.
 */
void wmap_draw_model(void* resource_table, s32 resource_index, s32 ot_index, s32 tpage, s32 clut, s32 blend_mode, s32 color_scale, s32 x_offset, s32 y_offset,
                     s32 z_divisor);

#endif
