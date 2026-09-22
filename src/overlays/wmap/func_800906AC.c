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


extern WmapConfigA D_800D93C8;
extern u8 D_8011D538;
extern void *D_801399CC;
extern s32 D_801B2A80;
extern s32 D_801B2A84;
extern void func_8009072C(void);

void func_800906AC(void)
{
    D_801399CC = &D_8011D538;
    D_800D93C8.field_06 = 0xF;
    D_800D93C8.field_10 = -1;
    D_800D93C8.field_26 = 2;
    D_800D93C8.field_0E = 1;
    D_800D93C8.field_24 = 1;
    D_800D93C8.field_02 = 0;
    D_800D93C8.field_22 = 0x81;
    D_801B2A84 = 0x60;
    D_801B2A80 += 1;
    func_8009072C();
}
