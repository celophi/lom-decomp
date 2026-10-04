#include "../internal/wmap_frame_render.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_effect_backdrop.h"
#include <libgpu.h>

#define WMAP_MESH_TRIANGLE_COUNT 176
#define WMAP_MESH_VERTEX_COUNT (WMAP_MESH_TRIANGLE_COUNT * 3)
#define WMAP_MESH_OT_INDEX 9
#define WMAP_MESH_TPAGE 0x7B54
#define WMAP_MESH_PACKET_WORDS 6
#define WMAP_MESH_POLY_G3_CODE 0x32
#define WMAP_MESH_TRANSITION_FRAMES 16

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

extern WmapPoint g_wmap_transition_mesh_vertices[];
extern WmapTriangle g_wmap_transition_mesh_triangles_0[];
extern WmapTriangle g_wmap_transition_mesh_triangles_1[];
extern WmapFixedPoint g_wmap_transition_mesh_positions[];
extern WmapFixedPoint* g_wmap_transition_mesh_motion;

extern s32 g_wmap_frame_count;
extern s32 g_wmap_transition_mesh_hidden;
extern WmapFixedPoint g_wmap_transition_mesh_motion_0[];
extern WmapFixedPoint g_wmap_transition_mesh_motion_1[];

/** @brief Initialize triangle packets in both buffers and expand their coordinates. */
void wmap_init_transition_mesh(void)
{
    s32 i;

    wmap_select_mesh_motion(0);
    for (i = 0; i < WMAP_MESH_TRIANGLE_COUNT; i++)
    {
        g_wmap_transition_mesh_triangles_0[i].header.packet.length = WMAP_MESH_PACKET_WORDS;
        g_wmap_transition_mesh_triangles_0[i].code = WMAP_MESH_POLY_G3_CODE;
        *(s32*)&g_wmap_transition_mesh_triangles_0[i].x0 = g_wmap_transition_mesh_vertices[i * 3].packed;
        *(s32*)&g_wmap_transition_mesh_triangles_0[i].x1 = g_wmap_transition_mesh_vertices[i * 3 + 1].packed;
        *(s32*)&g_wmap_transition_mesh_triangles_0[i].x2 = g_wmap_transition_mesh_vertices[i * 3 + 2].packed;
        g_wmap_transition_mesh_triangles_1[i] = g_wmap_transition_mesh_triangles_0[i];
    }
    for (i = 0; i < WMAP_MESH_VERTEX_COUNT; i++)
    {
        g_wmap_transition_mesh_positions[i].x.value = g_wmap_transition_mesh_vertices[i].point.x << 16;
        g_wmap_transition_mesh_positions[i].y.value = g_wmap_transition_mesh_vertices[i].point.y << 16;
    }
}

/** @brief Update the transition mesh and append its triangles for rendering. */
void wmap_draw_transition_mesh(void)
{
    WmapTriangle *triangles;
    s32 i;
    s32 remaining;

    if (g_wmap_transition_mesh_hidden != 1)
    {
        if (g_wmap_mesh_transition_direction != 0)
        {
            if (g_wmap_frame_count & 1)
            {
                triangles = g_wmap_transition_mesh_triangles_0;
            }
            else
            {
                triangles = g_wmap_transition_mesh_triangles_1;
            }
            if (g_wmap_mesh_transition_frames >= 2)
            {
                for (i = 0; i < WMAP_MESH_VERTEX_COUNT; i++)
                {
                    g_wmap_transition_mesh_positions[i].x.value += g_wmap_mesh_transition_direction * g_wmap_transition_mesh_motion[i].x.value;
                    g_wmap_transition_mesh_positions[i].y.value += g_wmap_mesh_transition_direction * g_wmap_transition_mesh_motion[i].y.value;
                }
            }
            for (i = 0; i < WMAP_MESH_TRIANGLE_COUNT; i++)
            {
                triangles->x0 = g_wmap_transition_mesh_positions[i * 3].x.parts.whole;
                triangles->y0 = g_wmap_transition_mesh_positions[i * 3].y.parts.whole;
                triangles->x1 = g_wmap_transition_mesh_positions[i * 3 + 1].x.parts.whole;
                triangles->y1 = g_wmap_transition_mesh_positions[i * 3 + 1].y.parts.whole;
                triangles->x2 = g_wmap_transition_mesh_positions[i * 3 + 2].x.parts.whole;
                triangles->y2 = g_wmap_transition_mesh_positions[i * 3 + 2].y.parts.whole;
                triangles++;
            }
            remaining = g_wmap_mesh_transition_frames - 1;
            g_wmap_mesh_transition_frames = remaining;
            if (remaining == 0)
            {
                g_wmap_mesh_transition_direction = 0;
            }
        }
        if (g_wmap_frame_count & 1)
        {
            triangles = g_wmap_transition_mesh_triangles_0;
        }
        else
        {
            triangles = g_wmap_transition_mesh_triangles_1;
        }
        for (i = 0; i < WMAP_MESH_TRIANGLE_COUNT; i++)
        {
            addPrim(&g_wmap_current_frame->ordering_table[WMAP_MESH_OT_INDEX], triangles);
            triangles++;
        }
        func_8006534C(WMAP_MESH_TPAGE, WMAP_MESH_OT_INDEX);
    }
}

/**
 * @brief Start a mesh transition when its selection changes.
 * @param selection Zero reverses the vertex motion; nonzero moves it forward.
 */
void wmap_start_mesh_transition(s32 selection)
{
    s32 previous_selection;

    if (selection != g_wmap_mesh_transition_selection)
    {
        if (g_wmap_mesh_transition_direction != 0)
        {
            func_80064F14();
        }
        if (selection == 0)
        {
            g_wmap_mesh_transition_direction = -1;
        }
        else
        {
            g_wmap_mesh_transition_direction = 1;
        }
        g_wmap_mesh_transition_frames = WMAP_MESH_TRANSITION_FRAMES;
        previous_selection = g_wmap_mesh_transition_selection;
        g_wmap_mesh_transition_selection = selection;
        g_wmap_mesh_previous_selection = previous_selection;
    }
}

/**
 * @brief Select the per-vertex motion table for the transition mesh.
 * @param use_second Nonzero selects the second motion table.
 */
void wmap_select_mesh_motion(s32 use_second)
{
    if (use_second != 0)
    {
        g_wmap_transition_mesh_motion = g_wmap_transition_mesh_motion_1;
        return;
    }
    g_wmap_transition_mesh_motion = g_wmap_transition_mesh_motion_0;
}
