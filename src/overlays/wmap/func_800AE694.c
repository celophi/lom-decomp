/* Partial WMAP decompilation: 87.407410% (gcc280_g0). */
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
extern u8 D_80123538[];
extern s32 D_801B0FD0;
extern s32 D_801B2F60;
extern s32 D_801B2F64;
extern s32 rand(void);
extern void func_800B09E8(void);

/** @brief Initialize effect actors with randomized angles and speeds. */
void func_800AE694(void)
{
    s32 i;
    s32 motion_offset;
    s32 resource_offset;
    WmapMotion *motion;
    WmapConfigA *actor;

    i = 0;
    motion_offset = 0;
    resource_offset = 400;
    D_801B0FD0 = 30;
    D_80139280[21] = -1;
    D_80139280[22] = -2;
    D_80139280[23] = 180;
    D_80139280[24] = 32;
    D_80139280[25] = -1;
    D_80139280[26] = -1600;
    D_80139280[27] = 50;
    D_80139280[28] = 8;
    D_80139280[29] = 1;
    D_80139280[30] = 102000;
    do
    {
        actor = &D_800D9B00[i];
        motion = (WmapMotion *)((u8 *)D_801AFFB8 + motion_offset);
        ((WmapResource *)((u8 *)D_80139988 + resource_offset))->resource = D_80123538;
        motion->state = 1;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_02 = 0;
        actor->field_0E = 1;
        actor->field_22 = 255;
        actor->field_24 = 1;
        actor->field_26 = 0;
        motion->state = 1;
        motion->angle = rand() & 4095;
        motion->z = 102000;
        motion->x = (-(rand() * 2) >> 15) - 1;
        motion->field_0E = ((rand() * 100) >> 15) + 180;
        motion_offset += 20;
        resource_offset += 8;
        i++;
    } while (i < 30);
    D_801B2F64 = 32;
    D_801B2F60++;
    func_800B09E8();
}
