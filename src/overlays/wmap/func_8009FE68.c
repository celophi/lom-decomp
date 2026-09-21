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

extern void func_8009FF00(void);
extern WmapConfigA D_800D939C;
extern s32 D_8011CF4C;
extern u8 D_80123538[];
extern s32 D_80139250;
extern u8 *D_801399C4;
extern s32 D_80182D58;
extern s32 D_801B2D30;
extern s32 D_801B2D34;

/** @brief Initialize the actor, save its screen position, and begin a 142-tick sequence step. */
void func_8009FE68(void)
{
    D_801399C4 = D_80123538;
    D_80139250 = 0x780;
    D_800D939C.field_06 = 0xF;
    D_800D939C.field_10 = -1;
    D_800D939C.field_26 = 1;
    D_800D939C.field_22 = 0x7F;
    D_800D939C.field_02 = 0;
    D_800D939C.field_0E = 0;
    D_800D939C.field_24 = 0;
    D_801B2D34 = 0x8E;
    D_80182D58 = D_8011CF4C;
    D_801B2D30 += 1;
    func_8009FF00();
}
