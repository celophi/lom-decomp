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

extern void func_80098DB4(void);
extern WmapConfigA D_800D9268[];
extern s32 *D_80139280;
extern s32 D_801B2C18;
extern s32 D_801B2C1C;

/** @brief Reset four actor configurations and begin a 16-tick sequence step. */
void func_80098D34(void)
{
    s32 index;

    D_80139280[0] = -1;
    D_80139280[4] = 0;
    D_80139280[5] = 0;
    for (index = 20; index < 80; index += 15)
    {
        D_800D9268[index].field_22 = 0;
        D_800D9268[index].field_26 = 8;
    }
    D_801B2C1C = 0x10;
    D_801B2C18 += 1;
    func_80098DB4();
}
