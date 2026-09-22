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

extern u8* D_80139A1C;
extern u8 D_8011F538[];
extern WmapConfigA D_800D9580;
extern s32 D_801B25E0;
extern s32 D_801B25B0;
extern s32 D_801B25B4;
extern void func_800748E4(void);

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80076394(void)
{
    D_80139A1C = D_8011F538;
    D_800D9580.field_06 = 0xF;
    D_800D9580.field_0E = 1;
    D_800D9580.field_10 = -1;
    D_801B25E0 = 0;
    D_800D9580.field_02 = 0;
    D_801B25B4 = 0x20;
    D_801B25B0 += 1;
    func_800748E4();
}
