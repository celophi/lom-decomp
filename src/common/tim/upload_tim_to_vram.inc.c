/*
 * Shared TIM upload function; see include/common/tim.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/tim.h"

/**
 * @brief Upload a TIM's optional CLUT and its pixel data to the given VRAM positions.
 * @param destinations VRAM destinations for the pixel and CLUT blocks.
 * @param tim TIM resource to upload.
 * @note The pixel data is always taken from after a CLUT block, so a TIM
 *       without one would upload the wrong bytes.
 */
void upload_tim_to_vram(TimUploadDestination* destinations, TimPrefix* tim)
{
    RECT upload_rect;
    s32 flags;
    s32 clut_block_size;
    TimDimensions* pixel_dimensions;

    flags = tim->flags;
    clut_block_size = tim->clut_block.bnum;

    if (flags & TIM_FLAG_HAS_CLUT)
    {
        setRECT(&upload_rect, destinations->clut_x, destinations->clut_y, CLUT_ENTRY_COUNT, 1);
        LoadImage(&upload_rect, (u_long*)tim->clut_data);
        pixel_dimensions = &TIM_PIXEL_BLOCK(tim, clut_block_size)->dimensions;
    }
    else
    {
        pixel_dimensions = &tim->clut_block.dimensions;
    }

    setRECT(&upload_rect, destinations->pixel_x, destinations->pixel_y, pixel_dimensions->width, pixel_dimensions->height);
    LoadImage(&upload_rect, (u_long*)(TIM_PIXEL_BLOCK(tim, clut_block_size) + 1));
}
