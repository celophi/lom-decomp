/* Partial WMAP decompilation: 99.481480% (gcc280_g0). */
#include "common.h"

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

typedef struct
{
    u8 pad_000[0x70];
    u32 order_tags[179];
    u8 *packet_cursor;
} WmapPacketBuffer;

extern WmapQuad D_800D043C;
extern WmapPacketBuffer *D_801398EC;
extern s32 D_800D921C;
extern s32 D_800D9228;
extern s32 D_8011D500;
extern s32 D_80182E38;

/** @brief Move the fade toward its target and append packets at the selected depth. */
s32 func_80064D64(void)
{
    u8 intensity;
    s32 depth;
    WmapQuad *quad;
    WmapTexturedTriangle *triangle;

    quad = (WmapQuad *)D_801398EC->packet_cursor;
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
    quad->header.tag = ((quad->header.tag & 0xFF000000) | (D_80182E38[D_801398EC->order_tags] & 0xFFFFFF));
    D_80182E38[D_801398EC->order_tags] = ((D_80182E38[D_801398EC->order_tags] & 0xFF000000) | ((u32) quad & 0xFFFFFF));
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += 0x18;
        D_801398EC->packet_cursor += sizeof(WmapQuad);
    }
    triangle = (WmapTexturedTriangle *)D_801398EC->packet_cursor;
    depth = D_80182E38;
    triangle->header.packet.length = 7;
    triangle->code = 0x24;
    *(s32 *)&triangle->x2 = 0x190;
    *(s32 *)&triangle->x1 = 0x190;
    *(s32 *)&triangle->x0 = 0x190;
    triangle->tpage = 0x40;
    triangle->header.tag = ((triangle->header.tag & 0xFF000000) | (depth[D_801398EC->order_tags] & 0xFFFFFF));
    depth[D_801398EC->order_tags] = ((depth[D_801398EC->order_tags] & 0xFF000000) | ((u32) triangle & 0xFFFFFF));
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += 0x20;
        D_801398EC->packet_cursor += sizeof(WmapTexturedTriangle);
    }
    return 1;
}
