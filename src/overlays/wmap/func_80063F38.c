#include "common.h"

/** @brief World-map tile and its cached neighboring layout data. */
typedef struct
{
    s32 tile;
    s16 field_04;
    s16 field_06;
    u8 neighbors[32];
} WmapTile;

/** @brief Per-tile display state. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[24];
} WmapTileDisplay;

extern WmapTile D_80139290[6][6];
extern WmapTileDisplay D_8011D108[6][6];
extern s32 D_8011D4FC;
extern s32 D_80139830;
extern s32 D_80182E20;
extern s32 func_8005D670(s32, s32);
extern s32 func_8005D554(u32, s32);
extern void func_8005D6B8(s32, s32, void *);
extern s16 func_8005B8C8(s32, s32, s32);

/** @brief Rebuild the tile layout and reset each tile's display state. */
void func_80063F38(void)
{
    s32 x;
    s32 y;
    s32 tile;

    for (y = 0; y < 6; y++)
    {
        for (x = 0; x < 6; x++)
        {
            tile = func_8005D670(x, y);
            if (D_80139830 != 0)
            {
                if (tile == 5)
                {
                    tile = 35;
                }
            }
            if (D_80182E20 != 0)
            {
                if (tile == 1)
                {
                    tile = 36;
                }
            }
            D_80139290[x][y].tile = tile;
            D_80139290[x][y].field_06 = func_8005D554(x, y);
            func_8005D6B8(x, y, D_80139290[x][y].neighbors);
            D_80139290[x][y].field_04 = func_8005B8C8(x, y, D_8011D4FC);
            D_8011D108[x][y].field_02 = 0;
        }
    }
}
