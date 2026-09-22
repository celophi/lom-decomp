/* Partial WMAP decompilation: 87.650940% (gcc280_g0). */
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

extern WmapConfigA D_800D95D8[];
extern WmapMotion D_801AFD60[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern u8 D_80123538[];
extern s32 D_801B0FD0;
extern s32 D_801B2F58;
extern s32 D_801B2F5C;
extern s32 rand(void);
extern void func_800B07CC(void);

/** @brief Initialize effect actors with randomized angles and speeds. */
void func_800AE4EC(void)
{
    s32 i;
    s32 motion_offset;
    s32 resource_offset;
    WmapMotion *motion;
    WmapConfigA *actor;

    i = 0;
    motion_offset = 0;
    resource_offset = 160;
    D_801B0FD0 = 10;
    D_80139280[11] = 0;
    D_80139280[12] = -2;
    D_80139280[13] = 220;
    D_80139280[14] = 32;
    D_80139280[15] = -1;
    D_80139280[16] = -900;
    D_80139280[17] = 20;
    D_80139280[18] = 8;
    D_80139280[19] = 0;
    D_80139280[20] = 102000;
    do
    {
        actor = &D_800D95D8[i];
        motion = (WmapMotion *)((u8 *)D_801AFD60 + motion_offset);
        ((WmapResource *)((u8 *)D_80139988 + resource_offset))->resource = D_80123538;
        motion->state = 1;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_02 = 0;
        actor->field_0E = 0;
        actor->field_22 = 255;
        actor->field_24 = 1;
        actor->field_26 = 0;
        motion->state = 1;
        motion->angle = rand() & 4095;
        motion->z = 102000;
        motion->x = (-(rand() * 2) >> 15);
        motion->field_0E = ((rand() * 50) >> 15) + 220;
        motion_offset += 20;
        resource_offset += 8;
        i++;
    } while (i < 10);
    D_801B2F5C = 32;
    D_801B2F58++;
    func_800B07CC();
}
