#include "common.h"

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

extern u8* D_801399DC;
extern u8 D_80121538[];
extern WmapConfigA D_800D9420;
extern s32 D_80182DF4;
extern s32 D_801B25C8;
extern s32 D_801B25CC;
extern void func_8007694C(void);

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800768D8(void)
{
    D_801399DC = D_80121538;
    D_800D9420.field_06 = 0xF;
    D_800D9420.field_0E = 1;
    D_800D9420.field_10 = -1;
    D_80182DF4 = 0;
    D_800D9420.field_02 = 0;
    D_801B25CC = 0x59;
    D_801B25C8 += 1;
    func_8007694C();
}
