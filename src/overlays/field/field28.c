/**
 * @file field28.c
 * @brief Field VRAM resource uploader, carved from the head of the unk2_h
 *        fragment (the single-function slot right after
 *        field_load_vram_resource and before func_80086494).
 */

#include "common.h"
#include "psyq_compat/libgte.h"
#include "psyq_compat/libgpu.h"

/**
 * @brief Upload a two-part image resource (palette/CLUT block followed by the
 *        pixel block) into VRAM and report the payload's trailing status word.
 * @param rect Destination framebuffer rectangle; supplies the upload x/y
 *             (from @c rect->x / @c rect->y) and the CLUT width/height (from
 *             @c rect->w / @c rect->h), and is updated on return to the pixel
 *             block's dimensions.
 * @param data Resource blob. Offset 0x8 holds the pixel-block byte offset,
 *             0x10 the CLUT dimensions, 0x14 the CLUT pixels, and 0x1F4 the
 *             status word returned to the caller.
 * @param mode When non-zero, upload the CLUT as a single 1-tall run of
 *             width*height entries; when zero, upload it with its natural
 *             width and height.
 * @return The status word stored at @c data+0x1F4.
 * @note The nested @c do{}while(0) wrappers reproduce the original codegen and
 *       are required to match; do not remove them.
 * @see decomp.me (100.00%)
 */
s32 func_80086374(RECT *rect, u8 *data, s32 mode)
{
    RECT load_rect;
    s32 offset;
    s32 ret;
    u8 *image;
    u8 *dims;
    u16 x;

    dims = data + 0x10;
    do { do { do { do { offset = *(s32 *)(data + 8); } while (0); } while (0); } while (0); } while (0);
    if (mode != 0)
    {
        load_rect.x = rect->w;
        load_rect.y = rect->h;
        load_rect.w = *(u16 *)dims * *(u16 *)(dims + 2);
        load_rect.h = 1;
    }
    else
    {
        load_rect.x = rect->w;
        load_rect.y = rect->h;
        load_rect.w = *(u16 *)dims;
        do { do { load_rect.h = *(u16 *)(dims + 2); } while (0); } while (0);
    }
    LoadImage(&load_rect, (u_long *)(data + 0x14));

    offset += 8;
    image = data + offset;
    x = rect->x;
    ret = *(s32 *)(data + 0x1F4);
    load_rect.x = x;
    load_rect.y = rect->y;
    dims = image + 8;
    load_rect.w = *(u16 *)dims;
    load_rect.h = *(u16 *)(dims + 2);
    LoadImage(&load_rect, (u_long *)(image + 0xC));
    rect->x = *(u16 *)dims;
    rect->y = *(u16 *)(dims + 2);
    return ret;
}
