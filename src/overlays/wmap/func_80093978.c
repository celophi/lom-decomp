/* Partial WMAP decompilation: 90.802200% (gcc280_g0). */
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

extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern u8 D_8011D538[];
extern s32 D_801B2B70;
extern s32 D_801B2B74;
extern s32 rand(void);
extern s32 ccos(s32);
extern void func_80093AE4(void);

/** @brief Initialize randomized actors along a cosine depth curve. */
void func_80093978(void)
{
    s32 i;
    WmapConfigA *actor;
    s32 phase;
    s32 motion_offset;
    WmapMotion *motion;

    phase = 0;
    motion_offset = 150 * sizeof(WmapMotion);
    for (i = 150; i < 180; i++)
    {
        actor = &D_800D9268[i];
        D_80139988[i].resource = D_8011D538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_26 = 2;
        actor->field_02 = 0;
        actor->field_0E = 1;
        actor->field_22 = 129;
        actor->field_24 = 1;
        motion = (WmapMotion *)((u8 *)D_801AFBD0 + motion_offset);
        motion_offset += sizeof(WmapMotion);
        motion->angle = rand() & 4095;
        motion->field_0E = (i - 150) * 4;
        motion->z = ccos(2048 - (phase / 30)) * 120;
        phase += 1024;
        motion->x = ((rand() * 80) >> 15) + 40;
    }
    D_801B2B74 = 64;
    D_801B2B70++;
    func_80093AE4();
}
