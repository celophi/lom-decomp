#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    u8 pad_000[0x70];
    u_long ordering_table[179];
    void* prim_cursor;
} WmapRenderState;

typedef struct
{
    s32 x;
    s32 y;
    s32 scale;
} WmapProjectionState;

typedef struct
{
    u16 x;
    u16 y;
} WmapMapPoint;

typedef struct
{
    s16 unk0;
    s16 texture_index;
} WmapMarker;

extern WmapMapPoint D_8004FD04[];
extern s16 D_800D036C[];
extern s32 D_800D921C;
extern s32 D_8013986C;
extern WmapRenderState* D_801398EC;
extern WmapProjectionState D_80139950;

void func_8006534C(s32 type, s32 depth);

/**
 * @brief Build and enqueue a world-map marker sprite.
 * @param map_x Map-grid X coordinate.
 * @param map_y Map-grid Y coordinate.
 * @param marker Marker record containing the texture selection.
 */
void func_80055E5C(s32 map_x, s32 map_y, WmapMarker* marker)
{
    SPRT* sprite;
    s32 depth;
    s32 type;
    SVECTOR position;
    u32 screen;
    u16* screen_ptr;
    s32 projected_z;

    sprite = D_801398EC->prim_cursor;

    switch (D_8013986C)
    {
    case 0:
    {
        s32 scale;
        s32 z_bucket;
        s32 sprite_y;

        scale = D_80139950.scale;
        position.vx = ((((map_x - 1) * 160) - ((D_80139950.x * 0x14000) / scale)) * 0x6000) / scale;
        position.vy = ((((map_y - 1) * 160) - ((D_80139950.y * 0x14000) / scale)) * 0x6000) / scale;
        position.vz = 0;

        gte_ldv0(&position);
        gte_rtps();
        gte_stsxy(&screen);
        gte_stszotz(&projected_z);
        screen_ptr = (u16*)&screen;

        if ((s32)(screen_ptr[1] << 16) < 0)
        {
            return;
        }

        z_bucket = (0x1B91 - projected_z) / 4;
        depth = z_bucket + 0x2F;
        if (depth < 0x2E || depth > 0xAE)
        {
            depth = 0x2E;
        }

        sprite->x0 = screen_ptr[0] - 8;
        sprite_y = screen_ptr[1] - 22;
        sprite->y0 = sprite_y;
        break;
    }
    case 1:
    {
        s32 sprite_x;
        s32 sprite_y;

        sprite_x = D_8004FD04[map_x + map_y * 6].x;
        sprite_y = D_8004FD04[map_x + map_y * 6].y;
        depth = 0xAE - map_y;
        sprite->x0 = sprite_x;
        sprite->y0 = sprite_y;
        break;
    }
    default:
        return;
    }

    *(u32*)&sprite->r0 = 0x808080;

    {
        s32 texture;
        s32 u_index;
        s32 v_index;

        texture = D_800D036C[marker->texture_index];
        u_index = texture & 7;
        sprite->u0 = u_index << 5;
        v_index = texture / 8;
        sprite->v0 = v_index << 5;

        switch (v_index)
        {
        case 0:
            sprite->clut = (u_index << 6) | 0x582E;
            type = 12;
            break;
        case 1:
            sprite->clut = (u_index << 6) | 0x5A2E;
            type = 12;
            break;
        case 2:
            sprite->clut = (u_index << 6) | 0x5C2E;
            type = 12;
            break;
        case 3:
            sprite->clut = (u_index << 6) | 0x5E2E;
            type = 12;
            break;
        }
    }

    *(u32*)&sprite->w = PACK_U16_PAIR(32, 32);
    setSprt(sprite);
    setSemiTrans(sprite, 1);

    addPrim(&D_801398EC->ordering_table[depth], sprite);

    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->prim_cursor = (u8*)D_801398EC->prim_cursor + sizeof(SPRT);
    }

    func_8006534C(type, depth);
}
