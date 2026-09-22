/* Partial WMAP decompilation: 96.525420% (gcc280_g0). */
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

typedef struct
{
    u8 bytes[8];
} WmapConfigBytes;
typedef struct
{
    s32 x, y, z, pad;
} WmapVector;
extern WmapConfigA D_800D9370[];
extern u8 D_8011F538[];
extern WmapConfigBytes D_80139258;
extern WmapConfigBytes D_801B24A0;
extern WmapVector D_80139870;
extern WmapVector D_80182DC0;
extern WmapResource D_80139988[];
extern WmapMotion D_801AFC98[];
extern WmapMotion *D_801B2560;
extern s32 D_80139980;
extern s32 D_801B2470;
extern s32 D_801B2540;
extern s32 D_801B2544;
extern s32 rand(void);
extern void func_80073CB0(void);

/** @brief Initialize twenty effect actors with alternating motion parameters. */
void func_80072B58(void)
{
    s32 i = 0;
    s16 angle = i;
    WmapConfigA *actor;
    WmapMotion *motion;

    D_801B2560 = D_801AFC98;
    D_801B24A0 = D_80139258;
    D_80139870 = D_80182DC0;
    D_801B2470 = 128;
    for (; i < 20; i++)
    {
        actor = &D_800D9370[i];
        D_801B2560[i].state = 0;
        D_80139988[i + 6].resource = D_8011F538 + ((i % 3) << 13);
        actor->field_02 = 0;
        actor->field_06 = 15;
        actor->field_0E = rand() & 1;
        actor->field_10 = -1;
        actor->field_22 = 128;
        actor->field_24 = 128;
        motion = (WmapMotion *)((i * sizeof(WmapMotion)) + (u8 *)D_801B2560);
        motion->state = 1;
        motion->x = 30000;
        motion->angle = angle;
        motion->z = 0;
        if (i & 1)
        {
            motion->field_0E = 30;
        }
        else
        {
            motion->field_0E = 0;
        }
        angle += 204;
    }
    D_80139980 = 128;
    D_801B2544 = 36;
    D_801B2540++;
    func_80073CB0();
}
