/* Partial WMAP decompilation: 94.914894% (gcc280_g0). */
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
extern s32 D_80182DEC;
extern s32 D_801B0FD0;
extern s32 D_801B2720;
extern s32 D_801B2724;
extern WmapConfigA D_800DB578[];
extern WmapMotion D_801B0B70[];
extern WmapResource D_80139988[];
extern u8 D_8011D538[];
extern void func_8007CB90(void);

/** @brief Initialize eight effect actors with evenly spaced angles. */
void func_8007B594(void)
{
    s32 i;
    WmapConfigA *actor;
    D_801B0FD0 = 8;
    D_80182DEC = 0x7F;
    D_80139240 = 0x50;
    D_8013924C = 2;
    D_80139250 = -1;
    D_80139260 = 0x44C;
    D_80139264 = 0xC8;
    D_80139268 = 0x15;
    D_8013926C = 2;
    D_80139284 = 0;
    D_8013B264 = 0x28;
    D_8013B270 = 0x50;
    D_8013B278 = 0xA;
    D_8013B280 = 0x50;
    for (i = 0; i < 8; i++)
    {
        actor = &D_800DB578[i];
        D_80139988[i + 204].resource = D_8011D538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_24 = 255;
        actor->field_26 = 3;
        actor->field_02 = 0;
        actor->field_22 = 0;
        actor->field_0E = D_8013926C;
        D_801B0B70[i].state = 1;
        D_801B0B70[i].scale = 80;
        D_801B0B70[i].angle = i << 9;
        D_801B0B70[i].x = 0;
        D_801B0B70[i].field_0E = 0;
        D_801B0B70[i].z = D_80139284;
    }
    D_801B2724 = 120;
    D_801B2720++;
    func_8007CB90();
}
