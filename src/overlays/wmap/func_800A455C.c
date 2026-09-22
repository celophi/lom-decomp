/* Partial WMAP decompilation: 87.156250% (gcc280_g0). */
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
} __attribute__((aligned(4))) WmapConfigA;


extern WmapConfigA D_800D9318;
extern u8 D_8011F538;
extern void *D_801399AC;
extern s32 D_801B2DE0;
extern s32 D_801B2DE4;
extern void func_800A45DC(void);

void func_800A455C(void)
{
    D_801399AC = &D_8011F538;
    D_800D9318.field_06 = 0xF;
    D_800D9318.field_10 = -1;
    D_800D9318.field_26 = 8;
    D_800D9318.field_0E = 1;
    D_800D9318.field_24 = 1;
    D_800D9318.field_02 = 0;
    D_800D9318.field_22 = 0x81;
    D_801B2DE4 = 0x80;
    D_801B2DE0 += 1;
    func_800A45DC();
}
