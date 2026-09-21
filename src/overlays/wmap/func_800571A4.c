#include "common.h"

typedef struct
{
    s16 unk00;
    s16 unk02;
    u8 state;
    u8 phase;
    s8 transition;
    u8 pad07[7];
    s16 unk0e;
    s16 unk10;
    u8 pad12[0xE];
    s16 unk20;
    u8 pad22[0xA];
} WmapObject;

typedef struct
{
    s16 unk00;
    s16 unk02;
    s16 state;
    s16 frame;
    s32 unk08;
    s32 unk0c;
    s16 unk10;
    u8 pad12[2];
    s32 unk14;
    s32 unk18;
} WmapEffectCell;

extern WmapObject D_80182248[64];
extern WmapEffectCell D_8011D108[6][6];

/**
 * @brief Reset world-map objects and effect cells to their initial state.
 */
void func_800571A4(void)
{
    s32 i;
    s32 x;
    s32 y;

    for (i = 0; i < 64; i++)
    {
        D_80182248[i].unk00 = i;
        D_80182248[i].unk02 = i;
        D_80182248[i].unk10 = 0;
        D_80182248[i].unk0e = 0;
        D_80182248[i].transition = 0;
        D_80182248[i].phase = 0;
        D_80182248[i].state = 0;
        D_80182248[i].unk20 = 0;
    }

    for (y = 0; y < 6; y++)
    {
        for (x = 0; x < 6; x++)
        {
            WmapEffectCell cell;

            D_8011D108[x][y].unk00 = i;
            D_8011D108[x][y].frame = 0;
            D_8011D108[x][y].state = 0;
            D_8011D108[x][y].unk02 = 0;
            D_8011D108[x][y].unk10 = 0;
            cell = D_8011D108[x][y];
        }
    }
}
