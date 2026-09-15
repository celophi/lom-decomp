/** @file field_coordinate_icon.c
 * @brief Draw coordinate-panel labels, upload their textures, and emit icon packets.
 */

/** @file Coordinate panel labels, texture upload, and icon packet construction. */
#include "common.h"
#include "sdk/libgpu.h"

s32 func_800AEAC0(POLY_FT4 *handle, u_long *ordering_table, s32 selector, s32 x, s32 y, s32 flip);
s32 func_800A88A0(void *arg0, void *arg1, void *arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);

extern s32 D_80122698;
extern u8 *D_801228F8[];

/**
 * @brief Draw the coordinate panel and its optional label rows.
 * @param arg0 Ordering-table context passed to each label draw.
 * @param arg1 Initial packet handle.
 * @param arg2 Horizontal panel offset.
 * @param arg3 Vertical panel offset.
 * @return The final packet handle after all enabled rows are emitted.
 */
s32 func_800AE8A8(void *arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 handle;
    s32 pad[2];

    handle = func_800AEAC0((POLY_FT4 *)arg1, arg0, D_80122698, 2 - arg2, -arg3, 1);
    if (D_801228F8[0] != 0)
    {
        handle = func_800A88A0((void *)handle, arg0, D_801228F8[0], 4, 0x38 - arg2, 1 - arg3, 0);
    }
    if (D_801228F8[1] != 0)
    {
        handle = func_800A88A0((void *)handle, arg0, D_801228F8[1], 4, 0x38 - arg2, 0x11 - arg3, 0);
    }
    if (D_801228F8[2] != 0)
    {
        handle = func_800A88A0((void *)handle, arg0, D_801228F8[2], 4, 0x38 - arg2, 0x21 - arg3, 0);
    }
    return handle;
}


/* func_800AE9E0 */
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


/* func_800AEAC0 */
#include "common.h"
#include "sdk/libgpu.h"


/**
 * @brief Build the textured coordinate icon primitives and append them to the ordering table.
 * @param handle Primitive buffer cursor.
 * @param ordering_table Ordering table entry that receives the generated primitives.
 * @param selector Selects the texture coordinates and CLUT.
 * @param x Horizontal screen coordinate.
 * @param y Vertical screen coordinate.
 * @param flip Nonzero to mirror the texture horizontally.
 * @return Primitive buffer cursor immediately after the generated draw-mode packet.
 */
s32 func_800AEAC0(POLY_FT4 *handle, u_long *ordering_table, s32 selector, s32 x, s32 y, s32 flip)
{
    POLY_FT4 *initial;
    POLY_FT4 *prim;
    POLY_FT4 *source;
    DR_TPAGE *draw_mode;
    u8 right_u;
    u8 u_value;

    initial = handle;
    prim = handle;
    prim++;
    prim--;
    *(u32 *)&initial->r0 = 0x808080;
    initial->x0 = x;
    setPolyFT4(initial);
    initial->x2 = initial->x0;
    initial->x3 = initial->x0 + 0x2F;
    initial->x1 = initial->x3;
    initial->y0 = y;
    initial->y1 = y;
    initial->y3 = initial->y0 + 0x2F;
    initial->y2 = initial->y3;

    if (selector == 2)
    {
        initial->u0 = 0xA0;
    }
    else
    {
        initial->u0 = 0xD0;
    }

    if (selector == 0)
    {
        prim->v0 = 0x20;
    }
    else
    {
        prim->v0 = 0x50;
    }

    prim->u2 = prim->u0;
    prim->u3 = prim->u0 + 0x2F;
    prim->u1 = prim->u3;
    prim->v1 = prim->v0;
    prim->v3 = prim->v0 + 0x2F;
    prim->v2 = prim->v3;
    if (flip != 0)
    {
        u_value = prim->u0;
        do
        {
            right_u = prim->u3;
        } while (0);
        prim->u1 = u_value;
        u_value = prim->u2;
        prim->u0 = right_u;
        prim->u2 = prim->u3;
        prim->u3 = u_value;
    }

    prim->clut = ((selector + 0x1D8) << 6) | 0x11;
    prim->tpage = 0x1F;
    addPrim(ordering_table, prim);

    source = prim;
    prim++;
    bcopy(source, prim, sizeof(POLY_FT4));

    setlen(prim, 9);
    *(u32 *)&prim->r0 = 0;
    setcode(prim, 0x2E);
    prim->x0 = (u16)(prim->x0 + 2);
    prim->x1 = (u16)(prim->x1 + 2);
    prim->x2 = (u16)(prim->x2 + 2);
    prim->x3 = (u16)(prim->x3 + 2);
    prim->y0 = (u16)(prim->y0 + 2);
    prim->y1 = (u16)(prim->y1 + 2);
    prim->y2 = (u16)(prim->y2 + 2);
    prim->y3 = (u16)(prim->y3 + 2);
    addPrim(ordering_table, prim);

    prim++;
    draw_mode = (DR_TPAGE *)prim;
    setDrawTPage(draw_mode, 0, 0, 0x1F);
    addPrim(ordering_table, draw_mode);
    return (s32)(draw_mode + 1);
}
