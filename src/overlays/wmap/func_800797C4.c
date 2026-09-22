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


extern s32 D_801B0FD0;
extern s32 D_801B25D8;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_8013B280;
extern s32 D_8013B284;
extern s32 D_801B26D8;
extern s32 D_801B26DC;
extern WmapConfigA D_800DB578[];
extern WmapMotion D_801B0B70[];
extern WmapResource D_80139988[];
extern u8 D_8011D538[];
extern s32 rand(void);
extern void func_8007AF6C(void);

/** @brief Initialize the effect actors and randomized motion angles. */
void func_800797C4(void)
{
    s32 i;
    WmapConfigA *actor;
    WmapMotion *motion;

    D_801B0FD0 = 40;
    D_801B25D8 = 256;
    D_80139234 = 48;
    D_8013923C = 10;
    D_80139240 = 60;
    D_8013924C = 2;
    D_80139250 = -1;
    D_80139260 = 8000;
    D_80139264 = 200;
    D_80139268 = 19;
    D_8013926C = 1;
    D_80139284 = 200;
    D_8013B264 = 54;
    D_8013B270 = 1;
    D_8013B278 = 30;
    D_8013B280 = 1;
    D_8013B284 = 3;
    for (i = 0; i < 40; i++)
    {
        actor = &D_800DB578[i];
        motion = &D_801B0B70[i];
        D_80139988[i + 204].resource = D_8011D538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_22 = 128;
        actor->field_24 = 128;
        actor->field_02 = 0;
        actor->field_26 = 0;
        actor->field_0E = D_8013926C;
        motion->state = 1;
        motion->angle = rand() & 0xFFF;
        motion->scale = 60;
        motion->field_0E = 50;
        motion->x = 0;
        motion->z = D_80139284;
    }
    D_801B26DC = 90;
    D_801B26D8++;
    func_8007AF6C();
}
