#ifndef WMAP_MODEL_RENDER_H
#define WMAP_MODEL_RENDER_H

#include "common.h"

/**
 * @brief Render a primitive list from a world-map model resource.
 * @param resource_table Resource table containing model offsets.
 * @param resource_index Model entry index.
 * @param ot_index Base ordering-table index.
 * @param tpage Texture-page value written to textured primitives.
 * @param clut CLUT value written to textured primitives.
 * @param blend_mode Primitive blend mode and culling flags.
 * @param color_scale Color intensity scale and texture flags.
 * @param x_offset Screen-space X offset.
 * @param y_offset Screen-space Y offset.
 * @param z_divisor Optional Z divisor, or -1 to leave Z unchanged.
 */
void func_800675F0(u8* resource_table, s32 resource_index, s32 ot_index, s32 tpage, s32 clut, s32 blend_mode, s32 color_scale, s32 x_offset, s32 y_offset,
                   s32 z_divisor);

#endif
