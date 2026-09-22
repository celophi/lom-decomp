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

/** @brief Byte-aligned configuration copied into the effect state. */
typedef struct
{
    u8 bytes[8];
} WmapConfigBytes;
extern WmapConfigBytes D_80139258;
extern WmapConfigBytes D_801B2490;
extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern u8 D_80121538[];
extern s32 D_801B24E8;
extern s32 D_801B24EC;
extern void func_800720C4(void);

/** @brief Initialize four effect actors and their angular spacing. */
void func_8007085C(void)
{
    s32 i;
    s32 *descriptor;
    WmapConfigA *actor;

    D_801B2490 = D_80139258;
    descriptor = D_80139280;
    descriptor[0] = 2;
    descriptor[1] = 1;
    descriptor[2] = 2;
    D_80139280[3] = 0;
    D_80139280[4] = 3500;
    D_80139280[5] = 96;
    D_80139280[6] = 4;
    D_80139280[7] = 0;
    D_80139280[8] = 10;
    D_80139280[9] = 8;
    for (i = 20; i < 80; i++)
    {
        D_801AFBD0[i].state = 0;
    }
    for (i = 20; i < 80; i += 15)
    {
        actor = &D_800D9268[i];
        D_80139988[i].resource = D_80121538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_02 = 0;
        actor->field_0E = 0;
        actor->field_26 = 4;
        actor->field_22 = 1;
        actor->field_24 = 129;
        D_801AFBD0[i].state = 1;
        D_801AFBD0[i].z = 150000;
        D_801AFBD0[i].angle = D_80139280[3];
        D_801AFBD0[i].field_0E = 0;
        D_80139280[3] += 1024;
    }
    D_801B24EC = 40;
    D_801B24E8++;
    func_800720C4();
}
