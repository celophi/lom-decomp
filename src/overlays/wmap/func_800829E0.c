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

extern void func_80082A70(void);
extern WmapConfigA D_800D9370;
extern s32 D_8011CF4C;
extern u8 D_80121538[];
extern u8 *D_801399BC;
extern s32 D_80182D58;
extern s32 D_801B2818;
extern s32 D_801B281C;

/** @brief Configure the actor, save its screen position, and begin a 44-tick delay. */
void func_800829E0(void)
{
    D_801399BC = D_80121538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_0E = 1;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 8;
    D_800D9370.field_22 = 0x80;
    D_800D9370.field_24 = 0x80;
    D_800D9370.field_02 = 0;
    D_801B281C = 0x2C;
    D_80182D58 = D_8011CF4C;
    D_801B2818 += 1;
    func_80082A70();
}
