/* Partial WMAP decompilation: 99.129036% (gcc280_g0). */
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
    s16 field_00;
    s16 angle;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    s16 field_10;
    s16 pad_12;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *data;
} WmapResource;

extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern u8 D_80123538[];
extern s32 D_801B0FD0;
extern s32 D_801B2BC8;
extern s32 D_801B2BCC;
extern void func_800970F8(void);

/** @brief Initialize the actor group and its animation resources, then advance. */
void func_80095C70(void)
{
    s32 i;

    D_801B0FD0 = 4;
    for (i = 140; i < 144; i++)
    {
        D_80139988[i].data = D_80123538;
        D_800D9268[i].field_02 = 0;
        D_800D9268[i].field_06 = 15;
        D_800D9268[i].field_0E = 1;
        D_800D9268[i].field_10 = -1;
        D_800D9268[i].field_22 = 129;
        D_800D9268[i].field_24 = 129;
        D_800D9268[i].field_26 = 8;
        D_801AFBD0[i].field_00 = 1;
        D_801AFBD0[i].angle = i << 10;
        D_801AFBD0[i].field_04 = -20000;
        D_801AFBD0[i].field_08 = 990000;
        D_801AFBD0[i].field_0C = 9999;
        D_801AFBD0[i].field_0E = 240;
        D_801AFBD0[i].field_10 = 160;
    }
    D_801B2BCC = 28;
    D_801B2BC8++;
    func_800970F8();
}
