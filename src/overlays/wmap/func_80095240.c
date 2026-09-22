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


extern WmapConfigA D_800D9370;
extern u8 D_80121538;
extern void *D_801399BC;
extern s32 D_801B2B68;
extern s32 D_801B2B6C;
extern void func_800952C0(void);

void func_80095240(void)
{
    D_801399BC = &D_80121538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 8;
    D_800D9370.field_0E = 1;
    D_800D9370.field_24 = 1;
    D_800D9370.field_02 = 0;
    D_800D9370.field_22 = 0x81;
    D_801B2B6C = 0x28;
    D_801B2B68 += 1;
    func_800952C0();
}
