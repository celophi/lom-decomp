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


extern WmapConfigA D_800D93F4;
extern u8 D_8011F538;
extern void *D_801399D4;
extern s32 D_801B2BF8;
extern s32 D_801B2BFC;
extern void func_8009869C(void);

void func_8009861C(void)
{
    D_801399D4 = &D_8011F538;
    D_800D93F4.field_06 = 0xF;
    D_800D93F4.field_10 = -1;
    D_800D93F4.field_26 = 2;
    D_800D93F4.field_0E = 1;
    D_800D93F4.field_24 = 1;
    D_800D93F4.field_02 = 0;
    D_800D93F4.field_22 = 0x81;
    D_801B2BFC = 0x64;
    D_801B2BF8 += 1;
    func_8009869C();
}
