#include "wmap_model_render.h"
#include "wmap_frame_render.h"
#include "wmap_resource_support.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define WMAP_MODEL_COLOR_SHIFT 7
#define WMAP_MODEL_BLEND_VALUE_MASK 0x0FFF
#define WMAP_MODEL_BLEND_MASK 0x03
#define WMAP_MODEL_BLEND_SHIFT 5
#define WMAP_MODEL_BLEND_TPAGE 0x0C
#define WMAP_MODEL_SEMI_TRANS 0x02
#define WMAP_MODEL_BACK_OT_OFFSET 4
#define WMAP_MODEL_TPAGE_XY 400 /* Packed (400, 0), outside the visible display. */

#define LOAD_MODEL_VERTEX(dst, source, index)                                                                                                                  \
    {                                                                                                                                                          \
        (dst).x = (source)[(index)].x;                                                                                                                         \
        (dst).y = (source)[(index)].y;                                                                                                                         \
        (dst).z = (source)[(index)].z;                                                                                                                         \
    }

#define LOAD_MODEL_VERTEX_SCALED(dst, source, index, divisor)                                                                                                  \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        (dst).x = (source)[(index)].x;                                                                                                                         \
        (dst).y = (source)[(index)].y;                                                                                                                         \
        (dst).z = (source)[(index)].z / (divisor);                                                                                                             \
    } while (0)

#define OFFSET_SCREEN_XY(value, dx, dy)                                                                                                                        \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        (value).point.x += (dx);                                                                                                                               \
        (value).point.y += (dy);                                                                                                                               \
    } while (0)

#define SCALE_COLOR(color, scale, product)                                                                                                                     \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        if ((scale) != -1)                                                                                                                                     \
        {                                                                                                                                                      \
            (product) = (color).r * (scale);                                                                                                                   \
            (color).r = (product) >> WMAP_MODEL_COLOR_SHIFT;                                                                                                   \
            (color).g = ((s32)(color).g * (scale)) >> WMAP_MODEL_COLOR_SHIFT;                                                                                  \
            (color).b = ((s32)(color).b * (scale)) >> WMAP_MODEL_COLOR_SHIFT;                                                                                  \
        }                                                                                                                                                      \
    } while (0)

/** @brief GPU polygon commands stored in the model's face records. */
typedef enum
{
    WMAP_MODEL_F3 = 0x20,
    WMAP_MODEL_FT3 = 0x24,
    WMAP_MODEL_F4 = 0x28,
    WMAP_MODEL_FT4 = 0x2C,
    WMAP_MODEL_G3 = 0x30,
    WMAP_MODEL_GT3 = 0x34,
    WMAP_MODEL_G4 = 0x38,
    WMAP_MODEL_GT4 = 0x3C
} WmapModelPrimitive;

/** @brief Packed projected coordinates returned by the GTE. */
typedef union
{
    s32 packed;
    struct
    {
        s16 x;
        s16 y;
    } point;
} WmapScreenPoint;

/** @brief Resource header followed by offsets relative to the resource base. */
typedef struct
{
    s32 unknown_0;
    s32 model_offsets[1];
} WmapModelDirectory;

/** @brief Packed world-map model vertex. */
typedef struct
{
    s16 x;
    s16 y;
    s16 z;
    u16 pad;
} WmapMeshVertex;

/** @brief World-map face flags and vertex indices. */
typedef struct
{
    u8 primitive_code;
    u8 semi_transparent;
    s16 vertex_indices[4];
} WmapMeshFace;

/** @brief Model header followed by vertices, faces, and face attributes. */
typedef struct
{
    s16 vertex_count;
    s16 face_count;
    WmapMeshVertex vertices[1];
} WmapModel;

/** @brief Packed UV pairs and vertex colors for one model face. */
typedef struct
{
    u16 uv[4];
    CVECTOR colors[4];
} WmapFaceAttributes;

/** @brief Flat-shaded world-map triangle packet. */
typedef struct
{
    u_long tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
} WmapPolyF3;

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

/**
 * @brief Project a model's faces into the current world-map ordering table.
 * @param resource_table Packed resource header and relative model offsets.
 * @param resource_index Zero-based model index in the resource.
 * @param ot_index Front-face ordering-table bucket; backfaces use four buckets higher.
 * @param tpage Texture page for textured faces.
 * @param clut Palette for textured faces.
 * @param blend_mode Blend setting with WMAP_MODEL_DRAW_BACKFACES; low two bits select the GPU blend equation.
 * @param color_scale RGB multiplier in units of 1/128; textured faces use the low 16 bits and WMAP_MODEL_FORCE_OPAQUE.
 * @param x_offset Screen X displacement, ignored by flat untextured quads.
 * @param y_offset Screen Y displacement, ignored by flat untextured quads.
 * @param z_divisor Vertex Z divisor, or -1 to keep Z; zero acts as one. Untextured quads ignore it.
 * @note Uses the caller's GTE transform. Untextured colors use -1 to bypass scaling.
 * @note The backface flag is stripped before the signed blend-mode test, including for negative inputs.
 */
void wmap_draw_model(void* resource_table, s32 resource_index, s32 ot_index, s32 tpage, s32 clut, s32 blend_mode, s32 color_scale, s32 x_offset, s32 y_offset,
                     s32 z_divisor)
{
    WmapModel* model;
    WmapModelDirectory* directory;
    WmapMeshVertex* vertices;
    WmapMeshFace* face;
    WmapMeshFace* next_face;
    WmapFaceAttributes* attributes;
    WmapFaceAttributes* next_attributes;
    s32 face_count;
    s32 face_index;
    s32 draw_backfaces;
    WmapScreenPoint screen_xy0;
    WmapScreenPoint screen_xy1;
    WmapScreenPoint screen_xy2;
    WmapScreenPoint screen_xy3;
    s32 face_order;
    s32 color_product;
    WmapMeshVertex projection_vertices[4];
    /* TODO: Recover the original color work-buffer layout. */
    CVECTOR colors[20];

    if (blend_mode & WMAP_MODEL_DRAW_BACKFACES)
    {
        blend_mode &= WMAP_MODEL_BLEND_VALUE_MASK;
        draw_backfaces = 1;
    }
    else
    {
        draw_backfaces = 0;
    }

    directory = resource_table;
    model = (WmapModel*)((u8*)resource_table + directory->model_offsets[resource_index]);
    vertices = model->vertices;
    face_count = model->face_count;
    face = (WmapMeshFace*)(vertices + model->vertex_count);
    attributes = (WmapFaceAttributes*)(face + face_count);

    if (face_count < 0)
    {
        func_80064F14(attributes);
        return;
    }

    if (z_divisor == 0)
    {
        z_divisor = 1;
    }

    face_index = 0;
    if (face_count > 0)
    {
        next_face = face + 1;
        next_attributes = attributes + 1;
        do
        {
            switch (face->primitive_code)
            {
            case WMAP_MODEL_FT4:
            {
                POLY_FT4* poly = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
                s32 primitive_code;
                if (z_divisor == -1)
                {
                    LOAD_MODEL_VERTEX(projection_vertices[0], vertices, next_face[-1].vertex_indices[0]);
                    LOAD_MODEL_VERTEX(projection_vertices[1], vertices, next_face[-1].vertex_indices[1]);
                    LOAD_MODEL_VERTEX(projection_vertices[2], vertices, next_face[-1].vertex_indices[2]);
                    LOAD_MODEL_VERTEX(projection_vertices[3], vertices, next_face[-1].vertex_indices[3]);
                }
                else
                {
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[0], vertices, next_face[-1].vertex_indices[0], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[1], vertices, next_face[-1].vertex_indices[1], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[2], vertices, next_face[-1].vertex_indices[2], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[3], vertices, next_face[-1].vertex_indices[3], z_divisor);
                }
                gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
                gte_rtpt();
                *(u16*)&poly->u0 = next_attributes[-1].uv[0];
                *(u16*)&poly->u1 = next_attributes[-1].uv[1];
                *(u16*)&poly->u2 = next_attributes[-1].uv[2];
                *(u16*)&poly->u3 = next_attributes[-1].uv[3];
                gte_stsxy0(&screen_xy0);
                gte_stsxy1(&screen_xy1);
                gte_stsxy2(&screen_xy2);
                gte_nclip();
                OFFSET_SCREEN_XY(screen_xy0, x_offset, y_offset);
                /* NCLIP gives signed area; reuse it as the backface bucket offset. */
                gte_stopz(&face_order);
                if (draw_backfaces == 0)
                {
                    if (face_order < 0)
                    {
                        break;
                    }
                    face_order = 0;
                }
                else
                {
                    if (face_order < 0)
                    {
                        face_order = WMAP_MODEL_BACK_OT_OFFSET;
                    }
                    else
                    {
                        face_order = 0;
                    }
                }
                OFFSET_SCREEN_XY(screen_xy1, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy2, x_offset, y_offset);
                gte_ldv0(&projection_vertices[3]);
                gte_rtps();
                poly->tpage = tpage;
                poly->clut = clut;
                gte_stsxy(&screen_xy3);
                OFFSET_SCREEN_XY(screen_xy3, x_offset, y_offset);
                *(s32*)&poly->x0 = screen_xy0.packed;
                *(s32*)&poly->x1 = screen_xy1.packed;
                *(s32*)&poly->x2 = screen_xy2.packed;
                *(s32*)&poly->x3 = screen_xy3.packed;
                colors[0] = next_attributes[-1].colors[0];
                SCALE_COLOR(colors[0], (color_scale & WMAP_MODEL_COLOR_MASK), color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                setlen(poly, 9);
                setcode(poly, WMAP_MODEL_FT4);
                if (!(color_scale & WMAP_MODEL_FORCE_OPAQUE))
                {
                    primitive_code = (WMAP_MODEL_FT4 | WMAP_MODEL_SEMI_TRANS);
                    if (next_face[-1].semi_transparent == 0)
                    {
                        primitive_code = WMAP_MODEL_FT4;
                    }
                    setcode(poly, primitive_code);
                }
                addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], poly);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_FT4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
                }
                break;
            }
            case WMAP_MODEL_FT3:
            {
                POLY_FT3* poly = (POLY_FT3*)g_wmap_current_frame->packet_cursor;
                if (z_divisor == -1)
                {
                    LOAD_MODEL_VERTEX(projection_vertices[0], vertices, next_face[-1].vertex_indices[0]);
                    LOAD_MODEL_VERTEX(projection_vertices[1], vertices, next_face[-1].vertex_indices[1]);
                    LOAD_MODEL_VERTEX(projection_vertices[2], vertices, next_face[-1].vertex_indices[2]);
                }
                else
                {
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[0], vertices, next_face[-1].vertex_indices[0], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[1], vertices, next_face[-1].vertex_indices[1], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[2], vertices, next_face[-1].vertex_indices[2], z_divisor);
                }
                gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
                gte_rtpt();
                poly->tpage = tpage;
                poly->clut = clut;
                *(u16*)&poly->u0 = attributes->uv[0];
                *(u16*)&poly->u1 = next_attributes[-1].uv[1];
                *(u16*)&poly->u2 = next_attributes[-1].uv[2];
                gte_stsxy0(&screen_xy0);
                gte_stsxy1(&screen_xy1);
                gte_stsxy2(&screen_xy2);
                gte_nclip();
                OFFSET_SCREEN_XY(screen_xy0, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy1, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy2, x_offset, y_offset);
                gte_stopz(&face_order);
                if (draw_backfaces == 0)
                {
                    if (face_order < 0)
                    {
                        break;
                    }
                    face_order = 0;
                }
                else
                {
                    if (face_order < 0)
                    {
                        face_order = WMAP_MODEL_BACK_OT_OFFSET;
                    }
                    else
                    {
                        face_order = 0;
                    }
                }
                *(s32*)&poly->x0 = screen_xy0.packed;
                *(s32*)&poly->x1 = screen_xy1.packed;
                *(s32*)&poly->x2 = screen_xy2.packed;
                colors[0] = next_attributes[-1].colors[0];
                SCALE_COLOR(colors[0], (color_scale & WMAP_MODEL_COLOR_MASK), color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                setlen(poly, 7);
                setcode(poly, WMAP_MODEL_FT3);
                if (!(color_scale & WMAP_MODEL_FORCE_OPAQUE))
                {
                    s32 primitive_code = WMAP_MODEL_FT3 | WMAP_MODEL_SEMI_TRANS;
                    if (next_face[-1].semi_transparent == 0)
                    {
                        primitive_code = WMAP_MODEL_FT3;
                    }
                    setcode(poly, primitive_code);
                }
                addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], poly);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_FT3);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_FT3);
                }
                break;
            }
            case WMAP_MODEL_G4:
            {
                POLY_G4* poly = (POLY_G4*)g_wmap_current_frame->packet_cursor;
                LOAD_MODEL_VERTEX(projection_vertices[0], vertices, next_face[-1].vertex_indices[0]);
                LOAD_MODEL_VERTEX(projection_vertices[1], vertices, next_face[-1].vertex_indices[1]);
                LOAD_MODEL_VERTEX(projection_vertices[2], vertices, next_face[-1].vertex_indices[2]);
                gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
                gte_rtpt();
                LOAD_MODEL_VERTEX(projection_vertices[3], vertices, next_face[-1].vertex_indices[3]);
                colors[0] = next_attributes[-1].colors[0];
                SCALE_COLOR(colors[0], color_scale, color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                colors[4] = next_attributes[-1].colors[1];
                SCALE_COLOR(colors[4], color_scale, color_product);
                *(s32*)&poly->r1 = *(s32*)&colors[4];
                gte_stsxy0(&screen_xy0);
                gte_stsxy1(&screen_xy1);
                gte_stsxy2(&screen_xy2);
                gte_nclip();
                colors[8] = next_attributes[-1].colors[2];
                SCALE_COLOR(colors[8], color_scale, color_product);
                *(s32*)&poly->r2 = *(s32*)&colors[8];
                colors[12] = next_attributes[-1].colors[3];
                SCALE_COLOR(colors[12], color_scale, color_product);
                *(s32*)&poly->r3 = *(s32*)&colors[12];
                gte_stopz(&face_order);
                if (draw_backfaces == 0)
                {
                    if (face_order < 0)
                    {
                        break;
                    }
                    face_order = 0;
                }
                else
                {
                    if (face_order < 0)
                    {
                        face_order = WMAP_MODEL_BACK_OT_OFFSET;
                    }
                    else
                    {
                        face_order = 0;
                    }
                }
                OFFSET_SCREEN_XY(screen_xy0, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy1, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy2, x_offset, y_offset);
                gte_ldv0(&projection_vertices[3]);
                gte_rtps();
                *(s32*)&poly->x0 = screen_xy0.packed;
                *(s32*)&poly->x1 = screen_xy1.packed;
                gte_stsxy(&screen_xy3);
                OFFSET_SCREEN_XY(screen_xy3, x_offset, y_offset);
                *(s32*)&poly->x2 = screen_xy2.packed;
                {
                    s32 fourth_screen_xy = screen_xy3.packed;
                    setlen(poly, 8);
                    setcode(poly, WMAP_MODEL_G4);
                    *(s32*)&poly->x3 = fourth_screen_xy;
                }
                if (blend_mode >= 0)
                {
                    setcode(poly, (WMAP_MODEL_G4 | WMAP_MODEL_SEMI_TRANS));
                }
                addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], poly);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_G4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_G4);
                }
                break;
            }
            case WMAP_MODEL_G3:
            {
                WmapPolyG3* poly = (WmapPolyG3*)g_wmap_current_frame->packet_cursor;
                if (z_divisor == -1)
                {
                    LOAD_MODEL_VERTEX(projection_vertices[0], vertices, next_face[-1].vertex_indices[0]);
                    LOAD_MODEL_VERTEX(projection_vertices[1], vertices, next_face[-1].vertex_indices[1]);
                    LOAD_MODEL_VERTEX(projection_vertices[2], vertices, next_face[-1].vertex_indices[2]);
                }
                else
                {
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[0], vertices, next_face[-1].vertex_indices[0], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[1], vertices, next_face[-1].vertex_indices[1], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[2], vertices, next_face[-1].vertex_indices[2], z_divisor);
                }
                gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
                gte_rtpt();
                colors[0] = next_attributes[-1].colors[0];
                SCALE_COLOR(colors[0], color_scale, color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                gte_stsxy0(&screen_xy0);
                gte_stsxy1(&screen_xy1);
                gte_stsxy2(&screen_xy2);
                gte_nclip();
                gte_stopz(&face_order);
                if (draw_backfaces == 0)
                {
                    if (face_order < 0)
                    {
                        break;
                    }
                    face_order = 0;
                }
                else
                {
                    if (face_order < 0)
                    {
                        face_order = WMAP_MODEL_BACK_OT_OFFSET;
                    }
                    else
                    {
                        face_order = 0;
                    }
                }
                OFFSET_SCREEN_XY(screen_xy0, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy1, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy2, x_offset, y_offset);
                *(s32*)&poly->x0 = screen_xy0.packed;
                *(s32*)&poly->x1 = screen_xy1.packed;
                *(s32*)&poly->x2 = screen_xy2.packed;
                colors[4] = next_attributes[-1].colors[1];
                SCALE_COLOR(colors[4], color_scale, color_product);
                *(s32*)&poly->r1 = *(s32*)&colors[4];
                colors[8] = next_attributes[-1].colors[2];
                SCALE_COLOR(colors[8], color_scale, color_product);
                *(s32*)&poly->r2 = *(s32*)&colors[8];
                setlen(poly, 6);
                setcode(poly, WMAP_MODEL_G3);
                if (blend_mode >= 0)
                {
                    setcode(poly, (WMAP_MODEL_G3 | WMAP_MODEL_SEMI_TRANS));
                }
                addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], poly);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(WmapPolyG3);
                    g_wmap_current_frame->packet_cursor += sizeof(WmapPolyG3);
                }
                break;
            }
            case WMAP_MODEL_F4:
            {
                POLY_F4* poly = (POLY_F4*)g_wmap_current_frame->packet_cursor;
                LOAD_MODEL_VERTEX(projection_vertices[0], vertices, next_face[-1].vertex_indices[0]);
                LOAD_MODEL_VERTEX(projection_vertices[1], vertices, next_face[-1].vertex_indices[1]);
                LOAD_MODEL_VERTEX(projection_vertices[2], vertices, next_face[-1].vertex_indices[2]);
                gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
                gte_rtpt();
                LOAD_MODEL_VERTEX(projection_vertices[3], vertices, next_face[-1].vertex_indices[3]);
                gte_stsxy0(&screen_xy0);
                gte_stsxy1(&screen_xy1);
                gte_stsxy2(&screen_xy2);
                gte_nclip();
                gte_stopz(&face_order);
                if (draw_backfaces == 0)
                {
                    if (face_order < 0)
                    {
                        break;
                    }
                    face_order = 0;
                }
                else
                {
                    if (face_order < 0)
                    {
                        face_order = WMAP_MODEL_BACK_OT_OFFSET;
                    }
                    else
                    {
                        face_order = 0;
                    }
                }
                gte_ldv0(&projection_vertices[3]);
                gte_rtps();
                *(s32*)&poly->x0 = screen_xy0.packed;
                *(s32*)&poly->x1 = screen_xy1.packed;
                gte_stsxy(&screen_xy3);
                *(s32*)&poly->x2 = screen_xy2.packed;
                *(s32*)&poly->x3 = screen_xy3.packed;
                colors[0] = next_attributes[-1].colors[0];
                SCALE_COLOR(colors[0], color_scale, color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                setlen(poly, 5);
                setcode(poly, WMAP_MODEL_F4);
                if (blend_mode >= 0)
                {
                    setcode(poly, (WMAP_MODEL_F4 | WMAP_MODEL_SEMI_TRANS));
                }
                addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], poly);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_F4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_F4);
                }
                break;
            }
            case WMAP_MODEL_F3:
            {
                WmapPolyF3* poly = (WmapPolyF3*)g_wmap_current_frame->packet_cursor;
                LOAD_MODEL_VERTEX(projection_vertices[0], vertices, next_face[-1].vertex_indices[0]);
                LOAD_MODEL_VERTEX(projection_vertices[1], vertices, next_face[-1].vertex_indices[1]);
                LOAD_MODEL_VERTEX(projection_vertices[2], vertices, next_face[-1].vertex_indices[2]);
                gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
                gte_rtpt();
                colors[0] = next_attributes[-1].colors[0];
                SCALE_COLOR(colors[0], color_scale, color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[0];
                gte_stsxy0(&screen_xy0);
                gte_stsxy1(&screen_xy1);
                gte_stsxy2(&screen_xy2);
                gte_nclip();
                gte_stopz(&face_order);
                if (draw_backfaces == 0)
                {
                    if (face_order < 0)
                    {
                        break;
                    }
                    face_order = 0;
                }
                else
                {
                    if (face_order < 0)
                    {
                        face_order = WMAP_MODEL_BACK_OT_OFFSET;
                    }
                    else
                    {
                        face_order = 0;
                    }
                }
                OFFSET_SCREEN_XY(screen_xy0, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy1, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy2, x_offset, y_offset);
                *(s32*)&poly->x0 = screen_xy0.packed;
                *(s32*)&poly->x1 = screen_xy1.packed;
                setlen(poly, 4);
                setcode(poly, WMAP_MODEL_F3);
                *(s32*)&poly->x2 = screen_xy2.packed;
                if (blend_mode >= 0)
                {
                    setcode(poly, (WMAP_MODEL_F3 | WMAP_MODEL_SEMI_TRANS));
                }
                addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], poly);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(WmapPolyF3);
                    g_wmap_current_frame->packet_cursor += sizeof(WmapPolyF3);
                }
                break;
            }
            case WMAP_MODEL_GT3:
            {
                WmapPolyGT3* poly;
                if (z_divisor == -1)
                {
                    LOAD_MODEL_VERTEX(projection_vertices[0], vertices, next_face[-1].vertex_indices[0]);
                    LOAD_MODEL_VERTEX(projection_vertices[1], vertices, next_face[-1].vertex_indices[1]);
                    LOAD_MODEL_VERTEX(projection_vertices[2], vertices, next_face[-1].vertex_indices[2]);
                }
                else
                {
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[0], vertices, next_face[-1].vertex_indices[0], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[1], vertices, next_face[-1].vertex_indices[1], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[2], vertices, next_face[-1].vertex_indices[2], z_divisor);
                }
                gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
                gte_rtpt();
                poly = (WmapPolyGT3*)g_wmap_current_frame->packet_cursor;
                poly->tpage = tpage;
                poly->clut = clut;
                *(u16*)&poly->u0 = attributes->uv[0];
                *(u16*)&poly->u1 = next_attributes[-1].uv[1];
                *(u16*)&poly->u2 = next_attributes[-1].uv[2];
                gte_stsxy0(&screen_xy0);
                gte_stsxy1(&screen_xy1);
                gte_stsxy2(&screen_xy2);
                gte_nclip();
                gte_stopz(&face_order);
                if (draw_backfaces == 0)
                {
                    if (face_order < 0)
                    {
                        break;
                    }
                    face_order = 0;
                }
                else
                {
                    if (face_order < 0)
                    {
                        face_order = WMAP_MODEL_BACK_OT_OFFSET;
                    }
                    else
                    {
                        face_order = 0;
                    }
                }
                OFFSET_SCREEN_XY(screen_xy0, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy1, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy2, x_offset, y_offset);
                *(s32*)&poly->x0 = screen_xy0.packed;
                *(s32*)&poly->x1 = screen_xy1.packed;
                *(s32*)&poly->x2 = screen_xy2.packed;
                colors[0] = next_attributes[-1].colors[0];
                colors[2] = colors[0];
                SCALE_COLOR(colors[2], (color_scale & WMAP_MODEL_COLOR_MASK), color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[2];
                colors[0] = next_attributes[-1].colors[1];
                colors[6] = colors[0];
                SCALE_COLOR(colors[6], (color_scale & WMAP_MODEL_COLOR_MASK), color_product);
                *(s32*)&poly->r1 = *(s32*)&colors[6];
                colors[0] = next_attributes[-1].colors[2];
                colors[10] = colors[0];
                SCALE_COLOR(colors[10], (color_scale & WMAP_MODEL_COLOR_MASK), color_product);
                *(s32*)&poly->r2 = *(s32*)&colors[10];
                setlen(poly, 9);
                setcode(poly, WMAP_MODEL_GT3);
                if (!(color_scale & WMAP_MODEL_FORCE_OPAQUE))
                {
                    s32 primitive_code = WMAP_MODEL_GT3 | WMAP_MODEL_SEMI_TRANS;
                    if (next_face[-1].semi_transparent == 0)
                    {
                        primitive_code = WMAP_MODEL_GT3;
                    }
                    setcode(poly, primitive_code);
                }
                addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], poly);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(WmapPolyGT3);
                    g_wmap_current_frame->packet_cursor += sizeof(WmapPolyGT3);
                }
                break;
            }
            case WMAP_MODEL_GT4:
            {
                POLY_GT4* poly = (POLY_GT4*)g_wmap_current_frame->packet_cursor;
                if (z_divisor == -1)
                {
                    LOAD_MODEL_VERTEX(projection_vertices[0], vertices, next_face[-1].vertex_indices[0]);
                    LOAD_MODEL_VERTEX(projection_vertices[1], vertices, next_face[-1].vertex_indices[1]);
                    LOAD_MODEL_VERTEX(projection_vertices[2], vertices, next_face[-1].vertex_indices[2]);
                    LOAD_MODEL_VERTEX(projection_vertices[3], vertices, next_face[-1].vertex_indices[3]);
                }
                else
                {
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[0], vertices, next_face[-1].vertex_indices[0], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[1], vertices, next_face[-1].vertex_indices[1], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[2], vertices, next_face[-1].vertex_indices[2], z_divisor);
                    LOAD_MODEL_VERTEX_SCALED(projection_vertices[3], vertices, next_face[-1].vertex_indices[3], z_divisor);
                }
                gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
                gte_rtpt();
                poly->tpage = tpage;
                poly->clut = clut;
                *(u16*)&poly->u0 = attributes->uv[0];
                *(u16*)&poly->u1 = next_attributes[-1].uv[1];
                *(u16*)&poly->u2 = next_attributes[-1].uv[2];
                gte_stsxy0(&screen_xy0);
                gte_stsxy1(&screen_xy1);
                gte_stsxy2(&screen_xy2);
                gte_nclip();
                *(u16*)&poly->u3 = next_attributes[-1].uv[3];
                gte_stopz(&face_order);
                if (draw_backfaces == 0)
                {
                    if (face_order < 0)
                    {
                        break;
                    }
                    face_order = 0;
                }
                else
                {
                    if (face_order < 0)
                    {
                        face_order = WMAP_MODEL_BACK_OT_OFFSET;
                    }
                    else
                    {
                        face_order = 0;
                    }
                }
                gte_ldv0(&projection_vertices[3]);
                gte_rtps();
                OFFSET_SCREEN_XY(screen_xy0, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy1, x_offset, y_offset);
                OFFSET_SCREEN_XY(screen_xy2, x_offset, y_offset);
                gte_stsxy(&screen_xy3);
                OFFSET_SCREEN_XY(screen_xy3, x_offset, y_offset);
                *(s32*)&poly->x0 = screen_xy0.packed;
                *(s32*)&poly->x1 = screen_xy1.packed;
                *(s32*)&poly->x2 = screen_xy2.packed;
                *(s32*)&poly->x3 = screen_xy3.packed;
                colors[0] = next_attributes[-1].colors[0];
                colors[2] = colors[0];
                SCALE_COLOR(colors[2], (color_scale & WMAP_MODEL_COLOR_MASK), color_product);
                *(s32*)&poly->r0 = *(s32*)&colors[2];
                colors[0] = next_attributes[-1].colors[1];
                colors[6] = colors[0];
                SCALE_COLOR(colors[6], (color_scale & WMAP_MODEL_COLOR_MASK), color_product);
                *(s32*)&poly->r1 = *(s32*)&colors[6];
                colors[0] = next_attributes[-1].colors[2];
                colors[10] = colors[0];
                SCALE_COLOR(colors[10], (color_scale & WMAP_MODEL_COLOR_MASK), color_product);
                *(s32*)&poly->r2 = *(s32*)&colors[10];
                colors[0] = next_attributes[-1].colors[3];
                colors[16] = colors[0];
                SCALE_COLOR(colors[16], (color_scale & WMAP_MODEL_COLOR_MASK), color_product);
                *(s32*)&poly->r3 = *(s32*)&colors[16];
                setlen(poly, 12);
                setcode(poly, WMAP_MODEL_GT4);
                if (!(color_scale & WMAP_MODEL_FORCE_OPAQUE))
                {
                    s32 primitive_code = WMAP_MODEL_GT4 | WMAP_MODEL_SEMI_TRANS;
                    if (next_face[-1].semi_transparent == 0)
                    {
                        primitive_code = WMAP_MODEL_GT4;
                    }
                    setcode(poly, primitive_code);
                }
                addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], poly);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_GT4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_GT4);
                }
                break;
            }
            default:
                func_80064F14(attributes);
                break;
            }

            face = next_face;
            next_face++;
            attributes = next_attributes;
            next_attributes++;
            face_index++;
        } while (face_index < face_count);
    }

    /* Degenerate triangles set the blend equation before each face bucket is drawn. */
    if (blend_mode >= 0)
    {
        POLY_FT3* poly;
        s16 final_tpage;

        poly = (POLY_FT3*)g_wmap_current_frame->packet_cursor;
        setlen(poly, 7);
        setcode(poly, WMAP_MODEL_FT3);
        *(s32*)&poly->x2 = WMAP_MODEL_TPAGE_XY;
        *(s32*)&poly->x1 = WMAP_MODEL_TPAGE_XY;
        *(s32*)&poly->x0 = WMAP_MODEL_TPAGE_XY;
        final_tpage = ((blend_mode & WMAP_MODEL_BLEND_MASK) << WMAP_MODEL_BLEND_SHIFT) | WMAP_MODEL_BLEND_TPAGE;
        poly->tpage = final_tpage;
        addPrim(&g_wmap_current_frame->ordering_table[ot_index], poly);
        if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
        {
            g_wmap_packet_bytes += sizeof(POLY_FT3);
            g_wmap_current_frame->packet_cursor += sizeof(POLY_FT3);
        }

        poly = (POLY_FT3*)g_wmap_current_frame->packet_cursor;
        setlen(poly, 7);
        setcode(poly, WMAP_MODEL_FT3);
        *(s32*)&poly->x2 = WMAP_MODEL_TPAGE_XY;
        *(s32*)&poly->x1 = WMAP_MODEL_TPAGE_XY;
        *(s32*)&poly->x0 = WMAP_MODEL_TPAGE_XY;
        poly->tpage = final_tpage;
        addPrim(&g_wmap_current_frame->ordering_table[ot_index + WMAP_MODEL_BACK_OT_OFFSET], poly);
        if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
        {
            g_wmap_packet_bytes += sizeof(POLY_FT3);
            g_wmap_current_frame->packet_cursor += sizeof(POLY_FT3);
        }
    }
}
