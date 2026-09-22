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

extern WmapConfigA D_800D95D8[];
extern WmapMotion D_801AFD60[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern s32 D_801B0FD0;
extern u8 D_80121538[];
extern s32 D_801B29A0;
extern s32 D_801B29A4;
extern void func_8008A7C8(void);

/** @brief Initialize the effect actors, resources, and evenly spaced angles. */
void func_80089004(void)
{
    s32 i;

    i = 0;
    D_801B0FD0 = 12;
    D_80139280[0x15] = 128;
    D_80139280[0x16] = 1;
    D_80139280[0x17] = 128;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = -1;
    D_80139280[0x1A] = 1500;
    D_80139280[0x1B] = 20;
    D_80139280[0x1C] = 8;
    D_80139280[0x1D] = 1;
    D_80139280[0x1E] = 10;
    do
    {
        D_801AFD60[i].state = 1;
        D_801AFD60[i].angle = i * 341;
        D_801AFD60[i].scale = 128;
        D_801AFD60[i].z = 10;
        D_801AFD60[i].x = 0;
        D_801AFD60[i].field_0E = 0;
        D_80139988[i + 20].resource = D_80121538;
        D_800D95D8[i].field_06 = 15;
        D_800D95D8[i].field_10 = -1;
        D_800D95D8[i].field_26 = 2;
        D_800D95D8[i].field_02 = 0;
        D_800D95D8[i].field_0E = 1;
        D_800D95D8[i].field_22 = 0;
        D_800D95D8[i].field_24 = 127;
        i++;
    } while (i < 12);
    D_801B29A4 = 32;
    D_801B29A0++;
    func_8008A7C8();
}
