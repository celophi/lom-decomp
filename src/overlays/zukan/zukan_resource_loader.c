#include "common.h"

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} ZukanRect;

typedef struct {
    s16 x;
    s16 y;
    s16 clut_x;
    s16 clut_y;
} ZukanImageVramLayout;

typedef struct {
    u8 pad[0xC];
    s32 size;
} ZukanResourceHeader;

extern u8 *D_80157520;
extern s32 D_80157D40;
extern s32 D_80157D5C;
extern u8 *D_80157D6C;
extern s32 D_80157D78;

void func_80142B3C(void)
{
    ZukanImageVramLayout destinations;
    ZukanRect upload_rect;
    ZukanImageVramLayout *layout;
    ZukanRect *rect;
    ZukanResourceHeader *header;
    u8 *src;
    u8 *dst;
    u8 *end;
    u8 *tim;
    u8 *tail_base;
    s32 flags;
    s32 clut_block_size;
    u16 *pixel_dimensions;
    s32 mode;
    s32 off0;
    s32 off1;

    func_80013F2C();

    header = (ZukanResourceHeader *)D_80157520;
    dst = D_80157D6C;
    end = (u8 *)header + header->size;
    src = (u8 *)header;
    if (src != end) {
        do {
            *dst++ = *src++;
        } while (src != end);
    }

    destinations.x = 0x380;
    destinations.y = 0x100;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1EE;

    rect = &upload_rect;
    layout = &destinations;
    tim = D_80157520 + *(s32 *)(D_80157520 + 0xC);
    flags = *(s32 *)(tim + 4);
    clut_block_size = *(s32 *)(tim + 8);
    mode = flags & 7;

    if (flags & 8) {
        upload_rect.x = layout->clut_x;
        upload_rect.y = layout->clut_y;
        rect->w = 0x100;
        rect->h = 1;
        func_80019A34(rect, tim + 0x14);
        do { pixel_dimensions = (u16 *)(clut_block_size - (-(s32)tim) + 0x10); } while (0);
    } else {
        pixel_dimensions = (u16 *)(tim + 0x10);
    }

    upload_rect.x = layout->x;
    upload_rect.y = layout->y;
    upload_rect.w = pixel_dimensions[0];
    upload_rect.h = pixel_dimensions[1];
    func_80019A34(&upload_rect, clut_block_size - (-(s32)tim) + 0x14);

    do { do { do { 
    tail_base = D_80157520;
    D_80157D5C = mode;
    off0 = *(volatile s32 *)(tail_base + 0x10);
    off1 = *(volatile s32 *)(tail_base + 0x10);
    D_80157D40 = *(u16 *)(tail_base + off0);
    D_80157D78 = *(u16 *)(tail_base + off1 + 2);
    } while (0); } while (0); } while (0); 
}
