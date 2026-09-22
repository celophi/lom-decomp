#include "common.h"

/** @brief World-map actor configuration with the original field layout. */
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

extern void func_80098F34(void);
extern WmapConfigA D_800D9370;
extern u8 D_8011D538[];
extern u8 *D_801399BC;
extern s32 D_801B2C20;
extern s32 D_801B2C24;

/** @brief Initialize the actor configuration and begin a 129-tick sequence step. */
void func_80098EB8(void)
{
    D_801399BC = D_8011D538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_0E = 1;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 1;
    D_800D9370.field_02 = 0;
    D_800D9370.field_22 = 0x80;
    D_800D9370.field_24 = 0x80;
    D_801B2C24 = 0x81;
    D_801B2C20 += 1;
    func_80098F34();
}
