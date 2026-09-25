/** @file field_coordinate_icon.c
 * @brief Draw coordinate-panel labels, upload their textures, and emit icon packets.
 */

#include "common.h"
#include "field_calls.h"
#include "gpu_packet.h"
#include "main.h"
#include "sdk/libgpu.h"

/*
 * VRAM layout of the three coordinate icons: each g_prim_rect_buf slot holds a
 * 16-color palette strip followed by a 12x48 (16-bit units) image block.
 */
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
/** @brief Upload source at byte @p offset of @p base, summed as integers like the original. */
#define PRIM_UPLOAD_PTR(base, offset) ((u_long*)(PRIM_ALIGN_UPLOAD_OFFSET(offset) + (u32)(base)))

/** @brief Width and height of the coordinate icon, minus one. */
#define FIELD_COORD_ICON_EXTENT 47

/** @brief Texture page holding the coordinate icons. */
#define FIELD_COORD_ICON_TPAGE getTPage(0, 0, 960, 256)

/** @brief Offset of the icon's drop shadow, in pixels. */
#define FIELD_COORD_ICON_SHADOW_OFFSET 2

s32 func_800AEAC0(POLY_FT4* handle, u_long* ordering_table, s32 selector, s32 x, s32 y, s32 flip);
s32 func_800A88A0(void* prim, void* ot, void* text, s32 color, s32 x, s32 y, s32 align);

extern s32 g_field_party_has_guest;
extern u8* g_field_coordinate_labels[];

/**
 * @brief Draw the coordinate panel icon and its optional label rows.
 * @param ot Ordering-table context passed to each label draw.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal offset subtracted from the panel position.
 * @param y_offset Vertical offset subtracted from the panel position.
 * @return Primitive-buffer cursor after all enabled rows.
 */
s32 func_800AE8A8(void* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 cursor;
    s32 unused[2]; /* never used; the original stack frame reserves it */

    cursor = func_800AEAC0((POLY_FT4*)prim, ot, g_field_party_has_guest, 2 - x_offset, -y_offset, 1);
    if (g_field_coordinate_labels[0] != 0)
    {
        cursor = func_800A88A0((void*)cursor, ot, g_field_coordinate_labels[0], 4, 0x38 - x_offset, 1 - y_offset, 0);
    }
    if (g_field_coordinate_labels[1] != 0)
    {
        cursor = func_800A88A0((void*)cursor, ot, g_field_coordinate_labels[1], 4, 0x38 - x_offset, 0x11 - y_offset, 0);
    }
    if (g_field_coordinate_labels[2] != 0)
    {
        cursor = func_800A88A0((void*)cursor, ot, g_field_coordinate_labels[2], 4, 0x38 - x_offset, 0x21 - y_offset, 0);
    }
    return cursor;
}

/**
 * @brief Upload the three coordinate-icon palette strips and image blocks to VRAM.
 *
 * Each of the three slots in g_prim_rect_buf holds a 16-color palette strip
 * followed by a 12x48 image block.
 */
void func_800AE9E0(void)
{
    s32 slot = 0;
    u8* scratch = g_prim_rect_buf;
    s32 block_byte_offset = PRIM_BLOCK_BUF_OFFSET;
    s32 strip_byte_offset = 0;
    RECT rect;
    u_long* upload_src;

    for (; slot < PRIM_SLOT_COUNT; slot++)
    {
        setRECT(&rect, PRIM_STRIP_VRAM_X, slot + PRIM_STRIP_VRAM_Y0, PRIM_STRIP_W, PRIM_STRIP_H);
        upload_src = PRIM_UPLOAD_PTR(scratch, strip_byte_offset);
        LoadImage(&rect, upload_src);

        setRECT(&rect, (slot == PRIM_SLOT_COUNT - 1) ? PRIM_BLOCK_VRAM_X2 : PRIM_BLOCK_VRAM_X, (slot == 0) ? PRIM_BLOCK_VRAM_Y0 : PRIM_BLOCK_VRAM_Y1,
                PRIM_BLOCK_W, PRIM_BLOCK_H);
        upload_src = PRIM_UPLOAD_PTR(scratch, block_byte_offset);
        LoadImage(&rect, upload_src);

        block_byte_offset += PRIM_SLOT_STRIDE;
        strip_byte_offset += PRIM_SLOT_STRIDE;
    }
}

/**
 * @brief Build the textured coordinate icon primitives and append them to the ordering table.
 * @param handle Primitive-buffer cursor.
 * @param ordering_table Ordering table entry that receives the generated primitives.
 * @param selector Icon index; picks the texture coordinates and palette.
 * @param x Horizontal screen coordinate.
 * @param y Vertical screen coordinate.
 * @param flip Nonzero to mirror the texture horizontally.
 * @return Primitive buffer cursor immediately after the generated draw-mode packet.
 */
s32 func_800AEAC0(POLY_FT4* handle, u_long* ordering_table, s32 selector, s32 x, s32 y, s32 flip)
{
    POLY_FT4* initial;
    POLY_FT4* prim;
    POLY_FT4* source;
    DR_TPAGE* draw_mode;
    u8 u_value;

    initial = handle;
    prim = handle;
    prim++; /* net-zero step; keeps prim a separate pointer from initial */
    prim--;
    SET_BGR0_PACKED(initial, GPU_TINT_NEUTRAL);
    initial->x0 = x;
    setPolyFT4(initial);
    initial->x2 = initial->x0;
    initial->x3 = initial->x0 + FIELD_COORD_ICON_EXTENT;
    initial->x1 = initial->x3;
    initial->y0 = y;
    initial->y1 = y;
    initial->y3 = initial->y0 + FIELD_COORD_ICON_EXTENT;
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
    prim->u3 = prim->u0 + FIELD_COORD_ICON_EXTENT;
    prim->u1 = prim->u3;
    prim->v1 = prim->v0;
    prim->v3 = prim->v0 + FIELD_COORD_ICON_EXTENT;
    prim->v2 = prim->v3;
    if (flip != 0)
    {
        u_value = prim->u0;
        prim->u0 = prim->u1;
        prim->u1 = u_value;
        u_value = prim->u2;
        prim->u2 = prim->u3;
        prim->u3 = u_value;
    }

    prim->clut = getClut(PRIM_STRIP_VRAM_X, selector + PRIM_STRIP_VRAM_Y0);
    prim->tpage = FIELD_COORD_ICON_TPAGE;
    addPrim(ordering_table, prim);

    source = prim;
    prim++;
    bcopy(source, prim, sizeof(POLY_FT4));

    SET_BGR0_PACKED(prim, 0);
    setPolyFT4(prim);
    setSemiTrans(prim, 1);
    prim->x0 = (u16)(prim->x0 + FIELD_COORD_ICON_SHADOW_OFFSET);
    prim->x1 = (u16)(prim->x1 + FIELD_COORD_ICON_SHADOW_OFFSET);
    prim->x2 = (u16)(prim->x2 + FIELD_COORD_ICON_SHADOW_OFFSET);
    prim->x3 = (u16)(prim->x3 + FIELD_COORD_ICON_SHADOW_OFFSET);
    prim->y0 = (u16)(prim->y0 + FIELD_COORD_ICON_SHADOW_OFFSET);
    prim->y1 = (u16)(prim->y1 + FIELD_COORD_ICON_SHADOW_OFFSET);
    prim->y2 = (u16)(prim->y2 + FIELD_COORD_ICON_SHADOW_OFFSET);
    prim->y3 = (u16)(prim->y3 + FIELD_COORD_ICON_SHADOW_OFFSET);
    addPrim(ordering_table, prim);

    prim++;
    draw_mode = (DR_TPAGE*)prim;
    setDrawTPage(draw_mode, 0, 0, FIELD_COORD_ICON_TPAGE);
    addPrim(ordering_table, draw_mode);
    return (s32)(draw_mode + 1);
}
