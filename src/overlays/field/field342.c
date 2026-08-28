#include "common.h"
#include "sdk/libgpu.h"

extern u8 D_800EF1BC[];

/**
 * @brief Load two fixed sub-images into VRAM from @c D_800EF1BC.
 *
 * Transfers a 0x10x2 block to VRAM (0x100, 0x1F3) from the base image data and
 * a 0x10x0x20 block to (0x110, 0x1E0) from the data 0x40 bytes further in.
 *
 * @see decomp.me (100%) TODO
 */
void func_800ADE2C(void)
{
    RECT rect;

    rect.x = 0x100;
    rect.y = 0x1F3;
    rect.w = 0x10;
    rect.h = 2;
    LoadImage(&rect, (u_long *)D_800EF1BC);
    rect.x = 0x110;
    rect.y = 0x1E0;
    rect.w = 0x10;
    rect.h = 0x20;
    LoadImage(&rect, (u_long *)(D_800EF1BC + 0x40));
}
