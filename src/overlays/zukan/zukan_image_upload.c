#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} ZukanRect;

typedef struct
{
    s16 x;
    s16 y;
    s16 clut_x;
    s16 clut_y;
} ZukanImageVramLayout;

extern u8 D_8014471C[];

s32 func_801411C4(ZukanImageVramLayout *destinations, u8 *tim);

/**
 * @brief Upload the two UI image blocks used by the encyclopedia screen.
 * @see decomp.me (100%)
 */
void func_80141144(void)
{
    ZukanImageVramLayout destinations;
    u8 *base = D_8014471C;

    destinations.x = 0x340;
    destinations.y = 0x100;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1F2;
    func_801411C4(&destinations, base + *(s32 *)(base + 8));

    destinations.x = 0x140;
    destinations.y = 0;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1F2;
    func_801411C4(&destinations, base + *(s32 *)(base + 4));
}

/**
 * @brief Upload a TIM image and its optional CLUT to VRAM.
 * @return The TIM pixel-mode bits from the flags word.
 * @see decomp.me (100%)
 */
s32 func_801411C4(ZukanImageVramLayout *destinations, u8 *tim)
{
    ZukanRect upload_rect;
    s32 flags;
    s32 clut_block_size;
    u16 *pixel_dimensions;
    s32 mode;

    flags = *(s32 *)(tim + 4);
    clut_block_size = *(s32 *)(tim + 8);
    mode = flags & 7;

    if (flags & 8)
    {
        upload_rect.x = destinations->clut_x;
        upload_rect.y = destinations->clut_y;
        upload_rect.w = 0x100;
        upload_rect.h = 1;
        func_80019A34(&upload_rect, tim + 0x14);
        pixel_dimensions = (u16 *)(clut_block_size - (-(s32)tim) + 0x10);
    }
    else
    {
        pixel_dimensions = (u16 *)(tim + 0x10);
    }

    upload_rect.x = destinations->x;
    upload_rect.y = destinations->y;
    upload_rect.w = pixel_dimensions[0];
    upload_rect.h = pixel_dimensions[1];
    func_80019A34(&upload_rect, clut_block_size - (-(s32)tim) + 0x14);
    return mode;
}
