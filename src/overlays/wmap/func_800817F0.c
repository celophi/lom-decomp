/* Partial WMAP decompilation: 92.707860% (gcc280_g0). */
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

extern WmapConfigA D_800D94FC[];
extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801AFCFC[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern u8 D_80125538[];
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_801B2860;
extern s32 D_801B2864;
extern void func_80081954(void);

/** @brief Initialize effect actors at regular angular intervals. */
void func_800817F0(void)
{
    s32 i;

    D_80139280[0x25] = 15;
    D_80139280[0x26] = 19;
    D_80139280[0x27] = 0;
    for (i = 0; i < 60; i++)
    {
        D_801AFBD0[i + 15].state = 0;
        D_80139988[i + 15].resource = D_80125538;
    }
    D_80139280[0x21] = 256;
    for (i = 0; i < 60; i += 5)
    {
        D_800D94FC[i].field_26 = 2;
        D_800D94FC[i].field_22 = 129;
        D_800D94FC[i].field_24 = 8;
        D_800D94FC[i].field_02 = 0;
        D_800D94FC[i].field_06 = 15;
        D_800D94FC[i].field_0E = 0;
        D_800D94FC[i].field_10 = -1;
        D_801AFCFC[i].state = 1;
        D_801AFCFC[i].z = 0;
        D_801AFCFC[i].scale = 60;
        D_801AFCFC[i].angle = D_80139280[0x21];
        D_80139280[0x21] += 341;
        D_801AFCFC[i].field_0E = 0;
    }
    D_8013B264 = 60;
    D_8013B270 = 4;
    D_8013B278 = 5000;
    D_80139280[0x23] = -1;
    D_80139280[0x28] = 0;
    D_801B2864 = 60;
    D_801B2860++;
    func_80081954();
}
