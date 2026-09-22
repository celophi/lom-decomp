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

extern u8* D_80139A14;
extern u8 D_8011F538[];
extern WmapConfigA D_800D9554;
extern s32 D_801B25DC;
extern s32 D_801B25A8;
extern s32 D_801B25AC;
extern void func_80074748(void);

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80076248(void)
{
    D_80139A14 = D_8011F538;
    D_800D9554.field_06 = 0xF;
    D_800D9554.field_0E = 1;
    D_800D9554.field_10 = -1;
    D_801B25DC = 0;
    D_800D9554.field_02 = 0;
    D_801B25AC = 0x18;
    D_801B25A8 += 1;
    func_80074748();
}
