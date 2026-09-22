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
    u8 pad_000[0x74];
    u32 order_tag;
    u8 pad_078[0x2C4];
    u8 *packet_cursor;
} WmapPacketBuffer;

extern WmapQuad D_800D043C;
extern WmapPacketBuffer *D_801398EC;
extern s32 D_800D921C;

/** @brief Append the overlay quad and texture-page triangle to the ordering table. */
void func_800641DC(void)
{
    WmapQuad *quad;
    WmapTexturedTriangle *triangle;

    quad = (WmapQuad *)D_801398EC->packet_cursor;
    *quad = D_800D043C;
    quad->b0 = 8;
    quad->g0 = 8;
    quad->r0 = 8;
    quad->header.packet.length = 5;
    quad->code = 0x2A;
    quad->header.tag = ((quad->header.tag & 0xFF000000) | (D_801398EC->order_tag & 0xFFFFFF));
    D_801398EC->order_tag = ((D_801398EC->order_tag & 0xFF000000) | ((u32) quad & 0xFFFFFF));
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += 0x18;
        D_801398EC->packet_cursor += sizeof(WmapQuad);
    }
    triangle = (WmapTexturedTriangle *)D_801398EC->packet_cursor;
    triangle->header.packet.length = 7;
    triangle->code = 0x24;
    *(s32 *)&triangle->x2 = 0x190;
    *(s32 *)&triangle->x1 = 0x190;
    *(s32 *)&triangle->x0 = 0x190;
    triangle->tpage = 0x40;
    triangle->header.tag = ((triangle->header.tag & 0xFF000000) | (D_801398EC->order_tag & 0xFFFFFF));
    D_801398EC->order_tag = ((D_801398EC->order_tag & 0xFF000000) | ((u32) triangle & 0xFFFFFF));
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += 0x20;
        D_801398EC->packet_cursor += sizeof(WmapTexturedTriangle);
    }
}
