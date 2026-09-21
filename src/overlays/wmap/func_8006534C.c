#include "common.h"
#include "sdk/libgpu.h"

/** @brief GPU packet for a flat-shaded textured triangle. */
typedef struct
{
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad;
} WmapTexturedTriangle;

/** @brief World-map ordering table and current primitive allocation cursor. */
typedef struct
{
    u8 unknown_00[0x70];
    u32 ordering_table[(0x33C - 0x70) / 4];
    u8 *primitive_cursor;
} WmapRenderContext;

extern WmapRenderContext *D_801398EC;
extern s32 D_800D921C;

/**
 * @brief Link an offscreen textured triangle that selects the texture page.
 * @param texture_page GPU texture-page bits.
 * @param depth Ordering-table index.
 */
void func_8006534C(s16 texture_page, s32 depth)
{
    WmapTexturedTriangle *primitive;
    WmapRenderContext *table_base;

    primitive = (WmapTexturedTriangle *)D_801398EC->primitive_cursor;
    ((u8 *)&primitive->tag)[3] = 7;
    primitive->code = 0x24;
    primitive->tpage = texture_page;
    *(s32 *)&primitive->x2 = 400;
    *(s32 *)&primitive->x1 = 400;
    *(s32 *)&primitive->x0 = 400;
    table_base = (WmapRenderContext *)(depth * 4 + (s32)D_801398EC);
    primitive->tag = (primitive->tag & 0xFF000000) | (table_base->ordering_table[0] & 0xFFFFFF);
    table_base->ordering_table[0] = (table_base->ordering_table[0] & 0xFF000000) | ((u32)primitive & 0xFFFFFF);
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += 32;
        D_801398EC->primitive_cursor += 32;
    }
}
