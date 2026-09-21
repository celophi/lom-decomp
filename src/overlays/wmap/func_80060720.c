#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"

extern s32 D_800DCEDC;
extern u8 D_80129560[];

/** @brief Configure the two world-map frame buffers and clear their ordering tables. */
void func_80060720(void)
{
    SetGeomOffset(160, 120);
    D_800DCEDC = 0x4000;
    SetGeomScreen(0x4000);
    SetDispMask(1);
    SetDefDrawEnv((DRAWENV *)D_80129560, 0, 8, 320, 224);
    SetDefDrawEnv((DRAWENV *)(D_80129560 + 0x7E40), 0, 248, 320, 224);
    SetDefDispEnv((DISPENV *)(D_80129560 + 0x5C), 0, 240, 320, 240);
    SetDefDispEnv((DISPENV *)(D_80129560 + 0x7E9C), 0, 0, 320, 240);
    *(s16 *)(D_80129560 + 0x7E40) = 8;
    ((DRAWENV *)D_80129560)->clip.x = 8;
    *(s16 *)(D_80129560 + 0x7E44) = 312;
    ((DRAWENV *)D_80129560)->clip.w = 312;
    ClearOTagR((u_long *)(D_80129560 + 0x70), 179);
    ClearOTagR((u_long *)(D_80129560 + 0x7EB0), 179);
}
