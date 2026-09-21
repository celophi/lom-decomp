#include "common.h"

typedef struct
{
    u8 pad_00[0x26];
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

extern WmapConfigA D_800D9268[];
extern s32 D_801B2F60;
extern s32 D_801B2F64;
extern void func_800B0AD4(void);

/** @brief World-map step handler: arm a config range, set the timer, and advance. */
void func_800B0A70(void)
{
    s32 i;

    for (i = 0; i < 0x1E; i++)
    {
        D_800D9268[i + 0x32].field_26 = 4;
    }
    D_801B2F64 = 0x40;
    D_801B2F60 += 1;
    func_800B0AD4();
}
