#include "common.h"

/** @brief World-map actor configuration with its original field layout. */
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

extern void func_800A01F0(void);
extern WmapConfigA D_800D93C8;
extern s32 D_8011CF4C;
extern u8 D_80123538[];
extern s32 D_80139260;
extern u8 *D_801399CC;
extern s32 D_80182D60;
extern s32 D_801B2D38;
extern s32 D_801B2D3C;

/** @brief Initialize the actor and begin a 48-tick sequence step. */
void func_800A0154(void)
{
    D_801399CC = D_80123538;
    D_80139260 = 0x320;
    D_800D93C8.field_06 = 0xF;
    D_800D93C8.field_02 = 0;
    D_800D93C8.field_0E = 1;
    D_800D93C8.field_10 = -1;
    D_800D93C8.field_26 = 4;
    D_800D93C8.field_22 = 0x81;
    D_800D93C8.field_24 = 1;
    D_801B2D3C = 0x30;
    D_80182D60 = D_8011CF4C;
    D_801B2D38 += 1;
    func_800A01F0();
}
