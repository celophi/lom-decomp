/* Partial WMAP decompilation: 87.641304% (gcc280_g0). */
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
extern u8 D_80121538[];
extern s32 D_801B0FD0;
extern s32 D_801B29F0;
extern s32 D_801B29F4;
extern s32 rand(void);
extern void func_8008C42C(void);

/** @brief Initialize the effect descriptor and actors with randomized angles. */
void func_8008ADC0(void)
{
    s32 i;
    s32 angle;
    WmapConfigA *actor;
    s32 actor_offset;
    s32 resource_offset;
    WmapResource *resource;
    WmapMotion *motion;

    actor_offset = 0;
    resource_offset = 50 * 8;
    D_801B0FD0 = 20;
    D_80139280[0xB] = 64;
    D_80139280[0xC] = 20;
    D_80139280[0xD] = 128;
    D_80139280[0xE] = 0;
    D_80139280[0xF] = -1;
    D_80139280[0x10] = 2000;
    D_80139280[0x11] = 50;
    D_80139280[0x12] = 8;
    D_80139280[0x13] = 1;
    D_80139280[0x14] = 10;
    for (i = 0; i < 20; i++)
    {
        motion = &D_801AFFB8[i];
        motion->state = 1;
        angle = rand();
        actor = (WmapConfigA *)((u8 *)D_800D9B00 + actor_offset);
        actor_offset += 44;
        resource = (WmapResource *)((u8 *)D_80139988 + resource_offset);
        resource_offset += 8;
        motion->angle = angle & 4095;
        motion->scale = 128;
        motion->z = 10;
        motion->x = 0;
        motion->field_0E = 0;
        resource->resource = D_80121538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_26 = 2;
        actor->field_24 = 127;
        actor->field_02 = 0;
        actor->field_0E = 1;
        actor->field_22 = 0;
    }
    D_801B29F4 = 64;
    D_801B29F0++;
    func_8008C42C();
}
