/* Partial WMAP decompilation: 99.777780% (gcc280_g0). */
#include "common.h"
#include "sdk/libgpu.h"

/** @brief World-map rendering buffer and its preallocated polygon packets. */
typedef struct
{
    u8 pad_000[0x340];
    POLY_FT4 tiles[26][26];
    POLY_F4 fade[184];
    u8 tail[0x20];
} WmapRenderBuffer;

extern WmapRenderBuffer *D_801398EC;

/** @brief Copy map edge coordinates into the fade polygons with a vertical offset. */
void func_80065F54(void)
{
    s32 row;
    s32 column;
    POLY_FT4 *tile;
    POLY_F4 *fade;
    POLY_FT4 *tiles;

    fade = D_801398EC->fade;
    for (row = 22; row < 25; row++)
    {
        tiles = D_801398EC->tiles[row];
        for (column = 1; column < 25; column++)
        {
            tile = &tiles[column];
            fade->x0 = tile->x0;
            fade->y0 = tile->y0 + 10;
            fade->x1 = tile->x1;
            fade->y1 = tile->y1 + 10;
            fade->x2 = tile->x2;
            fade->y2 = tile->y2 + 10;
            fade->x3 = tile->x3;
            fade->y3 = tile->y3 + 10;
            fade++;
        }
    }
    for (column = 23; column < 25; column++)
    {
        tiles = &D_801398EC->tiles[0][column];
        for (row = 3; row < 23; row++)
        {
            tile = &tiles[row * 26];
            fade->x0 = tile->x0;
            fade->y0 = tile->y0 + 10;
            fade->x1 = tile->x1;
            fade->y1 = tile->y1 + 10;
            fade->x2 = tile->x2;
            fade->y2 = tile->y2 + 10;
            fade->x3 = tile->x3;
            fade->y3 = tile->y3 + 10;
            fade++;
        }
    }
}
