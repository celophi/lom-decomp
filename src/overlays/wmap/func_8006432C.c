#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"

typedef struct
{
    u8 pad_000[0x70];
    u_long ordering_table[179];
    void* prim_cursor;
} WmapRenderState;

extern s32 D_800D921C;
extern u32 D_800D9238;
extern WmapRenderState* D_801398EC;

/**
 * @brief Draw two adjacent textured world-map quads from six projected vertices.
 * @param left_top Top-left vertex of the left quad.
 * @param center_top Shared top vertex between the two quads.
 * @param left_bottom Bottom-left vertex of the left quad.
 * @param center_bottom Shared bottom vertex between the two quads.
 * @param right_top Top-right vertex of the right quad.
 * @param right_bottom Bottom-right vertex of the right quad.
 */
void func_8006432C(VECTOR* left_top, VECTOR* center_top, VECTOR* left_bottom, VECTOR* center_bottom, VECTOR* right_top, VECTOR* right_bottom)
{
    POLY_FT4* quad;

    quad = D_801398EC->prim_cursor;
    quad->x0 = left_top->vx;
    quad->y0 = left_top->vy;
    quad->x1 = center_top->vx;
    quad->y1 = center_top->vy;
    quad->x2 = left_bottom->vx;
    quad->y2 = left_bottom->vy;
    quad->x3 = center_bottom->vx;
    quad->y3 = center_bottom->vy;

    quad->v0 = quad->v1 = 0;
    quad->u0 = quad->u2 = 0;
    quad->u1 = quad->u3 = 248;
    quad->v2 = quad->v3 = 240;
    *(u32*)&quad->r0 = D_800D9238;
    quad->u0 = quad->v0 = 0;
    setClut(quad, 0, 0);
    setTPage(quad, 2, 1, 0x140, 0);
    setPolyFT4(quad);
    setSemiTrans(quad, 1);

    addPrim(&D_801398EC->ordering_table[1], quad);
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(POLY_FT4);
        D_801398EC->prim_cursor = (u8*)D_801398EC->prim_cursor + sizeof(POLY_FT4);
    }

    quad = D_801398EC->prim_cursor;
    quad->x0 = center_top->vx;
    quad->y0 = center_top->vy;
    quad->x1 = right_top->vx;
    quad->y1 = right_top->vy;
    quad->x2 = center_bottom->vx;
    quad->y2 = center_bottom->vy;
    quad->x3 = right_bottom->vx;
    quad->y3 = right_bottom->vy;

    quad->v0 = quad->v1 = 0;
    quad->u0 = quad->u2 = 0;
    quad->u1 = quad->u3 = 63;
    quad->v2 = quad->v3 = 240;
    *(u32*)&quad->r0 = D_800D9238;
    quad->u0 = quad->v0 = 0;
    setClut(quad, 0, 0);
    setTPage(quad, 2, 1, 0x240, 0);
    setPolyFT4(quad);
    setSemiTrans(quad, 1);

    addPrim(&D_801398EC->ordering_table[1], quad);
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(POLY_FT4);
        D_801398EC->prim_cursor = (u8*)D_801398EC->prim_cursor + sizeof(POLY_FT4);
    }
}
