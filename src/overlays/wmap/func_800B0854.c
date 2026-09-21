#include "common.h"

typedef struct
{
    u8 pad_00[0x26];
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

extern WmapConfigA D_800D9268[];
extern s32 D_801B2F58;
extern s32 D_801B2F5C;
extern void func_800B08B8(void);

/** @brief World-map step handler: arm a config range, set the timer, and advance. */
void func_800B0854(void)
{
    s32 i;

    for (i = 0; i < 0xA; i++)
    {
        D_800D9268[i + 0x14].field_26 = 2;
    }
    D_801B2F5C = 0x64;
    D_801B2F58 += 1;
    func_800B08B8();
}
