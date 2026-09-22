#include "wmap_resource_support.h"
#include "wmap_effect_backdrop.h"

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

/** @brief Screen position or motion vector in 16.16 fixed point. */
typedef struct
{
    WmapFixed x;
    WmapFixed y;
} WmapFixedPoint;

/** @brief Render-buffer view exposing the backdrop ordering-table entry. */
typedef struct
{
    u8 pad_00[0x94];
    u32 order_tag;
} WmapOrderBuffer;

extern WmapPoint D_800D0FD4[];
extern WmapTriangle D_800D1814[];
extern WmapTriangle D_801B10B8[];
extern WmapFixedPoint D_8013A188[];
extern WmapFixedPoint* D_801B23F8;
extern WmapOrderBuffer* D_801398EC;
extern s32 D_8011CF70;
extern s32 D_8011CF74;
extern s32 D_80139244;
extern s32 D_80139960;
extern s32 D_8011CF58;
extern s32 D_80182D70;
extern WmapFixedPoint D_800D2B54[];
extern WmapFixedPoint D_800D3BD4[];

/** @brief Initialize triangle packets in both buffers and expand their coordinates. */
void func_8006D520(void)
{
    s32 i;

    func_8006D8F0(0);
    for (i = 0; i < 176; i++)
    {
        D_800D1814[i].header.packet.length = 6;
        D_800D1814[i].code = 0x32;
        /* Copy packed X/Y pairs into the packet coordinate fields. */
        *(s32*)&D_800D1814[i].x0 = D_800D0FD4[i * 3].packed;
        *(s32*)&D_800D1814[i].x1 = D_800D0FD4[i * 3 + 1].packed;
        *(s32*)&D_800D1814[i].x2 = D_800D0FD4[i * 3 + 2].packed;
        D_801B10B8[i] = D_800D1814[i];
    }
    for (i = 0; i < 528; i++)
    {
        D_8013A188[i].x.value = D_800D0FD4[i].point.x << 16;
        D_8013A188[i].y.value = D_800D0FD4[i].point.y << 16;
    }
}

/** @brief Update the transition mesh and append its triangles for rendering. */
void func_8006D674(void)
{
    WmapTriangle* triangles;
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
            triangles->header.tag = (triangles->header.tag & 0xFF000000) | (D_801398EC->order_tag & 0xFFFFFF);
            D_801398EC->order_tag = (D_801398EC->order_tag & 0xFF000000) | ((u32)triangles & 0xFFFFFF);
            triangles++;
        }
        func_8006534C(0x7B54, 9);
    }
}

/**
 * @brief Change the current effect selection and retain the previous selection.
 * @param selection New effect selection.
 */
void func_8006D870(s32 selection)
{
    s32 previous_selection;

    if (selection != D_8011CF58)
    {
        if (D_80139960 != 0)
        {
            func_80064F14();
        }
        if (selection == 0)
        {
            D_80139960 = -1;
        }
        else
        {
            D_80139960 = 1;
        }
        D_8011CF70 = 0x10;
        previous_selection = D_8011CF58;
        D_8011CF58 = selection;
        D_80182D70 = previous_selection;
    }
}

/**
 * @brief Select one of the two world-map data buffers.
 * @param use_second Nonzero selects the second buffer.
 */
void func_8006D8F0(s32 use_second)
{
    if (use_second != 0)
    {
        D_801B23F8 = D_800D3BD4;
        return;
    }
    D_801B23F8 = D_800D2B54;
}
