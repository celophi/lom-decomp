#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern u8* D_80122B74;
extern s32* D_80122B78;
extern void* D_80122B70;

void func_800B0BDC(void)
{
    D_80122B74 = g_menuLayoutBuffer;
    D_80122B78 = &D_80122C00;
    D_80122B70 = (void*)0x801ED480;
}
