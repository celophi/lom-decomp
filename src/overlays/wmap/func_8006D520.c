#include "common.h"
/** @brief Gouraud triangle packet with three packed screen coordinates. */
typedef struct
{
    union
    {
        u32 tag;
        struct
        {
            u8 address[3];
            u8 length;
        } packet;
    } header;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
} WmapTriangle;

/** @brief Packed screen coordinate and its signed components. */
typedef union
{
    s32 packed;
    struct
    {
        s16 x;
        s16 y;
    } point;
} WmapPoint;

/** @brief Screen coordinate represented in 16.16 fixed point. */
typedef struct
{
    s32 x;
    s32 y;
} WmapFixedPoint;

extern WmapPoint D_800D0FD4[];
extern WmapTriangle D_800D1814[];
extern WmapTriangle D_801B10B8[];
extern WmapFixedPoint D_8013A188[];
extern void func_8006D8F0(s32);

/** @brief Initialize triangle packets in both buffers and expand their coordinates. */
void func_8006D520(void)
{
    s32 i;

    func_8006D8F0(0);
    for (i = 0; i < 176; i++)
    {
        D_800D1814[i].header.packet.length = 6;
        D_800D1814[i].code = 0x32;
        *(s32 *)&D_800D1814[i].x0 = D_800D0FD4[i * 3].packed;
        *(s32 *)&D_800D1814[i].x1 = D_800D0FD4[i * 3 + 1].packed;
        *(s32 *)&D_800D1814[i].x2 = D_800D0FD4[i * 3 + 2].packed;
        D_801B10B8[i] = D_800D1814[i];
    }
    for (i = 0; i < 528; i++)
    {
        D_8013A188[i].x = D_800D0FD4[i].point.x << 16;
        D_8013A188[i].y = D_800D0FD4[i].point.y << 16;
    }
}
