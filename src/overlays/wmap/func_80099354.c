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

extern void func_800993F0(void);
extern WmapConfigA D_800D944C;
extern s32 D_8011CF4C;
extern u8 D_80121538[];
extern u8 *D_801399E4;
extern s16 D_80182D60[];
extern s32 D_801B2C38;
extern s32 D_801B2C3C;

/** @brief Initialize the actor and its screen coordinates, then advance the sequence. */
void func_80099354(void)
{
    s16 screen_y;

    D_801399E4 = D_80121538;
    D_800D944C.field_06 = 0xF;
    D_800D944C.field_02 = 0;
    D_800D944C.field_0E = 1;
    D_800D944C.field_10 = -1;
    D_800D944C.field_26 = 0x20;
    D_800D944C.field_22 = 0x81;
    D_800D944C.field_24 = 1;
    D_801B2C3C = 0x80;
    D_80182D60[0] = (u16) D_8011CF4C;
    screen_y = D_8011CF4C - 0x38;
    D_80182D60[1] = screen_y;
    D_801B2C38 += 1;
    func_800993F0();
}
