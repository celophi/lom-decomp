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

extern void func_8009FA0C(void);
extern WmapConfigA D_800D9370;
extern s32 D_8011CF4C;
extern u8 D_80125538[];
extern s32 D_8013924C;
extern u8 *D_801399BC;
extern s32 D_80182D64;
extern s32 D_801B2D20;
extern s32 D_801B2D24;

/** @brief Initialize the actor and its screen coordinates, then advance the sequence. */
void func_8009F970(void)
{
    D_801399BC = D_80125538;
    D_8013924C = 0x280;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 4;
    D_800D9370.field_22 = 0x81;
    D_800D9370.field_24 = 1;
    D_800D9370.field_02 = 0;
    D_800D9370.field_0E = 0;
    D_801B2D24 = 0x9C;
    D_80182D64 = D_8011CF4C;
    D_801B2D20 += 1;
    func_8009FA0C();
}
