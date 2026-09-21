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

extern WmapConfigA D_800D9268[];
extern s32 D_801B2A28;
extern s32 D_801B2A2C;
extern void func_8008E094(void);

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void func_8008E030(void)
{
    s32 i;

    for (i = 0xC8; i < 0xCD; i++)
    {
        D_800D9268[i].field_22 = 0;
        D_800D9268[i].field_26 = 8;
    }
    D_801B2A2C = 0x10;
    D_801B2A28 += 1;
    func_8008E094();
}
