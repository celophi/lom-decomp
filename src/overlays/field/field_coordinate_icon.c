/** @file field_coordinate_icon.c
 * @brief Coordinate panel and the 48x48 player icons it shares with the duel screens.
 */

#include "common.h"
#include "field_calls.h"
#include "gpu_packet.h"
#include "main.h"
#include "sdk/libgpu.h"

/*
 * VRAM layout of the three player icons: each g_prim_rect_buf slot holds a
 * 16-color palette strip followed by a 12x48 (16-bit units) image block.
 */
#define PRIM_STRIP_VRAM_X 0x110
#define PRIM_STRIP_VRAM_Y0 0x1D8
#define PRIM_STRIP_W 16
#define PRIM_STRIP_H 1
#define PRIM_BLOCK_VRAM_X 0x3F4
#define PRIM_BLOCK_VRAM_X2 0x3E8
#define PRIM_BLOCK_VRAM_Y0 0x120
#define PRIM_BLOCK_VRAM_Y1 0x150
#define PRIM_BLOCK_W 12
#define PRIM_BLOCK_H 48
#define PRIM_SLOT_COUNT 3
#define PRIM_STRIP_BYTE_SIZE (PRIM_STRIP_W * PRIM_STRIP_H * sizeof(u16))
#define PRIM_BLOCK_BYTE_SIZE (PRIM_BLOCK_W * PRIM_BLOCK_H * sizeof(u16))
#define PRIM_BLOCK_BUF_OFFSET PRIM_STRIP_BYTE_SIZE
#define PRIM_SLOT_STRIDE (PRIM_STRIP_BYTE_SIZE + PRIM_BLOCK_BYTE_SIZE)

/**
 * @brief Word-aligned upload source at byte @p offset of @p base.
 * @note Summed as integers: the target adds the scaled offset before the base.
 */
#define PRIM_UPLOAD_PTR(base, offset) ((u_long*)((((offset) >> 2) << 2) + (u32)(base)))

/** @brief Player icon selectors (the palette row and the texture cell). */
#define FIELD_PLAYER_ICON_FIRST 0
#define FIELD_PLAYER_ICON_SECOND 1
#define FIELD_PLAYER_ICON_THIRD 2

/** @brief Width and height of a player icon, minus one. */
#define FIELD_PLAYER_ICON_EXTENT 47

/** @brief Texture cells of the icons inside FIELD_PLAYER_ICON_TPAGE. */
#define FIELD_PLAYER_ICON_U 0xD0
#define FIELD_PLAYER_ICON_THIRD_U 0xA0
#define FIELD_PLAYER_ICON_FIRST_V 0x20
#define FIELD_PLAYER_ICON_V 0x50

/** @brief Texture page holding the player icons. */
#define FIELD_PLAYER_ICON_TPAGE getTPage(0, 0, 960, 256)

/** @brief Offset of an icon's drop shadow, in pixels. */
#define FIELD_PLAYER_ICON_SHADOW_OFFSET 2

/** @brief Coordinate panel layout: icon position and the three label rows. */
#define FIELD_COORDINATE_ICON_X 2
#define FIELD_COORDINATE_LABEL_X 56
#define FIELD_COORDINATE_LABEL_Y 1
#define FIELD_COORDINATE_LABEL_ROW_HEIGHT 16
#define FIELD_COORDINATE_LABEL_COLOR 4

s32 field_draw_player_icon(POLY_FT4* prim, u_long* ordering_table, s32 selector, s32 x, s32 y, s32 flip);
s32 field_draw_text(void* prim, void* ot, void* text, s32 color, s32 x, s32 y, s32 align);

extern s32 g_field_party_has_guest;
extern u8* g_field_coordinate_labels[];

/**
 * @brief Draw the coordinate panel: the second player's icon and the label rows present.
 * @param ot Ordering table passed to each draw.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal offset subtracted from the panel position.
 * @param y_offset Vertical offset subtracted from the panel position.
 * @return Primitive-buffer cursor after all enabled rows.
 */
s32 field_draw_coordinate_panel(void* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 cursor;
    s32 unused[2]; /* never used; the original stack frame reserves it */

    cursor = field_draw_player_icon((POLY_FT4*)prim, ot, g_field_party_has_guest, FIELD_COORDINATE_ICON_X - x_offset, -y_offset, 1);
    if (g_field_coordinate_labels[0] != NULL)
    {
        cursor = field_draw_text((void*)cursor, ot, g_field_coordinate_labels[0], FIELD_COORDINATE_LABEL_COLOR, FIELD_COORDINATE_LABEL_X - x_offset,
                               FIELD_COORDINATE_LABEL_Y - y_offset, 0);
    }
    if (g_field_coordinate_labels[1] != NULL)
    {
        cursor = field_draw_text((void*)cursor, ot, g_field_coordinate_labels[1], FIELD_COORDINATE_LABEL_COLOR, FIELD_COORDINATE_LABEL_X - x_offset,
                               FIELD_COORDINATE_LABEL_Y + FIELD_COORDINATE_LABEL_ROW_HEIGHT - y_offset, 0);
    }
    if (g_field_coordinate_labels[2] != NULL)
    {
        cursor = field_draw_text((void*)cursor, ot, g_field_coordinate_labels[2], FIELD_COORDINATE_LABEL_COLOR, FIELD_COORDINATE_LABEL_X - x_offset,
                               FIELD_COORDINATE_LABEL_Y + 2 * FIELD_COORDINATE_LABEL_ROW_HEIGHT - y_offset, 0);
    }
    return cursor;
}

/**
 * @brief Upload the palette strip and image block of the three player icons to VRAM.
 */
void field_upload_player_icons(void)
{
    s32 slot = 0;
    u8* buffer = g_prim_rect_buf;
    s32 block_offset = PRIM_BLOCK_BUF_OFFSET;
    s32 strip_offset = 0;
    RECT rect;
    u_long* upload_src;

    for (; slot < PRIM_SLOT_COUNT; slot++)
    {
        setRECT(&rect, PRIM_STRIP_VRAM_X, slot + PRIM_STRIP_VRAM_Y0, PRIM_STRIP_W, PRIM_STRIP_H);
        upload_src = PRIM_UPLOAD_PTR(buffer, strip_offset);
        LoadImage(&rect, upload_src);

        setRECT(&rect, (slot == PRIM_SLOT_COUNT - 1) ? PRIM_BLOCK_VRAM_X2 : PRIM_BLOCK_VRAM_X, (slot == 0) ? PRIM_BLOCK_VRAM_Y0 : PRIM_BLOCK_VRAM_Y1,
                PRIM_BLOCK_W, PRIM_BLOCK_H);
        upload_src = PRIM_UPLOAD_PTR(buffer, block_offset);
        LoadImage(&rect, upload_src);

        block_offset += PRIM_SLOT_STRIDE;
        strip_offset += PRIM_SLOT_STRIDE;
    }
}

/**
 * @brief Append a 48x48 player icon, its drop shadow and a draw-mode packet to the ordering table.
 * @param handle Primitive-buffer cursor.
 * @param ordering_table Ordering table entry that receives the primitives.
 * @param selector Icon index (FIELD_PLAYER_ICON_*); picks the texture cell and palette.
 * @param x Left screen coordinate.
 * @param y Top screen coordinate.
 * @param flip Nonzero to mirror the texture horizontally.
 * @return Primitive-buffer cursor after the draw-mode packet.
 */
s32 field_draw_player_icon(POLY_FT4* handle, u_long* ordering_table, s32 selector, s32 x, s32 y, s32 flip)
{
    POLY_FT4* icon;
    POLY_FT4* prim;
    POLY_FT4* source;
    DR_TPAGE* draw_mode;
    u8 u_value;

    icon = handle;
    prim = handle;
    prim++; /* net-zero step: keeps the texture half in its own pointer (icon/prim register split) */
    prim--;
    SET_BGR0_PACKED(icon, GPU_TINT_NEUTRAL);
    icon->x0 = x;
    setPolyFT4(icon);
    icon->x2 = icon->x0;
    icon->x1 = icon->x3 = icon->x0 + FIELD_PLAYER_ICON_EXTENT;
    icon->y0 = y;
    icon->y1 = y;
    icon->y2 = icon->y3 = icon->y0 + FIELD_PLAYER_ICON_EXTENT;
    icon->u0 = (selector == FIELD_PLAYER_ICON_THIRD) ? FIELD_PLAYER_ICON_THIRD_U : FIELD_PLAYER_ICON_U;

    prim->v0 = (selector == FIELD_PLAYER_ICON_FIRST) ? FIELD_PLAYER_ICON_FIRST_V : FIELD_PLAYER_ICON_V;
    prim->u2 = prim->u0;
    prim->u1 = prim->u3 = prim->u0 + FIELD_PLAYER_ICON_EXTENT;
    prim->v1 = prim->v0;
    prim->v2 = prim->v3 = prim->v0 + FIELD_PLAYER_ICON_EXTENT;
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
    prim->tpage = FIELD_PLAYER_ICON_TPAGE;
    addPrim(ordering_table, prim);

    /* The shadow is a black, semi-transparent copy moved down and right. */
    source = prim;
    prim++;
    bcopy(source, prim, sizeof(POLY_FT4));
    SET_BGR0_PACKED(prim, 0);
    setPolyFT4(prim);
    setSemiTrans(prim, 1);
    prim->x0 += FIELD_PLAYER_ICON_SHADOW_OFFSET;
    prim->x1 += FIELD_PLAYER_ICON_SHADOW_OFFSET;
    prim->x2 += FIELD_PLAYER_ICON_SHADOW_OFFSET;
    prim->x3 += FIELD_PLAYER_ICON_SHADOW_OFFSET;
    prim->y0 += FIELD_PLAYER_ICON_SHADOW_OFFSET;
    prim->y1 += FIELD_PLAYER_ICON_SHADOW_OFFSET;
    prim->y2 += FIELD_PLAYER_ICON_SHADOW_OFFSET;
    prim->y3 += FIELD_PLAYER_ICON_SHADOW_OFFSET;
    addPrim(ordering_table, prim);

    prim++;
    draw_mode = (DR_TPAGE*)prim;
    setDrawTPage(draw_mode, 0, 0, FIELD_PLAYER_ICON_TPAGE);
    addPrim(ordering_table, draw_mode);
    return (s32)(draw_mode + 1);
}
