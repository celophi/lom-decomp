#include "common.h"

/** @brief World-map actor configuration. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D9B00[];
extern WmapMotion D_801AFFB8[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern s32 D_801B0FD0;
extern u8 D_8011D538[];
extern s32 D_801B29E0;
extern s32 D_801B29E4;
extern void func_8008C134(void);

/** @brief Initialize the effect actors, resources, and evenly spaced angles. */
void func_8008AB94(void)
{
    s32 i;

    i = 0;
    D_801B0FD0 = 3;
    D_80139280[0xB] = 4;
    D_80139280[0xC] = 4;
    D_80139280[0xD] = 128;
    D_80139280[0xE] = 0;
    D_80139280[0xF] = -1;
    D_80139280[0x10] = 100;
    D_80139280[0x11] = 50;
    D_80139280[0x12] = 15;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 12000;
    do
    {
        D_801AFFB8[i].state = 1;
        D_801AFFB8[i].angle = i * 1365;
        D_801AFFB8[i].scale = 128;
        D_801AFFB8[i].z = 12000;
        D_801AFFB8[i].x = 0;
        D_801AFFB8[i].field_0E = 0;
        D_80139988[i + 50].resource = D_8011D538;
        D_800D9B00[i].field_06 = 15;
        D_800D9B00[i].field_10 = -1;
        D_800D9B00[i].field_02 = 0;
        D_800D9B00[i].field_0E = 2;
        D_800D9B00[i].field_26 = 2;
        D_800D9B00[i].field_22 = 0;
        D_800D9B00[i].field_24 = 127;
        i++;
    } while (i < 3);
    D_801B29E4 = 64;
    D_801B29E0++;
    func_8008C134();
}
