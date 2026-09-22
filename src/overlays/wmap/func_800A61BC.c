#include "common.h"

typedef struct
{
    s32 tile;
    u8 pad_04[36];
} WmapTile;
extern WmapTile D_80139290[6][6];
extern u8 D_800DCEF4[4];
extern s8 D_800DCEF5;
extern s8 D_800DCEF6;
extern s8 D_800DCEF7;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_8018222C;
extern s32 rand(void);
extern void func_8006D0F0(s32, s32 *, s32 *);

static inline s32 tile_exists(s32 x, s32 y)
{
    if (x < 0 || y < 0 || x >= 6 || y >= 6)
    {
        return 0;
    }
    return D_80139290[x][y].tile != 255;
}

/** @brief Select the first occupied neighbor, or a random direction when isolated. */
void func_800A61BC(s32 tile)
{
    func_8006D0F0(tile, &D_800DCEF8, &D_800DCF00);
    D_8018222C = 0;
    D_800DCEF4[3] = 0;
    D_800DCEF4[2] = 0;
    D_800DCEF4[1] = 0;
    D_800DCEF4[0] = 0;
    if (tile_exists(D_800DCEF8 + 1, D_800DCF00))
    {
        D_800DCEF4[0] = 1;
        return;
    }
    if (tile_exists(D_800DCEF8, D_800DCF00 + 1))
    {
        D_800DCEF5 = 1;
        return;
    }
    if (tile_exists(D_800DCEF8 - 1, D_800DCF00))
    {
        D_800DCEF6 = 1;
        return;
    }
    if (tile_exists(D_800DCEF8, D_800DCF00 - 1))
    {
        D_800DCEF7 = 1;
        return;
    }
    D_800DCEF4[rand() & 3] = 1;
}
