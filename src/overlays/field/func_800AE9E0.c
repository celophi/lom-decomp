#include "common.h"
#include "main.h"
#include "sdk/libgpu.h"

#define PRIM_STRIP_VRAM_X 0x110
#define PRIM_STRIP_VRAM_Y0 0x1D8
#define PRIM_STRIP_W 0x10
#define PRIM_STRIP_H 1
#define PRIM_BLOCK_VRAM_X 0x3F4
#define PRIM_BLOCK_VRAM_X2 0x3E8
#define PRIM_BLOCK_VRAM_Y0 0x120
#define PRIM_BLOCK_VRAM_Y1 0x150
#define PRIM_BLOCK_W 0xC
#define PRIM_BLOCK_H 0x30
#define PRIM_SLOT_COUNT 3
#define PRIM_STRIP_BYTE_SIZE (PRIM_STRIP_W * PRIM_STRIP_H * sizeof(u16))
#define PRIM_BLOCK_BYTE_SIZE (PRIM_BLOCK_W * PRIM_BLOCK_H * sizeof(u16))
#define PRIM_BLOCK_BUF_OFFSET PRIM_STRIP_BYTE_SIZE
#define PRIM_SLOT_STRIDE (PRIM_STRIP_BYTE_SIZE + PRIM_BLOCK_BYTE_SIZE)
#define PRIM_ALIGN_UPLOAD_OFFSET(offset) (((offset) >> 2) << 2)
#define PRIM_UPLOAD_PTR(base, offset) ((u_long *)(PRIM_ALIGN_UPLOAD_OFFSET(offset) + (u32)(base)))

void func_800AE9E0(void)
{
    s32 slot = 0;
    u8 *scratch = g_prim_rect_buf;
    s32 block_byte_offset = PRIM_BLOCK_BUF_OFFSET;
    s32 strip_byte_offset = 0;
    RECT rect;
    u_long *upload_src;

    for (; slot < PRIM_SLOT_COUNT; slot++)
    {
        rect.x = PRIM_STRIP_VRAM_X;
        rect.y = slot + PRIM_STRIP_VRAM_Y0;
        rect.w = PRIM_STRIP_W;
        rect.h = PRIM_STRIP_H;
        upload_src = PRIM_UPLOAD_PTR(scratch, strip_byte_offset);
        LoadImage(&rect, upload_src);

        rect.x = (slot == PRIM_SLOT_COUNT - 1) ? PRIM_BLOCK_VRAM_X2 : PRIM_BLOCK_VRAM_X;
        rect.y = (slot == 0) ? PRIM_BLOCK_VRAM_Y0 : PRIM_BLOCK_VRAM_Y1;
        rect.w = PRIM_BLOCK_W;
        rect.h = PRIM_BLOCK_H;
        upload_src = PRIM_UPLOAD_PTR(scratch, block_byte_offset);
        LoadImage(&rect, upload_src);

        block_byte_offset += PRIM_SLOT_STRIDE;
        strip_byte_offset += PRIM_SLOT_STRIDE;
    }
}
