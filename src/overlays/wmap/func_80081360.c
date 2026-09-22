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

extern s32 D_80139240;
extern s32 D_80139250;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_801B2850;
extern s32 D_801B2854;
extern WmapConfigA D_800DA708[];
extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801B0530[];
extern WmapResource D_80139988[];
extern u8 D_80123538[];
extern s32 rand(void);
extern void func_80081508(void);

/** @brief Initialize spaced actors with randomized animation choices. */
void func_80081360(void)
{
    s32 i;
    WmapConfigA *actor;

    D_80139264 = 120;
    D_80139268 = 19;
    D_8013926C = 0;
    for (i = 0; i < 60; i++)
    {
        D_801AFBD0[i + 120].state = 0;
        D_80139988[i + 120].resource = D_80123538;
    }
    D_80139240 = 0;
    for (i = 0; i < 60; i += 5)
    {
        actor = &D_800DA708[i];
        actor->field_02 = 0;
        actor->field_06 = 15;
        actor->field_0E = rand() % 3;
        actor->field_10 = -1;
        actor->field_26 = 2;
        actor->field_22 = 129;
        actor->field_24 = 8;
        D_801B0530[i].state = 1;
        D_801B0530[i].z = 0;
        D_801B0530[i].scale = 60;
        D_801B0530[i].angle = D_80139240;
        D_80139240 += 341;
        D_801B0530[i].field_0E = 0;
    }
    D_8013B264 = 60;
    D_8013B270 = 4;
    D_8013B278 = 5000;
    D_80139284 = 0;
    D_80139250 = -1;
    D_801B2854 = 60;
    D_801B2850++;
    func_80081508();
}
