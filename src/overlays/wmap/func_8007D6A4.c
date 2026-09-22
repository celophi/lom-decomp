/* Partial WMAP decompilation: 94.976746% (gcc280_g0). */
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

/** @brief Unaligned eight-byte configuration copy. */
typedef struct
{
    u8 bytes[8];
} WmapConfigBytes;
extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern WmapConfigBytes D_80139258;
extern WmapConfigBytes D_801B2490;
extern u8 D_8011D538[];
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_801B2770;
extern s32 D_801B2774;
extern void func_8007D7FC(void);

/** @brief Initialize four equally spaced actors and start the effect. */
void func_8007D6A4(void)
{
    s32 i;

    D_801B2490 = D_80139258;
    D_80139234 = 1;
    D_8013923C = 2;
    D_80139240 = 0;
    for (i = 20; i < 80; i++)
    {
        D_801AFBD0[i].state = 0;
    }
    for (i = 20; i < 80; i += 15)
    {
        D_80139988[i].resource = D_8011D538;
        D_800D9268[i].field_06 = 15;
        D_800D9268[i].field_10 = -1;
        D_800D9268[i].field_26 = 2;
        D_800D9268[i].field_02 = 0;
        D_800D9268[i].field_0E = 0;
        D_800D9268[i].field_22 = 1;
        D_800D9268[i].field_24 = 129;
        D_801AFBD0[i].state = 1;
        D_801AFBD0[i].z = 150000;
        D_801AFBD0[i].field_0E = 0;
        D_801AFBD0[i].angle = D_80139240;
        D_80139240 += 1024;
    }
    D_801B2774 = 32;
    D_801B2770++;
    func_8007D7FC();
}
