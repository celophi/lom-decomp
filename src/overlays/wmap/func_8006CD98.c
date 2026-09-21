#include "common.h"

extern void func_800675F0(u8 *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

/**
 * @brief Draw a world-map resource without screen offsets or an extra Z divisor.
 * @param resource_table Resource table.
 * @param resource_index Resource to draw.
 * @param ot_index Ordering-table index.
 * @param tpage Texture page.
 * @param clut Color lookup table.
 * @param blend_mode Blending mode.
 * @param color_scale Color scale.
 */
void func_8006CD98(u8 *resource_table, s32 resource_index, s32 ot_index, s32 tpage,
                   s32 clut, s32 blend_mode, s32 color_scale)
{
    func_800675F0(resource_table, resource_index, ot_index, tpage,
                  clut, blend_mode, color_scale, 0, 0, -1);
}
