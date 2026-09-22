#include "wmap_model_render.h"
#include "wmap_resource_support.h"
/* Partial WMAP decompilation: 97.333680% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Packed world-map model vertex. */
typedef struct
{
    u16 x;
    u16 y;
    u16 z;
    u16 pad;
} WmapMeshVertex;

/** @brief World-map face flags and vertex indices. */
typedef struct
{
    u8 code;
    u8 semi_trans;
    s16 vertex[4];
} WmapMeshFace;

/** @brief Flat-shaded world-map triangle packet. */
typedef struct
{
    u_long tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
} WmapPolyF3;

/** @brief Flat-shaded textured world-map triangle packet. */
typedef struct
{
    u_long tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
} WmapPolyFT3;

/** @brief Gouraud-shaded world-map triangle packet. */
typedef struct
{
    u_long tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
} WmapPolyG3;

/** @brief Gouraud-shaded textured world-map triangle packet. */
typedef struct
{
    u_long tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad3;
} WmapPolyGT3;

/** @brief World-map ordering table and primitive allocation cursor. */
typedef struct
{
    u8 pad000[0x70];
    u_long ot[179];
    u8* prim_cursor;
} WmapRenderState;

extern s32 D_800D921C;
extern WmapRenderState* D_801398EC;


#define LOAD_VERTEX(dst, source, index)                                                                                                                         \
    do                                                                                                                                                           \
    {                                                                                                                                                            \
        (dst).x = (source)[(index)].x;                                                                                                                          \
        (dst).y = (source)[(index)].y;                                                                                                                          \
        (dst).z = (source)[(index)].z;                                                                                                                          \
    } while (0)

#define LOAD_VERTEX_SCALED(dst, source, index, divisor)                                                                                                         \
    do                                                                                                                                                           \
    {                                                                                                                                                            \
        (dst).x = (source)[(index)].x;                                                                                                                          \
        (dst).y = (source)[(index)].y;                                                                                                                          \
        (dst).z = (u16)((s16)(source)[(index)].z / (divisor));                                                                                                  \
    } while (0)

#define OFFSET_SXY(value, dx, dy)                                                                                                                               \
    do                                                                                                                                                           \
    {                                                                                                                                                            \
        ((volatile u16*)&(value))[0] += (dx);                                                                                                                            \
        ((volatile u16*)&(value))[1] += (dy);                                                                                                                            \
    } while (0)

#define SCALE_COLOR(color, scale, product)                                                                                                                      \
    do                                                                                                                                                           \
    {                                                                                                                                                            \
        if ((scale) != -1)                                                                                                                                      \
        {                                                                                                                                                        \
            (product) = (color).r * (scale);                                                                                                                    \
            (color).r = (product) >> 7;                                                                                                                         \
            (color).g = ((s32)(color).g * (scale)) >> 7;                                                                                                        \
            (color).b = ((s32)(color).b * (scale)) >> 7;                                                                                                        \
        }                                                                                                                                                        \
    } while (0)

#define LINK_PACKET(state, ot_index, packet) addPrim(&(state)->ot[(ot_index)], (packet))

#define ADD_PRIM_TEST(ordering_table, prim) \
    { \
        P_TAG* test_ot = (P_TAG*)(ordering_table); \
        P_TAG* test_prim = (P_TAG*)(prim); \
        test_prim->addr = test_ot->addr; \
        test_ot->addr = (u32)test_prim; \
    }

/**
 * @brief Render a primitive list from a world-map model resource.
 * @note Project object comparison: 97.330210% matching.
 * @note In-progress import; source filename reports 97.333680% matching.
 * @param resource_table Resource table containing model offsets.
 * @param resource_index Model entry index.
 * @param ot_index Base ordering-table index.
 * @param tpage Texture-page value written to textured primitives.
 * @param clut CLUT value written to textured primitives.
 * @param blend_mode Primitive blend mode and culling flags.
 * @param color_scale Color intensity scale and texture flags.
 * @param x_offset Screen-space X offset.
 * @param y_offset Screen-space Y offset.
 * @param z_divisor Optional Z divisor, or -1 to leave Z unchanged.
 */
void func_800675F0(u8* resource_table, s32 resource_index, s32 ot_index, s32 tpage, s32 clut, s32 blend_mode, s32 color_scale, s32 x_offset, s32 y_offset,
                   s32 z_divisor)
{
    u8* model;
    WmapMeshVertex* vertices;
    u8* face_data;
    u8* next_face;
    u8* attributes;
    u8* next_attributes;
    s32 face_count;
    s32 i;
    s32 backface_mode;
    s32 packed0;
    s32 packed1;
    s32 packed2;
    s32 packed3;
    s32 triangle_area;
    s32 color_product;
    s32 effective_scale;
    WmapMeshVertex transformed[4];
    CVECTOR colors[20];

    if (blend_mode & 0x1000)
    {
        blend_mode &= 0xFFF;
        backface_mode = 1;
    }
    else
    {
        backface_mode = 0;
    }

    model = resource_table + *(s32*)(resource_table + resource_index * 4 + 4);
    vertices = (WmapMeshVertex*)(model + 4);
    face_count = *(s16*)(model + 2);
    face_data = (u8*)(vertices + *(s16*)model);
    attributes = face_data + face_count * 10;

    if (face_count < 0)
    {
        func_80064F14(attributes);
        return;
    }

    if (z_divisor == 0)
    {
        z_divisor = 1;
    }

    i = 0;
    if (face_count > 0)
    {
        effective_scale = color_scale & 0xFFFF;
        next_face = face_data + 10;
        next_attributes = attributes + 24;
        do
        {
            switch (*face_data)
            {
            case 0x2C:
            {
                POLY_FT4* poly = (POLY_FT4*)D_801398EC->prim_cursor;
                if (z_divisor == -1)
                {
                    LOAD_VERTEX(transformed[0], vertices, *(s16*)(next_face -8));
                    LOAD_VERTEX(transformed[1], vertices, *(s16*)(next_face -6));
                    LOAD_VERTEX(transformed[2], vertices, *(s16*)(next_face -4));
                    LOAD_VERTEX(transformed[3], vertices, *(s16*)(next_face -2));
                }
                else
                {
                    LOAD_VERTEX_SCALED(transformed[0], vertices, *(s16*)(next_face -8), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[1], vertices, *(s16*)(next_face -6), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[2], vertices, *(s16*)(next_face -4), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[3], vertices, *(s16*)(next_face -2), z_divisor);
                }
                gte_ldv3(&transformed[0], &transformed[1], &transformed[2]);
                gte_rtpt();
                *(u16*)&poly->u0 = *(u16*)(next_attributes - 24);
                *(u16*)&poly->u1 = *(u16*)(next_attributes - 16);
                *(u16*)&poly->u2 = *(u16*)(next_attributes - 14);
                *(u16*)&poly->u3 = *(u16*)(next_attributes - 12);
                gte_stsxy3(&packed0, &packed1, &packed2);
                gte_nclip();
                OFFSET_SXY(packed0, x_offset, y_offset);
                gte_stopz(&triangle_area);
                if (backface_mode == 0)
                {
                    if (triangle_area < 0)
                    {
                        break;
                    }
                    triangle_area = 0;
                }
                else
                {
                    if (triangle_area < 0)
                    {
                        triangle_area = 4;
                    }
                    else
                    {
                        triangle_area = 0;
                    }
                }
                OFFSET_SXY(packed1, x_offset, y_offset);
                OFFSET_SXY(packed2, x_offset, y_offset);
                gte_ldv0(&transformed[3]);
                gte_rtps();
                poly->tpage = tpage;
                poly->clut = clut;
                gte_stsxy(&packed3);
                OFFSET_SXY(packed3, x_offset, y_offset);
                *(s32*)&poly->x0 = packed0;
                *(s32*)&poly->x1 = packed1;
                *(s32*)&poly->x2 = packed2;
                *(s32*)&poly->x3 = packed3;
                colors[0] = *(CVECTOR*)(next_attributes - 16);
                if (effective_scale != -1)
                {
                    color_product = colors[0].r * effective_scale;
                    colors[0].r = color_product >> 7;
                    colors[0].g = ((s32)colors[0].g * effective_scale) >> 7;
                    colors[0].b = ((s32)colors[0].b * effective_scale) >> 7;
                }
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                do
                {
                    setlen(poly, 9);
                    setcode(poly, 0x2C);
                    if (!(color_scale & 0x10000))
                    {
                        setcode(poly, *(u8*)(next_face - 9) ? 0x2E : 0x2C);
                    }
                    { P_TAG* test_ot = (P_TAG*)(&D_801398EC->ot[ot_index + triangle_area]); P_TAG* test_prim = (P_TAG*)poly; test_prim->addr = test_ot->addr; do { test_ot->addr = (u32)test_prim; } while (0); }
                } while (0);
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->prim_cursor += sizeof(POLY_FT4);
                }
                break;
            }
            case 0x24:
            {
                WmapPolyFT3* poly = (WmapPolyFT3*)D_801398EC->prim_cursor;
                if (z_divisor == -1)
                {
                    LOAD_VERTEX(transformed[0], vertices, *(s16*)(next_face -8));
                    LOAD_VERTEX(transformed[1], vertices, *(s16*)(next_face -6));
                    LOAD_VERTEX(transformed[2], vertices, *(s16*)(next_face -4));
                }
                else
                {
                    LOAD_VERTEX_SCALED(transformed[0], vertices, *(s16*)(next_face -8), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[1], vertices, *(s16*)(next_face -6), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[2], vertices, *(s16*)(next_face -4), z_divisor);
                }
                gte_ldv3(&transformed[0], &transformed[1], &transformed[2]);
                gte_rtpt();
                poly->tpage = tpage;
                poly->clut = clut;
                *(u16*)&poly->u0 = *(u16*)(attributes + 0);
                *(u16*)&poly->u1 = *(u16*)(next_attributes - 16);
                *(u16*)&poly->u2 = *(u16*)(next_attributes - 14);
                gte_stsxy3(&packed0, &packed1, &packed2);
                gte_nclip();
                OFFSET_SXY(packed0, x_offset, y_offset);
                OFFSET_SXY(packed1, x_offset, y_offset);
                OFFSET_SXY(packed2, x_offset, y_offset);
                gte_stopz(&triangle_area);
                if (backface_mode == 0)
                {
                    if (triangle_area < 0)
                    {
                        break;
                    }
                    triangle_area = 0;
                }
                else
                {
                    if (triangle_area < 0)
                    {
                        triangle_area = 4;
                    }
                    else
                    {
                        triangle_area = 0;
                    }
                }
                *(s32*)&poly->x0 = packed0;
                *(s32*)&poly->x1 = packed1;
                *(s32*)&poly->x2 = packed2;
                colors[0] = *(CVECTOR*)(next_attributes - 16);
                if (effective_scale != -1)
                {
                    color_product = colors[0].r * effective_scale;
                    colors[0].r = color_product >> 7;
                    colors[0].g = ((s32)colors[0].g * effective_scale) >> 7;
                    colors[0].b = ((s32)colors[0].b * effective_scale) >> 7;
                }
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                setlen(poly, 7);
                setcode(poly, 0x24);
                if (!(color_scale & 0x10000))
                {
                    setcode(poly, *(u8*)(next_face - 9) ? 0x26 : 0x24);
                }
                ADD_PRIM_TEST(&D_801398EC->ot[ot_index + triangle_area], poly);
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(WmapPolyFT3);
                    D_801398EC->prim_cursor += sizeof(WmapPolyFT3);
                }
                break;
            }
            case 0x38:
            {
                POLY_G4* poly = (POLY_G4*)D_801398EC->prim_cursor;
                LOAD_VERTEX(transformed[0], vertices, *(s16*)(next_face -8));
                LOAD_VERTEX(transformed[1], vertices, *(s16*)(next_face -6));
                LOAD_VERTEX(transformed[2], vertices, *(s16*)(next_face -4));
                gte_ldv3(&transformed[0], &transformed[1], &transformed[2]);
                gte_rtpt();
                LOAD_VERTEX(transformed[3], vertices, *(s16*)(next_face -2));
                colors[0] = *(CVECTOR*)(next_attributes - 16);
                SCALE_COLOR(colors[0], color_scale, color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                colors[4] = *(CVECTOR*)(next_attributes - 12);
                SCALE_COLOR(colors[4], color_scale, color_product);
                *(s32*)&poly->r1 = *(s32*)&colors[4];
                gte_stsxy3(&packed0, &packed1, &packed2);
                gte_nclip();
                colors[8] = *(CVECTOR*)(next_attributes - 8);
                SCALE_COLOR(colors[8], color_scale, color_product);
                *(s32*)&poly->r2 = *(s32*)&colors[8];
                colors[12] = *(CVECTOR*)(next_attributes - 4);
                SCALE_COLOR(colors[12], color_scale, color_product);
                *(s32*)&poly->r3 = *(s32*)&colors[12];
                gte_stopz(&triangle_area);
                if (backface_mode == 0)
                {
                    if (triangle_area < 0)
                    {
                        break;
                    }
                    triangle_area = 0;
                }
                else
                {
                    if (triangle_area < 0)
                    {
                        triangle_area = 4;
                    }
                    else
                    {
                        triangle_area = 0;
                    }
                }
                OFFSET_SXY(packed0, x_offset, y_offset);
                OFFSET_SXY(packed1, x_offset, y_offset);
                OFFSET_SXY(packed2, x_offset, y_offset);
                gte_ldv0(&transformed[3]);
                gte_rtps();
                *(s32*)&poly->x0 = packed0;
                *(s32*)&poly->x1 = packed1;
                gte_stsxy(&packed3);
                OFFSET_SXY(packed3, x_offset, y_offset);
                *(s32*)&poly->x2 = packed2;
                setlen(poly, 8);
                setcode(poly, 0x38);
                *(s32*)&poly->x3 = packed3;
                if (blend_mode >= 0)
                {
                    setcode(poly, 0x3A);
                }
                ADD_PRIM_TEST(&D_801398EC->ot[ot_index + triangle_area], poly);
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_G4);
                    D_801398EC->prim_cursor += sizeof(POLY_G4);
                }
                break;
            }
            case 0x30:
            {
                WmapPolyG3* poly = (WmapPolyG3*)D_801398EC->prim_cursor;
                if (z_divisor == -1)
                {
                    LOAD_VERTEX(transformed[0], vertices, *(s16*)(next_face -8));
                    LOAD_VERTEX(transformed[1], vertices, *(s16*)(next_face -6));
                    LOAD_VERTEX(transformed[2], vertices, *(s16*)(next_face -4));
                }
                else
                {
                    LOAD_VERTEX_SCALED(transformed[0], vertices, *(s16*)(next_face -8), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[1], vertices, *(s16*)(next_face -6), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[2], vertices, *(s16*)(next_face -4), z_divisor);
                }
                gte_ldv3(&transformed[0], &transformed[1], &transformed[2]);
                gte_rtpt();
                colors[0] = *(CVECTOR*)(next_attributes - 16);
                SCALE_COLOR(colors[0], color_scale, color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                gte_stsxy3(&packed0, &packed1, &packed2);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (backface_mode == 0)
                {
                    if (triangle_area < 0)
                    {
                        break;
                    }
                    triangle_area = 0;
                }
                else
                {
                    if (triangle_area < 0)
                    {
                        triangle_area = 4;
                    }
                    else
                    {
                        triangle_area = 0;
                    }
                }
                OFFSET_SXY(packed0, x_offset, y_offset);
                OFFSET_SXY(packed1, x_offset, y_offset);
                OFFSET_SXY(packed2, x_offset, y_offset);
                *(s32*)&poly->x0 = packed0;
                *(s32*)&poly->x1 = packed1;
                *(s32*)&poly->x2 = packed2;
                colors[4] = *(CVECTOR*)(next_attributes - 12);
                SCALE_COLOR(colors[4], color_scale, color_product);
                *(s32*)&poly->r1 = *(s32*)&colors[4];
                colors[8] = *(CVECTOR*)(next_attributes - 8);
                SCALE_COLOR(colors[8], color_scale, color_product);
                *(s32*)&poly->r2 = *(s32*)&colors[8];
                setlen(poly, 6);
                setcode(poly, 0x30);
                if (blend_mode >= 0)
                {
                    setcode(poly, 0x32);
                }
                ADD_PRIM_TEST(&D_801398EC->ot[ot_index + triangle_area], poly);
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(WmapPolyG3);
                    D_801398EC->prim_cursor += sizeof(WmapPolyG3);
                }
                break;
            }
            case 0x28:
            {
                POLY_F4* poly = (POLY_F4*)D_801398EC->prim_cursor;
                LOAD_VERTEX(transformed[0], vertices, *(s16*)(next_face -8));
                LOAD_VERTEX(transformed[1], vertices, *(s16*)(next_face -6));
                LOAD_VERTEX(transformed[2], vertices, *(s16*)(next_face -4));
                gte_ldv3(&transformed[0], &transformed[1], &transformed[2]);
                gte_rtpt();
                LOAD_VERTEX(transformed[3], vertices, *(s16*)(next_face -2));
                gte_stsxy3(&packed0, &packed1, &packed2);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (backface_mode == 0)
                {
                    if (triangle_area < 0)
                    {
                        break;
                    }
                    triangle_area = 0;
                }
                else
                {
                    if (triangle_area < 0)
                    {
                        triangle_area = 4;
                    }
                    else
                    {
                        triangle_area = 0;
                    }
                }
                gte_ldv0(&transformed[3]);
                gte_rtps();
                *(s32*)&poly->x0 = packed0;
                *(s32*)&poly->x1 = packed1;
                gte_stsxy(&packed3);
                *(s32*)&poly->x2 = packed2;
                *(s32*)&poly->x3 = packed3;
                colors[0] = *(CVECTOR*)(next_attributes - 16);
                SCALE_COLOR(colors[0], color_scale, color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                setlen(poly, 5);
                setcode(poly, 0x28);
                if (blend_mode >= 0)
                {
                    setcode(poly, 0x2A);
                }
                ADD_PRIM_TEST(&D_801398EC->ot[ot_index + triangle_area], poly);
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_F4);
                    D_801398EC->prim_cursor += sizeof(POLY_F4);
                }
                break;
            }
            case 0x20:
            {
                WmapPolyF3* poly = (WmapPolyF3*)D_801398EC->prim_cursor;
                LOAD_VERTEX(transformed[0], vertices, *(s16*)(next_face -8));
                LOAD_VERTEX(transformed[1], vertices, *(s16*)(next_face -6));
                LOAD_VERTEX(transformed[2], vertices, *(s16*)(next_face -4));
                gte_ldv3(&transformed[0], &transformed[1], &transformed[2]);
                gte_rtpt();
                colors[0] = *(CVECTOR*)(next_attributes - 16);
                SCALE_COLOR(colors[0], color_scale, color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                gte_stsxy3(&packed0, &packed1, &packed2);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (backface_mode == 0)
                {
                    if (triangle_area < 0)
                    {
                        break;
                    }
                    triangle_area = 0;
                }
                else
                {
                    if (triangle_area < 0)
                    {
                        triangle_area = 4;
                    }
                    else
                    {
                        triangle_area = 0;
                    }
                }
                OFFSET_SXY(packed0, x_offset, y_offset);
                OFFSET_SXY(packed1, x_offset, y_offset);
                OFFSET_SXY(packed2, x_offset, y_offset);
                *(s32*)&poly->x0 = packed0;
                *(s32*)&poly->x1 = packed1;
                setlen(poly, 4);
                setcode(poly, 0x20);
                *(s32*)&poly->x2 = packed2;
                if (blend_mode >= 0)
                {
                    setcode(poly, 0x22);
                }
                ADD_PRIM_TEST(&D_801398EC->ot[ot_index + triangle_area], poly);
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(WmapPolyF3);
                    D_801398EC->prim_cursor += sizeof(WmapPolyF3);
                }
                break;
            }
            case 0x34:
            {
                WmapPolyGT3* poly;
                if (z_divisor == -1)
                {
                    LOAD_VERTEX(transformed[0], vertices, *(s16*)(next_face -8));
                    LOAD_VERTEX(transformed[1], vertices, *(s16*)(next_face -6));
                    LOAD_VERTEX(transformed[2], vertices, *(s16*)(next_face -4));
                }
                else
                {
                    LOAD_VERTEX_SCALED(transformed[0], vertices, *(s16*)(next_face -8), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[1], vertices, *(s16*)(next_face -6), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[2], vertices, *(s16*)(next_face -4), z_divisor);
                }
                gte_ldv3(&transformed[0], &transformed[1], &transformed[2]);
                gte_rtpt();
                poly = (WmapPolyGT3*)D_801398EC->prim_cursor;
                poly->tpage = tpage;
                poly->clut = clut;
                *(u16*)&poly->u0 = *(u16*)(attributes + 0);
                *(u16*)&poly->u1 = *(u16*)(next_attributes - 16);
                *(u16*)&poly->u2 = *(u16*)(next_attributes - 14);
                gte_stsxy3(&packed0, &packed1, &packed2);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (backface_mode == 0)
                {
                    if (triangle_area < 0)
                    {
                        break;
                    }
                    triangle_area = 0;
                }
                else
                {
                    if (triangle_area < 0)
                    {
                        triangle_area = 4;
                    }
                    else
                    {
                        triangle_area = 0;
                    }
                }
                OFFSET_SXY(packed0, x_offset, y_offset);
                OFFSET_SXY(packed1, x_offset, y_offset);
                OFFSET_SXY(packed2, x_offset, y_offset);
                *(s32*)&poly->x0 = packed0;
                *(s32*)&poly->x1 = packed1;
                *(s32*)&poly->x2 = packed2;
                colors[0] = *(CVECTOR*)(next_attributes - 16);
                colors[2] = colors[0];
                SCALE_COLOR(colors[2], effective_scale, color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[2];
                colors[0] = *(CVECTOR*)(next_attributes - 12);
                colors[6] = colors[0];
                SCALE_COLOR(colors[6], effective_scale, color_product);
                *(s32*)&poly->r1 = *(s32*)&colors[6];
                colors[0] = *(CVECTOR*)(next_attributes - 8);
                colors[10] = colors[0];
                SCALE_COLOR(colors[10], effective_scale, color_product);
                *(s32*)&poly->r2 = *(s32*)&colors[10];
                setlen(poly, 9);
                setcode(poly, 0x34);
                if (!(color_scale & 0x10000))
                {
                    setcode(poly, *(u8*)(next_face - 9) ? 0x36 : 0x34);
                }
                ADD_PRIM_TEST(&D_801398EC->ot[ot_index + triangle_area], poly);
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(WmapPolyGT3);
                    D_801398EC->prim_cursor += sizeof(WmapPolyGT3);
                }
                break;
            }
            case 0x3C:
            {
                POLY_GT4* poly = (POLY_GT4*)D_801398EC->prim_cursor;
                if (z_divisor == -1)
                {
                    LOAD_VERTEX(transformed[0], vertices, *(s16*)(next_face -8));
                    LOAD_VERTEX(transformed[1], vertices, *(s16*)(next_face -6));
                    LOAD_VERTEX(transformed[2], vertices, *(s16*)(next_face -4));
                    LOAD_VERTEX(transformed[3], vertices, *(s16*)(next_face -2));
                }
                else
                {
                    LOAD_VERTEX_SCALED(transformed[0], vertices, *(s16*)(next_face -8), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[1], vertices, *(s16*)(next_face -6), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[2], vertices, *(s16*)(next_face -4), z_divisor);
                    LOAD_VERTEX_SCALED(transformed[3], vertices, *(s16*)(next_face -2), z_divisor);
                }
                gte_ldv3(&transformed[0], &transformed[1], &transformed[2]);
                gte_rtpt();
                poly->tpage = tpage;
                poly->clut = clut;
                *(u16*)&poly->u0 = *(u16*)(attributes + 0);
                *(u16*)&poly->u1 = *(u16*)(next_attributes - 16);
                *(u16*)&poly->u2 = *(u16*)(next_attributes - 14);
                gte_stsxy3(&packed0, &packed1, &packed2);
                gte_nclip();
                *(u16*)&poly->u3 = *(u16*)(next_attributes - 12);
                gte_stopz(&triangle_area);
                if (backface_mode == 0)
                {
                    if (triangle_area < 0)
                    {
                        break;
                    }
                    triangle_area = 0;
                }
                else
                {
                    if (triangle_area < 0)
                    {
                        triangle_area = 4;
                    }
                    else
                    {
                        triangle_area = 0;
                    }
                }
                gte_ldv0(&transformed[3]);
                gte_rtps();
                OFFSET_SXY(packed0, x_offset, y_offset);
                OFFSET_SXY(packed1, x_offset, y_offset);
                OFFSET_SXY(packed2, x_offset, y_offset);
                gte_stsxy(&packed3);
                OFFSET_SXY(packed3, x_offset, y_offset);
                *(s32*)&poly->x0 = packed0;
                *(s32*)&poly->x1 = packed1;
                *(s32*)&poly->x2 = packed2;
                *(s32*)&poly->x3 = packed3;
                colors[0] = *(CVECTOR*)(next_attributes - 16);
                colors[2] = colors[0];
                SCALE_COLOR(colors[2], effective_scale, color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[2];
                colors[0] = *(CVECTOR*)(next_attributes - 12);
                colors[6] = colors[0];
                SCALE_COLOR(colors[6], effective_scale, color_product);
                *(s32*)&poly->r1 = *(s32*)&colors[6];
                colors[0] = *(CVECTOR*)(next_attributes - 8);
                colors[10] = colors[0];
                SCALE_COLOR(colors[10], effective_scale, color_product);
                *(s32*)&poly->r2 = *(s32*)&colors[10];
                colors[0] = *(CVECTOR*)(next_attributes - 4);
                colors[16] = colors[0];
                SCALE_COLOR(colors[16], effective_scale, color_product);
                *(s32*)&poly->r3 = *(s32*)&colors[16];
                setlen(poly, 12);
                setcode(poly, 0x3C);
                if (!(color_scale & 0x10000))
                {
                    setcode(poly, *(u8*)(next_face - 9) ? 0x3E : 0x3C);
                }
                ADD_PRIM_TEST(&D_801398EC->ot[ot_index + triangle_area], poly);
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_GT4);
                    D_801398EC->prim_cursor += sizeof(POLY_GT4);
                }
                break;
            }
            default:
                func_80064F14(attributes);
                break;
            }

            face_data = next_face;
            next_face += 10;
            attributes = next_attributes;
            next_attributes += 24;
            i++;
        } while (i < face_count);
    }

    if (blend_mode >= 0)
    {
        WmapPolyFT3* poly;
        s16 final_tpage;

        poly = (WmapPolyFT3*)D_801398EC->prim_cursor;
        setlen(poly, 7);
        setcode(poly, 0x24);
        *(s32*)&poly->x2 = 0x190;
        *(s32*)&poly->x1 = 0x190;
        *(s32*)&poly->x0 = 0x190;
        final_tpage = ((blend_mode & 3) << 5) | 0xC;
        poly->tpage = final_tpage;
        addPrim(&D_801398EC->ot[ot_index], poly);
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(WmapPolyFT3);
            D_801398EC->prim_cursor += sizeof(WmapPolyFT3);
        }

        poly = (WmapPolyFT3*)D_801398EC->prim_cursor;
        setlen(poly, 7);
        setcode(poly, 0x24);
        *(s32*)&poly->x2 = 0x190;
        *(s32*)&poly->x1 = 0x190;
        *(s32*)&poly->x0 = 0x190;
        poly->tpage = final_tpage;
        addPrim(&D_801398EC->ot[ot_index + 4], poly);
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(WmapPolyFT3);
            D_801398EC->prim_cursor += sizeof(WmapPolyFT3);
        }
    }
}
