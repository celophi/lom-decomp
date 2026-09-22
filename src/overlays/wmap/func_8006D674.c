/* Partial WMAP decompilation: 97.157480% (gcc280_g0). */
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

/** @brief Fixed-point coordinate with direct access to its integer halfword. */
typedef union
{
    s32 value;
    struct
    {
        unsigned int fraction : 16;
        signed int whole : 16;
    } parts;
} WmapFixed;
typedef struct
{
    WmapFixed x;
    WmapFixed y;
} WmapFixedPoint;
typedef struct
{
    u8 pad_00[0x94];
    u32 order_tag;
} WmapOrderBuffer;
extern WmapTriangle D_800D1814[];
extern WmapTriangle D_801B10B8[];
extern WmapFixedPoint D_8013A188[];
extern WmapFixedPoint *D_801B23F8;
extern WmapOrderBuffer *D_801398EC;
extern s32 D_8011CF70;
extern s32 D_8011CF74;
extern s32 D_80139244;
extern s32 D_80139960;
extern void func_8006534C(s16, s32);

/** @brief Update the transition mesh and append its triangles for rendering. */
void func_8006D674(void)
{
    WmapTriangle *triangles;
    s32 i;
    s32 remaining;

    if (D_80139244 != 1)
    {
        if (D_80139960 != 0)
        {
            if (D_8011CF74 & 1)
            {
                triangles = D_800D1814;
            }
            else
            {
                triangles = D_801B10B8;
            }
            if (D_8011CF70 >= 2)
            {
                for (i = 0; i < 528; i++)
                {
                    D_8013A188[i].x.value += D_80139960 * D_801B23F8[i].x.value;
                    D_8013A188[i].y.value += D_80139960 * D_801B23F8[i].y.value;
                }
            }
            for (i = 0; i < 176; i++)
            {
                triangles[i].x0 = D_8013A188[i * 3].x.parts.whole;
                triangles[i].y0 = D_8013A188[i * 3].y.parts.whole;
                triangles[i].x1 = D_8013A188[i * 3 + 1].x.parts.whole;
                triangles[i].y1 = D_8013A188[i * 3 + 1].y.parts.whole;
                triangles[i].x2 = D_8013A188[i * 3 + 2].x.parts.whole;
                triangles[i].y2 = D_8013A188[i * 3 + 2].y.parts.whole;
            }
            remaining = D_8011CF70 - 1;
            D_8011CF70 = remaining;
            if (remaining == 0)
            {
                D_80139960 = 0;
            }
        }
        if (D_8011CF74 & 1)
        {
            triangles = D_800D1814;
        }
        else
        {
            triangles = D_801B10B8;
        }
        for (i = 0; i < 176; i++)
        {
            triangles->header.tag = (triangles->header.tag & 0xFF000000) |
                                 (D_801398EC->order_tag & 0xFFFFFF);
            D_801398EC->order_tag = (D_801398EC->order_tag & 0xFF000000) |
                                    ((u32)triangles & 0xFFFFFF);
            triangles++;
        }
        func_8006534C(0x7B54, 9);
    }
}
