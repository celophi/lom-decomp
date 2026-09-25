#include "wmap_frame_render.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"
#include "sdk/libetc.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "cdrom.h"

#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8*)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;

typedef union
{
    u32 tag;
    struct
    {
        u8 address[3];
        u8 length;
    } packet;
} WmapPacketHeader;

typedef struct
{
    WmapPacketHeader header;
    u8 r0, g0, b0, code;
    s16 x0, y0, x1, y1, x2, y2, x3, y3;
} WmapQuad;

typedef struct
{
    WmapPacketHeader header;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad_1E;
} WmapTexturedTriangle;


extern WmapQuad D_800D043C;

extern u32 D_800D9238;
extern VECTOR D_80139870;
extern VECTOR D_80139888;
extern SVECTOR D_8013B238;
extern SVECTOR D_8013B240;
extern VECTOR D_801B2478;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B24A8;
extern s32 D_800DBE7C;
extern s32 D_800DCEDC;
extern u8 D_8010CF18[];
extern u8 D_80114F18[];
extern s32 D_8011CF74;
extern s32 D_80182E00;
extern s32 D_800D9228;
extern s32 D_8011D500;
extern s32 D_80182E38;

static void func_8006432C(VECTOR* left_top, VECTOR* center_top, VECTOR* left_bottom, VECTOR* center_bottom, VECTOR* right_top, VECTOR* right_bottom);

/** @brief Append the overlay quad and texture-page triangle to the ordering table. */
void func_800641DC(void)
{
    WmapQuad* quad;
    WmapTexturedTriangle* triangle;

    quad = (WmapQuad*)g_wmap_current_frame->packet_cursor;
    *quad = D_800D043C;
    quad->b0 = 8;
    quad->g0 = 8;
    quad->r0 = 8;
    quad->header.packet.length = 5;
    quad->code = 0x2A;
    quad->header.tag = ((quad->header.tag & 0xFF000000) | (g_wmap_current_frame->ordering_table[1] & 0xFFFFFF));
    g_wmap_current_frame->ordering_table[1] = ((g_wmap_current_frame->ordering_table[1] & 0xFF000000) | ((u32)quad & 0xFFFFFF));
    if (g_wmap_packet_bytes < 0x7D00)
    {
        g_wmap_packet_bytes += 0x18;
        g_wmap_current_frame->packet_cursor += sizeof(WmapQuad);
    }
    triangle = (WmapTexturedTriangle*)g_wmap_current_frame->packet_cursor;
    triangle->header.packet.length = 7;
    triangle->code = 0x24;
    *(s32*)&triangle->x2 = 0x190;
    *(s32*)&triangle->x1 = 0x190;
    *(s32*)&triangle->x0 = 0x190;
    triangle->tpage = 0x40;
    triangle->header.tag = ((triangle->header.tag & 0xFF000000) | (g_wmap_current_frame->ordering_table[1] & 0xFFFFFF));
    g_wmap_current_frame->ordering_table[1] = ((g_wmap_current_frame->ordering_table[1] & 0xFF000000) | ((u32)triangle & 0xFFFFFF));
    if (g_wmap_packet_bytes < 0x7D00)
    {
        g_wmap_packet_bytes += 0x20;
        g_wmap_current_frame->packet_cursor += sizeof(WmapTexturedTriangle);
    }
}

/**
 * @brief Draw two adjacent textured world-map quads from six projected vertices.
 * @param left_top Top-left vertex of the left quad.
 * @param center_top Shared top vertex between the two quads.
 * @param left_bottom Bottom-left vertex of the left quad.
 * @param center_bottom Shared bottom vertex between the two quads.
 * @param right_top Top-right vertex of the right quad.
 * @param right_bottom Bottom-right vertex of the right quad.
 */
static void func_8006432C(VECTOR* left_top, VECTOR* center_top, VECTOR* left_bottom, VECTOR* center_bottom, VECTOR* right_top, VECTOR* right_bottom)
{
    POLY_FT4* quad;

    quad = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
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

    addPrim(&g_wmap_current_frame->ordering_table[1], quad);
    if (g_wmap_packet_bytes < 0x7D00)
    {
        g_wmap_packet_bytes += sizeof(POLY_FT4);
        g_wmap_current_frame->packet_cursor = g_wmap_current_frame->packet_cursor + sizeof(POLY_FT4);
    }

    quad = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
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

    addPrim(&g_wmap_current_frame->ordering_table[1], quad);
    if (g_wmap_packet_bytes < 0x7D00)
    {
        g_wmap_packet_bytes += sizeof(POLY_FT4);
        g_wmap_current_frame->packet_cursor = g_wmap_current_frame->packet_cursor + sizeof(POLY_FT4);
    }
}

static __inline__ void project_panels(VECTOR* p38, VECTOR* p48, VECTOR* p58, VECTOR* p68, VECTOR* p78, VECTOR* p88)
{
    D_801B2490.vx = -0xA0;
    D_801B2490.vy = -0x78;
    D_801B2490.vz = 0;
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    D_801B2490.vx = 0x5F;
    D_801B2490.vy = -0x78;
    D_801B2490.vz = 0;
    gte_stlvnl(p38);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    D_801B2490.vx = -0xA0;
    D_801B2490.vy = 0x78;
    D_801B2490.vz = 0;
    gte_stlvnl(p48);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    D_801B2490.vx = 0x5F;
    D_801B2490.vy = 0x78;
    D_801B2490.vz = 0;
    gte_stlvnl(p58);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    D_801B2490.vx = 0xA0;
    D_801B2490.vy = -0x78;
    D_801B2490.vz = 0;
    gte_stlvnl(p68);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    D_801B2490.vx = 0xA0;
    D_801B2490.vy = 0x78;
    D_801B2490.vz = 0;
    gte_stlvnl(p78);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    gte_stlvnl(p88);
    func_8006432C(p38, p48, p58, p68, p78, p88);
}

/** @brief Update and project the paired world-map panels, then append their primitives. */
/* TODO: match the two adjacent LUI instructions in the packet setup; see working/func_8006454C/status.md. */
void func_8006454C(void)
{
    MATRIX sp18;
    VECTOR sp38;
    VECTOR sp48;
    VECTOR sp58;
    VECTOR sp68;
    VECTOR sp78;
    VECTOR sp88;
    s32 width;
    WmapQuad* quad;
    WmapTexturedTriangle* triangle;

    D_8013B238.vz = (u16)(D_8013B238.vz + D_801B24A8.vz);
    D_8013B238.vy = (u16)(D_8013B238.vy + D_801B24A8.vy);
    D_8013B238.vx = (u16)(D_8013B238.vx + D_801B24A8.vx);
    D_80139870.vz = (s32)(D_80139870.vz + D_801B2478.vz);
    D_80139870.vy = (s32)(D_80139870.vy + D_801B2478.vy);
    D_80139870.vx = (s32)(D_80139870.vx + D_801B2478.vx);
    RotMatrix(&D_8013B238, &sp18);
    TransMatrix(&sp18, &D_80139870);
    SetRotMatrix(&sp18);
    SetTransMatrix(&sp18);
    project_panels(&sp38, &sp48, &sp58, &sp68, &sp78, &sp88);
    D_8013B240.vz = (u16)(D_8013B240.vz - D_801B24A8.vz);
    D_8013B240.vy = (u16)(D_8013B240.vy - D_801B24A8.vy);
    D_8013B240.vx = (u16)(D_8013B240.vx - D_801B24A8.vx);
    D_80139888.vz = (s32)(D_80139888.vz - D_801B2478.vz);
    D_80139888.vy = (s32)(D_80139888.vy - D_801B2478.vy);
    D_80139888.vx = (s32)(D_80139888.vx - D_801B2478.vx);
    RotMatrix(&D_8013B240, &sp18);
    TransMatrix(&sp18, &D_80139888);
    SetRotMatrix(&sp18);
    SetTransMatrix(&sp18);
    project_panels(&sp38, &sp48, &sp58, &sp68, &sp78, &sp88);
    quad = (WmapQuad*)g_wmap_current_frame->packet_cursor;
    width = 0x140;
    quad->x3 = width;
    quad->x1 = width;
    quad->y3 = 0xF0;
    quad->y2 = 0xF0;
    quad->header.packet.length = 5;
    *(u32*)&quad->r0 = 0x101010;
    quad->code = 0x2A;
    quad->y1 = 0;
    quad->y0 = 0;
    quad->x2 = 0;
    quad->x0 = 0;
    addPrim(&g_wmap_current_frame->ordering_table[1], quad);
    if (g_wmap_packet_bytes < 0x7D00)
    {
        g_wmap_packet_bytes += 0x18;
        g_wmap_current_frame->packet_cursor = g_wmap_current_frame->packet_cursor + 0x18;
    }
    triangle = (WmapTexturedTriangle*)g_wmap_current_frame->packet_cursor;
    triangle->header.packet.length = 7;
    triangle->code = 0x24;
    *(s32*)&triangle->x2 = 0x190;
    *(s32*)&triangle->x1 = 0x190;
    *(s32*)&triangle->x0 = 0x190;
    triangle->tpage = 0x140;
    addPrim(&g_wmap_current_frame->ordering_table[1], triangle);
    if (g_wmap_packet_bytes < 0x7D00)
    {
        g_wmap_packet_bytes += 0x20;
        g_wmap_current_frame->packet_cursor = g_wmap_current_frame->packet_cursor + 0x20;
    }
}

/** @brief Select the back buffer, run effects, and submit a world-map frame. */
void func_80064AF8(void)
{
    if (D_8011CF74 & 1)
    {
        g_wmap_frames[0].packet_cursor = D_8010CF18;
        g_wmap_current_frame = &g_wmap_frames[0];
    }
    else
    {
        g_wmap_frames[1].packet_cursor = D_80114F18;
        g_wmap_current_frame = &g_wmap_frames[1];
    }
    g_wmap_packet_bytes = 0;
    ClearOTagR(g_wmap_current_frame->ordering_table, 179);
    D_800DBE7C = 0;
    D_8011CF74++;
    func_8006CB60();
    DrawSync(0);
    VSync(4);
    PutDispEnv(&g_wmap_current_frame->disp_env);
    PutDrawEnv(&g_wmap_current_frame->draw_env);
    SetGeomScreen(D_800DCEDC);
    DrawOTag(&g_wmap_current_frame->ordering_table[178]);
    cdrom_process_state();
    DrawSync(0);
}

/** @brief Append the overlay quad and texture-page triangle to the ordering table. */
void func_80064BF8(void)
{
    WmapQuad* quad;
    WmapTexturedTriangle* triangle;

    quad = (WmapQuad*)g_wmap_current_frame->packet_cursor;
    if (D_80182E00 >= 4)
    {
        *quad = D_800D043C;
        quad->r0 = quad->g0 = quad->b0 = D_80182E00;
        D_80182E00 -= 8;
        quad->header.packet.length = 5;
        quad->code = 0x2A;
        quad->header.tag = ((quad->header.tag & 0xFF000000) | (g_wmap_current_frame->ordering_table[1] & 0xFFFFFF));
        g_wmap_current_frame->ordering_table[1] = ((g_wmap_current_frame->ordering_table[1] & 0xFF000000) | ((u32)quad & 0xFFFFFF));
        if (g_wmap_packet_bytes < 0x7D00)
        {
            g_wmap_packet_bytes += 0x18;
            g_wmap_current_frame->packet_cursor += sizeof(WmapQuad);
        }
        triangle = (WmapTexturedTriangle*)g_wmap_current_frame->packet_cursor;
        triangle->header.packet.length = 7;
        triangle->code = 0x24;
        *(s32*)&triangle->x2 = 0x190;
        *(s32*)&triangle->x1 = 0x190;
        *(s32*)&triangle->x0 = 0x190;
        triangle->tpage = 0x40;
        triangle->header.tag = ((triangle->header.tag & 0xFF000000) | (g_wmap_current_frame->ordering_table[1] & 0xFFFFFF));
        g_wmap_current_frame->ordering_table[1] = ((g_wmap_current_frame->ordering_table[1] & 0xFF000000) | ((u32)triangle & 0xFFFFFF));
        if (g_wmap_packet_bytes < 0x7D00)
        {
            g_wmap_packet_bytes += 0x20;
            g_wmap_current_frame->packet_cursor += sizeof(WmapTexturedTriangle);
        }
    }
}

/**
 * @brief Move the fade toward its target and append packets at the selected depth.
 * @param initialize Sequence event selector, unused by this fade callback.
 * @return One to keep the callback registered.
 */
s32 func_80064D64(s32 initialize)
{
    u8 intensity;
    s32 depth;
    WmapQuad* quad;
    WmapTexturedTriangle* triangle;

    quad = (WmapQuad*)g_wmap_current_frame->packet_cursor;
    if (D_800D9228 > D_8011D500)
    {
        D_800D9228 -= 2;
    }
    else if (D_800D9228 < D_8011D500)
    {
        D_800D9228 += 2;
    }
    if (D_800D9228 == 0)
    {
        return 1;
    }
    intensity = D_800D9228;
    *quad = D_800D043C;
    quad->header.packet.length = 5;
    quad->code = 0x2A;
    quad->r0 = quad->g0 = quad->b0 = intensity;
    addPrim(&D_80182E38[g_wmap_current_frame->ordering_table], quad);
    if (g_wmap_packet_bytes < 0x7D00)
    {
        g_wmap_packet_bytes += 0x18;
        g_wmap_current_frame->packet_cursor += sizeof(WmapQuad);
    }
    triangle = (WmapTexturedTriangle*)g_wmap_current_frame->packet_cursor;
    depth = D_80182E38;
    triangle->header.packet.length = 7;
    triangle->code = 0x24;
    *(s32*)&triangle->x2 = 0x190;
    *(s32*)&triangle->x1 = 0x190;
    *(s32*)&triangle->x0 = 0x190;
    triangle->tpage = 0x40;
    addPrim(&depth[g_wmap_current_frame->ordering_table], triangle);
    if (g_wmap_packet_bytes < 0x7D00)
    {
        g_wmap_packet_bytes += 0x20;
        g_wmap_current_frame->packet_cursor += sizeof(WmapTexturedTriangle);
    }
    return 1;
}
