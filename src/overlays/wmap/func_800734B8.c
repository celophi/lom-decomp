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

extern void func_80073530(void);
extern WmapConfigA D_800D9268[];
extern u8 D_8011D538[];
extern u8 *D_801399AC;
extern s32 D_801B2528;
extern s32 D_801B252C;

/** @brief Initialize the actor configuration and begin an eight-tick sequence step. */
void func_800734B8(void)
{
    D_801399AC = D_8011D538;
    D_800D9268[4].field_06 = 0xF;
    D_800D9268[4].field_0E = 1;
    D_800D9268[4].field_10 = -1;
    D_800D9268[4].field_24 = 1;
    D_800D9268[4].field_02 = 0;
    D_800D9268[4].field_22 = 0xF1;
    D_801B252C = 8;
    D_801B2528 += 1;
    func_80073530();
}
