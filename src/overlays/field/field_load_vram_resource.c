#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

extern u8 *D_8010D038;
void cdrom_queue_read(s32 resource_index, void *dst_buffer);
void cdrom_wait_queue_empty(void);
s32 func_80086374(RECT *rect, u8 *data, s32 mode);

/**
 * @brief Loads a VRAM resource from disc and uploads it via func_80086374.
 *
 * Queues a CD read of resource @p id (masked to 16 bits) into the shared field
 * CD buffer @c D_8010D038, waits for the queue to drain, then hands the loaded
 * blob to func_80086374 to upload into VRAM using @p rect and @p arg2.
 *
 * @param id Resource index; masked to 16 bits for the CD queue.
 * @param rect Destination rectangle forwarded to func_80086374.
 * @param arg2 Upload mode forwarded to func_80086374.
 */
void field_load_vram_resource(s32 id, s16 *rect, s32 arg2)
{
    u8 *buf = D_8010D038;

    cdrom_queue_read(id & 0xFFFF, buf);
    cdrom_wait_queue_empty();
    func_80086374((RECT *)rect, buf, arg2);
}

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
