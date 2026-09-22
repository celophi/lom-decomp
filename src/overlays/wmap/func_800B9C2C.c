/* Partial WMAP decompilation: 87.156250% (gcc280_g0). */
#include "common.h"
typedef struct { s16 field_00; s16 field_02; u8 pad_04[2]; u8 field_06; u8 pad_07[7]; s16 field_0E; s16 field_10; u8 pad_12[0x10]; s16 field_22; s16 field_24; s16 field_26; u8 pad_28[4]; } WmapConfigA;
extern WmapConfigA D_800D93C8; extern u8 D_8011D538; extern void* D_801399CC; extern s32 D_801B30E0; extern s32 D_801B30E4; extern void func_800B9CAC(void);
void func_800B9C2C(void)
{
    s32 value;
    s32 one;
    volatile WmapConfigA* config;

    value = 2;
    one = 1;
    config = &D_800D93C8;
    D_801399CC = &D_8011D538;
    config->field_06 = 0xF; config->field_10 = -1; config->field_22 = 0x81;
    config->field_0E = value; config->field_26 = value; config->field_02 = 0; config->field_24 = one;
    D_801B30E4 = 0x6B; D_801B30E0++; func_800B9CAC();
}
