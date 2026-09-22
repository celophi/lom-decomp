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

extern void func_80078934(void);
extern WmapConfigA D_800D9370;
extern u8 D_8011D538[];
extern u8 *D_801399BC;
extern s32 D_801B2630;
extern s32 D_801B2634;

/** @brief Initialize actor configuration and begin a 20-tick sequence step. */
void func_800788B4(void)
{
    D_801399BC = D_8011D538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_0E = 2;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = -1;
    D_800D9370.field_22 = 0x81;
    D_800D9370.field_02 = 0;
    D_800D9370.field_24 = 0x90;
    D_801B2634 = 0x14;
    D_801B2630 += 1;
    func_80078934();
}
