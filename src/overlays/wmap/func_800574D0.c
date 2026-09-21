#include "common.h"
#include "sdk/libgpu.h"

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
    s16 x;
    s16 y;
    u8 pad_004[16];
} WmapSpritePosition;

extern WmapSpritePosition D_8004FD9C[];
extern SPRT D_8004FE34[];
extern SPRT D_8004FF74[];
extern s32 D_800D921C;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF18;
extern s32 D_8011CF74;
extern s32 D_8011D4FC;
extern s32 D_80129550;
extern WmapRenderState* D_801398EC;
extern WmapProjectionState D_80139950;
extern s32 D_8013B268;

s32 func_8005D8FC(void);
void func_8005D7A0(s32 value, s32* tile_indices);
void func_8005C404(s32 map_x, s32 map_y, s32 value, s32 map_x_copy, s32 map_y_copy, s32* tile_indices);
void func_8005D6B8(s32 map_x, s32 map_y, s32* tile_indices);
void func_8006534C(s32 type, s32 depth);

/**
 * @brief Build and queue two sprite layers for each of eight world-map entries.
 */
void func_800574D0(void)
{
    s32 tile_indices[8];
    s32 map_x;
    s32 map_y;
    s32 intensity;
    s32 i;

    if (D_8013B268 == 0)
    {
        return;
    }

    map_x = D_80139950.x / 48 + D_800DCEEC;
    map_y = D_80139950.y / 48 + D_800DCEF0;

    if (map_x < 0)
    {
        map_x = 0;
    }
    if (map_y < 0)
    {
        map_y = 0;
    }
    if (map_x >= 6)
    {
        map_x = 5;
    }
    if (map_y >= 6)
    {
        map_y = 5;
    }

    intensity = D_8013B268 + (D_8011CF74 & 8);
    if (intensity < 0)
    {
        intensity = 0;
    }

    if (D_8011CF18 == 2)
    {
        func_8005D7A0(func_8005D8FC(), tile_indices);
    }
    else if (D_80129550 == 1)
    {
        func_8005C404(map_x, map_y, D_8011D4FC, map_x, map_y, tile_indices);
    }
    else
    {
        func_8005D6B8(map_x, map_y, tile_indices);
    }

    for (i = 0; i < 8; i++)
    {
        SPRT* first_sprite;
        SPRT* second_sprite;
        s32 first_prim_count;
        s32 second_prim_count;

        first_sprite = D_801398EC->prim_cursor;
        *first_sprite = D_8004FF74[tile_indices[i]];
        first_sprite->r0 = first_sprite->g0 = first_sprite->b0 = intensity;
        *(u32*)&first_sprite->x0 = *(u32*)&D_8004FD9C[i].x;
        first_sprite->clut = (0x6A00 + i * 0x40) | 0x2E;
        setSemiTrans(first_sprite, 1);
        addPrim(&D_801398EC->ordering_table[1], first_sprite);
        first_prim_count = D_800D921C;

        if (first_prim_count < 0x7D00)
        {
            D_800D921C = first_prim_count + sizeof(SPRT);
            D_801398EC->prim_cursor = (u8*)D_801398EC->prim_cursor + sizeof(SPRT);
        }

        second_sprite = D_801398EC->prim_cursor;
        *second_sprite = D_8004FE34[tile_indices[i]];
        second_sprite->r0 = second_sprite->g0 = second_sprite->b0 = D_8013B268;
        *(u32*)&second_sprite->x0 = *(u32*)&D_8004FD9C[i].x;
        second_sprite->x0 += 4;
        second_sprite->y0 += 20;
        second_sprite->clut = (0x6800 + i * 0x40) | 0x2E;
        if (D_8013B268 < 0x40)
        {
            setSemiTrans(second_sprite, 1);
        }
        addPrim(&D_801398EC->ordering_table[1], second_sprite);
        second_prim_count = D_800D921C;

        if (second_prim_count < 0x7D00)
        {
            D_800D921C = second_prim_count + sizeof(SPRT);
            D_801398EC->prim_cursor = (u8*)D_801398EC->prim_cursor + sizeof(SPRT);
        }
    }

    func_8006534C(0x3D, 1);
}
