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
} WmapConfigA;

extern WmapConfigA D_800D9268[];
extern s32 D_801B2A00;
extern s32 D_801B2A04;
extern void func_8008C750(void);

/** @brief Set particle display parameters and begin their countdown. */
void func_8008C6E8(void)
{
    s32 i;

    for (i = 100; i < 124; i++)
    {
        D_800D9268[i].field_26 = 4;
        D_800D9268[i].field_22 = 1;
    }
    D_801B2A04 = 64;
    D_801B2A00++;
    func_8008C750();
}
