#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    u8 pad00[4];
    s16 state;
    s16 frame;
    u8 pad08[0x14];
} WmapEffectCell;

typedef struct
{
    u16 x;
    u16 y;
    u16 z;
    u16 pad;
} WmapQuadVertex;

typedef struct
{
    WmapQuadVertex vertices[4];
    CVECTOR colors[4];
} WmapQuadTemplate;

typedef struct
{
    s32 x;
    s32 y;
    s32 scale;
} WmapProjection;

typedef struct
{
    u8 pad000[0x328];
    u_long ot_entry;
    u8 pad32C[0x10];
    POLY_G4* prim_cursor;
} WmapRenderState;

extern s32 D_800500C8[];
extern WmapQuadTemplate D_800CB8E8[16];
extern s32 D_800D921C;
extern WmapEffectCell D_8011D108[6][6];
extern s32 D_8011D4FC;
extern WmapRenderState* D_801398EC;
extern WmapProjection D_80139950;

void func_8006534C(s32, s32);

/**
 * @brief Render the animated world-map quad set for one map cell.
 * @param x World-map grid X coordinate.
 * @param y World-map grid Y coordinate.
 */
void func_80056C30(s32 x, s32 y)
{
    WmapEffectCell* cell;
    WmapQuadVertex base;
    WmapQuadVertex transformed[4];
    s32 sxy0;
    s32 sxy1;
    s32 sxy2;
    s32 sxy3;
    s32 projected_x;
    s32 projected_y;
    s32 i;
    s32 color_frame;
    s32 frame_delta;

    cell = &D_8011D108[x][y];
    frame_delta = D_800500C8[cell->state];
    cell->frame += frame_delta;
    if (cell->frame >= 0x11)
    {
        cell->frame = 0x10;
    }
    else if (cell->frame < 0)
    {
        cell->frame = 0;
    }

    if (cell->frame >= 0xF)
    {
        cell->state = 2;
    }

    if (cell->frame <= 0)
    {
        cell->state = 1;
        return;
    }

    projected_x = (D_80139950.x * 0x14000) / D_80139950.scale;
    projected_y = (D_80139950.y * 0x14000) / D_80139950.scale;
    base.x = ((((x - 1) * 0xA0) - projected_x) * 0x6000) / D_80139950.scale + 0xA;
    base.y = ((((y - 1) * 0xA0) - projected_y) * 0x6000) / D_80139950.scale + 0xC;
    base.z = 0;

    color_frame = cell->frame * 7;

    for (i = 0; i < 16; i++)
    {
        WmapQuadTemplate* source;
        POLY_G4* poly;

        source = &D_800CB8E8[i];
        poly = D_801398EC->prim_cursor;

        transformed[0].x = base.x + source->vertices[0].x;
        transformed[0].y = base.y + source->vertices[0].y;
        transformed[0].z = base.z + source->vertices[0].z;
        transformed[1].x = base.x + source->vertices[1].x;
        transformed[1].y = base.y + source->vertices[1].y;
        transformed[1].z = base.z + source->vertices[1].z;
        transformed[2].x = base.x + source->vertices[2].x;
        transformed[2].y = base.y + source->vertices[2].y;
        transformed[2].z = base.z + source->vertices[2].z;
        transformed[3].x = base.x + source->vertices[3].x;
        transformed[3].y = base.y + source->vertices[3].y;
        transformed[3].z = base.z + source->vertices[3].z;

        gte_ldv3(&transformed[0], &transformed[1], &transformed[2]);
        gte_rtpt();

        *(u32*)&poly->r0 = func_8006CF40(source->colors[0], color_frame);
        *(u32*)&poly->r1 = func_8006CF40(source->colors[1], color_frame);
        *(u32*)&poly->r2 = func_8006CF40(source->colors[2], color_frame);
        *(u32*)&poly->r3 = func_8006CF40(source->colors[3], color_frame);

        if ((D_8011D4FC == 3) || (D_8011D4FC == 7) || (D_8011D4FC == 0x1B))
        {
            poly->r0 = poly->r1 = poly->r2 = poly->r3 = 0;
        }

        gte_stsxy3(&sxy0, &sxy1, &sxy2);
        gte_ldv0(&transformed[3]);
        gte_rtps();

        *(u32*)&poly->x0 = sxy0;
        *(u32*)&poly->x1 = sxy1;
        *(u32*)&poly->x2 = sxy2;
        gte_stsxy(&sxy3);
        *(u32*)&poly->x3 = sxy3;

        setlen(poly, 8);
        setcode(poly, 0x3A);
        addPrim(&D_801398EC->ot_entry, poly);

        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(POLY_G4);
            D_801398EC->prim_cursor++;
        }
    }

    func_8006534C(0xAE, 0xAE);
}
