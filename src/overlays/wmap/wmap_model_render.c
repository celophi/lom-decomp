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
    {                                                                                                                                                          \
        (dst).x = (source)[(index)].x;                                                                                                                         \
        (dst).y = (source)[(index)].y;                                                                                                                         \
        (dst).z = (source)[(index)].z / (divisor);                                                                                                             \
    }

#define OFFSET_SCREEN_XY(value, dx, dy)                                                                                                                        \
    {                                                                                                                                                          \
        (value).point.x += (dx);                                                                                                                               \
        (value).point.y += (dy);                                                                                                                               \
    }

/** @brief Write scaled RGB and the source high byte to a packet color word. */
#define SET_MODEL_COLOR(destination, color, scale)                                                                                                             \
    {                                                                                                                                                          \
        if ((scale) != -1)                                                                                                                                     \
        {                                                                                                                                                      \
            s32 red = (color).r * (scale);                                                                                                                     \
            s32 green = (color).g * (scale);                                                                                                                   \
            s32 blue = (color).b * (scale);                                                                                                                    \
            (color).r = red >> WMAP_MODEL_COLOR_SHIFT;                                                                                                         \
            (color).g = green >> WMAP_MODEL_COLOR_SHIFT;                                                                                                       \
            (color).b = blue >> WMAP_MODEL_COLOR_SHIFT;                                                                                                        \
            *(s32*)&(destination) = *(s32*)&(color);                                                                                                           \
        }                                                                                                                                                      \
        else                                                                                                                                                   \
        {                                                                                                                                                      \
            *(s32*)&(destination) = *(s32*)&(color);                                                                                                           \
        }                                                                                                                                                      \
    }

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
 * @brief Advance to the next fixed-size record in a model stream.
 * @param record Current face or attribute record.
 * @param size Size of one record in bytes.
 * @return Address of the next record.
 */
static inline void* wmap_advance_record(void* record, s32 size)
{
    return (u8*)record + size;
}

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
    WmapFaceAttributes* attributes;
    s32 face_count;
    s32 face_index;
    s32 draw_backfaces;
    WmapScreenPoint screen_xy0;
    WmapScreenPoint screen_xy1;
    WmapScreenPoint screen_xy2;
    WmapScreenPoint screen_xy3;
    s32 face_order;
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

    for (face_index = 0; face_index < face_count; face_index++)
    {
        switch (face->primitive_code)
        {
        case WMAP_MODEL_FT4:
        {
            POLY_FT4* ft4 = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
            s32 primitive_code;
            if (z_divisor == -1)
            {
                LOAD_MODEL_VERTEX(projection_vertices[0], vertices, face->vertex_indices[0]);
                LOAD_MODEL_VERTEX(projection_vertices[1], vertices, face->vertex_indices[1]);
                LOAD_MODEL_VERTEX(projection_vertices[2], vertices, face->vertex_indices[2]);
                LOAD_MODEL_VERTEX(projection_vertices[3], vertices, face->vertex_indices[3]);
            }
            else
            {
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[0], vertices, face->vertex_indices[0], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[1], vertices, face->vertex_indices[1], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[2], vertices, face->vertex_indices[2], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[3], vertices, face->vertex_indices[3], z_divisor);
            }
            gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
            gte_rtpt();
            *(u16*)&ft4->u0 = attributes->uv[0];
            *(u16*)&ft4->u1 = attributes->uv[1];
            *(u16*)&ft4->u2 = attributes->uv[2];
            *(u16*)&ft4->u3 = attributes->uv[3];
            gte_stsxy01(&screen_xy0, &screen_xy1);
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
            ft4->tpage = tpage;
            ft4->clut = clut;
            gte_stsxy(&screen_xy3);
            OFFSET_SCREEN_XY(screen_xy3, x_offset, y_offset);
            *(s32*)&ft4->x0 = screen_xy0.packed;
            *(s32*)&ft4->x1 = screen_xy1.packed;
            *(s32*)&ft4->x2 = screen_xy2.packed;
            *(s32*)&ft4->x3 = screen_xy3.packed;
            colors[0] = attributes->colors[0];
            SET_MODEL_COLOR(ft4->r0, colors[0], (color_scale & WMAP_MODEL_COLOR_MASK));
            setlen(ft4, 9);
            setcode(ft4, WMAP_MODEL_FT4);
            if (!(color_scale & WMAP_MODEL_FORCE_OPAQUE))
            {
                primitive_code = face->semi_transparent;
                if (primitive_code == 0)
                {
                    primitive_code = WMAP_MODEL_FT4;
                }
                else
                {
                    primitive_code = WMAP_MODEL_FT4 | WMAP_MODEL_SEMI_TRANS;
                }
                setcode(ft4, primitive_code);
            }
            addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], ft4);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(POLY_FT4);
                g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
            }
            break;
        }
        case WMAP_MODEL_FT3:
        {
            POLY_FT3* ft3 = (POLY_FT3*)g_wmap_current_frame->packet_cursor;
            if (z_divisor == -1)
            {
                LOAD_MODEL_VERTEX(projection_vertices[0], vertices, face->vertex_indices[0]);
                LOAD_MODEL_VERTEX(projection_vertices[1], vertices, face->vertex_indices[1]);
                LOAD_MODEL_VERTEX(projection_vertices[2], vertices, face->vertex_indices[2]);
            }
            else
            {
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[0], vertices, face->vertex_indices[0], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[1], vertices, face->vertex_indices[1], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[2], vertices, face->vertex_indices[2], z_divisor);
            }
            gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
            gte_rtpt();
            ft3->tpage = tpage;
            ft3->clut = clut;
            *(u16*)&ft3->u0 = attributes->uv[0];
            *(u16*)&ft3->u1 = attributes->uv[1];
            *(u16*)&ft3->u2 = attributes->uv[2];
            gte_stsxy01(&screen_xy0, &screen_xy1);
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
            *(s32*)&ft3->x0 = screen_xy0.packed;
            *(s32*)&ft3->x1 = screen_xy1.packed;
            *(s32*)&ft3->x2 = screen_xy2.packed;
            colors[0] = attributes->colors[0];
            SET_MODEL_COLOR(ft3->r0, colors[0], (color_scale & WMAP_MODEL_COLOR_MASK));
            setlen(ft3, 7);
            setcode(ft3, WMAP_MODEL_FT3);
            if (!(color_scale & WMAP_MODEL_FORCE_OPAQUE))
            {
                s32 primitive_code = face->semi_transparent;
                if (primitive_code == 0)
                {
                    primitive_code = WMAP_MODEL_FT3;
                }
                else
                {
                    primitive_code = WMAP_MODEL_FT3 | WMAP_MODEL_SEMI_TRANS;
                }
                setcode(ft3, primitive_code);
            }
            addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], ft3);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(POLY_FT3);
                g_wmap_current_frame->packet_cursor += sizeof(POLY_FT3);
            }
            break;
        }
        case WMAP_MODEL_G4:
        {
            POLY_G4* g4 = (POLY_G4*)g_wmap_current_frame->packet_cursor;
            LOAD_MODEL_VERTEX(projection_vertices[0], vertices, face->vertex_indices[0]);
            LOAD_MODEL_VERTEX(projection_vertices[1], vertices, face->vertex_indices[1]);
            LOAD_MODEL_VERTEX(projection_vertices[2], vertices, face->vertex_indices[2]);
            gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
            gte_rtpt();
            LOAD_MODEL_VERTEX(projection_vertices[3], vertices, face->vertex_indices[3]);
            colors[0] = attributes->colors[0];
            SET_MODEL_COLOR(g4->r0, colors[0], color_scale);
            colors[4] = attributes->colors[1];
            SET_MODEL_COLOR(g4->r1, colors[4], color_scale);
            gte_stsxy01(&screen_xy0, &screen_xy1);
            gte_stsxy2(&screen_xy2);
            gte_nclip();
            colors[8] = attributes->colors[2];
            SET_MODEL_COLOR(g4->r2, colors[8], color_scale);
            colors[12] = attributes->colors[3];
            SET_MODEL_COLOR(g4->r3, colors[12], color_scale);
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
            *(s32*)&g4->x0 = screen_xy0.packed;
            *(s32*)&g4->x1 = screen_xy1.packed;
            gte_stsxy(&screen_xy3);
            OFFSET_SCREEN_XY(screen_xy3, x_offset, y_offset);
            *(s32*)&g4->x2 = screen_xy2.packed;
            *(s32*)&g4->x3 = screen_xy3.packed;
            setlen(g4, 8);
            setcode(g4, WMAP_MODEL_G4);
            if (blend_mode >= 0)
            {
                setcode(g4, (WMAP_MODEL_G4 | WMAP_MODEL_SEMI_TRANS));
            }
            addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], g4);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(POLY_G4);
                g_wmap_current_frame->packet_cursor += sizeof(POLY_G4);
            }
            break;
        }
        case WMAP_MODEL_G3:
        {
            WmapPolyG3* g3 = (WmapPolyG3*)g_wmap_current_frame->packet_cursor;
            if (z_divisor == -1)
            {
                LOAD_MODEL_VERTEX(projection_vertices[0], vertices, face->vertex_indices[0]);
                LOAD_MODEL_VERTEX(projection_vertices[1], vertices, face->vertex_indices[1]);
                LOAD_MODEL_VERTEX(projection_vertices[2], vertices, face->vertex_indices[2]);
            }
            else
            {
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[0], vertices, face->vertex_indices[0], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[1], vertices, face->vertex_indices[1], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[2], vertices, face->vertex_indices[2], z_divisor);
            }
            gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
            gte_rtpt();
            colors[0] = attributes->colors[0];
            SET_MODEL_COLOR(g3->r0, colors[0], color_scale);
            gte_stsxy01(&screen_xy0, &screen_xy1);
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
            *(s32*)&g3->x0 = screen_xy0.packed;
            *(s32*)&g3->x1 = screen_xy1.packed;
            *(s32*)&g3->x2 = screen_xy2.packed;
            colors[4] = attributes->colors[1];
            SET_MODEL_COLOR(g3->r1, colors[4], color_scale);
            colors[8] = attributes->colors[2];
            SET_MODEL_COLOR(g3->r2, colors[8], color_scale);
            setlen(g3, 6);
            setcode(g3, WMAP_MODEL_G3);
            if (blend_mode >= 0)
            {
                setcode(g3, (WMAP_MODEL_G3 | WMAP_MODEL_SEMI_TRANS));
            }
            addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], g3);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(WmapPolyG3);
                g_wmap_current_frame->packet_cursor += sizeof(WmapPolyG3);
            }
            break;
        }
        case WMAP_MODEL_F4:
        {
            POLY_F4* f4 = (POLY_F4*)g_wmap_current_frame->packet_cursor;
            LOAD_MODEL_VERTEX(projection_vertices[0], vertices, face->vertex_indices[0]);
            LOAD_MODEL_VERTEX(projection_vertices[1], vertices, face->vertex_indices[1]);
            LOAD_MODEL_VERTEX(projection_vertices[2], vertices, face->vertex_indices[2]);
            gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
            gte_rtpt();
            LOAD_MODEL_VERTEX(projection_vertices[3], vertices, face->vertex_indices[3]);
            gte_stsxy01(&screen_xy0, &screen_xy1);
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
            *(s32*)&f4->x0 = screen_xy0.packed;
            *(s32*)&f4->x1 = screen_xy1.packed;
            gte_stsxy(&screen_xy3);
            *(s32*)&f4->x2 = screen_xy2.packed;
            *(s32*)&f4->x3 = screen_xy3.packed;
            colors[0] = attributes->colors[0];
            SET_MODEL_COLOR(f4->r0, colors[0], color_scale);
            setlen(f4, 5);
            setcode(f4, WMAP_MODEL_F4);
            if (blend_mode >= 0)
            {
                setcode(f4, (WMAP_MODEL_F4 | WMAP_MODEL_SEMI_TRANS));
            }
            addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], f4);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(POLY_F4);
                g_wmap_current_frame->packet_cursor += sizeof(POLY_F4);
            }
            break;
        }
        case WMAP_MODEL_F3:
        {
            WmapPolyF3* f3 = (WmapPolyF3*)g_wmap_current_frame->packet_cursor;
            LOAD_MODEL_VERTEX(projection_vertices[0], vertices, face->vertex_indices[0]);
            LOAD_MODEL_VERTEX(projection_vertices[1], vertices, face->vertex_indices[1]);
            LOAD_MODEL_VERTEX(projection_vertices[2], vertices, face->vertex_indices[2]);
            gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
            gte_rtpt();
            colors[0] = attributes->colors[0];
            SET_MODEL_COLOR(f3->r0, colors[0], color_scale);
            gte_stsxy01(&screen_xy0, &screen_xy1);
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
            *(s32*)&f3->x0 = screen_xy0.packed;
            *(s32*)&f3->x1 = screen_xy1.packed;
            *(s32*)&f3->x2 = screen_xy2.packed;
            setlen(f3, 4);
            setcode(f3, WMAP_MODEL_F3);
            if (blend_mode >= 0)
            {
                setcode(f3, (WMAP_MODEL_F3 | WMAP_MODEL_SEMI_TRANS));
            }
            addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], f3);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(WmapPolyF3);
                g_wmap_current_frame->packet_cursor += sizeof(WmapPolyF3);
            }
            break;
        }
        case WMAP_MODEL_GT3:
        {
            WmapPolyGT3* gt3;
            if (z_divisor == -1)
            {
                LOAD_MODEL_VERTEX(projection_vertices[0], vertices, face->vertex_indices[0]);
                LOAD_MODEL_VERTEX(projection_vertices[1], vertices, face->vertex_indices[1]);
                LOAD_MODEL_VERTEX(projection_vertices[2], vertices, face->vertex_indices[2]);
            }
            else
            {
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[0], vertices, face->vertex_indices[0], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[1], vertices, face->vertex_indices[1], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[2], vertices, face->vertex_indices[2], z_divisor);
            }
            gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
            gte_rtpt();
            gt3 = (WmapPolyGT3*)g_wmap_current_frame->packet_cursor;
            gt3->tpage = tpage;
            gt3->clut = clut;
            *(u16*)&gt3->u0 = attributes->uv[0];
            *(u16*)&gt3->u1 = attributes->uv[1];
            *(u16*)&gt3->u2 = attributes->uv[2];
            gte_stsxy01(&screen_xy0, &screen_xy1);
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
            *(s32*)&gt3->x0 = screen_xy0.packed;
            *(s32*)&gt3->x1 = screen_xy1.packed;
            *(s32*)&gt3->x2 = screen_xy2.packed;
            colors[0] = attributes->colors[0];
            colors[2] = colors[0];
            SET_MODEL_COLOR(gt3->r0, colors[2], (color_scale & WMAP_MODEL_COLOR_MASK));
            colors[0] = attributes->colors[1];
            colors[6] = colors[0];
            SET_MODEL_COLOR(gt3->r1, colors[6], (color_scale & WMAP_MODEL_COLOR_MASK));
            colors[0] = attributes->colors[2];
            colors[10] = colors[0];
            SET_MODEL_COLOR(gt3->r2, colors[10], (color_scale & WMAP_MODEL_COLOR_MASK));
            setlen(gt3, 9);
            setcode(gt3, WMAP_MODEL_GT3);
            if (!(color_scale & WMAP_MODEL_FORCE_OPAQUE))
            {
                s32 primitive_code = face->semi_transparent;
                if (primitive_code == 0)
                {
                    primitive_code = WMAP_MODEL_GT3;
                }
                else
                {
                    primitive_code = WMAP_MODEL_GT3 | WMAP_MODEL_SEMI_TRANS;
                }
                setcode(gt3, primitive_code);
            }
            addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], gt3);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(WmapPolyGT3);
                g_wmap_current_frame->packet_cursor += sizeof(WmapPolyGT3);
            }
            break;
        }
        case WMAP_MODEL_GT4:
        {
            POLY_GT4* gt4 = (POLY_GT4*)g_wmap_current_frame->packet_cursor;
            if (z_divisor == -1)
            {
                LOAD_MODEL_VERTEX(projection_vertices[0], vertices, face->vertex_indices[0]);
                LOAD_MODEL_VERTEX(projection_vertices[1], vertices, face->vertex_indices[1]);
                LOAD_MODEL_VERTEX(projection_vertices[2], vertices, face->vertex_indices[2]);
                LOAD_MODEL_VERTEX(projection_vertices[3], vertices, face->vertex_indices[3]);
            }
            else
            {
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[0], vertices, face->vertex_indices[0], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[1], vertices, face->vertex_indices[1], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[2], vertices, face->vertex_indices[2], z_divisor);
                LOAD_MODEL_VERTEX_SCALED(projection_vertices[3], vertices, face->vertex_indices[3], z_divisor);
            }
            gte_ldv3(&projection_vertices[0], &projection_vertices[1], &projection_vertices[2]);
            gte_rtpt();
            gt4->tpage = tpage;
            gt4->clut = clut;
            *(u16*)&gt4->u0 = attributes->uv[0];
            *(u16*)&gt4->u1 = attributes->uv[1];
            *(u16*)&gt4->u2 = attributes->uv[2];
            gte_stsxy01(&screen_xy0, &screen_xy1);
            gte_stsxy2(&screen_xy2);
            gte_nclip();
            *(u16*)&gt4->u3 = attributes->uv[3];
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
            *(s32*)&gt4->x0 = screen_xy0.packed;
            *(s32*)&gt4->x1 = screen_xy1.packed;
            *(s32*)&gt4->x2 = screen_xy2.packed;
            *(s32*)&gt4->x3 = screen_xy3.packed;
            colors[0] = attributes->colors[0];
            colors[2] = colors[0];
            SET_MODEL_COLOR(gt4->r0, colors[2], (color_scale & WMAP_MODEL_COLOR_MASK));
            colors[0] = attributes->colors[1];
            colors[6] = colors[0];
            SET_MODEL_COLOR(gt4->r1, colors[6], (color_scale & WMAP_MODEL_COLOR_MASK));
            colors[0] = attributes->colors[2];
            colors[10] = colors[0];
            SET_MODEL_COLOR(gt4->r2, colors[10], (color_scale & WMAP_MODEL_COLOR_MASK));
            colors[0] = attributes->colors[3];
            colors[16] = colors[0];
            SET_MODEL_COLOR(gt4->r3, colors[16], (color_scale & WMAP_MODEL_COLOR_MASK));
            setlen(gt4, 12);
            setcode(gt4, WMAP_MODEL_GT4);
            if (!(color_scale & WMAP_MODEL_FORCE_OPAQUE))
            {
                s32 primitive_code = face->semi_transparent;
                if (primitive_code == 0)
                {
                    primitive_code = WMAP_MODEL_GT4;
                }
                else
                {
                    primitive_code = WMAP_MODEL_GT4 | WMAP_MODEL_SEMI_TRANS;
                }
                setcode(gt4, primitive_code);
            }
            addPrim(&g_wmap_current_frame->ordering_table[ot_index + face_order], gt4);
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

        face = wmap_advance_record(face, sizeof(*face));
        attributes = wmap_advance_record(attributes, sizeof(*attributes));
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
